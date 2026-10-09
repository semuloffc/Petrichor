#pragma once

#include <JuceHeader.h>
#include "Theme.h"

namespace petrichor
{
class EngineCard : public juce::Component
{
public:
    EngineCard (juce::AudioProcessorValueTreeState& apvts, const theme::Accent& accent,
                const juce::String& name, const juce::String& enableId, const juce::String& soloId);

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;

    juce::Rectangle<int> contentArea() const;

private:
    juce::Rectangle<int> enableRect() const;
    juce::Rectangle<int> soloRect() const;
    void drawPill (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& text, bool on) const;

    juce::AudioProcessorValueTreeState& apvts;
    theme::Accent accent;
    juce::String name;
    juce::String enableId;
    juce::String soloId;
};
}
