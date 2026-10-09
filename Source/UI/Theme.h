#pragma once

#include <JuceHeader.h>

namespace petrichor::theme
{
using juce::Colour;

inline const Colour bgTop       { 0xFFF3F8F6 };
inline const Colour bgMid       { 0xFFE8F0EE };
inline const Colour bgBottom    { 0xFFB3C7C2 };
inline const Colour well        { 0xFFDCE8E5 };
inline const Colour divider     { 0xFFC9D9D5 };

inline const Colour textPrimary   { 0xFF2B3A38 };
inline const Colour textSecondary { 0xFF5A6B68 };
inline const Colour textMuted     { 0xFF7F918D };
inline const Colour textInverse   { 0xFFF3F8F6 };

inline const Colour bloomHover   { 0xFFB9DBCD };
inline const Colour bloomPressed { 0xFF94C2B0 };

struct Accent
{
    Colour tint;
    Colour fill;
    Colour stroke;
    juce::String name;
};

inline const Accent dew    { Colour (0xFFE3F3F8), Colour (0xFFBFE3EE), Colour (0xFF7FBBD0), "DEW" };
inline const Accent bloom  { Colour (0xFFDCEDE6), Colour (0xFFA6CFBF), Colour (0xFF6FA793), "BLOOM" };
inline const Accent breeze { Colour (0xFFE4EAEC), Colour (0xFFBCC9CF), Colour (0xFF8FA3AB), "BREEZE" };
inline const Accent roots  { Colour (0xFFDDE5DF), Colour (0xFF5F7566), Colour (0xFF3F5246), "ROOTS" };
inline const Accent burst  { Colour (0xFFF8E4E7), Colour (0xFFE9A8B0), Colour (0xFFD98794), "BURST" };

inline const Accent& accentForEngine (int engineIndex)
{
    switch (engineIndex)
    {
        case 0: return dew;
        case 1: return bloom;
        case 2: return breeze;
        case 3: return roots;
        default: return burst;
    }
}
}
