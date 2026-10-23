# Native Artwork and Asset Pipeline

All pre-rendered textures required to build the 3DS application are already committed under `gfx/`. A standard clone can be built immediately without running the asset preparation scripts.

See the root [THIRD_PARTY.md](../THIRD_PARTY.md) for asset attributions and license terms. Noto Sans (`tools/NotoSans.ttf`, licensed under `tools/OFL.txt`) is the only bundled font file; do not commit proprietary system fonts such as SF or PingFang to the repository.

## Importing Prepared Assets

If you need to sync or restore prepared native textures from another local checkout, run the asset importer from the destination repository root:

```sh
python3 tools/import_assets.py --native /path/to/native-project
```

This copies all PNGs and `.t3s` atlas definitions under `gfx/` and verifies every referenced image against the root `asset-manifest.json`. Running `make` then compiles the RomFS `.t3x` texture atlases using the checked-in C headers and registries.

### Texture Atlas Overview

| Atlas Definition | Contents |
|---|---|
| `ui.t3s` | Landscape upper/lower day & night backgrounds, two Photos preview images, 95 ASCII glyphs, 21 legacy icons, 21 legacy labels, and page-two lower backgrounds |
| `portrait.t3s` | Book-mode upper day & night backgrounds and lower page-one/page-two day & night backgrounds |
| `labels.t3s` | 21 Chinese and 21 English app labels (`56×16` px) |
| `icons_native.t3s` | 21 app icons pre-scaled to native `37×37` and `32×32` px |
| `music_widget.t3s` | `180×180` Apple Music widget texture and four calendar-free upper backgrounds |
| `lockscreen.t3s` | Eight native-resolution lock screen frames (upper/lower × landscape/book × day/night) |
| `english0.t3s` – `english3.t3s` | Sixteen English desktop backgrounds (standard and calendar-free upper screens, plus page-one/page-two lower screens) |
| `official_*.t3s` | Four native-resolution preview frames per supported app (FaceTime, Photos, Mail, Notes, Siri, TV) |
| `showcase_*.t3s` | Archived Music and App Store frames and overlays (retained for compatibility; inactive in default preview mode) |

## Regenerating Textures from Raw Captures

To rebuild the prepared PNGs in `gfx/` from raw source captures, set `DUO_ASSET_ROOT=/path/to/inputs` or pass explicit path flags to the scripts in `tools/`. Every script supports `--help`.

| Script | Default Inputs under `DUO_ASSET_ROOT` | CLI Overrides |
|---|---|---|
| `prepare_assets.py` | `prototype/`: `desert-day.jpg`, `desert-night.jpg`, `launch-home.jpg` | `--source`, optional `--font` |
| `prepare_icons.py` | `prototype/`: `launch-home.jpg`, `apple-icons/manifest.json` (source quads), `apple-icons/calculator-original.jpg` | `--source` |
| `prepare_labels.py` | Keeps existing Chinese label PNGs; renders English labels with `tools/NotoSans.ttf` | `--en-font`, optional `--font` and `--font-index` for Chinese |
| `prepare_music_widget.py` | `music-widget-assets/`, `apple-music-widget.png`, `apple-music-widget-source.json` | positional source path, `--widget`, `--provenance` |
| `prepare_lockscreen.py` | `lockscreen/` (lock/flashlight/camera PNGs), `lockscreen-assets/` (eight continuous wallpaper PNGs), `lockscreen-sources.json` | `--source`, `--wallpapers`, `--font`, `--provenance` |
| `prepare_english.py` | `english-assets/` (sixteen final desktop PNGs) | positional source path |
| `prepare_official.py` | `official-assets/` (versioned manifest and final app preview frames) | positional source path |
| `prepare_showcase.py` | `music-store-assets/` (manifest and referenced frames/overlays) | positional source path |

The sixteen English background filenames in `english-assets/` are `top_home_day/night`, `top_portrait_day/night`, `widget-top_home_day/night`, `widget-top_portrait_day/night`, `bottom_home_day/night`, `bottom_home_page2_day/night`, `bottom_portrait_day/night`, and `bottom_portrait_page2_day/night` (all `.png`).

Target screen dimensions are `400×240` (upper) and `320×240` (lower) in landscape mode, and `240×400` (upper) and `240×320` (lower) in book mode. Embedded text inside official app preview screenshots is preserved as-is.
