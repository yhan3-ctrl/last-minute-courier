"""Two original, synchronized 48-second layers for the existing campus theme.

Synthesized from oscillators; no recordings, samples or third-party melodies.
Both layers share CampusLoop's harmony, tempo and exact sample count.
"""
from array import array
from pathlib import Path
import math
import os
import wave

root = Path(__file__).resolve().parents[1]
out = root / "SourceAssets/Audio"
out.mkdir(parents=True, exist_ok=True)
rate = 24000
length = 48 * rate
tau = 2 * math.pi
chords = [(50, 57, 61, 66), (47, 54, 57, 62), (43, 54, 59, 62), (45, 52, 59, 61)] * 2

def hz(midi):
    return 440 * 2 ** ((midi - 69) / 12)

def save(name, values):
    assert len(values) == length
    edge = int(rate * .08)
    for i in range(edge):
        values[i] *= i / edge
        values[-i-1] *= i / edge
    assert max(abs(x) for x in values) < .5, "Keep generous mixing headroom"
    pcm = array("h", (int(x * 32767) for x in values))
    temp = root / "Runtime.nosync" / ("audio-build-" + name + ".wav")
    with wave.open(str(temp), "wb") as stream:
        stream.setnchannels(1)
        stream.setsampwidth(2)
        stream.setframerate(rate)
        stream.writeframes(pcm.tobytes())
    os.replace(temp, out / (name + ".wav"))

strings = [0.] * length
bells = [0.] * length
for bar, chord in enumerate(chords):
    frequencies = [hz(n + 12) for n in chord[1:]]
    for i in range(6 * rate):
        t = i / rate
        envelope = math.sin(math.pi * t / 6) ** .8
        pad = sum(math.sin(tau * f * t + .005 * math.sin(tau * .3 * t))
                  + .15 * math.sin(tau * f * 2 * t) for f in frequencies) / 3
        strings[bar * 6 * rate + i] = .11 * envelope * pad
    # Sparse answers leave room for the original marimba melody and feedback sounds.
    for onset, pitch in [(1.5, chord[2] + 24), (4.5, chord[3] + 12)]:
        for i in range(int(1.4 * rate)):
            t = i / rate
            attack = 1 - math.exp(-t * 65)
            bell = math.sin(tau * hz(pitch) * t) * math.exp(-t * 3.5)
            bell += .16 * math.sin(tau * hz(pitch) * 2.01 * t) * math.exp(-t * 6)
            bells[int((bar * 6 + onset) * rate) + i] += .09 * attack * bell

save("CampusStrings", strings)
save("CampusBells", bells)
print("Created two original, phase-aligned campus music layers.")
