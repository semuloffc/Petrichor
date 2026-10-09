#pragma once

#include <array>
#include <cmath>

namespace petrichor
{
inline const std::array<float, 2048>& sineTable()
{
    static const std::array<float, 2048> table = []()
    {
        std::array<float, 2048> a;
        for (int i = 0; i < 2048; ++i)
            a[i] = std::sin (2.0f * 3.14159265358979323846f * (float) i / 2048.0f);
        return a;
    }();
    return table;
}

inline float sineLookup (float phase)
{
    const auto& t = sineTable();
    const float x = phase * 2048.0f;
    int i = (int) x;
    const float frac = x - (float) i;
    i &= 2047;
    const int j = (i + 1) & 2047;
    return t[i] + (t[j] - t[i]) * frac;
}
}
