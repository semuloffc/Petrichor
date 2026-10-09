#include "BurstEngine.h"
#include "SineTable.h"
#include "../Parameters/ParameterIDs.h"

namespace petrichor
{
void BurstEngine::prepare (double sr)
{
    sampleRate = sr;
    hp.reset (sr);
    tiltLp.reset (sr);
    tiltLp.setLowPass (1000.0f);
    thumpFreqStep = 40.0f / (float) (0.120 * sr);
    thumpDecay = std::exp ((float) (-1.0 / (0.040 * sr)));
    reset();
}

void BurstEngine::reset()
{
    hp.clear();
    tiltLp.reset (sampleRate);
    tiltLp.setLowPass (1000.0f);
    env = 0.0f;
    thumpEnv = 0.0f;
    thumpActive = false;
    pending = false;
    burstBarCount = 0;
}

void BurstEngine::trigger()
{
    pending = true;
}

void BurstEngine::startHit()
{
    env = 1.0f;
    thumpActive = true;
    thumpEnv = 1.0f;
    thumpPhase = 0.0f;
    thumpFreq = 80.0f;
}

void BurstEngine::process (float* outL, float* outR, int n, const EngineContext& ctx, const MacroState& macro)
{
    const float sr = (float) sampleRate;
    const float size = ctx.param (IDs::burst_size);
    const float tone = ctx.param (IDs::burst_tone);
    const float every = ctx.param (IDs::burst_every);
    const float prob = ctx.param (IDs::burst_prob);

    const float tau = juce::jmax (0.04f, size / 5.0f);
    decayCoef = std::exp ((float) (-1.0 / (tau * sr)));
    hp.setHighPass (150.0f, 0.70710678f);
    const float tilt = (tone / 100.0f) * 0.5f + 0.5f;

    if (pending) { startHit(); pending = false; }

    if (ctx.clock->barChanged)
    {
        ++burstBarCount;
        const int everyInt = (int) std::max (1.0f, every);
        if (burstBarCount >= everyInt)
        {
            burstBarCount = 0;
            if (macro.growth.burstArmed)
            {
                const float effProb = juce::jlimit (0.0f, 100.0f, prob + macro.burstProbExtra);
                if (noise.next() * 100.0f < effProb)
                    startHit();
            }
        }
    }

    for (int i = 0; i < n; ++i)
    {
        float nOut = hp.processL (noise.bipolar());
        const float low = tiltLp.processL (nOut);
        const float high = nOut - low;
        nOut = low * (1.0f - tilt) + high * tilt;

        float s = nOut * env;

        if (thumpActive)
        {
            thumpPhase += thumpFreq / sr;
            if (thumpPhase >= 1.0f) thumpPhase -= 1.0f;
            s += sineLookup (thumpPhase) * thumpEnv * 0.8f;

            thumpFreq -= thumpFreqStep;
            if (thumpFreq < 40.0f) thumpFreq = 40.0f;
            thumpEnv *= thumpDecay;
            if (thumpEnv < 1.0e-4f) { thumpEnv = 0.0f; thumpActive = false; }
        }

        env *= decayCoef;
        if (env < 1.0e-4f) env = 0.0f;

        outL[i] = s * 0.6f;
        outR[i] = s * 0.6f;
    }
}
}
