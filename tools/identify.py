import argparse, json, os, subprocess, tempfile
from pathlib import Path
from catalog import ROOT, load
p = argparse.ArgumentParser()
for name in ['loader', 'title', 'fs-label', 'part-label', 'release-id', 'variant']:
    p.add_argument('--' + name, default='')
a = p.parse_args()
with tempfile.TemporaryDirectory() as tmp:
    exe = Path(tmp) / 'identify'
    subprocess.run([os.environ.get('CC', 'cc'), '-std=c99', '-fshort-wchar', str(ROOT / 'tests/registry_harness.c'), str(ROOT / 'src/runtime/os_registry.c'), '-o', str(exe)], check=True)
    result = subprocess.check_output([str(exe), a.loader, a.title, a.fs_label, a.part_label, a.release_id, a.variant], text=True).strip().split('|')
s = next((s for s in load() if s['id'] == result[0]))
print(json.dumps({'id': s['id'], 'title': s['title'], 'icon': 'os_' + s['icon']['key'] + '.png', 'score': int(result[1]), 'evidence': result[2]}, ensure_ascii=False, indent=2))
