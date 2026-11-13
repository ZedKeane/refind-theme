import json
import shutil
from PIL import Image, ImageDraw, ImageFont
from catalog import ROOT


def build_english_assets(theme, font_path):
    catalog = json.loads((ROOT / 'catalog/interface.json').read_text())
    icons = theme / 'icons-en'
    if icons.exists():
        shutil.rmtree(icons)
    shutil.copytree(theme / 'icons', icons)
    scale = 4
    for name, caption in catalog['tool_captions'].items():
        path = icons / (name + '.png')
        with Image.open(path) as source:
            image = source.convert('RGBA')
        ImageDraw.Draw(image).rectangle((0, 94, image.width, image.height), fill=(0, 0, 0, 0))
        overlay = Image.new('RGBA', (image.width * scale, image.height * scale))
        font = ImageFont.truetype(str(font_path), 16 * scale)
        while font.getlength(caption) > 120 * scale:
            font = ImageFont.truetype(str(font_path), font.size - 1)
        ImageDraw.Draw(overlay).text((image.width * scale // 2, 100 * scale), caption, font=font, fill='black', anchor='mt')
        image.alpha_composite(overlay.resize(image.size, Image.Resampling.LANCZOS))
        image.save(path, optimize=True)
    background = Image.new('RGB', (2560 * scale, 1440 * scale), 'white')
    font = ImageFont.truetype(str(font_path), 18 * scale)
    ImageDraw.Draw(background).text((1280 * scale, 1396 * scale), catalog['footer'], font=font, fill='black', anchor='mt')
    background.resize((2560, 1440), Image.Resampling.LANCZOS).save(theme / 'background-en.png', optimize=True)
