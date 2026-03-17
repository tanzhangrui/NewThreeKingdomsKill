#ifndef HERO_H
#define HERO_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QDebug>
#include <functional>

class Player;
class GameEngine;

enum class SkillType {
    Active,
    Passive
};

struct SkillInfo {
    QString name;
    QString desc;
    SkillType type;
    bool isActive() const { return type == SkillType::Active; }
};

class Hero {
public:
    explicit Hero(const QString& heroName, int maxHp);
    virtual ~Hero() = default;

    virtual bool activateSkill(int skillIndex, Player* self, Player* target, GameEngine* engine) = 0;
    
    virtual void onDamaged(Player* self, int dmg, GameEngine* engine) { Q_UNUSED(self) Q_UNUSED(dmg) Q_UNUSED(engine) }
    virtual void onTurnStart(Player* self, GameEngine* engine) { Q_UNUSED(self) Q_UNUSED(engine) }
    virtual void onTurnEnd(Player* self, GameEngine* engine) { Q_UNUSED(self) Q_UNUSED(engine) }
    virtual void onCardPlayed(Player* self, const QString& cardName, Player* target, GameEngine* engine) { 
        Q_UNUSED(self) Q_UNUSED(cardName) Q_UNUSED(target) Q_UNUSED(engine) 
    }
    virtual void onDeath(Player* self, GameEngine* engine) { Q_UNUSED(self) Q_UNUSED(engine) }

    QString getName() const { return m_name; }
    int     getMaxHp() const { return m_maxHp; }
    virtual int getSkillCount() const = 0;
    virtual SkillInfo getSkillInfo(int index) const = 0;
    QString getSkillName(int i) const { return getSkillInfo(i).name; }
    QString getSkillDesc(int i) const { return getSkillInfo(i).desc; }
    
    virtual QString kingdomColor() const { return "#8B1A1A"; }
    virtual QString portraitChar() const { return m_name.left(1); }
    virtual QString getDisplayName() const { return QString::fromUtf8("邪·") + m_name; }

protected:
    QString m_name;
    int     m_maxHp;
};

class CaoCao : public Hero {
public:
    CaoCao();
    
    bool activateSkill(int skillIndex, Player* self, Player* target, GameEngine* engine) override;
    
    void onTurnStart(Player* self, GameEngine* engine) override;
    void onTurnEnd(Player* self, GameEngine* engine) override;
    void onDeath(Player* self, GameEngine* engine) override;
    
    int getSkillCount() const override { return 4; }
    SkillInfo getSkillInfo(int index) const override;
    QString kingdomColor() const override { return "#8B1A1A"; }
    QString portraitChar() const override { return QString::fromUtf8("曹"); }

private:
    bool m_hasPlayedCardThisTurn = false;
    bool m_doujiyanTriggered = false;
};

class LiuBei : public Hero {
public:
    LiuBei();
    
    bool activateSkill(int skillIndex, Player* self, Player* target, GameEngine* engine) override;
    
    void onDamaged(Player* self, int dmg, GameEngine* engine) override;
    void onCardPlayed(Player* self, const QString& cardName, Player* target, GameEngine* engine) override;
    
    int getSkillCount() const override { return 3; }
    SkillInfo getSkillInfo(int index) const override;
    QString kingdomColor() const override { return "#1A5C1A"; }
    QString portraitChar() const override { return QString::fromUtf8("刘"); }

private:
    bool m_canSummonErDi = false;
};

class SimaYi : public Hero {
public:
    SimaYi();
    
    bool activateSkill(int skillIndex, Player* self, Player* target, GameEngine* engine) override;
    
    void onTurnStart(Player* self, GameEngine* engine) override;
    void onCardPlayed(Player* self, const QString& cardName, Player* target, GameEngine* engine) override;
    
    int getSkillCount() const override { return 2; }
    SkillInfo getSkillInfo(int index) const override;
    QString kingdomColor() const override { return "#1A2A6B"; }
    QString portraitChar() const override { return QString::fromUtf8("司"); }

private:
    int m_killCountThisTurn = 0;
};

#endif // HERO_H
