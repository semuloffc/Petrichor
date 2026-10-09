#pragma once

#include <JuceHeader.h>
#include <array>
#include <functional>
#include "Theme.h"

namespace petrichor
{
class ChordLane : public juce::Component
{
public:
    explicit ChordLane (std::function<void (int, int)> onToggle);

    void setChordA (const std::array<int, 24>& notes, int count);
    void setChordB (const std::array<int, 24>& notes, int count);
    int getSlot() const { return slot; }

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;

private:
    bool hasNote (const std::array<int, 24>& notes, int count, int semitone) const;

    int slot = 0;
    std::array<int, 24> chordA {};
    std::array<int, 24> chordB {};
    int countA = 0;
    int countB = 0;
    std::function<void (int, int)> onToggle;
};
}
