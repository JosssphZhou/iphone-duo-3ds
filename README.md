# iPhone Duo for Nintendo 3DS

[中文说明](README.zh-CN.md)

Drag the Music icon across the hinge. It becomes a widget on the other screen. Hold Music again, remove the widget, and watch it shrink back into the icon.

A native Nintendo 3DS homebrew UI toy with a companion browser prototype, inspired by the iPhone Duo launch interface. It recreates the interaction: it does not run iOS, execute iPhone apps, or play Apple Music.

**This is a source-only release.** The Apple imagery, app icons, screenshots, and other third-party visual assets used by the local demo are not included. A fresh clone can run the portable code tests, but it cannot reproduce the complete visual demo until a compatible, prepared local asset set is supplied. This repository does not provide that asset pack or a one-click downloader. See [Assets and rights](THIRD_PARTY.md).

## What it does

- Two-screen desktop: 400 × 240 on the upper display, 320 × 240 on the touchscreen.
- Long-press icon editing, drag-and-drop swapping, Dock placement, and two home pages.
- Music icon transfer to the upper screen, a removal menu on the touchscreen, and a return animation aimed at the icon's current position.
- Landscape and portrait book layouts with matching touch coordinates.
- Swipe-to-unlock lock screen and native-size icon labels.
- English and Chinese UI. The native app selects its initial language from the system and remembers an explicit selection separately from icon placement. The browser has its own language selector.
- Static app-preview scenes from a locally supplied asset set. Apps without a supplied preview stay on the desktop.

Only the lower physical screen accepts touch. In book orientation, hold the console with its upper screen on the left. The upper screen is then the left-hand display and the touchscreen is on the right.

## Controls

| Input | Action |
| --- | --- |
| Touch an icon / D-pad then A | Open its available preview |
| Hold an icon, then drag | Rearrange icons |
| Drag Music toward the hinge | Add the upper-screen Music widget |
| Hold Music while its widget is present | Show the removal menu; release, then tap the menu to remove |
| B / touch outside the removal menu | Cancel the menu |
| B | Finish icon editing or return to the desktop |
| L | Switch landscape / book layout; disable automatic rotation |
| R | Show sensor diagnostics |
| X | Switch day/night wallpaper |
| Y | Switch home pages |
| Hold SELECT, then tap Y | Switch English / Chinese |
| Press and release SELECT alone | Toggle the lock screen |
| Swipe upward from the touchscreen's bottom edge | Unlock |
| START | Exit to Homebrew Launcher |

The native HOME button remains controlled by the 3DS system. Automatic orientation is experimental; the physical sensor behavior still needs verification. Use L for a predictable orientation change.

## Repository layout

```text
iphone-duo-3ds-native/       C runtime, portable models, tests, asset preparation
iphone-duo-3ds-prototype/    Browser interaction prototype
tools/                     Asset import/check and portable test entry points
asset-manifest.json        Expected local visual files, sizes, and SHA-256 hashes
```

## Run the portable tests

Requires Python 3.9 or newer and a C11 compiler. No Apple assets or 3DS toolchain are needed for these tests.

```sh
bash tools/test_host.sh
python3 -m unittest discover -s tests -p '*_test.py' -v
```

Use `SANITIZE=1 bash tools/test_host.sh` for AddressSanitizer and UndefinedBehaviorSanitizer on supported compilers. CI also checks browser JavaScript syntax. These checks do not replace a console test.

## Supply local visual assets

You need an already prepared native `gfx/` directory and browser `assets/` directory matching `asset-manifest.json`. The manifest is a record of the demo's expected files, not a license to those files. An unprocessed Apple screenshot is not a compatible asset pack.

```sh
python3 tools/import_assets.py \
  --native /path/to/prepared/iphone-duo-3ds-native \
  --web /path/to/prepared/iphone-duo-3ds-prototype
python3 tools/check_assets.py
```

You can supply either `--native` or `--web` independently; the checker reports any remaining missing files. Importing never changes the source directories and never copies console saves. Files already matching the manifest are left alone. Differing destination files require `--replace`, which first saves them under `.asset-backups/`. Symlinks, paths outside the selected directories, and unlisted files are rejected. Imported visuals and backups are ignored by Git.

The asset preparation scripts document the local conversion pipeline, but complete source artwork, intermediate captures, and Apple fonts are not bundled. Regeneration is an advanced workflow requiring your own inputs. Check each script's `--help` and its source before running it.

## Build and run on 3DS

After supplying the native visual assets, install the devkitPro 3DS toolchain, including devkitARM, libctru, citro2d, citro3d, 3dstools, and tex3ds.

```sh
cd iphone-duo-3ds-native
./build.sh -j4
```

The outputs are `iphone-duo.3dsx` and `iphone-duo.smdh`. Copy them into `/3ds/iphone-duo/` on an SD card and launch with Homebrew Launcher. Use a console with a working homebrew entry point; this project does not install console firmware. Preserve existing `layout.bin` and `language.bin` when updating.

The visual layout uses prepared PNGs during the build and embeds the resulting atlases in RomFS. Do not commit a locally built binary containing third-party imagery as if it were covered by the code license.

## Browser prototype

After importing the browser assets, serve the repository locally:

```sh
python3 -m http.server 8000
```

Open `http://localhost:8000/iphone-duo-3ds-prototype/`. Use the language selector for English or Chinese. The browser preview demonstrates the interaction; it does not prove performance or input behavior on physical hardware.

## Current limits

The local v9.3 build has been installed on the developer's SD card, and the widget interaction has been exercised in a 3DS emulator. The subsequent bilingual build has not yet been verified on a physical console. Screenshots inside supplied app previews retain their original text. Preview content is offline; calls, accounts, downloads, and Apple services are not connected.

## License

Original project code is available under the [MIT License](LICENSE). Noto Sans has its own SIL Open Font License, retained with the font. Apple and Nintendo names, logos, artwork, and other third-party content are not covered by MIT. This is an independent project with no affiliation to Apple or Nintendo. See [THIRD_PARTY.md](THIRD_PARTY.md) for the source and license boundary.
