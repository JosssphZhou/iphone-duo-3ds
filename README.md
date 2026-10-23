<div align="center">

# iPhone Duo for Nintendo 3DS

### Two screens. One very unlikely iPhone.

A native Nintendo 3DS homebrew UI toy inspired by the iPhone Duo launch interface, paired with an interactive browser prototype.

[English](README.md) | [简体中文](README.zh-CN.md) | [Try It in Your Browser](#try-it-in-your-browser) | [Build for 3DS](#build-for-3ds)

[![Checks](https://github.com/JosssphZhou/iphone-duo-3ds/actions/workflows/ci.yml/badge.svg)](https://github.com/JosssphZhou/iphone-duo-3ds/actions)
![Platform](https://img.shields.io/badge/platform-Nintendo_3DS-222222)
![Languages](https://img.shields.io/badge/UI-English_%2F_中文-3478F6)
[![Code license](https://img.shields.io/badge/code-MIT-36A66B)](LICENSE)

<img src="docs/images/book-widget.png" width="820" alt="Browser prototype in book orientation: Music widget on the left display, touch-screen app icons on the right">

*Interactive browser prototype in book orientation. Native 3DS runtime shown below.*

</div>

## The Hinge Is the Trick

Hold **Music** until the icons start wiggling, then drag it toward the hinge. The icon leaves the touchscreen and expands into a Music widget on the other display.

Want it back on the home screen? Hold **Music** again, tap **Remove widget**, and watch it fold back into its icon slot, even if you have already moved that icon to another position or page.

| Make It Yours | Turn It Sideways | Familiar Details |
| --- | --- | --- |
| Long-press to enter edit mode, swap icon slots, and organize two home pages plus the Dock. | Press **L** for book orientation, placing the original upper screen on the left and the touchscreen on the right. | Swipe up to unlock, toggle day and night wallpapers, and switch the UI between English and Chinese. |

> **Scope note:** This project recreates the interface and motion design. It does not run iOS or iPhone apps. The Music widget and app previews are offline visual demos that do not play audio, place calls, or download software.

## Try It in Your Browser

**All prepared artwork is included.** Clone the repository, start a local server, and open the prototype with no asset import or package installation required.

```sh
git clone https://github.com/JosssphZhou/iphone-duo-3ds.git
cd iphone-duo-3ds
python3 -m http.server 8000
```

Open **http://localhost:8000/iphone-duo-3ds-prototype/** and interact with a mouse or touchscreen. Click **中文 / EN** to switch the UI language, or use **Recording stage** to enlarge the console frame for capture.

The browser edition is a companion prototype and does not measure physical 3DS performance. See its [controls and prototype notes](iphone-duo-3ds-prototype/README.md) for more details.

## Build for 3DS

Install the **devkitPro 3DS toolchain** (`devkitARM`, `libctru`, `citro2d`, `citro3d`, `3dstools`, and `tex3ds`). Prepared textures are already committed to the repository.

```sh
python3 tools/check_assets.py
cd iphone-duo-3ds-native
./build.sh -j4
```

Copy the two generated files to your SD card:

```text
SD:/
└── 3ds/
    └── iphone-duo/
        ├── iphone-duo.3dsx
        └── iphone-duo.smdh
```

Launch **iPhone Duo** from **Homebrew Launcher**. When updating an existing installation, keep your `layout.bin` and `language.bin` files to preserve your saved icon layout and language choice.

<img src="docs/images/native-emulator.png" width="760" alt="The native bilingual 3DS build running in Azahar, with English home-screen labels">

*Native C build running in Azahar (not the browser mockup). The bilingual v10 build has passed emulator checks; physical-console verification is still pending.*

## Controls

| Input | Action |
| --- | --- |
| Touch an icon / D-pad then A | Open its available app preview |
| Hold an icon, then drag | Enter edit mode and rearrange icons |
| Drag Music toward the hinge | Add the Music widget to the upper display |
| Hold Music while its widget is present | Open the removal menu; release, then tap the menu to remove the widget |
| B / touch outside the removal menu | Cancel the removal menu |
| B | Finish icon editing or return to the home screen |
| L | Switch landscape and book layout; disable automatic rotation |
| R | Toggle sensor diagnostics |
| X | Switch day and night wallpaper |
| Y | Switch home screen pages |
| Hold SELECT, then tap Y | Switch UI language between English and Chinese |
| Press and release SELECT alone | Toggle the lock screen |
| Swipe upward from the touchscreen's bottom edge | Unlock |
| START | Exit to Homebrew Launcher |

The hardware HOME button remains controlled by the 3DS system. Automatic orientation is experimental and its physical sensor behavior still needs console verification. Use **L** for predictable orientation changes.

## Under the Hood

The native app renders at **400 × 240** on the upper display and **320 × 240** on the touchscreen. Book orientation uses **240 × 400** and **240 × 320** logical layouts with matching touch-coordinate transforms. Only the lower physical screen accepts touch input.

Prepared textures, icons, and native-size text keep rendering crisp at 3DS resolution. The build packs texture atlases into RomFS, and English/Chinese preferences are stored separately from icon placement.

```text
iphone-duo-3ds-native/       C runtime, prepared textures, models, tests
iphone-duo-3ds-prototype/    Interactive browser prototype and artwork
docs/images/                 Browser and emulator screenshots
tools/                       Asset checks, optional import, host tests
asset-manifest.json          945 prepared assets with sizes and SHA-256 hashes
```

<details>
<summary><strong>Development checks</strong></summary>

Requires Python 3.9+ and a C11 compiler. No 3DS toolchain is needed for host tests.

```sh
python3 tools/check_assets.py
bash tools/test_host.sh
python3 -m unittest discover -s tests -p '*_test.py' -v
```

Run `SANITIZE=1 bash tools/test_host.sh` to enable AddressSanitizer and UndefinedBehaviorSanitizer on supported compilers. CI verifies artwork integrity, portable interaction regressions, import safety, and JavaScript syntax.

</details>

<details>
<summary><strong>Assets and regeneration</strong></summary>

The prepared image set is bundled in the repository: wallpapers, Apple app icons, static app previews, Music widget artwork, lock-screen visuals, and native textures. A standard clone is ready to preview and build immediately.

`asset-manifest.json` records every expected file size and SHA-256 hash. Restore accidentally modified files from Git, or use the optional importer to restore from a matching local asset directory:

```sh
python3 tools/import_assets.py \
  --native /path/to/prepared/iphone-duo-3ds-native \
  --web /path/to/prepared/iphone-duo-3ds-prototype
```

The importer validates hashes, rejects symlinks and unlisted files, and skips console save files. Overwriting existing assets requires `--replace` and creates a backup first. Raw asset regeneration is a separate workflow documented in [ASSET_INPUTS.md](iphone-duo-3ds-native/ASSET_INPUTS.md). Apple font software and full intermediate captures are not bundled.

</details>

## Status and Limits

- **Native bilingual v10:** Builds cleanly; language persistence, rotation controls, and Music widget add/remove flows have been verified in Azahar.
- **Physical 3DS hardware:** The earlier v9.3 build was copied to an SD card; bilingual v10 and automatic sensor rotation still await physical-console verification.
- **App previews:** App interiors use static reference screens where available. Apps without an official preview remain on the home screen, and text embedded inside screenshots keeps its original language.
- **Archived screens:** Experimental Music and App Store interiors are not part of the default preview path and are not fully translated.

## Credits and Rights

Original project code is [MIT licensed](LICENSE). The included Apple reference imagery, icons, screenshots, artwork, and third-party marks **are not covered by the MIT License**, and all rights remain with their respective owners. Noto Sans retains its SIL Open Font License.

This is an independent fan project, not affiliated with or endorsed by Apple or Nintendo. See [THIRD_PARTY.md](THIRD_PARTY.md) for source attributions and asset notices.
