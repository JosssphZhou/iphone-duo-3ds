"""Semantic crops of fixed Apple iPhone Duo source pixels; no generated UI."""
from pathlib import Path
from PIL import Image, ImageOps, ImageDraw, ImageFont
import json, shutil, argparse
ROOT=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--source',type=Path,default=ROOT/'.local-assets');parser.add_argument('--output',type=Path,default=ROOT/'assets/official');parser.add_argument('--native-output',type=Path,default=ROOT/'.local-assets/official-export');args=parser.parse_args()
SOURCE=args.source;OUT=args.output;NATIVE=args.native_output
NATIVE.mkdir(parents=True,exist_ok=True)
old=json.loads((OUT/'manifest.json').read_text())
ims={a['id']:Image.open(SOURCE/'screens'/f"{a['id']}-rectified.png").convert('RGB') for a in old['apps']}
records={}
def crop(k,box): return ims[k].crop(box)
def fit(im,size,bg='white',cover=False):
 if cover:return ImageOps.fit(im,size,Image.Resampling.LANCZOS)
 canvas=Image.new('RGB',size,bg); pic=ImageOps.contain(im,size,Image.Resampling.LANCZOS)
 canvas.paste(pic,((size[0]-pic.width)//2,(size[1]-pic.height)//2));return canvas
def put(canvas,im,box):
 x,y,w,h=box;canvas.paste(fit(im,(w,h)),(x,y))
def font(size,bold=False):
 import os
 candidates=[os.environ.get('DUO_FONT'),'/System/Library/Fonts/Helvetica.ttc','/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf']
 selected=next((p for p in candidates if p and Path(p).exists()),None)
 if not selected:raise RuntimeError('Set DUO_FONT to an installed TrueType font')
 return ImageFont.truetype(selected,size,index=(1 if bold else 0) if selected.endswith('.ttc') else 0)
def words(draw,text,xy,width,size=11,fill='black',spacing=3):
 x,y=xy;f=font(size);lines=[];line=''
 for word in text.split():
  trial=(line+' '+word).strip()
  if draw.textlength(trial,font=f)>width and line:lines.append(line);line=word
  else:line=trial
 if line:lines.append(line)
 for line in lines:draw.text((x,y),line,font=f,fill=fill);y+=size+spacing
 return y
def keyboard(size):
 w,h=size;im=Image.new('RGB',size,(57,57,57));d=ImageDraw.Draw(im);gap=4;rowh=(h-24)//4
 for row,letters in enumerate(['qwertyuiop','asdfghjkl','zxcvbnm']):
  kw=(w-12-gap*9)/10;count=len(letters);offset=(w-(count*kw+(count-1)*gap))/2
  for col,ch in enumerate(letters):
   x=offset+col*(kw+gap);y=6+row*(rowh+4);d.rounded_rectangle((x,y,x+kw,y+rowh),radius=5,fill=(99,99,99));box=d.textbbox((0,0),ch,font=font(15));d.text((x+(kw-box[2])/2,y+(rowh-15)/2-1),ch,font=font(15),fill='white')
 # Source keyboard has shift/delete, numeric/emoji, space and blue search key.
 for label,x,ww in [('123',7,34),('☺',45,28),('',77,w-142),('⌕',w-60,52)]:
  y=6+3*(rowh+4);d.rounded_rectangle((x,y,x+ww,y+rowh),radius=5,fill=(0,145,255) if label=='⌕' else (100,100,100));
  if label=='⌕':
   cx=x+ww/2;cy=y+rowh/2;d.ellipse((cx-5,cy-6,cx+3,cy+2),outline='white',width=2);d.line((cx+2,cy+1,cx+7,cy+6),fill='white',width=2)
  else:d.text((x+7,y+(rowh-12)/2),label,font=font(12),fill='white')
 return im
def build(k,orient,side,size):
 w,h=size; canvas=Image.new('RGB',size,'white'); P=orient=='portrait'
 if k=='mail':
  if side=='top':
   if P: return fit(crop(k,(605,0,1200,1000)),size)
   put(canvas,crop(k,(640,110,1080,520)),(0,0,260,240));put(canvas,crop(k,(630,525,1060,995)),(265,15,130,210))
  else:
   if P:return fit(crop(k,(0,45,600,845)),size)
   put(canvas,crop(k,(15,135,575,305)),(0,0,w,96));put(canvas,crop(k,(20,315,565,475)),(0,96,w,88));put(canvas,crop(k,(30,485,555,580)),(0,184,w,56))
 elif k=='photos':
  if side=='top':return ImageOps.fit(crop(k,(135,475,575,995)),size,Image.Resampling.LANCZOS,centering=(.6,.32))
  d=ImageDraw.Draw(canvas);d.text((12,9),'Library',font=font(23,True),fill='black');d.text((13,37),'April 1, 2026',font=font(10),fill='#666666');d.text((w-48,17),'Select',font=font(10),fill='#666666')
  cols=4 if not P else 3;rows=3 if not P else 4;cellw=w//cols;cellh=(h-56)//rows
  for i in range(cols*rows):
   col=i%3;row=(i//3)%4;tile=crop(k,(615+col*162,205+row*190,615+(col+1)*162,205+(row+1)*190))
   canvas.paste(ImageOps.fit(tile,(cellw-1,cellh-1),Image.Resampling.LANCZOS),((i%cols)*cellw,56+(i//cols)*cellh))
 elif k=='siri':
  if side=='top':
   if P:return fit(crop(k,(0,0,600,1000)),size)
   for i,box in enumerate([(128,22,344,361),(366,90,581,427),(128,386,344,724)]):put(canvas,crop(k,box),(i*133,0,133,240))
  else:
   d=ImageDraw.Draw(canvas);d.text((12,10),'Mindy Borovsky',font=font(12,True),fill='#222222');d.text((12,28),'To: Nate de Vries',font=font(9),fill='#777777');d.line((12,45,w-12,45),fill='#eeeeee');d.text((12,56),'Misty Zugspitze',font=font(16,True),fill='#111111')
   y=words(d,'Hey Nate,',(12,83),w-24,11)
   y=words(d,'Just got back from a long hike around Eibsee. It was really foggy all morning, but I happened to have my phone in hand when the summit ridge finally broke through for 90 seconds or so. It feels unreal every time timing is perfect.',(12,y+5),w-24,11,spacing=2)
   y=words(d,'Miss you!',(12,y+4),w-24,11)
   if h-y>18:canvas.paste(ImageOps.fit(crop(k,(635,645,1055,995)),(w-24,h-y-12),Image.Resampling.LANCZOS),(12,y+8))
 elif k=='notes':
  d=ImageDraw.Draw(canvas)
  if side=='top':
   d.text((12,12),'Intern Onboarding',font=font(20 if P else 18,True),fill='#222222')
   box=(12,51,w-12,h-14);d.rounded_rectangle(box,radius=17,fill='#303030')
   words(d,'Write a quick guide to photography for our summer intern Marie. Cover the following: setting up a workflow for e-commerce photography that simplifies file management; shooting in natural light around the Mission District; tips for retouching and color correction; how to avoid glare in products with reflective surfaces.',(24,64),w-48,11,fill='white',spacing=3)
  else:
   toolbar=crop(k,(675,510,1240,585));put(canvas,toolbar,(8,5,w-16,38));canvas.paste(keyboard((w,h-52)),(0,52))
 elif k=='facetime':
  if side=='top':return fit(crop(k,(420,0,1140,1000)),size,cover=P) if P else fit(ims[k],size,bg=(74,51,31))
  # Use only the small caller's video interior, excluding the main caller's shirt.
  canvas=ImageOps.fit(crop(k,(45,705,365,935)),size,Image.Resampling.LANCZOS,centering=(.55,.35))
  controls=crop(k,(1243,815,1322,953));controls.thumbnail((30,52),Image.Resampling.LANCZOS);mask=Image.new('L',controls.size);ImageDraw.Draw(mask).rounded_rectangle((0,0,controls.width,controls.height),radius=14,fill=255);canvas.paste(controls,(w-controls.width-10,h-controls.height-10),mask)
 elif k=='tv':
  # A single focal crop per screen; no duplicate panorama strips.
  return ImageOps.fit(ims[k],size,Image.Resampling.LANCZOS,centering=(.57,.54))
 return canvas
for a in old['apps']:
 k=a['id'];records[k]=[]
 for orient,sizes in [('landscape',[(400,240),(320,240)]),('portrait',[(240,400),(240,320)])]:
  for side,size in zip(['top','bottom'],sizes):
   im=build(k,orient,side,size);name=f'{k}-{orient}-{side}.png';im.save(OUT/name);im.save(NATIVE/name)
   records[k].append({'orientation':orient,'screen':side,'size':size,'source':f'{k}-rectified.png','operation':'semantic crop/reassembly; source text and keyboard redrawn at native readable sizes where authorized'})
 a['adaptation']=records[k]
old['version']=2;old['method']='Per-app semantic photo crops with uniform scaling. Source UI text/keyboard reflowed at native readable sizes where authorized. Static adaptation, not original device pixel geometry.'
(OUT/'manifest.json').write_text(json.dumps(old,ensure_ascii=False,indent=2));(OUT/'manifest.js').write_text('globalThis.DuoOfficialAssets='+json.dumps(old,ensure_ascii=False)+';\n')
native=json.loads(json.dumps(old))
for a in native['apps']:
 for o in ['landscape','portrait']:
  for side in ['top','bottom']:a[o][side]=Path(a[o][side]).name
(NATIVE/'manifest.json').write_text(json.dumps(native,ensure_ascii=False,indent=2))
# Large labeled contact sheet: one row per app, all four logical screens.
sheet=Image.new('RGB',(1240,6*440),(215,215,215));d=ImageDraw.Draw(sheet)
for row,a in enumerate(old['apps']):
 y=row*440;d.text((8,y+6),a['id'],fill='black');x=8
 for o,side in [('landscape','top'),('landscape','bottom'),('portrait','top'),('portrait','bottom')]:
  im=Image.open(OUT/Path(a[o][side]).name);d.text((x,y+25),o+' '+side,fill='black');sheet.paste(im,(x,y+42));x+=im.width+10
sheet.save(SOURCE/'v8-contactsheet.png');shutil.copy2(SOURCE/'v8-contactsheet.png',NATIVE/'contactsheet.png')
print(SOURCE/'v8-contactsheet.png')
