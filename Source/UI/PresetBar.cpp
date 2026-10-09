#include "PresetBar.h"
#include "../PluginProcessor.h"
#include "Theme.h"
#include "Fonts.h"

namespace petrichor
{
PresetBar::PresetBar (PluginProcessor& proc_, std::function<void()> onPresetLoaded_)
    : proc (proc_), onPresetLoaded (std::move (onPresetLoaded_))
{
    addAndMakeVisible (prev);
    addAndMakeVisible (next);
    addAndMakeVisible (save);
    addAndMakeVisible (menu);
    addAndMakeVisible (nameLabel);

    nameLabel.setFont (interMedium (14.0f));
    nameLabel.setColour (juce::Label::textColourId, theme::textPrimary);
    nameLabel.setJustificationType (juce::Justification::centred);
    nameLabel.setText (currentName, juce::dontSendNotification);

    prev.onClick = [this] { step (-1); };
    next.onClick = [this] { step (1); };
    save.onClick = [this] { showSaveDialog(); };
    menu.onClick = [this] { showMenu(); };
}

void PresetBar::setPresetName (const juce::String& n)
{
    currentName = n;
    nameLabel.setText (n, juce::dontSendNotification);
}

juce::StringArray PresetBar::combinedNames()
{
    juce::StringArray all;
    all.addArray (proc.getFactoryPresetNames());
    all.addArray (proc.getUserPresetNames());
    return all;
}

void PresetBar::loadByName (const juce::String& n)
{
    const auto factory = proc.getFactoryPresetNames();
    const int fi = factory.indexOf (n);
    if (fi >= 0) proc.loadFactoryPreset (fi);
    else if (proc.getUserPresetNames().contains (n)) proc.loadUserPreset (n);

    currentName = n;
    nameLabel.setText (n, juce::dontSendNotification);
    if (onPresetLoaded) onPresetLoaded();
}

void PresetBar::step (int dir)
{
    const auto all = combinedNames();
    if (all.isEmpty()) return;
    int idx = all.indexOf (currentName);
    if (idx < 0) idx = 0;
    else idx = (idx + dir + all.size()) % all.size();
    loadByName (all[idx]);
}

void PresetBar::showSaveDialog()
{
    juce::AlertWindow w ("Save Preset", "Enter a name for the user preset:", juce::AlertWindow::NoIcon);
    w.addTextEditor ("name", currentName, "Name:", false);
    w.addButton ("Save", 1);
    w.addButton ("Cancel", 0);
    if (w.runModalLoop() != 1) return;
    juce::String name = w.getTextEditorContents ("name").trim();
    if (name.isEmpty()) return;

    if (!proc.getUserPresetNames().contains (name))
    {
        proc.saveUserPreset (name, false);
    }
    else if (juce::AlertWindow::showOkCancelBox (juce::AlertWindow::QuestionIcon, "Overwrite Preset",
             "A preset named \"" + name + "\" already exists. Overwrite it?", "Overwrite", "Cancel"))
    {
        proc.saveUserPreset (name, true);
    }
    currentName = name;
    nameLabel.setText (name, juce::dontSendNotification);
}

void PresetBar::showMenu()
{
    constexpr int kSave = 1, kRename = 2, kDelete = 3, kFactory = 1000, kUser = 2000;

    juce::PopupMenu m;
    m.addSectionHeader ("Factory");
    const auto factory = proc.getFactoryPresetNames();
    for (int i = 0; i < factory.size(); ++i) m.addItem (kFactory + i, factory[i]);

    const auto user = proc.getUserPresetNames();
    m.addSectionHeader ("User");
    if (user.isEmpty()) m.addItem (kDelete + 100, "(no user presets)", false);
    for (int i = 0; i < user.size(); ++i) m.addItem (kUser + i, user[i]);

    m.addSeparator();
    m.addItem (kSave, "Save Preset\u2026");
    m.addItem (kRename, "Rename\u2026", !user.contains (currentName));
    m.addItem (kDelete, "Delete\u2026", !user.contains (currentName));

    const int result = m.show();
    if (result == kSave) { showSaveDialog(); return; }
    if (result == kRename)
    {
        juce::AlertWindow w ("Rename Preset", "New name:", juce::AlertWindow::NoIcon);
        w.addTextEditor ("name", currentName, "Name:", false);
        w.addButton ("Rename", 1);
        w.addButton ("Cancel", 0);
        if (w.runModalLoop() == 1)
        {
            juce::String nn = w.getTextEditorContents ("name").trim();
            if (nn.isNotEmpty() && proc.renameUserPreset (currentName, nn))
            {
                currentName = nn;
                nameLabel.setText (nn, juce::dontSendNotification);
            }
        }
        return;
    }
    if (result == kDelete)
    {
        if (juce::AlertWindow::showOkCancelBox (juce::AlertWindow::QuestionIcon, "Delete Preset",
             "Delete \"" + currentName + "\"?", "Delete", "Cancel"))
        {
            proc.deleteUserPreset (currentName);
        }
        return;
    }
    if (result >= kFactory && result < kFactory + factory.size())
    {
        loadByName (factory[result - kFactory]);
        return;
    }
    if (result >= kUser && result < kUser + user.size())
    {
        loadByName (user[result - kUser]);
        return;
    }
}

void PresetBar::resized()
{
    const int h = getHeight();
    const int bw = 26;
    const int y = (h - 24) / 2;

    prev.setBounds (8, y, bw, 24);
    next.setBounds (8 + bw + 4, y, bw, 24);

    const int menuW = 40;
    menu.setBounds (getWidth() - 8 - menuW, y, menuW, 24);
    save.setBounds (getWidth() - 8 - menuW - 60, y, 56, 24);
    nameLabel.setBounds (8 + bw * 2 + 12, y, getWidth() - (8 + bw * 2 + 12 + menuW + 68), 24);
}

void PresetBar::paint (juce::Graphics& g)
{
    g.setColour (theme::well);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 12.0f);
    g.setColour (theme::divider.withAlpha (0.6f));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f), 12.0f, 1.0f);
}
}
