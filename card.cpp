#include "Card.h"
#include "Player.h"
#include <QRandomGenerator>

// ---- Card base ----
Card::Card(const QString& n, const QString& d, CardType t)
    : name(n), description(d), type(t) {}

// ---- KillCard ----
KillCard::KillCard()
    : Card("\u6740", "\u5bf9\u4e00\u540d\u5176\u4ed6\u89d2\u8272\u9020\u621011\u70b9\u4f24\u5bb3", CardType::Kill) {}

bool KillCard::play(Player* from, Player* to) {
    Q_UNUSED(from)
    if (!to) return false;
    to->receiveDamage(1);
    return true;
}

// ---- DodgeCard ----
DodgeCard::DodgeCard()
    : Card("\u95EA", "\u6297\u6d88\u4e00\u5f20\u3010\u6740\u3011\u5bf9\u4f60\u9020\u6210\u7684\u4f24\u5bb3", CardType::Dodge) {}

bool DodgeCard::play(Player* from, Player* to) {
    Q_UNUSED(from) Q_UNUSED(to)
    return true;
}

// ---- PeachCard ----
PeachCard::PeachCard()
    : Card("\u6843", "\u6062\u590d1\u70b9\u4f53\u529b\uff08\u4e0a\u9650\u5185\uff09", CardType::Peach) {}

bool PeachCard::play(Player* from, Player* to) {
    Q_UNUSED(to)
    if (!from) return false;
    from->restoreHealth(1);
    return true;
}

// ---- TrickCard ----
TrickCard::TrickCard(TrickSubType st)
    : Card(st == StealCard
               ? "\u987a\u624b\u7635\u7f8a"
               : "\u8fc7\u6cb3\u62c6\u6865",
           st == StealCard
               ? "\u83b7\u5f97\u5bf9\u65b91\u5f20\u624b\u724c"
               : "\u5f03\u7f6e\u5bf9\u65b91\u5f20\u624b\u724c",
           CardType::Trick),
      subType(st) {}

bool TrickCard::play(Player* from, Player* to) {
    if (!from || !to) return false;
    auto cards = to->getHandCards();
    if (cards.isEmpty()) return false;
    int idx = QRandomGenerator::global()->bounded(cards.size());
    Card* target = cards.at(idx);
    if (subType == StealCard) {
        to->removeCardByPointer(target, false);
        from->addCard(target);
    } else {
        to->removeCardByPointer(target, true);
    }
    return true;
}

// ---- Factory ----
Card* createRandomCard() {
    int r = QRandomGenerator::global()->bounded(10);
    if (r < 4)       return new KillCard();
    else if (r < 6)  return new DodgeCard();
    else if (r < 8)  return new PeachCard();
    else             return new TrickCard(
        r == 8 ? TrickCard::StealCard : TrickCard::DiscardCard);
}
