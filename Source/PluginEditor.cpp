#include "PluginEditor.h"
#include "Parameters/ParameterIDs.h"
#include "Parameters/ParameterLayout.h"
#include "UI/Fonts.h"
#include "UI/Theme.h"

namespace petrichor
{
namespace
{
void layoutKnobs (juce::Component& parent, juce::Rectangle<int> area,
                  const std::vector<juce::Component*>& knobs, int cols)
{
    if (knobs.empty()) return;
    const int rows = (int) (knobs.size() + (size_t) cols - 1) / (size_t) cols;
    const float cw = (float) area.getWidth() / (float) cols;
    const float ch = (float) area.getHeight() / (float) rows;

    for (int i = 0; i < (int) knobs.size(); ++i)
    {
        const int r = i / cols;
        const int c = i % cols;
        const int w = juce::jmin ((int) (cw - 8.0f), 84);
        const int x = area.getX() + (int) (c * cw) + (int) ((cw - w) * 0.5f);
        const int y = area.getY() + (int) (r * ch);
        knobs[(size_t) i]->setBounds (x, y, w, (int) ch);
    }
}
}

PluginEditor::PluginEditor (PluginProcessor& p)
    : juce::AudioProcessorEditor (p), proc (p)
{
    setLookAndFeel (&lnf);
    setResizable (true, true);
    getConstrainer()->setMinimumSize (960, 540);
    getConstrainer()->setMaximumSize (1920, 1080);
    getConstrainer()->setFixedAspectRatio (16.0 / 9.0);
    setSize (1280, 720);

    addAndMakeVisible (background);
    addAndMakeVisible (content);

    buildTopBar();
    buildDewCard();
    buildBreezeCard();
    buildCentre();
    buildRootsCard();
    buildBurstCard();
    buildBloomCard();
    buildSpaceCard();

    refreshPattern();
}

PluginEditor::~PluginEditor()
{
    setLookAndFeel (nullptr);
}

KnobComponent* PluginEditor::makeKnob (juce::Component& parent, const juce::String& id,
                                       const juce::String& label, const theme::Accent* accent,
                                       juce::Rectangle<int> bounds)
{
    auto* k = new KnobComponent (proc.apvts, id, label, accent);
    parent.addAndMakeVisible (k);
    if (!bounds.isEmpty()) k->setBounds (bounds);
    return k;
}

juce::ComboBox* PluginEditor::makeCombo (juce::Component& parent, const juce::String& id,
                                         const juce::StringArray& items, juce::Rectangle<int> bounds)
{
    auto* c = new juce::ComboBox();
    c->addItemList (items, 1);
    c->setBounds (bounds);
    parent.addAndMakeVisible (c);
    comboAttachments.add (new juce::AudioProcessorValueTreeState::ComboBoxAttachment (proc.apvts, id, *c));
    return c;
}

void PluginEditor::buildTopBar()
{
    auto* logo = new juce::Label();
    logo->setFont (interMedium (20.0f));
    logo->setText ("Petrichor", juce::dontSendNotification);
    logo->setColour (juce::Label::textColourId, theme::textPrimary);
    logo->setBounds (24, 18, 160, 26);
    content.addAndMakeVisible (logo);

    auto* brand = new juce::Label();
    brand->setFont (interRegular (12.0f));
    brand->setText ("Reflexed", juce::dontSendNotification);
    brand->setColour (juce::Label::textColourId, theme::textSecondary);
    brand->setBounds (24, 44, 120, 16);
    content.addAndMakeVisible (brand);

    presetBar = new PresetBar (proc, [this] { refreshPattern(); });
    presetBar->setBounds (360, 22, 560, 44);
    content.addAndMakeVisible (presetBar);

    makeCombo (content, IDs::root, rootNames(), { 940, 24, 56, 22 });
    makeCombo (content, IDs::scale, scaleNames(), { 1000, 24, 96, 22 });

    auto* follow = new juce::ToggleButton ("Follow MIDI");
    follow->setBounds (1102, 24, 92, 22);
    follow->setColour (juce::ToggleButton::textColourId, theme::textSecondary);
    follow->setColour (juce::ToggleButton::tickColourId, theme::bloom.stroke);
    follow->setColour (juce::ToggleButton::tickDisabledColourId, theme::divider);
    follow->setFont (interRegular (11.0f));
    content.addAndMakeVisible (follow);
    followMidiAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, IDs::follow_midi, *follow);

    auto* meter = new Meter (proc.viz);
    meter->setBounds (1198, 24, 54, 22);
    content.addAndMakeVisible (meter);
}

