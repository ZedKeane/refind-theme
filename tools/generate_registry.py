from pathlib import Path
import argparse, json
from catalog import ROOT, load, FAMILIES

def literal(text):
    return 'L' + json.dumps(text, ensure_ascii=True)

def generated():
    systems = load()
    lines = ['#ifndef OS_REGISTRY_DATA_H', '#define OS_REGISTRY_DATA_H', 'typedef struct { unsigned system,kind,score; const osr_char *value; } OSR_RULE;', 'static const OSR_SYSTEM OSR_SYSTEMS[] = {']
    for s in systems:
        lines.append('{' + ','.join([json.dumps(s['id']), literal(s['title']), literal(s['icon']['key']), json.dumps(s['parent']) if s['parent'] else '0', FAMILIES[s['family']]]) + '},')
    lines += ['};', '#define OSR_SYSTEM_COUNT (sizeof(OSR_SYSTEMS)/sizeof(OSR_SYSTEMS[0]))', 'static const OSR_RULE OSR_RULES[] = {']
    kinds = [('directories', 1, 850), ('filenames', 2, 900), ('labels', 3, 450), ('release_ids', 4, 1200), ('path_suffixes', 5, 950), ('file_prefixes', 6, 100), ('release_variants', 7, 1300)]
    for (index, s) in enumerate(systems):
        for (field, kind, score) in kinds:
            for value in s['match'][field]:
                weight = 100 if s['match']['generic_loader'] and field in ('filenames', 'file_prefixes') else score
                lines.append('{' + f'{index},{kind},{weight},' + literal(value) + '},')
    lines += ['};', '#define OSR_RULE_COUNT (sizeof(OSR_RULES)/sizeof(OSR_RULES[0]))', '#endif', '']
    return '\n'.join(lines)
if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('--check', action='store_true')
    a = p.parse_args()
    target = ROOT / 'src/generated/os_registry_data.h'
    text = generated()
    if a.check:
        assert target.read_text() == text, 'Generated registry is stale; run tools/generate_registry.py'
    else:
        target.write_text(text)
    print('Registry:', len(load()), 'systems')
