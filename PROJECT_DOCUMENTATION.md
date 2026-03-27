# 三国杀·天意侵蚀 - 项目完整技术文档

***

## 目录

1. [项目概述](#1-项目概述)
2. [技术架构](#2-技术架构)
3. [核心模块详解](#3-核心模块详解)
4. [游戏规则系统](#4-游戏规则系统)
5. [武将技能系统](#5-武将技能系统)
6. [卡牌系统](#6-卡牌系统)
7. [AI系统](#7-ai系统)
8. [UI与动画系统](#8-ui与动画系统)
9. [信号与槽机制](#9-信号与槽机制)
10. [关键代码流程](#10-关键代码流程)
11. [答辩常见问题](#11-答辩常见问题)

***

## 1. 项目概述

### 1.1 项目背景

本项目是一款基于Qt框架开发的三国杀卡牌对战游戏，名为"三国杀·天意侵蚀"。游戏采用经典的回合制卡牌对战模式，支持1名玩家与2名AI对手进行三方混战。

### 1.2 项目特色

- **邪化武将系统**：对传统三国杀武将进行创意改编，赋予独特的"邪化"技能
- **丰富的动画效果**：粒子特效、卡牌飞行动画、技能特效视频
- **智能AI对手**：具备策略决策能力的AI系统
- **精美的UI设计**：深色主题、金色点缀、玄幻风格

### 1.3 开发环境

| 项目   | 说明                                                |
| ---- | ------------------------------------------------- |
| 开发框架 | Qt 5.x (Widgets + Multimedia)                     |
| 编程语言 | C++11                                             |
| 开发工具 | Qt Creator                                        |
| 编译系统 | qmake                                             |
| 核心模块 | core, gui, widgets, multimedia, multimediawidgets |

***

## 2. 技术架构

### 2.1 项目文件结构

```
NewThreeKingdomsKill/
├── main.cpp                    # 程序入口
├── mainwindow.h/cpp            # 主窗口与UI逻辑
├── gameengine.h/cpp            # 游戏引擎核心
├── Hero.h/cpp                  # 武将系统
├── Card.h/cpp                  # 卡牌系统
├── Player.h/cpp                # 玩家类
├── mainwindow.ui               # Qt Designer界面文件
├── NewThreeKingdomsKill.pro    # 项目配置文件
├── img/                        # 图片资源
│   ├── caocao.gif              # 曹操动态头像
│   ├── liubei.png              # 刘备头像
│   └── simayi.png              # 司马懿头像
└── video/                      # 技能特效视频
    ├── caocao_doujiyan.mp4     # 曹操-斗鸡眼
    ├── caocao_gua.mp4          # 曹操-呱
    ├── caocao_linzhong.mp4     # 曹操-临终关怀
    ├── caocao_wang.mp4         # 曹操-汪
    ├── liubei_erdi.mp4         # 刘备-二弟
    ├── liubei_jian.mp4         # 刘备-仁之剑义之剑
    ├── liubei_ziwen.mp4        # 刘备-自刎归天
    └── simayi_huagu.mp4        # 司马懿-化骨掌
```

### 2.2 架构设计图

```
┌─────────────────────────────────────────────────────────────┐
│                        MainWindow                            │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ 角色选择界面 │  │  游戏界面   │  │    动画/特效层      │  │
│  │ (Character  │  │  (GamePage) │  │ (AnimOverlay/       │  │
│  │ SelectPage) │  │             │  │  CenterEffect)      │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└───────────────────────────┬─────────────────────────────────┘
                            │ 信号/槽连接
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                      GameEngine                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐   │
│  │ 回合管理     │  │ 卡牌使用逻辑 │  │ AI决策系统       │   │
│  │ (Turn Mgmt)  │  │ (Card Logic) │  │ (AI System)      │   │
│  └──────────────┘  └──────────────┘  └──────────────────┘   │
└───────────────────────────┬─────────────────────────────────┘
                            │
            ┌───────────────┼───────────────┐
            ▼               ▼               ▼
     ┌──────────┐    ┌──────────┐    ┌──────────┐
     │ Player 0 │    │ Player 1 │    │ Player 2 │
     │ (玩家)   │    │ (AI)     │    │ (AI)     │
     └────┬─────┘    └────┬─────┘    └────┬─────┘
          │               │               │
          ▼               ▼               ▼
     ┌──────────┐    ┌──────────┐    ┌──────────┐
     │  Hero    │    │  Hero    │    │  Hero    │
     │ (武将)   │    │ (武将)   │    │ (武将)   │
     └──────────┘    └──────────┘    └──────────┘
```

### 2.3 类关系图

```
┌─────────────┐     包含      ┌─────────────┐
│   Player    │──────────────▶│    Hero     │
└─────────────┘               └─────────────┘
      │                              │
      │ 包含                         │ 继承
      ▼                              ▼
┌─────────────┐               ┌─────────────┐
│    Card     │               │  CaoCao     │
└─────────────┘               │  LiuBei     │
      │                       │  SimaYi     │
      │ 继承                  └─────────────┘
      ▼
┌─────────────────────────────────────────┐
│ KillCard | DodgeCard | PeachCard |      │
│ TrickCard                               │
└─────────────────────────────────────────┘
```

***

## 3. 核心模块详解

### 3.1 main.cpp - 程序入口

**文件位置**: [main.cpp](main.cpp)

**核心功能**:

```cpp
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("ThreeKingdomsKill");
    a.setOrganizationName("SKG");
    a.setStyle("Fusion");  // 使用Fusion风格
    
    // 设置全局样式表
    a.setStyleSheet(
        "QWidget{"
        "  font-family:'Microsoft YaHei','SimHei','Noto Sans CJK SC',sans-serif;"
        "}"
        // ... 工具提示、消息框样式
    );

    MainWindow w;
    w.show();
    return a.exec();
}
```

**关键点**:

1. **QApplication初始化**: 创建Qt应用程序实例
2. **全局样式设置**: 统一字体为微软雅黑，设置深色主题
3. **Fusion风格**: 提供跨平台一致的现代化外观

### 3.2 MainWindow - 主窗口类

**文件位置**: [mainwindow.h](mainwindow.h) | [mainwindow.cpp](mainwindow.cpp)

**类定义核心结构**:

```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    
private:
    // 核心组件
    GameEngine* m_engine;           // 游戏引擎
    Player* m_players[3];           // 三名玩家
    
    // UI组件
    QStackedWidget* m_stackedWidget;     // 页面切换器
    QWidget* m_characterSelectPage;       // 角色选择页
    QWidget* m_gamePage;                  // 游戏页面
    HeroPanel* m_heroPanels[3];          // 玩家面板
    AnimOverlay* m_overlay;              // 动画覆盖层
    CenterEffectWidget* m_centerEffect;  // 中央特效
    
    // 交互状态
    bool m_selectingTarget;               // 目标选择模式
    std::function<void(int)> m_targetCallback;  // 目标选择回调
};
```

**UI构建流程**:

```cpp
void MainWindow::buildUI() {
    // 1. 创建中央部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 2. 创建页面切换器
    m_stackedWidget = new QStackedWidget(m_centralWidget);
    
    // 3. 构建两个主要页面
    buildCharacterSelectUI();  // 角色选择界面
    buildGameUI();             // 游戏界面
    
    // 4. 默认显示角色选择页
    m_stackedWidget->setCurrentWidget(m_characterSelectPage);
}
```

**关键子类**:

#### HeroPanel - 玩家面板组件

```cpp
class HeroPanel : public QWidget {
    // 显示玩家信息：头像、名称、血量、手牌数、技能
    void setPortraitImage(const QString& imagePath);  // 设置静态头像
    void setPortraitGif(const QString& gifPath);      // 设置动态头像
    void flashDamage();    // 受伤闪烁效果
    void flashHeal();      // 回血效果
    void playTurnGlow();   // 回合开始发光
};
```

#### AnimOverlay - 动画覆盖层

```cpp
class AnimOverlay : public QWidget {
    // 四种动画类型
    void playCardFly(from, to, label, color);     // 卡牌飞行
    void playSkillFlash(center, label, color);    // 技能闪光
    void playDamageNumber(pos, amount, color);    // 伤害数字
    void playTurnStart(center, name);             // 回合开始
};
```

#### CenterEffectWidget - 中央特效组件

```cpp
class CenterEffectWidget : public QWidget {
    // 在屏幕中央显示大字特效
    void showEffect(const QString& text, const QString& type, int durationMs);
    // type: "kill", "dodge", "peach", "skill", "trick", "gameover"
};
```

### 3.3 GameEngine - 游戏引擎

**文件位置**: [gameengine.h](gameengine.h) | [gameengine.cpp](gameengine.cpp)

**核心职责**:

1. 游戏流程控制（开始、回合、结束）
2. 卡牌使用逻辑
3. 技能触发机制
4. AI行动决策
5. 胜负判定

**关键成员变量**:

```cpp
class GameEngine : public QObject {
    Q_OBJECT
private:
    QVector<Player*> m_players;     // 玩家列表
    int m_curIdx;                   // 当前玩家索引
    bool m_running;                 // 游戏运行状态
    QTimer* m_aiTimer;              // AI行动定时器
    
    // 闪避响应机制
    bool m_waitingForDodgeResponse;
    int m_dodgeResponseTarget;
    QTimer* m_dodgeTimer;
};
```

**核心信号**:

```cpp
signals:
    void logMessage(const QString& msg);           // 日志消息
    void gameStateChanged();                        // 状态变化
    void animationRequest(type, from, to, extra);  // 动画请求
    void gameOver(const QString& winnerName);       // 游戏结束
    void skillEffectRequest(hero, skill, video);   // 技能特效
    void requestDodgeResponse(target, timeout);    // 请求闪避响应
```

**游戏启动流程**:

```cpp
void GameEngine::startGame(Player* p0, Player* p1, Player* p2) {
    m_players << p0 << p1 << p2;
    m_curIdx = 0;
    m_running = true;
    
    // 所有玩家摸4张初始牌
    for (Player* p : m_players)
        p->drawCards(4);
    
    // 发送游戏开始信号
    emit logMessage("══════ 天意侵蚀·三方对弈 开始！══════");
    emit showCenterEffect("gamestart", "天意侵蚀", 2000);
    
    // 开始第一个回合
    startTurn();
}
```

***

## 4. 游戏规则系统

### 4.1 回合流程

```
回合开始
    │
    ▼
┌─────────────┐
│ 1. 重置状态 │  resetTurnState(): 清除已出杀/已用技能标记
└─────────────┘
    │
    ▼
┌─────────────┐
│ 2. 摸牌阶段 │  drawCards(2): 从牌堆摸2张牌
└─────────────┘
    │
    ▼
┌─────────────┐
│ 3. 回合开始 │  triggerHeroTurnStart(): 触发回合开始技能
│    技能触发 │  (如曹操的"战无不胜の斗鸡眼")
└─────────────┘
    │
    ▼
┌─────────────┐
│ 4. 出牌阶段 │  玩家/AI使用卡牌和技能
│    (主阶段) │  - 使用【杀】攻击
└─────────────┘  - 使用【桃】回血
    │            - 使用锦囊牌
    ▼            - 发动主动技能
┌─────────────┐
│ 5. 回合结束 │  triggerHeroTurnEnd(): 触发回合结束技能
│    技能触发 │  (如曹操的"呱")
└─────────────┘
    │
    ▼
┌─────────────┐
│ 6. 下一回合 │  advanceTurn(): 切换到下一位存活玩家
└─────────────┘
```

### 4.2 出牌限制

**【杀】的使用限制**:

```cpp
// 普通武将：每回合只能出一张杀
if (self->hasUsedKillThisTurn()) {
    emit logMessage("本回合已使用过【杀】！");
    return;
}

// 司马懿例外：可以无限出杀（天意化骨掌）
if (hero && hero->getName() == "司马懿") {
    canUseKill = true;  // 无限制
}
```

**【闪】的使用规则**:

```cpp
// 【闪】只能被动使用，不能主动打出
if (card->getType() == CardType::Dodge) {
    emit logMessage("【闪】只能应对他人的【杀】，不能主动使用");
    return;
}
```

### 4.3 伤害与治疗

```cpp
// 受伤逻辑
void Player::receiveDamage(int amount) {
    m_hp -= amount;
    if (m_hp < 0) m_hp = 0;
    // 触发受伤技能（如刘备的"无敌の二弟"）
}

// 回血逻辑
void Player::restoreHealth(int amount) {
    m_hp += amount;
    if (m_hp > m_maxHp) m_hp = m_maxHp;  // 不能超过上限
}
```

### 4.4 游戏结束判定

```cpp
void GameEngine::checkGameOver() {
    QVector<Player*> alive;
    for (Player* p : m_players)
        if (p->isAlive()) alive << p;

    if (alive.size() <= 1) {
        QString winner = alive.isEmpty() 
            ? "无人胜出" 
            : alive[0]->getDisplayName();
        emit gameOver(winner);
        endGame();
    }
}
```

***

## 5. 武将技能系统

### 5.1 技能类型定义

```cpp
enum class SkillType {
    Active,   // 主动技能：玩家主动发动
    Passive   // 被动技能：满足条件自动触发
};

struct SkillInfo {
    QString name;        // 技能名称
    QString desc;        // 技能描述
    SkillType type;      // 技能类型
    bool isActive() const { return type == SkillType::Active; }
};
```

### 5.2 武将基类

```cpp
class Hero {
public:
    Hero(const QString& name, int maxHp);
    
    // 纯虚函数：子类必须实现
    virtual bool activateSkill(int skillIndex, Player* self, 
                               Player* target, GameEngine* engine) = 0;
    virtual int getSkillCount() const = 0;
    virtual SkillInfo getSkillInfo(int index) const = 0;
    
    // 虚函数：子类可选择重写
    virtual void onTurnStart(Player* self, GameEngine* engine);
    virtual void onTurnEnd(Player* self, GameEngine* engine);
    virtual void onCardPlayed(Player* self, const QString& cardName, 
                              Player* target, GameEngine* engine);
    virtual void onDamaged(Player* self, int dmg, GameEngine* engine);
    virtual void onDeath(Player* self, GameEngine* engine);
};
```

### 5.3 曹操 - 邪化武将详解

**武将定位**: 高风险高回报的"自爆型"武将

**技能列表**:

| 技能名      | 类型 | 触发时机 | 效果                  |
| -------- | -- | ---- | ------------------- |
| 战无不胜の斗鸡眼 | 被动 | 回合开始 | 血量≤50%时，对全体敌方造成1点伤害 |
| 呱        | 被动 | 回合结束 | 本回合未出牌时，对指定玩家造成1点伤害 |
| 汪        | 主动 | 出牌阶段 | 与目标进行"狗叫对决"，输者受1点伤害 |
| 临终关怀     | 被动 | 死亡时  | 对全体玩家造成1点伤害         |

**技能实现代码**:

```cpp
// 技能0：战无不胜の斗鸡眼
void CaoCao::onTurnStart(Player* self, GameEngine* engine) {
    int hp = self->getHealth();
    int maxHp = self->getMaxHealth();
    
    // 血量≤50%时触发
    if (hp > 0 && hp * 2 <= maxHp && !m_doujiyanTriggered) {
        m_doujiyanTriggered = true;
        
        // 播放技能特效视频
        emit engine->skillEffectRequest("曹操", "战无不胜の斗鸡眼", 
                                        "video/caocao_doujiyan.mp4");
        
        // 对所有敌方造成伤害
        for (int i = 0; i < engine->playerCount(); ++i) {
            Player* p = engine->playerAt(i);
            if (p && p != self && p->isAlive()) {
                p->receiveDamage(1);
                emit engine->animationRequest("damage", ...);
            }
        }
    }
}

// 技能2：汪（狗叫对决）
bool CaoCao::activateSkill(int skillIndex, Player* self, 
                           Player* target, GameEngine* engine) {
    if (skillIndex == 2) {
        // 检查双方是否有【杀】
        bool selfHasKill = self->findKillCard() >= 0;
        bool targetHasKill = target->findKillCard() >= 0;
        
        if (!selfHasKill && !targetHasKill) {
            // 平局
        } else if (!selfHasKill) {
            // 自己受伤害
            self->receiveDamage(1);
        } else if (!targetHasKill) {
            // 对方受伤害
            target->receiveDamage(1);
        } else {
            // 双方各消耗一张【杀】
        }
        return true;
    }
}
```

### 5.4 刘备 - 邪化武将详解

**武将定位**: "同归于尽"型武将，牺牲自己伤害敌人

**技能列表**:

| 技能名    | 类型 | 触发时机  | 效果                 |
| ------ | -- | ----- | ------------------ |
| 自刎归天   | 被动 | 出【杀】时 | 全员（包括自己）受1点伤害      |
| 无敌の二弟  | 被动 | 受伤后   | 可召唤二弟对攻击者造成1点伤害    |
| 仁之剑义之剑 | 主动 | 出牌阶段  | 一名玩家回1血，另一名玩家受1点伤害 |

**技能实现代码**:

```cpp
// 技能0：自刎归天
void LiuBei::onCardPlayed(Player* self, const QString& cardName, 
                          Player* target, GameEngine* engine) {
    if (cardName == "杀") {
        // 全员受伤（包括自己！）
        for (int i = 0; i < engine->playerCount(); ++i) {
            Player* p = engine->playerAt(i);
            if (p && p->isAlive()) {
                p->receiveDamage(1);
            }
        }
    }
}

// 技能1：无敌の二弟
void LiuBei::onDamaged(Player* self, int dmg, GameEngine* engine) {
    m_canSummonErDi = true;
    // 提示玩家可以发动技能
    engine->prepareUseSkillAfterDamage(1, "无敌の二弟");
}
```

### 5.5 司马懿 - 邪化武将详解

**武将定位**: "控制型"武将，擅长持续输出和群体治疗

**技能列表**:

| 技能名   | 类型 | 触发时机  | 效果           |
| ----- | -- | ----- | ------------ |
| 天意化骨掌 | 被动 | 出【杀】时 | 可无限出【杀】      |
| 天意面瘫  | 被动 | 出【桃】时 | 自身回2血，其他人回1血 |

**技能实现代码**:

```cpp
// 技能0：天意化骨掌（无限出杀）
void SimaYi::onCardPlayed(Player* self, const QString& cardName, 
                          Player* target, GameEngine* engine) {
    if (cardName == "杀") {
        m_killCountThisTurn++;
        if (m_killCountThisTurn >= 2) {
            // 第二张杀开始显示特效
            emit engine->skillEffectRequest("司马懿", "天意化骨掌",
                                            "video/simayi_huagu.mp4");
        }
    }
}

// 技能1：天意面瘫（群体治疗）
void SimaYi::onCardPlayed(Player* self, const QString& cardName, 
                          Player* target, GameEngine* engine) {
    if (cardName == "桃") {
        self->restoreHealth(2);  // 自己回2血
        for (其他存活玩家) {
            p->restoreHealth(1);  // 其他人回1血
        }
    }
}
```

***

## 6. 卡牌系统

### 6.1 卡牌类型枚举

```cpp
enum class CardType {
    Kill,    // 杀：造成伤害
    Dodge,   // 闪：抵消伤害
    Peach,   // 桃：恢复体力
    Trick    // 锦囊：特殊效果
};
```

### 6.2 卡牌基类

```cpp
class Card {
protected:
    QString name;         // 卡牌名称
    QString description;  // 卡牌描述
    CardType type;        // 卡牌类型
    
public:
    // 核心方法：使用卡牌
    virtual bool play(Player* from, Player* to, GameEngine* engine);
    
    // 是否可以被闪避
    virtual bool canRespondWithDodge() const { return false; }
    
    // 是否需要目标
    virtual bool requiresTarget() const { return true; }
};
```

### 6.3 具体卡牌实现

#### KillCard - 【杀】

```cpp
class KillCard : public Card {
public:
    KillCard() : Card("杀", "对一名其他角色造成1点伤害...", CardType::Kill) {}
    
    bool play(Player* from, Player* to, GameEngine* engine) override {
        to->receiveDamage(1);
        return true;
    }
    
    bool canRespondWithDodge() const override { return true; }
};
```

#### PeachCard - 【桃】

```cpp
class PeachCard : public Card {
public:
    PeachCard() : Card("桃", "恢复1点体力...", CardType::Peach) {}
    
    bool play(Player* from, Player* to, GameEngine* engine) override {
        from->restoreHealth(1);
        return true;
    }
};
```

### 6.4 锦囊牌系统

```cpp
class TrickCard : public Card {
public:
    enum TrickSubType { 
        StealCard,        // 顺手牵羊：偷一张牌
        DiscardCard,      // 过河拆桥：弃一张牌
        JumpTigerShark,   // 一对跳跳虎两条脆脆鲨：全体回血
        TianyiInvasion    // 天意侵袭：不出杀就受伤
    };
    
    bool play(Player* from, Player* to, GameEngine* engine) override {
        switch (subType) {
            case StealCard:
                // 随机偷取目标一张手牌
                Card* stolen = to->getHandCards()[random];
                to->removeCard(stolen);
                from->addCard(stolen);
                break;
                
            case TianyiInvasion:
                // 全体必须出杀，否则受伤
                for (每个存活玩家) {
                    if (有杀) 消耗一张杀;
                    else 受1点伤害;
                }
                break;
        }
    }
};
```

### 6.5 卡牌生成机制

```cpp
Card* createRandomCard() {
    int r = QRandomGenerator::global()->bounded(12);
    if (r < 4)       return new KillCard();        // 33% 概率
    else if (r < 6)  return new DodgeCard();       // 17% 概率
    else if (r < 8)  return new PeachCard();       // 17% 概率
    else if (r < 9)  return new TrickCard(StealCard);      // 8%
    else if (r < 10) return new TrickCard(DiscardCard);    // 8%
    else if (r < 11) return new TrickCard(JumpTigerShark); // 8%
    else             return new TrickCard(TianyiInvasion); // 8%
}
```

***

## 7. AI系统

### 7.1 AI决策流程

```
AI回合开始
    │
    ▼
┌─────────────────┐
│ 1. 检查血量     │  血量不满且有【桃】时优先回血
│    优先回血     │
└─────────────────┘
    │
    ▼
┌─────────────────┐
│ 2. 选择目标     │  选择血量最低的敌人
│    (最弱敌人)   │
└─────────────────┘
    │
    ▼
┌─────────────────┐
│ 3. 使用【杀】   │  有杀且未使用过（司马懿除外）
│    攻击目标     │
└─────────────────┘
    │
    ▼
┌─────────────────┐
│ 4. 发动技能     │  50%概率发动主动技能
│    (随机决策)   │
└─────────────────┘
    │
    ▼
┌─────────────────┐
│ 5. 使用锦囊     │  有锦囊牌时使用
└─────────────────┘
    │
    ▼
┌─────────────────┐
│ 6. 结束回合     │
└─────────────────┘
```

### 7.2 AI核心代码

```cpp
void GameEngine::performAiAction() {
    Player* self = currentPlayer();
    
    // 1. 找出所有敌人
    QVector<Player*> enemies;
    for (int i = 0; i < m_players.size(); ++i) {
        if (i != m_curIdx && m_players[i]->isAlive())
            enemies << m_players[i];
    }
    
    // 2. 选择血量最低的敌人
    Player* weakest = enemies[0];
    for (Player* e : enemies)
        if (e->getHealth() < weakest->getHealth()) 
            weakest = e;
    
    // 3. 优先回血
    if (self->getHealth() <= self->getMaxHealth() - 1) {
        // 查找并使用【桃】
        for (int i = 0; i < hand.size(); ++i) {
            if (hand[i]->getType() == CardType::Peach) {
                hand[i]->play(self, self, this);
                self->removeCardByIndex(i);
                break;
            }
        }
    }
    
    // 4. 使用【杀】攻击
    if (!self->hasUsedKillThisTurn() || canUseMultipleKills) {
        for (int i = 0; i < hand.size(); ++i) {
            if (hand[i]->getType() == CardType::Kill) {
                hand[i]->play(self, weakest, this);
                self->setKillUsed(true);
                self->removeCardByIndex(i);
                break;
            }
        }
    }
    
    // 5. 随机发动技能
    if (!self->hasUsedSkillThisTurn()) {
        if (QRandomGenerator::global()->bounded(100) < 50) {
            h->activateSkill(skillIdx, self, weakest, this);
        }
    }
    
    // 6. 结束回合
    advanceTurn();
}
```

### 7.3 AI闪避处理

```cpp
// AI被攻击时的闪避逻辑
int dodgeIdx = weakest->findDodgeCard();
if (dodgeIdx >= 0 && targetIsHuman) {
    // 玩家被攻击，询问是否使用闪
    emit requestDodgeResponse(targetIdx, 5);
} else if (dodgeIdx >= 0) {
    // AI自动使用闪
    weakest->removeCardByIndex(dodgeIdx);
    emit logMessage(weakest->getName() + " 使用【闪】抵消！");
}
```

***

## 8. UI与动画系统

### 8.1 粒子系统

**粒子结构定义**:

```cpp
struct Particle {
    QPointF pos, vel;     // 位置和速度
    float life, maxLife;  // 生命周期
    QColor color;         // 颜色
    float size;           // 大小
    float rotation;       // 旋转角度
    float rotSpeed;       // 旋转速度
    QString style;        // 样式："burst", "spiral", "rise"
};
```

**粒子生成**:

```cpp
void MainWindow::spawnParticles(const QPoint& pos, const QColor& col, 
                                 int count, const QString& style) {
    for (int i = 0; i < count; ++i) {
        Particle pt;
        pt.pos = pos;
        
        // 随机方向和速度
        float angle = random(360) * PI / 180;
        float speed = random(100) * 0.04 + 0.5;
        
        // 根据样式设置速度
        if (style == "burst") {
            pt.vel = QPointF(cos(angle)*speed*2, sin(angle)*speed*2);
        } else if (style == "rise") {
            pt.vel = QPointF(random*0.01, -speed);  // 向上飘
        }
        
        m_particles.append(pt);
    }
}
```

### 8.2 卡牌飞行动画

```cpp
void AnimOverlay::playCardFly(const QPoint& from, const QPoint& to, 
                               const QString& label, const QColor& col) {
    FlyCard fc;
    fc.start = from;
    fc.end = to;
    fc.label = label;
    fc.color = col;
    fc.progress = 0;
    fc.active = true;
    m_flyCards.append(fc);
}

// 动画更新（每16ms）
void AnimOverlay::update() {
    for (auto& fc : m_flyCards) {
        fc.progress += 0.035;
        
        // 缓动函数
        float t = fc.progress;
        float ease = 1 - (1-t)*(1-t)*(1-t);  // ease-out cubic
        
        // 抛物线轨迹
        float arc = -4 * t * (t-1) * 80;
        fc.pos = start + (end - start) * ease;
        fc.pos.setY(fc.pos.y() - arc);
        
        // 旋转效果
        fc.rotation += 3;
    }
}
```

### 8.3 视频特效系统

```cpp
void MainWindow::playVideoEffect(const QString& videoPath) {
    // 创建视频播放器
    m_videoPlayer = new QMediaPlayer(this);
    m_videoWidget = new QVideoWidget(m_videoOverlay);
    m_videoPlayer->setVideoOutput(m_videoWidget);
    
    // 设置视频位置（居中显示）
    int videoW = gameRect.width() * 0.6;
    int videoH = gameRect.height() * 0.5;
    m_videoOverlay->setGeometry(centerX, centerY, videoW, videoH);
    
    // 播放视频
    m_videoPlayer->setMedia(QUrl::fromLocalFile(videoPath));
    m_videoPlayer->play();
    
    // 视频结束后自动隐藏
    connect(m_videoPlayer, &QMediaPlayer::stateChanged, [=](State state) {
        if (state == StoppedState) {
            m_videoOverlay->hide();
        }
    });
}
```

### 8.4 自定义绘制

**主窗口背景绘制**:

```cpp
void MainWindow::paintEvent(QPaintEvent*) {
    QPainter p(this);
    QRect r = rect();
    
    // 1. 渐变背景
    QLinearGradient bg(r.topLeft(), r.bottomRight());
    bg.setColorAt(0.0, QColor(0x0D, 0x08, 0x1E));  // 深紫
    bg.setColorAt(0.5, QColor(0x12, 0x08, 0x28));  // 紫
    bg.setColorAt(1.0, QColor(0x08, 0x04, 0x14));  // 黑紫
    p.fillRect(r, bg);
    
    // 2. 网格线
    p.setPen(QPen(QColor(255,255,255,5), 1));
    for (int x = 0; x < r.width(); x += 48)
        p.drawLine(x, 0, x, r.height());
    for (int y = 0; y < r.height(); y += 48)
        p.drawLine(0, y, r.width(), y);
    
    // 3. 脉冲光晕
    QRadialGradient rg(r.center(), r.width() * 0.5);
    int pulseAlpha = 20 + (int)(10 * sin(m_bgPhase));
    rg.setColorAt(0, QColor(0x45, 0x10, 0x90, pulseAlpha));
    rg.setColorAt(1, QColor(0, 0, 0, 0));
    p.fillRect(r, rg);
    
    // 4. 绘制粒子
    for (const auto& pt : m_particles) {
        // ... 粒子绘制
    }
}
```

***

## 9. 信号与槽机制

### 9.1 核心信号连接

```cpp
// MainWindow与GameEngine的信号连接
void MainWindow::startGameWithHero(const QString& heroName) {
    m_engine = new GameEngine(this);
    
    // 日志消息
    connect(m_engine, &GameEngine::logMessage, this, [this](const QString& msg){
        m_log->append(msg);
    });
    
    // 状态更新
    connect(m_engine, &GameEngine::gameStateChanged, 
            this, &MainWindow::onUpdateUI);
    
    // 游戏结束
    connect(m_engine, &GameEngine::gameOver, 
            this, &MainWindow::onGameOver);
    
    // 动画请求
    connect(m_engine, &GameEngine::animationRequest, 
            this, &MainWindow::onAnimationRequest);
    
    // 技能特效
    connect(m_engine, &GameEngine::skillEffectRequest,
            this, &MainWindow::onSkillEffectRequest);
    
    // 闪避响应
    connect(m_engine, &GameEngine::requestDodgeResponse,
            this, &MainWindow::onRequestDodgeResponse);
}
```

### 9.2 用户交互信号

```cpp
// 结束回合按钮
connect(m_btnEndTurn, &QPushButton::clicked, 
        this, &MainWindow::onEndTurn);

// 技能按钮
for (int i = 0; i < skillButtons.size(); ++i) {
    connect(btn, &QPushButton::clicked, this, [this, i](){
        onSkillClicked(i);
    });
}

// 玩家面板点击（目标选择）
for (int i = 0; i < 3; ++i) {
    connect(m_heroPanels[i], &HeroPanel::clicked, 
            this, &MainWindow::onTargetSelected);
}
```

### 9.3 目标选择模式

```cpp
void MainWindow::enterTargetSelectionMode(std::function<void(int)> callback) {
    m_selectingTarget = true;
    m_targetCallback = callback;
    
    // 高亮可选目标
    for (int i = 0; i < 3; ++i) 
        m_heroPanels[i]->highlight(i != 0);
    
    // 显示取消按钮
    m_btnCancel->show();
    m_log->append("请点击目标玩家...");
}

void MainWindow::onTargetSelected(int playerIndex) {
    if (!m_selectingTarget || !m_targetCallback) return;
    
    auto cb = m_targetCallback;
    exitTargetSelectionMode();
    cb(playerIndex);  // 执行回调
}
```

***

## 10. 关键代码流程

### 10.1 游戏启动完整流程

```
main()
  │
  ▼
QApplication初始化 → 设置全局样式
  │
  ▼
MainWindow构造
  │
  ├── buildUI()
  │     ├── buildCharacterSelectUI()  // 角色选择界面
  │     └── buildGameUI()             // 游戏界面
  │
  ▼
w.show()  // 显示主窗口
  │
  ▼
用户点击武将卡片
  │
  ▼
onCharacterSelected(heroName)
  │
  ├── 创建3个Player对象
  ├── 为每个Player设置Hero
  ├── 创建GameEngine
  ├── 连接所有信号
  │
  ▼
m_engine->startGame(p0, p1, p2)
  │
  ├── 所有玩家摸4张初始牌
  ├── 发送游戏开始日志
  ├── 显示"天意侵蚀"特效
  │
  ▼
startTurn()  // 开始第一个回合
```

### 10.2 出杀完整流程

```
玩家点击【杀】卡牌
  │
  ▼
onCardClicked(index)
  │
  ├── 检查是否已出过杀（司马懿除外）
  ├── 进入目标选择模式
  │
  ▼
玩家点击目标面板
  │
  ▼
onTargetSelected(targetIndex)
  │
  ▼
m_engine->playerUseCard(cardIndex, targetIndex)
  │
  ├── 获取卡牌和目标
  ├── card->play(from, to, engine)
  │     └── to->receiveDamage(1)
  │
  ├── 发送动画请求
  ├── 发送日志消息
  ├── 移除手牌
  │
  ▼
triggerHeroCardPlayed()  // 触发卡牌技能
  │
  ├── 刘备：触发"自刎归天"
  │     └── 全员受伤
  │
  ├── 司马懿：触发"天意化骨掌"
  │     └── 记录杀的数量
  │
  ▼
checkGameOver()  // 检查游戏是否结束
```

### 10.3 AI回合完整流程

```
m_aiTimer超时（1500ms后）
  │
  ▼
aiTakeTurn()
  │
  ▼
performAiAction()
  │
  ├── 1. 检查血量，优先使用【桃】回血
  │
  ├── 2. 选择血量最低的敌人
  │
  ├── 3. 使用【杀】攻击
  │     ├── 如果目标是玩家且有【闪】
  │     │     └── 等待玩家响应
  │     └── 否则直接造成伤害
  │
  ├── 4. 50%概率发动主动技能
  │
  ├── 5. 使用锦囊牌
  │
  ▼
advanceTurn()  // 切换到下一个玩家
```

***

## 11. 答辩常见问题

### Q1: 请介绍一下项目的整体架构？

**回答要点**:

- 本项目采用经典的MVC架构思想
- **Model层**: Player、Hero、Card等数据模型
- **View层**: MainWindow、HeroPanel等UI组件
- **Controller层**: GameEngine游戏引擎
- 使用Qt的信号槽机制实现模块间通信
- 采用面向对象设计，通过继承实现多态（武将、卡牌）

### Q2: 为什么选择Qt框架开发这个游戏？

**回答要点**:

1. **跨平台性**: Qt支持Windows/Linux/macOS
2. **丰富的UI组件**: 提供完善的Widgets模块
3. **多媒体支持**: 内置Multimedia模块，方便播放视频
4. **信号槽机制**: 简化事件处理和模块通信
5. **自定义绘制**: QPainter支持复杂的动画效果
6. **成熟的开发工具**: Qt Creator提供可视化设计

### Q3: 游戏中的技能系统是如何设计的？

**回答要点**:

- 使用**策略模式**：每个武将是一个独立的类，继承自Hero基类
- **主动技能**: 通过`activateSkill()`方法实现，玩家点击触发
- **被动技能**: 通过钩子方法实现（`onTurnStart`, `onDamaged`, `onCardPlayed`等）
- **技能信息封装**: 使用`SkillInfo`结构体存储技能名称、描述、类型
- **触发时机**: 游戏引擎在特定时机调用Hero的钩子方法

### Q4: AI系统是如何实现的？有什么策略？

**回答要点**:

- AI决策基于**优先级规则**，而非机器学习
- **优先级排序**:
  1. 血量危险时优先回血
  2. 攻击血量最低的敌人（斩杀优先）
  3. 随机发动技能（增加不可预测性）
  4. 使用锦囊牌
- **延迟执行**: 使用QTimer延迟1.5秒执行，给玩家观察时间
- **闪避处理**: AI自动判断是否使用【闪】

### Q5: 动画效果是如何实现的？

**回答要点**:

- **粒子系统**: 自定义Particle结构，每帧更新位置和生命周期
- **卡牌飞行动画**: 使用缓动函数和抛物线轨迹
- **视频特效**: 使用QMediaPlayer + QVideoWidget播放技能视频
- **自定义绘制**: 重写paintEvent()，使用QPainter绑定绘制
- **定时刷新**: 16ms定时器（约60FPS）实现流畅动画

### Q6: 如何处理玩家与AI的交互？

**回答要点**:

- **状态机模式**: 游戏引擎维护当前玩家索引
- **异步响应**: 使用信号槽处理玩家的闪避响应
- **超时机制**: 玩家有5秒时间决定是否出闪
- **目标选择**: 使用回调函数模式，等待玩家选择目标后执行操作

### Q7: 项目中遇到的最大挑战是什么？如何解决的？

**回答要点**:

- **挑战1**: 复杂的技能触发逻辑
  - **解决**: 设计钩子函数体系，在特定时机自动触发
- **挑战2**: 动画与游戏逻辑的同步
  - **解决**: 使用信号槽机制，游戏逻辑发送动画请求
- **挑战3**: AI决策的平衡性
  - **解决**: 基于优先级的规则系统，确保AI有合理的策略

### Q8: 如果要扩展这个项目，你会怎么做？

**回答要点**:

1. **网络对战**: 使用Qt Network模块实现联机功能
2. **更多武将**: 扩展Hero继承体系，添加新武将
3. **数据库存储**: 使用SQLite存储游戏记录和玩家数据
4. **音效系统**: 添加背景音乐和技能音效
5. **AI优化**: 引入决策树或简单的机器学习
6. **UI美化**: 使用QML实现更炫酷的界面

### Q9: 项目中的设计模式有哪些？

**回答要点**:

1. **策略模式**: 不同武将的技能实现
2. **观察者模式**: 信号槽机制
3. **工厂模式**: `createRandomCard()`创建随机卡牌
4. **模板方法模式**: Hero基类定义钩子方法
5. **状态模式**: 游戏的不同阶段（选将、出牌、结束）

### Q10: 请解释一下Qt的信号槽机制在项目中的应用？

**回答要点**:

- **解耦**: GameEngine和MainWindow通过信号槽通信，互不直接调用
- **异步处理**: 玩家响应闪避时，使用信号通知引擎
- **多对多**: 一个信号可以连接多个槽，一个槽可以接收多个信号
- **类型安全**: 编译时检查信号和槽的参数类型
- **示例代码**:

```cpp
// 信号定义
signals:
    void gameOver(const QString& winner);

// 连接信号
connect(engine, &GameEngine::gameOver, this, &MainWindow::onGameOver);

// 发射信号
emit gameOver("玩家");
```

***

## 附录：核心代码索引

| 功能模块  | 文件              | 关键函数                                               |
| ----- | --------------- | -------------------------------------------------- |
| 程序入口  | main.cpp        | main()                                             |
| UI构建  | mainwindow\.cpp | buildUI(), buildCharacterSelectUI(), buildGameUI() |
| 游戏引擎  | gameengine.cpp  | startGame(), startTurn(), advanceTurn()            |
| 出牌逻辑  | gameengine.cpp  | playerUseCard(), processKillCard()                 |
| AI系统  | gameengine.cpp  | performAiAction(), aiTakeTurn()                    |
| 曹操技能  | hero.cpp        | CaoCao::onTurnStart(), CaoCao::activateSkill()     |
| 刘备技能  | hero.cpp        | LiuBei::onCardPlayed(), LiuBei::onDamaged()        |
| 司马懿技能 | hero.cpp        | SimaYi::onCardPlayed()                             |
| 卡牌系统  | card.cpp        | KillCard::play(), TrickCard::play()                |
| 玩家管理  | Player.cpp      | receiveDamage(), restoreHealth(), drawCards()      |
| 动画系统  | mainwindow\.cpp | AnimOverlay::playCardFly(), spawnParticles()       |
| 粒子更新  | mainwindow\.cpp | onParticleTimer()                                  |

***

**文档版本**: v2.0\
**最后更新**: 2026年\
**适用对象**: 项目开发者、答辩准备者
