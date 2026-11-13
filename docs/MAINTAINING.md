# 维护指南

## 文件结构

- `catalog/systems.json`：系统标题、family、父系统、图标键/别名、素材路径/来源/许可、匹配证据。
- `assets/logos`：固定版本来源的 SVG/PNG，构建时不下载最新素材。
- `assets/theme-base`：白色背景、选中框、中文工具栏及指针等已有素材。
- `src/runtime/os_registry.c`：无操作系统依赖的识别器，UEFI 和宿主测试实际编译同一份 C。
- `src/runtime/disk_name.c` / `refind_disk_name.inc`：硬件名称字段解码与 UEFI 名称查询，按物理磁盘缓存，重新扫描时清除；不使用分区卷标。
- `src/runtime/refind_registry.inc`：只读磁盘解析与图标替换的 rEFInd 适配层。
- `src/generated`：由目录生成的 C 表，必须提交并通过 `--check`。
- `vendor/refind`：完整构建所需的 rEFInd 源文件，包含双语界面、字距、居中及自动分区说明实现。
- `vendor/gnu-efi`：固定 3.0.18 源码，构建使用 SysV 包装调用约定。
- `tools`：生成、构建、打包、校验、安装和回滚。
- `.local`：忽略的机器路径、字体配置和安装备份。`build` / `dist` 也不提交。

## 新增系统

1. 在目录中复制结构相近的条目，使用唯一小写 ID 和图标键。标题尽量短。`parent` 用于共用目录的衍生系统，不代表可以无证据自动判断版本。
2. 添加有来源和许可的素材；记录仓库、固定提交及源链接。没有可靠素材时使用 `asset: null`、`source: local-wordmark` 和 `wordmark`；共用父标志应标 `parent-mark` / `shared_with`。
3. `directories` 只填完整目录段；`release_ids` 对应 `os-release` 的 ID，`release_variants` 为 `ID:VARIANT_ID`；`labels` 为人工名称线索；通用 GRUB/shim 文件名仅给低权重通用识别。不要把短词加入宽泛子串匹配。
4. 运行 `python3 tools/lock_assets.py --write` 更新经过核对的素材校验锁，再运行生成和全部测试；新增有代表性的真实路径、歧义和误匹配测试。查看分类预览，确认图标居中、黑色、透明、名称不越界。
5. 构建程序和主题、打包，先在隔离测试盘验证，再更新 EFI。图标识别不等于该操作系统已通过实际启动测试。

## 优先级与限制

明确 Windows/macOS 文件优先级 1400；`ID:VARIANT_ID` 1300；`ID` 1200；路径后缀 950；专用文件名 900；完整目录段 850；名称/卷标约 450；通用加载器 100。同一父系统下有明确名称/专用目录可选择后代。未知 Linux ID 回落 Linux。来源说明与品牌推断分开，无法确定物理分区不影响已经有证据的品牌图标。

`/etc/os-release` 优先于 `/usr/lib/os-release`，只解析 ID 和 VARIANT_ID，不执行文件内容，最多读取 4 MiB；重复键以最后有效值为准。UEFI 文件系统驱动能读取目标根分区时才使用系统标识。没有驱动读取 XFS 的情况下，Red Hat 仍可由 `EFI/redhat` 识别，分区说明使用原有 GPT/XFS 元数据逻辑。

## 修改上游代码

适配入口在 `config.c` 的 `ApplySystemLocation`；`scan.c` / `global.h` 保存显式图标优先级；`refind/Makefile` 编入 `os_registry.o`。新增品牌通常只改目录与素材，不改这些 C 接口。

GNU-EFI 库必须保持 `GCCNEWENOUGH=0` 与 rEFInd 的 EFI 包装调用一致。LLD 生成 RELA，`toolchain_driver.py` 在链接后写入 addend，以适配 GNU-EFI 的原地重定位规则。不要全局添加 `GNU_EFI_USE_MS_ABI`。静态库使用替换式归档；构建会清理 rEFInd 对象，避免旧头文件依赖或旧归档成员影响结果。

## 验证层级

CI 检查目录一致性、所有登记 ID/目录/标题、共用目录分支、中文名称、词边界、恶意/异常 os-release，以及安装/保留配置/校验失败/回滚。UEFI 图形效果、计时和启动返回还需 QEMU/EDK2 测试，测试步骤与验证记录见 `docs/VALIDATION.md`。宿主测试通过不代表每一项系统都经过实际启动验证。

## 磁盘来源说明

0.14.2-zh3.3 起只显示磁盘名称及物理分区号。ATA Identify 使用交换字节的型号字段，USB/SCSI 使用 Inquiry 的厂商和产品字段；NVMe Disk Info Identify 返回 namespace 数据，不能当作控制器型号解码，因此通过 Component Name 的父控制器与物理子设备关系取设备名称。只查询这块物理磁盘的祖先路径，不查询分区名称或执行写盘命令。名称和分区都在启动/重扫时重新识别。

协议依据：[PI Disk Info](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/DiskInfo.h)、[UEFI Component Name 2](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/ComponentName2.h)。

## NVMe 型号精简

zh3.4 对 NVMe 物理设备的 EDK2 组合名称先按字段提取型号。标准布局为 20 字节序列号、分隔符、40 字节型号、分隔符、EUI-64 十六进制标识；同时兼容较旧的 19/39 字符布局。提取发生在压缩空白之前，保留型号中的连字符、数字和容量。只有 NVMe 设备且边界及标识格式完整匹配时才处理，不对普通名称按连字符截取。源数据、磁盘序列号、系统分区识别不被修改。维护依据：[EDK2 NVMe 名称构造](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Bus/Pci/NvmExpressDxe/NvmExpress.c)。

## 中英文界面 / Bilingual interface

`catalog/translations.json` 是程序文字的唯一来源；`tools/generate_languages.py` 生成 UTF-16 表并检查 printf 格式一致。静态菜单引用固定地址的可写字符串，语言预扫描发生在生成手动入口之前，因此主题 include 放在配置末尾也能正确切换。仅解析当前配置和一层 include，与主配置的 include 范围一致。

`catalog/interface.json` 提供英文工具标题和底部提示。构建时复用同一套图形，移除图标下部原有文字并重新排版英文；系统图标共用原图。`tools/release.py` 生成 `theme-zh.conf`、`theme-en.conf` 和标准程序兼容的 `theme.conf`。新增程序文字需加入双语目录。

`catalog/translations.json` is the source of application strings. The generator checks matching printf formats and emits UTF-16 tables. Static menus reference buffers with stable addresses. A language pre-pass runs before manual entries are created, including when the theme reference is last in the configuration. It reads the primary configuration and one include level, matching the configuration parser.

`catalog/interface.json` supplies English tool captions and footer text. Both languages reuse the same artwork. The release tool generates Chinese, English and standard-application configurations. Add new application text to the bilingual catalog.

源码与配置不保留说明性注释；第三方原始声明按文件路径归档在 `licenses/source-notices.txt`。发行 ZIP 使用固定时间戳，不包含 Git、机器路径、开发缓存或原始字体文件。

Source and configuration files omit explanatory comments. Third-party original notices are archived by path in `licenses/source-notices.txt`. Release ZIPs use fixed timestamps and exclude Git metadata, machine paths, development caches and original font files.
