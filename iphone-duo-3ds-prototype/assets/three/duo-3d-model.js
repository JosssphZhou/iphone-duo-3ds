import * as THREE from 'three';
import { RoundedBoxGeometry } from 'three/addons/geometries/RoundedBoxGeometry.js';

export const CONSOLE_W = 600;
export const CONSOLE_H = 658;

export function cxTo3D(cx) {
  return cx - CONSOLE_W / 2;
}

export function cyTo3D(cy) {
  return CONSOLE_H / 2 - cy;
}

// Create a 2D rounded rectangle Shape with independent corner radii [tl, tr, br, bl]
function roundedRectShape(width, height, radii) {
  const [tl, tr, br, bl] = Array.isArray(radii) ? radii : [radii, radii, radii, radii];
  const x = -width / 2;
  const y = -height / 2;
  const w = width;
  const h = height;
  const shape = new THREE.Shape();
  shape.moveTo(x + tl, y + h);
  shape.lineTo(x + w - tr, y + h);
  shape.absarc(x + w - tr, y + h - tr, tr, Math.PI / 2, 0, true);
  shape.lineTo(x + w, y + br);
  shape.absarc(x + w - br, y + br, br, 0, -Math.PI / 2, true);
  shape.lineTo(x + bl, y);
  shape.absarc(x + bl, y + bl, bl, -Math.PI / 2, -Math.PI, true);
  shape.lineTo(x, y + h - tl);
  shape.absarc(x + tl, y + h - tl, tl, Math.PI, Math.PI / 2, true);
  return shape;
}

// Create a rounded rectangular Path (hole) for ExtrudeGeometry
function roundedRectHole(cx, cy, width, height, radius) {
  const x = cx - width / 2;
  const y = cy - height / 2;
  const r = Math.min(radius, width / 2, height / 2);
  const path = new THREE.Path();
  path.moveTo(x + r, y);
  path.lineTo(x + width - r, y);
  path.absarc(x + width - r, y + r, r, -Math.PI / 2, 0, false);
  path.lineTo(x + width, y + height - r);
  path.absarc(x + width - r, y + height - r, r, 0, Math.PI / 2, false);
  path.lineTo(x + r, y + height);
  path.absarc(x + r, y + height - r, r, Math.PI / 2, Math.PI, false);
  path.lineTo(x, y + r);
  path.absarc(x + r, y + r, r, Math.PI, Math.PI * 1.5, false);
  return path;
}

function makeTextCanvas(width, height, drawFn) {
  const canvas = document.createElement('canvas');
  canvas.width = width;
  canvas.height = height;
  const ctx = canvas.getContext('2d');
  drawFn(ctx, width, height);
  const tex = new THREE.CanvasTexture(canvas);
  tex.colorSpace = THREE.SRGBColorSpace;
  tex.needsUpdate = true;
  return tex;
}

// Studio photographic lighting following dial-lighting.js & stackchan-lighting.js philosophy
export function lightConsole(scene, renderer) {
  const studio = new THREE.Scene();
  studio.background = new THREE.Color(0.11, 0.115, 0.125);
  const panels = [];
  const specs = [
    [[-520, 760, 620], [560, 740], 5.6],
    [[580, 220, 480], [340, 640], 2.8],
    [[-440, 340, -640], [320, 580], 4.6],
    [[480, -240, 620], [360, 520], 1.9],
    [[0, 920, -80], [680, 460], 2.3],
    [[0, -680, -520], [540, 420], 2.4],
  ];
  for (const [pos, size, power] of specs) {
    const panel = new THREE.Mesh(
      new THREE.PlaneGeometry(...size),
      new THREE.MeshBasicMaterial({
        color: new THREE.Color(power, power, power),
        side: THREE.DoubleSide,
        toneMapped: false,
      })
    );
    panel.position.fromArray(pos);
    panel.lookAt(0, 0, 0);
    studio.add(panel);
    panels.push(panel);
  }
  const pmrem = new THREE.PMREMGenerator(renderer);
  const environment = pmrem.fromScene(studio, 0.03, 1, 3000);
  scene.environment = environment.texture;
  scene.environmentIntensity = 0.95;
  for (const panel of panels) {
    panel.geometry.dispose();
    panel.material.dispose();
  }
  pmrem.dispose();

  const hemi = new THREE.HemisphereLight(0xffffff, 0xc7ccd1, 0.38);
  scene.add(hemi);

  const key = new THREE.DirectionalLight(0xfffdf8, 2.35);
  key.position.set(-460, 880, 740);
  key.castShadow = true;
  key.shadow.mapSize.set(1024, 1024);
  key.shadow.radius = 9;
  key.shadow.blurSamples = 16;
  Object.assign(key.shadow.camera, {
    left: -580,
    right: 580,
    top: 580,
    bottom: -580,
    near: 80,
    far: 2400,
  });
  key.shadow.normalBias = 0.8;
  key.shadow.bias = -0.0003;
  scene.add(key);

  const frontFill = new THREE.DirectionalLight(0xf3f7ff, 1.05);
  frontFill.position.set(260, 220, 920);
  scene.add(frontFill);

  const rimBack = new THREE.DirectionalLight(0xffffff, 1.75);
  rimBack.position.set(520, 420, -780);
  scene.add(rimBack);

  const backFill = new THREE.DirectionalLight(0xe4ecfa, 1.15);
  backFill.position.set(-380, -240, -820);
  scene.add(backFill);

  return { environment, revision: 1 };
}

