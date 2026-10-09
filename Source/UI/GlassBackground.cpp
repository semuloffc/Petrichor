#include "GlassBackground.h"

namespace petrichor
{
GlassBackground::GlassBackground()
{
    blobs[0] = { 0.18f, 0.22f, 200.0f, 0.0f, theme::bloom.fill };
    blobs[1] = { 0.62f, 0.14f, 160.0f, 1.3f, theme::dew.fill };
    blobs[2] = { 0.82f, 0.55f, 220.0f, 2.6f, theme::burst.tint };
    blobs[3] = { 0.35f, 0.72f, 170.0f, 4.0f, theme::breeze.fill };
    blobs[4] = { 0.08f, 0.55f, 140.0f, 5.2f, theme::dew.tint };

    startTimerHz (60);
}

GlassBackground::~GlassBackground() { stopTimer(); }

void GlassBackground::timerCallback()
{
    time += 1.0 / 60.0;
    repaint();
}

void GlassBackground::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float sx = bounds.getWidth() / 1280.0f;

    juce::ColourGradient grad (theme::bgTop, 0.0f, 0.0f,
                               theme::bgBottom, 0.0f, bounds.getHeight(), false);
    grad.addColour (0.5, theme::bgMid);
    g.setGradientFill (grad);
    g.fillAll();

    for (const auto& b : blobs)
    {
        const float cx = b.x * bounds.getWidth() + std::sin ((float) time * 0.13f + b.phase) * 24.0f * sx;
        const float cy = b.y * bounds.getHeight() + std::cos ((float) time * 0.11f + b.phase) * 18.0f * sx;
        const float r = b.radius * sx;

        juce::ColourGradient bg (b.colour.withAlpha (0.0f), cx, cy,
                                 b.colour.withAlpha (0.35f), cx + r, cy + r, true);
        g.setGradientFill (bg);
        g.fillEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f);
    }
}

void GlassBackground::drawGlassCard (juce::Graphics& g, juce::Rectangle<float> r)
{
    juce::Path p;
    p.addRoundedRectangle (r, 16.0f);
    juce::DropShadow (theme::textPrimary.withAlpha (0.08f), 24, juce::Point<int> (0, 8)).drawForPath (g, p);

    g.setColour (theme::bgTop.withAlpha (0.55f));
    g.fillRoundedRectangle (r, 16.0f);

    g.setColour (theme::divider.withAlpha (0.6f));
    g.drawRoundedRectangle (r.reduced (0.5f), 16.0f, 1.0f);
}
}
