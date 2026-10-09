#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>
#include <functional>
#include "../Engine/VisualizerBus.h"
#include "Theme.h"
#include "StepStrip.h"

namespace petrichor
{
class Visualizer : public juce::Component, private juce::Timer
{
public:
    Visualizer (VisualizerBus& bus, std::function<void (int, int)> onStepChanged);
    ~Visualizer() override;

    void setPattern (const std::array<int, 16>& steps);
    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    struct Ring { double birth = 0.0; float pan = 0.0f; };
    struct Flash { double birth = 0.0; float seed = 0.0f; };

    VisualizerBus& bus;
    StepStrip stepStrip;
    std::vector<Ring> rings;
    std::vector<Flash> flashes;
    float flashSeed = 0.0f;
};
}
