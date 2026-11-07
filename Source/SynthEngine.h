#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>
#include "Presets.h"

namespace tr
{
// Лёгкий движок: 8 голосов, 2 осциллятора (polyBLEP) + FM-синус, TPT-фильтр, 2 огибающие.
// Коэффициенты фильтра обновляются раз в 16 сэмплов, без оверсэмплинга -> очень низкая нагрузка.
class SynthEngine
{
public:
    static constexpr int kVoices = 8;

    void prepare (double sr)
    {
        sampleRate = sr;
        allNotesOff();
        for (auto& v : voices) v.active = false;
    }

    void setPreset (const Preset& p, float cutMul, float relMul)
    {
        current = &p;
        cutoffMul = cutMul;
        releaseMul = relMul;
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
        v->p2 = rng.nextDouble();
        v->pm = 0.0;
        for (int c = 0; c < 2; ++c) { v->ic1[c] = 0.f; v->ic2[c] = 0.f; }

        v->amp.setSampleRate (sampleRate);
        v->flt.setSampleRate (sampleRate);
        v->amp.setParameters ({ v->pr.a, v->pr.d, v->pr.s, v->pr.r * releaseMul });
        v->flt.setParameters ({ v->pr.a, v->pr.d, v->pr.s * 0.35f, v->pr.r * releaseMul });
        v->amp.noteOn();
        v->flt.noteOn();
    }

    void noteOff (int note)
    {
        for (auto& v : voices)
            if (v.active && v.held && v.note == note)
            {
                v.held = false;
                v.amp.setParameters ({ v.pr.a, v.pr.d, v.pr.s, v.pr.r * releaseMul });
                v.flt.setParameters ({ v.pr.a, v.pr.d, v.pr.s * 0.35f, v.pr.r * releaseMul });
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

    // Добавляет звук в буферы (буферы должны быть обнулены заранее)
    void render (float* L, float* R, int n)
    {
        const float maxFc = (float) juce::jmin (18000.0, sampleRate * 0.45);
        const float twoPi = juce::MathConstants<float>::twoPi;

        for (auto& v : voices)
        {
            if (! v.active) continue;

            const Preset& pr = v.pr;
            const double d1 = v.freq * v.r1 / sampleRate;
            const double d2 = v.freq * v.r2 / sampleRate;
            const double dm = v.freq * (double) pr.fmRatio / sampleRate;
            const bool fm = pr.fmAmt > 0.f;
            const float outGain = (0.3f + 0.7f * v.vel) * pr.gain;

            for (int i = 0; i < n; ++i)
            {
                const float ae = v.amp.getNextSample();
                const float fe = v.flt.getNextSample();

                if (! v.amp.isActive()) { v.active = false; break; }

                if ((i & 15) == 0)
                {
                    float fc = pr.cutoff * cutoffMul * std::exp2 (pr.fenv * fe);
                    fc = juce::jlimit (30.f, maxFc, fc);
                    const float g = std::tan (juce::MathConstants<float>::pi * fc / (float) sampleRate);
                    v.k = 2.f * (1.f - 0.92f * pr.res);
                    v.a1 = 1.f / (1.f + g * (g + v.k));
                    v.a2 = g * v.a1;
                    v.a3 = g * v.a2;
                }

                float o1;
                if (fm)
                {
                    const float idx = pr.fmAmt * (0.25f + 0.75f * fe);
                    o1 = std::sin (twoPi * (float) v.p1 + idx * std::sin (twoPi * (float) v.pm));
                }
                else
                    o1 = osc (pr.w1, v.p1, d1);

                const float o2 = osc (pr.w2, v.p2, d2);

                v.p1 += d1; if (v.p1 >= 1.0) v.p1 -= 1.0;
                v.p2 += d2; if (v.p2 >= 1.0) v.p2 -= 1.0;
                v.pm += dm; if (v.pm >= 1.0) v.pm -= std::floor (v.pm);

                const float l = o1 * 0.60f + o2 * 0.30f;
                const float r = o1 * 0.30f + o2 * 0.60f;
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
        uint32_t order = 0;
        float vel = 0.f;
        double freq = 440.0, p1 = 0.0, p2 = 0.0, pm = 0.0, r1 = 1.0, r2 = 1.0;
        Preset pr {};
        juce::ADSR amp, flt;
        float ic1[2] { 0.f, 0.f }, ic2[2] { 0.f, 0.f };
        float a1 = 0.f, a2 = 0.f, a3 = 0.f, k = 1.f;
    };

    static inline float blep (double t, double dt)
    {
        if (t < dt)       { t /= dt;               return (float) (t + t - t * t - 1.0); }
        if (t > 1.0 - dt) { t = (t - 1.0) / dt;    return (float) (t * t + t + t + 1.0); }
        return 0.f;
    }

    static inline float osc (int w, double p, double dt)
    {
        switch (w)
        {
            case Sine:   return std::sin (juce::MathConstants<float>::twoPi * (float) p);
            case Saw:    return (float) (2.0 * p - 1.0) - blep (p, dt);
            case Square:
            {
                double q = p + 0.5; if (q >= 1.0) q -= 1.0;
                return (p < 0.5 ? 1.f : -1.f) + blep (p, dt) - blep (q, dt);
            }
            default:     return 4.f * std::abs ((float) p - 0.5f) - 1.f;
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
    float cutoffMul = 1.f, releaseMul = 1.f;
    uint32_t counter = 0;
    juce::Random rng;
};
} // namespace tr
