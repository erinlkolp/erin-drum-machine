# Erin 808 Drum Machine — VST3 Plugin Design Spec

## Overview

A production-quality VST3 instrument plugin that emulates the classic Roland TR-808 drum machine. Sample-based, MIDI-triggered, with a skeuomorphic GUI styled after the original hardware. Built with C++/JUCE using CMake, targeting macOS, Windows, and Linux.

## Sound Engine

### Pads

8 pads, each mapped to a fixed MIDI note (General MIDI drum map):

| Pad        | MIDI Note | Note Number |
|------------|-----------|-------------|
| Kick       | C1        | 36          |
| Snare      | D1        | 38          |
| Clap       | D#1       | 39          |
| Closed HH  | F#1       | 42          |
| Open HH    | A#1       | 46          |
| Rim Shot   | C#1       | 37          |
| Cowbell    | G#1       | 56          |
| Tom        | G1        | 43          |

### Sample Playback

- Each pad backed by a `.wav` sample embedded as `BinaryData` at compile time
- Samples loaded into `juce::AudioBuffer` objects on plugin initialization
- Playback via `juce::Synthesiser` with `juce::SamplerVoice` instances
- 4 voices per pad to allow overlapping re-triggers

### Choke Group

Open Hi-Hat and Closed Hi-Hat form a choke group: triggering one immediately silences any playing voices of the other. This matches the behavior of the original TR-808 hardware where the open and closed hi-hat share a single cymbal mechanism.

### Per-Pad Controls

- **Volume:** 0.0 to 1.0, default 0.8
- **Pan:** -1.0 (hard left) to 1.0 (hard right), default 0.0 (center)

## GUI

### Window

Fixed size: 800x400 pixels. Not resizable.

### Layout

Horizontal strip layout mimicking the TR-808 faceplate:

- **Header bar** at top: "TR-808 DRUM MACHINE" title text in retro uppercase sans-serif
- **Pad strip** below: 8 pad columns arranged left to right, each containing:
  - Volume knob (top)
  - Pan knob (middle)
  - Colored pad button (bottom)
  - Pad name label (below pad)

### Visual Style — TR-808 Inspired

- **Background:** Dark charcoal/black panel with subtle horizontal striping
- **Knobs:** Rotary style, drawn as filled arcs with a position indicator line
- **Pads:** Colored rectangles matching the original 808 color coding:
  - Kick, Tom: orange
  - Snare, Rim Shot: white
  - Clap: pink/mauve
  - Hi-Hats: yellow
  - Cowbell: red
- **Typography:** Bold, uppercase sans-serif
- **Hit feedback:** Pads briefly flash/brighten when triggered via MIDI

### Knob Interaction

- Click-drag vertically to adjust value
- Backed by `juce::AudioParameterFloat` for DAW automation

### Pad Interaction

- Clickable with mouse to audition (sends internal MIDI note-on)
- Lights up on external MIDI trigger

### Rendering

All GUI elements drawn programmatically using JUCE's `Graphics` API. No image assets. The TR-808's visual style (colored rectangles, simple shapes, bold text) is well-suited to code-based rendering.

## State Management & DAW Integration

### Parameters

16 parameters total, registered via `juce::AudioProcessorValueTreeState` (APVTS):

```
kick_volume, kick_pan
snare_volume, snare_pan
clap_volume, clap_pan
closed_hh_volume, closed_hh_pan
open_hh_volume, open_hh_pan
rimshot_volume, rimshot_pan
cowbell_volume, cowbell_pan
tom_volume, tom_pan
```

### Features Provided by APVTS

- **DAW automation:** Parameters appear in DAW automation lanes
- **State persistence:** Automatic serialization/deserialization for DAW project save/recall
- **GUI binding:** Knobs attached via `SliderAttachment` for thread-safe UI/audio sync

### No Built-in Preset System

Plugin state is saved and recalled entirely through the DAW's own project/preset mechanism. No custom preset browser.

## Sample Sourcing

Samples will be synthesized (not sourced from sample packs) to ensure clean licensing. The original TR-808 sounds are analog synthesis, making them reproducible:

- **Kick:** Decaying sine wave with pitch sweep downward
- **Snare:** Sine oscillator + noise burst with decay envelope
- **Closed HH:** Filtered noise with short decay
- **Open HH:** Filtered noise with longer decay
- **Clap:** Short noise bursts with reverb/early reflections
- **Cowbell:** Two square wave oscillators with bandpass filter and decay
- **Rim Shot:** Short noise burst with resonant character
- **Tom:** Decaying sine wave at higher pitch than kick

Samples prepared as `.wav` files (44.1kHz, 24-bit, mono) and placed in the `samples/` directory. Embedded into the plugin binary via `juce_add_binary_data()`.

## Build System & Project Structure

### Toolchain

- **Framework:** JUCE (as git submodule)
- **Build system:** CMake with `juce_cmake_utils`
- **Dependencies:** JUCE only (no other third-party libraries)

### Project Structure

```
erin-drum-machine/
├── CMakeLists.txt
├── JUCE/                       # Git submodule
├── samples/
│   ├── kick.wav
│   ├── snare.wav
│   ├── clap.wav
│   ├── closed_hh.wav
│   ├── open_hh.wav
│   ├── rimshot.wav
│   ├── cowbell.wav
│   └── tom.wav
├── source/
│   ├── PluginProcessor.h
│   ├── PluginProcessor.cpp     # DrumMachineProcessor
│   ├── PluginEditor.h
│   ├── PluginEditor.cpp        # DrumMachineEditor (main GUI)
│   ├── PadComponent.h
│   ├── PadComponent.cpp        # Single pad widget (knobs + button + label)
│   ├── KnobLookAndFeel.h
│   ├── KnobLookAndFeel.cpp     # Custom rotary knob rendering
│   └── DrumPad.h               # Pad data struct
├── resources/
└── docs/
    └── superpowers/
        └── specs/
            └── 2026-03-28-808-drum-machine-design.md
```

### Build Targets

- **Erin808 (VST3):** The plugin binary
- **Erin808 (Standalone):** Standalone application for testing without a DAW

### Platform Generators

- macOS: `cmake -G Xcode`
- Windows: `cmake -G "Visual Studio 17 2022"`
- Linux: `cmake -G Ninja` or Unix Makefiles

## Signal Flow

```
MIDI Note-On
  → Processor identifies target pad by note number
  → SamplerVoice triggered (with choke group check for hi-hats)
  → Sample playback
  → Per-pad Volume applied
  → Per-pad Pan applied
  → Stereo mix to DAW output
```

## Scope Exclusions

The following are explicitly out of scope:

- Built-in step sequencer
- Per-pad pitch, decay, or filter controls
- User-loadable samples
- Custom preset browser
- Resizable GUI
- Effects (reverb, delay, compression, etc.)
- Multiple output routing
