import argparse, hashlib, json, shutil
from pathlib import Path
from catalog import ROOT, load
p = argparse.ArgumentParser()
p.add_argument('--runtime-font', type=Path)
a = p.parse_args()
cfg = json.loads((ROOT / '.local/toolchain.json').read_text()) if (ROOT / '.local/toolchain.json').exists() else {}
font = a.runtime_font or Path(cfg.get('runtime_font', 'build/apple-chinese-16.png'))
assert font.is_file(), 'Run build_font.py or supply --runtime-font'
assert (ROOT / 'build/refind_x64.efi').is_file()
dist = ROOT / 'dist'
dist.mkdir(exist_ok=True)
dest = dist / 'EFI/refind'
dest.mkdir(parents=True, exist_ok=True)
shutil.copy2(ROOT / 'build/refind_x64.efi', dest)
theme = dest / 'themes/rEFInd-minimal'
if theme.exists():
    shutil.rmtree(theme)
shutil.copytree(ROOT / 'build/theme', theme)
shutil.copy2(font, theme / 'apple-chinese-16.png')
manifest = {str(f.relative_to(dest)): hashlib.sha256(f.read_bytes()).hexdigest() for f in dest.rglob('*') if f.is_file()}
(dist / 'manifest.json').write_text(json.dumps({'version': (ROOT / 'VERSION').read_text().strip(), 'systems': len(load()), 'files': manifest}, indent=2) + '\n')
print('Packaged', len(manifest), 'owned files:', dest)
