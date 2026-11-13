import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def code_units(text):
    data = text.encode('utf-16-le') + b'\0\0'
    return [int.from_bytes(data[i:i + 2], 'little') for i in range(0, len(data), 2)]


def generate():
    items = json.loads((ROOT / 'catalog/translations.json').read_text())['strings']
    header = ['#ifndef UI_LANGUAGE_DATA_H', '#define UI_LANGUAGE_DATA_H']
    source = []
    rows = []
    ids = set()
    formats = re.compile(r'%(?:[-+ #0]*\d*(?:\.\d+)?[hlL]*[a-zA-Z]|%)')
    for item in items:
        key = item['id']
        if key in ids or not re.fullmatch('[a-z][a-z0-9_]*', key):
            raise ValueError(f'Invalid translation ID: {key}')
        ids.add(key)
        if formats.findall(item['zh']) != formats.findall(item['en']):
            raise ValueError(f'Format mismatch: {key}')
        values = [code_units(item[lang]) for lang in ('zh', 'en')]
        name = 'UI_' + key.upper()
        capacity = max(map(len, values))
        header.append(f'extern ui_char {name}[{capacity}];')
        source.append(f'ui_char {name}[{capacity}] = {{{", ".join(map(str, values[0]))}}};')
        for lang, units in zip(('zh', 'en'), values):
            source.append(f'static const ui_char {name}_{lang}[] = {{{", ".join(map(str, units))}}};')
        rows.append(f'    {{{name}, {name}_zh, {name}_en}}')
    header.append('#endif')
    source += ['static const UI_TRANSLATION UiTranslations[] = {', ',\n'.join(rows), '};']
    return {'src/generated/ui_language_data.h': '\n'.join(header) + '\n', 'src/generated/ui_language_data.inc': '\n'.join(source) + '\n'}


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    for relative, content in generate().items():
        path = ROOT / relative
        if args.check:
            if not path.is_file() or path.read_text() != content:
                raise SystemExit(f'Out of date: {relative}')
        else:
            path.write_text(content)
    print('Language catalog verified' if args.check else 'Language tables generated')
