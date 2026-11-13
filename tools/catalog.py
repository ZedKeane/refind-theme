from pathlib import Path
import json, re
ROOT = Path(__file__).resolve().parents[1]
FAMILIES = {'windows': 'OSR_WINDOWS', 'macos': 'OSR_MACOS', 'linux': 'OSR_LINUX', 'bsd': 'OSR_BSD', 'android': 'OSR_ANDROID', 'chromeos': 'OSR_CHROMEOS', 'boot': 'OSR_BOOT', 'other': 'OSR_OTHER'}

def load():
    data = json.loads((ROOT / 'catalog/systems.json').read_text())
    assert data['schema_version'] == 1
    systems = data['systems']
    ids = set()
    keys = {}
    rules = {}
    for s in systems:
        assert re.fullmatch('[a-z0-9][a-z0-9-]*', s['id']), s['id']
        assert s['id'] not in ids, s['id']
        ids.add(s['id'])
        assert s['family'] in FAMILIES
        assert s['title'] and len(s['title']) <= 40
        for key in [s['icon']['key']] + s['icon']['aliases']:
            assert re.fullmatch('[a-z0-9][a-z0-9-]*', key), key
            assert key not in keys or keys[key] == s['id'], (key, s['id'], keys.get(key))
            keys[key] = s['id']
        for kind in ['directories', 'release_ids', 'filenames', 'path_suffixes', 'release_variants', 'file_prefixes']:
            for value in s['match'][kind]:
                k = (kind, value.lower())
                assert k not in rules or rules[k] == s['id'], (k, rules.get(k), s['id'])
                rules[k] = s['id']
        asset = s['icon'].get('asset')
        if asset:
            assert (ROOT / asset).is_file(), asset
    for s in systems:
        assert not s['parent'] or s['parent'] in ids, s['id']
        seen = {s['id']}
        p = s['parent']
        while p:
            assert p not in seen, ('parent cycle', p)
            seen.add(p)
            p = next((x['parent'] for x in systems if x['id'] == p))
    return systems
