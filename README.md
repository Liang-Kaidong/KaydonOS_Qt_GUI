# KaydonOS

基于 **Qt 5.12.9** 开发的嵌入式车载 HMI（Human Machine Interface）桌面系统，专属适配 **正点原子 IMX6ULL-ALPHA** 开发板，复刻智能车载终端的完整交互逻辑，集成多媒体播放器、硬件传感器、系统工具套件，适用于 **嵌入式 Linux 学习、课程设计、毕业设计与功能扩展开发**。

![KaydonOS Preview](Pics/mainWindow.png)

---

## 📌 项目简介

KaydonOS 是一套轻量化嵌入式车载人机交互解决方案，采用 **C++ + Qt Widgets** 开发，针对 **NXP i.MX6ULL Cortex-A7** 低性能 ARM 平台进行了资源优化。

系统实现了车载中控常见功能：

* 多分页桌面与触摸滑动切换
* 顶部状态栏与底部导航栏
* 音乐 / 视频播放
* 相机拍照与图片相册
* 录音与音频播放
* DHT11 温湿度采集
* ALS 距离感应
* 蜂鸣器控制
* CPU / 内存监控
* 系统配置持久化

项目采用 **模块化分层架构**，桌面主程序、应用模块、底层驱动相互解耦，便于二次开发与硬件移植。

---

# ✨ 核心特性

| 特性     | 说明                      |
| ------ | ----------------------- |
| 硬件专属适配 | 原生适配 正点原子 IMX6ULL-ALPHA |
| 完整车载交互 | 分页桌面、状态栏、导航栏、弹窗系统       |
| 多媒体功能  | 音乐、视频、相机、录音、相册          |
| 传感器支持  | DHT11、ALS、蜂鸣器、V4L2 摄像头  |
| 触摸优化   | 基于 tslib 的校准与滤波         |
| 配置持久化  | 用户配置重启不丢失               |
| 资源优化   | 适配 512MB 内存平台           |

---

# 🧱 系统架构

```text
+--------------------------------------------------+
|                    KaydonOS                      |
+--------------------------------------------------+
|                 Qt Desktop Layer                 |
|  MainWindow / Gesture / StatusBar / Navigation   |
+--------------------------------------------------+
|                Application Layer                 |
| Music | Video | Camera | Gallery | Recorder      |
| Clock | Calendar | Weather | Monitor             |
+--------------------------------------------------+
|                 Driver Wrapper Layer             |
| DHT11 | ALS | Beep | V4L2 Camera                 |
+--------------------------------------------------+
|                  Linux Kernel                    |
| framebuffer | input | video4linux | sysfs        |
+--------------------------------------------------+
|                 IMX6ULL Hardware                 |
+--------------------------------------------------+
```

---

# 📦 功能模块

## 桌面系统

* 多分页应用桌面
* 左右滑动切换
* 顶部状态栏
* 底部导航栏
* 模态弹窗管理
* 用户配置持久化

## 多媒体应用

| 应用    | 功能                 |
| ----- | ------------------ |
| 音乐播放器 | MP3 播放、歌单、循环模式     |
| 视频播放器 | AVI/MP4 播放、全屏、进度拖拽 |
| 相机    | V4L2 实时预览与拍照       |
| 录音机   | WAV 录制与管理          |
| 图片相册  | 缩略图浏览与删除           |

## 系统工具

* 计算器
* 日历
* 数字时钟
* 天气面板
* CPU / 内存监控
* 亮度与音量调节

## 硬件驱动

| 驱动          | 功能    |
| ----------- | ----- |
| DHT11       | 温湿度采集 |
| ALS         | 距离检测  |
| Beep        | 蜂鸣器控制 |
| V4L2 Camera | 摄像头采集 |

---

# 🛠️ 开发环境

| 组件           | 版本               |
| ------------ | ---------------- |
| Qt           | 5.12.9           |
| C++          | C++11            |
| qmake        | Qt 自带            |
| tslib        | 1.21             |
| Toolchain    | gcc-linaro-4.9.4 |
| Linux Kernel | 4.1.15           |
| Qt Platform  | linuxfb          |

---

# 💻 目标硬件

* **开发板**：正点原子 IMX6ULL-ALPHA
* **CPU**：NXP i.MX6ULL Cortex-A7
* **内存**：512MB DDR3
* **存储**：8GB eMMC
* **屏幕**：7 寸 1024×600 电容触摸屏

---

# 🚀 编译与部署

## 1. 克隆仓库

