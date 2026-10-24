"""End-to-end check of the iOS UI sounds. Plays nothing aloud: AudioContext is replaced by a recorder.
usage: ui_sound_check.py <base-url> <out-dir>   (exit 0 = all passed)
"""
import glob, json, os, sys
from playwright.sync_api import sync_playwright

base, out = sys.argv[1], sys.argv[2]
os.makedirs(out, exist_ok=True)
EXE = glob.glob(os.path.expanduser('~/Library/Caches/ms-playwright/chromium_headless_shell-1243/*/chrome-headless-shell'))[0]
FAKE = """
window.__plays = [];
window.__ac = class {
  constructor() { this.state = 'suspended'; this.currentTime = 0; this.destination = {}; this.off = new OfflineAudioContext(1, 44100, 44100); }
  resume() { this.state = 'running'; return Promise.resolve(); }
  decodeAudioData(d, ok, fail) { return this.off.decodeAudioData(d).then(ok, fail); }
  createBufferSource() { const s = { connect: (n) => n || s, start: (t) => window.__plays.push({ name: window.__byLength[s.buffer.length], at: performance.now(), delay: t }) }; return s; }
  createGain() { return { gain: {}, connect: (n) => n }; }
};
window.AudioContext = window.__ac;
window.__byLength = {};
"""
results = []
def check(name, ok, detail=''):
    results.append(ok); print(('PASS ' if ok else 'FAIL ') + name + (f'  ({detail})' if detail else ''))

with sync_playwright() as p:
    b = p.chromium.launch(executable_path=EXE)
    pg = b.new_context(viewport={'width': 1200, 'height': 900}, locale='en-US').new_page()
    wavs = []
    pg.on('response', lambda r: wavs.append((r.url.rsplit('/', 1)[-1], r.status)) if r.url.endswith('.wav') else None)
    pg.add_init_script(FAKE)
    pg.goto(base + ('&' if '?' in base else '?') + 'webgl=0'); pg.wait_for_timeout(2500)
    check('no audio file is fetched before the first touch', not wavs, str(wavs))

    names = lambda: [x['name'] for x in pg.evaluate('window.__plays')]
    def reset(): pg.evaluate('window.__plays.length=0')
    # first touch loads the files
    pg.click('[data-hw="y"]'); pg.wait_for_timeout(1500)
    check('all 8 sound files load after the first touch with HTTP 200', len(wavs) == 8 and all(s == 200 for _, s in wavs), str(wavs))
    # name each decoded buffer by its sample count, which is distinct per file
    sizes = pg.evaluate("Promise.all(['tap','tock','tink','lock','unlock','open','back','drop'].map(n => fetch('assets/audio/'+n+'.wav').then(r => r.arrayBuffer()).then(async a => { const b = await new OfflineAudioContext(1, 44100, 44100).decodeAudioData(a.slice(0)); window.__byLength[b.length] = n; return [n, a.byteLength]; })))")
    check('total audio size under 200 KB', sum(s for _, s in sizes) < 200_000, f'{sum(s for _, s in sizes)} bytes')

    def expect(label, action, want):
        reset(); action(); pg.wait_for_timeout(500)
        got = names()
        check(label, got == want, f'got {got}, want {want}')

    app = pg.evaluate("[...document.querySelectorAll('#bottom [data-app]')].find(b => officialEntry(b.dataset.app)).dataset.app")
    expect('hardware key (Y) plays the key click', lambda: pg.click('[data-hw="y"]'), ['tap'])
    expect('D-pad plays the key click', lambda: pg.click('[data-dir="1"]'), ['tap'])
    expect('tapping an app plays click then open', lambda: pg.click(f'#bottom [data-app="{app}"]'), ['tap', 'open'])
    expect('B key inside an app plays click then back', lambda: pg.click('[data-hw="b"]'), ['tap', 'back'])
    expect('B key on the home screen plays only the click', lambda: pg.click('[data-hw="b"]'), ['tap'])
    pg.evaluate("state.sound=true")
    # lock button location: find a [data-hw="lock"]
    lock = '[data-hw="lock"]'
    expect('lock key plays the lock sound', lambda: pg.click(lock), ['lock'])
    expect('lock key again plays the unlock sound', lambda: pg.click(lock), ['unlock'])
    # long press an icon then drop it
    box = pg.evaluate("(() => { const r = document.querySelector('#bottom [data-slot=\"0\"]').getBoundingClientRect(); return [r.left + r.width/2, r.top + r.height/2]; })()")
    reset(); pg.mouse.move(*box); pg.mouse.down(); pg.wait_for_timeout(700)
    check('long press picks the icon up: Tock then Tink', names() == ['tock', 'tink'], str(names()))
    reset(); pg.mouse.move(box[0] + 60, box[1], steps=4); pg.mouse.up(); pg.wait_for_timeout(300)
    check('dropping the icon plays a soft Tock', names() == ['tock'], str(names()))
    # failure path: sound switch off silences everything
    pg.evaluate("state.sound=false"); reset()
    pg.click('[data-hw="y"]'); pg.click('[data-hw="x"]'); pg.wait_for_timeout(300)
    check('with the sound switch off nothing plays', names() == [], str(names()))
    # level check: every file peaks at or below the iOS key click (-8.4 dBFS)
    peaks = pg.evaluate("""async () => { const c = new OfflineAudioContext(1, 44100, 44100); const r = {};
      for (const n of ['tap','tock','tink','lock','unlock','open','back','drop']) { const buf = await c.decodeAudioData(await (await fetch('assets/audio/'+n+'.wav')).arrayBuffer());
        const d = buf.getChannelData(0); let m = 0; for (const v of d) m = Math.max(m, Math.abs(v)); r[n] = [Math.round(20*Math.log10(m)*10)/10, Math.round(buf.duration*1000)]; } return r; }""")
    check('no file is louder than the iOS key click', all(v[0] <= -8.3 for v in peaks.values()), json.dumps(peaks))
    pg.screenshot(path=f'{out}/sound-check.png')
    b.close()
print(f'{sum(results)}/{len(results)} passed'); sys.exit(0 if all(results) else 1)
