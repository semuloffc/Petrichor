#pragma once

#include <JuceHeader.h>
#include "Theme.h"

namespace petrichor
{
class GlassBackground : public juce::Component, private juce::Timer
{
public:
    GlassBackground();
    ~GlassBackground() override;

    void paint (juce::Graphics& g) override;

    static void drawGlassCard (juce::Graphics& g, juce::Rectangle<float> bounds);

private:
    void timerCallback() override;

    struct Blob
    {
        float x = 0.0f, y = 0.0f, radius = 0.0f;
        float phase = 0.0f;
        juce::Colour colour;
    };

    std::array<Blob, 5> blobs;
    double time = 0.0;
};
}