void PluginEditor::buildDewCard()
{
    auto* card = new EngineCard (proc.apvts, theme::dew, "DEW", IDs::dew_enable, IDs::dew_solo);
    card->setBounds (24, 84, 340, 220);
    content.addAndMakeVisible (card);

    makeCombo (*card, IDs::dew_octave, { "5", "6", "7", "8" }, { 86, 13, 52, 22 });
    makeCombo (*card, IDs::dew_rate, { "1/4", "1/8", "1/16", "1/8T" }, { 144, 13, 56, 22 });

    std::vector<juce::Component*> ks;
    ks.push_back (makeKnob (*card, IDs::dew_gate, "Gate", &theme::dew, {}));
    ks.push_back (makeKnob (*card, IDs::dew_purity, "Purity", &theme::dew, {}));
    ks.push_back (makeKnob (*card, IDs::dew_material, "Material", &theme::dew, {}));
    ks.push_back (makeKnob (*card, IDs::dew_prob, "Prob", &theme::dew, {}));
    ks.push_back (makeKnob (*card, IDs::dew_humanize, "Humanize", &theme::dew, {}));
    ks.push_back (makeKnob (*card, IDs::dew_level, "Level", &theme::dew, {}));
    layoutKnobs (*card, card->contentArea(), ks, 3);
}

void PluginEditor::buildBreezeCard()
{
    auto* card = new EngineCard (proc.apvts, theme::breeze, "BREEZE", IDs::breeze_enable, IDs::breeze_solo);
    card->setBounds (24, 316, 340, 220);
    content.addAndMakeVisible (card);

    std::vector<juce::Component*> ks;
    ks.push_back (makeKnob (*card, IDs::breeze_center, "Center", &theme::breeze, {}));
    ks.push_back (makeKnob (*card, IDs::breeze_width, "Width", &theme::breeze, {}));
    ks.push_back (makeKnob (*card, IDs::breeze_swell, "Swell", &theme::breeze, {}));
    ks.push_back (makeKnob (*card, IDs::breeze_depth, "Depth", &theme::breeze, {}));
    ks.push_back (makeKnob (*card, IDs::breeze_cut, "Cut", &theme::breeze, {}));
    ks.push_back (makeKnob (*card, IDs::breeze_level, "Level", &theme::breeze, {}));
    layoutKnobs (*card, card->contentArea(), ks, 3);
}

void PluginEditor::buildRootsCard()
{
    auto* card = new EngineCard (proc.apvts, theme::roots, "ROOTS", IDs::roots_enable, IDs::roots_solo);
    card->setBounds (916, 84, 340, 220);
    content.addAndMakeVisible (card);

    std::vector<juce::Component*> ks;
    ks.push_back (makeKnob (*card, IDs::roots_rate, "Rate", &theme::roots, {}));
    ks.push_back (makeKnob (*card, IDs::roots_depth, "Depth", &theme::roots, {}));
    ks.push_back (makeKnob (*card, IDs::roots_mono, "Mono", &theme::roots, {}));
    ks.push_back (makeKnob (*card, IDs::roots_level, "Level", &theme::roots, {}));
    layoutKnobs (*card, card->contentArea(), ks, 2);
}

void PluginEditor::buildBurstCard()
{
    auto* card = new EngineCard (proc.apvts, theme::burst, "BURST", IDs::burst_enable, IDs::burst_solo);
    card->setBounds (916, 316, 340, 220);
    content.addAndMakeVisible (card);

    std::vector<juce::Component*> ks;
    ks.push_back (makeKnob (*card, IDs::burst_size, "Size", &theme::burst, {}));
    ks.push_back (makeKnob (*card, IDs::burst_tone, "Tone", &theme::burst, {}));
    ks.push_back (makeKnob (*card, IDs::burst_every, "Every", &theme::burst, {}));
    ks.push_back (makeKnob (*card, IDs::burst_prob, "Prob", &theme::burst, {}));
    ks.push_back (makeKnob (*card, IDs::burst_level, "Level", &theme::burst, {}));

    auto* trigger = new juce::TextButton ("Trigger");
    trigger->onClick = [this] { proc.requestBurst(); };
    trigger->setColour (juce::TextButton::buttonColourId, theme::burst.fill);
    trigger->setColour (juce::TextButton::textColourOnId, theme::textPrimary);
    trigger->setColour (juce::TextButton::textColourOffId, theme::textSecondary);
    card->addAndMakeVisible (trigger);

    const auto area = card->contentArea();
    const int cw = area.getWidth() / 3;
    const int ch = area.getHeight() / 2;
    for (int i = 0; i < (int) ks.size(); ++i)
    {
        const int r = i / 3, c = i % 3;
        ks[(size_t) i]->setBounds (area.getX() + c * cw + (cw - 84) / 2, area.getY() + r * ch, 84, ch);
    }
    trigger->setBounds (area.getX() + 2 * cw + (cw - 72) / 2, area.getY() + ch + 22, 72, 36);
}

