#include "BloomEngine.h"
#include "SineTable.h"
#include "Dsp.h"
#include "../Parameters/ParameterIDs.h"

namespace petrichor
{
void BloomEngine::prepare (double sr)
{
    sampleRate = sr;
    const int delaySamples = (int) (0.012 * sr) + 2;
    delayL.prepare (delaySamples);
    delayR.prepare (delaySamples);
    baseDelay = (float) (0.004 * sr);
    depthSamples = (float) (0.003 * sr);
    reset();
}

void BloomEngine::reset()
{
    phases.fill (0.0f);
    for (auto& n : notes) { n.baseFreq = 0.0f; n.drift = 0.0f; n.active = false; }
    lfoPhase = 0.0f;
    slot = 0;
    barCount = 0;
    lastRoot = -1;
    lastScale = -1;
    lastBright = -1.0f;
    lastAttack = -1.0f;
    retunePending = true;
    fadeGain.reset (sampleRate, 1.5);
    fadeGain.setCurrentAndTargetValue (0.0f);
    fadeGain.setTargetValue (1.0f);
}

void BloomEngine::setChords (const std::array<int, 24>& a, int na, const std::array<int, 24>& b, int nb)
{
    chordA = a;
    chordB = b;
    countA = na;
    countB = nb;
    retunePending = true;
}

void BloomEngine::retune (const EngineContext& ctx)
{
    const int root = ctx.scale->rootNote();
    const auto& chord = (slot == 0) ? chordA : chordB;
    const int count = (slot == 0) ? countA : countB;

    for (int i = 0; i < kNotes; ++i)
    {
        if (i < count)
        {
            const int midi = ctx.scale->quantize (60 + root + chord[(size_t) i]);
            notes[(size_t) i].baseFreq = 440.0f * std::pow (2.0f, (midi - 69) / 12.0f);
            notes[(size_t) i].active = true;
        }
        else
        {
            notes[(size_t) i].active = false;
        }
    }
}

void BloomEngine::process (float* outL, float* outR, int n, const EngineContext& ctx, const MacroState& macro)
{
    const float sr = (float) sampleRate;

    static constexpr int barsChoice[3] = { 1, 2, 4 };
    const int bars = barsChoice[(int) ctx.param (IDs::bloom_bars)];
    const float attack = ctx.param (IDs::bloom_attack);
    const float bright = ctx.param (IDs::bloom_bright) / 100.0f;
    const float width = ctx.param (IDs::bloom_width) / 100.0f;
    const float driftCt = ctx.param (IDs::bloom_drift);

    if (ctx.clock->barChanged)
    {
        ++barCount;
        if (barCount >= bars) { barCount = 0; slot ^= 1; retunePending = true; }
    }

    const int root = ctx.scale->rootNote();
    if (root != lastRoot || ctx.scaleIndex != lastScale)
    {
        lastRoot = root;
        lastScale = ctx.scaleIndex;
        retunePending = true;
    }

    if (retunePending)
    {
        retune (ctx);
        retunePending = false;
    }

    if (bright != lastBright)
    {
        const float slope = 2.2f - 1.6f * bright;
        float sum = 0.0f;
        for (int p = 0; p < kPartials; ++p) { partialAmp[(size_t) p] = std::pow ((float) (p + 1), -slope); sum += partialAmp[(size_t) p]; }
        const float inv = 1.0f / sum;
        for (int p = 0; p < kPartials; ++p) partialAmp[(size_t) p] *= inv;
        lastBright = bright;
    }

    if (attack != lastAttack)
    {
        const float ramp = juce::jmax (0.05f, attack);
        const float current = fadeGain.getCurrentValue();
        fadeGain.reset (sampleRate, ramp);
        fadeGain.setCurrentAndTargetValue (current);
        fadeGain.setTargetValue (1.0f);
        lastAttack = attack;
    }

    const float detuneCents = width * 6.0f;
    const float detune0 = centsToRatio (-detuneCents);
    const float detune1 = centsToRatio (detuneCents);
    const float ang0 = (0.5f - 0.5f * width) * juce::MathConstants<float>::halfPi;
    const float ang1 = (0.5f + 0.5f * width) * juce::MathConstants<float>::halfPi;
    const float panL0 = std::cos (ang0), panR0 = std::sin (ang0);
    const float panL1 = std::cos (ang1), panR1 = std::sin (ang1);

    const float dt = 1.0f / sr;
    const float sqrtDt = std::sqrt (dt);
    const float tau = 1.0f / (6.2831853f * 0.2f);
    const float sigma = (driftCt > 0.0f) ? (driftCt / std::sqrt (tau * 0.5f)) : 0.0f;
    const float driftDecay = dt / tau;

    int voicesToRender = macro.growth.bloomVoices;
    if (voicesToRender > kNotes) voicesToRender = kNotes;

    const float lfoInc = 6.2831853f * 0.3f / sr;

    for (int i = 0; i < n; ++i)
    {
        float l = 0.0f, r = 0.0f;

        for (int ni = 0; ni < voicesToRender; ++ni)
        {
            Note& note = notes[(size_t) ni];
            if (!note.active) continue;

            note.drift += -note.drift * driftDecay + sigma * noise.normal() * sqrtDt;
            note.drift = juce::jlimit (-20.0f, 20.0f, note.drift);
            const float driftRatio = centsToRatio (note.drift);
            const float baseInc = note.baseFreq / sr;

            for (int p = 0; p < kPartials; ++p)
            {
                const float amp = partialAmp[(size_t) p];
                const float harm = (float) (p + 1);
                const size_t i0 = ((size_t) ni * kCopies + 0) * kPartials + (size_t) p;
                const size_t i1 = ((size_t) ni * kCopies + 1) * kPartials + (size_t) p;

                float ph0 = phases[i0] + baseInc * harm * detune0 * driftRatio;
                float ph1 = phases[i1] + baseInc * harm * detune1 * driftRatio;
                ph0 -= std::floor (ph0);
                ph1 -= std::floor (ph1);
                phases[i0] = ph0;
                phases[i1] = ph1;

                const float s0 = sineLookup (ph0) * amp;
                const float s1 = sineLookup (ph1) * amp;
                l += s0 * panL0 + s1 * panL1;
                r += s0 * panR0 + s1 * panR1;
            }
        }

        const float fg = fadeGain.getNextValue();
        l *= fg * 0.08f;
        r *= fg * 0.08f;

        lfoPhase += lfoInc;
        if (lfoPhase > 6.2831853f) lfoPhase -= 6.2831853f;

        delayL.write (l);
        delayR.write (r);
        const float mod0 = depthSamples * std::sin (lfoPhase);
        const float mod1 = depthSamples * std::sin (lfoPhase + juce::MathConstants<float>::pi);
        const float wetL = delayL.read (baseDelay + mod0);
        const float wetR = delayR.read (baseDelay + mod1);

        outL[i] = l * 0.5f + wetL * 0.5f;
        outR[i] = r * 0.5f + wetR * 0.5f;
    }
}
}
