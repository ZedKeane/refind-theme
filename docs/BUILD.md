# Build / 构建

纯白背景、纯黑图标、居中布局的 rEFInd 主题包，提供可切换的中文和英文界面。包含 130 个系统、版本及启动工具条目，156 个系统图标文件名。OpenCore 识别为 macOS，使用苹果图标。

A white rEFInd theme with black icons, centered text and switchable Chinese/English interfaces. Includes 130 system, version and boot-tool entries and 156 system icon filenames. OpenCore is recognized as macOS and uses the Apple icon.

安装与语言切换见 [主题包说明 / Installation and language selection](docs/PACKAGE.md)。完整菜单语言、动态倒计时、系统识别和磁盘来源功能使用配套程序；`theme.conf` 提供原版 rEFInd 兼容模式。

The supplied application provides bilingual menus, a live countdown, extended system detection and physical disk source descriptions. Use `theme.conf` for standard rEFInd compatibility.

## 构建 / Build

要求 Python 3.9+、GNU Make、Zig 0.13.0 和支持 `efi-app-x86_64` 的 GNU objcopy。测试需要宿主 C 编译器，完整程序目标为 x86_64。构建路径应避免空格。

Requires Python 3.9+, GNU Make, Zig 0.13.0 and GNU objcopy with `efi-app-x86_64` support. Tests also require a host C compiler. The application target is x86_64; avoid spaces in the build path.

```sh
python3 -m venv .venv
.venv/bin/python -m pip install -r requirements.txt
mkdir -p .local
cp config/toolchain.example.json .local/toolchain.json
```

在 `.local/toolchain.json` 填写工具与字体路径；`dylibs` 可指定 objcopy 的动态库目录。字体输入由用户提供，原始苹果字体不随仓库分发。

Set tool and font paths in `.local/toolchain.json`; the optional `dylibs` key sets the dynamic library directory for objcopy. Supply your own licensed fonts; original Apple font files are not distributed.

```sh
python3 tools/generate_registry.py
python3 tools/generate_languages.py
python3 tools/lock_assets.py
python3 -m unittest discover -s tests -v
.venv/bin/python tools/build_theme.py --font /path/to/SF-Pro-Text-Regular.otf
python3 tools/build_core.py
.venv/bin/python tools/build_font.py --mono /path/to/SF-Mono-Regular.otf --cjk /path/to/PingFangSC-Regular.otf --output build/apple-chinese-16.png
python3 tools/package.py --runtime-font build/apple-chinese-16.png
.venv/bin/python tools/release.py
```

`build/theme` 保存生成主题，`build/refind_x64.efi` 为配套程序，`dist` 用于安装器，`releases` 保存主题与源码 ZIP。依赖提前安装后可离线构建。中英文发行配置由 release 工具生成。

Generated assets are in `build/theme`, the application is `build/refind_x64.efi`, installer files are in `dist`, and release ZIPs are in `releases`. Builds work offline after dependencies are installed. The release tool generates the Chinese and English theme configurations.

## 识别与维护 / Detection and maintenance

系统、图标及识别规则集中在 [catalog/systems.json](catalog/systems.json)，界面文字集中在 [catalog/translations.json](catalog/translations.json)，英文图片说明集中在 [catalog/interface.json](catalog/interface.json)。生成文件必须通过 `--check`。新增品牌通常无需修改程序接口。

System and icon rules live in `catalog/systems.json`, interface strings in `catalog/translations.json`, and English image captions in `catalog/interface.json`. Generated files must pass `--check`. Adding a brand normally requires no application interface changes.

识别优先采用明确加载器、目录段及可读安装分区中的 os-release；明确指定的图标优先。来源动态读取磁盘、GPT、BCD 和引导配置，显示磁盘型号与物理分区号。NVMe 名称排除固件组合字段中的序列号和 EUI 标识。复杂加密、多根分区及不可读取文件系统可能限制来源判断。

Detection uses explicit loaders, directory segments and readable os-release data. Explicitly configured icons take precedence. Source descriptions use live disk, GPT, BCD and boot configuration data to show the hardware model and physical partition number. NVMe model extraction excludes serial and EUI fields. Encryption, multiple roots and inaccessible filesystems can limit partition detection.

更多说明见 [维护指南 / Maintenance](docs/MAINTAINING.md)、[系统目录 / Systems](docs/SYSTEMS.md)、[验证记录 / Validation](docs/VALIDATION.md)。

## 安装器 / Installer

安装器默认仅显示计划，保留现有 `refind.conf`；发行包的首次安装及语言切换按包内说明执行。

The installer previews changes by default and preserves `refind.conf`. Follow the package instructions for initial setup and language selection.

```sh
python3 tools/install.py --target /path/to/EFI/refind
python3 tools/install.py --target /path/to/EFI/refind --apply
python3 tools/install.py --target /path/to/EFI/refind --rollback .local/backups/YYYYMMDD-HHMMSS --apply
```

## 许可 / Licenses

代码使用 GPL-3.0-or-later；图标素材沿用各自许可。第三方原始声明集中保留在 `licenses/source-notices.txt`，文件来源见 [THIRD_PARTY.md](THIRD_PARTY.md)。

Code is licensed under GPL-3.0-or-later; artwork retains its individual licenses. Original third-party source notices are retained in `licenses/source-notices.txt`. See [THIRD_PARTY.md](THIRD_PARTY.md) for provenance.
