#pragma once

#include <JuceHeader.h>
#include <array>
#include "EngineContext.h"
#include "ModalBank.h"
#include "Noise.h"

namespace petrichor
{
class DewEngine
{
public:
    void prepare (double sampleRate);
    void reset();
    void setPattern (const std::array<int, 16>& steps);
    void process (float* outL, float* outR, int n, const EngineContext& ctx, const MacroState& macro);

private:
    struct Voice
    {
        bool active = false;
        bool modalOn = false;
        float freq = 0.0f;
        float phase = 0.0f;
        float env = 0.0f;
        float attackStep = 0.0f;
        float releaseStep = 0.0f;
        int stage = 0;
        int delay = 0;
        int gateCount = 0;
        int gateSamples = 0;
        float panL = 0.70710678f;
        float panR = 0.70710678f;
        float modalMix = 0.0f;
        ModalBank modal;
    };

    void triggerVoice (int noteValue, int root, int octave, float gateMs, float releaseMs,
                       float purity, float material, float humanizeMs, float spread,
                       const EngineContext& ctx, const MacroState& macro);

    std::array<Voice, 8> voices;
    std::array<int, 16> pattern {};
    Noise noise;
    double sampleRate = 48000.0;
    int currentStep = -1;
    int samplesToStep = 0;
};
}
