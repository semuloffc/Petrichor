#pragma once

#include <JuceHeader.h>
#include "EngineContext.h"
#include "Noise.h"
#include "Dsp.h"

namespace petrichor
{
class BurstEngine
{
public:
    void prepare (double sampleRate);
    void reset();
    void trigger();
    void process (float* outL, float* outR, int n, const EngineContext& ctx, const MacroState& macro);

private:
    void startHit();

    Biquad hp;
    OnePole tiltLp;
    Noise noise { 0x0badf00d };
    double sampleRate = 48000.0;
    float env = 0.0f;
    float decayCoef = 0.99f;
    float thumpEnv = 0.0f;
    float thumpPhase = 0.0f;
    float thumpFreq = 80.0f;
    float thumpFreqStep = 0.0f;
    float thumpDecay = 0.99f;
    bool thumpActive = false;
    bool pending = false;
    int burstBarCount = 0;
};
}
