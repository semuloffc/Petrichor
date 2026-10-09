#pragma once

#include <JuceHeader.h>
#include "EngineContext.h"
#include "Noise.h"
#include "Dsp.h"
#include "MonoSafe.h"

namespace petrichor
{
class RootsEngine
{
public:
    void prepare (double sampleRate);
    void reset();
    void process (float* outL, float* outR, int n, const EngineContext& ctx, const MacroState& macro);

private:
    Biquad hp, lp;
    MonoSafe mono;
    Noise noiseL { 0x31415926 };
    Noise noiseR { 0x27182818 };
    double sampleRate = 48000.0;
    float brownL = 0.0f;
    float brownR = 0.0f;
    float phase = 0.0f;
    float leak = 1.0f;
};
}
