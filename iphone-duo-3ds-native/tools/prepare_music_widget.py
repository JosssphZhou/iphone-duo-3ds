#!/usr/bin/env python3
"""Import the supplied Apple widget and calendar-free backdrops unchanged."""
import hashlib
import json
import shutil
import argparse
from asset_config import asset_dir
from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('source',nargs='?',type=Path,default=asset_dir('music-widget-assets'));parser.add_argument('--widget',type=Path,default=asset_dir('apple-music-widget.png'));parser.add_argument('--provenance',type=Path,default=asset_dir('apple-music-widget-source.json'));args=parser.parse_args()
source=args.source;widget=args.widget
inputs=[(widget,(180,180),'music-widget-official.png')]
for name,size in [('top_home_day.png',(400,240)),('top_home_night.png',(400,240)),('top_portrait_day.png',(240,400)),('top_portrait_night.png',(240,400))]:
    inputs.append((source/name,size,'widget-'+name))
for path,size,_ in inputs:
    with Image.open(path) as im:
        if im.size!=size or im.format!='PNG':raise ValueError(f'{path}: expected PNG {size}')
files=[];hashes={}
for path,_,name in inputs:
    shutil.copy2(path,root/'gfx'/name);files.append(name)
    hashes[name]=hashlib.sha256(path.read_bytes()).hexdigest()
(root/'gfx/music_widget.t3s').write_text('--atlas --border=edge -f rgba8888 -z auto\n'+'\n'.join(files)+'\n')
(root/'music-widget-assets.json').write_text(json.dumps({'method':'Unmodified copies of supplied PNGs','sha256':hashes},indent=2)+'\n')
provenance=args.provenance
if provenance.exists():shutil.copy2(provenance,root/'music-widget-provenance.json')
print('Imported 5 unchanged widget/background PNGs')
