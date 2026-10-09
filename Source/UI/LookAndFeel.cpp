#include "LookAndFeel.h"
#include "Theme.h"
#include "Fonts.h"

namespace petrichor
{
EucalyptusLookAndFeel::EucalyptusLookAndFeel()
{
    setColour (juce::ComboBox::backgroundColourId, theme::well);
    setColour (juce::ComboBox::textColourId, theme::textPrimary);
    setColour (juce::ComboBox::arrowColourId, theme::textSecondary);
    setColour (juce::ComboBox::outlineColourId, theme::divider);
    setColour (juce::ComboBox::buttonColourId, theme::well);

    setColour (juce::PopupMenu::backgroundColourId, theme::bgTop);
    setColour (juce::PopupMenu::textColourId, theme::textPrimary);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, theme::divider);
    setColour (juce::PopupMenu::highlightedTextColourId, theme::textPrimary);
    setColour (juce::PopupMenu::headerTextColourId, theme::textSecondary);

    setColour (juce::TextButton::buttonColourId, theme::bloom.fill);
    setColour (juce::TextButton::buttonOnColourId, theme::bloom.fill);
    setColour (juce::TextButton::textColourOnId, theme::textPrimary);
    setColour (juce::TextButton::textColourOffId, theme::textSecondary);

    setColour (juce::TextEditor::backgroundColourId, theme::well);
    setColour (juce::TextEditor::textColourId, theme::textPrimary);
    setColour (juce::TextEditor::highlightColourId, theme::bloom.fill);
    setColour (juce::TextEditor::highlightedTextColourId, theme::textPrimary);
    setColour (juce::TextEditor::outlineColourId, theme::divider);
    setColour (juce::TextEditor::focusedOutlineColourId, theme::bloom.stroke);

    setColour (juce::TooltipWindow::backgroundColourId, theme::bgTop);
    setColour (juce::TooltipWindow::textColourId, theme::textPrimary);
    setColour (juce::TooltipWindow::outlineColourId, theme::divider);

    setColour (juce::ResizableWindow::backgroundColourId, theme::bgMid);
}

void EucalyptusLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                          int, int, int buttonW, int buttonH, juce::ComboBox&)
{
    const float radius = 8.0f;
    g.setColour (findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (0.0f, 0.0f, (float) width, (float) height, radius);
    g.setColour (findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (0.5f, 0.5f, (float) width - 1.0f, (float) height - 1.0f, radius, 1.0f);

    const float cx = (float) (width - buttonW / 2);
    const float cy = (float) (height / 2);
    const float r = (float) juce::jmin (buttonW, buttonH) * 0.18f;
    juce::Path p;
    p.addTriangle (cx - r, cy - r * 0.5f, cx + r, cy - r * 0.5f, cx, cy + r * 0.5f);
    g.setColour (findColour (juce::ComboBox::arrowColourId));
    g.fillPath (p);
}

void EucalyptusLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (8, 0, box.getWidth() - 28, box.getHeight());
    label.setFont (interRegular (13.0f));
    label.setColour (juce::Label::textColourId, theme::textPrimary);
}
}
