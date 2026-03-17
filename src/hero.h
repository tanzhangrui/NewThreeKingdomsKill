#pragma once
#include <QString>
#include <QStringList>
#include <vector>
#include <functional>

class Player;
class GameEngine;

// ============================================================
// SkillInfo —— 技能描述信息（UI展示用）
// ============================================================
struct SkillInfo {
    QString name;
    QString description;
    bool    active;        // true=主动技能  false=被动技能
    bool    available;     // 当前是否可用
};

// ============================================================
// Hero 抽象基类
// ============================================================
class Hero {
public:
    enum class HeroId { CaoCao, SimaYi, LiuBei };

    Hero(HeroId id, const QString& name, int maxHp,
         const QString& kingdom, const QString& themeColor)
        : m_id(id), m_name(name), m_maxHp(maxHp),
          m_kingdom(kingdom), m_themeColor(themeColor) {}

    virtual ~Hero() = default;

    // 纯虚 —— 每个武将自己实现技能
    virtual bool  activateSkill(int skillIndex, Player* self,
                                Player* target, GameEngine* engine) = 0;
    virtual std::vector<SkillInfo> getSkillInfos() const = 0;
    virtual QString flavorText() const = 0;

    // 通用访问器
    HeroId  id()         const { return m_id;         }
    QString name()       const { return m_name;        }
    int     maxHp()      const { return m_maxHp;       }
    QString kingdom()    const { return m_kingdom;     }
    QString themeColor() const { return m_themeColor;  }

    // 被动触发钩子（子类可选重写）
    virtual void onDamaged(Player* self, Player* attacker,
                           int dmg, GameEngine* engine) { Q_UNUSED(self); Q_UNUSED(attacker); Q_UNUSED(dmg); Q_UNUSED(engine); }
    virtual void onTurnStart(Player* self, GameEngine* engine) { Q_UNUSED(self); Q_UNUSED(engine); }
    virtual void onTurnEnd(Player* self, GameEngine* engine)   { Q_UNUSED(self); Q_UNUSED(engine); }
    virtual void onKillDodged(Player* self, Player* attacker,
                               GameEngine* engine)             { Q_UNUSED(self); Q_UNUSED(attacker); Q_UNUSED(engine); }

protected:
    HeroId  m_id;
    QString m_name;
    int     m_maxHp;
    QString m_kingdom;
    QString m_themeColor;
};

// ============================================================
// 曹操 —— 魏
//   技能1：奸雄（被动）— 
