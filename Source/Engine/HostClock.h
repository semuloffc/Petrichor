#pragma once

#include <JuceHeader.h>
#include <cmath>

namespace petrichor
{
class HostClock
{
public:
    void reset()
    {
        bpm = 70.0;
        ppq = 0.0;
        playing = false;
        hasHost = false;
        lastBar = 0;
        barChanged = false;
    }

    void update (juce::AudioPlayHead* playhead, double seconds)
    {
        playing = false;
        hasHost = false;

        if (playhead != nullptr)
        {
            if (auto pos = playhead->getPosition())
            {
                hasHost = true;
                const double hostBpm = pos->getBpm().orFallback (70.0);
                if (hostBpm > 1.0 && hostBpm < 400.0)
                    bpm = hostBpm;

                if (pos->getIsPlaying())
                {
                    playing = true;
                    ppq = pos->getPpqPosition().orFallback (ppq);
                }
            }
        }

        if (!playing)
            ppq += (bpm / 60.0) * seconds;

        const long long b = barIndex();
        barChanged = (b != lastBar);
        lastBar = b;
    }

    double barFraction() const     { const double f = ppq / 4.0; return f - std::floor (f); }
    long long barIndex() const     { return (long long) std::floor (ppq / 4.0); }
    double beatFraction() const    { const double f = ppq; return f - std::floor (f); }
    double secondsPerBeat() const  { return 60.0 / bpm; }
    double secondsPerBar() const   { return 4.0 * secondsPerBeat(); }

    double bpm = 70.0;
    double ppq = 0.0;
    bool playing = false;
    bool hasHost = false;
    bool barChanged = false;

private:
    long long lastBar = 0;
};
}
