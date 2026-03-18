#include "gameengine.h"
#include "Hero.h"
#include "Card.h"
#include <QRandomGenerator>
#include <QDebug>

GameEngine::GameEngine(QObject* parent)
    : QObject(parent) {
    m_aiTimer = new QTimer(this);
    m_aiTimer->setSingleShot(true);
    connect(m_aiTimer, &QTimer::timeout, this, &GameEngine::aiTakeTurn);
    
    m_dodgeTimer = new QTimer(this);
    m_dodgeTimer->setSingleShot(true);
    connect(m_dodgeTimer, &QTimer::timeout, this, &GameEngine::onDodgeTimeout);
}

GameEngine::~GameEngine() {
}

int GameEngine::playerIndex(Player* p) const {
    return m_players.indexOf(p);
}

void GameEngine::startGame(Player* p0, Player* p1, Player* p2) {
    m_players.clear();
    m_players << p0 << p1 << p2;
    m_curIdx  = 0;
    m_running = true;

    for (Player* p : m_players)
        p->drawCards(4);

    emit logMessage(QString::fromUtf8("══════ 天意侵蚀·三方对弈 开始！══════"));
    emit logMessage(QString::fromUtf8("玩家：【%1】 vs 邪·%2 vs 邪·%3")
                    .arg(p0->getHero() ? p0->getHero()->getDisplayName() : "?")
                    .arg(p1->getHero() ? p1->getHero()->getName() : "?")
                    .arg(p2->getHero() ? p2->getHero()->getName() : "?"));
    
    emit showCenterEffect(QString::fromUtf8("gamestart"), QString::fromUtf8("天意侵蚀"), 2000);
    startTurn();
}

void GameEngine::endGame() {
    m_running = false;
    m_aiTimer->stop();
    m_dodgeTimer->stop();
    m_waitingForDodgeResponse = false;
    emit logMessage(QString::fromUtf8("══════ 游戏结束 ══════"));
}

void GameEngine::startTurn() {
    if (!m_running) return;
    Player* cur = currentPlayer();
    if (!cur || !cur->isAlive()) { advanceTurn(); return; }

    cur->resetTurnState();
    cur->drawCards(2);
    
    triggerHeroTurnStart(cur);
    
    emit animationRequest("turn", m_curIdx, -1, "");
    emit logMessage(QString::fromUtf8("【 %1 的回合 】摸2张牌")
                    .arg(cur->getDisplayName()));
    emit showCenterEffect(QString::fromUtf8("turn"), cur->getDisplayName(), 1500);
    emit gameStateChanged();

    if (!isHumanTurn()) {
        m_aiTimer->start(1500);
    }
}

void GameEngine::advanceTurn() {
    if (!m_running) return;
    
    Player* cur = currentPlayer();
    if (cur) {
        triggerHeroTurnEnd(cur);
    }
    
    int tried = 0;
    do {
        m_curIdx = (m_curIdx + 1) % m_players.size();
        ++tried;
        if (tried > m_players.size()) { endGame(); return; }
    } while (!m_players[m_curIdx]->isAlive());
    startTurn();
}

void GameEngine::triggerHeroTurnStart(Player* p) {
    if (p && p->getHero()) {
        p->getHero()->onTurnStart(p, this);
    }
}

void GameEngine::triggerHeroTurnEnd(Player* p) {
    if (p && p->getHero()) {
        p->getHero()->onTurnEnd(p, this);
    }
}

void GameEngine::triggerHeroCardPlayed(Player* p, const QString& cardName, Player* target) {
    if (p && p->getHero()) {
        p->getHero()->onCardPlayed(p, cardName, target, this);
    }
}

void GameEngine::triggerHeroDamaged(Player* p, int dmg) {
    if (p && p->getHero()) {
        p->getHero()->onDamaged(p, dmg, this);
    }
}

void GameEngine::triggerHeroDeath(Player* p) {
    if (p && p->getHero()) {
        p->getHero()->onDeath(p, this);
    }
}

void GameEngine::processKillCard(Player* from, Player* to) {
    m_pendingKillFrom = from;
    m_pendingKillTo = to;
    
    int toIdx = playerIndex(to);
    
    if (toIdx == 0) {
        int dodgeIdx = to->findDodgeCard();
        if (dodgeIdx >= 0) {
            m_waitingForDodgeResponse = true;
            m_dodgeResponseTarget = toIdx;
            m_dodgeCardIndex = dodgeIdx;
            
            emit logMessage(QString::fromUtf8("%1对你使用【杀】！你有【闪】，是否使用？").arg(from->getDisplayName()));
            emit showCenterEffect(QString::fromUtf8("kill"), QString::fromUtf8("杀"), 2000);
            emit requestDodgeResponse(toIdx, 5);
            m_dodgeTimer->start(5000);
            return;
        }
    }
    
    emit showCenterEffect(QString::fromUtf8("kill"), QString::fromUtf8("杀"), 2000);
    to->receiveDamage(1);
    emit animationRequest("damage", playerIndex(from), toIdx, "1");
    triggerHeroDamaged(to, 1);
    
    if (!to->isAlive()) {
        triggerHeroDeath(to);
    }
    
    emit gameStateChanged();
    checkGameOver();
}

