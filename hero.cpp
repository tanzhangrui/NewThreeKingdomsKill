#include "Hero.h"
#include "Player.h"
#include "Card.h"
#include "gameengine.h"
#include <QRandomGenerator>
#include <QDebug>

Hero::Hero(const QString& heroName, int hp)
    : m_name(heroName), m_maxHp(hp) {}

// ============================================================
// 曹操 - 4个技能
// 技能0：战无不胜の斗鸡眼（被动）- 血量<=50%时对全体敌方造成1点精神伤害
// 技能1："呱"（被动）- 本轮不出牌，对指定玩家造成1点精神伤害
// 技能2："汪"（主动）- 指定玩家轮流出杀，最先放弃的受1点伤害
// 技能3：临终关怀（被动）- 死亡时对全体玩家造成1点精神伤害
// ============================================================
CaoCao::CaoCao() : Hero(QString::fromUtf8("曹操"), 4) {}

SkillInfo CaoCao::getSkillInfo(int index) const {
    switch (index) {
        case 0: return { QString::fromUtf8("战无不胜の斗鸡眼"),
                        QString::fromUtf8("【被动】血量≤50%时，对全体敌方造成1点精神伤害（回合开始时触发）"),
                        SkillType::Passive };
        case 1: return { QString::fromUtf8("呱"),
                        QString::fromUtf8("【被动】本回合未出牌时，对一名玩家造成1点精神伤害（回合结束时触发）"),
                        SkillType::Passive };
        case 2: return { QString::fromUtf8("汪"),
                        QString::fromUtf8("【主动】指定一名玩家，敌我双方轮流出【杀】，最先放弃的受1点伤害"),
                        SkillType::Active };
        case 3: return { QString::fromUtf8("临终关怀"),
                        QString::fromUtf8("【被动】阵亡时，对全体玩家造成1点精神伤害"),
                        SkillType::Passive };
        default: return { QString(), QString(), SkillType::Passive };
    }
}

void CaoCao::onTurnStart(Player* self, GameEngine* engine) {
    m_hasPlayedCardThisTurn = self->hasPlayedCardThisTurn();

    if (!self || !engine) return;

    int hp = self->getHealth();
    int maxHp = self->getMaxHealth();

    if (hp > 0 && hp * 2 <= maxHp && !m_doujiyanTriggered) {
        m_doujiyanTriggered = true;
        emit engine->logMessage(QString::fromUtf8("【%1·战无不胜の斗鸡眼】发动！对全体敌方造成1点精神伤害！")
                               .arg(self->getHero()->getName()));
        emit engine->skillEffectRequest(self->getHero()->getName(),
                                        QString::fromUtf8("战无不胜の斗鸡眼"),
                                        QString::fromUtf8("video/caocao_doujiyan.mp4"));
        
        for (int i = 0; i < engine->playerCount(); ++i) {
            Player* p = engine->playerAt(i);
            if (p && p != self && p->isAlive()) {
                p->receiveDamage(1);
                emit engine->animationRequest("damage", engine->playerIndex(self), i, "1");
            }
        }
        emit engine->gameStateChanged();
        engine->checkGameOver();
    }
}

void CaoCao::onTurnEnd(Player* self, GameEngine* engine) {
    if (!self || !engine) return;

    if (!self->hasPlayedCardThisTurn() && self->isAlive()) {
        emit engine->logMessage(QString::fromUtf8("【%1·呱】触发！请指定一名玩家造成1点精神伤害！")
                               .arg(self->getHero()->getName()));
        emit engine->skillEffectRequest(self->getHero()->getName(),
                                        QString::fromUtf8("呱"),
                                        QString::fromUtf8("video/caocao_gua.mp4"));
        engine->prepareTargetSelectionForSkill(1, self->getHero()->getName());
    }

    m_doujiyanTriggered = false;
}

void CaoCao::onDeath(Player* self, GameEngine* engine) {
    if (!self || !engine) return;

    emit engine->logMessage(QString::fromUtf8("【%1·临终关怀】发动！对全体玩家造成1点精神伤害！")
                           .arg(self->getHero()->getName()));
    emit engine->skillEffectRequest(self->getHero()->getName(),
                                    QString::fromUtf8("临终关怀"),
                                    QString::fromUtf8("video/caocao_linzhong.mp4"));
    
    for (int i = 0; i < engine->playerCount(); ++i) {
        Player* p = engine->playerAt(i);
        if (p && p != self && p->isAlive()) {
            p->receiveDamage(1);
            emit engine->animationRequest("damage", engine->playerIndex(self), i, "1");
        }
    }
    emit engine->gameStateChanged();
}

