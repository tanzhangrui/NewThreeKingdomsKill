#include "Card.h"
#include "Player.h"
#include "gameengine.h"
#include <QRandomGenerator>

Card::Card(const QString& n, const QString& d, CardType t)
    : name(n), description(d), type(t) {}

bool Card::play(Player* from, Player* to, GameEngine* engine) {
    Q_UNUSED(from) Q_UNUSED(to) Q_UNUSED(engine)
    return true;
}

KillCard::KillCard()
    : Card(QString::fromUtf8("杀"), 
           QString::fromUtf8("对一名其他角色造成1点伤害。对方可以使用【闪】来抵消此伤害。"), 
           CardType::Kill) {}

bool KillCard::play(Player* from, Player* to, GameEngine* engine) {
    Q_UNUSED(from) Q_UNUSED(engine)
    if (!to) return false;
    to->receiveDamage(1);
    return true;
}

DodgeCard::DodgeCard()
    : Card(QString::fromUtf8("闪"), 
           QString::fromUtf8("抵消一张【杀】对你造成的伤害。"), 
           CardType::Dodge) {}

bool DodgeCard::play(Player* from, Player* to, GameEngine* engine) {
    Q_UNUSED(from) Q_UNUSED(to) Q_UNUSED(engine)
    return true;
}

PeachCard::PeachCard()
    : Card(QString::fromUtf8("桃"), 
           QString::fromUtf8("恢复1点体力（不能超过体力上限）。"), 
           CardType::Peach) {}

bool PeachCard::play(Player* from, Player* to, GameEngine* engine) {
    Q_UNUSED(to) Q_UNUSED(engine)
    if (!from) return false;
    from->restoreHealth(1);
    return true;
}

TrickCard::TrickCard(TrickSubType st)
    : Card(
        st == StealCard ? QString::fromUtf8("顺手牵羊") :
        st == DiscardCard ? QString::fromUtf8("过河拆桥") :
        st == JumpTigerShark ? QString::fromUtf8("一对跳跳虎两条脆脆鲨") :
        QString::fromUtf8("天意侵袭"),
        st == StealCard ? QString::fromUtf8("获得目标1张手牌。") :
        st == DiscardCard ? QString::fromUtf8("弃置目标1张手牌。") :
        st == JumpTigerShark ? QString::fromUtf8("不分敌我全体投喂脆脆鲨，所有存活玩家回复1点血量。") :
        QString::fromUtf8("全体玩家必须出【杀】，否则受到1点伤害。"),
        CardType::Trick
      ),
      subType(st) {}

bool TrickCard::requiresTarget() const {
    return subType == StealCard || subType == DiscardCard;
}

QString TrickCard::getVideoPath() const {
    switch (subType) {
        case JumpTigerShark: return QString::fromUtf8("video/trick_tiaotiaohu.mp4");
        case TianyiInvasion: return QString::fromUtf8("video/trick_tianyi.mp4");
        default: return QString();
    }
}

bool TrickCard::play(Player* from, Player* to, GameEngine* engine) {
    if (!engine) return false;
    
    switch (subType) {
        case StealCard: {
            if (!from || !to) return false;
            auto cards = to->getHandCards();
            if (cards.isEmpty()) return false;
            int idx = QRandomGenerator::global()->bounded(cards.size());
            Card* target = cards.at(idx);
            to->removeCardByPointer(target, false);
            from->addCard(target);
            return true;
        }
        case DiscardCard: {
            if (!to) return false;
            auto cards = to->getHandCards();
            if (cards.isEmpty()) return false;
            int idx = QRandomGenerator::global()->bounded(cards.size());
            to->removeCardByIndex(idx, true);
            return true;
        }
        case JumpTigerShark: {
            emit engine->logMessage(QString::fromUtf8("【一对跳跳虎两条脆脆鲨】发动！全体玩家回复1点血量！"));
            emit engine->skillEffectRequest(QString::fromUtf8("跳跳虎脆脆鲨"),
                                            QString::fromUtf8("一对跳跳虎两条脆脆鲨"),
                                            QString());
            
            for (int i = 0; i < engine->playerCount(); ++i) {
                Player* p = engine->playerAt(i);
                if (p && p->isAlive()) {
                    p->restoreHealth(1);
                    emit engine->animationRequest("heal", i, i, "1");
                }
            }
            emit engine->gameStateChanged();
            return true;
        }
        case TianyiInvasion: {
            emit engine->logMessage(QString::fromUtf8("【天意侵袭】发动！全体玩家必须出【杀】，否则受到1点伤害！"));
            emit engine->skillEffectRequest(QString::fromUtf8("天意侵袭"),
                                            QString::fromUtf8("天意侵袭"),
                                            QString());
            
            for (int i = 0; i < engine->playerCount(); ++i) {
                Player* p = engine->playerAt(i);
                if (p && p->isAlive()) {
                    bool hasKill = false;
                    const auto& cards = p->getHandCards();
                    for (int j = 0; j < cards.size(); ++j) {
                        if (cards[j]->getType() == CardType::Kill) {
                            hasKill = true;
                            p->removeCardByIndex(j);
                            emit engine->logMessage(QString::fromUtf8("%1出【杀】应对！").arg(p->getName()));
                            break;
                        }
                    }
                    if (!hasKill) {
                        emit engine->logMessage(QString::fromUtf8("%1没有【杀】，受到1点伤害！").arg(p->getName()));
                        p->receiveDamage(1);
                        emit engine->animationRequest("damage", i, i, "1");
                    }
                }
            }
            emit engine->gameStateChanged();
            engine->checkGameOver();
            return true;
        }
    }
    return false;
}

Card* createRandomCard() {
    int r = QRandomGenerator::global()->bounded(12);
    if (r < 4)       return new KillCard();
    else if (r < 6)  return new DodgeCard();
    else if (r < 8)  return new PeachCard();
    else if (r < 9)  return new TrickCard(TrickCard::StealCard);
    else if (r < 10) return new TrickCard(TrickCard::DiscardCard);
    else if (r < 11) return new TrickCard(TrickCard::JumpTigerShark);
    else             return new TrickCard(TrickCard::TianyiInvasion);
}
