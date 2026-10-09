#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets/FactoryPresets.h"

namespace petrichor
{
static juce::AudioProcessor::BusesProperties petrichorBuses()
{
    return juce::AudioProcessor::BusesProperties()
        .withInput  ("Pocket", juce::AudioChannelSet::stereo(), false)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
        .withOutput ("DEW",    juce::AudioChannelSet::stereo(), false)
        .withOutput ("BLOOM",  juce::AudioChannelSet::stereo(), false)
        .withOutput ("BREEZE", juce::AudioChannelSet::stereo(), false)
        .withOutput ("ROOTS",  juce::AudioChannelSet::stereo(), false)
        .withOutput ("BURST",  juce::AudioChannelSet::stereo(), false);
}

PluginProcessor::PluginProcessor()
    : juce::AudioProcessor (petrichorBuses()),
      apvts (*this, nullptr, "Parameters", createParameterLayout()),
      viz(),
      presets (apvts),
      rack (apvts, viz)
{
    ensurePatternTree();
    syncPattern();
}

PluginProcessor::~PluginProcessor() = default;

const juce::String PluginProcessor::getName() const { return "Petrichor"; }
bool PluginProcessor::acceptsMidi() const { return true; }
bool PluginProcessor::producesMidi() const { return false; }
bool PluginProcessor::isMidiEffect() const { return false; }
double PluginProcessor::getTailLengthSeconds() const { return 0.0; }
bool PluginProcessor::hasEditor() const { return true; }

int PluginProcessor::getNumPrograms() { return 1; }
int PluginProcessor::getCurrentProgram() { return 0; }
void PluginProcessor::setCurrentProgram (int) {}
const juce::String PluginProcessor::getProgramName (int) { return "Default"; }
void PluginProcessor::changeProgramName (int, const juce::String&) {}

void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    rack.prepare (sampleRate, samplesPerBlock);
}

