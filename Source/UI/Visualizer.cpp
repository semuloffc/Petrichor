#include "Visualizer.h"
#include "GlassBackground.h"

namespace petrichor
{
Visualizer::Visualizer (VisualizerBus& bus_, std::function<void (int, int)> onStepChanged)
    : bus (bus_), stepStrip (std::move (onStepChanged))
{
    addAndMakeVisible (stepStrip);
    startTimerHz (60);
}

Visualizer::~Visualizer() { stopTimer(); }

void Visualizer::setPattern (const std::array<int, 16>& steps)
{
    stepStrip.setSteps (steps);
}

void Visualizer::resized()
{
    const float w = (float) getWidth();
    const float stripW = 472.0f;
    const float x = (w - stripW) * 0.5f;
    stepStrip.setBounds ((int) x, getHeight() - 46, (int) stripW, 30);
}

void Visualizer::timerCallback()
{
    stepStrip.setCurrentStep (bus.dewStep.load());
    repaint();
}

void Visualizer::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const double t = juce::Time::getMillisecondCounterHiRes() * 0.001;

    GlassBackground::drawGlassCard (g, bounds);

    VizEvent ev[64];
    const int evCount = bus.read (ev, 64);
    for (int i = 0; i < evCount; ++i)
    {
        if (ev[i].type == VizType::DewTrigger)
            rings.push_back ({ t, ev[i].pan });
        else if (ev[i].type == VizType::BurstFlash)
            flashes.push_back ({ t, (flashSeed += 0.37f) });
    }

    const float bloomLvl = juce::jlimit (0.0f, 1.0f, bus.levels[1].load() * 8.0f);
    const float breezeLvl = juce::jlimit (0.0f, 1.0f, bus.levels[2].load() * 10.0f);
    const float rootsLvl = juce::jlimit (0.0f, 1.0f, bus.levels[3].load() * 6.0f);

    const float cx = bounds.getWidth() * 0.5f;
    const float cy = (bounds.getHeight() - 46.0f) * 0.5f;

    const float coreR = 54.0f + bloomLvl * 42.0f;
    g.setColour (theme::bloom.fill.withAlpha (0.85f));
    g.fillEllipse (cx - coreR, cy - coreR, coreR * 2.0f, coreR * 2.0f);
    g.setColour (theme::bloom.tint);
    g.drawEllipse (cx - coreR * 0.6f, cy - coreR * 0.6f, coreR * 1.2f, coreR * 1.2f, 1.5f);

    rings.erase (std::remove_if (rings.begin(), rings.end(), [t] (const Ring& r) { return t - r.birth > 0.7; }), rings.end());
    for (const auto& ring : rings)
    {
        const float age = (float) (t - ring.birth) / 0.7f;
        const float rad = 12.0f + age * 90.0f;
        const float alpha = 0.9f * (1.0f - age);
        const float rx = cx + ring.pan * coreR * 1.1f;
        g.setColour (theme::dew.stroke.withAlpha (juce::jlimit (0.0f, 1.0f, alpha)));
        g.drawEllipse (rx - rad, cy - rad, rad * 2.0f, rad * 2.0f, 2.0f);
    }

    const float windAlpha = juce::jlimit (0.0f, 1.0f, breezeLvl) * 0.85f;
    if (windAlpha > 0.01f)
    {
        g.setColour (theme::breeze.stroke.withAlpha (windAlpha));
        for (int k = 0; k < 6; ++k)
        {
            juce::Path p;
            const float baseY = cy + (float) (k - 2.5f) * 11.0f;
            p.startNewSubPath (0.0f, baseY);
            for (float x = 0.0f; x <= bounds.getWidth(); x += 8.0f)
                p.lineTo (x, baseY + std::sin (x * 0.02f + (float) t * 0.7f + k * 1.1f) * 7.0f);
            g.strokePath (p, juce::PathStrokeType (1.5f));
        }
    }

    const float rootBase = bounds.getHeight() - 48.0f;
    const float rootAmp = 8.0f + rootsLvl * 26.0f;
    juce::Path rp;
    rp.startNewSubPath (0.0f, rootBase);
    for (float x = 0.0f; x <= bounds.getWidth(); x += 8.0f)
        rp.lineTo (x, rootBase - (std::sin (x * 0.015f + (float) t * 0.3f) * 0.5f + 0.5f) * rootAmp);
    rp.lineTo (bounds.getWidth(), rootBase);
    rp.closeSubPath();
    g.setColour (theme::roots.fill.withAlpha (0.35f));
    g.fillPath (rp);

    flashes.erase (std::remove_if (flashes.begin(), flashes.end(), [t] (const Flash& f) { return t - f.birth > 0.5; }), flashes.end());
    for (const auto& flash : flashes)
    {
        const float age = (float) (t - flash.birth) / 0.5f;
        const float alpha = 0.6f * (1.0f - age);
        g.setColour (theme::burst.fill.withAlpha (juce::jlimit (0.0f, 1.0f, alpha)));
        const float fr = 30.0f + age * 110.0f;
        g.fillEllipse (cx - fr, cy - fr, fr * 2.0f, fr * 2.0f);

        g.setColour (theme::burst.stroke.withAlpha (juce::jlimit (0.0f, 1.0f, alpha)));
        for (int p = 0; p < 12; ++p)
        {
            const float ang = p * 0.5235987756f + flash.seed;
            const float dist = age * 95.0f;
            const float px = cx + std::cos (ang) * dist;
            const float py = cy + std::sin (ang) * dist;
            g.fillEllipse (px - 2.0f, py - 2.0f, 4.0f, 4.0f);
        }
    }
}
}
