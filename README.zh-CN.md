# rEFInd Theme

[English](README.md)

纯白背景、黑色图标，支持中英文界面切换的 rEFInd 主题。

- 130 个系统与启动工具条目，OpenCore 使用苹果图标。
- 文字居中、动态倒计时、无图标二级菜单。
- 自动显示磁盘来源与物理分区序号。

## 安装

1. 从 [Releases](https://github.com/ZedKeane/refind-theme/releases/latest) 下载主题包，备份现有 rEFInd 文件。
2. 将 `rEFInd-minimal` 复制到 `EFI/refind/themes/`。
3. 用包内 `runtime/` 的程序替换现有 `refind_x64.efi`。
4. 在 `refind.conf` 中加入一个主题引用：

```text
include themes/rEFInd-minimal/theme-zh.conf
```

英文使用 `theme-en.conf`，下次进入时生效。`timeout 10` 表示十秒倒计时。

配套程序适用于 x86_64。原版 rEFInd 可保留原程序并引用 `theme.conf`；完整双语菜单与扩展识别需要配套程序。

## 开发

[构建](docs/BUILD.md) · [系统目录](docs/SYSTEMS.md) · [维护](docs/MAINTAINING.md)

代码使用 GPL-3.0-or-later，图标沿用原始许可，详见[第三方声明](THIRD_PARTY.md)。