void GameEngine::playerRespondDodge(bool useDodge) {
    if (!m_waitingForDodgeResponse) return;
    
    m_dodgeTimer->stop();
    m_waitingForDodgeResponse = false;
    
    if (useDodge && m_pendingKillTo && m_dodgeCardIndex >= 0) {
        m_pendingKillTo->removeCardByIndex(m_dodgeCardIndex);
        emit logMessage(QString::fromUtf8("%1使用【闪】抵消了伤害！").arg(m_pendingKillTo->getDisplayName()));
        emit showCenterEffect(QString::fromUtf8("dodge"), QString::fromUtf8("闪"), 1500);
        emit animationRequest("card", playerIndex(m_pendingKillTo), playerIndex(m_pendingKillTo), QString::fromUtf8("闪"));
    } else if (m_pendingKillTo) {
        m_pendingKillTo->receiveDamage(1);
        emit animationRequest("damage", playerIndex(m_pendingKillFrom), playerIndex(m_pendingKillTo), "1");
        triggerHeroDamaged(m_pendingKillTo, 1);
        
        if (!m_pendingKillTo->isAlive()) {
            triggerHeroDeath(m_pendingKillTo);
        }
        
        checkGameOver();
    }
    
    m_pendingKillFrom = nullptr;
    m_pendingKillTo = nullptr;
    m_dodgeCardIndex = -1;
    emit gameStateChanged();
}

void GameEngine::onDodgeTimeout() {
    if (m_waitingForDodgeResponse) {
        emit logMessage(QString::fromUtf8("超时未出【闪】！"));
        playerRespondDodge(false);
    }
}

void GameEngine::playerUseCard(int cardIndex, int targetPlayerIndex) {
    if (!m_running || !isHumanTurn()) return;
    Player* self   = currentPlayer();
    Player* target = m_players.value(targetPlayerIndex, nullptr);
    if (!self || !target) return;
    if (cardIndex < 0 || cardIndex >= self->getHandCards().size()) return;

    Card* card = self->getHandCards().at(cardIndex);
    if (!card) return;

    if (card->getType() == CardType::Kill) {
        Hero* hero = self->getHero();
        bool canUseKill = true;
        if (hero && QString::fromUtf8("司马懿") == hero->getName()) {
            canUseKill = true;
        } else if (self->hasUsedKillThisTurn()) {
            emit logMessage(QString::fromUtf8("本回合已使用过【杀】！"));
            return;
        }
    }
    
    if (card->getType() == CardType::Peach) target = self;
    if (card->getType() == CardType::Dodge) {
        emit logMessage(QString::fromUtf8("【闪】只能应对他人的【杀】，不能主动使用"));
        return;
    }

    bool ok = card->play(self, target, this);
    if (ok) {
        if (card->getType() == CardType::Kill) {
            self->setKillUsed(true);
        }
        
        self->setPlayedCardThisTurn(true);
        
        emit animationRequest("card", 0, targetPlayerIndex, card->getName());
        emit logMessage(self->getDisplayName() + QString::fromUtf8(" 使用【") + card->getName()
                        + QString::fromUtf8("】→ ") + target->getDisplayName());
        
        emit showCenterEffect(QString::fromUtf8("card"), card->getName(), 2000);
        
        self->removeCardByIndex(cardIndex);
        
        triggerHeroCardPlayed(self, card->getName(), target);
        
        emit gameStateChanged();
        checkGameOver();
    } else {
        emit logMessage(QString::fromUtf8("使用失败：") + card->getName());
    }
}

void GameEngine::playerUseSkill(int skillIndex, int targetPlayerIndex) {
    if (!m_running || !isHumanTurn()) return;
    Player* self   = currentPlayer();
    Player* target = m_players.value(targetPlayerIndex, nullptr);
    if (!self || !self->getHero()) return;
    
    SkillInfo info = self->getHero()->getSkillInfo(skillIndex);
    if (!info.isActive()) {
        emit logMessage(QString::fromUtf8("【%1】是被动技能，无法主动发动！").arg(info.name));
        return;
    }
    
    if (self->hasUsedSkillThisTurn()) {
        emit logMessage(QString::fromUtf8("本回合已发动过技能！"));
        return;
    }

    bool ok = self->getHero()->activateSkill(skillIndex, self, target, this);
    if (ok) {
        self->setSkillUsed(true);
        emit animationRequest("skill", 0, targetPlayerIndex, info.name);
        emit logMessage(self->getDisplayName() + QString::fromUtf8(" 发动【") + info.name
                        + QString::fromUtf8("】→ ")
                        + (target ? target->getDisplayName() : QString::fromUtf8("自身")));
        emit showCenterEffect(QString::fromUtf8("skill"), info.name, 2000);
        emit gameStateChanged();
        checkGameOver();
    } else {
        emit logMessage(QString::fromUtf8("技能发动失败（条件不满足）"));
    }
}

