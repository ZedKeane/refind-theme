import argparse, datetime, hashlib, json, os, shutil, tempfile, zipfile
from pathlib import Path, PurePosixPath
from catalog import ROOT

def safe(name):
    p = PurePosixPath(name)
    if p.is_absolute() or '..' in p.parts or (not (name == 'refind_x64.efi' or name.startswith('themes/rEFInd-minimal/'))):
        raise ValueError('Unowned path: ' + name)
    return p

def digest(p):
    return hashlib.sha256(p.read_bytes()).hexdigest() if p.is_file() else None

def atomic(dst, data):
    dst.parent.mkdir(parents=True, exist_ok=True)
    (fd, tmp) = tempfile.mkstemp(prefix='.refind-update-', dir=dst.parent)
    try:
        with os.fdopen(fd, 'wb') as f:
            f.write(data)
            f.flush()
            os.fsync(f.fileno())
        os.replace(tmp, dst)
    finally:
        if os.path.exists(tmp):
            os.unlink(tmp)

def main():
    p = argparse.ArgumentParser()
    p.add_argument('--target', type=Path, required=True, help='Mounted EFI/refind directory')
    p.add_argument('--apply', action='store_true')
    p.add_argument('--backup-dir', type=Path, default=ROOT / '.local/backups')
    p.add_argument('--rollback', type=Path, help='Restore a backup directory')
    a = p.parse_args()
    target = a.target.resolve()
    assert (target / 'refind.conf').is_file(), 'Expected existing EFI/refind with refind.conf'

    def checked(name):
        safe(name)
        dst = target / name
        assert dst.resolve().is_relative_to(target), 'Symlink escapes target'
        return dst
    if a.rollback:
        record = json.loads((a.rollback / 'changes.json').read_text())
        for (name, info) in record.items():
            dst = checked(name)
            assert digest(dst) == info['after'], 'Changed since installation: ' + name
        if not a.apply:
            print('Rollback plan:', len(record), 'files')
            return
        for (name, info) in record.items():
            dst = checked(name)
            if info['before'] is None:
                dst.unlink()
            else:
                data = (a.rollback / 'files' / name).read_bytes()
                assert hashlib.sha256(data).hexdigest() == info['before']
                atomic(dst, data)
        print('Restored', len(record), 'files')
        return
    manifest = json.loads((ROOT / 'dist/manifest.json').read_text())['files']
    source = ROOT / 'dist/EFI/refind'
    changes = {}
    for (name, expected) in manifest.items():
        dst = checked(name)
        assert digest(source / name) == expected, 'Package checksum mismatch: ' + name
        previous = digest(dst)
        if previous != expected:
            changes[name] = {'before': previous, 'after': expected}
    print(json.dumps({'target': str(target), 'changed_files': len(changes), 'preserved_config_sha256': digest(target / 'refind.conf')}, ensure_ascii=False, indent=2))
    if not a.apply or not changes:
        return
    backup = a.backup_dir / datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
    backup.mkdir(parents=True, exist_ok=False)
    for (name, info) in changes.items():
        if info['before'] is not None:
            dst = backup / 'files' / name
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(checked(name), dst)
    (backup / 'changes.json').write_text(json.dumps(changes, indent=2) + '\n')
    config_before = digest(target / 'refind.conf')
    for name in changes:
        atomic(checked(name), (source / name).read_bytes())
    assert all((digest(checked(name)) == info['after'] for (name, info) in changes.items()))
    assert digest(target / 'refind.conf') == config_before
    print('Installed and verified. Backup:', backup)
if __name__ == '__main__':
    main()
