#include "FactoryPresets.h"

namespace petrichor
{
const char* defaultDewPattern() { return "1,2,1,2,0,1,2,1,2,1,2,0,1,2,1,2"; }
const char* defaultChordA()     { return "0,2,3,5,7,10"; }
const char* defaultChordB()     { return "3,7,10,14,17"; }

std::vector<FactoryPreset> createFactoryPresets()
{
    std::vector<FactoryPreset> presets;

    {
        FactoryPreset p;
        p.name = "Dew Garden";
        p.overrides =
        {
            { "growth", 8.0f },
            { "dew_enable", 1.0f },
            { "bloom_enable", 0.0f },
            { "breeze_enable", 0.0f },
            { "roots_enable", 0.0f },
            { "burst_enable", 0.0f },
            { "dew_level", -20.0f }
        };
        presets.push_back (std::move (p));
    }

    {
        FactoryPreset p;
        p.name = "Canopy at Dawn";
        presets.push_back (std::move (p));
    }

    {
        FactoryPreset p;
        p.name = "Monsoon";
        p.overrides =
        {
            { "weather_storm", 80.0f },
            { "burst_prob", 50.0f }
        };
        presets.push_back (std::move (p));
    }

    {
        FactoryPreset p;
        p.name = "Frost";
        p.overrides =
        {
            { "mist_cutoff", 3.0f },
            { "gh_size", 5.0f }
        };
        presets.push_back (std::move (p));
    }

    return presets;
}
}
