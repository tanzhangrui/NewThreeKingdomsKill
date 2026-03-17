#ifndef CARD_H
#define CARD_H

#include <QString>
#include <QDebug>
#include <vector>
#include <memory>

class Player;

enum class CardType {
    Kill,
    Dodge,
    Peach,
    Trick
};

// ============================================================
// Card  ——  抽象基类（纯虚 play，虚析构）
// ============================================================
class Card {
public:
    explicit Card(const QString& n, const QString& d, CardType t);
    virtual ~Card() = default;

    // 纯虚函数：多态核心，所有子类必须实现
    virtual bool play(Player* from, Player* to) = 0;

    // 卡牌外观（子类可重写以定制颜色主题）
    virtual QString colorHex() const { return "#C02020"; }
    virtual QString iconText() const { return name; }

    QString  getName()        const { return name; }
    QString  getDescription() const { return description; }
    CardType getType()        const { return type; }

protected:
    QString  name;
    QString  description;
    CardType type;
};

// ============================================================
// 具体卡牌
// ============================================================
class KillCard : public Card {
public:
    KillCard();
    bool    play(Player* from, Player* to) override;
    QString colorHex() const override { return "#C0202A"; }
    QString iconText()  const override { return "\u6740"; }
};

class DodgeCard : public Card {
public:
    DodgeCard();
    bool    play(Player* from, Player* to) override;
    QString colorHex() const override { return "#2080C0"; }
    QString iconText()  const override { return "\u95EA"; }
};

class PeachCard : public Card {
public:
    PeachCard();
    bool    play(Player* from, Player* to) override;
    QString colorHex() const override { return "#DD6090"; }
    QString iconText()  const override { return "\u6843"; }
};

// ---- 锦囊基类（中间层，体现继承层次）----
class TrickCard : public Card {
public:
    enum TrickSubType { StealCard, DiscardCard };
    explicit TrickCard(TrickSubType subType = StealCard);
    bool    play(Player* from, Player* to) override;
    QString colorHex() const override { return "#A055D0"; }
private:
    TrickSubType subType;
};

// ============================================================
// 工厂函数：随机创建一张牌
// ============================================================
Card* createRandomCard();

#endif // CARD_H
