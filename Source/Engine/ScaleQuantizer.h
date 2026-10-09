#pragma once

#include <JuceHeader.h>
#include <array>

namespace petrichor
{
class ScaleQuantizer
{
public:
    enum Scale { Neutral6, Major6, MinorPent, MajorPent, Chromatic };

    void set (int scaleIndex, int rootNote)
    {
        root = rootNote;
        switch (scaleIndex)
        {
            case Major6:    mask = { 0, 2, 4, 7, 9, 11 }; size = 6; break;
            case MinorPent: mask = { 0, 3, 5, 7, 10 };    size = 5; break;
            case MajorPent: mask = { 0, 2, 4, 7, 9 };     size = 5; break;
            case Chromatic: mask = { 0,1,2,3,4,5,6,7,8,9,10,11 }; size = 12; break;
            case Neutral6:
            default:        mask = { 0, 2, 3, 5, 7, 10 }; size = 6; break;
        }
    }

    int quantize (int midiNote) const
    {
        if (size == 12)
            return midiNote;

        const int pc = positiveMod (midiNote - root, 12);
        int best = mask[0], bestDist = 12;
        for (int i = 0; i < size; ++i)
        {
            const int d = std::abs (mask[i] - pc);
            const int dist = std::min (d, 12 - d);
            if (dist < bestDist || (dist == bestDist && mask[i] < best))
            {
                bestDist = dist;
                best = mask[i];
            }
        }
        return midiNote + (best - pc);
    }

    int degree (int k) const { return mask[k % size]; }
    int degreeCount() const  { return size; }
    int rootNote() const     { return root; }

    const std::array<int, 12>& maskArr() const { return mask; }

private:
    static int positiveMod (int x, int m) { return ((x % m) + m) % m; }

    std::array<int, 12> mask { 0, 2, 3, 5, 7, 10 };
    int size = 6;
    int root = 0;
};
}
