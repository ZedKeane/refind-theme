import ctypes
import importlib.util
import json
import os
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class LanguageTests(unittest.TestCase):
    def test_switch_preserves_menu_pointers_and_formats(self):
        spec = importlib.util.spec_from_file_location('languages', ROOT / 'tools/generate_languages.py')
        generator = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(generator)
        generator.generate()
        items = json.loads((ROOT / 'catalog/translations.json').read_text())['strings']
        with tempfile.TemporaryDirectory() as directory:
            library = Path(directory) / 'language.so'
            subprocess.run([os.environ.get('CC', 'cc'), '-std=c99', '-Wall', '-Wextra', '-Werror', '-shared', '-fPIC', str(ROOT / 'src/runtime/ui_language.c'), '-o', str(library)], check=True)
            runtime = ctypes.CDLL(str(library))
            runtime.ui_set_language.argtypes = [ctypes.c_int]
            runtime.ui_set_language.restype = None
            addresses = {}
            for language in ('zh', 'en', 'zh', 'en'):
                runtime.ui_set_language(language == 'en')
                for item in items:
                    name = 'UI_' + item['id'].upper()
                    size = max(len(generator.code_units(item[key])) for key in ('zh', 'en'))
                    buffer = (ctypes.c_ushort * size).in_dll(runtime, name)
                    address = ctypes.addressof(buffer)
                    self.assertEqual(address, addresses.setdefault(name, address))
                    units = list(buffer)
                    data = b''.join(value.to_bytes(2, 'little') for value in units[:units.index(0)])
                    self.assertEqual(data.decode('utf-16-le'), item[language], name)

    def test_generated_tables_are_current(self):
        subprocess.run([os.sys.executable, str(ROOT / 'tools/generate_languages.py'), '--check'], check=True)