```bash
git clone https://github.com/Liang-Kaidong/KaydonOS_Qt_GUI.git
cd KaydonOS_Qt_GUI
```

## 2. 创建构建目录

```bash
mkdir build
cd build
```

## 3. 生成 Makefile

```bash
/path/to/arm-qt/bin/qmake ../KaydonOS.pro
```

## 4. 编译

```bash
make -j$(nproc)
```

## 5. 部署到开发板

```bash
scp KaydonOS root@192.168.1.xxx:/home/root/
scp -r Audio Config Icons Pics Music Video root@192.168.1.xxx:/home/root/KaydonOS/
```

## 6. 运行

```bash
cd /home/root
./KaydonOS
```

---

# 📁 目录结构

```text
KaydonOS_Qt_GUI/
├── Audio/
├── Config/
├── Driver/
│   ├── DHT11/
│   ├── als/
│   ├── beep/
│   └── v4l2camera/
├── Icons/
├── Pics/
├── Music/
├── Video/
├── Caculator/
├── Calendar/
├── Camera/
├── Clock/
├── Gallery/
├── Monitor/
├── MusicPlayer/
├── Recorder/
├── SystemSetting/
├── VideoPlayer/
├── Weather/
├── gesture.cpp
├── main.cpp
├── mainwindow.cpp
└── KaydonOS.pro
```

---

# ⚠️ 硬件兼容性说明

> 本项目默认仅验证 **正点原子 IMX6ULL-ALPHA** 开发板。

移植到其他平台时需要重新适配：

* GPIO 引脚
* 摄像头驱动
* LCD 分辨率
* 设备树节点
* 触摸参数

---

# 📄 开源协议（重要）

## 源代码许可

本项目 **原创源代码部分** 采用 **MIT License** 开源。

* 允许学习、修改、分发与商业使用
* 必须保留原作者版权声明与许可证文本

完整协议见仓库根目录 `LICENSE` 文件。

## 第三方素材说明（不属于 MIT）

仓库中的 **图标、壁纸、界面图片、音频等素材** 可能来源于第三方公开资源，其版权归原作者或版权方所有。

这些素材：

* **不包含在 MIT License 授权范围内**
* 仅用于项目界面演示与学习研究
* 如需商业使用，请自行确认授权

如果您是相关素材的版权方，并认为仓库内容存在问题，请通过邮箱联系，我会及时处理。

---

# 📚 第三方资源

| 资源              | 说明                             |
| --------------- | ------------------------------ |
| Qt              | https://www.qt.io              |
| tslib           | https://github.com/libts/tslib |
| 正点原子 IMX6ULL 资料 | https://www.openedv.com        |
| Linux V4L2      | Linux 官方接口                     |

---

# 📬 联系方式

如需：

* 技术交流
* 二次开发合作
* 商业授权咨询
* 获取最新适配说明

请联系：

**Email：226429965@qq.com**

---

# ⚖️ 法律声明（Legal Notice）

本项目按 **“现状（AS IS）”** 提供，不提供任何形式的明示或暗示担保，包括但不限于：

* 适销性
* 特定用途适用性
* 不侵权保证

作者不对因使用本项目而导致的任何直接或间接损失承担责任，包括但不限于：

* 数据丢失
* 设备损坏
* 业务中断
* 第三方版权纠纷

使用者应自行评估并承担相关风险。

---

# 🐞 已知问题

* Windows 端拖动桌面可能出现果冻效应（开发板正常）
* Windows 版本仅用于界面预览，部分硬件功能无法完整演示
* 仍有部分功能处于开发或调试阶段
* 欢迎提交 Issue 反馈问题

---

# 🤝 贡献指南

## 提交流程

```bash
git checkout -b feature/your-feature
git commit -m "feat: add new feature"
git push origin feature/your-feature
```

然后提交 Pull Request。

## 代码规范

* 4 空格缩进
* 类名：PascalCase
* 函数 / 变量：camelCase
* 遵循 Qt 官方编码风格

---

# 🙏 致谢

感谢以下项目与组织：

* Qt 官方
* tslib 开源社区
* 正点原子（硬件资料与开发板）
* Linux Kernel 社区
* 所有为嵌入式开源生态做出贡献的开发者

---

<div align="center">

**KaydonOS**

Embedded Qt HMI for IMX6ULL

Author: **Kaydon**

Last Updated: **2026-07-22**

⭐ 如果这个项目对你有帮助，欢迎点一个 Star！

</div>
