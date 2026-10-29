# 第一单原型：源码已准备，尚未编译

## 当前范围

原生 Unreal C++ 工程，使用引擎内置几何体。源码包含：

- 三张开场故事卡；Enter 继续，Backspace 返回，按钮支持鼠标；L 切换中英文。
- 第三人称移动、鼠标视角、空格跳跃。
- 无倒计时练习区，按位置和操作进展逐条显示提示。
- 按住 Q 减慢蓝色运输车，最长 3 秒，结束后冷却 4 秒。
- E 领取第一单后开始 140 秒倒计时，到咖啡摊按 E 交付。
- 途中检查点，碰撞/越界恢复；R 重试本单，Esc 暂停，H 查看故事与操作。
- 送达后咖啡摊点亮，并显示第一单原型完成。

这只是源码预备阶段。没有 Unreal 编译/运行成功记录，以上功能不应被当作已验证成品。第一条直线路线用于检验流程和输入，不是最终关卡，也不能用来证明 5–10 分钟要求。

## 明确尚未完成

- UE/Xcode 安装、真实编译和运行验证。
- 镜头和胶片订单、移动平台、新用途谜题、最终电影票结局。
- 最终关卡布局和时长、正式美术与音频。
- 中文字库覆盖、文本换行、窗口缩放及暂停时鼠标按钮的视觉/交互验证。
- 设计中的 0.5 秒淡入淡出恢复、屏幕边缘目的地箭头、路线提示。
- 运行数据：目前只有开场卡从 JSON 读取；其他规则与教程在第一单源码中有初始常量，后续需统一配置。
- 三平台构建、独立启动和真实试玩。

## 工程文件

`LastMinuteCourier.uproject` 不绑定尚未确认的引擎版本。源码目标使用 BuildSettingsVersion.V5，应选择支持此设置的 UE 5.4 或更新版本，再核对当前 macOS 与 Xcode 兼容性。

工程中尚无二进制 `.umap`。安装完成、模块编译成功后，在编辑器内执行 `Scripts/bootstrap_map.py`，创建并保存地图。脚本遇到已有地图时保留它，不覆盖内容。

可以在 Terminal 用 `Scripts/build_editor.command /引擎安装根目录` 调用原生 Unreal 构建工具；编译日志与我们指定的派生缓存写入本项目。不能在缺少完整 Xcode 或不兼容版本的情况下宣称构建完成。

## 文件范围

用户要求只操作 `game development` 文件夹。新工程、素材、构建和主动生成的缓存都应留在本 Project3 中。

用户随后明确允许 Epic、Unreal 与开发工具必要的软件运行数据写入默认位置。此例外不授权修改其他个人文件、清理其他软件或移动旧项目。

## 后续一次完整检查

1. 编译模块，修正 UHT、C++ 和版本兼容错误。
2. 创建地图并按 Play，检查三张卡能正常读完、返回与切换语言。
3. 验证练习提示不会跳错阶段；失败后仍可完成教学。
4. 验证慢动作、充能、订单计时和暂停独立且一致。
5. 领取、送达、超时和重试各完成一次；确认无软锁。
6. 检查字体、颜色、灯光、镜头、人物比例和交互提示，再让真实玩家试用。

官方 API 参考：
- https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/AHUD
- https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/FInputBinding
- https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/LevelEditorSubsystem
