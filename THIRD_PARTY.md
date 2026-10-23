# Artwork, dependencies, and rights

The [MIT license](LICENSE) covers original project code and documentation. It does **not** relicense third-party images, screenshots, icons, fonts, artwork, or trademarks bundled with this demonstration.

## Included demo artwork

The repository includes the prepared visuals used by the native build and browser prototype. `asset-manifest.json` records 945 files with their paths, sizes, and SHA-256 hashes. These hashes establish file integrity, not ownership or permission.

| Material | Original source / provenance | Included location |
| --- | --- | --- |
| Launch interface, app icons, wallpapers, app preview scenes | [iPhone Duo product page](https://www.apple.com/iphone-duo/); browser icon and official-scene manifests | `iphone-duo-3ds-prototype/assets/` |
| Apple Music widget and lock-screen reference artwork | [Apple iPhone User Guide](https://support.apple.com/guide/iphone/welcome/ios); native `music-widget-provenance.json` and `lockscreen-provenance.json` | Browser assets and native `gfx/` |
| SF Symbols image representations | Browser `assets/sf-symbols/symbols.json` | Browser `assets/sf-symbols/` |
| Rasterized labels, UI captures, resized icons, prepared frames | Native provenance JSON files and `ASSET_INPUTS.md` document the conversion pipeline | `iphone-duo-3ds-native/gfx/` |
| Project demonstration screenshots | Browser prototype and Azahar emulator captures; captions identify their origin | `docs/images/` |

Apple and other respective rights holders retain their rights to the underlying imagery. Bundling the files does not make them open-source assets or grant a separate license to reuse them. This project does not claim permission from, affiliation with, or endorsement by Apple. Refer to [Apple's website terms](https://www.apple.com/legal/internet-services/terms/site.html) and the original sources when assessing use outside this project.

Original Apple font software (including SF and PingFang) is not included. Some prepared UI images contain rasterized text from the original local rendering pipeline. Console saves, SD backups, personal data, and machine-specific build records are not part of the repository.

## Font software

Noto Sans is included at `iphone-duo-3ds-native/tools/NotoSans.ttf`, under the [SIL Open Font License](iphone-duo-3ds-native/tools/OFL.txt). Retain that license when redistributing the font.

## Toolchain and generated binaries

Native builds use separately installed devkitPro libraries: libctru, citro2d, and citro3d. These dependencies are not vendored here; their upstream licenses continue to apply. Asset-preparation scripts use Pillow, installed separately.

The native build embeds the prepared artwork in RomFS. A generated binary therefore contains third-party imagery as well as project code; the code's MIT license does not apply to the imagery inside it. CI verifies the checked-in artwork and source code but does not publish compiled binaries.

Apple, iPhone, Apple Music, and related names are trademarks of their respective owners. Nintendo and Nintendo 3DS are trademarks of Nintendo. This is an independent fan project.
