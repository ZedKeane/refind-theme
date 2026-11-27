# 验证记录 / Validation

项目版本：0.2.4；配套程序：rEFInd 0.14.2-minimal0.2.3；系统目录：130 条、156 个系统图标文件名；界面目录：127 组中英文文字。

Project version: 0.2.4. Application: rEFInd 0.14.2-minimal0.2.3. The catalog contains 130 entries and 156 system icon filenames; the interface catalog contains 127 translation pairs.

## 构建与检查 / Build and checks

- 16 项宿主测试通过，覆盖登记 ID、目录、标题、共用目录、版本提示、中文别名、词边界、os-release 异常输入、磁盘名称、安装与回滚、配置保留及语言切换。双语测试重复切换全部字符串，检查静态菜单引用地址不变和 UTF-16 文字完整。
- 完整 x86_64 程序构建通过：macOS、Zig 0.13.0、GNU-EFI 3.0.18 SysV 包装调用及 GNU objcopy。上游代码保留既有编译警告；共享模块通过宿主 `-Wall -Wextra -Werror` 检查。
- 代码与配置的注释扫描通过；第三方声明保存在独立许可证目录。非界面修改的上游 C 文件与清理前进行词法比较，确认只改变注释和空白。
- ZIP 校验通过，逐文件 SHA-256 一致。发行 PNG 不含元数据，ZIP 使用固定时间戳，不包含开发目录、个人机器路径、AI 署名和原始字体文件。

All 16 host tests pass, including registry matching, malformed os-release input, disk model extraction, installation and rollback, configuration preservation and repeated language switching with stable menu pointers. The complete x86_64 application builds successfully. Comment and source-token checks pass. ZIP CRCs, file checksums and metadata checks pass.

## 图形与语言 / Graphics and languages

QEMU 9.2.0 / EDK2 使用独立 FAT/GPT 镜像、独立固件变量副本和 snapshot 写入层，未连接真实系统磁盘。

- 中文与英文主菜单、工具图片、底部提示和磁盘来源显示正确。
- 英文关于、启动选项和隐藏项目提示已检查；原有版权信息保留。
- 主题引用位于手动入口之后时，语言预扫描仍使入口来源采用正确语言。
- 在 1280×800 下检查分页、选择与箭头垂直对齐；程序会相应缩放系统图标和工具图标。
- 隔离 GPT 镜像中，Windows 自动解析为第 5 分区，Red Hat 为第 8 分区；独立 XBOOTLDR 启动分区排除在根分区判断之外，分区卷标不显示。
- NVMe 说明仅显示 `QEMU NVMe Ctrl` 型号，排除合成序列号和设备标识。
- `timeout 10` 的中英文截图均从 10 秒降至 8 秒，来源文字和倒计时分别居中。
- 原版 rEFInd 与 `theme.conf` 已隔离启动验证，使用独立 96 字符字体图集，英文工具图片与文字间距正常。

Isolated QEMU/EDK2 checks cover Chinese and English main menus, translated tool artwork and footer text, English About/submenus/hidden-item messages, late theme includes, scrolling arrows, physical partition descriptions and a countdown decreasing from 10 to 8 seconds. Standard rEFInd was also checked with the compatibility configuration and its separate 96-character font atlas.

## 基础功能验证 / Baseline validation

此前在相同识别实现上完成 SATA、NVMe 和 USB 磁盘名称查询；Windows 第 5 分区和 Linux 第 8 分区解析正确。NVMe 测试覆盖标准 20/40 字段和旧版 19/39 字段、型号连字符、容量、缓冲区边界、无效后缀和控制字符。ATA 测试覆盖交换字节，SCSI 测试排除修订号。

2560×1440 布局、原生工具栏、居中、启动返回和十个测试入口的翻页已完成基础验证。测试加载器替代真实操作系统，仅用于菜单和加载返回测试，不表示所有系统均已实机启动。

Baseline checks cover SATA, NVMe and USB names, both NVMe field layouts, model hyphens and capacities, buffer bounds and malformed input. The 2560×1440 layout and scrolling with ten stub loaders were also checked. Stub-loader tests do not establish that every listed operating system boots on physical hardware.

## 重现 / Reproduce

1. 按 README 构建并制作发行包，将主题和配套程序放入隔离 FAT 镜像的 `EFI/BOOT`，程序改名 `BOOTX64.EFI`。
2. 放入最小 EFI 测试加载器，创建 Windows、OpenCore、Red Hat 等手动入口，设置 `scanfor manual`，引用 `theme-zh.conf` 或 `theme-en.conf`。
3. 使用只读 EDK2 code 和独立 vars；测试镜像启用 snapshot。先设置 `timeout 0` 检查主菜单、二级菜单、工具栏和分页，再设置 `timeout 10` 检查倒计时。
4. 检查标准模式时，使用原版程序和 `theme.conf`；安装来源测试需要独立 GPT 分区及对应 BCD、根文件系统或引导配置元数据。

Build the release, place its files in an isolated EFI boot image, add stub loaders, and test each theme configuration with independent firmware variables and a snapshot disk. Use the supplied application for both language configurations and the upstream application for compatibility mode.

固件实际提供的图形模式、设备名称以及真实系统启动结果需要在目标硬件上确认。

Available graphics modes, firmware device names and real operating system boot results require verification on the target hardware.
