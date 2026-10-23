<div align="center">

# iPhone Duo for Nintendo 3DS

运行在 Nintendo 3DS 上的原生自制程序，还原 iPhone Duo 双屏桌面与跨屏交互。

[English](README.md) | [简体中文](README.zh-CN.md) | [编译 3DS 程序](#编译-3ds-程序) | [安装到 SD 卡](#安装到-sd-卡) | [按键与操作](#按键与操作)

[![Checks](https://github.com/JosssphZhou/iphone-duo-3ds/actions/workflows/ci.yml/badge.svg)](https://github.com/JosssphZhou/iphone-duo-3ds/actions)
![Platform](https://img.shields.io/badge/platform-Nintendo_3DS-222222)
![Languages](https://img.shields.io/badge/UI-English_%2F_中文-3478F6)
[![Code license](https://img.shields.io/badge/code-MIT-36A66B)](LICENSE)

</div>

## 编译 3DS 程序

先参照 [devkitPro 官方安装指南](https://devkitpro.org/wiki/Getting_Started) 安装 **devkitPro 3DS 工具链**（包含 `devkitARM`、`libctru`、`citro2d`、`citro3d`、`3dstools` 与 `tex3ds`），并确保本地已安装 Git、Python 3.9+ 和 `make`。

仓库已包含处理好的纹理素材，克隆后可直接编译：

```sh
git clone https://github.com/JosssphZhou/iphone-duo-3ds.git
cd iphone-duo-3ds
python3 tools/check_assets.py
cd iphone-duo-3ds-native
./build.sh -j4
```

构建脚本 `iphone-duo-3ds-native/build.sh` 会在 `iphone-duo-3ds-native/` 目录下生成 `iphone-duo.3dsx` 与 `iphone-duo.smdh`。脚本默认从 `/opt/devkitpro` 读取工具链；若安装在其他路径，请在编译前设置环境变量 `DEVKITPRO` 和 `DEVKITARM`。

## 安装到 SD 卡

准备一台已安装 **Homebrew Launcher** 的 3DS，将编译生成的两个文件拷入 SD 卡对应目录：

```text
SD:/
└── 3ds/
    └── iphone-duo/
        ├── iphone-duo.3dsx
        └── iphone-duo.smdh
```

所有图像资源均已通过 RomFS 打包进 `.3dsx`，无需把 `gfx/` 或网页原型目录拷到卡上。更新旧版本时，保留目录中原有的 `layout.bin` 和 `language.bin` 即可沿用之前的图标排布与语言设置。

将 SD 卡插回主机，在 Homebrew Launcher 中打开 **iPhone Duo** 即可运行。

## 按键与操作

| 操作 | 功能 |
| --- | --- |
| 点击图标，或方向键选中后按 A | 打开应用内页演示 |
| 长按图标并拖动 | 进入抖动编辑模式，调整图标位置 |
| 将音乐图标拖向两屏接缝 | 在另一块屏幕上展开音乐小组件 |
| 小组件展开时长按音乐图标 | 呼出移除菜单；松手后点击菜单将小组件收回 |
| B 或点击移除菜单外部 | 关闭移除菜单 |
| B | 退出桌面编辑状态或返回主屏幕 |
| L | 切换横屏与书本竖屏布局（同时关闭自动旋转） |
| R | 切换传感器调试信息显示 |
| X | 切换日间与夜间壁纸 |
| Y | 切换桌面分页 |
| 按住 SELECT 再按 Y | 切换中英文界面 |
| 单独按下并松开 SELECT | 锁屏或点亮屏幕 |
| 从触控屏底边向上滑动 | 解锁进入桌面 |
| START | 退出并返回 Homebrew Launcher |

硬件 HOME 键仍由 3DS 系统接管。重力感应自动旋转目前为实验功能，尚待真机校准，日常切换横竖屏请按 **L**。

## 主要特性

- **跨屏音乐小组件：** 长按**音乐**图标进入编辑模式，将其拖向转轴接缝即可在另一块屏幕上展开为音乐小组件。再次长按**音乐**并点击**移除组件**，小组件会动态缩回图标当前所在的格子（即使图标已被移到其他位置或另一页）。
- **自定义主屏幕：** 支持长按进入抖动编辑状态，在两页桌面与 Dock 之间自由换位，布局自动保存在 SD 卡上。
- **横屏与书本模式：** 按 **L** 可将双屏画面旋转 90 度切换为书本持握模式（原上屏在左、下触控屏在右）。
- **锁屏、主题与双语切换：** 支持上滑解锁、日夜壁纸切换，以及随时在中文和英文界面之间切换。

> **说明：** 本项目仅演示界面设计与双屏交互动效，不运行 iOS 应用，也不具备音频播放、通话或联网功能。

## 目录结构与技术实现

原生程序在上屏以 **400 × 240** 分辨率渲染，在下屏以 **320 × 240** 渲染；切换至书本模式后分别采用 **240 × 400** 与 **240 × 320** 的逻辑坐标，并同步变换触控输入映射（硬件上仅下屏支持触控）。

所有图标、文字标签与背景均按 3DS 原生像素尺寸预先栅格化，并在编译时打包为 RomFS 纹理图集。

```text
iphone-duo-3ds-native/       C 原生程序、纹理图集、交互逻辑与测试
iphone-duo-3ds-prototype/    网页交互原型与参考素材
docs/images/                 模拟器与网页原型运行截图
tools/                       素材校验、可选素材导入脚本与本地测试脚本
asset-manifest.json          945 个内置素材文件的大小与 SHA-256 清单
```

<details>
<summary><strong>本地测试与校验</strong></summary>

运行主机端交互逻辑与素材测试仅需 Python 3.9+ 和 C11 编译器，无需安装 3DS 工具链：

```sh
python3 tools/check_assets.py
bash tools/test_host.sh
python3 -m unittest discover -s tests -p '*_test.py' -v
```

在支持的编译器环境下，可运行 `SANITIZE=1 bash tools/test_host.sh` 启用 AddressSanitizer 与 UndefinedBehaviorSanitizer。

</details>

<details>
<summary><strong>素材清单与重生成流程</strong></summary>

`asset-manifest.json` 记录了全部内置纹理与网页素材的文件大小和 SHA-256 校验值。若本地素材被修改，可通过 `git checkout` 还原，或使用导入脚本从已准备好的目录恢复：

```sh
python3 tools/import_assets.py \
  --native /path/to/prepared/iphone-duo-3ds-native \
  --web /path/to/prepared/iphone-duo-3ds-prototype
```

覆盖已有文件需传入 `--replace`（覆盖前会自动备份）。仅在修改美术源图时才需要重新生成纹理，具体流程见 [ASSET_INPUTS.md](iphone-duo-3ds-native/ASSET_INPUTS.md)。

</details>

## 当前状态与已知限制

- **模拟器与真机测试：** 当前双语版本已在 Azahar 模拟器中完成验证，早期版本已在 3DS 真机运行；最新双语特性与重力感应旋转仍待真机复测。
- **静态应用预览：** 已配备素材的应用点击后展示静态参考界面；未提供内页素材的应用保留在桌面上，参考截图内的原始文字不做额外重绘翻译。
- **归档实验页面：** 早期实验性的 Music 与 App Store 内页未纳入默认导航流程，也未做完整双语适配。

## 网页原型与运行截图

<details>
<summary>查看网页交互原型说明与运行截图</summary>

`iphone-duo-3ds-prototype/` 是用于前期验证双屏布局与动效的独立网页原型，不参与 3DS 编译和安装。具体使用方式见[网页原型说明](iphone-duo-3ds-prototype/README.md)。

<img src="docs/images/book-widget.png" width="820" alt="网页原型书本模式运行画面">

*网页原型（书本模式）。*

<img src="docs/images/native-emulator.png" width="760" alt="原生 3DS 程序在 Azahar 模拟器中的运行画面">

*原生 3DS 程序在 Azahar 模拟器中运行。*

</details>

## 致谢与许可声明

本项目的原创代码采用 [MIT 许可证](LICENSE) 发布。仓库内包含的苹果参考图像、应用图标、界面截图、美术素材及相关第三方商标**不在 MIT 许可证授权范围内**，其权利归各自权利人所有。Noto Sans 字体遵循 SIL Open Font License。

本项目为独立爱好者作品，与 Apple 及任天堂（Nintendo）无任何隶属、赞助或官方背书关系。完整素材来源与权利说明见 [THIRD_PARTY.md](THIRD_PARTY.md)。
