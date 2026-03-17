#include "Player.h"
#include "Hero.h"
#include <QDebug>

Player::Player(const QString& n, int maxHp)
    : m_name(n), m_hp(maxHp), m_maxHp(maxHp) {}

Player::~Player() {
    qDeleteAll(m_hand);
    m_hand.clear();
    delete m_hero;
    m_hero = nullptr;
}

void Player::setHero(Hero* h) {
    delete m_hero;
    m_hero = h;
    if (h) {
        m_maxHp = h->getMaxHp();
        m_hp    = m_maxHp;
    }
}

QString Player::getDisplayName() const {
    if (m_hero) {
        return m_hero->getDisplayName();
    }
    return m_name;
}

void Player::receiveDamage(int amount) {
    if (amount <= 0) return;
    m_hp -= amount;
    if (m_hp < 0) m_hp = 0;
    qDebug() << m_name << QString::fromUtf8("受") << amount << QString::fromUtf8("伤，剩余HP:") << m_hp;
}

void Player::restoreHealth(int amount) {
    if (amount <= 0) return;
    m_hp += amount;
    if (m_hp > m_maxHp) m_hp = m_maxHp;
    qDebug() << m_name << QString::fromUtf8("回复") << amount << QString::fromUtf8("血，当前HP:") << m_hp;
}

void Player::drawCards(int count) {
    for (int i = 0; i < count; ++i) {
        Card* c = createRandomCard();
        if (c) {
            m_hand.append(c);
            qDebug() << m_name << QString::fromUtf8("摸到:") << c->getName();
        }
    }
}

void Player::addCard(Card* card) {
    if (card) m_hand.append(card);
}

bool Player::removeCardByIndex(int index, bool deleteCard) {
    if (index < 0 || index >= m_hand.size()) return false;
    Card* c = m_hand.takeAt(index);
    if (deleteCard) delete c;
    return true;
}

bool Player::removeCardByPointer(Card* card, bool deleteCard) {
    if (!card) return false;
    int idx = m_hand.indexOf(card);
    if (idx < 0) return false;
    m_hand.removeAt(idx);
    if (deleteCard) delete card;
    return true;
}

int Player::findDodgeCard() const {
    for (int i = 0; i < m_hand.size(); ++i) {
        if (m_hand[i]->getType() == CardType::Dodge) {
            return i;
        }
    }
    return -1;
}

int Player::findKillCard() const {
    for (int i = 0; i < m_hand.size(); ++i) {
        if (m_hand[i]->getType() == CardType::Kill) {
            return i;
        }
    }
    return -1;
}

void Player::resetTurnState() {
    m_killUsed  = false;
    m_skillUsed = false;
    m_playedCardThisTurn = false;
}
