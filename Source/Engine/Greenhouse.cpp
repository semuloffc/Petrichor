#include "Greenhouse.h"

namespace petrichor
{
void Greenhouse::prepare (double sr)
{
    sampleRate = sr;
    const int maxLen = (int) (sr * 6.0 * 137.0 / (double) primeSum) + 8;
    for (auto& l : lines) l.prepare (maxLen);
    predelay.prepare ((int) (0.08 * sr) + 8);
    for (auto& d : damp) d.reset (sr);
    lowcut.reset (sr);
    modDepth = (float) (0.0003 * sr);
    reset();
}

void Greenhouse::reset()
{
    for (auto& l : lines) l.clear();
    for (auto& d : damp) d.reset (sampleRate);
    lowcut.reset (sampleRate);
    predelay.clear();
    modPhase = 0.0f;
}

void Greenhouse::setParams (float sizeSec, float dampHz, float predelayMs, float lowcutHz)
{
    sizeSec = juce::jmax (0.1f, sizeSec);
    for (int i = 0; i < 8; ++i)
    {
        lineLen[(size_t) i] = juce::jlimit (16, lines[(size_t) i].len - 4,
                                            (int) (sampleRate * sizeSec * prime[(size_t) i] / (double) primeSum));
    }
    for (auto& d : damp) d.setLowPass (dampHz);
    lowcut.setHighPass (lowcutHz);
    predelayLen = juce::jlimit (1, predelay.len - 2, (int) (sampleRate * predelayMs * 0.001));
}

void Greenhouse::process (const float* inL, const float* inR, float* outL, float* outR, int n)
{
    const float sr = (float) sampleRate;
    const float invSqrt8 = 1.0f / 2.8284271247f;
    const float modInc = 6.2831853f * 0.15f / sr;

    for (int i = 0; i < n; ++i)
    {
        float mono = (inL[i] + inR[i]) * 0.5f;
        mono = lowcut.processL (mono);
        predelay.write (mono);
        const float in = predelay.readInt (predelayLen);

        float raw[8], damped[8];
        for (int k = 0; k < 8; ++k)
        {
            if (k < 2)
            {
                const float off = (float) lineLen[(size_t) k] + modDepth * std::sin (modPhase + k * 3.14159265358979f);
                raw[k] = lines[(size_t) k].readFrac (off);
            }
            else
            {
                raw[k] = lines[(size_t) k].readInt (lineLen[(size_t) k]);
            }
            damped[k] = damp[(size_t) k].processL (raw[k]);
        }

        modPhase += modInc;
        if (modPhase > 6.2831853f) modPhase -= 6.2831853f;

        for (int k = 0; k < 8; ++k)
        {
            float fb = 0.0f;
            for (int j = 0; j < 8; ++j)
                fb += hadamard[k][j] * damped[j];
            fb *= feedback * invSqrt8;
            lines[(size_t) k].write (fb + in * 0.5f);
        }

        float l = 0.0f, r = 0.0f;
        for (int k = 0; k < 8; ++k)
        {
            l += raw[k] * signL[k];
            r += raw[k] * signR[k];
        }
        outL[i] = l * invSqrt8 * 0.5f;
        outR[i] = r * invSqrt8 * 0.5f;
    }
}
}
