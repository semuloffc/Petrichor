#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>
#include "EngineContext.h"
#include "Noise.h"

namespace petrichor
{
class BloomEngine
{
public:
    void prepare (double sampleRate);
    void reset();
    void setChords (const std::array<int, 24>& a, int na, const std::array<int, 24>& b, int nb);
    void process (float* outL, float* outR, int n, const EngineContext& ctx, const MacroState& macro);

private:
    static constexpr int kNotes = 6;
    static constexpr int kCopies = 2;
    static constexpr int kPartials = 16;

    struct Note
    {
        float baseFreq = 0.0f;
        float drift = 0.0f;
        bool active = false;
    };

    struct Delay
    {
        std::vector<float> d;
        int size = 0;
        int w = 0;
        void prepare (int n) { d.assign ((size_t) n, 0.0f); size = n; w = 0; }
        void write (float x) { d[(size_t) w] = x; w = (w + 1) % size; }
        float read (float offset)
        {
            float pos = (float) w - offset;
            if (pos < 0.0f) pos += (float) size;
            const int i = (int) pos;
            const float frac = pos - (float) i;
            const int j = (i + 1) % size;
            return d[(size_t) i] + (d[(size_t) j] - d[(size_t) i]) * frac;
        }
    };

    void retune (const EngineContext& ctx);

    std::array<Note, kNotes> notes;
    std::array<float, kNotes * kCopies * kPartials> phases;
    std::array<float, kPartials> partialAmp;
    std::array<int, 24> chordA;
    std::array<int, 24> chordB;
    int countA = 6;
    int countB = 5;

    Noise noise;
    Delay delayL, delayR;
    juce::SmoothedValue<float> fadeGain;

    double sampleRate = 48000.0;
    float lfoPhase = 0.0f;
    float baseDelay = 0.0f;
    float depthSamples = 0.0f;

    int slot = 0;
    int barCount = 0;
    int lastRoot = -1;
    int lastScale = -1;
    float lastBright = -1.0f;
    float lastAttack = -1.0f;
    bool retunePending = true;
};
}
