#pragma once

#include <JuceHeader.h>
#include <functional>

namespace petrichor
{
class PluginProcessor;

class PresetBar : public juce::Component
{
public:
    PresetBar (PluginProcessor& proc, std::function<void()> onPresetLoaded);

    void paint (juce::Graphics& g) override;
    void resized() override;
    void setPresetName (const juce::String& name);

private:
    juce::StringArray combinedNames();
    void loadByName (const juce::String& name);
    void step (int dir);
    void showSaveDialog();
    void showMenu();

    PluginProcessor& proc;
    std::function<void()> onPresetLoaded;
    juce::TextButton prev { "<" }, next { ">" }, save { "Save" }, menu { "\xe2\x8b\xaf" };
    juce::Label nameLabel;
    juce::String currentName = "Canopy at Dawn";
};
}
