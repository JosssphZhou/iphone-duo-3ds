#!/usr/bin/env python3
"""Import supplied website crops unchanged. Does not crop or annotate images."""
import json
import shutil
import sys
from pathlib import Path
import argparse
from asset_config import asset_dir
from PIL import Image

root = Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('source',nargs='?',type=Path,default=asset_dir('official-assets'));args=parser.parse_args()
source=args.source
manifest = json.loads((source / 'manifest.json').read_text())
if manifest.get('version') not in (1, 2):
    raise ValueError('Unsupported official manifest version')
app_ids = {
    'facetime':'APP_FACETIME', 'calendar':'APP_CALENDAR', 'photos':'APP_PHOTOS',
    'camera':'APP_CAMERA', 'mail':'APP_MAIL', 'notes':'APP_NOTES', 'clock':'APP_CLOCK',
    'maps':'APP_MAPS', 'tv':'APP_TV', 'news':'APP_NEWS', 'store':'APP_STORE',
    'games':'APP_GAMES', 'health':'APP_HEALTH', 'wallet':'APP_WALLET', 'siri':'APP_SIRI',
    'settings':'APP_SETTINGS', 'phone':'APP_PHONE', 'safari':'APP_SAFARI',
    'messages':'APP_MESSAGES', 'music':'APP_MUSIC', 'calculator':'APP_CALCULATOR',
}
views = [('landscape','top',(400,240)),('landscape','bottom',(320,240)),
         ('portrait','top',(240,400)),('portrait','bottom',(240,320))]
# Validate the entire input before updating the runtime registry.
seen = set()
prepared = []
for app in manifest['apps']:
    key = app['id']
    if key not in app_ids or key in seen:
        raise ValueError('Unknown or duplicated app id: ' + key)
    seen.add(key)
    files = []
    for orientation, screen, expected in views:
        path = source / app[orientation][screen]
        with Image.open(path) as image:
            if image.format != 'PNG' or image.size != expected:
                raise ValueError(f'{path}: expected PNG {expected}, got {image.format} {image.size}')
        files.append(path)
    prepared.append((key, files))
output = root / 'gfx' / 'official'
output.mkdir(exist_ok=True)
entries = []
for key, files in prepared:
    names = []
    for index, path in enumerate(files):
        name = f'{key}-{index}.png'
        shutil.copy2(path, output / name)
        names.append('official/' + name)
    atlas = 'official_' + key
    (root / 'gfx' / (atlas + '.t3s')).write_text(
        '--atlas --border=edge -f rgba8888 -z auto\n' + '\n'.join(names) + '\n')
    entries.append('{' + app_ids[key] + ',' + json.dumps('romfs:/gfx/' + atlas + '.t3x') + '}')
lines = ['/* Generated from supplied official crops; no image edits. */', '#include "official_assets.h"']
if entries:
    lines += ['static const OfficialAppAssets assets[]={' + ','.join(entries) + '};',
              'const OfficialAppAssets *officialApps=assets;',
              'const size_t officialAppCount=sizeof(assets)/sizeof(assets[0]);']
else:
    lines += ['const OfficialAppAssets *officialApps=NULL;', 'const size_t officialAppCount=0;']
(root / 'source' / 'official_assets.c').write_text('\n'.join(lines) + '\n')
shutil.copy2(source / 'manifest.json', root / 'official-manifest.json')
print(f'Imported {len(entries)} official app(s); PNG bytes preserved')
