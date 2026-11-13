# rEFInd Theme

[简体中文](README.zh-CN.md)

A white rEFInd theme with black icons and switchable English/Chinese interfaces.

- 130 system and boot-tool entries, including OpenCore with an Apple icon.
- Centered text, live countdown and text-only submenus.
- Automatic disk and physical-partition descriptions.

## Install

1. Download the theme from [Releases](https://github.com/ZedKeane/refind-theme/releases/latest) and back up your existing rEFInd files.
2. Copy `rEFInd-minimal` into `EFI/refind/themes/`.
3. Replace the existing `refind_x64.efi` with the supplied file from `runtime/`.
4. Add one theme reference to `refind.conf`:

```text
include themes/rEFInd-minimal/theme-en.conf
```

For Chinese, use `theme-zh.conf`. The language changes on the next start. Set `timeout 10` for a ten-second countdown.

The supplied application supports x86_64. For stock rEFInd, keep your application and use `theme.conf`; extended menus and detection require the supplied application.

## Develop

[Build](docs/BUILD.md) · [System catalog](docs/SYSTEMS.md) · [Maintenance](docs/MAINTAINING.md)

GPL-3.0-or-later. Artwork retains its original licenses; see [third-party notices](THIRD_PARTY.md).
