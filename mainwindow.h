#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QTimer>
#include <QVector>
#include <QPointF>
#include <QColor>
#include <functional>

#include "gameengine.h"
#include "Player.h"

class HeroPanel;
class AnimOverlay;
class CardWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private slots:
    void onStartGame();
    void onEndTurn();
    void onSkill0();
    void onSkill1();
    void onCardClicked(int index);
    void onTargetSelected(int playerIndex);
    void onUpdateUI();
    void onGameOver(const QString& winner);
    void onAnimationRequest(const QString& type, int from, int to, const QString& extra);
    void onParticleTimer();
    void onBackgroundTimer();

private:
    void buildUI();
    void layoutWidgets();
    void styleButton(QPushButton* btn, const QString& bgColor, const QString& textColor);
    void refreshHandCards();
    void setGameButtonsEnabled(bool en);
    void enterTargetSelectionMode(std::function<void(int)> callback);
    void exitTargetSelectionMode();
    void showFloatingText(const QString& text, int playerIndex, const QString& color);
    void playCardAnimation(int fromIdx, int toIdx, const QString& cardName);
    void playSkillAnimation(int fromIdx, int toIdx, const QString& skillName);
    void playDamageAnimation(int playerIdx, int amount);
    void playHealAnimation(int playerIdx, int amount);
    void playTurnStartAnimation(int playerIdx);
    QPoint playerPanelCenter(int idx) const;
    void spawnParticles(const QPoint& pos, const QColor& col, int count, const QString& style = "burst");

    GameEngine* m_engine     = nullptr;
    Player*     m_players[3] = {};

    bool m_selectingTarget = false;
    std::function<void(int)> m_targetCallback;
    int  m_pendingCardIndex  = -1;
    int  m_pendingSkillIndex = -1;

    QWidget*      m_centralWidget = nullptr;
    HeroPanel*    m_heroPanels[3] = {};
    QWidget*      m_handArea      = nullptr;
    QVector<QPushButton*> m_cardButtons;
    
    QPushButton*  m_btnStart      = nullptr;
    QPushButton*  m_btnEndTurn    = nullptr;
    QPushButton*  m_btnSkill0     = nullptr;
    QPushButton*  m_btnSkill1     = nullptr;
    QPushButton*  m_btnCancel     = nullptr;
    
    QLabel*       m_turnLabel     = nullptr;
    QLabel*       m_statusLabel   = nullptr;
    QTextBrowser* m_log           = nullptr;
    AnimOverlay*  m_overlay       = nullptr;

    QTimer* m_particleTimer = nullptr;
    QTimer* m_backgroundTimer = nullptr;
    float   m_bgPhase = 0.0f;
    
    struct Particle {
        QPointF pos, vel;
        float   life, maxLife;
        QColor  color;
        float   size;
        float   rotation;
        float   rotSpeed;
        QString style;
    };
    QVector<Particle> m_particles;
};

class HeroPanel : public QWidget {
    Q_OBJECT
public:
    explicit HeroPanel(int idx, QWidget* parent = nullptr);
    void setPlayer(Player* p);
    void refresh();
    void highlight(bool on);
    void flashDamage();
    void flashHeal();
    void playTurnGlow();
    int  playerIndex() const { return m_idx; }

signals:
    void clicked(int idx);

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void enterEvent(QEvent* e) override;
    void leaveEvent(QEvent* e) override;

private:
    int     m_idx;
    Player* m_player      = nullptr;
    bool    m_highlighted = false;
    bool    m_hovered     = false;
    qreal   m_flashAlpha  = 0.0;
    qreal   m_glowIntensity = 0.0;
    qreal   m_healAlpha   = 0.0;
    QTimer* m_flashTimer  = nullptr;
    QTimer* m_glowTimer   = nullptr;
    float   m_animPhase   = 0.0f;
};

class AnimOverlay : public QWidget {
    Q_OBJECT
public:
    explicit AnimOverlay(QWidget* parent = nullptr);
    void playCardFly(const QPoint& from, const QPoint& to, const QString& label, const QColor& col);
    void playSkillFlash(const QPoint& center, const QString& label, const QColor& col);
    void playDamageNumber(const QPoint& pos, int amount, const QColor& col);
    void playHealNumber(const QPoint& pos, int amount, const QColor& col);
    void playTurnStart(const QPoint& center, const QString& name);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    struct FlyCard {
        QPointF pos, start, end;
        QString label;
        QColor  color;
        float   progress;
        bool    active;
        float   rotation;
        float   scale;
    };
    struct FlashLabel {
        QPointF center;
        QString label;
        QColor  color;
        float   alpha;
        float   scale;
        bool    active;
        float   glowPhase;
    };
    struct FloatingNumber {
        QPointF pos;
        QString text;
        QColor  color;
        float   alpha;
        float   scale;
        float   velY;
        bool    active;
        bool    isDamage;
    };
    struct TurnStartAnim {
        QPointF center;
        QString name;
        float   progress;
        bool    active;
    };
    QVector<FlyCard>        m_flyCards;
    QVector<FlashLabel>     m_flashes;
    QVector<FloatingNumber> m_numbers;
    QVector<TurnStartAnim>  m_turnStarts;
    QTimer*                 m_timer = nullptr;
};

#endif // MAINWINDOW_H
