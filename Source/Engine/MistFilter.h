#pragma once

#include "Dsp.h"

namespace petrichor
{
class MistFilter
{
public:
    void setSampleRate (double sr)
    {
        for (int i = 0; i < 4; ++i) poles[i].reset (sr);
    }
    void setCutoff (float hz)
    {
        for (int i = 0; i < 4; ++i) poles[i].setLowPass (hz);
    }
    void process (float* l, float* r, int n)
    {
        for (int i = 0; i < 4; ++i) poles[i].processStereo (l, r, n);
    }
    void clear()
    {
        for (int i = 0; i < 4; ++i) poles[i].reset (48000.0);
    }
private:
    OnePole poles[4];
};
}
