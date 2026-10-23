#!/usr/bin/env python3
"""Import final English desktop backgrounds without modifying source pixels."""
from pathlib import Path
import argparse,shutil,json,hashlib
from PIL import Image
from asset_config import asset_dir
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('source',nargs='?',type=Path,default=asset_dir('english-assets'));a=p.parse_args()
names=['top_home_day','top_home_night','top_portrait_day','top_portrait_night','widget-top_home_day','widget-top_home_night','widget-top_portrait_day','widget-top_portrait_night','bottom_home_day','bottom_home_night','bottom_home_page2_day','bottom_home_page2_night','bottom_portrait_day','bottom_portrait_night','bottom_portrait_page2_day','bottom_portrait_page2_night']
for name in names:
 size=(240,400) if 'top_portrait' in name else (240,320) if 'bottom_portrait' in name else (400,240) if 'top_home' in name else (320,240)
 with Image.open(a.source/(name+'.png')) as im:
  if im.size!=size:raise SystemExit(f'{name}: expected {size}, found {im.size}')
files=[];hashes={}
for name in names:
 path=a.source/(name+'.png');target=root/'gfx'/('english-'+name+'.png');shutil.copy2(path,target);files.append(target.name);hashes[target.name]=hashlib.sha256(path.read_bytes()).hexdigest()
for group in range(4):
 (root/'gfx'/f'english{group}.t3s').write_text('--atlas --border=edge -f rgba8888 -z auto\n'+'\n'.join(files[group*4:group*4+4])+'\n')
(root/'english-assets.json').write_text(json.dumps({'files':files,'sha256':hashes},indent=2)+'\n')
print('Imported 16 pixel-identical English backgrounds')
