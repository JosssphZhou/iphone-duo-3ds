#!/usr/bin/env python3
"""Build native-size reference textures and an ASCII glyph atlas.
The final four home textures may be replaced by verified browser captures.
"""
from pathlib import Path
import math, argparse
from asset_config import asset_dir
from PIL import Image, ImageDraw, ImageFont
ROOT=Path(__file__).resolve().parents[1]
GFX=ROOT/'gfx'; GFX.mkdir(exist_ok=True)
parser=argparse.ArgumentParser();parser.add_argument('--source',type=Path,default=asset_dir('prototype'));parser.add_argument('--font',type=Path,default=ROOT/'tools/NotoSans.ttf');args=parser.parse_args()
SRC=args.source
font=ImageFont.truetype(str(args.font),20)
for theme in ['day','night']:
    bg=Image.open(SRC/f'desert-{theme}.jpg').convert('RGBA')
    for screen,size in [('top',(400,240)),('bottom',(320,240))]:
        out=GFX/f'{screen}_home_{theme}.png'
        if not out.exists(): bg.resize(size,Image.Resampling.LANCZOS).save(out)
Image.open(SRC/'desert-day.jpg').convert('RGBA').resize((400,240),Image.Resampling.LANCZOS).save(GFX/'photo_desert.png')
if not (GFX/'photo_memory.png').exists():
    im=Image.open(SRC/'launch-home.jpg').convert('RGBA')
    im.crop((440,125,920,900)).resize((149,240),Image.Resampling.LANCZOS).save(GFX/'photo_memory.png')
files=['top_home_day.png','bottom_home_day.png','top_home_night.png','bottom_home_night.png','photo_desert.png','photo_memory.png']
widths=[]
for code in range(32,127):
    char=chr(code); advance=math.ceil(font.getlength(char)); widths.append(advance)
    glyph=Image.new('RGBA',(max(advance+2,2),29),(0,0,0,0))
    ImageDraw.Draw(glyph).text((1,-3),char,font=font,fill=(255,255,255,255))
    name=f'glyph_{code}.png';glyph.save(GFX/name);files.append(name)
stems=['facetime', 'calendar', 'photos', 'camera', 'mail', 'notes', 'clock', 'maps', 'apple-tv', 'news', 'app-store', 'games', 'health', 'wallet', 'siri', 'settings', 'phone', 'safari', 'messages', 'music', 'calculator']
files += ['icon-'+stem+'.png' for stem in stems]
files += ['label-'+stem+'.png' for stem in stems]
files += ['bottom_home_page2_day.png','bottom_home_page2_night.png']
portrait_files=['top_portrait_day.png','top_portrait_night.png','bottom_portrait_day.png','bottom_portrait_night.png','bottom_portrait_page2_day.png','bottom_portrait_page2_night.png']
(GFX/'portrait.t3s').write_text('--atlas --border=edge -f rgba8888 -z auto\n'+'\n'.join(portrait_files)+'\n')
(GFX/'ui.t3s').write_text('--atlas --border=edge -f rgba8888 -z auto\n'+'\n'.join(files)+'\n')
(ROOT/'include/assets.h').write_text('#pragma once\n#define GLYPH_BASE 6\n#define ICON_BASE 101\n#define LABEL_BASE 122\n#define PAGE2_BASE 143\n#define TOP_PORTRAIT_BASE 145\n#define BOTTOM_PORTRAIT_BASE 147\n#define UI_IMAGES 145\n#define PORTRAIT_IMAGES 6\n#define TOTAL_IMAGES 151\nstatic const unsigned char glyph_widths[95] = {'+','.join(map(str,widths))+'};\n')
