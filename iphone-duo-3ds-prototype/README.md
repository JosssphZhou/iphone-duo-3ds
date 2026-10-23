# iPhone Duo on Nintendo 3DS: web prototype

A small interactive desktop for a two-screen Nintendo 3DS mockup. Open `index.html` locally; no package installation or server is required. This folder is the browser prototype, not the native 3DS build.

## Bundled artwork

Prepared Apple reference images, icons, wallpapers, and app-preview screenshots are included in `assets/`. Clone the repository and open this prototype directly. See the root [README](../README.md) for a local HTTP server command and [THIRD_PARTY.md](../THIRD_PARTY.md) for sources and the separate rights notice.

If files are missing or modified, run `python3 tools/check_assets.py` from the repository root. The prototype displays an asset-import notice when required images cannot load. The importer remains available for restoring a compatible local set.

## Language

The interface starts in Chinese for a Chinese browser locale and English otherwise. The **中文 / EN** button switches languages and remembers the choice locally. App identities and saved layouts stay unchanged. Search accepts both English and Chinese app names.

The default official-image mode is localized: desktop labels, accessibility labels, help, search, messages and the widget menu. Text inside original Apple screenshots is retained. Archived experimental Music/App Store screens are developer export references and are not fully translated; they are not the default user path.

## Controls

- Click or tap an icon. Arrow keys select; **A / Enter** opens; **B / Esc / HOME** returns.
- **L** or the orientation button changes between landscape and book-style portrait.
- **X** changes day/night wallpaper. **Y** changes the desktop page.
- **START** locks. Swipe up at least 64 logical pixels from the bottom edge to unlock; taps and A do not unlock.
- Hold an icon for 450 ms, then drag to rearrange. Hold near a page edge to change page. Layouts are stored locally.
- Drag Music toward the main screen to add its widget. With the widget present, hold Music without moving to show **Remove widget**; release, then click the menu item to remove it. Continuing to move performs normal rearrangement.
- **Recording stage** enlarges the console and hides explanatory text. It keeps the web-prototype badge and never plays a scripted demo.

The app screens and music widget are static visual references. They do not play music, place calls, send messages or install applications.

## Developer exports

- `scripts/export_english.mjs` exports English desktop textures from a supplied Ego Page. Pass `page`, `projectDir` and `outputDir` explicitly.
- `scripts/adapt_official_screens_v8.py --source PATH --output PATH --native-output PATH` adapts user-supplied rectified source images. Default intermediates are in `.local-assets/`. Set `DUO_FONT` when the platform has no supported default TrueType font.
- `scripts/export_icon_sizes.py` rectifies each target size directly from the original source. Set `DUO_NATIVE_ICON_OUTPUT` to select the native export directory.

Original app identities, validated layout permutations and language preferences use separate local-storage entries. The one-time Music/Mail layout migration preserves all other slots.
