#include "PresetManager.h"

namespace petrichor
{
PresetManager::PresetManager (juce::AudioProcessorValueTreeState& a)
    : apvts (a)
{
    folder = getPresetFolder();
    folder.createDirectory();
    factory = createFactoryPresets();
}

juce::File PresetManager::getPresetFolder() const
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("Reflexed").getChildFile ("Petrichor").getChildFile ("Presets");
}

juce::StringArray PresetManager::getFactoryNames() const
{
    juce::StringArray names;
    for (const auto& p : factory) names.add (p.name);
    return names;
}

juce::StringArray PresetManager::getUserNames()
{
    juce::StringArray names;
    for (const auto& f : folder.findChildFiles (juce::File::findFiles, false, "*.ptch"))
        names.add (f.getFileNameWithoutExtension());
    return names;
}

juce::String PresetManager::buildDefaultXml (const std::vector<std::pair<juce::String, float>>& overrides) const
{
    juce::ValueTree t = apvts.copyState();

    for (int i = 0; i < t.getNumChildren(); ++i)
    {
        juce::ValueTree c = t.getChild (i);
        if (!c.hasType ("PARAM")) continue;
        const juce::String id = c.getProperty ("id").toString();
        if (auto* p = apvts.getParameter (id))
            c.setProperty ("value", p->getDefaultValue(), nullptr);
    }

    juce::ValueTree pat = t.getChildWithName ("Pattern");
    if (!pat.isValid())
    {
        pat = juce::ValueTree ("Pattern");
        t.appendChild (pat, nullptr);
    }
    pat.setProperty ("dew", juce::String (defaultDewPattern()), nullptr);
    pat.setProperty ("chordA", juce::String (defaultChordA()), nullptr);
    pat.setProperty ("chordB", juce::String (defaultChordB()), nullptr);

    for (const auto& [id, native] : overrides)
    {
        if (auto* p = apvts.getParameter (id))
        {
            juce::ValueTree c = t.getChildWithProperty ("id", id);
            if (c.isValid())
            {
                const float norm = p->getNormalisableRange().convertTo0to1 (native);
                c.setProperty ("value", norm, nullptr);
            }
        }
    }

    return t.toXmlString();
}

juce::String PresetManager::getFactoryPreset (int index) const
{
    if (index < 0 || index >= (int) factory.size()) return {};
    return buildDefaultXml (factory[(size_t) index].overrides);
}

bool PresetManager::loadUserPreset (const juce::String& name, juce::String& outXml)
{
    const juce::File f = folder.getChildFile (name + ".ptch");
    if (!f.existsAsFile()) return false;
    outXml = f.loadFileAsString();
    return true;
}

bool PresetManager::saveUserPreset (const juce::String& name, const juce::String& xml, bool overwrite)
{
    const juce::File f = folder.getChildFile (name + ".ptch");
    if (f.existsAsFile() && !overwrite) return false;
    return f.replaceWithText (xml);
}

bool PresetManager::renameUserPreset (const juce::String& oldName, const juce::String& newName)
{
    const juce::File f = folder.getChildFile (oldName + ".ptch");
    const juce::File n = folder.getChildFile (newName + ".ptch");
    if (!f.existsAsFile() || n.existsAsFile()) return false;
    return f.moveFileTo (n);
}

bool PresetManager::deleteUserPreset (const juce::String& name)
{
    const juce::File f = folder.getChildFile (name + ".ptch");
    if (!f.existsAsFile()) return false;
    return f.deleteFile();
}
}
