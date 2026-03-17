#include "gameengine.h"
#include "Hero.h"
#include <QRandomGenerator>
#include <QDebug>

GameEngine::GameEngine(QObject* parent)
    : QObject(parent) {
    m_aiTimer = new QTimer(this);
    m_aiTimer->setSingleShot(true);
    connect(m_aiTimer, &QTimer::timeout, this, &GameEngine::aiTakeTurn);
}

GameEngine::~GameEngine() {
}

void GameEngine::startGame(Player* p0, Player* p1, Player* p2) {
    m_players.clear();
    m_players << p0 << p1 << p2;
    m_curIdx  = 0;
    m_running = true;

    for (Player* p : m_players)
        p->drawCards(4);

    emit logMessage(QString::fromUtf8("══════ 天意侵蚀·三方对弈 开始！══════"));
    emit logMessage(QString::fromUtf8("玩家：【%1】 vs AI-魏：【%2】 vs AI-吴：【%3】")
                    .arg(p0->getHero() ? p0->getHero()->getName() : "?")
                    .arg(p1->getHero() ? p1->getHero()->getName() : "?")
                    .arg(p2->getHero() ? p2->getHero()->getName() : "?"));
    startTurn();
}

void GameEngine::endGame() {
    m_running = false;
    m_aiTimer->stop();
    emit logMessage(QString::fromUtf8("══════ 游戏结束 ══════"));
}

void GameEngine::startTurn() {
    if (!m_running) return;
    Player* cur = currentPlayer();
    if (!cur || !cur->isAlive()) { advanceTurn(); return; }

    cur->resetTurnState();
    cur->drawCards(2);
    
    emit animationRequest("turn", m_curIdx, -1, "");
    emit logMessage(QString::fromUtf8("【 %1 的回合 】摸2张牌")
                    .arg(cur->getName()));
    emit gameStateChanged();

    if (!isHumanTurn()) {
        m_aiTimer->start(1500);
    }
}

void GameEngine::advanceTurn() {
    if (!m_running) return;
    int tried = 0;
    do {
        m_curIdx = (m_curIdx + 1) % m_players.size();
        ++tried;
        if (tried > m_players.size()) { endGame(); return; }
    } while (!m_players[m_curIdx]->isAlive());
    startTurn();
}

void GameEngine::playerUseCard(int cardIndex, int targetPlayerIndex) {
    if (!m_running || !isHumanTurn()) return;
    Player* self   = currentPlayer();
    Player* target = m_players.value(targetPlayerIndex, nullptr);
    if (!self || !target) return;
    if (cardIndex < 0 || cardIndex >= self->getHandCards().size()) return;

    Card* card = self->getHandCards().at(cardIndex);
    if (!card) return;

    if (card->getType() == CardType::Kill && self->hasUsedKillThisTurn()) {
        emit logMessage(QString::fromUtf8("本回合已使用过【杀】！"));
        return;
    }
    if (card->getType() == CardType::Peach) target = self;
    if (card->getType() == CardType::Dodge) {
        emit logMessage(QString::fromUtf8("【闪】只能应对他人的【杀】，不能主动使用"));
        return;
    }

    bool ok = card->play(self, target);
    if (ok) {
        if (card->getType() == CardType::Kill) self->setKillUsed(true);
        emit animationRequest("card", 0, targetPlayerIndex, card->getName());
        emit logMessage(self->getName() + QString::fromUtf8(" 使用【") + card->getName()
                        + QString::fromUtf8("】→ ") + target->getName());
        self->removeCardByIndex(cardIndex);
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
    if (self->hasUsedSkillThisTurn()) {
        emit logMessage(QString::fromUtf8("本回合已发动过技能！"));
        return;
    }

    bool ok = self->getHero()->activateSkill(skillIndex, self, target);
    if (ok) {
        self->setSkillUsed(true);
        QString skillName = self->getHero()->getSkillName(skillIndex);
        emit animationRequest("skill", 0, targetPlayerIndex, skillName);
        emit logMessage(self->getName() + QString::fromUtf8(" 发动【") + skillName
                        + QString::fromUtf8("】→ ")
                        + (target ? target->getName() : QString::fromUtf8("自身")));
        emit gameStateChanged();
        checkGameOver();
    } else {
        emit logMessage(QString::fromUtf8("技能发动失败（条件不满足）"));
    }
}

void GameEngine::playerEndTurn() {
    if (!m_running || !isHumanTurn()) return;
    emit logMessage(currentPlayer()->getName() + QString::fromUtf8(" 结束回合"));
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
                hand[i]->play(self, self);
                emit logMessage(self->getName() + QString::fromUtf8(" 使用【桃】回复体力"));
                self->removeCardByIndex(i);
                emit animationRequest("card", m_curIdx, m_curIdx, QString::fromUtf8("桃"));
                emit animationRequest("heal", m_curIdx, m_curIdx, "1");
                emit gameStateChanged();
                break;
            }
        }
    }

    if (!self->hasUsedKillThisTurn()) {
        const auto& hand = self->getHandCards();
        for (int i = 0; i < hand.size(); ++i) {
            if (hand[i]->getType() == CardType::Kill) {
                int tIdx = m_players.indexOf(weakest);
                hand[i]->play(self, weakest);
                self->setKillUsed(true);
                emit logMessage(self->getName() + QString::fromUtf8(" 使用【杀】攻击 ")
                                + weakest->getName());
                self->removeCardByIndex(i);
                emit animationRequest("card", m_curIdx, tIdx, QString::fromUtf8("杀"));
                emit animationRequest("damage", m_curIdx, tIdx, "1");
                emit gameStateChanged();
                checkGameOver();
                if (!m_running) return;
                break;
            }
        }
    }

    if (!self->hasUsedSkillThisTurn() && self->getHero()) {
        if (QRandomGenerator::global()->bounded(100) < 70) {
            int skillIdx = QRandomGenerator::global()->bounded(2);
            int tIdx = m_players.indexOf(weakest);
            bool ok = self->getHero()->activateSkill(skillIdx, self, weakest);
            if (ok) {
                self->setSkillUsed(true);
                QString sn = self->getHero()->getSkillName(skillIdx);
                emit logMessage(self->getName() + QString::fromUtf8(" 发动【") + sn + QString::fromUtf8("】"));
                emit animationRequest("skill", m_curIdx, tIdx, sn);
                emit gameStateChanged();
                checkGameOver();
                if (!m_running) return;
            }
        }
    }

    {
        const auto& hand = self->getHandCards();
        for (int i = 0; i < hand.size(); ++i) {
            if (hand[i]->getType() == CardType::Trick) {
                int tIdx = m_players.indexOf(weakest);
                bool ok  = hand[i]->play(self, weakest);
                if (ok) {
                    emit logMessage(self->getName() + QString::fromUtf8(" 使用【")
                                    + hand[i]->getName() + QString::fromUtf8("】"));
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

    emit logMessage(self->getName() + QString::fromUtf8(" 结束回合"));
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
            : alive[0]->getName();
        emit gameOver(winner);
        endGame();
    }
}
