import json, os, subprocess, sys, struct
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
cfg = json.loads((ROOT / '.local/toolchain.json').read_text())
mode = sys.argv[1]
args = sys.argv[2:]
env = os.environ.copy()
if cfg.get('dylibs'):
    env['DYLD_LIBRARY_PATH'] = cfg['dylibs']
cc = [cfg['zig'], 'cc', '-target', 'x86_64-linux-gnu', '-mcpu=baseline']
if mode == 'cc' and len(args) == 1 and (args[0] in ('-v', '-dumpversion', '-dumpmachine')):
    print({'-v': 'clang version 18.1.6 (Zig 0.13.0)', '-dumpversion': '18.1.6', '-dumpmachine': 'x86_64-linux-gnu'}[args[0]])
    sys.exit(0)
if mode == 'ld':
    out = None
    converted = []
    items = iter(args)
    for a in items:
        if a in ('-shared', '-nostdlib', '-znocombreloc'):
            continue
        if a == '-o':
            out = next(items)
            converted += ['-o', out]
        elif a.endswith(('.o', '.a', '.so')) or a.startswith(('-L', '-l')):
            converted.append(a)
        else:
            converted += ['-Xlinker', a]
    subprocess.run(cc + ['-nostdlib', '-shared', '-Wl,--hash-style=sysv', '-Wl,-znorelro'] + converted, check=True, env=env, stdin=subprocess.DEVNULL)
    p = Path(out)
    data = bytearray(p.read_bytes())
    assert data[:6] == b'\x7fELF\x02\x01'
    shoff = struct.unpack_from('<Q', data, 40)[0]
    (width, num) = struct.unpack_from('<HH', data, 58)
    sections = [struct.unpack_from('<IIQQQQIIQQ', data, shoff + i * width) for i in range(num)]
    for s in sections:
        if s[1] != 4:
            continue
        for off in range(s[4], s[4] + s[5], s[9]):
            (target, info, addend) = struct.unpack_from('<QQq', data, off)
            assert info & 4294967295 == 8
            sec = next((t for t in sections if t[3] <= target and target + 8 <= t[3] + t[5] and (t[1] != 8)))
            pos = sec[4] + target - sec[3]
            assert struct.unpack_from('<Q', data, pos)[0] in (0, addend)
            struct.pack_into('<Q', data, pos, addend)
    p.write_bytes(data)
else:
    command = {'cc': cc, 'ar': [cfg['zig'], 'ar'], 'ranlib': [cfg['zig'], 'ranlib'], 'objcopy': [cfg['objcopy']]}[mode]
    subprocess.run(command + args, check=True, env=env, stdin=subprocess.DEVNULL)
