# Third-Party Artwork, Dependencies, and Licenses

The [MIT license](LICENSE) applies to the original source code and documentation in this repository. It does **not** cover third-party images, screenshots, icons, fonts, artwork, or trademarks included with the project.

## Bundled Visual Assets

This repository includes pre-processed textures and reference visuals for both the native 3DS build and the browser prototype. `asset-manifest.json` lists all 945 bundled asset files along with their file sizes and SHA-256 checksums for integrity verification.

| Material | Source / Provenance | Location in Repository |
| --- | --- | --- |
| Home screen reference, app icons, wallpapers, app preview screens | [iPhone Duo product page](https://www.apple.com/iphone-duo/); documented in browser icon and official-scene manifests | `iphone-duo-3ds-prototype/assets/` |
| Apple Music widget and lock screen reference graphics | [Apple iPhone User Guide](https://support.apple.com/guide/iphone/welcome/ios); documented in `music-widget-provenance.json` and `lockscreen-provenance.json` | Browser `assets/` and native `gfx/` |
| SF Symbols raster exports | Documented in `assets/sf-symbols/symbols.json` | `iphone-duo-3ds-prototype/assets/sf-symbols/` |
| Pre-rendered labels, UI captures, resized icons, native frames | Generated via the asset pipeline described in [ASSET_INPUTS.md](iphone-duo-3ds-native/ASSET_INPUTS.md) and native provenance JSON files | `iphone-duo-3ds-native/gfx/` |
| Project screenshots | Captures from the browser prototype and the Azahar 3DS emulator | `docs/images/` |

All rights to the underlying Apple imagery and icons remain with Apple and their respective owners. Including these files in the repository for demonstration and build reproducibility does not make them open-source assets or grant permission for standalone reuse. Please consult [Apple's website terms](https://www.apple.com/legal/internet-services/terms/site.html) and the original sources regarding any use outside this project.

Proprietary Apple font files (such as SF Pro and PingFang) are not distributed in this repository, though some pre-rendered UI textures contain rasterized text produced during asset preparation.

## Fonts

`iphone-duo-3ds-native/tools/NotoSans.ttf` is distributed under the [SIL Open Font License](iphone-duo-3ds-native/tools/OFL.txt). Keep the license file intact when redistributing the font.

## Toolchain and Compiled Binaries

Building the native 3DS application requires external devkitPro packages (`libctru`, `citro2d`, and `citro3d`), and the asset preparation scripts require `Pillow`. These dependencies are installed separately and remain governed by their respective upstream licenses.

Because the 3DS build embeds `gfx/` textures into RomFS, any compiled `.3dsx` binary contains both MIT-licensed project code and third-party imagery. Automated CI checks source code and asset checksums only and does not publish prebuilt binaries.

Apple, iPhone, Apple Music, and related marks are trademarks of Apple Inc. Nintendo and Nintendo 3DS are trademarks of Nintendo. This is an independent fan project and is not affiliated with or endorsed by Apple or Nintendo.
