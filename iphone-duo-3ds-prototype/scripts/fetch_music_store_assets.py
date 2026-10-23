from pathlib import Path
import urllib.request,urllib.parse,json,concurrent.futures
root=Path(__file__).resolve().parents[1];out=root/'assets/catalog';out.mkdir(exist_ok=True,parents=True)
def load_id(kind,i):
 u='https://itunes.apple.com/lookup?'+urllib.parse.urlencode({'id':i,'entity':'song' if kind=='album' else 'software','country':'us'})
 return kind,i,u,json.load(urllib.request.urlopen(u))['results']
inputs=[('album',6779097512),('album',1836002188),('album',1739659134),('app',570060128),('app',6502453075),('app',916366645)]
data={'albums':[],'apps':[],'sources':['https://www.apple.com/apple-music/','https://www.apple.com/app-store/']}
for kind,i,url,results in concurrent.futures.ThreadPoolExecutor().map(lambda args:load_id(*args),inputs):
 x=results[0]; art=x.get('artworkUrl512',x.get('artworkUrl100'));art=art.replace('100x100bb','600x600bb').replace('512x512bb','600x600bb')
 name=('album-' if kind=='album' else 'app-')+str(i)
 urllib.request.urlretrieve(art,out/(name+'.jpg'))
 if kind=='album':
  item={'id':i,'title':x['collectionName'],'artist':x['artistName'],'art':'assets/catalog/'+name+'.jpg','source':url,'artSource':art,'tracks':[{'title':t['trackName'],'duration':round(t.get('trackTimeMillis',180000)/1000)} for t in results[1:] if t.get('wrapperType')=='track'][:3]}
  data['albums'].append(item)
 else:
  shots=x.get('screenshotUrls',x.get('ipadScreenshotUrls',[]));screen=None
  if shots:
   try:urllib.request.urlretrieve(shots[0],out/(name+'-screen.jpg'));screen='assets/catalog/'+name+'-screen.jpg'
   except Exception:pass
  item={'id':i,'title':x['trackName'],'artist':x['artistName'],'art':'assets/catalog/'+name+'.jpg','screen':screen,'source':url,'artSource':art,'screenshotSource':shots[0] if shots else None,'genre':x.get('primaryGenreName'),'rating':round(x.get('averageUserRating',4.7),1),'ratingCount':x.get('userRatingCount',0),'age':x.get('contentAdvisoryRating','4+'),'price':x.get('formattedPrice','Free'),'subtitle':{'570060128':'学习英语及更多语言','6502453075':'每一局，都意想不到','916366645':'随时随地，挥洒创意'}.get(str(i),'')}
  data['apps'].append(item)
(out/'catalog.json').write_text(json.dumps(data,ensure_ascii=False,indent=2));(out/'catalog.js').write_text('window.DUO_CATALOG='+json.dumps(data,ensure_ascii=False)+';')
print(json.dumps(data,ensure_ascii=False,indent=2))
