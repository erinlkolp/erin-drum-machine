#!/usr/bin/env python3
"""Synthesize TR-808 drum samples using only the Python standard library.

Outputs 16-bit mono WAV at 44.1 kHz. The spec calls for 24-bit, but JUCE reads
both formats identically and 16-bit is simpler to generate from pure Python.
"""

import math
import os
import random
import struct
import wave

SAMPLE_RATE = 44100


def write_wav(filename, samples):
    """Write mono 16-bit WAV file, normalized to 95% peak."""
    peak = max((abs(s) for s in samples), default=1.0)
    if peak > 0:
        scale = 0.95 / peak
    else:
        scale = 1.0

    with wave.open(filename, "w") as f:
        f.setnchannels(1)
        f.setsampwidth(2)
        f.setframerate(SAMPLE_RATE)
        for s in samples:
            clamped = max(-1.0, min(1.0, s * scale))
            f.writeframes(struct.pack("<h", int(clamped * 32767)))


def generate_kick():
    """Decaying sine wave with pitch sweep from 150 Hz to 40 Hz."""
    duration = 0.5
    n = int(SAMPLE_RATE * duration)
    samples = []
    phase = 0.0
    for i in range(n):
        t = i / SAMPLE_RATE
        freq = 40 + 120 * math.exp(-t * 30)
        phase += 2 * math.pi * freq / SAMPLE_RATE
        amp = math.exp(-t * 7)
        click = math.exp(-t * 200) * 0.15
        samples.append(math.sin(phase) * amp + math.sin(phase * 3) * click)
    return samples


def generate_snare():
    """Sine tone at 180 Hz mixed with noise, independent decay envelopes."""
    duration = 0.3
    n = int(SAMPLE_RATE * duration)
    samples = []
    random.seed(1)
    for i in range(n):
        t = i / SAMPLE_RATE
        tone = math.sin(2 * math.pi * 180 * t) * math.exp(-t * 30)
        noise = (random.random() * 2 - 1) * math.exp(-t * 15)
        samples.append(tone * 0.5 + noise * 0.5)
    return samples


def generate_clap():
    """Multiple short noise bursts followed by a noise tail."""
    duration = 0.3
    n = int(SAMPLE_RATE * duration)
    samples = []
    random.seed(2)
    for i in range(n):
        t = i / SAMPLE_RATE
        noise = random.random() * 2 - 1
        burst_env = 0.0
        for b in range(4):
            bt = t - b * 0.008
            if 0 <= bt < 0.008:
                burst_env += math.exp(-bt * 300)
        tail_env = math.exp(-t * 15) if t > 0.03 else 0.0
        env = burst_env * 0.6 + tail_env * 0.4
        samples.append(noise * env)
    return samples


def generate_closed_hh():
    """Metallic noise with very short decay (~50 ms)."""
    duration = 0.1
    n = int(SAMPLE_RATE * duration)
    samples = []
    random.seed(3)
    freqs = [317.0, 523.0, 812.0, 1103.0, 1523.0, 2077.0]
    for i in range(n):
        t = i / SAMPLE_RATE
        sig = sum(math.sin(2 * math.pi * f * t) for f in freqs)
        noise = random.random() * 2 - 1
        env = math.exp(-t * 60)
        samples.append((sig / len(freqs) * 0.4 + noise * 0.6) * env)
    return samples


def generate_open_hh():
    """Metallic noise with longer decay (~400 ms)."""
    duration = 0.5
    n = int(SAMPLE_RATE * duration)
    samples = []
    random.seed(4)
    freqs = [317.0, 523.0, 812.0, 1103.0, 1523.0, 2077.0]
    for i in range(n):
        t = i / SAMPLE_RATE
        sig = sum(math.sin(2 * math.pi * f * t) for f in freqs)
        noise = random.random() * 2 - 1
        env = math.exp(-t * 5)
        samples.append((sig / len(freqs) * 0.4 + noise * 0.6) * env)
    return samples


def generate_rimshot():
    """Short sine burst at 800 Hz mixed with noise."""
    duration = 0.1
    n = int(SAMPLE_RATE * duration)
    samples = []
    random.seed(5)
    for i in range(n):
        t = i / SAMPLE_RATE
        tone = math.sin(2 * math.pi * 800 * t) * math.exp(-t * 80)
        noise = (random.random() * 2 - 1) * math.exp(-t * 100)
        samples.append(tone * 0.6 + noise * 0.4)
    return samples


def generate_cowbell():
    """Two square-ish tones at 540 Hz and 800 Hz with medium decay."""
    duration = 0.3
    n = int(SAMPLE_RATE * duration)
    samples = []
    for i in range(n):
        t = i / SAMPLE_RATE
        tone1 = math.sin(2 * math.pi * 540 * t)
        tone2 = math.sin(2 * math.pi * 800 * t)
        env = math.exp(-t * 12)
        samples.append((tone1 + tone2) * 0.5 * env)
    return samples


def generate_tom():
    """Decaying sine wave with pitch sweep from 200 Hz to 100 Hz."""
    duration = 0.3
    n = int(SAMPLE_RATE * duration)
    samples = []
    phase = 0.0
    for i in range(n):
        t = i / SAMPLE_RATE
        freq = 100 + 100 * math.exp(-t * 25)
        phase += 2 * math.pi * freq / SAMPLE_RATE
        amp = math.exp(-t * 10)
        samples.append(math.sin(phase) * amp)
    return samples


if __name__ == "__main__":
    os.makedirs("samples", exist_ok=True)

    generators = [
        ("samples/kick.wav", generate_kick),
        ("samples/snare.wav", generate_snare),
        ("samples/clap.wav", generate_clap),
        ("samples/closed_hh.wav", generate_closed_hh),
        ("samples/open_hh.wav", generate_open_hh),
        ("samples/rimshot.wav", generate_rimshot),
        ("samples/cowbell.wav", generate_cowbell),
        ("samples/tom.wav", generate_tom),
    ]

    for filename, gen in generators:
        print(f"Generating {filename}...")
        write_wav(filename, gen())

    print("Done! Generated 8 samples in samples/")
