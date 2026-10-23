<div align="center">

# iPhone Duo for Nintendo 3DS

### 两块屏幕，一台意想不到的 iPhone。

参考 iPhone Duo 发布界面制作的原生 3DS 自制程序，附带可交互的浏览器原型。

[English](README.md) | [简体中文](README.zh-CN.md) | [浏览器体验](#浏览器体验) | [构建与安装](#构建与安装)

[![Checks](https://github.com/JosssphZhou/iphone-duo-3ds/actions/workflows/ci.yml/badge.svg)](https://github.com/JosssphZhou/iphone-duo-3ds/actions)
![Platform](https://img.shields.io/badge/platform-Nintendo_3DS-222222)
![Languages](https://img.shields.io/badge/UI-English_%2F_中文-3478F6)
[![Code license](https://img.shields.io/badge/code-MIT-36A66B)](LICENSE)

<img src="docs/images/book-widget.png" width="820" alt="竖屏书本模式的网页原型：左侧音乐组件，右侧触控桌面">

*上图为竖屏书本模式下的可交互网页原型，下方另有原生 3DS 程序运行截图。*

</div>

## 最好玩的地方，在两块屏幕之间

长按**音乐**图标进入抖动编辑状态，把它拖向两屏接缝，图标就会跨过触控屏，在另一块屏幕上展开成音乐小组件。

想收回来？再次长按**音乐**，点击**移除组件**，小组件便会缩回图标当前所在的格子。就算你已经把图标挪到了别的位置或另一页，收回动画也会准确跟过去。

| 整理桌面 | 转过来玩 | 熟悉的细节 |
| --- | --- | --- |
| 长按抖动、拖动换位，支持两页桌面与 Dock。 | 按 **L** 切换书本模式，原上屏在左，触控屏在右。 | 上滑解锁、日夜壁纸切换，以及中英文界面切换。 |

> **项目边界说明：** 本项目复刻的是双屏界面与交互动效，不运行 iOS 或 iPhone 应用。音乐组件与应用内页均为离线视觉演示，不播放音乐、不通话，也不下载应用。

## 浏览器体验

**图片与纹理素材已包含在仓库中。** 克隆仓库后直接启动本地静态服务即可体验，无需额外导入素材或安装前端依赖。

```sh
git clone https://github.com/JosssphZhou/iphone-duo-3ds.git
cd iphone-duo-3ds
python3 -m http.server 8000
```

在浏览器打开 **http://localhost:8000/iphone-duo-3ds-prototype/**，即可用鼠标或触控屏交互。点击 **中文 / EN** 可切换界面语言，点击**录制舞台**可放大机身画面以便录屏或截图。

浏览器版本是配套的交互原型，不代表实体 3DS 的硬件性能表现。更多细节见[网页原型说明](iphone-duo-3ds-prototype/README.md)。

## 构建与安装

先安装 **devkitPro 3DS 工具链**（包含 `devkitARM`、`libctru`、`citro2d`、`citro3d`、`3dstools` 和 `tex3ds`）。所需预制纹理已随仓库提交，可直接校验并编译：

```sh
python3 tools/check_assets.py
cd iphone-duo-3ds-native
./build.sh -j4
```

将编译生成的两个文件复制到 SD 卡对应目录：

```text
SD:/
└── 3ds/
    └── iphone-duo/
        ├── iphone-duo.3dsx
        └── iphone-duo.smdh
```

从 **Homebrew Launcher** 启动 **iPhone Duo**。覆盖更新时，保留原有的 `layout.bin` 和 `language.bin` 即可延续此前的图标排布与语言设置。

<img src="docs/images/native-emulator.png" width="760" alt="原生中英文 3DS 程序在 Azahar 中运行，桌面显示英文图标文字">

*上图为原生 C 程序在 Azahar 模拟器中的运行画面（非网页原型）。中英文 v10 版本已完成模拟器验证，实体机仍待复测。*

## 按键与操作

| 操作 | 作用 |
| --- | --- |
| 点击图标，或方向键选择后按 A | 打开已有的应用内页演示 |
| 长按图标后拖动 | 进入编辑状态并整理桌面图标 |
| 把音乐图标拖向两屏接缝 | 在上屏添加音乐小组件 |
| 已有组件时长按音乐图标 | 呼出移除菜单；松手后点击菜单即可移除组件 |
| B 或点击菜单外部 | 取消移除菜单 |
| B | 结束桌面整理或返回桌面 |
| L | 切换横屏与书本模式，并关闭自动旋转 |
| R | 显示或隐藏传感器诊断信息 |
| X | 切换日间与夜间壁纸 |
| Y | 切换桌面分页 |
| 按住 SELECT 后按 Y | 切换中英文界面 |
| 单独按下并松开 SELECT | 切换锁屏状态 |
| 从触控屏底部边缘向上滑动 | 解锁进入桌面 |
| START | 退出并返回 Homebrew Launcher |

硬件 HOME 键仍由 3DS 系统接管。重力感应自动旋转目前属于实验特性，仍待实体机复测；需要稳定切换横竖屏时请按 **L**。

## 实现与目录结构

原生程序在上屏以 **400 × 240** 渲染，在下触控屏以 **320 × 240** 渲染。切换到书本模式后，界面改用 **240 × 400** 与 **240 × 320** 的逻辑布局，并同步转换触控坐标。硬件上仅下方屏幕支持触控输入。

图标、纹理与文字均按 3DS 原生像素尺寸预制，构建时将纹理图集打包进 RomFS。语言偏好与桌面图标布局分别独立保存。

```text
iphone-duo-3ds-native/       C 原生程序、预制纹理、交互模型与测试
iphone-duo-3ds-prototype/    网页交互原型与图片素材
docs/images/                 网页原型与模拟器运行截图
tools/                       素材校验、可选导入脚本与本地测试
asset-manifest.json          945 个预制资源的路径、大小与 SHA-256
```

<details>
<summary><strong>开发检查</strong></summary>

需要 Python 3.9+ 和 C11 编译器，运行本地主机测试无需安装 3DS 工具链：

```sh
python3 tools/check_assets.py
bash tools/test_host.sh
python3 -m unittest discover -s tests -p '*_test.py' -v
```

在支持相应特性的编译器上，可执行 `SANITIZE=1 bash tools/test_host.sh` 开启 AddressSanitizer 与 UndefinedBehaviorSanitizer 检查。CI 会自动校验素材完整性、可移植交互回归测试、导入安全性及 JavaScript 语法。

</details>

<details>
<summary><strong>素材校验与重新生成</strong></summary>

仓库已内置处理完毕的壁纸、苹果应用图标、静态应用演示画面、音乐小组件、锁屏视觉与原生纹理，常规克隆后即可直接预览和构建。

`asset-manifest.json` 记录了全部预制文件的字节大小与 SHA-256 哈希。若误改素材，可直接从 Git 恢复，或使用可选导入脚本从匹配的本地素材目录恢复：

```sh
python3 tools/import_assets.py \
  --native /path/to/prepared/iphone-duo-3ds-native \
  --web /path/to/prepared/iphone-duo-3ds-prototype
```

导入器会校验哈希、拒绝符号链接与未登记文件，并跳过主机存档；覆盖已有素材需显式传入 `--replace`，且覆盖前会自动备份。从原始素材重新生成纹理属于独立流程，详见 [ASSET_INPUTS.md](iphone-duo-3ds-native/ASSET_INPUTS.md)。仓库不打包苹果字体软件及全部原始中间截图。

</details>

## 当前状态与限制

- **原生中英文 v10（模拟器）：** 构建通过，语言设置持久化、按键旋转控制，以及跨屏音乐组件的添加与移除均已在 Azahar 中完成验证。
- **实体 3DS 主机：** 早期 v9.3 版本已拷入 SD 卡；中英文 v10 版本与自动旋转仍待实体机验证。
- **应用内页演示：** 已提供素材的应用使用静态参考画面展示内页；无对应演示图的应用保留在桌面上，截图内嵌文字保持原语言。
- **归档实验页面：** 归档的实验性 Music 与 App Store 页面不在默认预览路径中，且未做完整翻译。

## 来源与许可

本项目的原创代码采用 [MIT 许可证](LICENSE)。仓库附带的苹果参考图片、应用图标、截图、美术素材及第三方商标**不属于 MIT 授权范围**，相关权利归各自权利人所有。Noto Sans 字体保留其独立的 SIL Open Font License。

本项目为独立爱好者作品，与 Apple 或任天堂（Nintendo）不存在隶属、赞助或官方认可关系。详细来源说明与素材声明见 [THIRD_PARTY.md](THIRD_PARTY.md)。
