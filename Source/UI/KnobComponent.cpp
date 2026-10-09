#include "KnobComponent.h"
#include "Fonts.h"

namespace petrichor
{
KnobComponent::KnobComponent (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
                              const juce::String& label_, const theme::Accent* accent_)
    : label (label_), accent (accent_)
{
    param = apvts.getParameter (paramId);
    jassert (param != nullptr);

    if (param != nullptr)
    {
        attachment = std::make_unique<juce::ParameterAttachment> (*param,
            [this] (float v) { targetValue = v; },
            nullptr);
        attachment->sendInitialUpdate();
        displayedValue = targetValue;
    }

    setWantsKeyboardFocus (true);
    startTimerHz (60);
}

KnobComponent::~KnobComponent() { stopTimer(); }

void KnobComponent::resized() {}

void KnobComponent::timerCallback()
{
    displayedValue += (targetValue - displayedValue) * 0.22f;
    repaint();
}

void KnobComponent::paint (juce::Graphics& g)
{
    const float w = (float) getWidth();
    const float h = (float) getHeight();
    const float textH = 32.0f;
    const float d = juce::jmin (w, h - textH);
    const float cx = w * 0.5f;
    const float cy = d * 0.5f;
    const float radius = juce::jmax (4.0f, d * 0.5f - 6.0f);

    const float startRad = juce::degreesToRadians (-135.0f);
    const float endRad = juce::degreesToRadians (135.0f);
    const float valueRad = juce::degreesToRadians (-135.0f + displayedValue * 270.0f);

    juce::Path track;
    track.addCentredArc (cx, cy, radius, radius, 0.0f, startRad, endRad, true);
    g.setColour (theme::divider);
    g.strokePath (track, juce::PathStrokeType (3.0f));

    if (std::fabs (displayedValue) > 0.001f)
    {
        juce::Path val;
        val.addCentredArc (cx, cy, radius, radius, 0.0f, startRad, valueRad, true);
        g.setColour (accent != nullptr ? accent->stroke : theme::textSecondary);
        g.strokePath (val, juce::PathStrokeType (4.0f));
    }

    const float px = cx + std::cos (valueRad) * radius;
    const float py = cy + std::sin (valueRad) * radius;

    g.setColour (theme::bgTop);
    g.fillEllipse (px - 5.0f, py - 5.0f, 10.0f, 10.0f);
    g.setColour (accent != nullptr ? accent->fill : theme::textPrimary);
    g.fillEllipse (px - 3.5f, py - 3.5f, 7.0f, 7.0f);

    if (hasKeyboardFocus (true))
    {
        g.setColour (theme::bloom.stroke);
        g.drawRoundedRectangle (0.5f, 0.5f, w - 1.0f, d - 1.0f, 10.0f, 2.0f);
    }

    const juce::String valueText = (param != nullptr) ? param->getText (displayedValue, 0) : juce::String();
    g.setFont (interRegular (13.0f));
    g.setColour (theme::textPrimary);
    g.drawText (valueText, juce::Rectangle<float> (0.0f, d, w, 17.0f), juce::Justification::centred, false);

    g.setFont (interRegular (12.0f));
    g.setColour (theme::textSecondary);
    g.drawText (label, juce::Rectangle<float> (0.0f, d + 16.0f, w, 15.0f), juce::Justification::centred, false);
}

void KnobComponent::mouseDown (const juce::MouseEvent& e)
{
    if (param == nullptr) return;
    dragStartValue = targetValue;
    dragStartY = e.getMouseDownY();
}

void KnobComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (param == nullptr) return;
    const float sensitivity = e.mods.isShiftDown() ? 1800.0f : 180.0f;
    const float delta = (float) (dragStartY - e.getPosition().getY());
    const float v = juce::jlimit (0.0f, 1.0f, dragStartValue + delta / sensitivity);
    param->setValueNotifyingHost (v);
}

void KnobComponent::mouseDoubleClick (const juce::MouseEvent&)
{
    if (param != nullptr)
        param->setValueNotifyingHost (param->getDefaultValue());
}

void KnobComponent::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    if (param == nullptr) return;
    const float step = wheel.isReversed ? -0.01f : 0.01f;
    const float fine = 0.1f;
    const float delta = step * wheel.deltaY * (juce::ModifierKeys::currentModifiers.isShiftDown() ? fine : 1.0f);
    param->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, targetValue + delta));
}

bool KnobComponent::keyPressed (const juce::KeyPress& key)
{
    if (param == nullptr) return false;
    const float step = juce::ModifierKeys::currentModifiers.isShiftDown() ? 0.002f : 0.01f;
    if (key == juce::KeyPress::upKey)    { param->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, targetValue + step)); return true; }
    if (key == juce::KeyPress::downKey)  { param->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, targetValue - step)); return true; }
    return false;
}
}
