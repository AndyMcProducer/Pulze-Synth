# Pulze (JUCE VST3)

`Pulze` is a 4-oscillator synth plugin scaffold built for your P2PDAW flow, ready to be pulled into the main Tracktion Engine project later.

## Current DSP Features

- 4 oscillators per voice with independent `Wave`, `Level`, and `Tune (cents)` controls.
- Free-running oscillator phase with optional `Retrigger`.
- Per-oscillator analog-style `Drift (cents)` random walk.
- Pre-filter `Drive (dB)` using soft saturation (`tanh`).
- Per-voice low-pass filter (`Cutoff`, `Resonance`).
- Automatic bass compensation as resonance increases, with `Bass Comp` amount control.
- Exponential ADSR behavior (`Attack`, `Decay`, `Sustain`, `Release`).
- Unison mode (`1/2/4/8`) with `Unison Detune` and `Stereo Spread`.
- `Mono Width` control to collapse stereo unison toward mono-compatible bass patches.
- Supersaw-style non-linear detune distribution for thicker outer voices.
- Built-in post-synth stereo chorus (`Rate`, `Depth`, `Mix`).
- Workstation FX macros: `FX Space` with integrated `Delay` (`Time`, `Feedback`, `Mix`) and `Reverb` (`Size`, `Damping`, `Mix`).
- Host-sync timing options: `Delay Sync` + note divisions, and `Chorus Sync` + cycle divisions.
- Synced arp with performance controls: `Arp Division`, `Arp Gate`, `Arp Mode` (`Up/Down/UpDown/Random`), `Arp Latch`, and `Arp Octaves` (`1-3`).
- Custom neon-blue computerized UI (replacing the generic JUCE editor).
- Animated neon arp step indicator in the header (shows step pulse and host BPM).
- Arp indicator is mode-reactive with distinct color/motion for `Up`, `Down`, `UpDown`, and `Random`.
- Neon spark particle bursts on each arp step trigger, color-matched to arp mode.
- Subtle panel glow breathing synced to host BPM while arp runs (with ambient idle pulse).
- Startup neon boot-sequence overlay with scanlines and init progress.
- Polyphonic synth engine (16 voices).
- Full plugin state save/restore via `AudioProcessorValueTreeState`.

## Build (Windows, CMake, VST3)

1. Put JUCE in a folder named `JUCE` at the project root:
   - `E:\Web stuff\P2PDAW\4osc synth\JUCE\CMakeLists.txt`
2. Configure:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022"
```

3. Build:

```powershell
cmake --build build --config Release
```

4. The VST3 target is generated from the `FourOscPro` plugin target.

## Next Recommended Upgrades

- Add HQ mode with optional oversampling around the drive stage.
