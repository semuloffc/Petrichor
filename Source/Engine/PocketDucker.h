#pragma once

#include <JuceHeader.h>
#include "Dsp.h"

namespace petrichor
{
class PocketDucker
{
public:
    void setSampleRate (double sr)
    {
        fs = sr;
        lp.reset (sr);
        hp.reset (sr);
        peak.reset (sr);
        lp.setLowPass (4000.0f, 0.70710678f);
        hp.setHighPass (1500.0f, 0.70710678f);
        peak.setPeak (2300.0f, 1.2f, 0.0f);
        amount.reset (sr, 0.02);
        amount.setCurrentAndTargetValue (0.0f);
        env = 0.0f;
        duckDb = 0.0f;
        attack = 1.0f - std::exp (-1.0f / (0.005f * (float) sr));
        release = 1.0f - std::exp (-1.0f / (0.150f * (float) sr));
    }
    void setAmount (float amt01) { amount.setTargetValue (amt01); }
    void clear()
    {
        lp.clear(); hp.clear(); peak.clear(); env = 0.0f; duckDb = 0.0f;
    }
    void process (float* sideL, float* sideR, float* tgtL, float* tgtR, int n)
    {
        for (int i = 0; i < n; ++i)
        {
            float s = 0.5f * (sideL[i] + sideR[i]);
            s = hp.processL (lp.processL (s));
            const float d = std::fabs (s);
            env = d > env ? env + attack * (d - env) : env + release * (d - env);
        }

        const float amt = amount.getNextValue();
        const float target = -9.0f * amt * clampf (env * 3.0f, 0.0f, 1.0f);
        duckDb += (target - duckDb) * 0.05f;

        peak.setPeak (2300.0f, 1.2f, duckDb);
        peak.processStereo (tgtL, tgtR, n);
    }
private:
    double fs = 48000.0;
    Biquad lp, hp, peak;
    juce::SmoothedValue<float> amount;
    float env = 0.0f;
    float duckDb = 0.0f;
    float attack = 0.0f;
    float release = 0.0f;
};
}
