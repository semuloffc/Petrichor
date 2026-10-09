#pragma once

#include <JuceHeader.h>
#include <vector>
#include "FactoryPresets.h"

namespace petrichor
{
class PresetManager
{
public:
    explicit PresetManager (juce::AudioProcessorValueTreeState& apvts);

    juce::StringArray getFactoryNames() const;
    juce::StringArray getUserNames();
    juce::String getFactoryPreset (int index) const;

    bool loadUserPreset (const juce::String& name, juce::String& outXml);
    bool saveUserPreset (const juce::String& name, const juce::String& xml, bool overwrite);
    bool renameUserPreset (const juce::String& oldName, const juce::String& newName);
    bool deleteUserPreset (const juce::String& name);

    juce::File getPresetFolder() const;

private:
    juce::String buildDefaultXml (const std::vector<std::pair<juce::String, float>>& overrides) const;

    juce::AudioProcessorValueTreeState& apvts;
    std::vector<FactoryPreset> factory;
    juce::File folder;
};
}
