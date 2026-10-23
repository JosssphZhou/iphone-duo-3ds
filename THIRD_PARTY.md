# Assets and rights

The root MIT license applies to original project code and documentation. It does not grant rights to third-party trademarks, photographs, app icons, fonts, screenshots, or UI artwork.

## What this source release includes

- Original C runtime and interaction models, browser code, tests, conversion scripts, and atlas definitions.
- Provenance and expected-file metadata, including hashes in `asset-manifest.json`.
- Noto Sans font software, with its existing [SIL Open Font License](iphone-duo-3ds-native/tools/OFL.txt). The license notice identifies the Noto Project Authors.

## What stays outside the repository

- Apple app icons and screenshots extracted from the [iPhone Duo product page](https://www.apple.com/iphone-duo/).
- Apple Music widget and lock-screen artwork referenced from [Apple's iPhone User Guide](https://support.apple.com/guide/iphone/welcome/ios).
- SF Symbols image data, including the browser's `symbols-data.js` carrier.
- Apple fonts and rasterized text or UI captures made with locally installed Apple fonts.
- Photographs, wallpapers, album/app artwork, native texture atlases, screenshots, and compiled demo binaries containing those materials.
- Console saves, local SD backups, machine-specific logs, and private build records.

The earlier local demo used Apple screenshots and artwork for visual reference. Those images are not represented as original work or as licensed for public redistribution. See [Apple's website terms](https://www.apple.com/legal/internet-services/terms/site.html) for Apple's stated terms. Availability on a public web page is not an open-source license.

The project retains source URLs and conversion code where available so the origin of local materials is visible. The checked-in manifest does not contain the image bytes. A matching hash is an integrity check, not evidence of permission.

## Local imports

`tools/import_assets.py` accepts an existing prepared asset set, verifies its bytes against the manifest, and copies only the listed visual files. It does not download materials or obtain usage rights. The repository does not include a ready-to-use asset pack. Users must supply appropriate inputs and determine whether their intended use and redistribution are permitted.

The native build embeds visual assets in RomFS, so a binary built from imported artwork contains that artwork. The code license alone does not authorize publishing such a binary. The browser's local image files are subject to the same boundary.

## Toolchain

Native builds link the separately installed devkitPro libraries libctru, citro2d, and citro3d. These dependencies are not vendored here; retain their upstream license requirements when distributing linked software. Asset scripts use Pillow, installed separately. GitHub Actions only runs portable source checks and does not fetch or publish third-party artwork.

Apple, iPhone, Apple Music, and related names are trademarks of their respective owners. Nintendo and Nintendo 3DS are trademarks of Nintendo. This project is independent and is not endorsed by either company.
