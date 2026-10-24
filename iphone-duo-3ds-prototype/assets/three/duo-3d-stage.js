import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { CSS3DRenderer, CSS3DObject } from 'three/addons/renderers/CSS3DRenderer.js';
import {
  CONSOLE_W,
  CONSOLE_H,
  lightConsole,
  buildConsoleModel,
} from './duo-3d-model.js';

function clamp(v, min, max) {
  return Math.max(min, Math.min(max, v));
}

function smooth(t) {
  const x = clamp(t, 0, 1);
  return x * x * (3 - 2 * x);
}

export function initDuo3DStage() {
  const viewport = document.querySelector('.viewport');
  const consoleEl = document.querySelector('.console');
  const viewControlsEl = document.querySelector('#view-3d-controls');

  if (!viewport || !consoleEl) return null;

  const params = new URLSearchParams(window.location.search);
  const forceNoWebGL =
    window.__DUO_DISABLE_WEBGL__ === true ||
    params.get('webgl') === '0' ||
    params.get('mode') === '2d';

  let renderer;
  let disposed = false;
  let controls;
  function fallback(err) {
    disposed = true;
    controls?.dispose();
    renderer?.dispose();
    if (consoleEl.parentElement !== viewport) viewport.appendChild(consoleEl);
    consoleEl.inert = false;
    document.querySelectorAll('#duo-3d-canvas, #duo-css3d-layer, .stage-3d-hint').forEach(el => el.remove());
    document.body.classList.remove('webgl-3d');
    if (viewControlsEl) viewControlsEl.hidden = true;
    const status = { active: false, webgl: false, fallback2D: true, reason: String(err?.message || err) };
    window.duo3D = { ...status, snapshot: () => ({ ...status }) };
    window.duo3DSnapshot = () => window.duo3D.snapshot();
    window.duo3DSetView = () => {};
    if (typeof window.size === 'function') window.size();
    return window.duo3D;
  }
  const canvas = document.createElement('canvas');
  canvas.id = 'duo-3d-canvas';
  canvas.setAttribute('aria-label', 'Nintendo 3DS 3D hardware enclosure');

  try {
    if (forceNoWebGL) {
      throw new Error('WebGL disabled by configuration');
    }
    renderer = new THREE.WebGLRenderer({
      canvas,
      antialias: true,
      alpha: true,
      powerPreference: 'high-performance',
      preserveDrawingBuffer: true,
    });
  } catch (err) {
    return fallback(err);
  }

  try {

  document.body.classList.add('webgl-3d');
  if (viewControlsEl) viewControlsEl.hidden = false;

  renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
  renderer.setClearColor(0x000000, 0);
  renderer.outputColorSpace = THREE.SRGBColorSpace;
  renderer.toneMapping = THREE.NeutralToneMapping;
  renderer.toneMappingExposure = 1.02;
  renderer.shadowMap.enabled = true;
  renderer.shadowMap.type = THREE.VSMShadowMap;

  const scene = new THREE.Scene();
  const camera = new THREE.PerspectiveCamera(34, 1, 20, 4500);

  const lighting = lightConsole(scene, renderer);
  const model = buildConsoleModel();
  scene.add(model.root);
  // Local bounds of the shell, used to keep a turned console inside the stage card.
  const shellCorners = [];
  {
    const box = new THREE.Box3().setFromObject(model.root);
    for (const x of [box.min.x, box.max.x])
      for (const y of [box.min.y, box.max.y])
        for (const z of [box.min.z, box.max.z]) shellCorners.push(new THREE.Vector3(x, y, z));
  }

  // Soft studio backdrop shadow receiver plane behind the 3DS
  const shadowPlane = new THREE.Mesh(
    new THREE.PlaneGeometry(1800, 1800),
    new THREE.ShadowMaterial({ opacity: 0.11 })
  );
  shadowPlane.position.set(0, -18, -42);
  shadowPlane.receiveShadow = true;
  scene.add(shadowPlane);

  // Set up CSS3DRenderer layer for crisp live DOM screens & hardware button hit targets
  const cssLayer = document.createElement('div');
  cssLayer.id = 'duo-css3d-layer';
  const cssRenderer = new CSS3DRenderer({ element: cssLayer });

  const carrier = document.createElement('div');
  carrier.className = 'console-3d-carrier';
  viewport.prepend(canvas);
  viewport.appendChild(cssLayer);
  carrier.appendChild(consoleEl);

  const cssObject = new CSS3DObject(carrier);
  // CSS3DObject opts into pointer events; only the live controls should do so.
  carrier.style.pointerEvents = 'none';
  cssObject.position.set(0, 0, 0);
  model.root.add(cssObject);

  // First-visit hint under the stage; fades a few seconds after the first touch.
  const stageHint = document.createElement('p');
  stageHint.className = 'stage-3d-hint';
  stageHint.textContent = '拖动机身旋转，点下屏直接操作';
  viewport.after(stageHint);
  if (typeof window.translateDOM === 'function') {
    window.translateDOM(stageHint);
  }
  viewport.addEventListener('pointerdown', () => {
    setTimeout(() => stageHint.classList.add('is-faded'), 4000);
  }, { once: true });

  // OrbitControls on the WebGL canvas
  controls = new OrbitControls(camera, canvas);
  controls.enableDamping = true;
  controls.dampingFactor = 0.085;
  controls.enablePan = false;
  controls.minDistance = 480;
  controls.maxDistance = 2600;
  controls.rotateSpeed = 0.85;

  let currentView = 'front';
  let frontZ = 1150;
  let currentScale = 1;
  let viewTween = null;
  const buttonPress = new Map();
  const reducedMotion = () =>
    window.matchMedia('(prefers-reduced-motion: reduce)').matches;

  function viewPose(mode) {
    if (mode === 'back') {
      return {
        pos: new THREE.Vector3(frontZ * 0.04, frontZ * 0.04, -frontZ * 1.02),
        target: new THREE.Vector3(0, 0, -14),
      };
    }
    if (mode === 'hero') {
      return {
        pos: new THREE.Vector3(frontZ * 0.36, frontZ * 0.19, frontZ * 0.94),
        target: new THREE.Vector3(0, 0, -8),
      };
    }
    return {
      pos: new THREE.Vector3(0, 0, frontZ),
      target: new THREE.Vector3(0, 0, 0),
    };
  }

  function updateViewButtons(activeMode) {
    document.querySelectorAll('[data-view]').forEach((btn) => {
      btn.setAttribute('aria-pressed', String(btn.dataset.view === activeMode));
    });
  }

  const frontNormal = new THREE.Vector3();
  const camToOrigin = new THREE.Vector3();

  // The live screens stay interactive at every angle: the browser maps taps
  // through the CSS3D transform, so there is no "switch to front view first" step.
  let needsRender = true;
  let orbitFrom = null;
  controls.addEventListener('change', () => {
    needsRender = true;
    // A tap on the shell fires 'start' without moving; only real drags leave the preset view.
    if (orbitFrom && camera.position.distanceTo(orbitFrom) > 1) {
      orbitFrom = null;
      currentView = 'custom';
      updateViewButtons(null);
    }
  });

  // Turned away from the front, the near corner comes toward the camera and grows past the
  // rounded stage card. fitFactor() is how much farther the camera must sit, relative to the
  // front distance, so every shell corner stays inside the card (1 when it already fits).
  const fitRight = new THREE.Vector3();
  const fitUp = new THREE.Vector3();
  const fitBack = new THREE.Vector3();
  const fitCorner = new THREE.Vector3();
  let fitLast = 1;
  function fitFactor() {
    fitBack.copy(camera.position).sub(controls.target).normalize();
    fitRight.crossVectors(camera.up, fitBack);
    if (fitRight.lengthSq() < 1e-6) return fitLast;
    fitRight.normalize();
    fitUp.crossVectors(fitBack, fitRight);
    const tanY = Math.tan(THREE.MathUtils.degToRad(camera.fov / 2)) * 0.97;
    const tanX = tanY * camera.aspect;
    model.root.updateMatrixWorld(true);
    let need = 0;
    for (const c of shellCorners) {
      fitCorner.copy(c).applyMatrix4(model.root.matrixWorld).sub(controls.target);
      const toward = fitCorner.dot(fitBack);
      need = Math.max(
        need,
        toward + Math.abs(fitCorner.dot(fitRight)) / tanX,
        toward + Math.abs(fitCorner.dot(fitUp)) / tanY
      );
    }
    return Math.max(1, need / frontZ);
  }

  function renderFrame() {
    needsRender = false;

    model.root.updateMatrixWorld(true);
    camera.updateMatrixWorld(true);

    // Hide the front DOM screen layer when looking at the rear of the 3DS
    frontNormal.set(0, 0, 1).applyQuaternion(model.root.quaternion).normalize();
    camToOrigin.copy(camera.position).sub(controls.target).normalize();
    const facingDot = frontNormal.dot(camToOrigin);
    const frontVisible = facingDot > 0.08;
    carrier.style.visibility = frontVisible ? 'visible' : 'hidden';
    carrier.style.opacity = frontVisible
      ? String(clamp((facingDot - 0.08) / 0.18, 0, 1))
      : '0';

    // Hide shadow backdrop when inspecting the rear shell so it never blocks the camera
    shadowPlane.visible = camera.position.z > -10;

    renderer.render(scene, camera);
    cssRenderer.render(scene, camera);
  }

  function setView(mode, { immediate = false, duration = 260 } = {}) {
    if (!['front', 'hero', 'back'].includes(mode)) mode = 'front';
    controls.enableDamping = false;
    controls.update();
    controls.enableDamping = true;
    currentView = mode;
    needsRender = true;
    updateViewButtons(mode);
    const pose = viewPose(mode);
    camera.up.set(0, 1, 0);
    if (immediate || reducedMotion() || duration <= 0) {
      viewTween = null;
      camera.position.copy(pose.pos);
      controls.target.copy(pose.target);
      controls.update();
      renderFrame();
      return;
    }
    viewTween = {
      fromPos: camera.position.clone(),
      toPos: pose.pos.clone(),
      fromTarget: controls.target.clone(),
      toTarget: pose.target.clone(),
      start: performance.now(),
      duration,
    };
  }

  function setPortrait(portrait) {
    model.root.rotation.z = portrait ? Math.PI / 2 : 0;
    renderFrame();
  }

  function pressHardwareButton(key) {
    const targetKey = ['up', 'down', 'left', 'right'].includes(key) ? 'dpad' : key;
    if (model.interactiveButtons.has(targetKey)) {
      buttonPress.set(targetKey, { start: performance.now() });
    }
  }

  function computeStageSize({
    pixelSize = false,
    portrait = false,
    recording = false,
  } = {}) {
    const docWidth = document.documentElement.clientWidth || window.innerWidth;
    const availW = Math.max(280, docWidth - (docWidth <= 440 ? 8 : 16));
    const consoleW = portrait ? CONSOLE_H : CONSOLE_W;
    const consoleH = portrait ? CONSOLE_W : CONSOLE_H;

    // Leave subtle breathing room around the 3D shell inside the stage
    const marginFactor = docWidth <= 440 ? 1.03 : 1.12;
    const maxStageW = Math.min(availW, recording ? 1120 : 880);
    const maxStageH = Math.max(
      340,
      window.innerHeight - (recording ? 76 : docWidth <= 620 ? 118 : 132)
    );

    const s = pixelSize
      ? Math.min(1, availW / (consoleW * marginFactor))
      : Math.min(
          recording ? 1.45 : 1.16,
          maxStageW / (consoleW * marginFactor),
          maxStageH / (consoleH * marginFactor)
        );

    currentScale = s;
    const vpW = Math.min(availW, Math.round(consoleW * s * marginFactor));
    const vpH = Math.round(consoleH * s * marginFactor);

    viewport.style.width = vpW + 'px';
    viewport.style.height = vpH + 'px';
    consoleEl.style.transform = 'none';

    renderer.setSize(vpW, vpH, false);
    cssRenderer.setSize(vpW, vpH);
    camera.aspect = vpW / Math.max(1, vpH);
    camera.updateProjectionMatrix();

    const fovPx = camera.projectionMatrix.elements[5] * (vpH / 2);
    const previousFrontZ = frontZ;
    frontZ = fovPx / s;

    model.root.rotation.z = portrait ? Math.PI / 2 : 0;

    if (currentView === 'custom') {
      // Keep the user's own angle on resize (mobile URL bar, rotation); only rescale the distance.
      const tween = viewTween;
      viewTween = null;
      if (tween) camera.position.copy(tween.toPos);
      const offset = camera.position.clone().sub(controls.target);
      offset.setLength(offset.length() * (frontZ / previousFrontZ));
      camera.position.copy(controls.target).add(offset);
      controls.update();
    } else {
      viewTween = null;
      const pose = viewPose(currentView);
      camera.position.copy(pose.pos);
      controls.target.copy(pose.target);
      controls.update();
    }
    renderFrame();
  }

  // Wire header view buttons
  document.querySelectorAll('[data-view]').forEach((btn) => {
    btn.addEventListener('click', () => {
      setView(btn.dataset.view, { immediate: false, duration: 220 });
    });
  });

  // When the user orbits manually, the 'change' handler marks the view as custom.
  controls.addEventListener('start', () => {
    needsRender = true;
    viewTween = null;
    orbitFrom = camera.position.clone();
  });
  controls.addEventListener('end', () => { orbitFrom = null; });

  // Double-clicking stage background resets to Front operate view
  canvas.addEventListener('dblclick', () => {
    setView('front', { immediate: false, duration: 220 });
  });

  // Ensure any pointerdown on #bottom or hardware buttons finishes any active camera tween
  // and does not trigger camera orbiting.
  viewport.addEventListener(
    'pointerdown',
    (e) => {
      const onInteractiveDOM = e.target.closest(
        '#bottom-shell, .dpad button, .abxy button, .start-select button, .home'
      );
      if (onInteractiveDOM) {
        if (viewTween) {
          camera.position.copy(viewTween.toPos);
          controls.target.copy(viewTween.toTarget);
          viewTween = null;
          controls.update();
          renderFrame();
        }
        const hwBtn = e.target.closest('[data-hw],[data-dir]');
        if (hwBtn) {
          pressHardwareButton(hwBtn.dataset.hw || 'dpad');
        }
      }
    },
    true
  );

  // Animate 3D buttons on keyboard shortcuts too
  document.addEventListener('keydown', (e) => {
    if (e.target.matches?.('input,textarea,select,[contenteditable]')) return;
    const keyMap = {
      Enter: 'a',
      a: 'a',
      A: 'a',
      b: 'b',
      B: 'b',
      x: 'x',
      X: 'x',
      y: 'y',
      Y: 'y',
      Escape: 'b',
      Home: 'home',
      ArrowUp: 'dpad',
      ArrowDown: 'dpad',
      ArrowLeft: 'dpad',
      ArrowRight: 'dpad',
    };
    if (keyMap[e.key]) pressHardwareButton(keyMap[e.key]);
  });

  // Initial sizing
  computeStageSize({
    pixelSize: false,
    portrait: document.body.classList.contains('portrait'),
    recording: document.body.classList.contains('recording'),
  });

  // Tasteful initial hero reveal (3/4 studio angle -> readable Front view), respects reduced motion
  if (!reducedMotion()) {
    const heroPose = viewPose('hero');
    const frontPose = viewPose('front');
    camera.position.copy(heroPose.pos);
    controls.target.copy(heroPose.target);
    controls.update();
    viewTween = {
      fromPos: heroPose.pos.clone(),
      toPos: frontPose.pos.clone(),
      fromTarget: heroPose.target.clone(),
      toTarget: frontPose.target.clone(),
      start: performance.now(),
      duration: 900,
    };
    currentView = 'front';
    updateViewButtons('front');
  } else {
    setView('front', { immediate: true });
  }

  function animateLoop(now) {
    if (disposed) return;
    if (viewTween || buttonPress.size) needsRender = true;
    if (viewTween) {
      const t = reducedMotion()
        ? 1
        : clamp((now - viewTween.start) / viewTween.duration, 0, 1);
      const k = smooth(t);
      camera.position.lerpVectors(viewTween.fromPos, viewTween.toPos, k);
      controls.target.lerpVectors(viewTween.fromTarget, viewTween.toTarget, k);
      if (t >= 1) {
        camera.position.copy(viewTween.toPos);
        controls.target.copy(viewTween.toTarget);
        viewTween = null;
      }
    }

    // Update physical 3D button press animations
    for (const [key, mesh] of model.interactiveButtons.entries()) {
      const press = buttonPress.get(key);
      if (!press) continue;
      const elapsed = now - press.start;
      if (elapsed >= 160) {
        mesh.position.z = mesh.userData.baseZ;
        buttonPress.delete(key);
      } else {
        const wave = Math.sin((elapsed / 160) * Math.PI);
        mesh.position.z = mesh.userData.baseZ - wave * 1.45;
      }
    }

    controls.update();
    // While the user turns the console, ease the camera back just enough to keep it in frame.
    // Scaling by the change in fit keeps any wheel or pinch zoom the user chose.
    const fit = fitFactor();
    if (!viewTween && currentView === 'custom' && Math.abs(fit / fitLast - 1) > 1e-4) {
      camToOrigin.copy(camera.position).sub(controls.target).multiplyScalar(fit / fitLast);
      camera.position.copy(controls.target).add(camToOrigin);
      needsRender = true;
    }
    fitLast = fit;
    if (needsRender && !document.hidden) renderFrame();
    requestAnimationFrame(animateLoop);
  }
  requestAnimationFrame(animateLoop);

  const api = {
    active: true,
    webgl: true,
    fallback2D: false,
    scene,
    camera,
    renderer,
    cssRenderer,
    controls,
    model,
    lighting,
    setView,
    setPortrait,
    pressHardwareButton,
    resize: computeStageSize,
    snapshot() {
      renderFrame();
      const topEl = document.querySelector('#top-shell');
      const botEl = document.querySelector('#bottom-shell');
      const topRect = topEl?.getBoundingClientRect();
      const botRect = botEl?.getBoundingClientRect();
      return {
        active: true,
        webgl: true,
        view: currentView,
        portrait: document.body.classList.contains('portrait'),
        scale: Number(currentScale.toFixed(4)),
        drawCalls: renderer.info.render.calls,
        triangles: renderer.info.render.triangles,
        geometries: renderer.info.memory.geometries,
        textures: renderer.info.memory.textures,
        meshCount: model.meshCount,
        shellThickness: model.shellThickness,
        cameraPosition: camera.position.toArray().map((n) => Number(n.toFixed(2))),
        cameraTarget: controls.target.toArray().map((n) => Number(n.toFixed(2))),
        topScreenRect: topRect
          ? {
              x: Math.round(topRect.x),
              y: Math.round(topRect.y),
              width: Math.round(topRect.width),
              height: Math.round(topRect.height),
            }
          : null,
        bottomScreenRect: botRect
          ? {
              x: Math.round(botRect.x),
              y: Math.round(botRect.y),
              width: Math.round(botRect.width),
              height: Math.round(botRect.height),
            }
          : null,
      };
    },
  };

  window.duo3D = api;
  window.duo3DSnapshot = () => api.snapshot();
  window.duo3DSetView = (mode, immediate = true) =>
    api.setView(mode, { immediate });
  canvas.addEventListener('webglcontextlost', (event) => {
    event.preventDefault();
    fallback(new Error('WebGL context lost'));
  }, { once: true });
  return api;
  } catch (err) {
    return fallback(err);
  }
}

if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', () => initDuo3DStage());
} else {
  initDuo3DStage();
}
