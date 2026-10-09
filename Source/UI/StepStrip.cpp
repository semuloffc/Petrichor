#include "StepStrip.h"

namespace petrichor
{
StepStrip::StepStrip (std::function<void (int, int)> onChanged_)
    : onChanged (std::move (onChanged_))
{
}

void StepStrip::setSteps (const std::array<int, 16>& s)
{
    steps = s;
    repaint();
}

void StepStrip::setCurrentStep (int s)
{
    if (currentStep != s) { currentStep = s; repaint(); }
}

int StepStrip::cellAt (int x, int y) const
{
    const float total = 16.0f * 22.0f + 15.0f * 8.0f;
    const float x0 = ((float) getWidth() - total) * 0.5f;
    const float y0 = ((float) getHeight() - 22.0f) * 0.5f;

    if (y < y0 - 2.0f || y > y0 + 24.0f) return -1;
    const float rel = (float) x - x0;
    if (rel < 0.0f) return -1;
    const int idx = (int) (rel / 30.0f);
    if (idx >= 16) return -1;
    const float cellX = x0 + idx * 30.0f;
    if (rel > cellX - x0 + 22.0f) return -1;
    return idx;
}

void StepStrip::paint (juce::Graphics& g)
{
    const float total = 16.0f * 22.0f + 15.0f * 8.0f;
    const float x0 = ((float) getWidth() - total) * 0.5f;
    const float y0 = ((float) getHeight() - 22.0f) * 0.5f;

    g.setColour (theme::well);
    g.fillRoundedRectangle (x0 - 8.0f, y0 - 6.0f, total + 16.0f, 34.0f, 10.0f);

    for (int i = 0; i < 16; ++i)
    {
        const juce::Rectangle<float> r (x0 + i * 30.0f, y0, 22.0f, 22.0f);
        const int v = steps[(size_t) i];
        if (v == 1) g.setColour (theme::dew.fill);
        else if (v == 2) g.setColour (theme::dew.stroke);
        else g.setColour (theme::divider);
        g.fillRoundedRectangle (r, 4.0f);

        if (i == currentStep)
        {
            g.setColour (theme::textPrimary);
            g.drawRoundedRectangle (r.reduced (0.5f), 4.0f, 2.0f);
        }
    }
}

void StepStrip::mouseDown (const juce::MouseEvent& e)
{
    const int idx = cellAt (e.getPosition().getX(), e.getPosition().getY());
    if (idx < 0) return;
    const int nv = (steps[(size_t) idx] + 1) % 3;
    steps[(size_t) idx] = nv;
    paintValue = nv;
    lastPainted = idx;
    if (onChanged) onChanged (idx, nv);
    repaint();
}

void StepStrip::mouseDrag (const juce::MouseEvent& e)
{
    const int idx = cellAt (e.getPosition().getX(), e.getPosition().getY());
    if (idx < 0 || idx == lastPainted) return;
    steps[(size_t) idx] = paintValue;
    lastPainted = idx;
    if (onChanged) onChanged (idx, paintValue);
    repaint();
}
}
