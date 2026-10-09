#include "BreezeEngine.h"
#include "../Parameters/ParameterIDs.h"

namespace petrichor
{
void BreezeEngine::prepare (double sr)
{
    sampleRate = sr;
    for (auto& s : stage) s.reset (sr);
    reset();
}

void BreezeEngine::reset()
{
    for (auto& s : stage) s.clear();
    gust = 0.0f;
    cycleBar = 0;
}

void BreezeEngine::process (float* outL, float* outR, int n, const EngineContext& ctx, const MacroState& macro)
{
    const float sr = (float) sampleRate;
    const float center = ctx.param (IDs::breeze_center) * 1000.0f;
    const float widthOct = ctx.param (IDs::breeze_width);
    const float swell = ctx.param (IDs::breeze_swell);
    const float depth = ctx.param (IDs::breeze_depth) + macro.breezeExtraDb;
    const float cut = ctx.param (IDs::breeze_cut);

    const float q = 1.0f / (std::pow (2.0f, widthOct * 0.5f) - std::pow (2.0f, -widthOct * 0.5f));
    for (auto& s : stage) s.setBandPass (center, q);

    const float floorGain = std::pow (10.0f, -depth * 0.05f);
    const float cutGain = std::pow (10.0f, -cut * 0.05f);

    if (ctx.clock->barChanged)
    {
        ++cycleBar;
        const int swellInt = (int) std::max (1.0f, swell);
        if (cycleBar >= swellInt)
        {
            cycleBar = 0;
            gust *= cutGain;
        }
    }
    if (gust < floorGain) gust = floorGain;

    const double swellSec = ctx.clock->secondsPerBar() * (double) swell;
    const double tau = juce::jmax (0.1, swellSec / 3.0);
    const float rise = 1.0f - std::exp ((float) (-1.0 / (tau * sr)));

    for (int i = 0; i < n; ++i)
    {
        const float nL = noiseL.bipolar();
        const float nR = nL * 0.8f + noiseR.bipolar() * 0.2f;

        float bL = nL, bR = nR;
        for (auto& s : stage) { bL = s.processL (bL); bR = s.processR (bR); }

        gust += rise * (1.0f - gust);
        outL[i] = bL * gust * 0.3f;
        outR[i] = bR * gust * 0.3f;
    }
}
}
