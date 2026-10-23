#!/usr/bin/env python3
"""Import privately held artwork into a source checkout; never downloads media."""
import argparse,json,shutil
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--from-project',required=True,type=Path,help='Existing native project with prepared gfx/ assets');p.add_argument('--prototype',type=Path,help='Optional prototype assets directory for source regeneration');p.add_argument('--destination',type=Path,default=root)
a=p.parse_args();source=a.from_project.resolve();destination=a.destination.resolve()
if not (source/'gfx').is_dir():raise SystemExit('Source must contain prepared gfx/')
count=0
for path in (source/'gfx').rglob('*'):
 if path.is_file() and path.suffix.lower() in ('.png','.t3s'):
  target=destination/path.relative_to(source);target.parent.mkdir(parents=True,exist_ok=True)
  if target.resolve()!=path.resolve():shutil.copy2(path,target)
  count+=1
for path in source.glob('*provenance.json'):
 target=destination/path.name
 if target.resolve()!=path.resolve():shutil.copy2(path,target)
if a.prototype:
 target=destination/'asset-input/prototype'
 if target.resolve()!=a.prototype.resolve():shutil.copytree(a.prototype,target,dirs_exist_ok=True)
missing=[]
for atlas in (destination/'gfx').glob('*.t3s'):
 for line in atlas.read_text().splitlines():
  if line and not line.startswith('--') and not (atlas.parent/line).is_file():missing.append(str(atlas.parent/line))
if missing:raise SystemExit('Missing atlas sources:\n'+'\n'.join(missing))
print(f'Imported/validated {count} private asset files. No network access; no font files imported.')
