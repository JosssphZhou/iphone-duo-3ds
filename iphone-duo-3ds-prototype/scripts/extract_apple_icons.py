from PIL import Image, ImageDraw
from pathlib import Path
import json
import urllib.request
root=Path(__file__).resolve().parents[1]
im=Image.open(root/'assets/launch-home.jpg').convert('RGB')
# Clockwise corners, measured on the unmodified Apple press image.
quads={
'facetime':[(1004,412),(1081,408),(1082,499),(1003,500)],
'calendar':[(1110,406),(1197,404),(1196,500),(1110,500)],
'photos':[(1221,404),(1307,404),(1306,501),(1219,501)],
'camera':[(1337,405),(1423,404),(1423,498),(1335,499)],
'mail':[(1004,533),(1082,533),(1083,624),(1004,624)],
'notes':[(1110,532),(1196,533),(1197,628),(1110,627)],
'clock':[(1221,534),(1306,535),(1307,630),(1220,630)],
'maps':[(1338,537),(1423,538),(1424,633),(1336,632)],
'apple-tv':[(1004,658),(1081,656),(1084,744),(1004,744)],
'news':[(1110,660),(1196,661),(1197,752),(1111,751)],
'app-store':[(1221,666),(1306,666),(1307,755),(1221,755)],
'games':[(1339,668),(1424,671),(1424,760),(1338,759)],
'health':[(1004,777),(1081,777),(1083,864),(1005,864)],
'wallet':[(1110,783),(1195,784),(1197,874),(1111,871)],
'siri':[(1222,788),(1307,790),(1307,881),(1221,880)],
'settings':[(1340,794),(1424,795),(1425,888),(1338,885)],
'phone':[(1475,360),(1537,360),(1539,425),(1475,427)],
'safari':[(1475,452),(1538,452),(1539,518),(1475,518)],
'messages':[(1476,546),(1541,547),(1541,612),(1476,612)],
'music':[(1477,639),(1542,640),(1543,705),(1477,706)],
}
out=root/'assets/apple-icons'
for name,p in quads.items():
    # Pillow QUAD order is TL, BL, BR, TR.
    tl,tr,br,bl=p
    tile=im.transform((128,128),Image.Transform.QUAD,tuple(tl+bl+br+tr),resample=Image.Resampling.BICUBIC).convert('RGBA')
    mask=Image.new('L',(512,512));d=ImageDraw.Draw(mask);d.rounded_rectangle((0,0,511,511),radius=115,fill=255)
    tile.putalpha(mask.resize((128,128),Image.Resampling.LANCZOS));tile.save(out/(name+'.png'))
manifest={'source':'https://www.apple.com/v/iphone-duo/d/images/overview/media-hero/hero_endframe__gdt3l3spaqie_large_2x.jpg','local_source':'../launch-home.jpg','source_dimensions':[1932,1280],'provenance':'Original Apple app icon pixels extracted and perspective-rectified from the official press image. These are not OS asset dumps and have not been redrawn. Rounded alpha masks remove surrounding wallpaper.','output_dimensions':[128,128],'quad_order':'top-left, top-right, bottom-right, bottom-left','icons':{k:{'file':k+'.png','source_quad':v} for k,v in quads.items()}}
calculator_url='https://is1-ssl.mzstatic.com/image/thumb/Purple211/v4/f4/35/37/f435379c-2c09-a52a-1bbc-326d8cd54853/calculatorCalistoga-0-0-1x_U007epad-0-1-0-85-220.png/512x512bb.jpg'
calculator_original=out/'calculator-original.jpg'
if not calculator_original.exists(): urllib.request.urlretrieve(calculator_url, calculator_original)
calc=Image.open(calculator_original).convert('RGBA').resize((128,128),Image.Resampling.LANCZOS)
calc.putalpha(mask.resize((128,128),Image.Resampling.LANCZOS));calc.save(out/'calculator.png')
manifest['icons']['calculator']={'file':'calculator.png','source_url':calculator_url,'source_page':'https://apps.apple.com/us/app/calculator/id1069511488','provenance':'Apple Inc. official App Store icon; resized and rounded-alpha masked'}
(out/'manifest.json').write_text(json.dumps(manifest,indent=2,ensure_ascii=False))
map_widget=im.transform((220,220),Image.Transform.QUAD,(1218,141,1218,372,1427,371,1427,140),resample=Image.Resampling.BICUBIC)
map_widget.save(root/'assets/apple-map-widget.png')
# Contact sheet for inspection, no resizing beyond nearest native 128 outputs.
sheet=Image.new('RGB',(640,560),'#e8e9e9');d=ImageDraw.Draw(sheet)
for i,k in enumerate(quads):
    x=(i%5)*128;y=(i//5)*140;tile=Image.open(out/(k+'.png'));sheet.paste(tile,(x,y),tile);d.text((x+5,y+128),k,fill='black')
sheet.save(root/'assets/apple-icons/contact-sheet.png')