bool CaoCao::activateSkill(int skillIndex, Player* self, Player* target, GameEngine* engine) {
    if (!self || !engine) return false;

    if (skillIndex == 2) {
        if (!target) return false;

        emit engine->logMessage(QString::fromUtf8("【%1·汪】发动！与%2进行狗叫对决！")
                               .arg(self->getHero()->getName()).arg(target->getName()));
        emit engine->skillEffectRequest(self->getHero()->getName(),
                                        QString::fromUtf8("汪"),
                                        QString::fromUtf8("video/caocao_wang.mp4"));
        
        bool selfHasKill = false;
        bool targetHasKill = false;
        int selfKillIdx = -1;
        int targetKillIdx = -1;
        
        const auto& selfCards = self->getHandCards();
        for (int i = 0; i < selfCards.size(); ++i) {
            if (selfCards[i]->getType() == CardType::Kill) {
                selfHasKill = true;
                selfKillIdx = i;
                break;
            }
        }
        
        const auto& targetCards = target->getHandCards();
        for (int i = 0; i < targetCards.size(); ++i) {
            if (targetCards[i]->getType() == CardType::Kill) {
                targetHasKill = true;
                targetKillIdx = i;
                break;
            }
        }
        
        if (!selfHasKill && !targetHasKill) {
            emit engine->logMessage(QString::fromUtf8("双方都没有【杀】，平局！"));
            return true;
        }
        
        if (!selfHasKill) {
            emit engine->logMessage(QString::fromUtf8("%1没有【杀】，受到1点伤害！").arg(self->getName()));
            self->receiveDamage(1);
            emit engine->animationRequest("damage", engine->playerIndex(target), engine->playerIndex(self), "1");
        } else if (!targetHasKill) {
            emit engine->logMessage(QString::fromUtf8("%1没有【杀】，受到1点伤害！").arg(target->getName()));
            target->receiveDamage(1);
            emit engine->animationRequest("damage", engine->playerIndex(self), engine->playerIndex(target), "1");
            self->removeCardByIndex(selfKillIdx);
        } else {
            emit engine->logMessage(QString::fromUtf8("双方都有【杀】，各自消耗一张【杀】！"));
            self->removeCardByIndex(selfKillIdx);
            target->removeCardByIndex(targetKillIdx);
        }
        
        emit engine->gameStateChanged();
        engine->checkGameOver();
        return true;
    }
    
    return false;
}

// ============================================================
// 刘备 - 3个技能
// 技能0：自刎归天（被动）- 打出"杀"时，全员受1点伤害
// 技能1：无敌の二弟（被动）- 受攻击时可召唤二弟，对对方造成1点伤害
// 技能2：仁之剑义之剑（主动）- 选择一名玩家恢复1血，对另一玩家造成1点伤害
// ============================================================
LiuBei::LiuBei() : Hero(QString::fromUtf8("刘备"), 4) {}

SkillInfo LiuBei::getSkillInfo(int index) const {
    switch (index) {
        case 0: return { QString::fromUtf8("自刎归天"),
                        QString::fromUtf8("【被动】打出【杀】时，无论敌我全员都受到1点伤害"),
                        SkillType::Passive };
        case 1: return { QString::fromUtf8("无敌の二弟"),
                        QString::fromUtf8("【被动】受到攻击时，可召唤二弟对攻击者造成1点伤害"),
                        SkillType::Passive };
        case 2: return { QString::fromUtf8("仁之剑义之剑"),
                        QString::fromUtf8("【主动】选择一名玩家恢复1血，对另一玩家造成1点伤害"),
                        SkillType::Active };
        default: return { QString(), QString(), SkillType::Passive };
    }
}

void LiuBei::onCardPlayed(Player* self, const QString& cardName, Player* target, GameEngine* engine) {
    Q_UNUSED(target)
    if (!self || !engine) return;

    if (cardName == QString::fromUtf8("杀")) {
        emit engine->logMessage(QString::fromUtf8("【%1·自刎归天】发动！全员受到1点伤害！")
                               .arg(self->getHero()->getName()));
        emit engine->skillEffectRequest(self->getHero()->getName(),
                                        QString::fromUtf8("自刎归天"),
                                        QString::fromUtf8("video/liubei_ziwen.mp4"));
        
        for (int i = 0; i < engine->playerCount(); ++i) {
            Player* p = engine->playerAt(i);
            if (p && p->isAlive()) {
                p->receiveDamage(1);
                emit engine->animationRequest("damage", engine->playerIndex(self), i, "1");
            }
        }
        emit engine->gameStateChanged();
        engine->checkGameOver();
    }
}

void LiuBei::onDamaged(Player* self, int dmg, GameEngine* engine) {
    if (!self || !engine || dmg <= 0) return;

    m_canSummonErDi = true;
    emit engine->logMessage(QString::fromUtf8("【%1·无敌の二弟】可发动！是否召唤二弟反击？")
                           .arg(self->getHero()->getName()));
    engine->prepareUseSkillAfterDamage(1, self->getHero()->getName());
}

