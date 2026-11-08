#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>
#include "Presets.h"

namespace tr
{
// Лёгкий движок: 8 голосов. Осцилляторы polyBLEP, FM-синус, FM "tine" для e-piano, саб, шум,
// питч-огибающая, вибрато, TPT-фильтр с key-tracking. Без оверсэмплинга.
// Против "хрипа": пониженные уровни голосов (аккорды не клиппируют), индекс FM падает на высоких
// нотах и отключается рядом с Найквистом, треугольник без алиасинга, потолок частоты фильтра.
class SynthEngine
{
public:
    static constexpr int kVoices = 8;

    void prepare (double sr)
    {
        sampleRate = sr;
        for (auto& v : voices) { v.active = false; v.held = false; }
    }

    void setMacros (const Preset& p, float cutMul, float attMul, float relMul, float resOff)
    {
        current = &p;
        cutoffMul = cutMul;
        attackMul = attMul;
        releaseMul = relMul;
        resOffset = resOff;
    }

    void noteOn (int note, float vel)
    {
        if (current == nullptr) return;

        Voice* v = nullptr;
        for (auto& x : voices)
            if (! x.active) { v = &x; break; }

        if (v == nullptr)
        {
            v = &voices[0];
            for (auto& x : voices)
                if (x.order < v->order) v = &x;
        }

        v->active = true;
        v->held = true;
        v->note = note;
        v->order = ++counter;
        v->vel = vel;
        v->pr = *current;
        v->freq = 440.0 * std::exp2 ((note - 69) / 12.0);
        v->r1 = std::exp2 (-(double) v->pr.detune / 2400.0);
        v->r2 = std::exp2 (v->pr.semi2 / 12.0 + (double) v->pr.detune / 2400.0);
        v->p1 = 0.0;
        v->p2 = (v->pr.tine > 0.f || v->pr.fmAmt > 0.f) ? 0.0 : (double) rng.nextFloat();
        v->p3 = 0.0;
        v->pm = 0.0;
        v->pt = 0.0;
        v->lfoPh = rng.nextFloat();
        v->pe = v->pr.pitchEnv;
        v->peDecay = (float) std::exp (-16.0 / (0.06 * sampleRate));
        v->nz = (uint32_t) rng.nextInt() | 1u;
        v->ktrack = juce::jlimit (0.5f, 2.2f, (float) std::pow (v->freq / 261.63, 0.35));
        for (int c = 0; c < 2; ++c) { v->ic1[c] = 0.f; v->ic2[c] = 0.f; }

        v->amp.setSampleRate (sampleRate);
        v->flt.setSampleRate (sampleRate);
        setEnv (*v);
        v->amp.noteOn();
        v->flt.noteOn();
    }

    void noteOff (int note)
    {
        for (auto& v : voices)
            if (v.active && v.held && v.note == note)
            {
                v.held = false;
                setEnv (v);
                v.amp.noteOff();
                v.flt.noteOff();
            }
    }

    void allNotesOff()
    {
        for (auto& v : voices)
            if (v.active && v.held)
            {
                v.held = false;
                v.amp.noteOff();
                v.flt.noteOff();
            }
    }

    int numActive() const
    {
        int n = 0;
        for (auto& v : voices) if (v.active) ++n;
        return n;
    }

