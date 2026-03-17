#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include "Player.h"

// ============================================================
// GameEngine  ——  三国杀游戏引擎（三玩家轮流回合制）
// ============================================================
class GameEngine : public QObject {
    Q_OBJECT
public:
    explicit GameEngine(QObject* parent = nullptr);
    ~GameEngine() override;

    // 初始化并启动（传入已分配武将的三名玩家）
    void startGame(Player* p0, Player* p1, Player* p2);
    void endGame();

    // 查询状态
    bool    isRunning()          const { return m_running; }
    Player* currentPlayer()      const { return m_players.value(m_curIdx, nullptr); }
    int     currentPlayerIndex() const { return m_curIdx; }
    Player* playerAt(int i)      const { return m_players.value(i, nullptr); }
    int     playerCount()        const { return m_players.size(); }
    bool    isHumanTurn()        const { return m_curIdx == 0 && m_running; }

    // 玩家操作（仅人类回合有效）
    void playerUseCard(int cardIndex, int targetPlayerIndex);
    void playerUseSkill(int skillIndex, int targetPlayerIndex);
    void playerEndTurn();

signals:
    void logMessage(const QString& msg);
    void gameStateChanged();
    void animationRequest(const QString& type, int fromIdx, int toIdx, const QString& extra);
    void gameOver(const QString& winnerName);

private slots:
    void aiTakeTurn();

private:
    void advanceTurn();
    void startTurn();
    void performAiAction();
    void checkGameOver();

    QVector<Player*> m_players;
    int              m_curIdx  = 0;
    bool             m_running = false;
    QTimer*          m_aiTimer = nullptr;
};

#endif // GAMEENGINE_H
