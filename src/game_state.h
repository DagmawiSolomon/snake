#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <deque>
#include <vector>
#include "cube_topology.h"

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

struct Snake {
    std::deque<GridPos> body; // body.front() is head, body.back() is tail
    Dir currentDir;
    Dir pendingDir;           // buffered input applied on next tick
    bool growPending;
};

class GameManager {
public:
    GameManager();

    void initNewGame();
    void reset();

    // Input handlers
    void requestDirection(Dir dir);
    void togglePause();
    void startGame();

    // Tick update (called at fixed intervals)
    bool tick(); // returns true if snake moved, false if game over / paused

    // Queries
    GameState getState() const { return m_state; }
    void setState(GameState s) { m_state = s; }
    const Snake& getSnake() const { return m_snake; }
    const GridPos& getFood() const { return m_food; }
    int getScore() const { return m_score; }
    int getHighScore() const { return m_highScore; }
    int getTickIntervalMs() const { return m_tickIntervalMs; }
    Face getActiveFace() const { return m_snake.body.front().face; }

    bool isOccupiedBySnake(const GridPos& pos) const;

private:
    void spawnFood();

    GameState m_state;
    Snake m_snake;
    GridPos m_food;
    int m_score;
    int m_highScore;
    int m_tickIntervalMs;
};

#endif // GAME_STATE_H
