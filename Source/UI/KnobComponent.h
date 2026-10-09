#pragma once

#include <JuceHeader.h>
#include <memory>
#include "Theme.h"

namespace petrichor
{
class KnobComponent : public juce::Component, private juce::Timer
{
public:
    KnobComponent (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
                   const juce::String& label, const theme::Accent* accent);
    ~KnobComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;
    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    bool keyPressed (const juce::KeyPress& key) override;
    void focusGained (juce::Component::FocusChangeType) override { repaint(); }
    void focusLost (juce::Component::FocusChangeType) override { repaint(); }

    void timerCallback() override;

private:
    juce::RangedAudioParameter* param = nullptr;
    std::unique_ptr<juce::ParameterAttachment> attachment;
    juce::String label;
    const theme::Accent* accent = nullptr;

    float targetValue = 0.0f;
    float displayedValue = 0.0f;
    float dragStartValue = 0.0f;
    int dragStartY = 0;
};
}
