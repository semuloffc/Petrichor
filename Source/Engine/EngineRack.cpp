#include "EngineRack.h"
#include "Dsp.h"
#include "../Parameters/ParameterIDs.h"

namespace petrichor
{
EngineRack::EngineRack (juce::AudioProcessorValueTreeState& a, VisualizerBus& v)
    : params (a), viz (v)
{
}

void EngineRack::prepare (double sr, int blockSize)
{
    sampleRate = sr;

    auto resize = [&] (std::vector<float>& v) { v.assign ((size_t) blockSize, 0.0f); };
    resize (dewL);  resize (dewR);
    resize (bloomL); resize (bloomR);
    resize (breezeL); resize (breezeR);
    resize (rootsL); resize (rootsR);
    resize (burstL); resize (burstR);
    resize (sendL); resize (sendR);
    resize (wetL);  resize (wetR);
    resize (dryL);  resize (dryR);
    resize (duckL); resize (duckR);
    resize (mainL); resize (mainR);
    resize (zero);

    dew.prepare (sr);
    bloom.prepare (sr);
    breeze.prepare (sr);
    roots.prepare (sr);
    burst.prepare (sr);
    greenhouse.prepare (sr);
    mist.setSampleRate (sr);
    pocket.setSampleRate (sr);

    for (auto& g : auxGain)  g.reset (sr, 0.02);
    for (auto& g : mainGain) g.reset (sr, 0.02);
    masterGain.reset (sr, 0.02);
    ghMix.reset (sr, 0.02);

    reset();
}

void EngineRack::reset()
{
    dew.reset();
    bloom.reset();
    breeze.reset();
    roots.reset();
    burst.reset();
    greenhouse.reset();
    mist.clear();
    pocket.clear();
    clock.reset();
    held.fill (false);
    prevGrowth = -1.0f;
    spaceInit = false;
    for (auto& g : auxGain)  g.setCurrentAndTargetValue (0.0f);
    for (auto& g : mainGain) g.setCurrentAndTargetValue (0.0f);
    masterGain.setCurrentAndTargetValue (1.0f);
    ghMix.setCurrentAndTargetValue (0.0f);
    viz.reset();
}

void EngineRack::publishState (const EngineState& s)
{
    const int idx = stateIdx.load (std::memory_order_relaxed);
    stateBuf[(size_t) (1 - idx)] = s;
    stateIdx.store (1 - idx, std::memory_order_release);
    stateVersion.fetch_add (1, std::memory_order_release);
}

void EngineRack::requestBurst()
{
    burstRequested.store (true);
}

MacroState EngineRack::computeMacro() const
{
    MacroState m;
    const float growth = params.getRawParameterValue (IDs::growth)->load() / 100.0f;
    const float storm = params.getRawParameterValue (IDs::weather_storm)->load() / 100.0f;
    const float humid = params.getRawParameterValue (IDs::weather_humid)->load() / 100.0f;
    m.growth = GrowthMapper::map (growth);
    m.storm = storm;
    m.humid = humid;
    m.dewReleaseMul = 0.5f + 1.5f * humid;
    m.humanizeMul = 1.0f + 2.0f * storm;
    m.breezeExtraDb = 6.0f * storm;
    m.burstProbExtra = 40.0f * storm;
    m.extraTripletProb = 0.5f * storm;
    return m;
}

void EngineRack::inferRootScale()
{
    int count = 0;
    int lowest = 128;
    for (int n = 0; n < 128; ++n)
        if (held[(size_t) n]) { ++count; if (n < lowest) lowest = n; }

    if (count == 0) return;

    const int root = lowest % 12;

    auto* rp = params.getParameter (IDs::root);
    if (rp != nullptr)
    {
        const float nv = (float) root / 11.0f;
        if (std::fabs (rp->getValue() - nv) > 1.0e-4f)
            rp->setValueNotifyingHost (nv);
    }

    if (count >= 2)
    {
        bool hasMajor = false, hasMinor = false;
        for (int n = 0; n < 128; ++n)
        {
            if (!held[(size_t) n]) continue;
            const int pc = ((n - root) % 12 + 12) % 12;
            if (pc == 3) hasMinor = true;
            if (pc == 4) hasMajor = true;
        }
        int scaleIdx = hasMajor ? 1 : 0;
        auto* sp = params.getParameter (IDs::scale);
        if (sp != nullptr)
        {
            const float nv = (float) scaleIdx / 4.0f;
            if (std::fabs (sp->getValue() - nv) > 1.0e-4f)
                sp->setValueNotifyingHost (nv);
        }
    }
}

void EngineRack::handleMidi (const juce::MidiBuffer& midi, const MacroState& macro)
{
    for (const auto& md : midi)
    {
        const auto msg = md.getMessage();
        if (msg.isNoteOn())
        {
            const int note = msg.getNoteNumber();
            if (note == 24) { if (macro.growth.burstArmed) burst.trigger(); }
            else held[(size_t) note] = true;
        }
        else if (msg.isNoteOff())
        {
            held[(size_t) msg.getNoteNumber()] = false;
        }
    }

    if (params.getRawParameterValue (IDs::follow_midi)->load() >= 0.5f)
        inferRootScale();
}

void EngineRack::process (float* mainOutL, float* mainOutR,
                          float* const* auxOutL, float* const* auxOutR,
                          const float* sideL, const float* sideR,
                          const juce::MidiBuffer& midi, juce::AudioPlayHead* playhead, int n)
{
    const double seconds = (double) n / sampleRate;
    clock.update (playhead, seconds);

    const int scaleIdx = (int) params.getRawParameterValue (IDs::scale)->load();
    const int root = (int) params.getRawParameterValue (IDs::root)->load();
    quantizer.set (scaleIdx, root);

    const MacroState macro = computeMacro();

    const float growthRaw = params.getRawParameterValue (IDs::growth)->load();
    if (prevGrowth >= 0.0f && growthRaw >= 66.0f && prevGrowth < 66.0f && macro.growth.burstArmed)
        burst.trigger();
    prevGrowth = growthRaw;

    if (burstRequested.exchange (false))
        burst.trigger();

    handleMidi (midi, macro);

    const int v = stateVersion.load (std::memory_order_acquire);
    if (v != lastStateVersion)
    {
        const EngineState& s = stateBuf[(size_t) stateIdx.load (std::memory_order_acquire)];
        dew.setPattern (s.dew);
        bloom.setChords (s.chordA, s.countA, s.chordB, s.countB);
        lastStateVersion = v;
    }

    EngineContext ctx;
    ctx.sampleRate = sampleRate;
    ctx.params = &params;
    ctx.scale = &quantizer;
    ctx.clock = &clock;
    ctx.viz = &viz;
    ctx.scaleIndex = scaleIdx;

    dew.process (dewL.data(), dewR.data(), n, ctx, macro);
    bloom.process (bloomL.data(), bloomR.data(), n, ctx, macro);
    breeze.process (breezeL.data(), breezeR.data(), n, ctx, macro);
    roots.process (rootsL.data(), rootsR.data(), n, ctx, macro);
    burst.process (burstL.data(), burstR.data(), n, ctx, macro);

    const bool dewOn = params.getRawParameterValue (IDs::dew_enable)->load() >= 0.5f;
    const bool bloomOn = params.getRawParameterValue (IDs::bloom_enable)->load() >= 0.5f;
    const bool breezeOn = params.getRawParameterValue (IDs::breeze_enable)->load() >= 0.5f;
    const bool rootsOn = params.getRawParameterValue (IDs::roots_enable)->load() >= 0.5f;
    const bool burstOn = params.getRawParameterValue (IDs::burst_enable)->load() >= 0.5f;

    const bool dewSolo = params.getRawParameterValue (IDs::dew_solo)->load() >= 0.5f;
    const bool bloomSolo = params.getRawParameterValue (IDs::bloom_solo)->load() >= 0.5f;
    const bool breezeSolo = params.getRawParameterValue (IDs::breeze_solo)->load() >= 0.5f;
    const bool rootsSolo = params.getRawParameterValue (IDs::roots_solo)->load() >= 0.5f;
    const bool burstSolo = params.getRawParameterValue (IDs::burst_solo)->load() >= 0.5f;
    const bool anySolo = dewSolo || bloomSolo || breezeSolo || rootsSolo || burstSolo;

    const float dewLevel = dbToGain (params.getRawParameterValue (IDs::dew_level)->load());
    const float bloomLevel = dbToGain (params.getRawParameterValue (IDs::bloom_level)->load());
    const float breezeLevel = dbToGain (params.getRawParameterValue (IDs::breeze_level)->load());
    const float rootsLevel = dbToGain (params.getRawParameterValue (IDs::roots_level)->load());
    const float burstLevel = dbToGain (params.getRawParameterValue (IDs::burst_level)->load());

    const float bloomGrowth = dbToGain (macro.growth.bloomGainDb);

    const float auxDew = dewLevel * macro.growth.dewGain * (dewOn ? 1.0f : 0.0f);
    const float auxBloom = bloomLevel * bloomGrowth * (bloomOn ? 1.0f : 0.0f);
    const float auxBreeze = breezeLevel * macro.growth.breezeGain * (breezeOn ? 1.0f : 0.0f);
    const float auxRoots = rootsLevel * macro.growth.rootsGain * (rootsOn ? 1.0f : 0.0f);
    const float auxBurst = burstLevel * (burstOn ? 1.0f : 0.0f);

    auxGain[0].setTargetValue (auxDew);
    auxGain[1].setTargetValue (auxBloom);
    auxGain[2].setTargetValue (auxBreeze);
    auxGain[3].setTargetValue (auxRoots);
    auxGain[4].setTargetValue (auxBurst);

    auto soloPass = [&] (bool s) { return (!anySolo || s) ? 1.0f : 0.0f; };
    mainGain[0].setTargetValue (auxDew * soloPass (dewSolo));
    mainGain[1].setTargetValue (auxBloom * soloPass (bloomSolo));
    mainGain[2].setTargetValue (auxBreeze * soloPass (breezeSolo));
    mainGain[3].setTargetValue (auxRoots * soloPass (rootsSolo));
    mainGain[4].setTargetValue (auxBurst * soloPass (burstSolo));

    masterGain.setTargetValue (dbToGain (params.getRawParameterValue (IDs::master_gain)->load()));

    const float humid = macro.humid;
    const float ghMixVal = juce::jlimit (0.0f, 100.0f, params.getRawParameterValue (IDs::gh_mix)->load() + 25.0f * humid);
    ghMix.setTargetValue (ghMixVal / 100.0f);

    const float mistCut = params.getRawParameterValue (IDs::mist_cutoff)->load() * (1.3f - 0.7f * humid);
    const float size = params.getRawParameterValue (IDs::gh_size)->load() * (0.6f + 1.0f * humid);
    const float damp = params.getRawParameterValue (IDs::gh_damp)->load();
    const float pred = params.getRawParameterValue (IDs::gh_predelay)->load();
    const float lowcut = params.getRawParameterValue (IDs::gh_lowcut)->load();
    const float pocketAmt = params.getRawParameterValue (IDs::pocket_amount)->load() / 100.0f;

    const float c = 0.2f;
    if (!spaceInit)
    {
        mistSm = mistCut * 1000.0f;
        sizeSm = size;
        dampSm = damp * 1000.0f;
        predelaySm = pred;
        lowcutSm = lowcut;
        pocketSm = pocketAmt;
        spaceInit = true;
    }
    else
    {
        mistSm += c * (mistCut * 1000.0f - mistSm);
        sizeSm += c * (size - sizeSm);
        dampSm += c * (damp * 1000.0f - dampSm);
        predelaySm += c * (pred - predelaySm);
        lowcutSm += c * (lowcut - lowcutSm);
        pocketSm += c * (pocketAmt - pocketSm);
    }

    mist.setCutoff (mistSm);
    greenhouse.setParams (sizeSm, dampSm, predelaySm, lowcutSm);
    pocket.setAmount (pocketSm);

    std::fill (sendL.begin(), sendL.end(), 0.0f);
    std::fill (sendR.begin(), sendR.end(), 0.0f);
    std::fill (dryL.begin(), dryL.end(), 0.0f);
    std::fill (dryR.begin(), dryR.end(), 0.0f);
    std::fill (duckL.begin(), duckL.end(), 0.0f);
    std::fill (duckR.begin(), duckR.end(), 0.0f);

    const float* sideLp = (sideL != nullptr) ? sideL : zero.data();
    const float* sideRp = (sideR != nullptr) ? sideR : zero.data();

    for (int i = 0; i < n; ++i)
    {
        const float dG = auxGain[0].getNextValue();
        const float bG = auxGain[1].getNextValue();
        const float zG = auxGain[2].getNextValue();
        const float rG = auxGain[3].getNextValue();
        const float uG = auxGain[4].getNextValue();

        const float dM = mainGain[0].getNextValue();
        const float bM = mainGain[1].getNextValue();
        const float zM = mainGain[2].getNextValue();
        const float rM = mainGain[3].getNextValue();
        const float uM = mainGain[4].getNextValue();

        if (auxOutL[0] != nullptr) { auxOutL[0][i] = dewL[(size_t) i] * dG; auxOutR[0][i] = dewR[(size_t) i] * dG; }
        if (auxOutL[1] != nullptr) { auxOutL[1][i] = bloomL[(size_t) i] * bG; auxOutR[1][i] = bloomR[(size_t) i] * bG; }
        if (auxOutL[2] != nullptr) { auxOutL[2][i] = breezeL[(size_t) i] * zG; auxOutR[2][i] = breezeR[(size_t) i] * zG; }
        if (auxOutL[3] != nullptr) { auxOutL[3][i] = rootsL[(size_t) i] * rG; auxOutR[3][i] = rootsR[(size_t) i] * rG; }
        if (auxOutL[4] != nullptr) { auxOutL[4][i] = burstL[(size_t) i] * uG; auxOutR[4][i] = burstR[(size_t) i] * uG; }

        sendL[(size_t) i] = dewL[(size_t) i] * dM * 0.30f + bloomL[(size_t) i] * bM * 0.50f
                          + breezeL[(size_t) i] * zM * 0.20f + burstL[(size_t) i] * uM * 0.40f;
        sendR[(size_t) i] = dewR[(size_t) i] * dM * 0.30f + bloomR[(size_t) i] * bM * 0.50f
                          + breezeR[(size_t) i] * zM * 0.20f + burstR[(size_t) i] * uM * 0.40f;

        duckL[(size_t) i] = dewL[(size_t) i] * dM + breezeL[(size_t) i] * zM;
        duckR[(size_t) i] = dewR[(size_t) i] * dM + breezeR[(size_t) i] * zM;

        dryL[(size_t) i] = bloomL[(size_t) i] * bM + rootsL[(size_t) i] * rM + burstL[(size_t) i] * uM;
        dryR[(size_t) i] = bloomR[(size_t) i] * bM + rootsR[(size_t) i] * rM + burstR[(size_t) i] * uM;
    }

    greenhouse.process (sendL.data(), sendR.data(), wetL.data(), wetR.data(), n);
    pocket.process (sideLp, sideRp, duckL.data(), duckR.data(), n);

    for (int i = 0; i < n; ++i)
    {
        const float g = ghMix.getNextValue();
        mainL[(size_t) i] = duckL[(size_t) i] + dryL[(size_t) i] + wetL[(size_t) i] * g;
        mainR[(size_t) i] = duckR[(size_t) i] + dryR[(size_t) i] + wetR[(size_t) i] * g;
    }

    mist.process (mainL.data(), mainR.data(), n);

    for (int i = 0; i < n; ++i)
    {
        const float mg = masterGain.getNextValue();
        mainL[(size_t) i] = std::tanh (mainL[(size_t) i] * mg) * 0.8912509f;
        mainR[(size_t) i] = std::tanh (mainR[(size_t) i] * mg) * 0.8912509f;
    }

    if (mainOutL != nullptr) std::memcpy (mainOutL, mainL.data(), (size_t) n * sizeof (float));
    if (mainOutR != nullptr) std::memcpy (mainOutR, mainR.data(), (size_t) n * sizeof (float));

    auto rms = [] (const std::vector<float>& l, const std::vector<float>& r, int count)
    {
        double s = 0.0;
        for (int i = 0; i < count; ++i) s += (double) l[(size_t) i] * l[(size_t) i] + (double) r[(size_t) i] * r[(size_t) i];
        return (float) std::sqrt (s / (2.0 * count));
    };

    viz.levels[0].store (rms (dewL, dewR, n) * auxGain[0].getCurrentValue() * 6.0f);
    viz.levels[1].store (rms (bloomL, bloomR, n) * auxGain[1].getCurrentValue() * 6.0f);
    viz.levels[2].store (rms (breezeL, breezeR, n) * auxGain[2].getCurrentValue() * 6.0f);
    viz.levels[3].store (rms (rootsL, rootsR, n) * auxGain[3].getCurrentValue() * 6.0f);
    viz.levels[4].store (rms (burstL, burstR, n) * auxGain[4].getCurrentValue() * 6.0f);
    viz.levels[5].store (rms (mainL, mainR, n) * 1.5f);
}
}
