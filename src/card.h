#pragma once
#include <QString>
#include <QPixmap>
#include <memory>

class Player;

// ============================================================
// Card 抽象基类  ——  所有卡牌的根
// ============================================================
class Card {
public:
    enum class Suit { Spade, Heart, Club, Diamond, None };
    enum class Type { Kill, Dodge, Peach, FireKill, ThunderKill,
                      Duel, BarleyWine, PeachGarden, ArrowBarrage,
                      Dismantle, Steal, Unknown };

    Card(Type type, Suit suit, int number, const QString& name)
        : m_type(type), m_suit(suit), m_number(number), m_name(name) {}

    virtual ~Card() = default;

    // 纯虚函数 —— 多态核心
    virtual bool play(Player* from, Player* to) = 0;
    virtual QString description() const = 0;
    virtual QString colorStyle() const = 0;   // 卡牌颜色主题

    // 访问器
    Type    type()   const { return m_type;   }
    Suit    suit()   const { return m_suit;   }
    int     number() const { return m_number; }
    QString name()   const { return m_name;   }
    QString suitSymbol() const;
    bool    isRed()  const { return m_suit==Suit::Heart || m_suit==Suit::Diamond; }

protected:
    Type    m_type;
    Suit    m_suit;
    int     m_number;
    QString m_name;
};

// ============================================================
// 基础牌
// ============================================================
class KillCard : public Card {
public:
    KillCard(Suit suit, int number)
        : Card(Type::Kill, suit, number, QStringLiteral("杀")) {}
    bool    play(Player* from, Player* to) override;
    QString description() const override { return QStringLiteral("对目标造成1点伤害"); }
    QString colorStyle()  const override { return QStringLiteral("kill"); }
};

class FireKillCard : public Card {
public:
    FireKillCard(Suit suit, int number)
        : Card(Type::FireKill, suit, number, QStringLiteral("火杀")) {}
    bool    play(Player* from, Player* to) override;
    QString description() const override { return QStringLiteral("火属性伤害，无法被闪避"); }
    QString colorStyle()  const override { return QStringLiteral("firekill"); }
};

class ThunderKillCard : public Card {
public:
    ThunderKillCard(Suit suit, int number)
        : Card(Type::ThunderKill, suit, number, QStringLiteral("雷杀")) {}
    bool    play(Player* from, Player* to) override;
    QString description() const override { return QStringLiteral("雷属性伤害"); }
    QString colorStyle()  const override { return QStringLiteral("thunderkill"); }
};

class DodgeCard : public Card {
public:
    DodgeCard(Suit suit, int number)
        : Card(Type::Dodge, suit, number, QStringLiteral("闪")) {}
    bool    play(Player* from, Player* to) override;
    QString description() const override { return QStringLiteral("抵消一张「杀」或「雷杀」"); }
    QString colorStyle()  const override { return QStringLiteral("dodge"); }
};

class PeachCard : public Card {
public:
    PeachCard(Suit suit, int number)
        : Card(Type::Peach, suit, number, QStringLiteral("桃")) {}
    bool    play(Player* from, Player* to) override;
    QString description() const override { return QStringLiteral("恢复1点体力"); }
    QString colorStyle()  const override { return QStringLiteral("peach"); }
};

// ============================================================
// 锦囊牌
// ============================================================
class DuelCard : public Card {
public:
    DuelCard(Suit suit, int number)
        : Card(Type::Duel, suit, number, QStringLiteral("决斗")) {}
    bool    play(Player* from, Player* to) override;
    QString description() const override { return QStringLiteral("与目标轮流出杀，先不出者受1伤害"); }
    QString colorStyle()  const override { return QStringLiteral("trick"); }
};

class ArrowBarrageCard : public Card {
public:
    ArrowBarrageCard(Suit suit, int number)
        : Card(Type::ArrowBarrage, suit, number, QStringLiteral("万箭齐发")) {}
    bool    play(Player* from, Player* to) override;
    QString description() const override { return QStringLiteral("所有其他玩家各出一张闪，否则受1伤害"); }
    QString colorStyle()  const override { return QStringLiteral("trick"); }
};

class PeachGardenCard : public Card {
public:
    PeachGardenCard(Suit suit, int number)
        : Card(Type::PeachGarden, suit, number, QStringLiteral("桃园结义")) {}
    bool    play(Player* from, Player* to) override;
    QString description() const override { return QStringLiteral("所有玩家各回复1点体力"); }
    QString colorStyle()  const override { return QStringLiteral("trick"); }
};

class DismantleCard : public Card {
public:
    DismantleCard(Suit suit, int number)
        : Card(Type::Dismantle, suit, number, QStringLiteral("过河拆桥")) {}
    bool    play(Player* from, Player* to) override;
    QString description() const override { return QStringLiteral("弃置目标一张手牌"); }
    QString colorStyle()  const override { return QStringLiteral("trick"); }
};

// ============================================================
// 工厂：生成完整牌堆
// ============================================================
namespace CardFactory {
    std::vector<std::unique_ptr<Card>> createFullDeck();
}

