"""End-to-end check of the header controls: rotate button, menu rows, language list, phone-width fit.
usage: menu_check.py <base-url> <out-dir>   (exit 0 = all passed)
"""
import glob, os, sys
from playwright.sync_api import sync_playwright

base, out = sys.argv[1], sys.argv[2]
os.makedirs(out, exist_ok=True)
EXE = glob.glob(os.path.expanduser('~/Library/Caches/ms-playwright/chromium_headless_shell-1243/*/chrome-headless-shell'))[0]
results = []
def check(name, ok, detail=''):
    results.append(ok); print(('PASS ' if ok else 'FAIL ') + name + (f'  ({detail})' if detail else ''))

with sync_playwright() as p:
    b = p.chromium.launch(executable_path=EXE)
    for width in (393, 320):
        pg = b.new_context(viewport={'width': width, 'height': 700}, locale='en-US', has_touch=True).new_page()
        errors = []; pg.on('pageerror', lambda e: errors.append(str(e)))
        pg.goto(base + ('&' if '?' in base else '?') + 'webgl=0'); pg.wait_for_timeout(1500)
        is_open = lambda: pg.evaluate("document.querySelector('#stage-menu').open")
        # rotate button sits outside the menu
        pg.tap('#orientation-toggle'); pg.wait_for_timeout(300)
        check(f'{width}px: rotate button turns the console to portrait without opening the menu',
              pg.evaluate('state.portrait') is True and pg.get_attribute('#orientation-toggle', 'aria-pressed') == 'true' and not is_open())
        pg.tap('#orientation-toggle'); pg.wait_for_timeout(300)
        check(f'{width}px: rotate button again returns to landscape', pg.evaluate('state.portrait') is False)
        # language list replaces the native select
        pg.tap('#stage-menu > summary')
        check(f'{width}px: no native select in the menu', pg.locator('#stage-menu select').count() == 0)
        pg.tap('#language-toggle')
        check(f'{width}px: language list shows 8 languages', pg.locator('#language-list [data-lang]:visible').count() == 8)
        pg.tap('#language-list [data-lang="ja"]')
        check(f'{width}px: picking 日本語 switches the page and closes the menu',
              pg.evaluate('document.documentElement.lang').startswith('ja') and not is_open())
        pg.tap('#stage-menu > summary')
        check(f'{width}px: Language row shows the current language', pg.text_content('#language-value') == '日本語')
        # every language fits at this width with the menu open
        overflow = []
        for code in pg.evaluate('Object.keys(UI_LANGUAGES)'):
            pg.evaluate(f"setLanguage('{code}')")
            pg.evaluate("document.querySelector('#stage-menu').open=true")
            r = pg.evaluate("""() => { const m = document.querySelector('.menu-panel').getBoundingClientRect(), a = document.querySelector('.topbar-actions').getBoundingClientRect();
              return [document.documentElement.scrollWidth, Math.round(m.left), Math.round(m.right), Math.round(a.right)] }""")
            if r[0] > width or r[1] < 0 or r[2] > width or r[3] > width: overflow.append((code, r))
        check(f'{width}px: menu and header stay on screen in all 8 languages', not overflow, str(overflow))
        pg.evaluate("setLanguage('en')")
        # failure path: outside tap and Escape close the menu without side effects
        pg.evaluate("document.querySelector('#stage-menu').open=true")
        pg.tap('h1'); pg.wait_for_timeout(100)
        check(f'{width}px: tapping outside closes the menu', not is_open())
        pg.tap('#stage-menu > summary'); pg.keyboard.press('Escape'); pg.wait_for_timeout(100)
        check(f'{width}px: Escape closes the menu and stays on the home screen', not is_open() and not pg.evaluate('state.app'))
        check(f'{width}px: no page errors', not errors, str(errors))
        pg.screenshot(path=f'{out}/menu-check-{width}.png')
        pg.context.close()
    b.close()
print(f'{sum(results)}/{len(results)} passed'); sys.exit(0 if all(results) else 1)
