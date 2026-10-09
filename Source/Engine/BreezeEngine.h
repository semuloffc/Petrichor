#pragma once

#include <JuceHeader.h>
#include "EngineContext.h"
#include "Noise.h"
#include "Dsp.h"

namespace petrichor
{
class BreezeEngine
{
public:
    void prepare (double sampleRate);
    void reset();
    void process (float* outL, float* outR, int n, const EngineContext& ctx, const MacroState& macro);

private:
    Biquad stage[2];
    Noise noiseL { 0x1234abcd };
    Noise noiseR { 0xdeadbeef };
    double sampleRate = 48000.0;
    float gust = 0.0f;
    int cycleBar = 0;
};
}
