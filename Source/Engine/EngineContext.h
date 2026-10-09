#pragma once

#include <JuceHeader.h>
#include "ScaleQuantizer.h"
#include "HostClock.h"
#include "GrowthMapper.h"
#include "VisualizerBus.h"

namespace petrichor
{
struct MacroState
{
    GrowthState growth;
    float storm = 0.0f;
    float humid = 0.0f;
    float dewReleaseMul = 1.0f;
    float humanizeMul = 1.0f;
    float breezeExtraDb = 0.0f;
    float burstProbExtra = 0.0f;
    float extraTripletProb = 0.0f;
};

struct EngineContext
{
    double sampleRate = 48000.0;
    juce::AudioProcessorValueTreeState* params = nullptr;
    const ScaleQuantizer* scale = nullptr;
    const HostClock* clock = nullptr;
    VisualizerBus* viz = nullptr;

    float param (const juce::String& id) const { return params->getRawParameterValue (id)->load(); }
    bool paramBool (const juce::String& id) const { return param (id) >= 0.5f; }
};
}