void PluginProcessor::releaseResources()
{
    rack.reset();
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mainIn = layouts.getMainInputChannelSet();
    if (!mainIn.isDisabled() && mainIn != juce::AudioChannelSet::stereo())
        return false;

    for (int i = 0; i < layouts.outputBuses.size(); ++i)
    {
        const auto cs = layouts.getChannelSet (false, i);
        if (cs != juce::AudioChannelSet::stereo() && !cs.isDisabled())
            return false;
    }
    return true;
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    if (n == 0) return;

    auto mainBuf = getBusBuffer (buffer, false, 0);
    if (mainBuf.getNumChannels() == 0) return;

    float* mainL = mainBuf.getWritePointer (0);
    float* mainR = (mainBuf.getNumChannels() > 1) ? mainBuf.getWritePointer (1) : mainL;

    float* auxL[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
    float* auxR[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
    for (int k = 0; k < 5; ++k)
    {
        auto b = getBusBuffer (buffer, false, k + 1);
        if (b.getNumChannels() >= 2)
        {
            auxL[k] = b.getWritePointer (0);
            auxR[k] = b.getWritePointer (1);
        }
    }

    auto side = getBusBuffer (buffer, true, 0);
    const float* sideL = (side.getNumChannels() >= 2) ? side.getReadPointer (0) : nullptr;
    const float* sideR = (side.getNumChannels() >= 2) ? side.getReadPointer (1) : nullptr;

    rack.process (mainL, mainR, auxL, auxR, sideL, sideR, midi, getPlayHead(), n);
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto xml = apvts.copyState().createXml();
    if (xml != nullptr)
        copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        applyStateXml (xml->toString());
}

void PluginProcessor::ensurePatternTree()
{
    if (apvts.state.getChildWithName ("Pattern").isValid()) return;

    juce::ValueTree pat ("Pattern");
    pat.setProperty ("dew", juce::String (defaultDewPattern()), nullptr);
    pat.setProperty ("chordA", juce::String (defaultChordA()), nullptr);
    pat.setProperty ("chordB", juce::String (defaultChordB()), nullptr);
    apvts.state.appendChild (pat, nullptr);
}

void PluginProcessor::syncPattern()
{
    EngineState s;
    ensurePatternTree();
    const juce::ValueTree pat = apvts.state.getChildWithName ("Pattern");

    auto parseDew = [] (const juce::String& str)
    {
        std::array<int, 16> a {};
        const auto toks = juce::StringArray::fromTokens (str, ",", "");
        for (int i = 0; i < 16 && i < toks.size(); ++i)
            a[(size_t) i] = juce::jlimit (0, 2, toks[i].getIntValue());
        return a;
    };
    auto parseChord = [] (const juce::String& str, std::array<int, 24>& a, int& count)
    {
        count = 0;
        for (const auto& t : juce::StringArray::fromTokens (str, ",", ""))
            if (count < 24) a[(size_t) count++] = t.getIntValue();
    };

    s.dew = parseDew (pat.getProperty ("dew").toString());
    parseChord (pat.getProperty ("chordA").toString(), s.chordA, s.countA);
    parseChord (pat.getProperty ("chordB").toString(), s.chordB, s.countB);

    rack.publishState (s);
}

std::array<int, 16> PluginProcessor::getDewPattern() const
{
    std::array<int, 16> a {};
    const juce::ValueTree pat = apvts.state.getChildWithName ("Pattern");
    if (pat.isValid())
    {
        const auto toks = juce::StringArray::fromTokens (pat.getProperty ("dew").toString(), ",", "");
        for (int i = 0; i < 16 && i < toks.size(); ++i)
            a[(size_t) i] = juce::jlimit (0, 2, toks[i].getIntValue());
    }
    return a;
}

std::array<int, 24> PluginProcessor::getChordA() const
{
    std::array<int, 24> a {};
    const juce::ValueTree pat = apvts.state.getChildWithName ("Pattern");
    if (pat.isValid())
    {
        int i = 0;
        for (const auto& t : juce::StringArray::fromTokens (pat.getProperty ("chordA").toString(), ",", ""))
            if (i < 24) a[(size_t) i++] = t.getIntValue();
    }
    return a;
}

std::array<int, 24> PluginProcessor::getChordB() const
{
    std::array<int, 24> a {};
    const juce::ValueTree pat = apvts.state.getChildWithName ("Pattern");
    if (pat.isValid())
    {
        int i = 0;
        for (const auto& t : juce::StringArray::fromTokens (pat.getProperty ("chordB").toString(), ",", ""))
            if (i < 24) a[(size_t) i++] = t.getIntValue();
    }
    return a;
}

int PluginProcessor::getChordACount() const
{
    const juce::ValueTree pat = apvts.state.getChildWithName ("Pattern");
    if (!pat.isValid()) return 0;
    return juce::StringArray::fromTokens (pat.getProperty ("chordA").toString(), ",", "").size();
}

int PluginProcessor::getChordBCount() const
{
    const juce::ValueTree pat = apvts.state.getChildWithName ("Pattern");
    if (!pat.isValid()) return 0;
    return juce::StringArray::fromTokens (pat.getProperty ("chordB").toString(), ",", "").size();
}

void PluginProcessor::setDewStep (int step, int value)
{
    if (step < 0 || step > 15) return;
    ensurePatternTree();
    juce::ValueTree pat = apvts.state.getChildWithName ("Pattern");
    auto toks = juce::StringArray::fromTokens (pat.getProperty ("dew").toString(), ",", "");
    while (toks.size() < 16) toks.add ("0");
    toks.set (step, juce::String (juce::jlimit (0, 2, value)));
    pat.setProperty ("dew", toks.joinIntoString (","), nullptr);
    syncPattern();
}

void PluginProcessor::toggleChordNote (int slot, int semitone)
{
    ensurePatternTree();
    juce::ValueTree pat = apvts.state.getChildWithName ("Pattern");
    const auto prop = (slot == 0) ? "chordA" : "chordB";

    std::vector<int> notes;
    for (const auto& t : juce::StringArray::fromTokens (pat.getProperty (prop).toString(), ",", ""))
        notes.push_back (t.getIntValue());

    const auto it = std::find (notes.begin(), notes.end(), semitone);
    if (it != notes.end()) notes.erase (it);
    else { notes.push_back (semitone); std::sort (notes.begin(), notes.end()); }

    juce::StringArray out;
    for (const int n : notes) out.add (juce::String (n));
    pat.setProperty (prop, out.joinIntoString (","), nullptr);
    syncPattern();
}

void PluginProcessor::requestBurst() { rack.requestBurst(); }

void PluginProcessor::applyStateXml (const juce::String& xml)
{
    if (xml.isEmpty()) return;
    if (auto parsed = juce::parseXML (xml))
    {
        juce::ValueTree t = juce::ValueTree::fromXml (*parsed);
        if (t.isValid())
        {
            apvts.replaceState (t);
            ensurePatternTree();
            syncPattern();
        }
    }
}

void PluginProcessor::loadFactoryPreset (int index)
{
    applyStateXml (presets.getFactoryPreset (index));
}

void PluginProcessor::loadUserPreset (const juce::String& name)
{
    juce::String xml;
    if (presets.loadUserPreset (name, xml))
        applyStateXml (xml);
}

void PluginProcessor::saveUserPreset (const juce::String& name, bool overwrite)
{
    if (auto xml = apvts.copyState().createXml())
        presets.saveUserPreset (name, xml->toString(), overwrite);
}

void PluginProcessor::renameUserPreset (const juce::String& oldName, const juce::String& newName)
{
    presets.renameUserPreset (oldName, newName);
}

void PluginProcessor::deleteUserPreset (const juce::String& name)
{
    presets.deleteUserPreset (name);
}

juce::StringArray PluginProcessor::getUserPresetNames() { return presets.getUserNames(); }
juce::StringArray PluginProcessor::getFactoryPresetNames() { return presets.getFactoryNames(); }
}
