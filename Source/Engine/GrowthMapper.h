#pragma once

#include <cmath>

namespace petrichor
{
struct GrowthState
{
    float dewGain = 1.0f;
    int   bloomVoices = 6;
    float bloomGainDb = 0.0f;
    float breezeGain = 1.0f;
    float rootsGain = 1.0f;
    bool  burstArmed = true;
};

class GrowthMapper
{
public:
    static GrowthState map (float growth01)
    {
        GrowthState s;
        growth01 = growth01 < 0.0f ? 0.0f : (growth01 > 1.0f ? 1.0f : growth01);

        if (growth01 <= 0.33f)
        {
            s.dewGain = 1.0f;
            s.bloomVoices = 1;
            s.bloomGainDb = -24.0f;
            s.breezeGain = 0.0f;
            s.rootsGain = 0.0f;
            s.burstArmed = false;
        }
        else if (growth01 <= 0.66f)
        {
            const float r = (growth01 - 0.33f) / 0.33f;
            s.dewGain = 1.0f;
            s.bloomVoices = 1 + (int) std::lround (5.0f * r);
            s.bloomGainDb = -24.0f + 24.0f * r;
            s.breezeGain = r;
            s.rootsGain = 0.0f;
            s.burstArmed = false;
        }
        else
        {
            const float r = (growth01 - 0.66f) / 0.34f;
            s.dewGain = 1.0f;
            s.bloomVoices = 6;
            s.bloomGainDb = 0.0f;
            s.breezeGain = 1.0f;
            s.rootsGain = r;
            s.burstArmed = true;
        }
        return s;
    }
};
}