void GameEngine::playerEndTurn() {
    if (!m_running || !isHumanTurn()) return;
    emit logMessage(currentPlayer()->getDisplayName() + QString::fromUtf8(" 结束回合"));
    advanceTurn();
}

void GameEngine::aiTakeTurn() {
    if (!m_running || isHumanTurn()) return;
    performAiAction();
}

void GameEngine::performAiAction() {
    Player* self = currentPlayer();
    if (!self || !self->isAlive()) { advanceTurn(); return; }

    QVector<Player*> enemies;
    for (int i = 0; i < m_players.size(); ++i) {
        if (i != m_curIdx && m_players[i]->isAlive())
            enemies << m_players[i];
    }
    if (enemies.isEmpty()) { checkGameOver(); return; }

    Player* weakest = enemies[0];
    for (Player* e : enemies)
        if (e->getHealth() < weakest->getHealth()) weakest = e;

    if (self->getHealth() <= self->getMaxHealth() - 1) {
        const auto& hand = self->getHandCards();
        for (int i = 0; i < hand.size(); ++i) {
            if (hand[i]->getType() == CardType::Peach) {
                hand[i]->play(self, self, this);
                emit logMessage(self->getDisplayName() + QString::fromUtf8(" 使用【桃】回复体力"));
                emit showCenterEffect(QString::fromUtf8("peach"), QString::fromUtf8("桃"), 1500);
                self->removeCardByIndex(i);
                triggerHeroCardPlayed(self, QString::fromUtf8("桃"), self);
                emit animationRequest("heal", m_curIdx, m_curIdx, "1");
                emit gameStateChanged();
                break;
            }
        }
    }

    Hero* hero = self->getHero();
    bool canUseMultipleKills = hero && QString::fromUtf8("司马懿") == hero->getName();
    
    if (!self->hasUsedKillThisTurn() || canUseMultipleKills) {
        const auto& hand = self->getHandCards();
        for (int i = 0; i < hand.size(); ++i) {
            if (hand[i]->getType() == CardType::Kill) {
                if (!canUseMultipleKills && self->hasUsedKillThisTurn()) break;
                
                int tIdx = m_players.indexOf(weakest);
                
                int dodgeIdx = weakest->findDodgeCard();
                if (dodgeIdx >= 0 && tIdx == 0) {
                    emit logMessage(self->getDisplayName() + QString::fromUtf8(" 使用【杀】攻击 ")
                                    + weakest->getDisplayName());
                    emit showCenterEffect(QString::fromUtf8("kill"), QString::fromUtf8("杀"), 2000);
                    emit animationRequest("card", m_curIdx, tIdx, QString::fromUtf8("杀"));
                    
                    weakest->removeCardByIndex(dodgeIdx);
                    emit logMessage(weakest->getDisplayName() + QString::fromUtf8(" 使用【闪】抵消！"));
                    emit showCenterEffect(QString::fromUtf8("dodge"), QString::fromUtf8("闪"), 1500);
                } else {
                    hand[i]->play(self, weakest, this);
                    emit logMessage(self->getDisplayName() + QString::fromUtf8(" 使用【杀】攻击 ")
                                    + weakest->getDisplayName());
                    emit showCenterEffect(QString::fromUtf8("kill"), QString::fromUtf8("杀"), 2000);
                    emit animationRequest("card", m_curIdx, tIdx, QString::fromUtf8("杀"));
                    emit animationRequest("damage", m_curIdx, tIdx, "1");
                    triggerHeroCardPlayed(self, QString::fromUtf8("杀"), weakest);
                    triggerHeroDamaged(weakest, 1);
                    
                    if (!weakest->isAlive()) {
                        triggerHeroDeath(weakest);
                    }
                }
                
                self->setKillUsed(true);
                self->removeCardByIndex(i);
                emit gameStateChanged();
                checkGameOver();
                if (!m_running) return;
                break;
            }
        }
    }

    if (!self->hasUsedSkillThisTurn() && self->getHero()) {
        Hero* h = self->getHero();
        for (int skillIdx = 0; skillIdx < h->getSkillCount(); ++skillIdx) {
            SkillInfo info = h->getSkillInfo(skillIdx);
            if (info.isActive() && QRandomGenerator::global()->bounded(100) < 50) {
                int tIdx = m_players.indexOf(weakest);
                bool ok = h->activateSkill(skillIdx, self, weakest, this);
                if (ok) {
                    self->setSkillUsed(true);
                    emit logMessage(self->getDisplayName() + QString::fromUtf8(" 发动【") + info.name + QString::fromUtf8("】"));
                    emit showCenterEffect(QString::fromUtf8("skill"), info.name, 2000);
                    emit animationRequest("skill", m_curIdx, tIdx, info.name);
                    emit gameStateChanged();
                    checkGameOver();
                    if (!m_running) return;
                    break;
                }
            }
        }
    }

    {
        const auto& hand = self->getHandCards();
        for (int i = 0; i < hand.size(); ++i) {
            if (hand[i]->getType() == CardType::Trick) {
                TrickCard* trick = dynamic_cast<TrickCard*>(hand[i]);
                int tIdx = -1;
                if (trick && !trick->requiresTarget()) {
                    tIdx = m_curIdx;
                } else {
                    tIdx = m_players.indexOf(weakest);
                }
                
                bool ok  = hand[i]->play(self, weakest, this);
                if (ok) {
                    emit logMessage(self->getDisplayName() + QString::fromUtf8(" 使用【")
                                    + hand[i]->getName() + QString::fromUtf8("】"));
                    emit showCenterEffect(QString::fromUtf8("trick"), hand[i]->getName(), 2000);
                    self->removeCardByIndex(i);
                    emit animationRequest("card", m_curIdx, tIdx, QString::fromUtf8("锦囊"));
                    emit gameStateChanged();
                    checkGameOver();
                    if (!m_running) return;
                }
                break;
            }
        }
    }

    emit logMessage(self->getDisplayName() + QString::fromUtf8(" 结束回合"));
    advanceTurn();
}

