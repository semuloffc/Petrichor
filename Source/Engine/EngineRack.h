#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>
#include <atomic>
#include "EngineContext.h"
#include "ScaleQuantizer.h"
#include "HostClock.h"
#include "GrowthMapper.h"
#include "DewEngine.h"
#include "BloomEngine.h"
#include "BreezeEngine.h"
#include "RootsEngine.h"
#include "BurstEngine.h"
#include "Greenhouse.h"
#include "MistFilter.h"
#include "PocketDucker.h"

namespace petrichor
{
struct EngineState
{
    std::array<int, 16> dew {};
    std::array<int, 24> chordA {};
    std::array<int, 24> chordB {};
    int countA = 6;
    int countB = 5;
};

class EngineRack
{
public:
    EngineRack (juce::AudioProcessorValueTreeState& apvts, VisualizerBus& viz);

    void prepare (double sampleRate, int blockSize);
    void reset();
    void publishState (const EngineState& s);
    void requestBurst();

    void process (float* mainL, float* mainR,
                  float* const* auxL, float* const* auxR,
                  const float* sideL, const float* sideR,
                  const juce::MidiBuffer& midi, juce::AudioPlayHead* playhead, int n);

private:
    MacroState computeMacro() const;
    void handleMidi (const juce::MidiBuffer& midi, const MacroState& macro);
    void inferRootScale();

    juce::AudioProcessorValueTreeState& params;
    VisualizerBus& viz;
    ScaleQuantizer quantizer;
    HostClock clock;

    DewEngine dew;
    BloomEngine bloom;
    BreezeEngine breeze;
    RootsEngine roots;
    BurstEngine burst;
    Greenhouse greenhouse;
    MistFilter mist;
    PocketDucker pocket;

    std::vector<float> dewL, dewR, bloomL, bloomR, breezeL, breezeR, rootsL, rootsR, burstL, burstR;
    std::vector<float> sendL, sendR, wetL, wetR, dryL, dryR, duckL, duckR, mainL, mainR, zero;

    std::array<juce::SmoothedValue<float>, 5> auxGain;
    std::array<juce::SmoothedValue<float>, 5> mainGain;
    juce::SmoothedValue<float> masterGain;
    juce::SmoothedValue<float> ghMix;

    std::array<EngineState, 2> stateBuf;
    std::atomic<int> stateIdx { 0 };
    std::atomic<int> stateVersion { 0 };
    int lastStateVersion = -1;
    std::atomic<bool> burstRequested { false };

    std::array<bool, 128> held {};
    float prevGrowth = -1.0f;
    double sampleRate = 48000.0;
    float mistSm = 6000.0f;
    float sizeSm = 2.5f;
    float dampSm = 5000.0f;
    float predelaySm = 10.0f;
    float lowcutSm = 120.0f;
    float pocketSm = 0.0f;
    bool spaceInit = false;
};
}
