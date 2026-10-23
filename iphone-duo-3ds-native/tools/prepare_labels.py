#!/usr/bin/env python3
"""Render English labels at native size; optionally regenerate Chinese labels."""
from pathlib import Path
import argparse,json
from PIL import Image,ImageDraw,ImageFont
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser()
p.add_argument('--font',type=Path,help='Optional local Chinese font; never bundled by this script')
p.add_argument('--font-index',type=int,default=0)
p.add_argument('--en-font',type=Path,default=root/'tools/NotoSans.ttf')
a=p.parse_args()
stems=['facetime','calendar','photos','camera','mail','notes','clock','maps','apple-tv','news','app-store','games','health','wallet','siri','settings','phone','safari','messages','music','calculator']
chinese=['FaceTime','日历','照片','相机','邮件','备忘录','时钟','地图','TV','新闻','App Store','游戏','健康','钱包','Siri','设置','电话','Safari','信息','音乐','计算器']
english=['FaceTime','Calendar','Photos','Camera','Mail','Notes','Clock','Maps','TV','News','App Store','Games','Health','Wallet','Siri','Settings','Phone','Safari','Messages','Music','Calculator']
files=[];specs=[]
for lang,names in [('zh',chinese),('en',english)]:
 for stem,name in zip(stems,names):
  filename=('labelsharp-' if lang=='zh' else 'labelen-')+stem+'.png';files.append(filename)
  if lang=='zh' and not a.font:
   if not (root/'gfx'/filename).exists():raise SystemExit('Import prepared Chinese labels or supply --font and --font-index.')
   specs.append({'language':'zh','app':stem,'text':name,'fontSize':9 if name in ('FaceTime','App Store') else 11,'canvas':[56,16],'imported':True})
   continue
  size=9 if lang=='zh' and name in ('FaceTime','App Store') else 11
  fontpath=a.font if lang=='zh' else a.en_font
  f=ImageFont.truetype(str(fontpath),size,index=a.font_index if lang=='zh' else 0)
  while lang=='en' and f.getlength(name)>45 and size>8:
   size-=1;f=ImageFont.truetype(str(fontpath),size)
  box=f.getbbox(name);x=round((56-(box[2]-box[0]))/2-box[0]);y=round((15-(box[3]-box[1]))/2-box[1])
  shadow=Image.new('RGBA',(56,16));ImageDraw.Draw(shadow).text((x,y+1),name,font=f,fill=(0,0,0,90))
  lettering=Image.new('RGBA',(56,16));ImageDraw.Draw(lettering).text((x,y),name,font=f,fill='white')
  Image.alpha_composite(shadow,lettering).save(root/'gfx'/filename)
  specs.append({'language':lang,'app':stem,'text':name,'fontSize':size,'canvas':[56,16],'position':[x,y]})
(root/'gfx/labels.t3s').write_text('--atlas --border=edge -f rgba8888 -z auto\n'+'\n'.join(files)+'\n')
(root/'label-spec.json').write_text(json.dumps({'englishFont':'Noto Sans (OFL)','chineseFont':'Imported prepared Chinese sprites or explicitly supplied local font','render':'native pixel size; white fill, no outline, light downward shadow','shadow':{'offset':[0,1],'rgba':[0,0,0,90]},'strokeWidth':0,'labels':specs},ensure_ascii=False,indent=2)+'\n')
print('Prepared 21 Chinese and 21 English native labels; original glyph sizes retained for imported Chinese sprites')
