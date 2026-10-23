<div align="center">

# iPhone Duo for Nintendo 3DS

### Two screens. One very unlikely iPhone.

Native Nintendo 3DS homebrew that recreates the iPhone Duo desktop and dual-screen interactions.

[English](README.md) | [简体中文](README.zh-CN.md) | [Build for 3DS](#build-for-3ds) | [Install on Your SD Card](#install-on-your-sd-card) | [Controls](#controls)

[![Checks](https://github.com/JosssphZhou/iphone-duo-3ds/actions/workflows/ci.yml/badge.svg)](https://github.com/JosssphZhou/iphone-duo-3ds/actions)
![Platform](https://img.shields.io/badge/platform-Nintendo_3DS-222222)
![Languages](https://img.shields.io/badge/UI-English_%2F_中文-3478F6)
[![Code license](https://img.shields.io/badge/code-MIT-36A66B)](LICENSE)

</div>

## Build for 3DS

Follow the [official devkitPro setup guide](https://devkitpro.org/wiki/Getting_Started) to install the **devkitPro 3DS toolchain** (`devkitARM`, `libctru`, `citro2d`, `citro3d`, `3dstools`, and `tex3ds`). Prepared textures are already committed to the repository.

```sh
git clone https://github.com/JosssphZhou/iphone-duo-3ds.git
cd iphone-duo-3ds
python3 tools/check_assets.py
cd iphone-duo-3ds-native
./build.sh -j4
```

The build entry point is `iphone-duo-3ds-native/build.sh`. It produces `iphone-duo.3dsx` and `iphone-duo.smdh` in that directory. All prepared artwork is included; building does not require running the browser prototype or regenerating images.

The script defaults to `/opt/devkitpro`. For another installation location, set `DEVKITPRO` and `DEVKITARM` before building. The commands above require Git, Python 3.9+, make, and the installed 3DS toolchain.

## Install on Your SD Card

You need a 3DS with a working **Homebrew Launcher** and its SD card. Copy the two generated files into this directory on the card:

```text
SD:/
└── 3ds/
    └── iphone-duo/
        ├── iphone-duo.3dsx
        └── iphone-duo.smdh
```

When updating an existing installation, keep your `layout.bin` and `language.bin` files to preserve your saved icon layout and language choice.

Safely eject the SD card, put it back into the 3DS, and open the app in Homebrew Launcher. Artwork is embedded in the `.3dsx`; you do not need to copy the browser directory or `gfx/` onto the card.

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

## The Hinge Is the Trick

Hold **Music** until the icons start wiggling, then drag it toward the hinge. The icon leaves the touchscreen and expands into a Music widget on the other display.

Want it back on the home screen? Hold **Music** again, tap **Remove widget**, and watch it fold back into its icon slot, even if you have already moved that icon to another position or page.

| Make It Yours | Turn It Sideways | Familiar Details |
| --- | --- | --- |
| Long-press to enter edit mode, swap icon slots, and organize two home pages plus the Dock. | Press **L** for book orientation, placing the original upper screen on the left and the touchscreen on the right. | Swipe up to unlock, toggle day and night wallpapers, and switch the UI between English and Chinese. |

> **Scope note:** This project recreates the interface and motion design. It does not run iOS or iPhone apps. The Music widget and app previews are offline visual demos that do not play audio, place calls, or download software.

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

## Development Extras

<details>
<summary>Browser prototype and development screenshots</summary>

The browser prototype is a development aid for checking layouts and interactions. It is not part of the 3DS build or installation process. See its [separate instructions](iphone-duo-3ds-prototype/README.md).

<img src="docs/images/book-widget.png" width="820" alt="Browser prototype in book orientation, not a physical-console photograph">

*Browser prototype in book orientation.*

<img src="docs/images/native-emulator.png" width="760" alt="Native build running in Azahar for development validation">

*Native build running in Azahar for development validation.*

</details>

## Credits and Rights

Original project code is [MIT licensed](LICENSE). The included Apple reference imagery, icons, screenshots, artwork, and third-party marks **are not covered by the MIT License**, and all rights remain with their respective owners. Noto Sans retains its SIL Open Font License.

This is an independent fan project, not affiliated with or endorsed by Apple or Nintendo. See [THIRD_PARTY.md](THIRD_PARTY.md) for source attributions and asset notices.
