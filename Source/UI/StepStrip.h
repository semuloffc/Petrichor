#pragma once

#include <JuceHeader.h>
#include <array>
#include <functional>
#include "Theme.h"

namespace petrichor
{
class StepStrip : public juce::Component
{
public:
    explicit StepStrip (std::function<void (int, int)> onChanged);

    void setSteps (const std::array<int, 16>& steps);
    void setCurrentStep (int step);
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

private:
    int cellAt (int x, int y) const;

    std::array<int, 16> steps {};
    int currentStep = -1;
    int paintValue = -1;
    int lastPainted = -1;
    std::function<void (int, int)> onChanged;
};
}
