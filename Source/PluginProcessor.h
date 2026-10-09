#pragma once

#include <JuceHeader.h>
#include <array>
#include "Parameters/ParameterLayout.h"
#include "Engine/EngineRack.h"
#include "Presets/PresetManager.h"

namespace petrichor
{
class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    std::array<int, 16> getDewPattern() const;
    std::array<int, 24> getChordA() const;
    std::array<int, 24> getChordB() const;
    int getChordACount() const;
    int getChordBCount() const;
    void setDewStep (int step, int value);
    void toggleChordNote (int slot, int semitone);

    void requestBurst();
    void loadFactoryPreset (int index);
    void loadUserPreset (const juce::String& name);
    void saveUserPreset (const juce::String& name, bool overwrite);
    void renameUserPreset (const juce::String& oldName, const juce::String& newName);
    void deleteUserPreset (const juce::String& name);
    juce::StringArray getUserPresetNames();
    juce::StringArray getFactoryPresetNames();

    juce::AudioProcessorValueTreeState apvts;
    VisualizerBus viz;
    PresetManager presets;

private:
    void ensurePatternTree();
    void syncPattern();
    void applyStateXml (const juce::String& xml);

    EngineRack rack;
    juce::String currentPresetName;
};
}
