import os, subprocess, tempfile, unittest
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]

class DiskNameTests(unittest.TestCase):

    def test_hardware_fields_and_invalid_inputs(self):
        with tempfile.TemporaryDirectory() as tmp:
            exe = Path(tmp) / 'disk_name'
            subprocess.run([os.environ.get('CC', 'cc'), '-std=c99', '-Wall', '-Wextra', '-Werror', str(ROOT / 'tests/disk_name_harness.c'), str(ROOT / 'src/runtime/disk_name.c'), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
