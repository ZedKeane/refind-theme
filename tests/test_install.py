import hashlib, importlib.util, json, sys, tempfile, unittest
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
spec = importlib.util.spec_from_file_location('installer', ROOT / 'tools/install.py')
installer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(installer)

class InstallTests(unittest.TestCase):

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.target = self.root / 'EFI/refind'
        self.target.mkdir(parents=True)
        (self.target / 'refind.conf').write_text('timeout 10\n')
        (self.target / 'refind_x64.efi').write_bytes(b'old')
        (self.root / 'EFI/OC').mkdir()
        (self.root / 'EFI/OC/OpenCore.efi').write_bytes(b'untouched')
        source = self.root / 'dist/EFI/refind'
        (source / 'themes/rEFInd-minimal').mkdir(parents=True)
        (source / 'refind_x64.efi').write_bytes(b'new')
        (source / 'themes/rEFInd-minimal/icon.png').write_bytes(b'icon')
        files = {str(p.relative_to(source)): hashlib.sha256(p.read_bytes()).hexdigest() for p in source.rglob('*') if p.is_file()}
        (self.root / 'dist/manifest.json').write_text(json.dumps({'files': files}))
        self.oldroot = installer.ROOT
        installer.ROOT = self.root
        self.argv = sys.argv

    def tearDown(self):
        installer.ROOT = self.oldroot
        sys.argv = self.argv
        self.tmp.cleanup()

    def run_install(self, *args):
        sys.argv = ['install', '--target', str(self.target), '--backup-dir', str(self.root / 'backups'), *args]
        installer.main()

    def test_plan_apply_rollback(self):
        self.run_install()
        self.assertEqual((self.target / 'refind_x64.efi').read_bytes(), b'old')
        self.run_install('--apply')
        self.assertEqual((self.target / 'refind_x64.efi').read_bytes(), b'new')
        self.assertEqual((self.target / 'refind.conf').read_text(), 'timeout 10\n')
        self.assertEqual((self.root / 'EFI/OC/OpenCore.efi').read_bytes(), b'untouched')
        backup = next((self.root / 'backups').iterdir())
        self.run_install('--rollback', str(backup), '--apply')
        self.assertEqual((self.target / 'refind_x64.efi').read_bytes(), b'old')
        self.assertFalse((self.target / 'themes/rEFInd-minimal/icon.png').exists())

    def test_bad_package_is_rejected(self):
        (self.root / 'dist/EFI/refind/refind_x64.efi').write_bytes(b'corrupt')
        with self.assertRaises(AssertionError):
            self.run_install('--apply')
        self.assertEqual((self.target / 'refind_x64.efi').read_bytes(), b'old')

    def test_traversal_is_rejected(self):
        (self.root / 'dist/manifest.json').write_text(json.dumps({'files': {'../OC/OpenCore.efi': 'bad'}}))
        with self.assertRaises(ValueError):
            self.run_install('--apply')
