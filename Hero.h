#ifndef HERO_H
#define HERO_H

#include <QString>
#include <QStringList>
#include <QDebug>

class Player;

// ============================================================
// Hero  ——  武将抽象基类
//   每位武将有两个技能槽（可扩展至更多）
// ============================================================
class Hero {
public:
    // 技能描述结构体（UI展示用）
    struct SkillInfo {
        QString name;
        QString desc;
        bool    isActive;   // true=主动  false=被动
    };

    explicit Hero(const QString& heroName,
                  const QString& skill1Name, const QString& skill1Desc,
                  const QString& skill2Name, const QString& skill2Desc,
                  int maxHp);
    virtual ~Hero() = default;

    // 纯虚：激活技能（skillIndex: 0 或 1；可扩展到更多）
    virtual bool activateSkill(int skillIndex, Player* self, Player* target) = 0;

    // 被动触发钩子（子类可选重写）
    virtual void onDamaged(Player* /*self*/, int /*dmg*/) {}
    virtual void onTurnStart(Player* /*self*/) {}
    virtual void onTurnEnd(Player* /*self*/) {}

    QString getName()           const { return name; }
    QString getSkillName(int i) const { return (i == 0) ? sName1 : sName2; }
    QString getSkillDesc(int i) const { return (i == 0) ? sDesc1 : sDesc2; }
    int     getMaxHp()          const { return maxHp; }
    int     getSkillCount()     const { return 2; }  // 预留扩展

    // 获取所有技能信息（UI用）
    QList<SkillInfo> getAllSkills() const {
        return { {sName1, sDesc1, true}, {sName2, sDesc2, true} };
    }

    // 武将阵营颜色主题
    virtual QString kingdomColor() const { return "#8B1A1A"; }
    virtual QString portraitChar() const { return name.left(1); }

protected:
    QString name;
    QString sName1, sDesc1;
    QString sName2, sDesc2;
    int     maxHp;
};

// ============================================================
// 曹操  ——  魏 / 血量4
//   技能0「奸雄」：立即摸1张牌（被动触发演示）
//   技能1「护驾」：令目标出【闪】，否则受1伤
// ============================================================
class CaoCao : public Hero {
public:
    CaoCao();
    bool activateSkill(int skillIndex, Player* self, Player* target) override;
    QString kingdomColor() const override { return "#8B1A1A"; }
    QString portraitChar() const override { return "\u64CD"; }
};

// ============================================================
// 司马懿  ——  魏 / 血量3
//   技能0「反馈」：摸1张牌，并对目标造成1伤
//   技能1「鬼才」：消耗1张手牌，令目标弃1张手牌
// ============================================================
class SimaYi : public Hero {
public:
    SimaYi();
    bool activateSkill(int skillIndex, Player* self, Player* target) override;
    QString kingdomColor() const override { return "#1A2A6B"; }
    QString portraitChar() const override { return "\u61FF"; }
};

// ============================================================
// 刘备  ——  蜀 / 血量4
//   技能0「仁德」：给目标1张手牌，回复自身1血
//   技能1「激将」：对目标造成1伤，令其弃1张牌
// ============================================================
class LiuBei : public Hero {
public:
    LiuBei();
    bool activateSkill(int skillIndex, Player* self, Player* target) override;
    QString kingdomColor() const override { return "#1A5C1A"; }
    QString portraitChar() const override { return "\u5907"; }
};

#endif // HERO_H
