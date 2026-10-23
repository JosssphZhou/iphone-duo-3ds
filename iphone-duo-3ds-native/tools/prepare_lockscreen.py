#!/usr/bin/env python3
"""Native-size lock frames from a supplied font and original Apple glyphs."""
from pathlib import Path
import json,shutil,argparse
from asset_config import asset_dir
from PIL import Image,ImageDraw,ImageFont
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--source',type=Path,default=asset_dir('lockscreen'));parser.add_argument('--wallpapers',type=Path,default=asset_dir('lockscreen-assets'));parser.add_argument('--font',type=Path,default=root/'tools/NotoSans.ttf');parser.add_argument('--provenance',type=Path,default=asset_dir('lockscreen-sources.json'));args=parser.parse_args()
source=args.source;font=str(args.font)
def typeface(size,bold=False):
    f=ImageFont.truetype(font,size)
    try:f.set_variation_by_name('Semibold' if bold else 'Regular')
    except (OSError,ValueError):pass
    return f
def centered(d,text,cx,cy,font):
    b=d.textbbox((0,0),text,font=font)
    d.text((round(cx-(b[2]-b[0])/2-b[0]),round(cy-(b[3]-b[1])/2-b[1])),text,font=font,fill='white')
def glyph(canvas,name,box):
    im=Image.open(source/(name+'.png')).convert('RGBA');im=im.crop(im.getbbox())
    im.thumbnail((box[2],box[3]),Image.Resampling.LANCZOS)
    canvas.alpha_composite(im,(round(box[0]+(box[2]-im.width)/2),round(box[1]+(box[3]-im.height)/2)))
wallpapers=args.wallpapers
files=[]
for night in [False,True]:
  for portrait in [False,True]:
    orientation='portrait' if portrait else 'landscape'
    theme='night' if night else 'day'
    w,h=(240,400) if portrait else (400,240)
    im=Image.open(wallpapers/f'wallpaper-{orientation}-{theme}-top.png').convert('RGBA');d=ImageDraw.Draw(im)
    glyph(im,'lock',(w/2-6,(38 if portrait else 18)-8,12,16))
    centered(d,'Wednesday, April 1',w/2,70 if portrait else 43,typeface(14 if portrait else 13))
    centered(d,'9:41',w/2,131 if portrait else 99,typeface(88 if portrait else 80,True))
    name=f'lock-top-{orientation}-{theme}.png';im.save(root/'gfx'/name);files.append(name)
    w,h=(240,320) if portrait else (320,240)
    im=Image.open(wallpapers/f'wallpaper-{orientation}-{theme}-bottom.png').convert('RGBA')
    controls=Image.new('RGBA',im.size);d=ImageDraw.Draw(controls)
    for x,name in [(24,'flashlight'),(w-64,'camera')]:
        y=h-74;d.ellipse((x,y,x+39,y+39),fill=(0,0,0,78));glyph(controls,name,(x+9,y+8,22,24))
    d.rounded_rectangle((w/2-52,h-13,w/2+51,h-10),radius=2,fill='white')
    im=Image.alpha_composite(im,controls)
    name=f'lock-bottom-{orientation}-{theme}.png';im.save(root/'gfx'/name);files.append(name)
# Input order: day then night; each has landscape top/bottom, portrait top/bottom.
(root/'gfx/lockscreen.t3s').write_text('--atlas --border=edge -f rgba8888 -z auto\n'+'\n'.join(files)+'\n')
(root/'lockscreen-spec.json').write_text(json.dumps({'font':Path(font).name+' at native size','date':'Wednesday, April 1','time':'9:41','files':files,'wallpaper':'one continuous cover per orientation; landscape 400x480, portrait 480x400 with smaller display centered','gesture':{'startBottomPixels':64,'threshold':64,'verticalDominance':1.2,'settleMs':180}},indent=2)+'\n')
p=args.provenance
if p.exists():shutil.copy2(p,root/'lockscreen-provenance.json')
print('Generated eight native-size lock frames; icons retain original aspect ratio')
