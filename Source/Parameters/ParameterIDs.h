#pragma once

#include <JuceHeader.h>

namespace IDs
{
#define PETRICHOR_ID(name) inline const juce::String name { #name };

    PETRICHOR_ID(dew_enable)
    PETRICHOR_ID(dew_solo)
    PETRICHOR_ID(dew_octave)
    PETRICHOR_ID(dew_rate)
    PETRICHOR_ID(dew_gate)
    PETRICHOR_ID(dew_release)
    PETRICHOR_ID(dew_purity)
    PETRICHOR_ID(dew_material)
    PETRICHOR_ID(dew_prob)
    PETRICHOR_ID(dew_humanize)
    PETRICHOR_ID(dew_spread)
    PETRICHOR_ID(dew_level)

    PETRICHOR_ID(bloom_enable)
    PETRICHOR_ID(bloom_solo)
    PETRICHOR_ID(bloom_bars)
    PETRICHOR_ID(bloom_attack)
    PETRICHOR_ID(bloom_bright)
    PETRICHOR_ID(bloom_width)
    PETRICHOR_ID(bloom_drift)
    PETRICHOR_ID(bloom_level)

    PETRICHOR_ID(breeze_enable)
    PETRICHOR_ID(breeze_solo)
    PETRICHOR_ID(breeze_center)
    PETRICHOR_ID(breeze_width)
    PETRICHOR_ID(breeze_swell)
    PETRICHOR_ID(breeze_depth)
    PETRICHOR_ID(breeze_cut)
    PETRICHOR_ID(breeze_level)

    PETRICHOR_ID(roots_enable)
    PETRICHOR_ID(roots_solo)
    PETRICHOR_ID(roots_rate)
    PETRICHOR_ID(roots_depth)
    PETRICHOR_ID(roots_mono)
    PETRICHOR_ID(roots_level)

    PETRICHOR_ID(burst_enable)
    PETRICHOR_ID(burst_solo)
    PETRICHOR_ID(burst_size)
    PETRICHOR_ID(burst_tone)
    PETRICHOR_ID(burst_every)
    PETRICHOR_ID(burst_prob)
    PETRICHOR_ID(burst_level)

    PETRICHOR_ID(mist_cutoff)
    PETRICHOR_ID(gh_mix)
    PETRICHOR_ID(gh_size)
    PETRICHOR_ID(gh_damp)
    PETRICHOR_ID(gh_predelay)
    PETRICHOR_ID(gh_lowcut)
    PETRICHOR_ID(pocket_amount)
    PETRICHOR_ID(master_gain)

    PETRICHOR_ID(growth)
    PETRICHOR_ID(weather_storm)
    PETRICHOR_ID(weather_humid)

    PETRICHOR_ID(root)
    PETRICHOR_ID(scale)
    PETRICHOR_ID(follow_midi)

#undef PETRICHOR_ID
}
