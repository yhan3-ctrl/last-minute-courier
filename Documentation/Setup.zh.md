# 本机开发环境

最后更新：2026-10-29。请以实际安装器显示和运行验证为准。

## 已确认

- MacBook Air M4，16 GB 内存。
- 开发工具路径原为 Command Line Tools；未发现完整 Xcode。
- 两轮以上经用户授权的清理后，约 55 GiB 可用；安装会改变剩余空间。
- 官方 EpicGamesLauncher.dmg 当前入口下载的 20.3.3 为 x86_64/arm64 通用版，已安装在个人 Applications。
- 系统 Gatekeeper 检查结果为 accepted / Notarized Developer ID。
- 启动器已完成约 405 MB 的自身更新，用户创建账号并登录。已观察到登录后的商城页，仍需进入引擎安装选项。

## 安装器纠正

早先 UnrealEngine.dmg 旧入口返回了 2015 年的 Intel 启动器。它没有运行，不能继续使用。当前安装来自 EpicGamesLauncher.dmg 入口。旧文件仅用于记录，后续可清理。

## 待完成

- [x] Epic 账号登录，由用户完成。
- [ ] 用户本人确认任何必要的许可条款。
- [ ] 从 Epic 官方版本兼容表和实际启动器核对 UE/Xcode 组合。
- [ ] 核对引擎、Xcode、下载临时空间及项目缓存的总容量；不能将 iCloud 视为额外运行磁盘。
- [ ] 完成安装并启动空项目。
- [ ] 生成并运行本地最小构建。
- [ ] 安排 Windows/Linux 构建与实际运行验证。

## 文件位置

设计、文案和后续源码：当前 Project3 文件夹，位于 iCloud 同步的桌面。

本机软件、活跃缓存：需要驻留本地。避免将生成缓存加入 Git；也不要依赖 iCloud 的同步状态代替版本历史。

用户约束：只操作 game development 文件夹。用户明确允许必要的软件运行数据写入默认位置；除此以外不得修改其他个人目录。主动生成的工程、素材、构建和缓存保留在 Project3。

## 当前操作阻碍

Epic 观察截图可用，原生鼠标点击反复返回 `noWindowsAvailable`。键盘 Tab 可移动商城中的焦点，但尚未可靠定位引擎栏目。已请用户点击“虚幻引擎→库”，先核对可安装版本、选项和大小，不直接确认大体积下载。

## 课件状态

工作区存在第 3、4 套课件文件名，但读取遇到超时/无效 ZIP，尚未获取内容。不要宣称已按课件完成结构核对。

## 2026-10-29：引擎安装 IS-0007

用户开始安装 UE 5.8.4，最小组件显示下载 15.867 GB、安装 43.721 GB。启动器随后显示“Install failed at 100%”。日志明确记录 `SaveJsonAtomic: could not open staging file`，对应 `Data/Manifests/Pending/*.item.tmp`；检查发现 Manifests 与 Pending 目录均不存在，父目录属于当前用户。磁盘约剩 55 GiB，引擎目录只有约 36 MB，此时不能声称已下载或安装完整引擎。

已在用户允许的 Epic 必要运行数据范围内补建 `Manifests/Pending`，未改动其他目录权限，未伪造清单。临时文件写入、fsync 与原子重命名检查通过，检查文件已移除。下一步通过启动器重试并核对新日志；只有成功后才能将此故障标为已解决。

## 2026-10-29：引擎安装成功，首次构建环境检查

用户截图显示引擎与 Fab/Bridge 插件 Installed。磁盘上确认 `UnrealEditor.app` 存在，`Build.version` 为 5.8.4 / CL 58902052。安装后本机剩余约 14 GiB。

运行 `Scripts/build_editor.command`：第一次因沙盒限制无法创建 UnrealBuildTool 的运行数据目录；在用户已允许的必要运行数据范围内重试后，构建工具正常启动，最终因 `Platform Mac is not a valid platform to build. SDK validation failed: Sdk: not found` 退出。尚未进入项目 C++ 编译，不能声称源码已编译或可运行。

