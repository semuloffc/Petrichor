#include "ParameterLayout.h"
#include "ParameterIDs.h"

namespace petrichor
{
juce::StringArray scaleNames()
{
    return { "Neutral-6", "Major-6", "Minor pent", "Major pent", "Chromatic" };
}

juce::StringArray rootNames()
{
    return { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;
    using P = std::unique_ptr<RangedAudioParameter>;
    using FL = AudioParameterFloat;
    using IN = AudioParameterInt;
    using CH = AudioParameterChoice;
    using BL = AudioParameterBool;

    auto f = [] (const String& id, const String& name, float lo, float hi, float def, const String& suffix)
    {
        return std::make_unique<FL>(ParameterID { id, 1 }, name, NormalisableRange<float> (lo, hi), def, suffix);
    };

    std::vector<P> params;

    params.push_back (std::make_unique<BL> (ParameterID { IDs::dew_enable, 1 }, "DEW Enable", true));
    params.push_back (std::make_unique<BL> (ParameterID { IDs::dew_solo, 1 }, "DEW Solo", false));
    params.push_back (std::make_unique<IN> (ParameterID { IDs::dew_octave, 1 }, "DEW Octave", 5, 8, 7));
    params.push_back (std::make_unique<CH> (ParameterID { IDs::dew_rate, 1 }, "DEW Rate", StringArray { "1/4", "1/8", "1/16", "1/8T" }, 2));
    params.push_back (f (IDs::dew_gate, "DEW Gate", 10.0f, 400.0f, 190.0f, " ms"));
    params.push_back (f (IDs::dew_release, "DEW Release", 20.0f, 600.0f, 120.0f, " ms"));
    params.push_back (f (IDs::dew_purity, "DEW Purity", 0.0f, 100.0f, 100.0f, " %"));
    params.push_back (f (IDs::dew_material, "DEW Material", 0.0f, 100.0f, 0.0f, " %"));
    params.push_back (f (IDs::dew_prob, "DEW Prob", 0.0f, 100.0f, 100.0f, " %"));
    params.push_back (f (IDs::dew_humanize, "DEW Humanize", 0.0f, 30.0f, 4.0f, " ms"));
    params.push_back (f (IDs::dew_spread, "DEW Spread", 0.0f, 100.0f, 0.0f, " %"));
    params.push_back (f (IDs::dew_level, "DEW Level", -30.0f, 0.0f, -24.0f, " dB"));

    params.push_back (std::make_unique<BL> (ParameterID { IDs::bloom_enable, 1 }, "BLOOM Enable", true));
    params.push_back (std::make_unique<BL> (ParameterID { IDs::bloom_solo, 1 }, "BLOOM Solo", false));
    params.push_back (std::make_unique<CH> (ParameterID { IDs::bloom_bars, 1 }, "BLOOM Bars", StringArray { "1", "2", "4" }, 0));
    params.push_back (f (IDs::bloom_attack, "BLOOM Attack", 0.2f, 8.0f, 1.5f, " s"));
    params.push_back (f (IDs::bloom_bright, "BLOOM Bright", 0.0f, 100.0f, 40.0f, " %"));
    params.push_back (f (IDs::bloom_width, "BLOOM Width", 0.0f, 100.0f, 85.0f, " %"));
    params.push_back (f (IDs::bloom_drift, "BLOOM Drift", 0.0f, 20.0f, 4.0f, " ct"));
    params.push_back (f (IDs::bloom_level, "BLOOM Level", -30.0f, 0.0f, -26.0f, " dB"));

    params.push_back (std::make_unique<BL> (ParameterID { IDs::breeze_enable, 1 }, "BREEZE Enable", true));
    params.push_back (std::make_unique<BL> (ParameterID { IDs::breeze_solo, 1 }, "BREEZE Solo", false));
    params.push_back (f (IDs::breeze_center, "BREEZE Center", 2.0f, 8.0f, 4.6f, " kHz"));
    params.push_back (f (IDs::breeze_width, "BREEZE Width", 0.5f, 3.0f, 0.8f, " oct"));
    params.push_back (f (IDs::breeze_swell, "BREEZE Swell", 1.0f, 8.0f, 2.0f, " bars"));
    params.push_back (f (IDs::breeze_depth, "BREEZE Depth", 0.0f, 24.0f, 18.0f, " dB"));
    params.push_back (f (IDs::breeze_cut, "BREEZE Cut", 0.0f, 20.0f, 11.0f, " dB"));
    params.push_back (f (IDs::breeze_level, "BREEZE Level", -40.0f, 0.0f, -34.0f, " dB"));

    params.push_back (std::make_unique<BL> (ParameterID { IDs::roots_enable, 1 }, "ROOTS Enable", true));
    params.push_back (std::make_unique<BL> (ParameterID { IDs::roots_solo, 1 }, "ROOTS Solo", false));
    params.push_back (f (IDs::roots_rate, "ROOTS Rate", 0.1f, 1.0f, 0.3f, " Hz"));
    params.push_back (f (IDs::roots_depth, "ROOTS Depth", 0.0f, 100.0f, 60.0f, " %"));
    params.push_back (f (IDs::roots_mono, "ROOTS Mono", 0.0f, 250.0f, 120.0f, " Hz"));
    params.push_back (f (IDs::roots_level, "ROOTS Level", -40.0f, 0.0f, -30.0f, " dB"));

    params.push_back (std::make_unique<BL> (ParameterID { IDs::burst_enable, 1 }, "BURST Enable", true));
    params.push_back (std::make_unique<BL> (ParameterID { IDs::burst_solo, 1 }, "BURST Solo", false));
    params.push_back (f (IDs::burst_size, "BURST Size", 0.2f, 2.5f, 0.9f, " s"));
    params.push_back (f (IDs::burst_tone, "BURST Tone", -100.0f, 100.0f, 0.0f, ""));
    params.push_back (f (IDs::burst_every, "BURST Every", 1.0f, 8.0f, 4.0f, " bars"));
    params.push_back (f (IDs::burst_prob, "BURST Prob", 0.0f, 100.0f, 25.0f, " %"));
    params.push_back (f (IDs::burst_level, "BURST Level", -24.0f, 0.0f, -9.0f, " dB"));

    params.push_back (f (IDs::mist_cutoff, "MIST Cutoff", 1.5f, 16.0f, 6.0f, " kHz"));
    params.push_back (f (IDs::gh_mix, "Greenhouse Mix", 0.0f, 60.0f, 25.0f, " %"));
    params.push_back (f (IDs::gh_size, "Greenhouse Size", 0.5f, 6.0f, 2.5f, " s"));
    params.push_back (f (IDs::gh_damp, "Greenhouse Damp", 2.0f, 12.0f, 5.0f, " kHz"));
    params.push_back (f (IDs::gh_predelay, "Greenhouse Pre", 0.0f, 80.0f, 10.0f, " ms"));
    params.push_back (f (IDs::gh_lowcut, "Greenhouse Lowcut", 20.0f, 400.0f, 120.0f, " Hz"));
    params.push_back (f (IDs::pocket_amount, "Pocket", 0.0f, 100.0f, 0.0f, " %"));
    params.push_back (f (IDs::master_gain, "Master", -24.0f, 6.0f, 0.0f, " dB"));

    params.push_back (f (IDs::growth, "Growth", 0.0f, 100.0f, 55.0f, " %"));
    params.push_back (f (IDs::weather_storm, "Weather Storm", 0.0f, 100.0f, 30.0f, ""));
    params.push_back (f (IDs::weather_humid, "Weather Humid", 0.0f, 100.0f, 40.0f, ""));

    params.push_back (std::make_unique<CH> (ParameterID { IDs::root, 1 }, "Root", rootNames(), 0));
    params.push_back (std::make_unique<CH> (ParameterID { IDs::scale, 1 }, "Scale", scaleNames(), 0));
    params.push_back (std::make_unique<BL> (ParameterID { IDs::follow_midi, 1 }, "Follow MIDI", false));

    return { params.begin(), params.end() };
}
}
