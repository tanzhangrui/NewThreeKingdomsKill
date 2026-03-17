#ifndef PLAYER_H
#define PLAYER_H

#include <QString>
#include <QList>
#include "Card.h"

class Hero;
class GameEngine;

class Player {
public:
    explicit Player(const QString& n, int maxHp = 4);
    ~Player();

    QString getName()      const { return m_name; }
    int     getHealth()    const { return m_hp; }
    int     getMaxHealth() const { return m_maxHp; }
    bool    isAlive()      const { return m_hp > 0; }

    Hero*   getHero()      const { return m_hero; }
    void    setHero(Hero* h);
    
    QString getDisplayName() const;

    const QList<Card*>& getHandCards() const { return m_hand; }
    void addCard(Card* card);
    bool removeCardByIndex(int index, bool deleteCard = true);
    bool removeCardByPointer(Card* card, bool deleteCard = true);
    
    int findDodgeCard() const;
    int findKillCard() const;

    void receiveDamage(int amount);
    void restoreHealth(int amount);
    void drawCards(int count);

    bool hasUsedKillThisTurn()  const { return m_killUsed; }
    void setKillUsed(bool v)          { m_killUsed  = v; }
    bool hasUsedSkillThisTurn() const { return m_skillUsed; }
    void setSkillUsed(bool v)         { m_skillUsed = v; }
    void resetTurnState();
    
    void setPlayedCardThisTurn(bool v) { m_playedCardThisTurn = v; }
    bool hasPlayedCardThisTurn() const { return m_playedCardThisTurn; }

private:
    QString      m_name;
    int          m_hp;
    int          m_maxHp;
    QList<Card*> m_hand;
    Hero*        m_hero     = nullptr;
    bool         m_killUsed  = false;
    bool         m_skillUsed = false;
    bool         m_playedCardThisTurn = false;
};

#endif // PLAYER_H
