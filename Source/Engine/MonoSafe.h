#pragma once

#include "Dsp.h"

namespace petrichor
{
class MonoSafe
{
public:
    void setSampleRate (double sr)
    {
        for (auto& b : lp) b.reset (sr);
        for (auto& b : hp) b.reset (sr);
        active = false;
    }
    void setCrossover (float hz)
    {
        active = hz > 0.5f;
        if (!active) return;
        const float q = 0.70710678f;
        for (auto& b : lp) b.setLowPass (hz, q);
        for (auto& b : hp) b.setHighPass (hz, q);
    }
    void process (float* l, float* r, int n)
    {
        if (!active) return;
        for (int i = 0; i < n; ++i)
        {
            const float il = l[i], ir = r[i];
            float ll = lp[0].processL (il); ll = lp[1].processL (ll);
            float lr = lp[0].processR (ir); lr = lp[1].processR (lr);
            float hl = hp[0].processL (il); hl = hp[1].processL (hl);
            float hr = hp[0].processR (ir); hr = hp[1].processR (hr);
            const float mono = 0.5f * (ll + lr);
            l[i] = mono + hl;
            r[i] = mono + hr;
        }
    }
    void clear()
    {
        for (auto& b : lp) b.clear();
        for (auto& b : hp) b.clear();
    }
private:
    Biquad lp[2], hp[2];
    bool active = false;
};
}
