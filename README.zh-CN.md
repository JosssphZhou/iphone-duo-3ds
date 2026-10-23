# iPhone Duo for Nintendo 3DS

[English](README.md)

把音乐图标拖向两屏接缝，它就变成另一块屏幕上的小组件。再次长按音乐，选择移除，组件会缩回图标当前位置。

这是一个原生 3DS 自制界面玩具，附带浏览器交互原型，参考 iPhone Duo 发布时的界面。它不运行 iOS，不执行 iPhone 应用，也不播放 Apple Music。

**本仓库只发布源码。** 本地演示使用的苹果图片、应用图标、截图和其他第三方视觉素材不包含在仓库中。直接克隆可以运行纯代码测试，但完整画面需要自行提供与清单匹配、已经处理好的本地素材。本仓库不提供完整素材包或一键下载器，详见 [素材与许可](THIRD_PARTY.md)。

## 交互

- 上屏 400 × 240，下触控屏 320 × 240。
- 长按抖动、拖动换位、两页桌面和 Dock。
- 音乐图标跨屏变成小组件，通过下屏长按菜单移除，并缩回图标当前位置。
- 横屏与竖屏书本布局，触控坐标跟随旋转。
- 从触控屏底部上滑解锁，图标文字按实际分辨率绘制。
- 中英文界面；真机初始语言参考系统设置，手动选择独立保存。网页也有语言切换入口。
- 有本地素材的应用显示静态演示图，未提供演示图的应用停留在桌面。

只有下方物理屏幕支持触控。竖屏模式下，把机身转到原上屏在左的位置，右边才是触控屏。

## 按键

| 操作 | 作用 |
| --- | --- |
| 点图标，或方向键选择后按 A | 打开已有的应用演示 |
| 长按图标后拖动 | 整理桌面 |
| 把音乐图标拖向两屏接缝 | 添加上屏音乐组件 |
| 已有组件时长按音乐 | 松手保留菜单，再点菜单移除组件 |
| B 或点菜单外 | 取消移除菜单 |
| B | 结束整理或返回桌面 |
| L | 切换横竖屏，并关闭自动旋转 |
| R | 显示传感器诊断 |
| X | 切换日夜壁纸 |
| Y | 切换桌面分页 |
| 按住 SELECT 后点 Y | 切换中英文 |
| 单独按下再松开 SELECT | 切换锁屏状态 |
| 从触控屏底部向上滑动 | 解锁 |
| START | 退出到 Homebrew Launcher |

HOME 仍由 3DS 系统处理。重力感应仍待实体机验证，需要稳定演示时使用 L 切换。

## 本地测试

需要 Python 3.9 及以上版本和 C11 编译器，无需苹果素材或 3DS 工具链：

```sh
bash tools/test_host.sh
python3 -m unittest discover -s tests -p '*_test.py' -v
```

支持的编译器可用 `SANITIZE=1 bash tools/test_host.sh` 开启内存和未定义行为检查。CI 也会检查网页 JavaScript 语法；这些检查不等于实体机测试。

## 导入素材

先自行准备与 `asset-manifest.json` 匹配的原生 `gfx/` 和网页 `assets/`。原始截图不能直接当作处理好的素材包；清单只记录文件、大小和 SHA-256，不授予素材许可。

```sh
python3 tools/import_assets.py \
  --native /path/to/prepared/iphone-duo-3ds-native \
  --web /path/to/prepared/iphone-duo-3ds-prototype
python3 tools/check_assets.py
```

可单独导入原生或网页素材，检查工具会报告剩余缺失文件。导入不会修改源目录，也不会复制游戏机存档。目标文件内容不同时默认停止；加 `--replace` 才会先备份到 `.asset-backups/` 再替换。路径越界、软链接和清单之外的文件会被拒绝。导入后的素材及备份由 Git 忽略。

素材处理脚本保留了转换过程，但原始图片、中间截图和苹果字体未随仓库分发。重新生成需要自行提供相应输入，请先读脚本的 `--help` 和源码。

## 构建与启动

原生素材齐全后，安装 devkitPro 3DS 工具链，包括 devkitARM、libctru、citro2d、citro3d、3dstools 和 tex3ds：

```sh
cd iphone-duo-3ds-native
./build.sh -j4
```

把生成的 `iphone-duo.3dsx` 和 `iphone-duo.smdh` 复制到 SD 卡的 `/3ds/iphone-duo/`，通过 Homebrew Launcher 启动。游戏机需已有可用的自制程序入口，本项目不安装或修改主机固件。更新时保留 `layout.bin` 和 `language.bin`。

构建会把本地图片图集嵌入程序。包含第三方素材的二进制不因代码采用 MIT 而自动获得再分发许可。

## 网页原型

导入网页素材后，在仓库根目录运行：

```sh
python3 -m http.server 8000
```

打开 `http://localhost:8000/iphone-duo-3ds-prototype/`，通过语言入口切换中英文。网页用于预演交互，不能证明实体机性能。

## 当前状态与许可

本地 v9.3 已写入开发者的 SD 卡，组件交互已在 3DS 模拟器中走通。后续中英文版本尚未完成实体机验证。应用演示图里的文字保留原样；整个项目不接入通话、账号、下载或苹果服务。

原创代码使用 [MIT](LICENSE)，Noto Sans 保留独立的 SIL Open Font License。苹果、任天堂名称、标志和第三方素材不在 MIT 授权范围内。本项目与苹果或任天堂没有隶属关系，详见 [THIRD_PARTY.md](THIRD_PARTY.md)。
