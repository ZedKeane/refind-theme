import argparse, re
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
from fontTools.ttLib import TTFont
from catalog import ROOT
p = argparse.ArgumentParser()
p.add_argument('--mono', type=Path, required=True)
p.add_argument('--cjk', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
a = p.parse_args()
chars = ''.join((chr(int(n, 16)) for n in re.findall('0x([0-9a-f]+)', (ROOT / 'vendor/refind/libeg/zh_glyphs.h').read_text())))
cmap = TTFont(a.cjk).getBestCmap()
assert all((ord(c) in cmap for c in chars)), 'CJK font lacks required glyphs'
scale = 3
w = 16
h = 24
im = Image.new('RGBA', ((96 + len(chars)) * w * scale, h * scale))
d = ImageDraw.Draw(im)
fonts = [ImageFont.truetype(str(f), 16 * scale) for f in [a.mono, a.cjk]]
for (i, c) in enumerate(''.join((chr(n) for n in range(32, 128))) + chars):
    if i == 95:
        c = '?'
    ft = fonts[i >= 96]
    d.text((i * w * scale + (w * scale - d.textlength(c, font=ft)) / 2, 19 * scale), c, font=ft, fill='black', anchor='ls')
a.output.parent.mkdir(parents=True, exist_ok=True)
im.resize((im.width // scale, h), Image.Resampling.LANCZOS).save(a.output)
print('Built font atlas:', len(chars), 'CJK glyphs')
