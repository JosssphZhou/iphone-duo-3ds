// iOS UI sounds for the page. Files load after the first touch so they never slow the first screen.
// Web Audio is used because iOS Safari plays it with low latency once the context is resumed by a user gesture.
window.sfx = (() => {
  const NAMES = ['tap', 'tock', 'tink', 'lock', 'unlock', 'open', 'back', 'drop'];
  const BASE = new URL('./', document.currentScript.src);
  const buffers = {};
  let ctx = null;
  let loading = false;
  const api = { enabled: () => true };

  function ensure() {
    try {
      ctx ??= new (window.AudioContext || window.webkitAudioContext)();
      if (ctx.state !== 'running') ctx.resume();
      if (!loading) {
        loading = true;
        NAMES.forEach((name) => {
          fetch(new URL(`${name}.wav`, BASE))
            .then((r) => r.arrayBuffer())
            .then((data) => new Promise((ok, fail) => ctx.decodeAudioData(data, ok, fail)))
            .then((buffer) => { buffers[name] = buffer; })
            .catch(() => {});
        });
      }
    } catch {}
  }

  // iOS Safari only lets a context start from a touchend or click, so keep trying until it runs.
  ['pointerdown', 'pointerup', 'click', 'keydown'].forEach((type) =>
    document.addEventListener(type, () => { if (!ctx || ctx.state !== 'running') ensure(); }, { capture: true, passive: true })
  );

  api.play = (name, { gain = 1, delay = 0 } = {}) => {
    if (!api.enabled()) return;
    ensure();
    const buffer = buffers[name];
    if (!ctx || !buffer || ctx.state !== 'running') return;
    const source = ctx.createBufferSource();
    source.buffer = buffer;
    if (gain === 1) {
      source.connect(ctx.destination);
    } else {
      const node = ctx.createGain();
      node.gain.value = gain;
      source.connect(node).connect(ctx.destination);
    }
    source.start(ctx.currentTime + delay);
  };

  return api;
})();
