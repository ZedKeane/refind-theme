PYTHON ?= python3
.PHONY: registry check theme core package release
registry:
	$(PYTHON) tools/generate_registry.py
	$(PYTHON) tools/generate_languages.py
check:
	$(PYTHON) tools/generate_registry.py --check
	$(PYTHON) tools/generate_languages.py --check
	$(PYTHON) -m unittest discover -s tests -v
theme: registry
	$(PYTHON) tools/build_theme.py --font "$(FONT)"
core: registry
	$(PYTHON) tools/build_core.py
package:
	$(PYTHON) tools/package.py --runtime-font "$(RUNTIME_FONT)"

release:
	$(PYTHON) tools/release.py
