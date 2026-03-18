# 三国杀·天意侵蚀 🎮

<div align="center">

![Qt](https://img.shields.io/badge/Qt-5.14.2-green.svg)
![C++](https://img.shields.io/badge/C++-11-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

**一款基于Qt框架开发的三国题材卡牌对战游戏**

[功能特性](#功能特性) • [快速开始](#快速开始) • [游戏玩法](#游戏玩法) • [武将介绍](#武将介绍) • [技术架构](#技术架构)

</div>

---

## 📖 项目简介

《三国杀·天意侵蚀》是一款经典三国杀玩法的桌面卡牌游戏，采用三方对弈模式。玩家将扮演一名"邪化"的三国武将，与两个AI对手展开激烈对决。游戏融合了策略卡牌、武将技能、华丽特效等元素，带来沉浸式的游戏体验。

![游戏截图](img/screenshot.png)

## ✨ 功能特性

- 🎯 **三方对弈** - 1名玩家 vs 2个AI对手，最后存活者获胜
- 🦸 **三大武将** - 曹操、刘备、司马懿，各具独特技能
- 🃏 **丰富卡牌** - 杀、闪、桃、锦囊等多种卡牌类型
- 🎬 **华丽特效** - 卡牌飞行、伤害数字、粒子效果、技能视频
- 🤖 **智能AI** - 具备策略决策能力的AI对手
- 🎨 **精美UI** - 渐变背景、发光边框、动态高亮

## 🚀 快速开始

### 环境要求

- **Qt**: 5.14.2 或更高版本
- **编译器**: MinGW 7.3.0 64-bit (或 MSVC 2017+)
- **操作系统**: Windows 7/10/11

### 编译步骤

```bash
# 1. 克隆仓库
git clone https://github.com/yourusername/NewThreeKingdomsKill.git
cd NewThreeKingdomsKill

# 2. 使用Qt Creator打开项目
# 双击 NewThreeKingdomsKill.pro

# 3. 选择编译套件 (Desktop Qt 5.14.2 MinGW 64-bit)

# 4. 构建并运行
```

### 命令行编译

```bash
# 生成Makefile
qmake NewThreeKingdomsKill.pro

# 编译
mingw32-make

# 运行
debug\ThreeKingdomsKill.exe
```

## 🎮 游戏玩法

### 基本规则

1. **选择武将** - 游戏开始时选择你的武将（曹操/刘备/司马懿）
2. **回合流程** - 每回合摸2张牌，可出牌、使用技能，然后结束回合
3. **出牌限制** - 每回合只能出1张【杀】（司马懿除外）
4. **胜利条件** - 成为最后存活的玩家

### 卡牌类型

| 卡牌 | 说明 |
|------|------|
| **杀** 🔴 | 对目标造成1点伤害，对方可用【闪】抵消 |
| **闪** 🔵 | 抵消【杀】造成的伤害 |
| **桃** 🩷 | 恢复1点体力（不能超过上限） |
| **顺手牵羊** 🟣 | 获得目标1张手牌 |
| **过河拆桥** 🟣 | 弃置目标1张手牌 |
| **跳跳虎脆脆鲨** 🟣 | 全体玩家恢复1点体力 |
| **天意侵袭** 🟣 | 全体玩家必须出【杀】，否则受伤 |

## 🦸 武将介绍

### 曹操 (4血)

| 技能 | 类型 | 效果 |
|------|------|------|
| **战无不胜の斗鸡眼** | 被动 | 血量≤50%时，对全体敌方造成1点精神伤害 |
| **呱** | 被动 | 回合未出牌时，对一名玩家造成1点精神伤害 |
| **汪** | 主动 | 与目标轮流出【杀】，无杀者受伤 |
| **临终关怀** | 被动 | 死亡时对全体玩家造成1点精神伤害 |

### 刘备 (4血)

| 技能 | 类型 | 效果 |
|------|------|------|
| **自刎归天** | 被动 | 出【杀】时，全员受到1点伤害 |
| **无敌の二弟** | 被动 | 受伤后可召唤二弟反击 |
| **仁之剑义之剑** | 主动 | 选择两人：一个受伤，一个回血 |

### 司马懿 (3血)

| 技能 | 类型 | 效果 |
|------|------|------|
| **天意化骨掌** | 被动 | 可无限出【杀】 |
| **天意面瘫** | 被动 | 出【桃】时，自身恢复2点，其他人恢复1点 |

## 🏗️ 技术架构

### 项目结构

```
NewThreeKingdomsKill/
├── main.cpp              # 程序入口
├── mainwindow.h/cpp      # 主窗口与UI
├── gameengine.h/cpp      # 游戏逻辑引擎
├── Hero.h/cpp            # 武将与技能系统
├── Player.h/cpp          # 玩家状态管理
├── Card.h/cpp            # 卡牌系统
├── img/                  # 武将头像资源
└── video/                # 技能特效视频
```

### 核心类关系

```
┌─────────────┐
│ MainWindow  │  UI层：界面显示、动画、用户交互
└──────┬──────┘
       │ Signals/Slots
       ▼
┌─────────────┐
│ GameEngine  │  逻辑层：回合管理、卡牌处理、AI决策
└──────┬──────┘
       │ 调用
       ▼
┌─────────────┐
│Player/Hero/ │  数据层：状态存储、技能实现
│   Card      │
└─────────────┘
```

### 技术特点

- **Qt信号槽机制** - 松耦合的事件驱动架构
- **自定义绘制** - QPainter实现精美UI
- **动画系统** - 粒子效果、卡牌飞行、浮动数字
- **视频播放** - QMediaPlayer播放技能特效
- **面向对象设计** - 继承、多态、策略模式

## 📋 依赖项

- Qt Core
- Qt GUI
- Qt Widgets
- Qt Multimedia
- Qt MultimediaWidgets

## 🤝 贡献指南

欢迎提交Issue和Pull Request！

1. Fork本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启Pull Request

## 📝 开发计划

- [ ] 添加更多武将
- [ ] 网络对战功能
- [ ] 音效系统
- [ ] 成就系统
- [ ] 存档功能

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 🙏 致谢

- 灵感来源：经典桌游《三国杀》
- 开发框架：Qt Framework
- 图标设计：项目自制

---

<div align="center">

**如果这个项目对你有帮助，请给一个 ⭐ Star！**

Made with ❤️ and Qt

</div>