系统的 developer directory 仍为 `/Library/Developer/CommandLineTools`，两个常见 Applications 位置均未发现完整 Xcode。安装包内 `Apple_SDK.json` 配置显示 MainVersion 26.1.1、MinVersion 15.2.0、MaxVersion 27.9.0；此配置可用于 SDK 版本门槛判断，但不代替编译和运行兼容性验证。

已查看 Apple 官方 Xcode 页面（当前 App Store 版本 27.1、标示大小 3.1 GB），这不代表完整安装后及可选组件的实际总占用。下一步是安装并配置完整 Xcode，仅选择桌面所需的平台与 Metal 工具，随后重新编译。

## 2026-10-29：Xcode 27.1 安装

用户完成 App Store 下载，确认 `/Applications/Xcode.app` 存在，`xcodebuild -version` 输出 27.1 / 27A9275。此时剩余约 11 GiB。系统默认 developer directory 尚未变更；构建脚本改为仅在当前进程使用完整 Xcode 的 `DEVELOPER_DIR`，不修改全局设置。

SDK/Metal 检查因尚未同意 Xcode 许可而被阻止。已打开 Xcode，显示 Xcode and Apple SDKs Agreement，等待用户本人决定是否同意。不能绕过许可确认，也不能将“下载完成”记为“首次配置完成”。

用户随后确认已同意，Xcode 进入 Welcome 页面。`xcrun --sdk macosx --show-sdk-path` 已返回 MacOSX27.0.sdk；Metal 的工具路径虽存在，但执行 `metal --version` 报告缺少 Metal Toolchain，因此通过 Apple 官方 `xcodebuild -downloadComponent MetalToolchain` 安装必要组件（下载显示约 838.9 MB）。

用户已取消两套 iOS 模拟器和离线开发文档下载。文档的“User cancelled download”状态不表示 Xcode 故障。

完整 SDK 可见后的首次构建暴露 V5 目标设置与 UE 5.8 预编译编辑器的共享配置不一致，已根据构建工具输出更新为 V7 / Unreal5_8 并将工程关联到 5.8。再次构建已识别 Mac SDK 27.0，后续结果待完成。

随后 Editor 目标构建成功，产出 `Binaries/Mac/libUnrealEditor-LastMinuteCourier.dylib`。Metal 组件下载完成，`xcrun metal --version` 输出 Apple metal 32023.921，可执行检查通过。

通过 Unreal 官方 Python commandlet 执行 `Scripts/bootstrap_map.py`，退出码 0、脚本成功、0 errors / 0 warnings，确认 `Content/Maps/L_Courier.umap` 已保存（约 9 KB）。正在启动带图形界面的独立游戏模式进行首次实际检查。

## 第一单视觉预览与七语言菜单（2026-10-29）

实现提交 `e1dd7f1`。Editor 模块编译成功；`Scripts/play_prototype.command` 可以启动第一单预览。`Scripts/build_visual_materials.py` 已生成项目材质，运行成功，0 errors / 0 warnings。

代理已从故事卡、练习、取件运行至第一单成功交付，并验证 R 回到取件点。途中碰撞触发检查点恢复后仍可继续。语言下拉菜单提供英文（默认）、简体中文、日语、韩语、西班牙语、法语和德语。已实际查看日/韩/西/法/德的开场，以及中文与德语的暂停帮助页；用键盘完成了下拉列表选日语的测试。

`python3 Scripts/validate_localization.py` 检查当前 46 条运行文字的六种附加翻译、字体和授权。此检查不替代母语审校、各语言完整试玩或镜头舒适度测试。场景中的英文店招目前保留原文。

预览还包含稳定第三人称镜头、80–100° FOV、鼠标灵敏度和可选方向锁定。尚未打包三平台，尚未完成真实玩家试玩、第二/三单与 5–10 分钟时长验证。
