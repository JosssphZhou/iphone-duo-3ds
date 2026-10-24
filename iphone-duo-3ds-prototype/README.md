# iPhone Duo on Nintendo 3DS — Web Prototype

An interactive browser prototype used to design and test the dual-screen layout and interactions before porting to native 3DS hardware.

Serve this directory over HTTP for the interactive 3D enclosure. No build step or package installation is needed:

```sh
cd iphone-duo-3ds-prototype
python3 -m http.server 8080
```

Open `http://localhost:8080/`. The original model is generated locally with bundled Three.js modules; the screens remain live HTML. Drag the enclosure or background to orbit, or pick **Angled** or **Back** in the **Menu**. Pick **Front** to align the screens and enable touch interactions. **L** still changes the console orientation.

WebGL initialization or context failure restores the 2D interface. Add `?webgl=0` to request it explicitly. Opening `index.html` as a local file uses the 2D interface because browsers restrict local JavaScript module loading.

For the native 3DS application, see the root [README](../README.md).

## Bundled Assets

All required wallpapers, icons, and static app previews are included under `assets/`. See [THIRD_PARTY.md](../THIRD_PARTY.md) for source attributions and license details.

To verify asset integrity from the repository root, run `python3 tools/check_assets.py`. If required images are missing or modified, the prototype displays an on-screen asset notice.

## Language Support

Choose **English, 简体中文, 日本語, 한국어, Español, Français, Deutsch, or Português** from **Menu > Language**. The first visit follows your browser language preferences, falling back to English. An explicit choice is saved in `localStorage`, independently of your icon layout. App search accepts names in any supported language and ignores accents. Dates follow the selected locale.

UI labels, accessibility labels, help, search, status messages, lock-screen dates, and the widget menu are localized. Text embedded inside reference Apple screenshots remains in its original language. Archived Music and App Store mockups are kept as developer export references and are not part of the default user flow.

## Controls

- **Open / close apps:** Click or tap an icon, or select with the arrow keys and press **A / Enter**. Swipe up from the bottom edge, or press **B / Esc**, to return to the home screen.
- **Rotate console:** Press **L** or click the rotate button next to **Menu** to switch between landscape and book-style portrait mode.
- **Theme and pages:** Press **X** to toggle day/night wallpaper, and **Y** to switch home screen pages.
- **Lock screen:** Press **START** to lock. Swipe up at least 64 logical pixels from the bottom edge to unlock.
- **Rearrange icons:** Hold an icon for 450 ms, then drag to move it. Dragging near a screen edge flips to the adjacent page. Custom layouts are persisted in `localStorage`.
- **Music widget:** Drag **Music** across the hinge toward the upper screen to expand it into a widget. When the widget is active, hold **Music** without dragging to open the **Remove widget** menu, then release and click the menu item to fold the widget back.
- **Recording stage:** Enlarges the console frame and hides surrounding explanatory text for clean screen captures.

> **Note:** App interiors and the Music widget are static visual demos and do not play audio, place calls, send messages, or install apps.

## Asset Export Scripts

These developer scripts prepare and export textures for the native 3DS build:

- `scripts/export_english.mjs` renders and exports English desktop textures from an Ego Page instance (requires explicit `page`, `projectDir`, and `outputDir` arguments).
- `scripts/adapt_official_screens_v8.py --source PATH --output PATH --native-output PATH` adapts rectified source screenshots into target resolutions, writing intermediate files to `.local-assets/`. Set `DUO_FONT` if the host system lacks the default TrueType font.
- `scripts/export_icon_sizes.py` resamples icons directly from source images for each target resolution. Set `DUO_NATIVE_ICON_OUTPUT` to specify the native export directory.
