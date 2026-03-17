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
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QStackedWidget>
#include <functional>

#include "gameengine.h"
#include "Player.h"

class HeroPanel;
class AnimOverlay;
class CenterEffectWidget;
class CharacterSelectWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onStartGame();
    void onCharacterSelected(const QString& heroName);
    void onEndTurn();
    void onSkillClicked(int skillIndex);
    void onCardClicked(int index);
    void onTargetSelected(int playerIndex);
    void onUpdateUI();
    void onGameOver(const QString& winner);
    void onAnimationRequest(const QString& type, int from, int to, const QString& extra);
    void onParticleTimer();
    void onBackgroundTimer();
    void onShowCenterEffect(const QString& type, const QString& text, int durationMs);
    void onSkillEffectRequest(const QString& heroName, const QString& skillName, const QString& videoPath);
    void onRequestDodgeResponse(int targetPlayerIndex, int secondsTimeout);
    void onDodgeResponse(bool useDodge);

private:
    void buildUI();
    void buildCharacterSelectUI();
    void buildGameUI();
    void styleButton(QPushButton* btn, const QString& bgColor, const QString& textColor);
    void refreshHandCards();
    void refreshSkillButtons();
    void setGameButtonsEnabled(bool en);
    void enterTargetSelectionMode(std::function<void(int)> callback);
    void exitTargetSelectionMode();
    void showFloatingText(const QString& text, int playerIndex, const QString& color);
    void playCardAnimation(int fromIdx, int toIdx, const QString& cardName);
    void playSkillAnimation(int fromIdx, int toIdx, const QString& skillName);
    void playDamageAnimation(int playerIdx, int amount);
    void playHealAnimation(int playerIdx, int amount);
    void playTurnStartAnimation(int playerIdx);
    void showCenterEffectText(const QString& text, const QString& type, int durationMs);
    void playVideoEffect(const QString& videoPath);
    QPoint playerPanelCenter(int idx) const;
    void spawnParticles(const QPoint& pos, const QColor& col, int count, const QString& style = "burst");
    void startGameWithHero(const QString& heroName);

    GameEngine* m_engine     = nullptr;
    Player*     m_players[3] = {};
    QString     m_selectedHero;

    bool m_selectingTarget = false;
    std::function<void(int)> m_targetCallback;
    int  m_pendingCardIndex  = -1;
    int  m_pendingSkillIndex = -1;

    QStackedWidget* m_stackedWidget = nullptr;
    QWidget*        m_characterSelectPage = nullptr;
    QWidget*        m_gamePage = nullptr;
    
    QWidget*      m_centralWidget = nullptr;
    HeroPanel*    m_heroPanels[3] = {};
    QWidget*      m_handArea      = nullptr;
    QVector<QPushButton*> m_cardButtons;
    
    QPushButton*  m_btnEndTurn    = nullptr;
    QVector<QPushButton*> m_skillButtons;
    QPushButton*  m_btnCancel     = nullptr;
    QPushButton*  m_btnUseDodge   = nullptr;
    QPushButton*  m_btnNoDodge    = nullptr;
    
    QLabel*       m_turnLabel     = nullptr;
    QTextBrowser* m_log           = nullptr;
    AnimOverlay*  m_overlay       = nullptr;
    CenterEffectWidget* m_centerEffect = nullptr;
    
    QMediaPlayer*   m_videoPlayer = nullptr;
    QVideoWidget*   m_videoWidget = nullptr;
    QWidget*        m_videoOverlay = nullptr;

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

class CenterEffectWidget : public QWidget {
    Q_OBJECT
public:
    explicit CenterEffectWidget(QWidget* parent = nullptr);
    void showEffect(const QString& text, const QString& type, int durationMs);
    
protected:
    void paintEvent(QPaintEvent* e) override;

private:
    QString m_text;
    QString m_type;
    float   m_alpha = 0.0f;
    float   m_scale = 0.5f;
    bool    m_active = false;
    QTimer* m_animTimer = nullptr;
    float   m_phase = 0.0f;
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
