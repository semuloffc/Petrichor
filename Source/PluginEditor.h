#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/LookAndFeel.h"
#include "UI/GlassBackground.h"
#include "UI/EngineCard.h"
#include "UI/KnobComponent.h"
#include "UI/Visualizer.h"
#include "UI/ChordLane.h"
#include "UI/PresetBar.h"

namespace petrichor
{
class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    class GlassPanel : public juce::Component
    {
    public:
        void paint (juce::Graphics& g) override { GlassBackground::drawGlassCard (g, getLocalBounds().toFloat()); }
    };

    class Meter : public juce::Component, private juce::Timer
    {
    public:
        explicit Meter (VisualizerBus& b);
        ~Meter() override;
        void paint (juce::Graphics& g) override;
        void timerCallback() override;
    private:
        VisualizerBus& bus;
    };

    void buildTopBar();
    void buildDewCard();
    void buildBloomCard();
    void buildBreezeCard();
    void buildRootsCard();
    void buildBurstCard();
    void buildCentre();
    void buildSpaceCard();
    void refreshPattern();

    KnobComponent* makeKnob (juce::Component& parent, const juce::String& id,
                             const juce::String& label, const theme::Accent* accent,
                             juce::Rectangle<int> bounds);
    juce::ComboBox* makeCombo (juce::Component& parent, const juce::String& id,
                               const juce::StringArray& items, juce::Rectangle<int> bounds);

    PluginProcessor& proc;
    EucalyptusLookAndFeel lnf;
    juce::Component content;
    GlassBackground background;

    PresetBar* presetBar = nullptr;
    Visualizer* visualizer = nullptr;
    ChordLane* chordLane = nullptr;

    juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAttachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> followMidiAttachment;
};
}
