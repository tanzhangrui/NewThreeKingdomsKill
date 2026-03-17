#include "Hero.h"
#include "Player.h"
#include "Card.h"
#include <QRandomGenerator>
#include <QDebug>

// ---- Hero base ----
Hero::Hero(const QString& heroName,
           const QString& skill1Name, const QString& skill1Desc,
           const QString& skill2Name, const QString& skill2Desc,
           int hp)
    : name(heroName),
      sName1(skill1Name), sDesc1(skill1Desc),
      sName2(skill2Name), sDesc2(skill2Desc),
      maxHp(hp) {}

// ============================================================
// 曹操
//   技能0「奸雄」：立即摸1张牌
//   技能1「护驾」：令目标出【闪】，否则受1伤
// ============================================================
CaoCao::CaoCao()
    : Hero("\u66f9\u64cd",
           "\u5978\u96c4", "\u7acb\u5373\u6478\u53961\u5f20\u724c\uff08\u88ab\u52a8\u89e6\u53d1\u6f14\u793a\uff09",
           "\u62a4\u9a7e", "\u4ee4\u76ee\u6807\u51fa\u3010\u95ea\u3011\uff0c\u5426\u5219\u53d7\u5c061\u70b9\u4f24\u5bb3",
           4) {}

bool CaoCao::activateSkill(int skillIndex, Player* self, Player* target) {
    if (!self) return false;
    if (skillIndex == 0) {
        // 奸雄：摸1张牌
        self->drawCards(1);
        qDebug() << "[\u66f9\u64cd\u00b7\u5978\u96c4]" << self->getName() << "\u64781\u5f20\u724c";
        return true;
    } else {
        // 护驾：令目标出闪否则受伤
        if (!target) return false;
        bool hasDodge = false;
        const auto& cards = target->getHandCards();
        for (int i = 0; i < cards.size(); ++i) {
            if (cards.at(i)->getType() == CardType::Dodge) {
                hasDodge = true;
                target->removeCardByIndex(i);
                break;
            }
        }
        if (!hasDodge) {
            target->receiveDamage(1);
            qDebug() << "[\u66f9\u64cd\u00b7\u62a4\u9a7e]" << target->getName() << "\u65e0\u95ea\uff0c\u53d71\u4f24";
        } else {
            qDebug() << "[\u66f9\u64cd\u00b7\u62a4\u9a7e]" << target->getName() << "\u51fa\u95ea\uff0c\u5316\u89e3";
        }
        return true;
    }
}

// ============================================================
// 司马懿
//   技能0「反馈」：摸1张牌并对目标造成1伤
//   技能1「鬼才」：消耗1张手牌，令目标弃1张手牌
// ============================================================
SimaYi::SimaYi()
    : Hero("\u53f8\u9a6c\u61ff",
           "\u53cd\u9988", "\u6478\u53961\u5f20\u724c\uff0c\u5e76\u5bf9\u76ee\u6807\u9020\u621011\u70b9\u4f24\u5bb3",
           "\u9b3c\u624d", "\u6d88\u800811\u5f20\u624b\u724c\uff0c\u4ee4\u76ee\u6807\u5f131\u5f20\u624b\u724c",
           3) {}

bool SimaYi::activateSkill(int skillIndex, Player* self, Player* target) {
    if (!self) return false;
    if (skillIndex == 0) {
        self->drawCards(1);
        if (target) target->receiveDamage(1);
        qDebug() << "[\u53f8\u9a6c\u61ff\u00b7\u53cd\u9988] \u64781\u5f20\u5e76\u5bf9"
                 << (target ? target->getName() : QString("\u65e0")) << "\u9020\u621011\u4f24";
        return true;
    } else {
        if (!target || self->getHandCards().isEmpty()) return false;
        int idx = QRandomGenerator::global()->bounded(self->getHandCards().size());
        self->removeCardByIndex(idx);
        if (!target->getHandCards().isEmpty()) {
            int tidx = QRandomGenerator::global()->bounded(target->getHandCards().size());
            target->removeCardByIndex(tidx);
        }
        qDebug() << "[\u53f8\u9a6c\u61ff\u00b7\u9b3c\u624d] \u6d88\u80801\u724c\uff0c\u4ee4"
                 << target->getName() << "\u51311\u5f20";
        return true;
    }
}

// ============================================================
// 刘备
//   技能0「仁德」：给目标1张手牌，回复自身1血
//   技能1「激将」：对目标造成1伤，令其弃1张牌
// ============================================================
LiuBei::LiuBei()
    : Hero("\u5218\u5907",
           "\u4ec1\u5fb7", "\u7ed9\u4e88\u76ee\u6807\u00b11\u5f20\u624b\u724c\uff0c\u56de\u590d\u81ea\u8eab1\u70b9\u4f53\u529b",
           "\u6fc0\u5c06", "\u5bf9\u76ee\u6807\u9020\u621011\u70b9\u4f24\u5bb3\uff0c\u4ee4\u5176\u5f131\u5f20\u724c",
           4) {}

bool LiuBei::activateSkill(int skillIndex, Player* self, Player* target) {
    if (!self) return false;
    if (skillIndex == 0) {
        if (self->getHandCards().isEmpty() || !target) return false;
        int idx = QRandomGenerator::global()->bounded(self->getHandCards().size());
        Card* gift = self->getHandCards().at(idx);
        self->removeCardByPointer(gift, false);
        target->addCard(gift);
        self->restoreHealth(1);
        qDebug() << "[\u5218\u5907\u00b7\u4ec1\u5fb7] \u7ed9" << target->getName() << "1\u5f20\u724c\uff0c\u56de\u590d1\u8840";
        return true;
    } else {
        if (!target) return false;
        target->receiveDamage(1);
        if (!target->getHandCards().isEmpty()) {
            int tidx = QRandomGenerator::global()->bounded(target->getHandCards().size());
            target->removeCardByIndex(tidx);
        }
        qDebug() << "[\u5218\u5907\u00b7\u6fc0\u5c06] \u5bf9" << target->getName() << "\u9020\u621011\u4f24\u5e76\u4ee4\u5176\u51311\u5f20";
        return true;
    }
}
