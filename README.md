# Petrichor

A generative VST3 synthesizer of natural phenomena by **Reflexed**. Five engines — **DEW**, **BLOOM**, **BREEZE**, **ROOTS**, **BURST** — play a scene that adapts to the key and tempo of the track, and fits almost any music. All default values come from a spectral analysis of a real audio sample.

## Features

- 5 engines, each with Enable, Solo and Level.
- **Scale Lock** (Root C–B + Scale) quantizes every pitch to the chosen note set.
- **Host sync** via `AudioPlayHead`; free-runs at 70 BPM without a host.
- 16-step **DEW** pattern with a mouse editor, saved inside the preset.
- Macros: **GROWTH**, **WEATHER Storm**, **WEATHER Humid**.
- Factory + user presets (save, load, rename, delete, prev/next).
- Sidechain input (**Pocket**) and multi-out (Main + 5 stereo buses, one per engine).
- **Follow MIDI**: a held chord sets Root and Scale; note **C1** triggers BURST.

## Building

Requirements: CMake ≥ 3.24, a C++20 compiler, and network access for the first
configure (JUCE 8 is fetched via `FetchContent`, pinned in `CMakeLists.txt`).

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

Artifacts are written under `build/Petrichor_artefacts/` (VST3 and a Standalone
for a smoke test). A GitHub Actions workflow (`.github/workflows/build.yml`)
builds on Windows, macOS (universal) and Linux, runs pluginval at strictness 5,
and packages `Petrichor-v<VERSION>-<os>.zip`; on a `v*` tag it publishes a
GitHub Release. The version comes from the `VERSION` file.

## Inter font

Drop `Inter-Regular.ttf` and `Inter-Medium.ttf` into `Resources/` to embed them;
if they are absent the plugin falls back to the system sans at runtime. The
CMake build detects the files automatically.

## Notes / closest-solution deviations

- Discrete parameters (Root, Scale, Octave, Rate, Bars, Enable/Solo) are applied
  as steps; continuous parameters are smoothed over 20 ms. This matches the
  "every parameter is smoothed" rule for everything where smoothing is audible
  and click-free.
- The Wood material uses `{1, 4.0, 10.0, 13.0}` so the modal bank keeps its four
  resonators; Glass and Metal use the exact ratios from the spec.
- The FDN uses 8 mutually prime delay lengths scaled to `gh_size` with a
  Hadamard feedback matrix; two lines are modulated (±0.3 ms at 0.15 Hz).
- BURST C1 uses the JUCE/MIDI convention C4 = 60, so C1 = MIDI note 24.
- `Weather Storm` and `Weather Humid` are applied proportionally to the macro
  value (e.g. Storm 100 = full "+50% triplet steps, humanize ×3, BREEZE +6 dB,
  burst_prob +40 points").
