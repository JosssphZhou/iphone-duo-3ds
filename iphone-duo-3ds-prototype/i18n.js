/* UI language only; internal app identities and Apple image text stay unchanged. */
const UI_LANGUAGE_KEY = 'iphone-duo-ui-language-v1';
const UI_LANGUAGES = {
  en: {label: 'English', locale: 'en-US'},
  zh: {label: '简体中文', locale: 'zh-CN'},
  ja: {label: '日本語', locale: 'ja-JP'},
  ko: {label: '한국어', locale: 'ko-KR'},
  es: {label: 'Español', locale: 'es-ES'},
  fr: {label: 'Français', locale: 'fr-FR'},
  de: {label: 'Deutsch', locale: 'de-DE'},
  pt: {label: 'Português', locale: 'pt-BR'},
};
function supportedLanguage(value) {
  const code = String(value || '').toLowerCase().split(/[-_]/)[0];
  return Object.hasOwn(UI_LANGUAGES, code) ? code : null;
}
function initialLanguage() {
  try {
    const saved = supportedLanguage(localStorage.getItem(UI_LANGUAGE_KEY));
    if (saved) return saved;
  } catch {}
  for (const candidate of navigator.languages || [navigator.language]) {
    const code = supportedLanguage(candidate);
    if (code) return code;
  }
  return 'en';
}
let uiLanguage = initialLanguage();
const uiNames={'日历':'Calendar','照片':'Photos','相机':'Camera','邮件':'Mail','备忘录':'Notes','时钟':'Clock','地图':'Maps','新闻':'News','游戏':'Games','健康':'Health','钱包':'Wallet','设置':'Settings','电话':'Phone','信息':'Messages','音乐':'Music','计算器':'Calculator','搜索':'Search'};
const uiStrings={...uiNames,'网页交互预演':'Interactive web prototype','切换竖屏':'Portrait','切换横屏':'Landscape','正面操作':'Front view','立体视角':'3D angle','背面细节':'Back view','拖动机身或空白处可旋转视角，点「正面操作」回到正面触屏':'Drag the console or background to orbit, or select Front view to align the touch screen','录制舞台':'Recording stage','退出舞台':'Exit stage','重置演示':'Reset demo','发布时的桌面，装进两块熟悉的屏幕。':'The launch screen, on two familiar displays.','点图标打开应用，长按拖动整理，按 HOME 回桌面。':'Tap an app, hold to rearrange, and press HOME to return.','试试备忘录、照片、音乐，再按 X 切换日夜壁纸。':'Explore the original screens. Press X for day or night.','1× 像素尺寸':'1× pixel size','适应窗口':'Fit to window','键盘与按键说明':'Controls & help','网页交互预演；真机版本单独验证。':'Web prototype; native build validated separately.','完成':'Done','移除小组件':'Remove widget','创作时间':'Creative time','出去散步':'Take a walk','向上':'Up','向下':'Down','向左':'Left','向右':'Right','锁屏':'Lock screen','搜索应用':'Search apps','查看回忆':'View memory','第一页':'Page 1','第二页':'Page 2','官网未展示此应用':'This app is not shown on the official site.','本次排列未保存，刷新会恢复':'Layout could not be saved. Reloading will restore the previous layout.','桌面布局已还原':'Layout restored.','你想打开什么？':'What would you like to open?','在触屏找到你的应用。':'Find an app on the touch screen.','输入应用名称':'Search app names','部分 Apple 素材缺失。请按 README 的本地素材导入说明安装素材后刷新。':'Apple artwork is missing. Follow the local asset import guide in README, then reload.','素材导入说明':'Asset import guide'};
uiStrings['点击应用查看 Apple 原图。长按图标整理；将音乐拖向靠近主屏的边缘添加组件，再长按音乐移除。方向键选择，A/Enter 打开，B/Esc 返回，Y 翻页，X 切换壁纸，L 旋转。START 锁屏，从触屏底边上滑解锁。音乐组件为静态预览，天气与日程为示例。默认官网图片模式支持双语；历史实验页面和图片内文字保留原语言。']='Tap an app to view an original Apple screen. Hold an icon to rearrange it. Drag Music toward the top screen to add a widget; hold Music again to remove it. Arrow keys select, A/Enter opens, B/Esc returns, Y changes page, X changes wallpaper, and L rotates. START locks; swipe up from the bottom edge to unlock. The music widget is a static preview. Weather and events are examples. The default image mode supports both languages; archived experimental screens and image text keep their original language.';
function englishText(value){const s=String(value);if(uiStrings[s])return uiStrings[s];if(s.endsWith(' 官网展示画面'))return 'Official '+(uiNames[s.slice(0,-7)]||s.slice(0,-7))+' screenshot';if(s==='Apple Music 官方小组件')return 'Official Apple Music widget';if(s.startsWith('打开'))return 'Open '+(uiNames[s.slice(2)]||s.slice(2));if(/^空位 \d+$/.test(s))return s.replace('空位','Empty slot');if(/^\d+月\d+日$/.test(s)){const [,m,d]=s.match(/(\d+)月(\d+)日/);return ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'][Number(m)-1]+' '+d;}if(s.startsWith('星期'))return ['Sunday','Monday','Tuesday','Wednesday','Thursday','Friday','Saturday']['日一二三四五六'.indexOf(s.slice(2))]||s;const map={'日':'S','一':'M','二':'T','三':'W','四':'T','五':'F','六':'S','X 切换壁纸':'X: change wallpaper','A 打开':'A: open','B 返回':'B: back','Y 下一页':'Y: next page','HOME 返回桌面':'HOME: return to desktop','3DS 上屏 400乘240':'3DS top screen 400 by 240','3DS 下屏 320乘240':'3DS touch screen 320 by 240','3DS 左屏 240乘400':'3DS left screen 240 by 400','3DS 右触屏 240乘320':'3DS right touch screen 240 by 320'};return map[s]||s;}

const sourceText = new WeakMap(), lastText = new WeakMap();
const sourceAttrs = new WeakMap(), lastAttrs = new WeakMap();
function localized(english) {
  const catalog = typeof UI_TRANSLATIONS === 'undefined' ? {} : UI_TRANSLATIONS[uiLanguage];
  return catalog?.[english] || english.replace('supports both languages', 'supports multiple languages');
}
function formatDate(date, options) {
  return new Intl.DateTimeFormat(UI_LANGUAGES[uiLanguage].locale, options).format(date);
}
function uiText(value) {
  const s = String(value);
  if (s === 'Language') return uiLanguage === 'zh' ? '语言' : localized(s);
  if (s === 'On This Day') return uiLanguage === 'zh' ? '往年今日' : localized(s);
  if (s === 'Partly Cloudy') return uiLanguage === 'zh' ? '晴间多云' : localized(s);
  if (s === 'H:57° L:49°') return uiLanguage === 'zh' ? '最高57° 最低49°' : localized(s);
  if (s === 'APRIL 1, 2025') return formatDate(new Date(2025, 3, 1), {year:'numeric', month:'long', day:'numeric'});
  if (s === 'Wednesday, April 1') return formatDate(new Date(2026, 3, 1), {weekday:'long', month:'long', day:'numeric'});
  const md = s.match(/^(\d+)月(\d+)日$/);
  if (md) return formatDate(new Date(2026, Number(md[1])-1, Number(md[2])), {month:'short', day:'numeric'});
  const weekday = s.match(/^星期([日一二三四五六])$/);
  if (weekday) return formatDate(new Date(2026, 0, 4 + '日一二三四五六'.indexOf(weekday[1])), {weekday:'long'});
  if (/^[日一二三四五六]$/.test(s)) return formatDate(new Date(2026, 0, 4 + '日一二三四五六'.indexOf(s)), {weekday:'narrow'});
  if (uiLanguage === 'zh') return s.replace('支持双语', '支持多语言');
  if (s.endsWith(' 官网展示画面')) return localized('Official {app} screenshot').replace('{app}', uiAppName(s.slice(0,-7)));
  if (s === 'Apple Music 官方小组件') return localized('Official Apple Music widget');
  if (s.startsWith('打开')) return localized('Open {app}').replace('{app}', uiAppName(s.slice(2)));
  if (/^空位 \d+$/.test(s)) return localized('Empty slot {n}').replace('{n}', s.split(' ')[1]);
  return localized(englishText(s));
}
function translateDOM(root = document.body) {
  if (!root) return;
  document.documentElement.lang = UI_LANGUAGES[uiLanguage].locale;
  document.body.classList.add('notranslate');
  document.body.setAttribute('translate', 'no');
  for (const el of root.querySelectorAll('*')) {
    if (!el.matches('script,style')) { el.classList.add('notranslate'); el.setAttribute('translate', 'no'); }
  }
  const walker = document.createTreeWalker(root, NodeFilter.SHOW_TEXT);
  for (let node; node = walker.nextNode();) {
    if (node.parentElement?.closest('script,style,textarea,select')) continue;
    let original = sourceText.get(node);
    if (original === undefined || (lastText.has(node) && lastText.get(node) !== node.nodeValue)) {
      original = node.nodeValue;
      sourceText.set(node, original);
    }
    const trimmed = original.trim();
    if (trimmed) node.nodeValue = original.replace(trimmed, uiText(trimmed));
    lastText.set(node, node.nodeValue);
  }
  const elements = [...root.querySelectorAll('[aria-label],[placeholder],[alt]')];
  if (root.matches?.('[aria-label],[placeholder],[alt]')) elements.unshift(root);
  for (const el of elements) {
    let originals = sourceAttrs.get(el);
    if (!originals) {
      originals = {};
      for (const key of ['aria-label','placeholder','alt']) if (el.hasAttribute(key)) originals[key] = el.getAttribute(key);
      sourceAttrs.set(el, originals);
    }
    const last = lastAttrs.get(el) || {};
    for (const key of Object.keys(originals)) {
      if (key in last && el.getAttribute(key) !== last[key]) originals[key] = el.getAttribute(key);
      el.setAttribute(key, uiText(originals[key]));
      last[key] = el.getAttribute(key);
    }
    lastAttrs.set(el, last);
  }
  const selector = document.querySelector('#language-toggle');
  if (selector) selector.value = uiLanguage;
  // Fit translated app names to the same physical icon grid; full labels remain accessible.
  for (const el of root.querySelectorAll('.app-label')) {
    el.title = el.textContent;
    el.style.fontSize = '';
    let size = parseFloat(getComputedStyle(el).fontSize);
    while (el.scrollWidth > el.clientWidth && size > 7) {
      size -= 0.25;
      el.style.fontSize = size + 'px';
    }
  }
}
function uiAppName(name) { return uiLanguage === 'zh' ? name : localized(uiNames[name] || name); }
function searchKey(value) { return String(value).normalize('NFD').replace(/[\u0300-\u036f]/g, '').toLocaleLowerCase(); }
function matchesApp(name, query) {
  const english = uiNames[name] || name;
  const translations = typeof UI_TRANSLATIONS === 'undefined' ? [] : Object.values(UI_TRANSLATIONS).map(c => c[english] || '');
  return searchKey([name, english, ...translations].join(' ')).includes(searchKey(query));
}
function setLanguage(value) {
  const next = supportedLanguage(value);
  if (!next) return;
  if (typeof cancelDrag === 'function') cancelDrag();
  uiLanguage = next;
  try { localStorage.setItem(UI_LANGUAGE_KEY, next); } catch {}
  render();
  translateDOM();
  if (typeof size === 'function') size();
}
function showMissingAssets() {
  let el = document.querySelector('#asset-notice');
  if (!el) {
    el = document.createElement('p'); el.id = 'asset-notice';
    el.style.cssText = 'max-width:620px;margin:10px auto;padding:12px 16px;background:#fff4d8;border-radius:10px;font-size:13px;color:#5c4517';
    el.innerHTML = '部分 Apple 素材缺失。请按 README 的本地素材导入说明安装素材后刷新。 <a href="README.md">素材导入说明</a>';
    document.querySelector('header').append(el);
  }
  translateDOM(el);
}
document.addEventListener('error', e => {
  if (e.target instanceof HTMLImageElement) { e.target.style.visibility = 'hidden'; showMissingAssets(); }
}, true);
document.addEventListener('DOMContentLoaded', () => {
  const selector = document.querySelector('#language-toggle');
  for (const [code, info] of Object.entries(UI_LANGUAGES)) {
    const option = document.createElement('option'); option.value = code; option.textContent = info.label;
    selector.append(option);
  }
  selector.value = uiLanguage;
  selector.onchange = () => setLanguage(selector.value);
  translateDOM();
  for (const path of ['assets/launch-home.jpg','assets/desert-day.jpg']) {
    const image = new Image(); image.onerror = showMissingAssets; image.src = path;
  }
});
