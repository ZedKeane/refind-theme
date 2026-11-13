import argparse
import hashlib
import json
import shutil
import stat
import subprocess
import zipfile
from pathlib import Path
from PIL import Image
from catalog import ROOT, load


def save_png(source, target):
    with Image.open(source) as original:
        clean = Image.frombytes(original.mode, original.size, original.tobytes())
        clean.save(target, format='PNG', optimize=True)
        with Image.open(target) as result:
            if original.mode != result.mode or original.size != result.size or original.tobytes() != result.tobytes():
                raise ValueError(f'Pixel mismatch: {source}')


def zip_files(path, files):
    with zipfile.ZipFile(path, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name, source in sorted(files.items()):
            entry = zipfile.ZipInfo(name, (1980, 1, 1, 0, 0, 0))
            entry.create_system = 3
            entry.external_attr = (stat.S_IFREG | 0o644) << 16
            entry.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(entry, source.read_bytes(), compresslevel=9)


def release(output):
    version = (ROOT / 'VERSION').read_text().strip()
    staged = ROOT / 'build/release-package'
    if staged.exists():
        shutil.rmtree(staged)
    package = staged / 'refind-theme'
    theme = package / 'rEFInd-minimal'
    theme.mkdir(parents=True)
    source_theme = ROOT / 'dist/EFI/refind/themes/rEFInd-minimal'
    for source in source_theme.rglob('*'):
        if not source.is_file() or source.name in ('theme.conf', 'catalog-manifest.json'):
            continue
        target = theme / source.relative_to(source_theme)
        target.parent.mkdir(parents=True, exist_ok=True)
        if source.suffix == '.png':
            save_png(source, target)
        else:
            shutil.copy2(source, target)
    config = (source_theme / 'theme.conf').read_text()
    lines = [line for line in config.splitlines() if line.strip() and not line.lstrip().startswith('#')]
    zh_config = 'language zh\n' + '\n'.join('arrow_icon_center 128' if line.startswith('arrow_icon_center ') else line for line in lines) + '\n'
    en_config = zh_config.replace('language zh', 'language en').replace('/icons\n', '/icons-en\n').replace('/background.png', '/background-en.png')
    native_config = '\n'.join(line.replace('apple-chinese-16.png', 'font-latin-16.png') for line in en_config.splitlines() if not line.startswith(('arrow_icon_center ', 'language '))) + '\n'
    (theme / 'theme-zh.conf').write_text(zh_config)
    (theme / 'theme-en.conf').write_text(en_config)
    (theme / 'theme.conf').write_text(native_config)
    for folder in ('icons', 'icons-en'):
        for direction in ('left', 'right'):
            path = theme / folder / f'arrow_{direction}.png'
            with Image.open(path) as image:
                image.load()
                moved = Image.new('RGBA', image.size)
                moved.paste(image, (0, -24))
                moved.save(path, optimize=True)
    with Image.open(theme / 'apple-chinese-16.png') as atlas:
        if atlas.height != 24 or atlas.width < 96 * 16:
            raise ValueError('Unexpected font atlas dimensions')
        latin = Image.new('RGBA', (96 * 10, 24))
        for index in range(96):
            latin.paste(atlas.crop((index * 16 + 3, 0, index * 16 + 13, 24)), (index * 10, 0))
        latin.save(theme / 'font-latin-16.png', optimize=True)
    runtime = package / 'runtime'
    runtime.mkdir()
    shutil.copy2(ROOT / 'dist/EFI/refind/refind_x64.efi', runtime)
    shutil.copy2(ROOT / 'docs/PACKAGE.md', package / 'README.md')
    shutil.copy2(ROOT / 'docs/SYSTEMS.md', package / 'SYSTEMS.md')
    shutil.copy2(ROOT / 'VERSION', package / 'VERSION')
    licenses = package / 'LICENSES'
    shutil.copytree(ROOT / 'licenses', licenses)
    shutil.copy2(ROOT / 'LICENSE', licenses / 'GPL-3.0.txt')
    shutil.copy2(ROOT / 'assets/theme-base/LICENSE', licenses / 'rEFInd-minimal-MIT.txt')
    shutil.copy2(ROOT / 'assets/theme-base/LICENSE-logos.md', licenses / 'rEFInd-minimal-logos.txt')
    (licenses / 'SOURCES.md').write_text((ROOT / 'THIRD_PARTY.md').read_text() + '\n文件路径对应配套源码包；当前目录保留所需许可和原始声明。\nPaths refer to the accompanying source archive; licenses and original notices are retained in this directory.\n')
    manifest = {str(p.relative_to(package)): hashlib.sha256(p.read_bytes()).hexdigest() for p in package.rglob('*') if p.is_file()}
    (package / 'SHA256SUMS.txt').write_text(''.join(f'{digest}  {name}\n' for name, digest in sorted(manifest.items())))
    output.mkdir(parents=True, exist_ok=True)
    binary_zip = output / f'refind-theme-{version}.zip'
    zip_files(binary_zip, {str(p.relative_to(staged)): p for p in package.rglob('*') if p.is_file()})
    names = subprocess.check_output(['git', 'ls-files', '--cached', '--others', '--exclude-standard', '-z'], cwd=ROOT).decode().split('\0')
    source_files = {f'refind-theme-source/{name}': ROOT / name for name in set(names) if name and (ROOT / name).is_file()}
    source_zip = output / f'refind-theme-{version}-source.zip'
    zip_files(source_zip, source_files)
    checksums = {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in (binary_zip, source_zip)}
    (output / 'SHA256SUMS.txt').write_text(''.join(f'{digest}  {name}\n' for name, digest in sorted(checksums.items())))
    print(json.dumps({'version': version, 'systems': len(load()), 'theme_files': len(manifest), 'source_files': len(source_files), 'archives': checksums}, indent=2))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, default=ROOT / 'releases')
    args = parser.parse_args()
    release(args.output)
