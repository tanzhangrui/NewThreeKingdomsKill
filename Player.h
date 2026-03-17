#ifndef PLAYER_H
#define PLAYER_H

#include <QString>
#include <QList>
#include "Card.h"

class Hero;

// ============================================================
// Player  ——  玩家类（组合 Hero* 与 Card* 列表）
// ============================================================
class Player {
public:
    explicit Player(const QString& n, int maxHp = 4);
    ~Player();

    // 基本属性
    QString getName()      const { return m_name; }
    int     getHealth()    const { return m_hp; }
    int     getMaxHealth() const { return m_maxHp; }
    bool    isAlive()      const { return m_hp > 0; }

    // 武将（组合关系）
    Hero*   getHero()      const { return m_hero; }
    void    setHero(Hero* h);

    // 手牌
    const QList<Card*>& getHandCards() const { return m_hand; }
    void addCard(Card* card);
    bool removeCardByIndex(int index, bool deleteCard = true);
    bool removeCardByPointer(Card* card, bool deleteCard = true);

    // 游戏行为
    void receiveDamage(int amount);
    void restoreHealth(int amount);
    void drawCards(int count);

    // 回合状态
    bool hasUsedKillThisTurn()  const { return m_killUsed;  }
    void setKillUsed(bool v)          { m_killUsed  = v;    }
    bool hasUsedSkillThisTurn() const { return m_skillUsed; }
    void setSkillUsed(bool v)         { m_skillUsed = v;    }
    void resetTurnState();

private:
    QString      m_name;
    int          m_hp;
    int          m_maxHp;
    QList<Card*> m_hand;
    Hero*        m_hero     = nullptr;
    bool         m_killUsed  = false;
    bool         m_skillUsed = false;
};

#endif // PLAYER_H
