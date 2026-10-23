"""Rectify final-size icons directly from locally retained Apple source pixels."""
from pathlib import Path
from PIL import Image, ImageDraw
import json, os
root=Path(__file__).resolve().parents[1];src=root/'assets/apple-icons';meta=json.loads((src/'manifest.json').read_text());raw=Image.open(root/'assets/launch-home.jpg').convert('RGB');out=root/'assets/apple-icons-sized';out.mkdir(exist_ok=True)
mask=Image.new('L',(512,512));ImageDraw.Draw(mask).rounded_rectangle((0,0,511,511),radius=115,fill=255)
for size in [32,37,64,74,96,111]:
 d=out/str(size);d.mkdir(exist_ok=True)
 for name,entry in meta['icons'].items():
  if 'source_quad' in entry:
   tl,tr,br,bl=entry['source_quad'];im=raw.transform((size,size),Image.Transform.QUAD,tuple(tl+bl+br+tr),Image.Resampling.BICUBIC).convert('RGBA')
  else:im=Image.open(src/'calculator-original.jpg').convert('RGBA').resize((size,size),Image.Resampling.LANCZOS)
  im.putalpha(mask.resize((size,size),Image.Resampling.LANCZOS));im.save(d/(name+'.png'))
  if size in [32,37]:
   native=Path(os.environ.get('DUO_NATIVE_ICON_OUTPUT',str(root/'.local-assets/icon-native')))/str(size);native.mkdir(parents=True,exist_ok=True);im.save(native/(name+'.png'))
(out/'manifest.json').write_text(json.dumps({'source':meta['source'],'localSource':'../launch-home.jpg','sizes':[32,37,64,74,96,111],'method':'Each final size rectified independently from original press pixels, never from 128px intermediate. Calculator independently downsampled from retained official original.'},indent=2))
