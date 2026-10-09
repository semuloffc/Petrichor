#pragma once

#include <JuceHeader.h>

namespace petrichor
{
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
juce::StringArray scaleNames();
juce::StringArray rootNames();
}
