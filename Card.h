#ifndef CARD_H
#define CARD_H

#include <QString>
#include <QDebug>
#include <vector>
#include <memory>

class Player;
class GameEngine;

enum class CardType {
    Kill,
    Dodge,
    Peach,
    Trick
};

class Card {
public:
    explicit Card(const QString& n, const QString& d, CardType t);
    virtual ~Card() = default;

    virtual bool play(Player* from, Player* to, GameEngine* engine);
    virtual bool canRespondWithDodge() const { return false; }
    virtual bool requiresTarget() const { return true; }

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

class KillCard : public Card {
public:
    KillCard();
    bool play(Player* from, Player* to, GameEngine* engine) override;
    bool canRespondWithDodge() const override { return true; }
    QString colorHex() const override { return "#C0202A"; }
    QString iconText() const override { return QString::fromUtf8("杀"); }
};

class DodgeCard : public Card {
public:
    DodgeCard();
    bool play(Player* from, Player* to, GameEngine* engine) override;
    bool requiresTarget() const override { return false; }
    QString colorHex() const override { return "#2080C0"; }
    QString iconText() const override { return QString::fromUtf8("闪"); }
};

class PeachCard : public Card {
public:
    PeachCard();
    bool play(Player* from, Player* to, GameEngine* engine) override;
    QString colorHex() const override { return "#DD6090"; }
    QString iconText() const override { return QString::fromUtf8("桃"); }
};

class TrickCard : public Card {
public:
    enum TrickSubType { 
        StealCard, 
        DiscardCard, 
        JumpTigerShark,
        TianyiInvasion
    };
    explicit TrickCard(TrickSubType subType = StealCard);
    bool play(Player* from, Player* to, GameEngine* engine) override;
    bool requiresTarget() const override;
    QString colorHex() const override { return "#A055D0"; }
    QString getVideoPath() const;
    
private:
    TrickSubType subType;
};

Card* createRandomCard();

#endif // CARD_H