    // Добавляет звук в буферы (они должны быть обнулены заранее)
    void render (float* L, float* R, int n)
    {
        const float sr = (float) sampleRate;
        const float maxFc = juce::jmin (11000.f, sr * 0.4f);
        const float twoPi = juce::MathConstants<float>::twoPi;

        for (auto& v : voices)
        {
            if (! v.active) continue;

            const Preset& pr = v.pr;
            const double base = v.freq / sampleRate;
            const bool fm = pr.fmAmt > 0.f;
            const bool tine = pr.tine > 0.f;
            const float velSc = 0.5f + 0.5f * v.vel;
            const float fmSc = velSc * juce::jlimit (0.2f, 1.f, 700.f / (float) v.freq);
            const float outGain = (0.3f + 0.7f * v.vel) * pr.gain * 0.55f;
            const float res = juce::jlimit (0.f, 0.95f, pr.res + resOffset);
            const float kq = 2.f * (1.f - 0.92f * res);

            double d1 = base, d2 = base, d3 = base * 0.5, dm = base, dt = base;

            for (int i = 0; i < n; ++i)
            {
                const float ae = v.amp.getNextSample();
                const float fe = v.flt.getNextSample();

                if (! v.amp.isActive()) { v.active = false; break; }

                if ((i & 15) == 0)
                {
                    float lfo = 0.f;
                    if (pr.lfoRate > 0.f)
                    {
                        lfo = std::sin (twoPi * v.lfoPh) * pr.lfoDepth;
                        v.lfoPh += pr.lfoRate * 16.f / sr;
                        if (v.lfoPh >= 1.f) v.lfoPh -= 1.f;
                    }
                    const double pm = std::exp2 ((double) (v.pe + lfo) / 12.0);
                    v.pe *= v.peDecay;

                    d1 = juce::jmin (0.45, base * v.r1 * pm);
                    d2 = juce::jmin (0.45, base * v.r2 * pm);
                    d3 = juce::jmin (0.45, base * 0.5 * pm);
                    dm = base * (double) pr.fmRatio * pm;
                    dt = base * 14.0 * pm;

                    float fc = pr.cutoff * cutoffMul * v.ktrack * std::exp2 (pr.fenv * fe);
                    fc = juce::jlimit (30.f, maxFc, fc);
                    const float g = std::tan (juce::MathConstants<float>::pi * fc / sr);
                    v.a1 = 1.f / (1.f + g * (g + kq));
                    v.a2 = g * v.a1;
                    v.a3 = g * v.a2;
                }

                // osc1: обычная волна или FM
                float o1;
                if (fm)
                {
                    const float idx = (dm < 0.4) ? pr.fmAmt * (0.25f + 0.75f * fe) * fmSc : 0.f;
                    o1 = std::sin (twoPi * (float) v.p1 + idx * std::sin (twoPi * (float) v.pm));
                }
                else
                    o1 = osc (pr.w1, v.p1, d1);

                // osc2: обычная волна или FM-"tine"
                float o2;
                if (tine)
                {
                    const float ti = (dt < 0.4) ? pr.tine * fe * fe * fmSc : 0.f;
                    o2 = std::sin (twoPi * (float) v.p2 + ti * std::sin (twoPi * (float) v.pt));
                }
                else
                    o2 = osc (pr.w2, v.p2, d2);

                float extra = 0.f;
                if (pr.sub > 0.f)
                    extra += std::sin (twoPi * (float) v.p3) * pr.sub * 0.7f;
                if (pr.noise > 0.f)
                {
                    v.nz ^= v.nz << 13; v.nz ^= v.nz >> 17; v.nz ^= v.nz << 5;
                    extra += ((float) (int32_t) v.nz * (1.0f / 2147483648.0f)) * pr.noise * fe * 0.6f;
                }

                v.p1 += d1; if (v.p1 >= 1.0) v.p1 -= 1.0;
                v.p2 += d2; if (v.p2 >= 1.0) v.p2 -= 1.0;
                v.p3 += d3; if (v.p3 >= 1.0) v.p3 -= 1.0;
                v.pm += dm; if (v.pm >= 1.0) v.pm -= std::floor (v.pm);
                v.pt += dt; if (v.pt >= 1.0) v.pt -= std::floor (v.pt);

                const float o2m = o2 * pr.mix2;
                const float l = o1 * 0.65f + o2m * 0.35f + extra;
                const float r = o1 * 0.35f + o2m * 0.65f + extra;
                const float g2 = ae * outGain;

                L[i] += svf (v, 0, l) * g2;
                R[i] += svf (v, 1, r) * g2;
            }
        }
    }

private:
    struct Voice
    {
        bool active = false, held = false;
        int note = -1;
        uint32_t order = 0, nz = 1;
        float vel = 0.f, lfoPh = 0.f, pe = 0.f, peDecay = 0.f, ktrack = 1.f;
        double freq = 440.0, p1 = 0.0, p2 = 0.0, p3 = 0.0, pm = 0.0, pt = 0.0, r1 = 1.0, r2 = 1.0;
        Preset pr {};
        juce::ADSR amp, flt;
        float ic1[2] { 0.f, 0.f }, ic2[2] { 0.f, 0.f };
        float a1 = 0.f, a2 = 0.f, a3 = 0.f;
    };

    void setEnv (Voice& v) const
    {
        const float a = juce::jmax (0.001f, v.pr.a * attackMul);
        const float r = juce::jmax (0.01f, v.pr.r * releaseMul);
        v.amp.setParameters ({ a, v.pr.d, v.pr.s, r });
        v.flt.setParameters ({ a, v.pr.d, v.pr.s * 0.35f, r });
    }

    static inline float blep (double t, double dt)
    {
        if (t < dt)       { t /= dt;            return (float) (t + t - t * t - 1.0); }
        if (t > 1.0 - dt) { t = (t - 1.0) / dt; return (float) (t * t + t + t + 1.0); }
        return 0.f;
    }

    static inline float pulse (double p, double dt, double w)
    {
        double q = p - w; if (q < 0.0) q += 1.0;
        return (p < w ? 1.f : -1.f) + blep (p, dt) - blep (q, dt);
    }

    static inline float osc (int w, double p, double dt)
    {
        switch (w)
        {
            case Sine:   return std::sin (juce::MathConstants<float>::twoPi * (float) p);
            case Saw:    return (float) (2.0 * p - 1.0) - blep (p, dt);
            case Square: return pulse (p, dt, 0.5);
            case Pulse:  return pulse (p, dt, 0.25);
            default:
            {   // треугольник без алиасинга: sin(x) - sin(3x)/9 = (2/3)s + (4/9)s^3
                const float s = std::sin (juce::MathConstants<float>::twoPi * (float) p);
                return s * (0.6667f + 0.4444f * s * s) * 0.9f;
            }
        }
    }

    // TPT state-variable lowpass
    static inline float svf (Voice& v, int ch, float x)
    {
        const float v3 = x - v.ic2[ch];
        const float v1 = v.a1 * v.ic1[ch] + v.a2 * v3;
        const float v2 = v.ic2[ch] + v.a2 * v.ic1[ch] + v.a3 * v3;
        v.ic1[ch] = 2.f * v1 - v.ic1[ch];
        v.ic2[ch] = 2.f * v2 - v.ic2[ch];
        return v2;
    }

    Voice voices[kVoices];
    const Preset* current = nullptr;
    double sampleRate = 44100.0;
    float cutoffMul = 1.f, attackMul = 1.f, releaseMul = 1.f, resOffset = 0.f;
    uint32_t counter = 0;
    juce::Random rng;
};
} // namespace tr
