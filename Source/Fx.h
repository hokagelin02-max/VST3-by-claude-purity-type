#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <cmath>

namespace tr
{
struct FxParams
{
    float drive = 0.f, lofi = 0.f, chorus = 0.f;
    float delayMix = 0.f, delayFb = 0.35f;
    float reverbAmt = 0.f, reverbSize = 0.5f;
    double delaySamples = 11025.0;
};

// Лёгкая цепочка эффектов: Drive -> Lo-Fi -> Chorus -> Delay (ping-pong) -> Reverb.
// Выключенные эффекты не считаются, поэтому нагрузка на CPU минимальная.
class FxChain
{
public:
    void prepare (double sr)
    {
        sampleRate = sr;
        const int dsize = (int) (sr * 2.0) + 8;
        delayL.assign ((size_t) dsize, 0.f);
        delayR.assign ((size_t) dsize, 0.f);
        delayPos = 0;
        for (auto& c : chorusBuf) c.assign (kChorusSize, 0.f);
        chorusPos = 0;
        chorusPhase = 0.f;
        lpL = lpR = 0.f;
        holdCount = 0;
        holdL = holdR = 0.f;
        reverb.setSampleRate (sr);
        reverb.reset();
    }

    void process (float* L, float* R, int n, const FxParams& p)
    {
        const float twoPi = juce::MathConstants<float>::twoPi;

        // ---- Drive
        if (p.drive > 0.01f)
        {
            const float k = 1.f + p.drive * 7.f;
            const float comp = 1.f / std::sqrt (k);
            for (int i = 0; i < n; ++i)
            {
                L[i] = std::tanh (L[i] * k) * comp;
                R[i] = std::tanh (R[i] * k) * comp;
            }
        }

        // ---- Lo-Fi (снижение разрядности и частоты дискретизации)
        if (p.lofi > 0.01f)
        {
            const int hold = 1 + (int) (p.lofi * 6.f);
            const float levels = std::exp2 (15.f - p.lofi * 7.f);
            for (int i = 0; i < n; ++i)
            {
                if (holdCount <= 0)
                {
                    holdL = std::round (L[i] * levels) / levels;
                    holdR = std::round (R[i] * levels) / levels;
                    holdCount = hold;
                }
                --holdCount;
                L[i] = holdL;
                R[i] = holdR;
            }
        }

        // ---- Chorus
        if (p.chorus > 0.01f)
        {
            const float baseD = (float) (sampleRate * 0.012);
            const float depth = (float) (sampleRate * 0.004) * (0.4f + 0.6f * p.chorus);
            const float inc = twoPi * 0.45f / (float) sampleRate;
            const float mix = p.chorus * 0.5f;

            for (int i = 0; i < n; ++i)
            {
                chorusBuf[0][(size_t) chorusPos] = L[i];
                chorusBuf[1][(size_t) chorusPos] = R[i];

                const float wl = readChorus (0, baseD + depth * std::sin (chorusPhase));
                const float wr = readChorus (1, baseD + depth * std::cos (chorusPhase));

                L[i] = L[i] * (1.f - mix * 0.5f) + wl * mix;
                R[i] = R[i] * (1.f - mix * 0.5f) + wr * mix;

                chorusPos = (chorusPos + 1) & (kChorusSize - 1);
                chorusPhase += inc;
                if (chorusPhase > twoPi) chorusPhase -= twoPi;
            }
        }

        // ---- Delay (ping-pong, тёмный фидбэк). Буфер пишется всегда, чтобы не было "старого эха".
        {
            const int size = (int) delayL.size();
            const int ds = juce::jlimit (1, size - 1, (int) p.delaySamples);
            const float fb = juce::jlimit (0.f, 0.9f, p.delayFb) * 0.9f;
            const float a = 1.f - std::exp (-twoPi * 3500.f / (float) sampleRate);

            for (int i = 0; i < n; ++i)
            {
                int rp = delayPos - ds;
                if (rp < 0) rp += size;

                const float dL = delayL[(size_t) rp];
                const float dR = delayR[(size_t) rp];
                lpL += a * (dL - lpL);
                lpR += a * (dR - lpR);

                delayL[(size_t) delayPos] = L[i] + lpR * fb;
                delayR[(size_t) delayPos] = R[i] + lpL * fb;

                L[i] += dL * p.delayMix;
                R[i] += dR * p.delayMix;

                if (++delayPos >= size) delayPos = 0;
            }
        }

        // ---- Reverb
        if (p.reverbAmt > 0.005f)
        {
            juce::Reverb::Parameters rp;
            rp.roomSize = 0.3f + 0.65f * p.reverbSize;
            rp.damping = 0.55f;
            rp.wetLevel = p.reverbAmt * 0.4f;
            rp.dryLevel = 1.f - 0.2f * p.reverbAmt;
            rp.width = 1.f;
            reverb.setParameters (rp);
            reverb.processStereo (L, R, n);
        }
    }

private:
    static constexpr int kChorusSize = 4096;

    float readChorus (int ch, float delaySamples) const
    {
        float pos = (float) chorusPos - delaySamples;
        if (pos < 0.f) pos += (float) kChorusSize;
        const int i0 = (int) pos & (kChorusSize - 1);
        const int i1 = (i0 + 1) & (kChorusSize - 1);
        const float fr = pos - std::floor (pos);
        return chorusBuf[ch][(size_t) i0] * (1.f - fr) + chorusBuf[ch][(size_t) i1] * fr;
    }

    double sampleRate = 44100.0;
    std::vector<float> delayL, delayR;
    int delayPos = 0;
    float lpL = 0.f, lpR = 0.f;
    std::vector<float> chorusBuf[2];
    int chorusPos = 0;
    float chorusPhase = 0.f;
    int holdCount = 0;
    float holdL = 0.f, holdR = 0.f;
    juce::Reverb reverb;
};
} // namespace tr
