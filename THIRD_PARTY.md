# 第三方来源

| 内容 | 固定版本/提交 | 来源与许可 |
|---|---|---|
| rEFInd 源码与既有图标 | 0.14.2 / cf542da；中英文修改版 minimal0.2.1 | [rEFInd](https://github.com/rodsmith/refind)，GPL-3.0-or-later，见 LICENSE 和 licenses/source-notices.txt |
| GNU-EFI | 3.0.18 | [GNU-EFI](https://sourceforge.net/projects/gnu-efi/)，原始声明见 licenses/source-notices.txt，各文件 BSD/LGPL 条款 |
| Minimal 原主题及部分图标 | 56a06a4bd9ed76c05523a22017056dbecca4145d | [rEFInd-minimal](https://github.com/EvanPurkhiser/rEFInd-minimal)，MIT，见 assets/theme-base/LICENSE |
| Simple Icons SVG | cd796c98565fc466923fc1128b962e133fa3c612 | [Simple Icons](https://github.com/simple-icons/simple-icons)，CC0，见 licenses/Simple-Icons-CC0.txt |
| Dashboard Icons 中少量 SVG | d767d7327db7f8ebd3715b552523f61889eea41c | [Dashboard Icons](https://github.com/homarr-labs/dashboard-icons)，Apache-2.0，见 licenses/Dashboard-Icons-Apache-2.0.txt |
| 几何图形和文字回退 | 本项目 | GPL-3.0-or-later |

素材均转换为黑色透明图像，裁切留白并增加名称；部分发行版沿用父系统标志。逐条来源和是否共用在 catalog/systems.json 中，渲染清单保存素材 SHA-256。第三方项目许可不授予商标使用权；品牌名称与标志仍归各权利人。

字体输入由用户提供（SF Pro Text、SF Mono、PingFang SC）；不包含原始苹果字体包。生成的本地主题用于用户本机，公开分发前应确认所用字体授权。

系统标识格式参考 [os-release 官方文档](https://www.freedesktop.org/software/systemd/man/latest/os-release.html)。

源码中的原始版权和许可注释按文件路径集中保存在 `licenses/source-notices.txt`，独立 LICENSE/COPYING 文件保留原文。

Original source copyright and license notices are retained by file path in `licenses/source-notices.txt`. Standalone LICENSE/COPYING files retain their original text.
