#pragma once

#include <JuceHeader.h>
#include <cmath>

namespace petrichor
{
inline float dbToGain (float db) { return std::pow (10.0f, db * 0.05f); }
inline float gainToDb (float g)  { return 20.0f * std::log10 (g + 1.0e-9f); }
inline float centsToRatio (float cents) { return std::pow (2.0f, cents / 1200.0f); }
inline float clampf (float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

class OnePole
{
public:
    void reset (double sr) { fs = sr; a = 0.0; zL = zR = 0.0; }
    void setLowPass (float hz)
    {
        hz = clampf (hz, 1.0f, (float) (fs * 0.45));
        a = 1.0 - std::exp (-2.0 * juce::MathConstants<double>::pi * hz / fs);
    }
    void setHighPass (float hz)
    {
        hz = clampf (hz, 1.0f, (float) (fs * 0.45));
        a = std::exp (-2.0 * juce::MathConstants<double>::pi * hz / fs);
    }
    void processMono (float* data, int n)
    {
        double z = zL;
        for (int i = 0; i < n; ++i) { z += a * ((double) data[i] - z); data[i] = (float) z; }
        zL = z;
    }
    void processStereo (float* l, float* r, int n)
    {
        double zl = zL, zr = zR;
        for (int i = 0; i < n; ++i)
        {
            zl += a * ((double) l[i] - zl);
            zr += a * ((double) r[i] - zr);
            l[i] = (float) zl;
            r[i] = (float) zr;
        }
        zL = zl; zR = zr;
    }
    void processStereoHP (float* l, float* r, int n)
    {
        double zl = zL, zr = zR;
        for (int i = 0; i < n; ++i)
        {
            double hl = (double) l[i] - zl, hr = (double) r[i] - zr;
            zl += a * hl; zr += a * hr;
            l[i] = (float) hl; r[i] = (float) hr;
        }
        zL = zl; zR = zr;
    }
private:
    double fs = 48000.0, a = 0.0, zL = 0.0, zR = 0.0;
};

class Biquad
{
public:
    void reset (double sr) { fs = sr; b0 = 1.0; b1 = b2 = a1 = a2 = 0.0; z1L = z2L = z1R = z2R = 0.0; }

    void setLowPass (float f, float q)
    {
        double w = 2.0 * juce::MathConstants<double>::pi * f / fs;
        double cs = std::cos (w), sn = std::sin (w);
        double alpha = sn / (2.0 * q);
        double a0 = 1.0 + alpha;
        b0 = (float) ((1.0 - cs) * 0.5 / a0);
        b1 = (float) ((1.0 - cs) / a0);
        b2 = (float) ((1.0 - cs) * 0.5 / a0);
        a1 = (float) (-2.0 * cs / a0);
        a2 = (float) ((1.0 - alpha) / a0);
    }
    void setHighPass (float f, float q)
    {
        double w = 2.0 * juce::MathConstants<double>::pi * f / fs;
        double cs = std::cos (w), sn = std::sin (w);
        double alpha = sn / (2.0 * q);
        double a0 = 1.0 + alpha;
        b0 = (float) ((1.0 + cs) * 0.5 / a0);
        b1 = (float) (-(1.0 + cs) / a0);
        b2 = (float) ((1.0 + cs) * 0.5 / a0);
        a1 = (float) (-2.0 * cs / a0);
        a2 = (float) ((1.0 - alpha) / a0);
    }
    void setBandPass (float f, float q)
    {
        double w = 2.0 * juce::MathConstants<double>::pi * f / fs;
        double cs = std::cos (w), sn = std::sin (w);
        double alpha = sn / (2.0 * q);
        double a0 = 1.0 + alpha;
        b0 = (float) (alpha / a0);
        b1 = 0.0f;
        b2 = (float) (-alpha / a0);
        a1 = (float) (-2.0 * cs / a0);
        a2 = (float) ((1.0 - alpha) / a0);
    }
    void setPeak (float f, float q, float gainDb)
    {
        double w = 2.0 * juce::MathConstants<double>::pi * f / fs;
        double cs = std::cos (w), sn = std::sin (w);
        double alpha = sn / (2.0 * q);
        double A = std::pow (10.0, gainDb / 40.0);
        double a0 = 1.0 + alpha / A;
        b0 = (float) ((1.0 + alpha * A) / a0);
        b1 = (float) (-2.0 * cs / a0);
        b2 = (float) ((1.0 - alpha * A) / a0);
        a1 = (float) (-2.0 * cs / a0);
        a2 = (float) ((1.0 - alpha / A) / a0);
    }

    inline float processL (float x)
    {
        float y = b0 * x + z1L;
        z1L = b1 * x - a1 * y + z2L;
        z2L = b2 * x - a2 * y;
        return y;
    }
    inline float processR (float x)
    {
        float y = b0 * x + z1R;
        z1R = b1 * x - a1 * y + z2R;
        z2R = b2 * x - a2 * y;
        return y;
    }
    void processStereo (float* l, float* r, int n)
    {
        for (int i = 0; i < n; ++i) { l[i] = processL (l[i]); r[i] = processR (r[i]); }
    }
    void clear() { z1L = z2L = z1R = z2R = 0.0; }

private:
    double fs = 48000.0;
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
    float z1L = 0.0f, z2L = 0.0f, z1R = 0.0f, z2R = 0.0f;
};
}
