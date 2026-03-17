#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include "Player.h"

class GameEngine : public QObject {
    Q_OBJECT
public:
    explicit GameEngine(QObject* parent = nullptr);
    ~GameEngine() override;

    void startGame(Player* p0, Player* p1, Player* p2);
    void endGame();

    bool    isRunning()          const { return m_running; }
    Player* currentPlayer()      const { return m_players.value(m_curIdx, nullptr); }
    int     currentPlayerIndex() const { return m_curIdx; }
    Player* playerAt(int i)      const { return m_players.value(i, nullptr); }
    int     playerCount()        const { return m_players.size(); }
    int     playerIndex(Player* p) const;
    bool    isHumanTurn()        const { return m_curIdx == 0 && m_running; }

    void playerUseCard(int cardIndex, int targetPlayerIndex);
    void playerUseSkill(int skillIndex, int targetPlayerIndex);
    void playerEndTurn();
    void playerRespondDodge(bool useDodge);
    
    bool isWaitingForDodgeResponse() const { return m_waitingForDodgeResponse; }
    int  getDodgeResponseTarget() const { return m_dodgeResponseTarget; }
    
    void checkGameOver();

signals:
    void logMessage(const QString& msg);
    void gameStateChanged();
    void animationRequest(const QString& type, int fromIdx, int toIdx, const QString& extra);
    void gameOver(const QString& winnerName);
    void skillEffectRequest(const QString& heroName, const QString& skillName, const QString& videoPath);
    void requestDodgeResponse(int targetPlayerIndex, int secondsTimeout);
    void showCenterEffect(const QString& effectType, const QString& text, int durationMs);

private slots:
    void aiTakeTurn();
    void onDodgeTimeout();

private:
    void advanceTurn();
    void startTurn();
    void performAiAction();
    void processKillCard(Player* from, Player* to);
    void triggerHeroTurnStart(Player* p);
    void triggerHeroTurnEnd(Player* p);
    void triggerHeroCardPlayed(Player* p, const QString& cardName, Player* target);
    void triggerHeroDamaged(Player* p, int dmg);
    void triggerHeroDeath(Player* p);

    QVector<Player*> m_players;
    int              m_curIdx  = 0;
    bool             m_running = false;
    QTimer*          m_aiTimer = nullptr;
    
    bool             m_waitingForDodgeResponse = false;
    int              m_dodgeResponseTarget = -1;
    int              m_dodgeCardIndex = -1;
    QTimer*          m_dodgeTimer = nullptr;
    Player*          m_pendingKillFrom = nullptr;
    Player*          m_pendingKillTo = nullptr;
};

#endif // GAMEENGINE_H
