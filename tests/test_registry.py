import json, os, subprocess, tempfile, unittest
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]

class RegistryTests(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.exe = Path(cls.tmp.name) / 'registry'
        subprocess.run([os.environ.get('CC', 'cc'), '-std=c99', '-fshort-wchar', '-Wall', '-Wextra', '-Werror', str(ROOT / 'tests/registry_harness.c'), str(ROOT / 'src/runtime/os_registry.c'), '-o', str(cls.exe)], check=True)
        cls.systems = json.loads((ROOT / 'catalog/systems.json').read_text())['systems']

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def match(self, path='', title='', fs='', part='', id='', variant=''):
        return subprocess.check_output([str(self.exe), path, title, fs, part, id, variant], text=True).strip().split('|')[0]

    def test_catalog_exact_ids(self):
        for s in self.systems:
            for id in s['match']['release_ids']:
                with self.subTest(id=id):
                    self.assertEqual(self.match(id=id), s['id'])

    def test_catalog_directories(self):
        for s in self.systems:
            for directory in s['match']['directories']:
                with self.subTest(directory=directory):
                    self.assertEqual(self.match(path='/EFI/' + directory + '/entry.efi'), s['id'])

    def test_catalog_labels(self):
        for s in self.systems:
            with self.subTest(title=s['title']):
                self.assertEqual(self.match(title=s['title']), s['id'])

    def test_primary_paths(self):
        for (p, e) in [('/EFI/OC/OpenCore.efi', 'macos'), ('/EFI/Microsoft/Boot/bootmgfw.efi', 'windows'), ('/EFI/redhat/grubx64.efi', 'rhel'), ('/EFI/ubuntu/shimx64.efi', 'ubuntu'), ('/EFI/debian/grubx64.efi', 'debian'), ('/EFI/BOOT/BOOTX64.EFI', 'uefi'), ('/EFI/freebsd/loader.efi', 'freebsd')]:
            self.assertEqual(self.match(path=p), e)

    def test_specificity(self):
        self.assertEqual(self.match(path='/EFI/redhat/grubx64.efi', id='rocky'), 'rocky')
        self.assertEqual(self.match(path='/EFI/ubuntu/grubx64.efi', id='linuxmint'), 'linuxmint')
        self.assertEqual(self.match(path='/EFI/ubuntu/grubx64.efi', title='LMDE'), 'lmde')
        self.assertEqual(self.match(path='/EFI/ubuntu/grubx64.efi', title='Kubuntu'), 'kubuntu')
        self.assertEqual(self.match(path='/EFI/fedora/grubx64.efi', id='fedora', variant='silverblue'), 'silverblue')
        self.assertEqual(self.match(path='/EFI/fedora/grubx64.efi', id='unlisted-os'), 'linux')
        self.assertEqual(self.match(path='/EFI/Microsoft/Boot/bootmgfw.efi', title='Windows 11', id='ubuntu'), 'windows11')
        self.assertEqual(self.match(path='/EFI/ubuntu/grubx64.efi', title='Windows 11'), 'ubuntu')

    def test_ventoy_boot_media(self):
        for path in ('/EFI/BOOT/BOOTX64.EFI', '/EFI/BOOT/grubx64_real.efi'):
            with self.subTest(path=path):
                self.assertEqual(self.match(path=path, fs='VTOYEFI'), 'ventoy')
                self.assertEqual(self.match(path=path, part='VTOYEFI'), 'ventoy')
        for name in ('ventoy_x64.efi', 'ventoy_ia32.efi', 'ventoy_aa64.efi', 'ventoy_mips64.efi'):
            self.assertEqual(self.match(path='/EFI/BOOT/' + name), 'ventoy')
        self.assertEqual(self.match(path='/EFI/BOOT/BOOTX64.EFI', fs='Installer'), 'uefi')
        self.assertEqual(self.match(path='/EFI/BOOT/BOOTX64.EFI', fs='NOTVTOYEFI'), 'uefi')
        self.assertEqual(self.match(path='/EFI/Microsoft/Boot/bootmgfw.efi', fs='VTOYEFI'), 'windows')

    def test_generated_table_is_current(self):
        subprocess.run([os.sys.executable, str(ROOT / 'tools/generate_registry.py'), '--check'], check=True)

    def test_asset_lock(self):
        subprocess.run([os.sys.executable, str(ROOT / 'tools/lock_assets.py')], check=True)

    def test_boundaries(self):
        for title in ['Research archive', 'searcharch', 'minted', 'steamroller', 'voided', 'deepinside']:
            with self.subTest(title=title):
                self.assertEqual(self.match(path='/EFI/BOOT/BOOTX64.EFI', title=title), 'uefi')
        self.assertEqual(self.match(path='/EFI/research/entry.efi'), 'uefi')
        self.assertEqual(self.match(title='银河麒麟'), 'kylin')
        self.assertEqual(self.match(title='开放麒麟'), 'openkylin')

    def test_release_parser(self):
        for (source, expected) in [('ID="ubuntu"\nVARIANT_ID=desktop\n', 'ubuntu|desktop'), ('# ID=bad\r\nID=debian\r\nID=fedora\r\n', 'fedora|'), ("ID='rocky'\n", 'rocky|'), ('ID=$(danger)\n', '|'), ('ID="unterminated\n', '|'), ('PRETTY_NAME=Fedora\nID=debian # comment\n', 'debian|')]:
            with self.subTest(source=source):
                self.assertEqual(subprocess.check_output([str(self.exe), '--release', source], text=True).strip(), expected)
if __name__ == '__main__':
    unittest.main()