void PluginEditor::buildCentre()
{
    visualizer = new Visualizer (proc.viz, [this] (int step, int value) { proc.setDewStep (step, value); });
    visualizer->setBounds (380, 84, 520, 270);
    content.addAndMakeVisible (visualizer);

    auto* macro = new GlassPanel();
    macro->setBounds (380, 366, 520, 170);
    content.addAndMakeVisible (macro);

    auto* title = new juce::Label();
    title->setFont (interMedium (15.0f));
    title->setText ("MACROS", juce::dontSendNotification);
    title->setColour (juce::Label::textColourId, theme::textPrimary);
    title->setBounds (16, 12, 100, 20);
    macro->addAndMakeVisible (title);

    makeKnob (*macro, IDs::weather_storm, "Storm", nullptr, { 92, 56, 64, 96 });
    makeKnob (*macro, IDs::growth, "Growth", &theme::bloom, { 208, 16, 104, 140 });
    makeKnob (*macro, IDs::weather_humid, "Humid", nullptr, { 360, 56, 64, 96 });
}

void PluginEditor::buildBloomCard()
{
    auto* card = new EngineCard (proc.apvts, theme::bloom, "BLOOM", IDs::bloom_enable, IDs::bloom_solo);
    card->setBounds (24, 552, 700, 144);
    content.addAndMakeVisible (card);

    makeCombo (*card, IDs::bloom_bars, { "1", "2", "4" }, { 150, 13, 56, 22 });

    std::vector<juce::Component*> ks;
    ks.push_back (makeKnob (*card, IDs::bloom_attack, "Attack", &theme::bloom, {}));
    ks.push_back (makeKnob (*card, IDs::bloom_bright, "Bright", &theme::bloom, {}));
    ks.push_back (makeKnob (*card, IDs::bloom_width, "Width", &theme::bloom, {}));
    ks.push_back (makeKnob (*card, IDs::bloom_drift, "Drift", &theme::bloom, {}));
    ks.push_back (makeKnob (*card, IDs::bloom_level, "Level", &theme::bloom, {}));

    const auto area = card->contentArea();
    const int knobW = (area.getWidth() - 240) / 5;
    for (int i = 0; i < (int) ks.size(); ++i)
        ks[(size_t) i]->setBounds (area.getX() + i * knobW, area.getY(), knobW - 4, area.getHeight());

    chordLane = new ChordLane ([this] (int slot, int semitone) { proc.toggleChordNote (slot, semitone); });
    chordLane->setBounds (area.getX() + 5 * knobW + 10, area.getY() + 24, 230, 34);
    card->addAndMakeVisible (chordLane);
}

void PluginEditor::buildSpaceCard()
{
    auto* card = new GlassPanel();
    card->setBounds (740, 552, 516, 144);
    content.addAndMakeVisible (card);

    auto* title = new juce::Label();
    title->setFont (interMedium (15.0f));
    title->setText ("SPACE", juce::dontSendNotification);
    title->setColour (juce::Label::textColourId, theme::textPrimary);
    title->setBounds (16, 12, 100, 20);
    card->addAndMakeVisible (title);

    std::vector<juce::Component*> ks;
    ks.push_back (makeKnob (*card, IDs::mist_cutoff, "Mist", nullptr, {}));
    ks.push_back (makeKnob (*card, IDs::gh_mix, "Mix", nullptr, {}));
    ks.push_back (makeKnob (*card, IDs::gh_size, "Size", nullptr, {}));
    ks.push_back (makeKnob (*card, IDs::gh_damp, "Damp", nullptr, {}));
    ks.push_back (makeKnob (*card, IDs::gh_lowcut, "Low-Cut", nullptr, {}));
    ks.push_back (makeKnob (*card, IDs::pocket_amount, "Pocket", nullptr, {}));
    ks.push_back (makeKnob (*card, IDs::master_gain, "Master", nullptr, {}));

    layoutKnobs (*card, { 12, 46, 492, 86 }, ks, 7);
}

void PluginEditor::refreshPattern()
{
    if (visualizer != nullptr)
        visualizer->setPattern (proc.getDewPattern());
    if (chordLane != nullptr)
    {
        chordLane->setChordA (proc.getChordA(), proc.getChordACount());
        chordLane->setChordB (proc.getChordB(), proc.getChordBCount());
    }
}

void PluginEditor::paint (juce::Graphics&)
{
}

void PluginEditor::resized()
{
    background.setBounds (getLocalBounds());

    const float sx = (float) getWidth() / 1280.0f;
    const float sy = (float) getHeight() / 720.0f;
    content.setBounds (0, 0, getWidth(), getHeight());
    content.setTransform (juce::AffineTransform::scale (sx, sy));
}

PluginEditor::Meter::Meter (VisualizerBus& b) : bus (b) { startTimerHz (30); }
PluginEditor::Meter::~Meter() { stopTimer(); }

void PluginEditor::Meter::timerCallback() { repaint(); }

void PluginEditor::Meter::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    g.setColour (theme::well);
    g.fillRoundedRectangle (b, 6.0f);

    const float lvl = juce::jlimit (0.0f, 1.0f, bus.levels[5].load() * 1.5f);
    if (lvl > 0.002f)
    {
        g.setColour (theme::bloom.fill);
        g.fillRoundedRectangle (b.withWidth (b.getWidth() * lvl), 6.0f);
    }
    if (lvl >= 0.98f)
    {
        g.setColour (theme::burst.stroke);
        g.fillRoundedRectangle (b, 6.0f);
    }
}
}
