#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>
#include "Dsp.h"

namespace petrichor
{
class Greenhouse
{
public:
    void prepare (double sampleRate);
    void reset();
    void setParams (float sizeSec, float dampHz, float predelayMs, float lowcutHz);
    void process (const float* inL, const float* inR, float* outL, float* outR, int n);

private:
    struct Line
    {
        std::vector<float> buf;
        int len = 0;
        int w = 0;
        void prepare (int n) { buf.assign ((size_t) n, 0.0f); len = n; w = 0; }
        void clear() { std::fill (buf.begin(), buf.end(), 0.0f); w = 0; }
        void write (float x) { buf[(size_t) w] = x; w = (w + 1) % len; }
        float readInt (int offset)
        {
            int i = w - offset;
            if (i < 0) i += len;
            return buf[(size_t) i];
        }
        float readFrac (float offset)
        {
            float p = (float) w - offset;
            if (p < 0.0f) p += (float) len;
            const int i = (int) p;
            const float f = p - (float) i;
            const int j = (i + 1) % len;
            return buf[(size_t) i] + (buf[(size_t) j] - buf[(size_t) i]) * f;
        }
    };

    double sampleRate = 48000.0;
    std::array<Line, 8> lines;
    std::array<int, 8> prime { 101, 103, 107, 109, 113, 127, 131, 137 };
    static constexpr int primeSum = 928;
    std::array<int, 8> lineLen;
    std::array<OnePole, 8> damp;
    OnePole lowcut;
    Line predelay;
    int predelayLen = 0;

    float modPhase = 0.0f;
    float modDepth = 0.0f;
    float feedback = 0.82f;

    static constexpr float hadamard[8][8] =
    {
        { 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1,-1, 1,-1, 1,-1, 1,-1 },
        { 1, 1,-1,-1, 1, 1,-1,-1 },
        { 1,-1,-1, 1, 1,-1,-1, 1 },
        { 1, 1, 1, 1,-1,-1,-1,-1 },
        { 1,-1, 1,-1,-1, 1,-1, 1 },
        { 1, 1,-1,-1,-1,-1, 1, 1 },
        { 1,-1,-1, 1,-1, 1, 1,-1 }
    };

    static constexpr float signL[8] = { 1,-1, 1,-1, 1,-1, 1,-1 };
    static constexpr float signR[8] = { 1, 1,-1,-1, 1, 1,-1,-1 };
};
}
