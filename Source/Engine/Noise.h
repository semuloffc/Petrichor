#pragma once

#include <cstdint>

namespace petrichor
{
class Noise
{
public:
    explicit Noise (uint32_t seed = 0x1f2e3d4cu)
    {
        s[0] = seed;
        s[1] = seed ^ 0x9e3779b9u;
        s[2] = seed ^ 0x85ebca6bu;
        s[3] = seed ^ 0xc2b2ae35u;
        for (auto& v : s) if (v == 0) v = 0x12345678u;
    }

    uint32_t nextU32()
    {
        const uint32_t result = s[0] + s[3];
        const uint32_t t = s[1] << 9;
        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];
        s[2] ^= t;
        s[3] = rotl (s[3], 11);
        return result;
    }

    float next()       { return (nextU32() >> 8) * (1.0f / 16777216.0f); }
    float bipolar()    { return next() * 2.0f - 1.0f; }
    float normal()     { return (bipolar() + bipolar() + bipolar()) * 0.3333333f; }

private:
    uint32_t s[4];
    static uint32_t rotl (uint32_t x, int k) { return (x << k) | (x >> (32 - k)); }
};
}