void GameEngine::checkGameOver() {
    if (!m_running) return;
    QVector<Player*> alive;
    for (Player* p : m_players)
        if (p->isAlive()) alive << p;

    if (alive.size() <= 1) {
        QString winner = alive.isEmpty()
            ? QString::fromUtf8("无人胜出")
            : alive[0]->getDisplayName();
        emit showCenterEffect(QString::fromUtf8("gameover"), QString::fromUtf8("胜利"), 2000);
        emit gameOver(winner);
        endGame();
    }
}

void GameEngine::executeSecondTargetSkill(int skillIndex, int secondTargetIdx) {
    if (!m_running) return;

    Player* self = currentPlayer();
    if (!self || !self->getHero()) return;

    Player* secondTarget = m_players.value(secondTargetIdx, nullptr);
    if (!secondTarget) return;

    Player* firstTarget = m_firstTargetForSkill;

    if (skillIndex == 2 && self->getHero()->getName() == QString::fromUtf8("刘备")) {
        emit logMessage(QString::fromUtf8("【刘备·仁之剑义之剑】发动！%1受到1点伤害，%2恢复1血！")
                       .arg(firstTarget->getName()).arg(secondTarget->getName()));
        emit skillEffectRequest(self->getHero()->getName(),
                                QString::fromUtf8("仁之剑义之剑"),
                                QString::fromUtf8("video/liubei_jian.mp4"));

        firstTarget->receiveDamage(1);
        emit animationRequest("damage", playerIndex(self), playerIndex(firstTarget), "1");

        secondTarget->restoreHealth(1);
        emit animationRequest("heal", playerIndex(self), playerIndex(secondTarget), "1");

        self->setSkillUsed(true);
        emit gameStateChanged();
        checkGameOver();
    }

    m_firstTargetForSkill = nullptr;
    m_waitingForSecondTarget = false;
}

void GameEngine::prepareTargetSelectionForSkill(int skillIndex, const QString& skillName) {
    m_pendingSkillIndex = skillIndex;
    m_pendingSkillName = skillName;
    if (m_aiTimer) m_aiTimer->stop();
    emit requestTargetSelectionForSkill(skillIndex, skillName);
}

void GameEngine::prepareUseSkillAfterDamage(int skillIndex, const QString& skillName) {
    m_pendingSkillIndex = skillIndex;
    m_pendingSkillName = skillName;
    emit requestUseSkillAfterDamage(skillIndex, skillName);
}

void GameEngine::prepareSecondTargetForSkill(int skillIndex, Player* firstTarget) {
    m_firstTargetForSkill = firstTarget;
    m_pendingSkillIndex = skillIndex;
    m_waitingForSecondTarget = true;
    emit requestSecondTargetForSkill(skillIndex, firstTarget);
}