export function createConsoleMaterials() {
  return {
    shellOuter: new THREE.MeshPhysicalMaterial({
      name: 'Cosmos Graphite Outer',
      color: new THREE.Color(0x1f2228),
      roughness: 0.34,
      metalness: 0.16,
      clearcoat: 0.48,
      clearcoatRoughness: 0.22,
      envMapIntensity: 1.08,
    }),
    shellInner: new THREE.MeshPhysicalMaterial({
      name: 'Matte Inner Enclosure',
      color: new THREE.Color(0x23272e),
      roughness: 0.44,
      metalness: 0.08,
      clearcoat: 0.18,
      clearcoatRoughness: 0.36,
      envMapIntensity: 0.92,
    }),
    seamGroove: new THREE.MeshPhysicalMaterial({
      name: 'Sidewall Seam',
      color: new THREE.Color(0x0a0c0f),
      roughness: 0.68,
      metalness: 0.05,
      envMapIntensity: 0.5,
    }),
    bezelFrame: new THREE.MeshPhysicalMaterial({
      name: 'Display Bezel Frame',
      color: new THREE.Color(0x0d0f13),
      roughness: 0.22,
      metalness: 0.04,
      clearcoat: 0.72,
      clearcoatRoughness: 0.14,
      envMapIntensity: 1.05,
    }),
    opticalGlass: new THREE.MeshPhysicalMaterial({
      name: 'Display Cover Glass',
      color: new THREE.Color(0x07090c),
      roughness: 0.12,
      metalness: 0.0,
      clearcoat: 1.0,
      clearcoatRoughness: 0.06,
      envMapIntensity: 1.2,
    }),
    hingeBarrel: new THREE.MeshPhysicalMaterial({
      name: 'Hinge Barrel',
      color: new THREE.Color(0x282c33),
      roughness: 0.28,
      metalness: 0.38,
      clearcoat: 0.36,
      clearcoatRoughness: 0.2,
      envMapIntensity: 1.15,
    }),
    circlePad: new THREE.MeshPhysicalMaterial({
      name: 'Circle Pad Elastomer',
      color: new THREE.Color(0x90959d),
      roughness: 0.52,
      metalness: 0.02,
      clearcoat: 0.08,
      clearcoatRoughness: 0.45,
      envMapIntensity: 0.75,
    }),
    wellDark: new THREE.MeshPhysicalMaterial({
      name: 'Recessed Well',
      color: new THREE.Color(0x111418),
      roughness: 0.64,
      metalness: 0.04,
      envMapIntensity: 0.55,
    }),
    buttonCap: new THREE.MeshPhysicalMaterial({
      name: 'Tactile Button Cap',
      color: new THREE.Color(0x2c3037),
      roughness: 0.26,
      metalness: 0.08,
      clearcoat: 0.58,
      clearcoatRoughness: 0.18,
      envMapIntensity: 1.0,
    }),
    homeRing: new THREE.MeshPhysicalMaterial({
      name: 'HOME Bezel Ring',
      color: new THREE.Color(0x4c525b),
      roughness: 0.24,
      metalness: 0.52,
      clearcoat: 0.4,
      envMapIntensity: 1.2,
    }),
    shoulderButton: new THREE.MeshPhysicalMaterial({
      name: 'Shoulder Trigger',
      color: new THREE.Color(0x2a2e36),
      roughness: 0.3,
      metalness: 0.14,
      clearcoat: 0.32,
      envMapIntensity: 1.0,
    }),
    lensGlass: new THREE.MeshPhysicalMaterial({
      name: 'Camera Optical Lens',
      color: new THREE.Color(0x0c1b30),
      roughness: 0.08,
      metalness: 0.1,
      clearcoat: 1.0,
      clearcoatRoughness: 0.04,
      envMapIntensity: 1.45,
    }),
    irGlass: new THREE.MeshPhysicalMaterial({
      name: 'IR Transceiver Lens',
      color: new THREE.Color(0x26060b),
      roughness: 0.1,
      metalness: 0.0,
      clearcoat: 1.0,
      envMapIntensity: 1.3,
    }),
    metalPort: new THREE.MeshPhysicalMaterial({
      name: 'Port Shielding Metal',
      color: new THREE.Color(0x747b86),
      roughness: 0.26,
      metalness: 0.85,
      envMapIntensity: 1.3,
    }),
    chargeOrange: new THREE.MeshPhysicalMaterial({
      name: 'Charge Port Insulator',
      color: new THREE.Color(0xb86b2b),
      roughness: 0.42,
      metalness: 0.1,
    }),
    powerLed: new THREE.MeshPhysicalMaterial({
      name: 'Power LED',
      color: new THREE.Color(0x78bcff),
      emissive: new THREE.Color(0x398eff),
      emissiveIntensity: 1.85,
      roughness: 0.15,
      clearcoat: 0.9,
    }),
    rubberFoot: new THREE.MeshPhysicalMaterial({
      name: 'Silicone Grip Pad',
      color: new THREE.Color(0x14171c),
      roughness: 0.82,
      metalness: 0.0,
    }),
  };
}

