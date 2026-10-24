#!/usr/bin/env python3
"""Build the small UI sound files in iphone-duo-3ds-prototype/assets/audio/.

Sources are the iOS 27.0 simulator UISounds (pass the directory as argv[1]); the unlock sound is
synthesized because iOS has no unlock sound. Each file is peak-normalized to the level it had in the
iPhone Duo demo video mix (key clicks measured after a 1500 Hz high-pass), shifted down so the loudest
sound is no louder than the iOS original. Output: 16-bit mono WAV, decodable by every browser including iOS Safari.

Usage: python3 -I tools/make-ui-sounds.py <UISounds dir> <iOS音效.py dir>
"""
import subprocess
import sys
import tempfile
from math import gcd
from pathlib import Path

import numpy as np
import soundfile as sf
from scipy.signal import butter, resample_poly, sosfilt

ui, synth_dir = Path(sys.argv[1]), Path(sys.argv[2])
sys.path.insert(0, str(synth_dir))
import importlib.util
spec = importlib.util.spec_from_file_location('ios', synth_dir / 'iOS音效.py')
ios = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ios)

out_dir = Path(__file__).resolve().parent.parent / 'iphone-duo-3ds-prototype' / 'assets' / 'audio'
out_dir.mkdir(parents=True, exist_ok=True)
HP = butter(4, 1500, btype='highpass', fs=48000, output='sos')
# The video mix peaks 5.5 dB hotter than the raw iOS key click (-2.9 vs -8.4 dBFS full-band), so shift the whole set
# down to keep the relative balance of the video but never exceed the iOS original.
OFFSET_DB = -5.5

# name: (source, target peak dBFS from the video mix, output sample rate, key click measured after high-pass)
SOUNDS = {
    'tap': ('key_press_click', -6.6, 44100, True),
    'tock': ('Tock', -8.6, 44100, True),
    'tink': ('Tink', -12.6, 44100, True),
    'lock': ('lock', -8.6, 44100, True),
    'unlock': (None, -7.6, 44100, True),
    'open': ('navigation_push', -9.6, 24000, False),
    'back': ('navigation_pop', -10.6, 24000, False),
    'drop': ('focus_change_app_icon', -10.6, 24000, False),
}

for name, (src, target, sr, clicky) in SOUNDS.items():
    if src is None:
        x = ios.合成('解锁', np.random.default_rng(7)).astype(float)
    else:
        x = ios.read_ref(ui, src)
    meas = np.abs(sosfilt(HP, x)).max() if clicky else np.abs(x).max()
    x = x * 10 ** ((target + OFFSET_DB) / 20) / meas
    g = gcd(sr, 48000)
    y = resample_poly(x, sr // g, 48000 // g)
    y = y[: int(np.nonzero(np.abs(y) > 1e-4)[0][-1]) + 1]  # trim trailing silence
    assert np.abs(y).max() < 0.99, name
    path = out_dir / f'{name}.wav'
    sf.write(path, y, sr, subtype='PCM_16')
    print(f'{name}: {len(y) / sr * 1000:.0f} ms, peak {20 * np.log10(np.abs(y).max()):.1f} dBFS, {path.stat().st_size} bytes')
