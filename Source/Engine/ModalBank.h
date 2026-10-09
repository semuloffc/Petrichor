#pragma once

#include <JuceHeader.h>
#include <array>
#include "Dsp.h"

namespace petrichor
{
class ModalBank
{
public:
    void setSampleRate (double sr) { fs = sr; }

    void setModes (float material, float fundamental)
    {
        static constexpr std::array<float, 4> glass { 1.0f, 2.32f, 4.25f, 6.63f };
        static constexpr std::array<float, 4> wood  { 1.0f, 4.0f, 10.0f, 13.0f };
        static constexpr std::array<float, 4> metal { 1.0f, 2.756f, 5.404f, 8.933f };

        const float glassTau = 0.12f, woodTau = 0.028f, metalTau = 0.7f;

        std::array<float, 4> ratios;
        float tau;
        if (material <= 50.0f)
        {
            const float t = material / 50.0f;
            for (int i = 0; i < 4; ++i) ratios[i] = glass[i] + (wood[i] - glass[i]) * t;
            tau = glassTau + (woodTau - glassTau) * t;
        }
        else
        {
            const float t = (material - 50.0f) / 50.0f;
            for (int i = 0; i < 4; ++i) ratios[i] = wood[i] + (metal[i] - wood[i]) * t;
            tau = woodTau + (metalTau - woodTau) * t;
        }

        const double nyq = fs * 0.49;
        for (int i = 0; i < 4; ++i)
        {
            const double f = std::min ((double) (fundamental * ratios[i]), nyq);
            const double theta = 2.0 * juce::MathConstants<double>::pi * f / fs;
            const double R = std::exp (-1.0 / (tau * fs));
            a1[i] = (float) (2.0 * R * std::cos (theta));
            a2[i] = (float) (-R * R);
        }
    }

    void excite (float amp)
    {
        for (int i = 0; i < 4; ++i)
            y1[i] += amp;
    }

    float process()
    {
        float out = 0.0f;
        for (int i = 0; i < 4; ++i)
        {
            const float y = a1[i] * y1[i] + a2[i] * y2[i];
            y2[i] = y1[i];
            y1[i] = y;
            out += y;
        }
        return out * 0.25f;
    }

    void clear()
    {
        for (int i = 0; i < 4; ++i) { y1[i] = 0.0f; y2[i] = 0.0f; a1[i] = 0.0f; a2[i] = 0.0f; }
    }

private:
    double fs = 48000.0;
    float a1[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float a2[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float y1[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float y2[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};
}
