#include "EngineCard.h"
#include "GlassBackground.h"
#include "Fonts.h"
#include "../Parameters/ParameterIDs.h"

namespace petrichor
{
EngineCard::EngineCard (juce::AudioProcessorValueTreeState& apvts_, const theme::Accent& accent_,
                        const juce::String& name_, const juce::String& enableId_, const juce::String& soloId_)
    : apvts (apvts_), accent (accent_), name (name_), enableId (enableId_), soloId (soloId_)
{
}

juce::Rectangle<int> EngineCard::contentArea() const
{
    auto r = getLocalBounds();
    return r.withTrimmedTop (46).withTrimmedBottom (12).withTrimmedLeft (12).withTrimmedRight (12);
}

juce::Rectangle<int> EngineCard::enableRect() const
{
    return { getWidth() - 56, 12, 44, 22 };
}

juce::Rectangle<int> EngineCard::soloRect() const
{
    return { getWidth() - 102, 12, 42, 22 };
}

void EngineCard::drawPill (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& text, bool on) const
{
    g.setColour (on ? accent.fill : theme::divider);
    g.fillRoundedRectangle (r.toFloat(), 11.0f);
    g.setColour (theme::textPrimary);
    g.setFont (interMedium (11.0f));
    g.drawText (text, r, juce::Justification::centred, false);
}

void EngineCard::paint (juce::Graphics& g)
{
    GlassBackground::drawGlassCard (g, getLocalBounds().toFloat());

    g.setColour (accent.fill);
    g.fillEllipse (14.0f, 19.0f, 8.0f, 8.0f);

    g.setFont (interMedium (15.0f));
    g.setColour (theme::textPrimary);
    g.drawText (name, juce::Rectangle<int> (28, 10, 140, 24), juce::Justification::left, false);

    const bool enabled = apvts.getRawParameterValue (enableId)->load() >= 0.5f;
    const bool solo = apvts.getRawParameterValue (soloId)->load() >= 0.5f;
    drawPill (g, enableRect(), enabled ? "On" : "Off", enabled);
    drawPill (g, soloRect(), "Solo", solo);
}

void EngineCard::mouseDown (const juce::MouseEvent& e)
{
    const auto p = e.getPosition();
    if (enableRect().contains (p))
    {
        const bool v = apvts.getRawParameterValue (enableId)->load() >= 0.5f;
        if (auto* param = apvts.getParameter (enableId))
            param->setValueNotifyingHost (v ? 0.0f : 1.0f);
        repaint();
    }
    else if (soloRect().contains (p))
    {
        const bool v = apvts.getRawParameterValue (soloId)->load() >= 0.5f;
        if (auto* param = apvts.getParameter (soloId))
            param->setValueNotifyingHost (v ? 0.0f : 1.0f);
        repaint();
    }
}
}
