#pragma once

#include <JuceHeader.h>
#include <vector>

namespace petrichor
{
struct FactoryPreset
{
    juce::String name;
    std::vector<std::pair<juce::String, float>> overrides;
};

std::vector<FactoryPreset> createFactoryPresets();
const char* defaultDewPattern();
const char* defaultChordA();
const char* defaultChordB();
}
