# Artwork input contract

Prepared artwork is imported locally and is excluded from the public source distribution. No tool downloads it or grants redistribution rights. Keep the original source/provenance records with any privately held inputs. Noto Sans (`tools/NotoSans.ttf`, `tools/OFL.txt`) is the only supplied font. Do not copy SF or PingFang font files into the source repository.

## Minimal path: import an existing prepared native project

Run `python3 tools/import_assets.py --from-project /path/to/native-project` from the destination checkout. It copies all PNGs and `.t3s` atlas descriptions under `gfx/` and validates every referenced image. This is sufficient to build with the checked-in generated C headers and registries; generated RomFS textures are rebuilt by `make`. No regeneration of proprietary font glyphs or original website artwork is required.

Prepared groups:

| Atlas | Required prepared images |
|---|---|
| `ui.t3s` | Landscape desktop top/bottom day/night; two photo images; 95 ASCII glyphs; 21 original icons; 21 legacy labels; page-two lower backgrounds |
| `portrait.t3s` | Portrait top day/night; lower two pages day/night |
| `labels.t3s` | 21 Chinese + 21 English 56×16 labels |
| `icons_native.t3s` | 21 icons at each native 37×37 and 32×32 size |
| `music_widget.t3s` | Original 180×180 Apple widget and four calendar-free backgrounds |
| `lockscreen.t3s` | Eight final native-size top/bottom × orientation × day/night frames |
| `english0.t3s` through `english3.t3s` | Sixteen English desktop normal/calendar-free top and page-one/page-two bottom backgrounds |
| `official_*.t3s` | Four final native-size frames for each supported app: FaceTime, Photos, Mail, Notes, Siri, TV |
| `showcase_*.t3s` | Legacy optional Music/Store prepared screenshots and overlays referenced by the existing atlas descriptions; inactive in the default official-preview mode |

The exact local list is defined by the `.t3s` descriptions; `asset-input-manifest.json` records the current paths and SHA256 values. Keeping these descriptions alongside private artwork avoids relying on absolute paths or machine-specific font installations.

## Raw regeneration inputs

Use `DUO_ASSET_ROOT=/path/to/inputs` or explicit options. All scripts support `--help`.

| Script | Default under asset root | Override |
|---|---|---|
| `prepare_assets.py` | `prototype/`: desert-day.jpg, desert-night.jpg, launch-home.jpg | `--source`; optional `--font` |
| `prepare_icons.py` | `prototype/`: launch-home.jpg, apple-icons/manifest.json with source quads, apple-icons/calculator-original.jpg | `--source` |
| `prepare_labels.py` | Retains prepared Chinese label PNGs; generates English using tools/NotoSans.ttf | `--en-font`; optional local `--font` and `--font-index` for Chinese |
| `prepare_music_widget.py` | `music-widget-assets/`, `apple-music-widget.png`, `apple-music-widget-source.json` | positional source, `--widget`, `--provenance` |
| `prepare_lockscreen.py` | `lockscreen/` with lock/flashlight/camera PNGs, `lockscreen-assets/` with eight continuous wallpaper PNGs, `lockscreen-sources.json` | `--source`, `--wallpapers`, `--font`, `--provenance` |
| `prepare_english.py` | `english-assets/` with sixteen final PNGs | positional source |
| `prepare_official.py` | `official-assets/` containing versioned manifest and final frames | positional source |
| `prepare_showcase.py` | `music-store-assets/` containing manifest and all referenced frames/overlays | positional source |

The English asset filenames are top_home_day/night, top_portrait_day/night, widget-top_home_day/night, widget-top_portrait_day/night, bottom_home_day/night, bottom_home_page2_day/night, bottom_portrait_day/night, bottom_portrait_page2_day/night, all with `.png`. Native dimensions are unchanged: landscape 400×240 upper, 320×240 lower; book 240×400 upper, 240×320 lower. Official screenshot text is not translated or painted over.