bool LiuBei::activateSkill(int skillIndex, Player* self, Player* target, GameEngine* engine) {
    if (!self || !engine) return false;

    if (skillIndex == 1) {
        if (!m_canSummonErDi) {
            emit engine->logMessage(QString::fromUtf8("【无敌の二弟】需要在受到攻击后才能发动！"));
            return false;
        }

        Player* attacker = nullptr;
        for (int i = 0; i < engine->playerCount(); ++i) {
            if (i != engine->playerIndex(self)) {
                Player* p = engine->playerAt(i);
                if (p && p->isAlive()) {
                    attacker = p;
                    break;
                }
            }
        }

        if (attacker) {
            emit engine->logMessage(QString::fromUtf8("【%1·无敌の二弟】发动！召唤二弟对%2造成1点伤害！")
                                   .arg(self->getHero()->getName()).arg(attacker->getName()));
            emit engine->skillEffectRequest(self->getHero()->getName(),
                                            QString::fromUtf8("无敌の二弟"),
                                            QString::fromUtf8("video/liubei_erdi.mp4"));
            attacker->receiveDamage(1);
            emit engine->animationRequest("damage", engine->playerIndex(self), engine->playerIndex(attacker), "1");
            emit engine->gameStateChanged();
            engine->checkGameOver();
        }

        m_canSummonErDi = false;
        return true;
    }

    if (skillIndex == 2) {
        if (!target) return false;

        emit engine->logMessage(QString::fromUtf8("【%1·仁之剑义之剑】已选择伤害目标：%2！请再选择恢复目标！")
                               .arg(self->getHero()->getName()).arg(target->getName()));
        engine->prepareSecondTargetForSkill(2, target);
        return true;
    }

    return false;
}

// ============================================================
// 司马懿 - 2个技能
// 技能0：天意化骨掌（被动）- 可无限出杀（第二张杀视为发动）
// 技能1：天意面瘫（被动）- 打出"桃"时，自身恢复2点，其余人恢复1点
// ============================================================
SimaYi::SimaYi() : Hero(QString::fromUtf8("司马懿"), 3) {}

SkillInfo SimaYi::getSkillInfo(int index) const {
    switch (index) {
        case 0: return { QString::fromUtf8("天意化骨掌"),
                        QString::fromUtf8("【被动】可无限出【杀】，每回合出第二张【杀】时自动发动"),
                        SkillType::Passive };
        case 1: return { QString::fromUtf8("天意面瘫"),
                        QString::fromUtf8("【被动】打出【桃】时，自身恢复2点，其余存活玩家恢复1点"),
                        SkillType::Passive };
        default: return { QString(), QString(), SkillType::Passive };
    }
}

bool SimaYi::activateSkill(int skillIndex, Player* self, Player* target, GameEngine* engine) {
    Q_UNUSED(skillIndex) Q_UNUSED(self) Q_UNUSED(target) Q_UNUSED(engine)
    return false;
}

void SimaYi::onTurnStart(Player* self, GameEngine* engine) {
    Q_UNUSED(self) Q_UNUSED(engine)
    m_killCountThisTurn = 0;
}

void SimaYi::onCardPlayed(Player* self, const QString& cardName, Player* target, GameEngine* engine) {
    Q_UNUSED(target)
    if (!self || !engine) return;

    if (cardName == QString::fromUtf8("杀")) {
        m_killCountThisTurn++;
        if (m_killCountThisTurn >= 2) {
            emit engine->logMessage(QString::fromUtf8("【%1·天意化骨掌】发动！无限出杀！")
                                   .arg(self->getHero()->getName()));
            emit engine->skillEffectRequest(self->getHero()->getName(),
                                            QString::fromUtf8("天意化骨掌"),
                                            QString::fromUtf8("video/simayi_huagu.mp4"));
        }
    }

    if (cardName == QString::fromUtf8("桃")) {
        emit engine->logMessage(QString::fromUtf8("【%1·天意面瘫】发动！自身恢复2点，其余人恢复1点！")
                               .arg(self->getHero()->getName()));
        emit engine->skillEffectRequest(self->getHero()->getName(),
                                        QString::fromUtf8("天意面瘫"),
                                        QString::fromUtf8("video/simayi_miantan.mp4"));

        self->restoreHealth(2);
        emit engine->animationRequest("heal", engine->playerIndex(self), engine->playerIndex(self), "2");

        for (int i = 0; i < engine->playerCount(); ++i) {
            Player* p = engine->playerAt(i);
            if (p && p != self && p->isAlive()) {
                p->restoreHealth(1);
                emit engine->animationRequest("heal", engine->playerIndex(self), i, "1");
            }
        }
        emit engine->gameStateChanged();
    }
}
