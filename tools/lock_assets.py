import argparse, hashlib, json
from catalog import ROOT, load
p = argparse.ArgumentParser()
p.add_argument('--write', action='store_true')
args = p.parse_args()
assets = {str(f.relative_to(ROOT)): hashlib.sha256(f.read_bytes()).hexdigest() for folder in ['assets/logos', 'assets/theme-base'] for f in sorted((ROOT / folder).rglob('*')) if f.is_file()}
data = json.dumps({'schema_version': 1, 'sha256': assets}, indent=2) + '\n'
path = ROOT / 'catalog/assets.lock.json'
if args.write:
    path.write_text(data)
else:
    assert path.read_text() == data, 'Asset lock changed: review sources, then run lock_assets.py --write'
print('Verified asset inputs:', len(assets))
