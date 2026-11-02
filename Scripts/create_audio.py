"""Author the project's original, quiet campus loop and short feedback sounds.

No recordings, samples, downloaded melodies or third-party audio are used.
Outputs are PCM WAV masters, preserved alongside the source for reproducibility.
"""
from pathlib import Path
from array import array
import math
import os
import random
import wave

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "SourceAssets/Audio"
OUT.mkdir(parents=True, exist_ok=True)
RATE = 24000
TAU = 2 * math.pi

def note(midi):
    return 440 * 2 ** ((midi - 69) / 12)

def save(name, values):
    peak = max(max(abs(x) for x in values), .001)
    gain = min(1., .65 / peak)
    pcm = array("h", (int(max(-1, min(1, x * gain)) * 32767) for x in values))
    temp = ROOT / "Runtime.nosync" / ("audio-build-" + name + ".wav")
    with wave.open(str(temp), "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(RATE)
        wav.writeframes(pcm.tobytes())
    os.replace(temp, OUT / (name + ".wav"))

# An original 16-bar, 80 BPM cue: soft electric keys, wooden plucks,
# a restrained bass and brushed percussion. No sampled instruments or songs.
duration = 48.
beat = .75
music = [0.] * int(duration * RATE)
chords = [(50, 57, 61, 66), (47, 54, 57, 62), (43, 54, 59, 62), (45, 52, 59, 61)] * 2
motifs = [(62, 66, 69, 73, 71, 69, 66, 64), (62, 66, 69, 74, 73, 69, 66, 62),
          (67, 71, 74, 78, 76, 74, 71, 69), (69, 73, 76, 71, 69, 66, 64, 61)]
for bar, chord in enumerate(chords):
    for i in range(int(6 * RATE)):
        t = i / RATE
        envelope = math.sin(math.pi * t / 6) ** .55
        pad = sum(math.sin(TAU * note(n) * t) + .13 * math.sin(TAU * note(n) * 2 * t) for n in chord) / 4
        bass_local = t % (beat * 2)
        bass = math.sin(TAU * note(chord[0] - 12) * t) * (1 - math.exp(-bass_local * 60)) * math.exp(-bass_local * 2.6)
        music[bar * 6 * RATE + i] = envelope * (.16 * pad + .1 * bass)
    for step, pitch in enumerate(motifs[bar % 4]):
        if step == 5 and bar % 2 == 0:
            continue
        onset = bar * 6 + step * beat
        for i in range(int(.65 * RATE)):
            t = i / RATE
            # The inharmonic, fast-decaying partials suggest a wooden marimba.
            pluck = math.sin(TAU * note(pitch) * t) * math.exp(-t * 5.2)
            pluck += .23 * math.sin(TAU * note(pitch) * 2.78 * t) * math.exp(-t * 15)
            pluck += .06 * math.sin(TAU * note(pitch) * 5.4 * t) * math.exp(-t * 24)
            idx = int(onset * RATE) + i
            if idx < len(music):
                music[idx] += .16 * (1 - math.exp(-t * 160)) * pluck
random.seed(43)
for pulse in range(128):
    onset = pulse * beat / 2
    for i in range(int(.085 * RATE)):
        t = i / RATE
        idx = int(onset * RATE) + i
        if idx < len(music):
            brush = random.uniform(-1, 1) * math.exp(-t * 65) * .018
            soft_beat = .035 * math.sin(TAU * 100 * t) * math.exp(-t * 38) if pulse % 4 == 0 else 0
            music[idx] += brush + soft_beat
# Taper only the outer edges so a loop never clicks.
for i in range(int(.08 * RATE)):
    f = i / (.08 * RATE)
    music[i] *= f
    music[-i-1] *= f
save("CampusLoop", music)

def phrase(name, notes, seconds, level=.28):
    out = []
    for i in range(int(seconds * RATE)):
        t = i / RATE
        index = min(len(notes) - 1, int(t / seconds * len(notes)))
        local = t % (seconds / len(notes))
        envelope = (1 - math.exp(-local * 110)) * math.exp(-local * 7) * min(1., (seconds - t) * 30)
        out.append(level * envelope * (math.sin(TAU * note(notes[index]) * t) + .12 * math.sin(TAU * note(notes[index]) * 2 * t)))
    save(name, out)

phrase("Pickup", [72, 76], .4)
phrase("Finale", [62, 66, 69, 74, 78, 81], 2.4, .24)
random.seed(43)
save("Paper", [random.uniform(-1, 1) * .09 * math.sin(math.pi * i / (.22 * RATE)) ** 2 for i in range(int(.22 * RATE))])
save("Stamp", [(.24 * math.sin(TAU * 170 * i / RATE) + .08 * random.uniform(-1, 1)) * math.exp(-i / RATE * 26) for i in range(int(.3 * RATE))])
save("Borrow", [.2 * math.sin(TAU * (520 * (i / RATE) - 180 * (i / RATE) ** 2)) * math.sin(math.pi * i / (.4 * RATE)) ** 2 for i in range(int(.4 * RATE))])
save("Footstep", [(.2 * math.sin(TAU * 110 * i / RATE) + .035 * random.uniform(-1, 1)) * math.exp(-i / RATE * 38) for i in range(int(.13 * RATE))])
print("Created seven original WAV masters in SourceAssets/Audio.")
