#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>

namespace petrichor
{
enum class VizType : int { DewTrigger, BurstFlash };

struct VizEvent
{
    VizType type = VizType::DewTrigger;
    float pan = 0.0f;
};

class VisualizerBus
{
public:
    static constexpr int kEvents = 2048;

    void reset()
    {
        fifo.reset();
        for (auto& l : levels) l.store (0.0f);
    }

    bool push (const VizEvent& e)
    {
        int s1, sz1, s2, sz2;
        fifo.prepareToWrite (1, s1, sz1, s2, sz2);
        if (sz1 + sz2 == 0) return false;
        events[sz1 > 0 ? s1 : s2] = e;
        fifo.finishedWrite (1);
        return true;
    }

    int read (VizEvent* out, int max)
    {
        int s1, sz1, s2, sz2;
        fifo.prepareToRead (max, s1, sz1, s2, sz2);
        const int n = sz1 + sz2;
        for (int i = 0; i < sz1; ++i) out[i] = events[s1 + i];
        for (int i = 0; i < sz2; ++i) out[sz1 + i] = events[s2 + i];
        fifo.finishedRead (n);
        return n;
    }

    std::array<std::atomic<float>, 8> levels;
    std::atomic<int> dewStep { 0 };

private:
    juce::AbstractFifo fifo { kEvents };
    std::array<VizEvent, kEvents> events;
};
}
