#pragma once

#include <JuceHeader.h>

#if PETRICHOR_HAS_EMBEDDED_FONTS
#include "BinaryData.h"
#endif

namespace petrichor
{
inline juce::Font interRegular (float height)
{
#if PETRICHOR_HAS_EMBEDDED_FONTS
    static juce::Typeface::Ptr tf = juce::Typeface::createSystemTypefaceFor (BinaryData::Inter_Regular_ttf, BinaryData::Inter_Regular_ttfSize);
    if (tf != nullptr)
        return juce::Font (tf).withHeight (height);
#endif
    return juce::Font (juce::Font::getDefaultSansSerifFontName(), height, juce::Font::plain);
}

inline juce::Font interMedium (float height)
{
#if PETRICHOR_HAS_EMBEDDED_FONTS
    static juce::Typeface::Ptr tf = juce::Typeface::createSystemTypefaceFor (BinaryData::Inter_Medium_ttf, BinaryData::Inter_Medium_ttfSize);
    if (tf != nullptr)
        return juce::Font (tf).withHeight (height);
#endif
    return juce::Font (juce::Font::getDefaultSansSerifFontName(), height, juce::Font::plain);
}
}
