#include "DewEngine.h"
#include "SineTable.h"
#include "../Parameters/ParameterIDs.h"

namespace petrichor
{
void DewEngine::prepare (double sr)
{
    sampleRate = sr;
    for (auto& v : voices) v.modal.setSampleRate (sr);
    reset();
}

void DewEngine::reset()
{
    for (auto& v : voices)
    {
        v.active = false;
        v.env = 0.0f;
        v.modal.clear();
    }
    currentStep = -1;
    samplesToStep = 0;
}

void DewEngine::setPattern (const std::array<int, 16>& steps)
{
    pattern = steps;
}

void DewEngine::triggerVoice (int noteValue, int root, int octave, float gateMs, float releaseMs,
                              float purity, float material, float humanizeMs, float spread,
                              const EngineContext& ctx, const MacroState& macro)
{
    (void) macro;
    Voice* v = nullptr;
    float lowest = 1.0e9f;
    for (auto& vv : voices)
    {
        if (!vv.active) { v = &vv; break; }
        if (vv.env < lowest) { lowest = vv.env; v = &vv; }
    }

    const int degree = (noteValue == 1) ? 0 : 2;
    const int midi = root + ctx.scale->degree (degree) + octave * 12;
    const float freq = 440.0f * std::pow (2.0f, (midi - 69) / 12.0f);

    v->active = true;
    v->freq = freq;
    v->phase = noise.next();
    v->env = 0.0f;
    v->stage = 0;
    v->gateCount = 0;
    v->gateSamples = (int) std::max (1.0f, gateMs * 0.001f * (float) sampleRate);
    v->attackStep = 1.0f / (0.002f * (float) sampleRate);
    const float relMs = std::max (1.0f, releaseMs);
    v->releaseStep = 1.0f / (relMs * 0.001f * (float) sampleRate);
    v->delay = humanizeMs > 0.0f ? (int) (noise.next() * humanizeMs * 0.001f * (float) sampleRate) : 0;
    v->modalMix = 1.0f - purity / 100.0f;
    v->modalOn = v->modalMix > 0.001f;
    if (v->modalOn)
    {
        v->modal.setModes (material, freq);
        v->modal.excite (0.6f);
    }

    float pan = (noteValue == 1 ? -0.5f : 0.5f) * (spread / 100.0f);
    pan += noise.bipolar() * 0.12f * (spread / 100.0f);
    pan = juce::jlimit (-1.0f, 1.0f, pan);
    const float ang = (pan * 0.5f + 0.5f) * juce::MathConstants<float>::halfPi;
    v->panL = std::cos (ang);
    v->panR = std::sin (ang);

    if (ctx.viz != nullptr)
        ctx.viz->push ({ VizType::DewTrigger, pan });
}

void DewEngine::process (float* outL, float* outR, int n, const EngineContext& ctx, const MacroState& macro)
{
    const float sr = (float) sampleRate;
    const int octave = (int) ctx.param (IDs::dew_octave);
    const int rateIdx = (int) ctx.param (IDs::dew_rate);
    const float gateMs = ctx.param (IDs::dew_gate);
    const float releaseMs = ctx.param (IDs::dew_release) * macro.dewReleaseMul;
    const float purity = ctx.param (IDs::dew_purity);
    const float material = ctx.param (IDs::dew_material);
    const float prob = ctx.param (IDs::dew_prob);
    const float humanizeMs = ctx.param (IDs::dew_humanize) * macro.humanizeMul;
    const float spread = ctx.param (IDs::dew_spread);
    const int root = ctx.scale->rootNote();

    static constexpr float beatsPerStep[4] = { 1.0f, 0.5f, 0.25f, 1.0f / 3.0f };
    const double stepSec = (double) beatsPerStep[rateIdx] * ctx.clock->secondsPerBeat();
    const int stepSamples = (int) std::max (1.0, stepSec * sampleRate);

    std::memset (outL, 0, (size_t) n * sizeof (float));
    std::memset (outR, 0, (size_t) n * sizeof (float));

    for (int i = 0; i < n; ++i)
    {
        if (samplesToStep <= 0)
        {
            currentStep = (currentStep + 1) % 16;
            samplesToStep += stepSamples;
            const int value = pattern[currentStep];
            if (value != 0 && noise.next() * 100.0f < prob)
                triggerVoice (value, root, octave, gateMs, releaseMs, purity, material, humanizeMs, spread, ctx, macro);

            if (value != 0 && macro.extraTripletProb > 0.0f && noise.next() < macro.extraTripletProb)
            {
                const int deg = (value == 1) ? 0 : 2;
                const int midi = root + ctx.scale->degree (deg) + octave * 12;
                const float f = 440.0f * std::pow (2.0f, (midi - 69) / 12.0f);
                Voice* v = nullptr;
                float lowest = 1.0e9f;
                for (auto& vv : voices)
                {
                    if (!vv.active) { v = &vv; break; }
                    if (vv.env < lowest) { lowest = vv.env; v = &vv; }
                }
                v->active = true;
                v->freq = f;
                v->phase = noise.next();
                v->env = 0.0f;
                v->stage = 0;
                v->gateCount = 0;
                v->gateSamples = (int) std::max (1.0f, gateMs * 0.5f * 0.001f * (float) sampleRate);
                v->attackStep = 1.0f / (0.002f * (float) sampleRate);
                v->releaseStep = 1.0f / (std::max (1.0f, releaseMs) * 0.001f * (float) sampleRate);
                v->delay = (int) (stepSamples * 0.5f);
                v->modalMix = 1.0f - purity / 100.0f;
                v->modalOn = v->modalMix > 0.001f;
                if (v->modalOn) { v->modal.setModes (material, f); v->modal.excite (0.4f); }
                v->panL = 0.70710678f; v->panR = 0.70710678f;
            }
        }
        samplesToStep--;

        float l = 0.0f, r = 0.0f;
        for (auto& v : voices)
        {
            if (!v.active) continue;
            if (v.delay > 0) { v.delay--; continue; }

            v.phase += v.freq / sr;
            if (v.phase >= 1.0f) v.phase -= 1.0f;
            const float sine = sineLookup (v.phase);

            float modal = 0.0f;
            if (v.modalOn) modal = v.modal.process();

            if (v.stage == 0)
            {
                v.env += v.attackStep;
                if (v.env >= 1.0f) { v.env = 1.0f; v.stage = 1; }
            }
            else if (v.stage == 1)
            {
                if (++v.gateCount >= v.gateSamples) v.stage = 2;
            }
            else
            {
                v.env -= v.releaseStep;
                if (v.env <= 0.0f) { v.env = 0.0f; v.active = false; continue; }
            }

            const float sig = sine + (modal - sine) * v.modalMix;
            l += sig * v.env * v.panL;
            r += sig * v.env * v.panR;
        }
        outL[i] = l * 0.5f;
        outR[i] = r * 0.5f;
    }
}
}
