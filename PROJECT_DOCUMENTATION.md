# 三国杀·天意侵蚀 - 项目技术文档

## 目录

1. [项目概述](#1-项目概述)
2. [技术架构](#2-技术架构)
3. [核心类设计](#3-核心类设计)
4. [游戏流程详解](#4-游戏流程详解)
5. [武将技能系统](#5-武将技能系统)
6. [卡牌系统](#6-卡牌系统)
7. [UI与动画系统](#7-ui与动画系统)
8. [信号槽机制详解](#8-信号槽机制详解)
9. [AI系统](#9-ai系统)
10. [常见问题解答](#10-常见问题解答)

---

## 1. 项目概述

### 1.1 项目简介

**三国杀·天意侵蚀**是一款基于Qt框架开发的桌面卡牌对战游戏，灵感来源于经典桌游《三国杀》。游戏采用三方对弈模式，玩家选择一名武将，与两个AI对手进行战斗，最后存活的玩家获胜。

### 1.2 技术栈

| 技术 | 版本/说明 |
|------|----------|
| Qt Framework | 5.14.2 |
| 编译器 | MinGW 7.3.0 64-bit |
| C++标准 | C++11 |
| 构建系统 | qmake |
| 多媒体 | Qt Multimedia / Qt MultimediaWidgets |

### 1.3 项目文件结构

```
NewThreeKingdomsKill/
├── main.cpp              # 程序入口，全局样式设置
├── mainwindow.h/cpp      # 主窗口，UI逻辑，动画系统
├── gameengine.h/cpp      # 游戏引擎，核心逻辑
├── Hero.h/cpp            # 武将类及技能实现
├── Player.h/cpp          # 玩家类，手牌管理
├── Card.h/cpp            # 卡牌类系统
├── mainwindow.ui         # Qt Designer界面文件
├── NewThreeKingdomsKill.pro  # Qt项目配置文件
├── img/                  # 武将头像资源
│   ├── caocao.gif
│   ├── liubei.png
│   └── simayi.png
└── video/                # 技能特效视频
    ├── caocao_doujiyan.mp4
    ├── caocao_gua.mp4
    ├── caocao_linzhong.mp4
    ├── caocao_wang.mp4
    ├── liubei_erdi.mp4
    ├── liubei_jian.mp4
    ├── liubei_ziwen.mp4
    └── simayi_huagu.mp4
```

---

## 2. 技术架构

### 2.1 架构设计图

```
┌─────────────────────────────────────────────────────────────┐
│                        MainWindow                            │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ 角色选择界面 │  │  游戏界面   │  │    动画/特效系统    │  │
│  │ (Character  │  │ (GamePage)  │  │ (AnimOverlay/       │  │
│  │ SelectPage) │  │             │  │  CenterEffectWidget)│  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└───────────────────────────┬─────────────────────────────────┘
                            │ Signals/Slots
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                       GameEngine                             │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ 回合管理    │  │ 卡牌处理    │  │    AI行为决策       │  │
│  │ (Turn       │  │ (Card       │  │    (AI Logic)       │  │
│  │ Management) │  │ Processing) │  │                     │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└───────────────────────────┬─────────────────────────────────┘
                            │ 调用
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                    Player / Hero / Card                      │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ Player      │  │ Hero        │  │    Card             │  │
│  │ (玩家状态)  │  │ (武将技能)  │  │    (卡牌效果)       │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 设计模式应用

| 设计模式 | 应用场景 | 说明 |
|---------|---------|------|
| **观察者模式** | 信号槽机制 | GameEngine状态变化通知MainWindow更新UI |
| **策略模式** | 武将技能 | 不同武将实现不同的技能策略 |
| **模板方法模式** | Hero基类 | 定义技能钩子方法，子类重写实现 |
| **工厂模式** | createRandomCard() | 随机生成不同类型卡牌 |
| **单例思想** | GameEngine | 每局游戏只有一个引擎实例 |

---

## 3. 核心类设计

### 3.1 GameEngine - 游戏引擎

**职责**：管理游戏状态、回合流转、卡牌使用、AI决策

```cpp
class GameEngine : public QObject {
    Q_OBJECT
public:
    // 游戏控制
    void startGame(Player* p0, Player* p1, Player* p2);
    void endGame();
    
    // 玩家操作
    void playerUseCard(int cardIndex, int targetPlayerIndex);
    void playerUseSkill(int skillIndex, int targetPlayerIndex);
    void playerEndTurn();
    void playerRespondDodge(bool useDodge);
    
    // 状态查询
    bool isHumanTurn() const;
    Player* currentPlayer() const;
    int currentPlayerIndex() const;
    
signals:
    void logMessage(const QString& msg);           // 日志消息
    void gameStateChanged();                        // 状态变化
    void gameOver(const QString& winnerName);       // 游戏结束
    void animationRequest(...);                     // 动画请求
    void requestDodgeResponse(...);                 // 请求闪避响应
    
private:
    QVector<Player*> m_players;    // 玩家列表
    int m_curIdx;                  // 当前回合玩家索引
    bool m_running;                // 游戏运行状态
    QTimer* m_aiTimer;             // AI行动定时器
};
```

**关键成员变量说明**：

| 变量名 | 类型 | 用途 |
|-------|------|------|
| `m_players` | `QVector<Player*>` | 存储3个玩家对象指针 |
| `m_curIdx` | `int` | 当前行动玩家索引(0=人类玩家) |
| `m_running` | `bool` | 游戏是否正在进行 |
| `m_aiTimer` | `QTimer*` | 控制AI行动延迟 |
| `m_waitingForDodgeResponse` | `bool` | 等待玩家出闪 |
| `m_pendingKillFrom/To` | `Player*` | 待处理的杀牌来源/目标 |

### 3.2 Player - 玩家类

**职责**：管理玩家状态、手牌、体力值

```cpp
class Player {
public:
    // 基础属性
    QString getName() const;
    int getHealth() const;
    int getMaxHealth() const;
    bool isAlive() const;
    
    // 武将关联
    Hero* getHero() const;
    void setHero(Hero* h);
    
    // 手牌管理
    const QList<Card*>& getHandCards() const;
    void addCard(Card* card);
    bool removeCardByIndex(int index, bool deleteCard = true);
    int findDodgeCard() const;  // 查找闪牌
    int findKillCard() const;   // 查找杀牌
    
    // 体力操作
    void receiveDamage(int amount);
    void restoreHealth(int amount);
    void drawCards(int count);  // 摸牌
    
    // 回合状态
    bool hasUsedKillThisTurn() const;
    bool hasUsedSkillThisTurn() const;
    void resetTurnState();      // 重置回合状态

private:
    QString m_name;
    int m_hp, m_maxHp;
    QList<Card*> m_hand;        // 手牌列表
    Hero* m_hero;               // 武将对象
    bool m_killUsed;            // 本回合是否出过杀
    bool m_skillUsed;           // 本回合是否用过技能
};
```

### 3.3 Hero - 武将基类

**职责**：定义武将技能接口，提供技能触发钩子

```cpp
class Hero {
public:
    Hero(const QString& heroName, int maxHp);
    virtual ~Hero() = default;
    
    // 纯虚函数 - 子类必须实现
    virtual bool activateSkill(int skillIndex, Player* self, 
                               Player* target, GameEngine* engine) = 0;
    virtual int getSkillCount() const = 0;
    virtual SkillInfo getSkillInfo(int index) const = 0;
    
    // 虚函数 - 生命周期钩子（默认空实现）
    virtual void onTurnStart(Player* self, GameEngine* engine);
    virtual void onTurnEnd(Player* self, GameEngine* engine);
    virtual void onCardPlayed(Player* self, const QString& cardName, 
                              Player* target, GameEngine* engine);
    virtual void onDamaged(Player* self, int dmg, GameEngine* engine);
    virtual void onDeath(Player* self, GameEngine* engine);
    
protected:
    QString m_name;
    int m_maxHp;
};
```

**技能类型枚举**：

```cpp
enum class SkillType {
    Active,   // 主动技能 - 玩家主动点击发动
    Passive   // 被动技能 - 条件触发自动发动
};

struct SkillInfo {
    QString name;        // 技能名称
    QString desc;        // 技能描述
    SkillType type;      // 技能类型
    bool isActive() const { return type == SkillType::Active; }
};
```

### 3.4 Card - 卡牌基类

**职责**：定义卡牌类型和使用效果

```cpp
enum class CardType {
    Kill,    // 杀 - 攻击卡
    Dodge,   // 闪 - 防御卡
    Peach,   // 桃 - 回复卡
    Trick    // 锦囊 - 特殊效果卡
};

class Card {
public:
    Card(const QString& n, const QString& d, CardType t);
    virtual ~Card() = default;
    
    // 卡牌使用
    virtual bool play(Player* from, Player* to, GameEngine* engine);
    
    // 卡牌属性
    virtual bool canRespondWithDodge() const { return false; }
    virtual bool requiresTarget() const { return true; }
    
    QString getName() const;
    QString getDescription() const;
    CardType getType() const;

protected:
    QString name, description;
    CardType type;
};
```

**卡牌子类**：

| 类名 | 类型 | 效果 |
|-----|------|------|
| `KillCard` | Kill | 对目标造成1点伤害，可被闪抵消 |
| `DodgeCard` | Dodge | 抵消杀的伤害 |
| `PeachCard` | Peach | 恢复1点体力 |
| `TrickCard` | Trick | 锦囊牌，有4种子类型 |

**锦囊牌子类型**：

```cpp
enum TrickSubType { 
    StealCard,        // 顺手牵羊 - 获得目标1张手牌
    DiscardCard,      // 过河拆桥 - 弃置目标1张手牌
    JumpTigerShark,   // 一对跳跳虎两条脆脆鲨 - 全体回复1血
    TianyiInvasion    // 天意侵袭 - 全体必须出杀否则受伤
};
```

---

## 4. 游戏流程详解

### 4.1 游戏启动流程

```
main()
  │
  ├── QApplication初始化
  ├── 设置全局样式表
  ├── 创建MainWindow
  │     │
  │     └── buildUI()
  │           ├── buildCharacterSelectUI()  // 角色选择界面
  │           └── buildGameUI()             // 游戏界面
  │
  └── w.show() → 进入事件循环
```

### 4.2 选择角色流程

```
用户点击武将卡片
  │
  ├── eventFilter捕获鼠标事件
  │     └── 获取heroName属性
  │
  ├── onCharacterSelected(heroName)
  │     │
  │     ├── 切换到游戏页面
  │     └── startGameWithHero(heroName)
  │           │
  │           ├── 创建3个Player对象
  │           ├── 分配Hero对象
  │           ├── 设置HeroPanel头像
  │           ├── 创建GameEngine
  │           ├── 连接所有信号槽
  │           └── engine->startGame(p0, p1, p2)
  │
  └── 游戏开始
```

### 4.3 回合流程

```
startTurn()
  │
  ├── 检查玩家存活
  ├── resetTurnState()      // 重置出杀/技能使用状态
  ├── drawCards(2)          // 摸2张牌
  ├── triggerHeroTurnStart() // 触发回合开始技能
  ├── 发送动画请求
  ├── 发送日志消息
  │
  └── 判断是否人类玩家
        │
        ├── 是 → 等待玩家操作
        │
        └── 否 → m_aiTimer->start(1500)  // 1.5秒后AI行动
                    │
                    └── aiTakeTurn() → performAiAction()
```

### 4.4 出牌流程

```
玩家点击手牌
  │
  ├── onCardClicked(index)
  │     │
  │     ├── 判断卡牌类型
  │     │     ├── Peach → 直接使用，目标为自己
  │     │     ├── Dodge → 提示不能主动使用
  │     │     └── Kill/Trick → 进入目标选择模式
  │     │
  │     └── enterTargetSelectionMode(callback)
  │
  ├── 玩家点击目标HeroPanel
  │     │
  │     └── onTargetSelected(playerIndex)
  │           │
  │           └── callback(playerIndex)
  │                 │
  │                 └── engine->playerUseCard(cardIndex, targetIndex)
  │                       │
  │                       ├── 验证卡牌使用条件
  │                       ├── card->play(from, to, engine)
  │                       │     │
  │                       │     └── 执行卡牌效果
  │                       │
  │                       ├── 发送动画请求
  │                       ├── 移除手牌
  │                       ├── 触发英雄技能钩子
  │                       ├── 检查游戏结束
  │                       └── 更新UI
  │
  └── 回到等待状态
```

### 4.5 杀牌特殊处理流程

```
processKillCard(from, to)
  │
  ├── 判断目标是否为人类玩家
  │     │
  │     ├── 是 → 检查是否有闪
  │     │     │
  │     │     ├── 有闪 → 请求玩家选择
  │     │     │     │
  │     │     │     ├── emit requestDodgeResponse()
  │     │     │     ├── 启动5秒超时定时器
  │     │     │     └── 等待玩家响应
  │     │     │           │
  │     │     │           ├── 使用闪 → 抵消伤害
  │     │     │           └── 不使用/超时 → 受到伤害
  │     │     │
  │     │     └── 无闪 → 直接受伤
  │     │
  │     └── 否(AI目标) → AI自动判断是否出闪
  │           │
  │           ├── 有闪 → 自动使用，抵消伤害
  │           └── 无闪 → 受到伤害
  │
  └── 触发受伤/死亡钩子
```

---

## 5. 武将技能系统

### 5.1 曹操（4血4技能）

| 技能名 | 类型 | 触发时机 | 效果 |
|-------|------|---------|------|
| 战无不胜の斗鸡眼 | 被动 | 回合开始 | 血量≤50%时，对全体敌方造成1点精神伤害 |
| 呱 | 被动 | 回合结束 | 本回合未出牌时，对一名玩家造成1点精神伤害 |
| 汪 | 主动 | 玩家点击 | 指定玩家轮流出杀，无杀者受伤 |
| 临终关怀 | 被动 | 死亡时 | 对全体玩家造成1点精神伤害 |

**代码实现要点**：

```cpp
// 战无不胜の斗鸡眼 - 回合开始触发
void CaoCao::onTurnStart(Player* self, GameEngine* engine) {
    int hp = self->getHealth();
    int maxHp = self->getMaxHealth();
    
    if (hp * 2 <= maxHp && !m_doujiyanTriggered) {
        m_doujiyanTriggered = true;
        // 对全体敌方造成伤害
        for (int i = 0; i < engine->playerCount(); ++i) {
            Player* p = engine->playerAt(i);
            if (p && p != self && p->isAlive()) {
                p->receiveDamage(1);
            }
        }
    }
}

// 呱 - 回合结束触发，需要玩家选择目标
void CaoCao::onTurnEnd(Player* self, GameEngine* engine) {
    if (!self->hasPlayedCardThisTurn() && self->isAlive()) {
        // 发出信号请求玩家选择目标
        engine->prepareTargetSelectionForSkill(1, self->getHero()->getName());
    }
}
```

### 5.2 刘备（4血3技能）

| 技能名 | 类型 | 触发时机 | 效果 |
|-------|------|---------|------|
| 自刎归天 | 被动 | 出杀时 | 全员（包括自己）受到1点伤害 |
| 无敌の二弟 | 被动 | 受伤后 | 可召唤二弟对攻击者造成1点伤害 |
| 仁之剑义之剑 | 主动 | 玩家点击 | 选择两个目标：一个受伤，一个回血 |

**代码实现要点**：

```cpp
// 自刎归天 - 出牌时触发
void LiuBei::onCardPlayed(Player* self, const QString& cardName, 
                          Player* target, GameEngine* engine) {
    if (cardName == QString::fromUtf8("杀")) {
        // 全员受伤
        for (int i = 0; i < engine->playerCount(); ++i) {
            Player* p = engine->playerAt(i);
            if (p && p->isAlive()) {
                p->receiveDamage(1);
            }
        }
    }
}

// 仁之剑义之剑 - 需要选择两个目标
bool LiuBei::activateSkill(int skillIndex, Player* self, Player* target, 
                           GameEngine* engine) {
    if (skillIndex == 2) {
        // 第一个目标已选择（伤害目标）
        // 请求选择第二个目标（恢复目标）
        engine->prepareSecondTargetForSkill(2, target);
        return true;
    }
}
```

### 5.3 司马懿（3血2技能）

| 技能名 | 类型 | 触发时机 | 效果 |
|-------|------|---------|------|
| 天意化骨掌 | 被动 | 出杀时 | 可无限出杀，第二张杀时触发特效 |
| 天意面瘫 | 被动 | 出桃时 | 自身恢复2点，其余存活玩家恢复1点 |

**代码实现要点**：

```cpp
// 司马懿的特殊规则：可以无限出杀
void GameEngine::playerUseCard(int cardIndex, int targetPlayerIndex) {
    if (card->getType() == CardType::Kill) {
        Hero* hero = self->getHero();
        bool canUseKill = true;
        // 司马懿例外
        if (hero && QString::fromUtf8("司马懿") == hero->getName()) {
            canUseKill = true;  // 始终可以出杀
        } else if (self->hasUsedKillThisTurn()) {
            emit logMessage(QString::fromUtf8("本回合已使用过【杀】！"));
            return;
        }
    }
}
```

---

## 6. 卡牌系统

### 6.1 卡牌生成概率

```cpp
Card* createRandomCard() {
    int r = QRandomGenerator::global()->bounded(12);
    if (r < 4)       return new KillCard();        // 33% - 杀
    else if (r < 6)  return new DodgeCard();       // 17% - 闪
    else if (r < 8)  return new PeachCard();       // 17% - 桃
    else if (r < 9)  return new TrickCard(TrickCard::StealCard);        // 8% - 顺手牵羊
    else if (r < 10) return new TrickCard(TrickCard::DiscardCard);      // 8% - 过河拆桥
    else if (r < 11) return new TrickCard(TrickCard::JumpTigerShark);   // 8% - 跳跳虎脆脆鲨
    else             return new TrickCard(TrickCard::TianyiInvasion);   // 8% - 天意侵袭
}
```

### 6.2 卡牌效果实现

**杀牌**：
```cpp
bool KillCard::play(Player* from, Player* to, GameEngine* engine) {
    if (!to) return false;
    to->receiveDamage(1);  // 直接扣血，实际由engine处理闪避逻辑
    return true;
}
```

**桃牌**：
```cpp
bool PeachCard::play(Player* from, Player* to, GameEngine* engine) {
    if (!from) return false;
    from->restoreHealth(1);  // 恢复1点体力
    return true;
}
```

**锦囊牌 - 跳跳虎脆脆鲨**：
```cpp
case JumpTigerShark: {
    // 全体回复1血
    for (int i = 0; i < engine->playerCount(); ++i) {
        Player* p = engine->playerAt(i);
        if (p && p->isAlive()) {
            p->restoreHealth(1);
        }
    }
    return true;
}
```

---

## 7. UI与动画系统

### 7.1 MainWindow布局结构

```
MainWindow
├── m_centralWidget
│   └── m_stackedWidget (QStackedWidget)
│       ├── m_characterSelectPage (角色选择页)
│       │   └── QVBoxLayout
│       │       ├── 标题Label
│       │       ├── 副标题Label
│       │       └── QHBoxLayout (武将卡片)
│       │           ├── 曹操卡片
│       │           ├── 刘备卡片
│       │           └── 司马懿卡片
│       │
│       └── m_gamePage (游戏页)
│           └── QVBoxLayout
│               ├── QHBoxLayout (顶部区域)
│               │   ├── m_heroPanels[1] (AI玩家1)
│               │   ├── m_turnLabel (回合提示)
│               │   └── m_heroPanels[2] (AI玩家2)
│               │
│               ├── QHBoxLayout (中间区域)
│               │   ├── m_log (游戏日志)
│               │   └── m_centerEffect (中央特效)
│               │
│               └── QHBoxLayout (底部区域)
│                   ├── m_heroPanels[0] (人类玩家)
│                   ├── m_handArea (手牌区域)
│                   └── 按钮面板
│                       ├── m_btnEndTurn
│                       ├── m_btnCancel
│                       ├── m_skillButtons[]
│                       └── 闪避选择按钮
│
├── m_overlay (AnimOverlay) - 动画覆盖层
├── m_videoOverlay - 视频覆盖层
└── m_videoWidget - 视频播放控件
```

### 7.2 HeroPanel绘制

HeroPanel是自定义绘制的玩家面板，使用`paintEvent`实现复杂视觉效果：

```cpp
void HeroPanel::paintEvent(QPaintEvent*) {
    QPainter p(this);
    
    // 1. 绘制背景渐变
    QLinearGradient grad(r.topLeft(), r.bottomLeft());
    grad.setColorAt(0.0, QColor(0x2C,0x1A,0x50,245));
    grad.setColorAt(1.0, QColor(0x0D,0x07,0x22,245));
    
    // 2. 绘制高亮边框（当前回合玩家）
    if (m_highlighted) {
        for (int i = 8; i >= 1; --i) {
            QColor gc = C_GOLD;
            gc.setAlpha(30*i);
            p.setPen(QPen(gc, i*3+4));
            p.drawPath(bg);
        }
    }
    
    // 3. 绘制武将头像
    if (!m_portraitPixmap.isNull()) {
        p.drawPixmap(portrait, scaled);
    }
    
    // 4. 绘制血条
    for (int i = 0; i < maxHp; ++i) {
        // 根据血量比例显示不同颜色
        if (ratio > 0.6f) fc = QColor(0x22,0xDD,0x66);  // 绿色
        else if (ratio > 0.3f) fc = QColor(0xFF,0x99,0x22);  // 橙色
        else fc = QColor(0xEE,0x33,0x33);  // 红色
    }
    
    // 5. 绘制技能列表
    for (int i = 0; i < h->getSkillCount(); ++i) {
        SkillInfo info = h->getSkillInfo(i);
        QString prefix = info.isActive() ? "[主]" : "[被]";
        p.drawText(tx, skillY, prefix + info.name);
    }
    
    // 6. 绘制受伤/治疗闪烁效果
    if (m_flashAlpha > 0.0) {
        p.fillPath(bg, QColor(0xFF,0x10,0x10,(int)(m_flashAlpha*140)));
    }
    
    // 7. 绘制阵亡遮罩
    if (!alive) {
        p.fillPath(bg, QColor(0,0,0,180));
        p.drawText(r, Qt::AlignCenter, "阵亡");
    }
}
```

### 7.3 AnimOverlay动画系统

AnimOverlay管理多种动画效果：

```cpp
class AnimOverlay : public QWidget {
    // 飞行卡牌动画
    struct FlyCard {
        QPointF pos, start, end;
        QString label;
        QColor color;
        float progress, rotation, scale;
        bool active;
    };
    
    // 技能闪烁文字
    struct FlashLabel {
        QPointF center;
        QString label;
        QColor color;
        float alpha, scale, glowPhase;
        bool active;
    };
    
    // 浮动数字（伤害/治疗）
    struct FloatingNumber {
        QPointF pos;
        QString text;  // "-1" 或 "+1"
        QColor color;
        float alpha, scale, velY;
        bool isDamage;
    };
    
    // 回合开始动画
    struct TurnStartAnim {
        QPointF center;
        QString name;
        float progress;
        bool active;
    };
};
```

**动画更新逻辑**：

```cpp
// 每16ms更新一次（约60fps）
connect(m_timer, &QTimer::timeout, this, [this]() {
    // 更新飞行卡牌
    for (auto& fc : m_flyCards) {
        fc.progress += 0.035f;
        float t = qMin(fc.progress, 1.0f);
        float e = 1.f - (1.f-t)*(1.f-t)*(1.f-t);  // 缓动函数
        float arc = -4.f * t * (t-1.f) * 80.f;     // 抛物线弧度
        fc.pos = fc.start + (fc.end - fc.start) * e;
        fc.pos.setY(fc.pos.y() - arc);             // 添加弧度
        fc.rotation += 3.0f;
    }
    
    // 更新闪烁文字
    for (auto& fl : m_flashes) {
        fl.alpha -= 0.018f;
        fl.scale += 0.025f;
    }
    
    // 更新浮动数字
    for (auto& fn : m_numbers) {
        fn.alpha -= 0.015f;
        fn.pos.setY(fn.pos.y() + fn.velY);
        fn.velY *= 0.98f;  // 减速
    }
});
```

### 7.4 粒子系统

```cpp
struct Particle {
    QPointF pos, vel;       // 位置和速度
    float life, maxLife;    // 生命周期
    QColor color;           // 颜色
    float size;             // 大小
    float rotation, rotSpeed;  // 旋转
    QString style;          // 样式: "burst"/"spiral"/"rise"
};

void MainWindow::spawnParticles(const QPoint& pos, const QColor& col, 
                                int count, const QString& style) {
    for (int i = 0; i < count; ++i) {
        Particle pt;
        pt.pos = pos;
        
        // 随机角度和速度
        float angle = QRandomGenerator::global()->bounded(360) * 3.14159f / 180.f;
        float speed = QRandomGenerator::global()->bounded(100) * 0.04f + 0.5f;
        
        if (style == "burst") {
            pt.vel = QPointF(qCos(angle)*speed*2, qSin(angle)*speed*2);
        } else if (style == "spiral") {
            pt.vel = QPointF(qCos(angle)*speed, qSin(angle)*speed);
        } else if (style == "rise") {
            pt.vel = QPointF((rand-50)*0.01f, -speed);
        }
        
        m_particles.append(pt);
    }
}
```

---

## 8. 信号槽机制详解

### 8.1 GameEngine发出的信号

| 信号 | 参数 | 触发时机 | 接收者处理 |
|-----|------|---------|-----------|
| `logMessage` | `QString msg` | 任何需要显示日志时 | MainWindow追加到m_log |
| `gameStateChanged` | 无 | 游戏状态变化 | MainWindow刷新所有UI |
| `gameOver` | `QString winner` | 游戏结束 | MainWindow显示结果对话框 |
| `animationRequest` | `type, from, to, extra` | 需要播放动画 | MainWindow调用对应动画函数 |
| `showCenterEffect` | `type, text, duration` | 中央特效 | CenterEffectWidget显示特效 |
| `skillEffectRequest` | `heroName, skillName, videoPath` | 技能发动 | MainWindow播放视频 |
| `requestDodgeResponse` | `targetIdx, timeout` | 玩家被杀 | MainWindow显示闪避按钮 |
| `requestTargetSelectionForSkill` | `skillIndex, skillName` | 需要选择目标 | MainWindow进入目标选择模式 |
| `requestUseSkillAfterDamage` | `skillIndex, skillName` | 受伤后可发动技能 | MainWindow高亮技能按钮 |
| `requestSecondTargetForSkill` | `skillIndex, firstTarget` | 需要选择第二目标 | MainWindow进入目标选择模式 |

### 8.2 信号槽连接代码

```cpp
void MainWindow::startGameWithHero(const QString& heroName) {
    // ...
    m_engine = new GameEngine(this);
    
    // 连接所有信号槽
    connect(m_engine, &GameEngine::logMessage, this, [this](const QString& msg){
        m_log->append(msg);
        m_log->verticalScrollBar()->setValue(m_log->verticalScrollBar()->maximum());
    });
    
    connect(m_engine, &GameEngine::gameStateChanged, this, &MainWindow::onUpdateUI);
    connect(m_engine, &GameEngine::gameOver, this, &MainWindow::onGameOver);
    connect(m_engine, &GameEngine::animationRequest, this, &MainWindow::onAnimationRequest);
    connect(m_engine, &GameEngine::showCenterEffect, this, &MainWindow::onShowCenterEffect);
    connect(m_engine, &GameEngine::skillEffectRequest, this, &MainWindow::onSkillEffectRequest);
    connect(m_engine, &GameEngine::requestDodgeResponse, this, &MainWindow::onRequestDodgeResponse);
    connect(m_engine, &GameEngine::requestTargetSelectionForSkill, 
            this, &MainWindow::onRequestTargetSelectionForSkill);
    connect(m_engine, &GameEngine::requestUseSkillAfterDamage, 
            this, &MainWindow::onRequestUseSkillAfterDamage);
    connect(m_engine, &GameEngine::requestSecondTargetForSkill, 
            this, &MainWindow::onRequestSecondTargetForSkill);
}
```

### 8.3 特殊信号处理：技能目标选择

```cpp
// 曹操"呱"技能需要玩家选择目标
void MainWindow::onRequestTargetSelectionForSkill(int skillIndex, const QString& skillName) {
    m_log->append(">>> 请点击一个目标玩家！");
    
    enterTargetSelectionMode([this, skillIndex](int tgt){
        if (m_engine) {
            Player* target = m_engine->playerAt(tgt);
            // 直接造成伤害
            target->receiveDamage(1);
            m_engine->animationRequest("damage", m_engine->playerIndex(self), tgt, "1");
            m_engine->logMessage(QString("【呱】对%1造成1点精神伤害！").arg(target->getDisplayName()));
            
            // 如果是AI回合，延迟后继续
            if (!m_engine->isHumanTurn()) {
                QTimer::singleShot(1000, this, [this](){
                    if (m_engine && m_engine->isRunning()) {
                        m_engine->playerEndTurn();
                    }
                });
            }
        }
    });
}
```

---

## 9. AI系统

### 9.1 AI决策流程

```cpp
void GameEngine::performAiAction() {
    Player* self = currentPlayer();
    
    // 1. 收集存活敌人
    QVector<Player*> enemies;
    for (int i = 0; i < m_players.size(); ++i) {
        if (i != m_curIdx && m_players[i]->isAlive())
            enemies << m_players[i];
    }
    
    // 2. 选择最弱目标（血量最低）
    Player* weakest = enemies[0];
    for (Player* e : enemies)
        if (e->getHealth() < weakest->getHealth()) weakest = e;
    
    // 3. 优先使用桃回血
    if (self->getHealth() <= self->getMaxHealth() - 1) {
        // 查找并使用桃
    }
    
    // 4. 使用杀攻击
    if (!self->hasUsedKillThisTurn() || canUseMultipleKills) {
        // 查找并使用杀
        // 处理目标的闪避
    }
    
    // 5. 随机使用主动技能
    if (!self->hasUsedSkillThisTurn()) {
        // 50%概率尝试发动技能
    }
    
    // 6. 使用锦囊牌
    // ...
    
    // 7. 结束回合
    advanceTurn();
}
```

### 9.2 AI闪避逻辑

```cpp
// AI被杀时自动判断是否出闪
int dodgeIdx = weakest->findDodgeCard();
if (dodgeIdx >= 0 && tIdx == 0) {  // 人类玩家攻击AI
    // AI有闪，自动使用
    weakest->removeCardByIndex(dodgeIdx);
    emit logMessage(weakest->getDisplayName() + " 使用【闪】抵消！");
} else {
    // AI无闪，受伤
    hand[i]->play(self, weakest, this);
    triggerHeroDamaged(weakest, 1);
}
```

---

## 10. 常见问题解答

### Q1: 为什么signals不能有实现？

**A**: Qt的元对象系统(MOC)会自动为signals生成实现代码。如果在cpp文件中再写实现，就会导致"多重定义"链接错误。如果需要在emit signal前执行逻辑，应该创建一个public slot方法，在其中设置状态后emit signal。

### Q2: 如何添加新武将？

1. 在Hero.h中创建新的武将类，继承Hero
2. 实现所有纯虚函数：`getSkillCount()`, `getSkillInfo()`, `activateSkill()`
3. 重写需要的生命周期钩子
4. 在MainWindow::startGameWithHero()中添加武将创建逻辑
5. 添加头像图片到img/目录

### Q3: 如何添加新卡牌？

1. 在Card.h中定义新的卡牌子类
2. 重写`play()`方法实现效果
3. 在`createRandomCard()`中添加生成逻辑
4. 如需特殊处理，修改GameEngine::playerUseCard()

### Q4: 动画卡顿怎么解决？

- 确保粒子数量不要过多（建议<100）
- 使用`QPainter::setRenderHint`时适度
- 避免在paintEvent中创建对象
- 使用定时器控制帧率在60fps以下

### Q5: 如何调试信号槽连接问题？

```cpp
// 使用Qt5新式连接语法，编译时检查
connect(sender, &Sender::signal, receiver, &Receiver::slot);

// 检查连接是否成功
QMetaObject::Connection conn = connect(...);
if (!conn) {
    qDebug() << "Connection failed!";
}

// 使用lambda调试
connect(sender, &Sender::signal, this, [](){
    qDebug() << "Signal received!";
});
```

---

## 附录A：项目配置说明

### .pro文件解析

```qmake
QT += core gui widgets multimedia multimediawidgets
# 添加多媒体模块支持视频播放

CONFIG += c++11
CONFIG += utf8_source
# 启用C++11标准和UTF-8源码编码

SOURCES += main.cpp card.cpp hero.cpp Player.cpp gameengine.cpp mainwindow.cpp
HEADERS += Card.h Hero.h Player.h gameengine.h mainwindow.h

DISTFILES += img/* video/*
# 包含资源文件
```

---

## 附录B：编译与运行

### 编译步骤

1. 打开Qt Creator
2. 文件 → 打开文件或项目 → 选择NewThreeKingdomsKill.pro
3. 选择编译套件(Desktop Qt 5.14.2 MinGW 64-bit)
4. 点击"构建" → "构建项目"
5. 点击"运行"

### 常见编译错误

| 错误 | 原因 | 解决 |
|-----|------|------|
| BOM字符错误 | 文件编码问题 | 删除文件开头的BOM字符 |
| 多重定义 | signal有实现 | 删除cpp中的signal实现 |
| 找不到头文件 | 路径问题 | 检查INCLUDEPATH |
| 链接错误 | 库未链接 | 检查QT += multimedia |

---

**文档版本**: 1.0  
**最后更新**: 2024年  
**作者**: 项目开发者
