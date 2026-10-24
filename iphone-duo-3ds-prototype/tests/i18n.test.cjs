const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const root = path.resolve(__dirname, '..');
function load(saved, languages = ['en-US'], storageError = false) {
  const context = {navigator:{languages}, localStorage:{getItem(){if(storageError)throw Error();return saved;},setItem(){}}, document:{addEventListener(){}}, Intl};
  vm.createContext(context);
  vm.runInContext(fs.readFileSync(path.join(root,'locales.js'),'utf8'),context);
  vm.runInContext(fs.readFileSync(path.join(root,'i18n.js'),'utf8'),context);
  return code => vm.runInContext(code,context);
}
assert.equal(load(null,['ja-JP'])('uiLanguage'),'ja');
assert.equal(load('fr',['ja-JP'])('uiLanguage'),'fr');
assert.equal(load('invalid',['xx-XX','pt-BR'])('uiLanguage'),'pt');
assert.equal(load(null,['xx-XX'])('uiLanguage'),'en');
assert.equal(load(null,['ko-KR'],true)('uiLanguage'),'ko');
assert.equal(load('zh',['en-US'])('uiLanguage'),'zh');
const run=load('en');
run(`for(const [code,catalog] of Object.entries(UI_TRANSLATIONS)) {
  for(const key of [...Object.values(uiStrings),'Language','On This Day','Partly Cloudy','H:57° L:49°','Open {app}','Empty slot {n}','Official {app} screenshot','Official Apple Music widget']) {
    if(typeof catalog[key] !== 'string' || !catalog[key].trim()) throw Error(code+': '+key);
    for(const token of key.match(/\\{[^}]+\\}/g)||[]) if(!catalog[key].includes(token))throw Error('Missing placeholder '+code+': '+token);
  }
}`);
assert.equal(run("matchesApp('照片','fotos')"),true);
assert.equal(run("matchesApp('设置','reglages')"),true);
assert.equal(run("matchesApp('音乐','ミュージック')"),true);
assert.equal(run("matchesApp('音乐','음악')"),true);
assert.equal(run("uiLanguage='fr'; uiText('打开照片')"),'Ouvrir Photos');
assert.match(run("uiLanguage='ja'; uiText('Wednesday, April 1')"),/4月1日/);
assert.match(run("uiLanguage='de'; uiText('12月1日')"),/.+/);
assert.equal(run("uiLanguage='en'; uiText('打开音乐')"),'Open Music');
assert.equal(run("uiLanguage='zh'; uiText('音乐')"),'音乐');
assert.equal(run("uiLanguage='de'; uiText('未知品牌')"),'未知品牌');
console.log('Language detection, fallback, catalogue completeness, placeholders, dates and multilingual search passed.');