export function buildConsoleModel() {
  const materials = createConsoleMaterials();
  const root = new THREE.Group();
  root.name = 'New3DSConsole';

  const interactiveButtons = new Map();
  const registerButton = (key, mesh) => {
    mesh.userData.hwKey = key;
    mesh.userData.baseZ = mesh.position.z;
    interactiveButtons.set(key, mesh);
  };

  // =========================================================================
  // 1. UPPER LID ASSEMBLY (600 x 304, center at cx=300, cy=152 => x=0, y=177)
  // =========================================================================
  const lidGroup = new THREE.Group();
  lidGroup.name = 'LidAssembly';
  const lidCenterY = cyTo3D(152); // +177
  lidGroup.position.set(0, lidCenterY, 0);
  root.add(lidGroup);

  // Lid main shell with recessed screen aperture
  const lidShape = roundedRectShape(600, 304, [35, 35, 23, 23]);
  // Top screen is at cx=300, cy=154 => relative to lid center (300, 152), y offset is -2
  const topScreenRelY = cyTo3D(154) - lidCenterY; // -2
  lidShape.holes.push(roundedRectHole(0, topScreenRelY, 400, 240, 8));
  const lidFrontGeo = new THREE.ExtrudeGeometry(lidShape, {
    depth: 9.5,
    bevelEnabled: true,
    bevelSegments: 5,
    steps: 1,
    bevelSize: 2.2,
    bevelThickness: 2.2,
    curveSegments: 24,
  });
  lidFrontGeo.translate(0, 0, -11.5);
  const lidFrontMesh = new THREE.Mesh(lidFrontGeo, materials.shellInner);
  lidFrontMesh.castShadow = true;
  lidFrontMesh.receiveShadow = true;
  lidGroup.add(lidFrontMesh);

  // Sidewall parting seam between inner lid and outer lid shell
  const lidSeamShape = roundedRectShape(602.2, 306.2, [36, 36, 24, 24]);
  const lidSeamGeo = new THREE.ExtrudeGeometry(lidSeamShape, {
    depth: 1.4,
    bevelEnabled: false,
    curveSegments: 24,
  });
  lidSeamGeo.translate(0, 0, -12.8);
  const lidSeamMesh = new THREE.Mesh(lidSeamGeo, materials.seamGroove);
  lidGroup.add(lidSeamMesh);

  // Outer rear lid shell (with deeper rounded chamfer on the back)
  const lidBackShape = roundedRectShape(596, 300, [34, 34, 22, 22]);
  const lidBackGeo = new THREE.ExtrudeGeometry(lidBackShape, {
    depth: 7.5,
    bevelEnabled: true,
    bevelSegments: 6,
    steps: 1,
    bevelSize: 3.4,
    bevelThickness: 3.4,
    curveSegments: 24,
  });
  lidBackGeo.translate(0, 0, -21.2);
  const lidBackMesh = new THREE.Mesh(lidBackGeo, materials.shellOuter);
  lidBackMesh.castShadow = true;
  lidBackMesh.receiveShadow = true;
  lidGroup.add(lidBackMesh);

  // Recessed Top Screen Frame & Bezel (400 x 240 screen opening, outer bezel 422 x 262)
  const topBezelShape = roundedRectShape(422, 262, 14);
  topBezelShape.holes.push(roundedRectHole(0, 0, 400, 240, 8));
  const topBezelGeo = new THREE.ExtrudeGeometry(topBezelShape, {
    depth: 2.4,
    bevelEnabled: true,
    bevelSegments: 3,
    bevelSize: 1.0,
    bevelThickness: 1.0,
    curveSegments: 16,
  });
  topBezelGeo.translate(0, topScreenRelY, -0.8);
  const topBezelMesh = new THREE.Mesh(topBezelGeo, materials.bezelFrame);
  lidGroup.add(topBezelMesh);

  // Top Screen Optical Glass Backing (sits at z = -0.6 right behind the DOM #top-shell)
  const topGlassGeo = new THREE.PlaneGeometry(404, 244);
  const topGlassMesh = new THREE.Mesh(topGlassGeo, materials.opticalGlass);
  topGlassMesh.position.set(0, topScreenRelY, -0.4);
  lidGroup.add(topGlassMesh);

  // Inner Camera Assembly (cx = 300.5, cy = 17.5 => relY = 134.5)
  const camRelX = cxTo3D(300.5);
  const camRelY = cyTo3D(19.5) - lidCenterY;
  const camRing = new THREE.Mesh(
    new THREE.RingGeometry(4.2, 7.6, 28),
    materials.bezelFrame
  );
  camRing.position.set(camRelX, camRelY, 0.35);
  lidGroup.add(camRing);

  const camLens = new THREE.Mesh(
    new THREE.SphereGeometry(4.3, 20, 12, 0, Math.PI * 2, 0, Math.PI * 0.45),
    materials.lensGlass
  );
  camLens.rotation.x = Math.PI / 2;
  camLens.position.set(camRelX, camRelY, -0.6);
  lidGroup.add(camLens);

  // Small camera indicator dot next to lens
  const camLed = new THREE.Mesh(
    new THREE.CircleGeometry(1.7, 16),
    materials.wellDark
  );
  camLed.position.set(camRelX + 14, camRelY, 0.35);
  lidGroup.add(camLed);

  // Stereo Speaker Holes (6 holes each side, matching .speaker at left:43px, top:124px)
  for (const side of [-1, 1]) {
    const baseCx = side < 0 ? 46.5 : 600 - 46.5;
    for (let row = 0; row < 3; row++) {
      for (let col = 0; col < 2; col++) {
        const hx = cxTo3D(baseCx + (col - 0.5) * 8);
        const hy = cyTo3D(128 + row * 8) - lidCenterY;
        const holeRim = new THREE.Mesh(
          new THREE.CylinderGeometry(2.1, 1.7, 2.2, 14),
          materials.wellDark
        );
        holeRim.rotation.x = Math.PI / 2;
        holeRim.position.set(hx, hy, -0.2);
        lidGroup.add(holeRim);
      }
    }
  }

  // Upper lid rubber bumpers (classic 3DS lid cushions)
  for (const side of [-1, 1]) {
    const bumper = new THREE.Mesh(
      new RoundedBoxGeometry(16, 5.5, 2.2, 2, 2),
      materials.rubberFoot
    );
    bumper.position.set(side * 256, 122, 0.4);
    lidGroup.add(bumper);
  }

  // Sidewall 3D Depth & Volume Sliders on the Lid
  for (const [side, labelY] of [[-1, 18], [1, 24]]) {
    const sliderTrack = new THREE.Mesh(
      new RoundedBoxGeometry(3.2, 46, 5.5, 2, 1.2),
      materials.wellDark
    );
    sliderTrack.position.set(side * 301.2, labelY, -10.5);
    lidGroup.add(sliderTrack);

    const sliderKnob = new THREE.Mesh(
      new RoundedBoxGeometry(4.8, 18, 4.4, 2, 1.4),
      materials.homeRing
    );
    sliderKnob.position.set(side * 302.2, labelY + 6, -10.5);
    lidGroup.add(sliderKnob);
  }

  // Outer Lid Rear Stereoscopic Dual Cameras & Emblem (at z = -24.7)
  const lidBackZ = -24.7;
  const cameraBar = new THREE.Mesh(
    new RoundedBoxGeometry(148, 30, 2.4, 3, 8),
    materials.bezelFrame
  );
  cameraBar.position.set(0, 92, lidBackZ);
  lidGroup.add(cameraBar);

  for (const camX of [-44, 44]) {
    const outerRing = new THREE.Mesh(
      new THREE.CylinderGeometry(9.5, 10.2, 2.8, 28),
      materials.homeRing
    );
    outerRing.rotation.x = Math.PI / 2;
    outerRing.position.set(camX, 92, lidBackZ - 0.6);
    lidGroup.add(outerRing);

    const outerLens = new THREE.Mesh(
      new THREE.CylinderGeometry(7.2, 7.2, 3.1, 24),
      materials.lensGlass
    );
    outerLens.rotation.x = Math.PI / 2;
    outerLens.position.set(camX, 92, lidBackZ - 0.8);
    lidGroup.add(outerLens);
  }

  // =========================================================================
  // 2. CENTRAL HINGE ASSEMBLY (576 x 24 at cy = 316 => y = +13)
  // =========================================================================
  const hingeGroup = new THREE.Group();
  hingeGroup.name = 'HingeAssembly';
  const hingeY = cyTo3D(316); // +13
  hingeGroup.position.set(0, hingeY, -10.5);
  root.add(hingeGroup);

  const barrelGeo = new THREE.CylinderGeometry(13.2, 13.2, 574, 32);
  barrelGeo.rotateZ(Math.PI / 2);
  const barrelMesh = new THREE.Mesh(barrelGeo, materials.hingeBarrel);
  barrelMesh.castShadow = true;
  barrelMesh.receiveShadow = true;
  hingeGroup.add(barrelMesh);

  // Hinge end caps (rounded domes at left and right ends)
  for (const side of [-1, 1]) {
    const capGeo = new THREE.SphereGeometry(13.1, 24, 16, 0, Math.PI * 2, 0, Math.PI / 2);
    capGeo.rotateZ(side > 0 ? -Math.PI / 2 : Math.PI / 2);
    capGeo.translate(side * 287, 0, 0);
    const capMesh = new THREE.Mesh(capGeo, materials.hingeBarrel);
    hingeGroup.add(capMesh);
  }

  // Hinge knuckle seam rings (matching .hinge:before at left:78px, right:78px => x = ±210)
  for (const seamX of [-210, 210]) {
    const ringGeo = new THREE.CylinderGeometry(13.55, 13.55, 2.8, 32);
    ringGeo.rotateZ(Math.PI / 2);
    ringGeo.translate(seamX, 0, 0);
    hingeGroup.add(new THREE.Mesh(ringGeo, materials.seamGroove));
  }

  // Notification LED diffuser ring on right hinge side
  const notifRingGeo = new THREE.CylinderGeometry(13.45, 13.45, 5.5, 32);
  notifRingGeo.rotateZ(Math.PI / 2);
  notifRingGeo.translate(268, 0, 0);
  const notifMat = new THREE.MeshPhysicalMaterial({
    color: 0x3b4859,
    emissive: 0x163860,
    emissiveIntensity: 0.45,
    roughness: 0.2,
    clearcoat: 0.9,
  });
  hingeGroup.add(new THREE.Mesh(notifRingGeo, notifMat));

  // =========================================================================
  // 3. LOWER BASE ASSEMBLY (600 x 324, center at cx=300, cy=490 => x=0, y=-161)
  // =========================================================================
  const baseGroup = new THREE.Group();
  baseGroup.name = 'BaseAssembly';
  const baseCenterY = cyTo3D(490); // -161
  baseGroup.position.set(0, baseCenterY, 0);
  root.add(baseGroup);

  const baseShape = roundedRectShape(600, 324, [22, 22, 40, 40]);
  const bottomScreenRelY = cyTo3D(477) - baseCenterY; // +13

  baseShape.holes.push(roundedRectHole(0, bottomScreenRelY, 320, 240, 6));
  const baseFrontGeo = new THREE.ExtrudeGeometry(baseShape, {
    depth: 13.5,
    bevelEnabled: true,
    bevelSegments: 5,
    steps: 1,
    bevelSize: 2.2,
    bevelThickness: 2.2,
    curveSegments: 24,
  });
  baseFrontGeo.translate(0, 0, -15.5);
  const baseFrontMesh = new THREE.Mesh(baseFrontGeo, materials.shellInner);
  baseFrontMesh.castShadow = true;
  baseFrontMesh.receiveShadow = true;
  baseGroup.add(baseFrontMesh);

  // Base sidewall parting seam
  const baseSeamShape = roundedRectShape(602.2, 326.2, [23, 23, 41, 41]);
  const baseSeamGeo = new THREE.ExtrudeGeometry(baseSeamShape, {
    depth: 1.5,
    bevelEnabled: false,
    curveSegments: 24,
  });
  baseSeamGeo.translate(0, 0, -16.8);
  baseGroup.add(new THREE.Mesh(baseSeamGeo, materials.seamGroove));

  // Base Rear Ergonomic Lower Housing
  const baseBackShape = roundedRectShape(596, 320, [21, 21, 39, 39]);
  const baseBackGeo = new THREE.ExtrudeGeometry(baseBackShape, {
    depth: 10.5,
    bevelEnabled: true,
    bevelSegments: 6,
    steps: 1,
    bevelSize: 3.8,
    bevelThickness: 3.8,
    curveSegments: 24,
  });
  baseBackGeo.translate(0, 0, -28.0);
  const baseBackMesh = new THREE.Mesh(baseBackGeo, materials.shellOuter);
  baseBackMesh.castShadow = true;
  baseBackMesh.receiveShadow = true;
  baseGroup.add(baseBackMesh);

  // Recessed Bottom Touchscreen Bezel (320 x 240 opening, outer rim 338 x 258)
  const botBezelShape = roundedRectShape(338, 258, 11);
  botBezelShape.holes.push(roundedRectHole(0, 0, 320, 240, 6));
  const botBezelGeo = new THREE.ExtrudeGeometry(botBezelShape, {
    depth: 2.6,
    bevelEnabled: true,
    bevelSegments: 3,
    bevelSize: 1.0,
    bevelThickness: 1.0,
    curveSegments: 16,
  });
  botBezelGeo.translate(0, bottomScreenRelY, -0.6);
  baseGroup.add(new THREE.Mesh(botBezelGeo, materials.bezelFrame));

  // Bottom Screen Backing Plane (at z = -0.4 right behind #bottom-shell)
  const botGlassMesh = new THREE.Mesh(
    new THREE.PlaneGeometry(324, 244),
    materials.opticalGlass
  );
  botGlassMesh.position.set(0, bottomScreenRelY, -0.4);
  baseGroup.add(botGlassMesh);

  // -------------------------------------------------------------------------
  // 4. FRONT PHYSICAL CONTROLS ON BASE
  // -------------------------------------------------------------------------

  // A. Circle Pad (cx = 76, cy = 404 => x = -224, relY = +86, diameter = 64)
  const cpadX = cxTo3D(76);
  const cpadY = cyTo3D(404) - baseCenterY;
  const cpadWell = new THREE.Mesh(
    new THREE.CylinderGeometry(35, 33, 2.2, 36),
    materials.wellDark
  );
  cpadWell.rotation.x = Math.PI / 2;
  cpadWell.position.set(cpadX, cpadY, -0.2);
  baseGroup.add(cpadWell);

  const cpadCapGroup = new THREE.Group();
  cpadCapGroup.position.set(cpadX, cpadY, 2.2);
  const cpadBaseDisk = new THREE.Mesh(
    new THREE.CylinderGeometry(28.5, 30.5, 3.2, 36),
    materials.circlePad
  );
  cpadBaseDisk.rotation.x = Math.PI / 2;
  cpadBaseDisk.castShadow = true;
  cpadCapGroup.add(cpadBaseDisk);

  const cpadRim = new THREE.Mesh(
    new THREE.TorusGeometry(24.5, 3.1, 16, 36),
    materials.circlePad
  );
  cpadRim.position.z = 1.2;
  cpadCapGroup.add(cpadRim);
  baseGroup.add(cpadCapGroup);

  // B. Cross D-Pad (cx = 77, cy = 511 => x = -223, relY = -21, size 62 x 62, arm 22)
  const dpadX = cxTo3D(77);
  const dpadY = cyTo3D(511) - baseCenterY;
  const dpadGroup = new THREE.Group();
  dpadGroup.position.set(dpadX, dpadY, 2.0);
  baseGroup.add(dpadGroup);

  const crossShape = new THREE.Shape();
  const arm = 11;
  const reach = 31;
  crossShape.moveTo(-arm, reach);
  crossShape.lineTo(arm, reach);
  crossShape.lineTo(arm, arm);
  crossShape.lineTo(reach, arm);
  crossShape.lineTo(reach, -arm);
  crossShape.lineTo(arm, -arm);
  crossShape.lineTo(arm, -reach);
  crossShape.lineTo(-arm, -reach);
  crossShape.lineTo(-arm, -arm);
  crossShape.lineTo(-reach, -arm);
  crossShape.lineTo(-reach, arm);
  crossShape.lineTo(-arm, arm);
  crossShape.closePath();

  const dpadGeo = new THREE.ExtrudeGeometry(crossShape, {
    depth: 3.2,
    bevelEnabled: true,
    bevelSegments: 3,
    bevelSize: 1.1,
    bevelThickness: 1.1,
  });
  dpadGeo.translate(0, 0, -1.6);
  const dpadMesh = new THREE.Mesh(dpadGeo, materials.buttonCap);
  dpadMesh.castShadow = true;
  dpadGroup.add(dpadMesh);

  // D-Pad directional glyph markings
  const dpadTex = makeTextCanvas(256, 256, (ctx, w, h) => {
    ctx.clearRect(0, 0, w, h);
    ctx.fillStyle = '#9aa1aa';
    ctx.font = 'bold 28px sans-serif';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText('▲', w / 2, 42);
    ctx.fillText('▼', w / 2, h - 42);
    ctx.fillText('◀', 42, h / 2);
    ctx.fillText('▶', w - 42, h / 2);
  });
  const dpadOverlay = new THREE.Mesh(
    new THREE.PlaneGeometry(58, 58),
    new THREE.MeshBasicMaterial({ map: dpadTex, transparent: true })
  );
  dpadOverlay.position.z = 2.85;
  dpadGroup.add(dpadOverlay);
  registerButton('dpad', dpadGroup);

  // C. C-Stick Trackpoint Nub (cx = 529.5, cy = 365.5 => x = +229.5, relY = +124.5)
  const cstickX = cxTo3D(529.5);
  const cstickY = cyTo3D(365.5) - baseCenterY;
  const cstickWell = new THREE.Mesh(
    new THREE.CylinderGeometry(10.2, 9.8, 1.8, 24),
    materials.wellDark
  );
  cstickWell.rotation.x = Math.PI / 2;
  cstickWell.position.set(cstickX, cstickY, 0.1);
  baseGroup.add(cstickWell);

  const cstickNub = new THREE.Mesh(
    new THREE.SphereGeometry(8.2, 24, 16, 0, Math.PI * 2, 0, Math.PI * 0.6),
    materials.circlePad
  );
  cstickNub.rotation.x = Math.PI / 2;
  cstickNub.position.set(cstickX, cstickY, 1.6);
  cstickNub.castShadow = true;
  baseGroup.add(cstickNub);

  // D. ABXY Face Buttons
  const abxySpecs = [
    { key: 'x', label: 'X', cx: 525.5, cy: 410.5, accent: '#6ca8ff' },
    { key: 'a', label: 'A', cx: 551.5, cy: 435.5, accent: '#ff7068' },
    { key: 'b', label: 'B', cx: 525.5, cy: 461.5, accent: '#ffd257' },
    { key: 'y', label: 'Y', cx: 500.5, cy: 435.5, accent: '#63d98a' },
  ];
  for (const spec of abxySpecs) {
    const bx = cxTo3D(spec.cx);
    const by = cyTo3D(spec.cy) - baseCenterY;

    const btnWell = new THREE.Mesh(
      new THREE.CylinderGeometry(16.6, 16.0, 1.8, 28),
      materials.wellDark
    );
    btnWell.rotation.x = Math.PI / 2;
    btnWell.position.set(bx, by, 0.0);
    baseGroup.add(btnWell);

    const btnGroup = new THREE.Group();
    btnGroup.position.set(bx, by, 2.2);

    const capMesh = new THREE.Mesh(
      new THREE.CylinderGeometry(14.2, 15.2, 3.6, 28),
      materials.buttonCap
    );
    capMesh.rotation.x = Math.PI / 2;
    capMesh.castShadow = true;
    btnGroup.add(capMesh);

    const labelTex = makeTextCanvas(128, 128, (ctx, w, h) => {
      ctx.clearRect(0, 0, w, h);
      ctx.fillStyle = spec.accent;
      ctx.font = '600 66px -apple-system, BlinkMacSystemFont, "Helvetica Neue", Arial, sans-serif';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.fillText(spec.label, w / 2, h / 2 + 2);
    });
    const capTop = new THREE.Mesh(
      new THREE.CircleGeometry(13.2, 24),
      new THREE.MeshBasicMaterial({ map: labelTex, transparent: true })
    );
    capTop.position.z = 1.85;
    btnGroup.add(capTop);

    baseGroup.add(btnGroup);
    registerButton(spec.key, btnGroup);
  }

  // E. START & SELECT Buttons (right: 48px, bottom: 55px in base)
  const startSelectSpecs = [
    { key: 'lock', label: 'START', cx: 506, cy: 569 },
    { key: 'search', label: 'SELECT', cx: 502, cy: 589 },
  ];
  for (const spec of startSelectSpecs) {
    const sx = cxTo3D(spec.cx);
    const sy = cyTo3D(spec.cy) - baseCenterY;

    const ssGroup = new THREE.Group();
    ssGroup.position.set(sx, sy, 1.5);

    const nub = new THREE.Mesh(
      new THREE.CylinderGeometry(5.2, 5.8, 2.8, 20),
      materials.buttonCap
    );
    nub.rotation.x = Math.PI / 2;
    ssGroup.add(nub);

    const lblTex = makeTextCanvas(160, 48, (ctx, w, h) => {
      ctx.clearRect(0, 0, w, h);
      ctx.fillStyle = '#9298a1';
      ctx.font = '600 22px -apple-system, BlinkMacSystemFont, Arial, sans-serif';
      ctx.textAlign = 'left';
      ctx.textBaseline = 'middle';
      ctx.fillText(spec.label, 8, h / 2);
    });
    const lblPlane = new THREE.Mesh(
      new THREE.PlaneGeometry(44, 13),
      new THREE.MeshBasicMaterial({ map: lblTex, transparent: true })
    );
    lblPlane.position.set(30, 0, 0.2);
    ssGroup.add(lblPlane);

    baseGroup.add(ssGroup);
    registerButton(spec.key, ssGroup);
  }

  // G. Front Brand Wordmark & Glowing Power LED
  const brandTex = makeTextCanvas(256, 48, (ctx, w, h) => {
    ctx.clearRect(0, 0, w, h);
    ctx.fillStyle = '#7c828c';
    ctx.font = '600 22px -apple-system, BlinkMacSystemFont, Arial, sans-serif';
    ctx.textAlign = 'left';
    ctx.textBaseline = 'middle';
    ctx.fillText('NINTENDO 3DS', 6, h / 2);
  });
  const brandMesh = new THREE.Mesh(
    new THREE.PlaneGeometry(92, 17),
    new THREE.MeshBasicMaterial({ map: brandTex, transparent: true })
  );
  brandMesh.position.set(cxTo3D(82), cyTo3D(628) - baseCenterY, 0.35);
  baseGroup.add(brandMesh);

  const ledX = cxTo3D(563);
  const ledY = cyTo3D(634) - baseCenterY;
  const ledMesh = new THREE.Mesh(
    new THREE.SphereGeometry(2.6, 16, 16),
    materials.powerLed
  );
  ledMesh.position.set(ledX, ledY, 0.8);
  baseGroup.add(ledMesh);

  // Sidewall LED lens continuation (visible when viewing right/bottom sidewall)
  const sideLed = new THREE.Mesh(
    new RoundedBoxGeometry(3.2, 6.5, 5.5, 2, 1),
    materials.powerLed
  );
  sideLed.position.set(286, -158, -9.5);
  baseGroup.add(sideLed);

  // =========================================================================
  // 5. TOP SHOULDER BUTTONS (L / R / ZL / ZR), PORTS & REAR DETAILS
  // =========================================================================
  const shoulderY = 158; // Top edge of base (near hinge)
  for (const side of [-1, 1]) {
    // Outer L / R wrap-around trigger
    const lrTrigger = new THREE.Mesh(
      new RoundedBoxGeometry(92, 16, 18, 4, 6),
      materials.shoulderButton
    );
    lrTrigger.position.set(side * 246, shoulderY, -19.5);
    lrTrigger.castShadow = true;
    baseGroup.add(lrTrigger);

    // Inner ZL / ZR trigger
    const zTrigger = new THREE.Mesh(
      new RoundedBoxGeometry(54, 14, 15, 3, 5),
      materials.shoulderButton
    );
    zTrigger.position.set(side * 164, shoulderY + 1, -20.5);
    zTrigger.castShadow = true;
    baseGroup.add(zTrigger);
  }

  // Top-edge ports along the rear spine of the base (y = +161, z = -19)
  // 1) AC Adapter Charge Port (center-right)
  const chargeOuter = new THREE.Mesh(
    new RoundedBoxGeometry(28, 8, 10, 2, 2),
    materials.metalPort
  );
  chargeOuter.position.set(95, 160, -19.5);
  baseGroup.add(chargeOuter);

  const chargeInner = new THREE.Mesh(
    new RoundedBoxGeometry(18, 6, 5.5, 1, 1),
    materials.chargeOrange
  );
  chargeInner.position.set(95, 161.5, -19.5);
  baseGroup.add(chargeInner);

  // 2) Infrared Transceiver Window (center-left)
  const irWindow = new THREE.Mesh(
    new RoundedBoxGeometry(26, 7, 8, 2, 2.5),
    materials.irGlass
  );
  irWindow.position.set(-22, 160.5, -19.5);
  baseGroup.add(irWindow);

  // 3) Game Card Slot (left of center)
  const slotRecess = new THREE.Mesh(
    new RoundedBoxGeometry(88, 8, 9.5, 2, 2.5),
    materials.wellDark
  );
  slotRecess.position.set(-88, 160, -19.5);
  baseGroup.add(slotRecess);

  const gameCartridge = new THREE.Mesh(
    new RoundedBoxGeometry(80, 6, 6.2, 2, 1.8),
    materials.homeRing
  );
  gameCartridge.position.set(-88, 160.8, -19.5);
  baseGroup.add(gameCartridge);

  // 4) Bottom-edge Headphone Jack & Stylus Silo (y = -161, z = -19)
  const audioJack = new THREE.Mesh(
    new THREE.CylinderGeometry(4.5, 4.5, 6, 20),
    materials.metalPort
  );
  audioJack.position.set(0, -161, -19.5);
  baseGroup.add(audioJack);

  const stylusHead = new THREE.Mesh(
    new RoundedBoxGeometry(11, 8, 14, 2, 2.5),
    materials.bezelFrame
  );
  stylusHead.position.set(78, -161, -19.5);
  baseGroup.add(stylusHead);

  // 5) Detailed Rear Base Plate (at z = -31.8)
  const baseRearZ = -31.8;
  const backPlateTex = makeTextCanvas(1024, 512, (ctx, w, h) => {
    ctx.fillStyle = '#1d2026';
    ctx.fillRect(0, 0, w, h);

    // Shoulder button labels along top edge
    ctx.fillStyle = '#8b929c';
    ctx.font = 'bold 26px -apple-system, BlinkMacSystemFont, Arial, sans-serif';
    ctx.textAlign = 'center';
    ctx.fillText('R', 120, 34);
    ctx.fillText('ZR', 260, 34);
    ctx.fillText('ZL', w - 260, 34);
    ctx.fillText('L', w - 120, 34);

  });

  const rearPlateMesh = new THREE.Mesh(
    new THREE.PlaneGeometry(510, 255),
    new THREE.MeshPhysicalMaterial({
      map: backPlateTex,
      roughness: 0.42,
      metalness: 0.14,
      clearcoat: 0.25,
    })
  );
  rearPlateMesh.rotation.y = Math.PI;
  rearPlateMesh.position.set(0, -4, baseRearZ - 0.15);
  baseGroup.add(rearPlateMesh);

  // Captive backplate screws + 4 rubber grip feet on the rear base
  for (const sx of [-195, 195]) {
    const screw = new THREE.Mesh(
      new THREE.CylinderGeometry(4.8, 4.8, 1.5, 18),
      materials.metalPort
    );
    screw.rotation.x = Math.PI / 2;
    screw.position.set(sx, 78, baseRearZ - 0.2);
    baseGroup.add(screw);
  }

  for (const [fx, fy] of [
    [-238, 108],
    [238, 108],
    [-238, -112],
    [238, -112],
  ]) {
    const foot = new THREE.Mesh(
      new RoundedBoxGeometry(24, 10, 2.2, 2, 2.5),
      materials.rubberFoot
    );
    foot.position.set(fx, fy, baseRearZ - 0.8);
    baseGroup.add(foot);
  }

  // Count meshes & geometry stats for verification
  let meshCount = 0;
  root.traverse((obj) => {
    if (obj.isMesh) {
      meshCount++;
    }
  });

  return {
    root,
    lidGroup,
    hingeGroup,
    baseGroup,
    materials,
    interactiveButtons,
    meshCount,
    shellThickness: {
      lid: 24.9,
      base: 32.8,
      totalDepthSpan: 35.5,
    },
  };
}
