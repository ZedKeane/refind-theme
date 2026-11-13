import subprocess, sys, shlex
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
wrapper = lambda mode: shlex.join([sys.executable, str(ROOT / 'tools/toolchain_driver.py'), mode])

def run(args, cwd):
    subprocess.run(['make', '-j4'] + args, cwd=cwd, check=True)
efi = ROOT / 'vendor/gnu-efi'
src = ROOT / 'vendor/refind'
base = ['ARCH=x86_64', 'CC=' + wrapper('cc'), 'AR=' + wrapper('ar'), 'RANLIB=' + wrapper('ranlib')]
run(base + ['GCCNEWENOUGH=0', 'USING_CLANG=clang', 'USING_APPLE=0', 'lib', 'gnuefi'], efi)
args = base + ['MAKEWITH=GNUEFI', 'CFLAGS=-Os -fno-strict-aliasing -fno-stack-protector -fshort-wchar -Wall -ffreestanding', 'EFIINC=' + str(efi / 'inc'), 'EFILIB=' + str(efi / 'x86_64/lib'), 'GNUEFILIB=' + str(efi / 'x86_64/gnuefi'), 'EFICRT0=' + str(efi / 'x86_64/gnuefi'), 'GNUEFI_LDSCRIPT=' + str(efi / 'gnuefi/elf_x86_64_efi.lds'), 'GNUEFI_LIBS=-lefi -lgnuefi', 'LD=' + wrapper('ld'), 'OBJCOPY=' + wrapper('objcopy')]
for sub in ['libeg', 'mok', 'EfiLib', 'gzip', 'refind']:
    for p in (src / sub).iterdir():
        if p.suffix in ('.o', '.a', '.so', '.efi'):
            p.unlink()
    run(args, src / sub)
out = ROOT / 'build'
out.mkdir(exist_ok=True)
(out / 'refind_x64.efi').write_bytes((src / 'refind/refind_x64.efi').read_bytes())
print('Built', out / 'refind_x64.efi')
