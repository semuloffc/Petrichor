#include "RootsEngine.h"
#include "../Parameters/ParameterIDs.h"

namespace petrichor
{
void RootsEngine::prepare (double sr)
{
    sampleRate = sr;
    hp.reset (sr);
    lp.reset (sr);
    mono.setSampleRate (sr);
    leak = std::exp ((float) (-1.0 / (0.7 * sr)));
    reset();
}

void RootsEngine::reset()
{
    hp.clear();
    lp.clear();
    mono.clear();
    brownL = brownR = 0.0f;
    phase = 0.0f;
}

void RootsEngine::process (float* outL, float* outR, int n, const EngineContext& ctx, const MacroState& macro)
{
    const float sr = (float) sampleRate;
    const float rate = ctx.param (IDs::roots_rate);
    const float depth = ctx.param (IDs::roots_depth) / 100.0f;
    const float monoHz = ctx.param (IDs::roots_mono);

    hp.setHighPass (40.0f, 0.70710678f);
    lp.setLowPass (200.0f, 0.70710678f);
    mono.setCrossover (monoHz);

    const float k = 0.03f;
    const float phaseInc = 6.2831853f * rate / sr;

    for (int i = 0; i < n; ++i)
    {
        brownL = brownL * leak + noiseL.bipolar() * k;
        brownR = brownR * leak + noiseR.bipolar() * k;

        phase += phaseInc;
        if (phase > 6.2831853f) phase -= 6.2831853f;
        const float amp = 1.0f - depth * (0.5f - 0.5f * std::sin (phase));

        outL[i] = lp.processL (hp.processL (brownL)) * amp * 0.5f;
        outR[i] = lp.processR (hp.processR (brownR)) * amp * 0.5f;
    }

    mono.process (outL, outR, n);
}
}
