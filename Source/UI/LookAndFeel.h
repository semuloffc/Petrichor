#pragma once

#include <JuceHeader.h>

namespace petrichor
{
class EucalyptusLookAndFeel : public juce::LookAndFeel_V4
{
public:
    EucalyptusLookAndFeel();

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override;

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override;
};
}
