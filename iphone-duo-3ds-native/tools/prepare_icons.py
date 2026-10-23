#!/usr/bin/env python3
"""Extract existing Apple pixels directly at native grid/Dock sizes."""
from pathlib import Path
import hashlib,json,shutil,argparse
from asset_config import asset_dir
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--source',type=Path,default=asset_dir('prototype'));args=parser.parse_args()
source=args.source
manifest=json.loads((source/'apple-icons/manifest.json').read_text())
photo=Image.open(source/'launch-home.jpg').convert('RGB')
stems=['facetime','calendar','photos','camera','mail','notes','clock','maps','apple-tv','news','app-store','games','health','wallet','siri','settings','phone','safari','messages','music','calculator']
files=[]
for size in [37,32]:
    mask=Image.new('L',(size*4,size*4));ImageDraw.Draw(mask).rounded_rectangle((0,0,size*4-1,size*4-1),radius=size*4*115/512,fill=255)
    mask=mask.resize((size,size),Image.Resampling.LANCZOS)
    for stem in stems:
        spec=manifest['icons'][stem]
        if 'source_quad' in spec:
            tl,tr,br,bl=spec['source_quad']
            tile=photo.transform((size,size),Image.Transform.QUAD,tuple(tl+bl+br+tr),Image.Resampling.BICUBIC).convert('RGBA')
        else:
            tile=Image.open(source/'apple-icons/calculator-original.jpg').convert('RGBA').resize((size,size),Image.Resampling.LANCZOS)
        tile.putalpha(mask);name=f'iconnative-{size}-{stem}.png';tile.save(root/'gfx'/name);files.append(name)
(root/'gfx/icons_native.t3s').write_text('--atlas --border=edge -f rgba8888 -z auto\n'+'\n'.join(files)+'\n')
(root/'icon-native-provenance.json').write_text(json.dumps({'source':manifest['source'],'source_sha256':hashlib.sha256((source/'launch-home.jpg').read_bytes()).hexdigest(),'method':'Apple press-image pixels mapped from the original recorded quads directly to 37 or 32 pixels; rounded alpha mask only. Calculator from existing original 512px Apple icon. No redesign.','sizes':[37,32],'order':stems},indent=2)+'\n')
review=root/'icon-review';review.mkdir(exist_ok=True)
selected=['facetime','calendar','photos','camera','maps','app-store','settings','music']
canvas=Image.new('RGB',(224,len(selected)*55+26),(95,111,119));d=ImageDraw.Draw(canvas)
d.text((4,5),'OLD 37',fill='white');d.text((60,5),'NEW 37',fill='white');d.text((116,5),'OLD 32',fill='white');d.text((172,5),'NEW 32',fill='white')
for row,stem in enumerate(selected):
    old=Image.open(root/'gfx'/('icon-'+stem+'.png')).convert('RGBA');shutil.copy2(root/'gfx'/('icon-'+stem+'.png'),review/('before-'+stem+'.png'))
    for col,size in enumerate([37,37,32,32]):
        im=old.resize((size,size),Image.Resampling.BILINEAR) if col%2==0 else Image.open(root/'gfx'/f'iconnative-{size}-{stem}.png')
        canvas.paste(im,(col*56+(56-size)//2,30+row*55),im)
canvas.save(review/'icons-before-after-1x.png')
canvas.resize((canvas.width*4,canvas.height*4),Image.Resampling.NEAREST).save(review/'icons-before-after-4x.png')
print('Generated 42 direct-size icons; old comparison columns approximate old GPU scaling, not runtime screenshots')
