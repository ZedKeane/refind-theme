import argparse, hashlib, io, json, shutil, html, math
from pathlib import Path
from PIL import Image, ImageFont, ImageDraw, ImageChops
import resvg_py
import xml.etree.ElementTree as ET

def render_svg(p):
    root = ET.fromstring(p.read_text())
    if root.get('viewBox'):
        box = root.get('viewBox').split()
        root.set('width', box[2])
        root.set('height', box[3])
    return resvg_py.svg_to_bytes(svg_string=ET.tostring(root, encoding='unicode'), width=600, height=600)
from catalog import ROOT, load
from build_language_assets import build_english_assets

def build(font_path, output):
    systems = load()
    output.mkdir(parents=True, exist_ok=True)
    icons = output / 'icons'
    icons.mkdir(exist_ok=True)
    for p in (ROOT / 'assets/theme-base').iterdir():
        if p.suffix == '.png':
            shutil.copy2(p, output if p.name.startswith(('background', 'selection')) else icons)
    shutil.copy2(ROOT / 'assets/theme-base/theme.conf', output / 'theme.conf')
    for direction in ['left', 'right']:
        p = ROOT / f'assets/theme-base/arrow_{direction}.svg'
        Image.open(io.BytesIO(render_svg(p))).resize((128, 128), Image.Resampling.LANCZOS).save(icons / f'arrow_{direction}.png')
    cells = []
    manifest = {}
    sources = {}
    for s in systems:
        im = Image.new('RGBA', (256, 256))
        asset = s['icon'].get('asset')
        if asset:
            p = ROOT / asset
            raw = render_svg(p) if p.suffix == '.svg' else p.read_bytes()
            logo = Image.open(io.BytesIO(raw)).convert('RGBA')
            mask = ImageChops.multiply(logo.getchannel('A'), logo.convert('L').point(lambda x: 255 if x < 235 else 0))
            box = mask.getbbox()
            if not box:
                raise ValueError('Empty logo: ' + asset)
            mask = mask.crop(box)
            mask.thumbnail((152, 148), Image.Resampling.LANCZOS)
            im.paste((0, 0, 0, 255), ((256 - mask.width) // 2, 30 + (148 - mask.height) // 2), mask)
            sources[asset] = hashlib.sha256(p.read_bytes()).hexdigest()
        else:
            text = s['icon'].get('wordmark', s['title'])
            f = ImageFont.truetype(str(font_path), 28)
            while f.getlength(text) > 204:
                f = ImageFont.truetype(str(font_path), f.size - 1)
            ImageDraw.Draw(im).text((128, 104), text, anchor='mm', font=f, fill='black')
        d = ImageDraw.Draw(im)
        f = ImageFont.truetype(str(font_path), 22)
        while f.getlength(s['title']) > 236:
            f = ImageFont.truetype(str(font_path), f.size - 1)
        d.text((128, 205), s['title'], anchor='mm', font=f, fill='black')
        for key in dict.fromkeys([s['icon']['key']] + s['icon']['aliases']):
            p = icons / ('os_' + key + '.png')
            im.save(p)
            manifest[p.name] = hashlib.sha256(p.read_bytes()).hexdigest()
        cells.append((s, im))
    build_english_assets(output, font_path)
    previews = ROOT / 'build/previews'
    previews.mkdir(parents=True, exist_ok=True)
    pages = []
    for family in dict.fromkeys((s['family'] for s in systems)):
        group = [(s, im) for (s, im) in cells if s['family'] == family]
        cols = min(6, len(group))
        w = 192
        sheet = Image.new('RGB', (cols * w, math.ceil(len(group) / cols) * w), 'white')
        for (i, (s, im)) in enumerate(group):
            sheet.paste(im.resize((w, w), Image.Resampling.LANCZOS), (i % cols * w, i // cols * w), im.resize((w, w)).getchannel('A'))
        sheet.save(previews / (family + '.png'))
        pages.append(family)
    (output / 'catalog-manifest.json').write_text(json.dumps({'systems': len(systems), 'icon_files': manifest, 'source_sha256': sources}, ensure_ascii=False, indent=2) + '\n')
    rows = ['# 系统目录（自动生成）', '', '识别列列出可用证据；共享目录并不能证明具体衍生版本。无独立素材的条目使用父系统标志或文字标志。', '', '| 系统 | 图标键 | 识别证据 | 素材来源 |', '|---|---|---|---|']
    for s in systems:
        evidence = []
        for (k, label) in [('directories', 'EFI目录'), ('release_ids', 'ID'), ('release_variants', 'ID:VARIANT'), ('filenames', '文件名'), ('labels', '名称')]:
            if s['match'][k]:
                evidence.append(label + ': ' + ', '.join(s['match'][k]))
        rows.append('| ' + ' | '.join([s['title'], s['icon']['key'], '; '.join(evidence), s['icon']['source']]) + ' |')
    (ROOT / 'docs/SYSTEMS.md').write_text('\n'.join(rows) + '\n')
    print(f'{len(systems)} systems, {len(manifest)} icon filenames; gallery: {previews}')
if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('--font', type=Path)
    p.add_argument('--output', type=Path, default=ROOT / 'build/theme')
    args = p.parse_args()
    cfg = json.loads((ROOT / '.local/toolchain.json').read_text()) if (ROOT / '.local/toolchain.json').exists() else {}
    font = args.font or Path(cfg.get('font', ''))
    if not font.is_file():
        p.error('Supply --font /path/to/a/licensed/font.otf')
    build(font, args.output)
