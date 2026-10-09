#include "ChordLane.h"
#include "Fonts.h"

namespace petrichor
{
ChordLane::ChordLane (std::function<void (int, int)> onToggle_)
    : onToggle (std::move (onToggle_))
{
}

void ChordLane::setChordA (const std::array<int, 24>& notes, int count)
{
    chordA = notes;
    countA = count;
    repaint();
}

void ChordLane::setChordB (const std::array<int, 24>& notes, int count)
{
    chordB = notes;
    countB = count;
    repaint();
}

bool ChordLane::hasNote (const std::array<int, 24>& notes, int count, int semitone) const
{
    for (int i = 0; i < count; ++i)
        if (notes[(size_t) i] == semitone) return true;
    return false;
}

void ChordLane::paint (juce::Graphics& g)
{
    const float h = (float) getHeight();
    const float cell = h - 4.0f;
    const float y = 2.0f;

    auto drawToggle = [&] (float x, const char* txt, bool active)
    {
        const juce::Rectangle<float> r (x, y, cell, cell);
        g.setColour (active ? theme::bloom.fill : theme::well);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (theme::textPrimary);
        g.setFont (interMedium (12.0f));
        g.drawText (txt, r, juce::Justification::centred, false);
    };

    drawToggle (0.0f, "A", slot == 0);
    drawToggle (cell + 4.0f, "B", slot == 1);

    const float startX = cell * 2.0f + 12.0f;
    const float avail = (float) getWidth() - startX - 4.0f;
    const int n = 18;
    const float gap = 4.0f;
    const float cw = (avail - gap * (n - 1)) / n;

    const auto& activeNotes = (slot == 0) ? chordA : chordB;
    const int activeCount = (slot == 0) ? countA : countB;

    for (int i = 0; i < n; ++i)
    {
        const juce::Rectangle<float> r (startX + i * (cw + gap), y, cw, cell);
        const bool on = hasNote (activeNotes, activeCount, i);
        g.setColour (on ? theme::bloom.fill : theme::well);
        g.fillRoundedRectangle (r, 6.0f);
    }
}

void ChordLane::mouseDown (const juce::MouseEvent& e)
{
    const float h = (float) getHeight();
    const float cell = h - 4.0f;
    const int mx = e.getPosition().getX();

    if (mx < cell + 2.0f)
    {
        slot = 0;
        repaint();
        return;
    }
    if (mx < cell * 2.0f + 4.0f)
    {
        slot = 1;
        repaint();
        return;
    }

    const float startX = cell * 2.0f + 12.0f;
    const float avail = (float) getWidth() - startX - 4.0f;
    const int n = 18;
    const float gap = 4.0f;
    const float cw = (avail - gap * (n - 1)) / n;

    const float rel = (float) mx - startX;
    if (rel < 0.0f) return;
    const int idx = (int) (rel / (cw + gap));
    if (idx >= n) return;
    if (onToggle) onToggle (slot, idx);
}
}
