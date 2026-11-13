# rEFInd Theme

## English

White background, black icons, switchable English/Chinese menus and text-only submenus. The supplied application supports x86_64.

1. Back up your existing rEFInd files.
2. Copy `rEFInd-minimal` into `EFI/refind/themes/`.
3. Replace the existing `EFI/refind/refind_x64.efi` with the supplied application from `runtime/`.
4. Add one reference to `refind.conf`:

```text
include themes/rEFInd-minimal/theme-en.conf
```

For Chinese, use `theme-zh.conf`. Restart rEFInd to apply the language. Keep your scan settings and boot entries. Set `timeout 10` for ten seconds.

For stock rEFInd, keep your application and use `theme.conf`. Extended language and detection features require the supplied application. Disk descriptions are detected at startup; ambiguous system partitions are reported explicitly.

## 简体中文

纯白背景、黑色图标，中英文菜单可切换，二级菜单只保留文字。配套程序适用于 x86_64。

1. 备份现有 rEFInd 文件。
2. 将 `rEFInd-minimal` 复制到 `EFI/refind/themes/`。
3. 用 `runtime/` 中的程序替换现有 `EFI/refind/refind_x64.efi`。
4. 在 `refind.conf` 中加入一个引用：

```text
include themes/rEFInd-minimal/theme-zh.conf
```

英文使用 `theme-en.conf`，下次进入 rEFInd 时生效。保留原有扫描与系统入口，`timeout 10` 表示十秒倒计时。

原版 rEFInd 可保留原程序并引用 `theme.conf`，完整双语和扩展识别需要配套程序。磁盘来源启动时自动检测，无法唯一确定系统分区时会提示。

## Files / 文件

- `rEFInd-minimal/`: theme assets / 主题素材。
- `runtime/`: bilingual application / 双语程序。
- `SYSTEMS.md`: system catalog / 系统目录。
- `LICENSES/`: licenses and original notices / 许可与原始声明。
- `SHA256SUMS.txt`: file checksums / 文件校验。

Source: https://github.com/ZedKeane/refind-theme
