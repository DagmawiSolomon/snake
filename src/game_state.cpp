#include "game_state.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>

constexpr int INITIAL_TICK_MS = 160;
constexpr int MIN_TICK_MS = 75;
constexpr int SPEED_RAMP_MS = 3;

GameManager::GameManager()
    : m_state(GameState::MENU),
      m_score(0),
      m_highScore(0),
      m_tickIntervalMs(INITIAL_TICK_MS)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    initNewGame();
}

void GameManager::initNewGame() {
    m_snake.body.clear();
    // Start on PZ (+Z face, Front), center, moving RIGHT
    int startU = GRID_N / 2;
    int startV = GRID_N / 2;

    m_snake.body.push_back({ Face::PZ, startU,     startV });     // head
    m_snake.body.push_back({ Face::PZ, startU - 1, startV });     // mid
    m_snake.body.push_back({ Face::PZ, startU - 2, startV });     // tail

    m_snake.currentDir = Dir::RIGHT;
    m_snake.pendingDir = Dir::RIGHT;
    m_snake.growPending = false;

    m_score = 0;
    m_tickIntervalMs = INITIAL_TICK_MS;

    spawnFood();
}

void GameManager::reset() {
    initNewGame();
    m_state = GameState::PLAYING;
}

void GameManager::startGame() {
    initNewGame();
    m_state = GameState::PLAYING;
}

void GameManager::togglePause() {
    if (m_state == GameState::PLAYING) {
        m_state = GameState::PAUSED;
    } else if (m_state == GameState::PAUSED) {
        m_state = GameState::PLAYING;
    }
}

void GameManager::requestDirection(Dir dir) {
    if (m_state != GameState::PLAYING) return;

    // Reject direct 180-degree reversals against current movement direction
    if (!isOpposite(dir, m_snake.currentDir)) {
        m_snake.pendingDir = dir;
    }
}

bool GameManager::isOccupiedBySnake(const GridPos& pos) const {
    for (const auto& segment : m_snake.body) {
        if (segment == pos) return true;
    }
    return false;
}

void GameManager::spawnFood() {
    // Find all free cells on the 6 faces
    std::vector<GridPos> freeCells;
    freeCells.reserve(6 * GRID_N * GRID_N);

    for (int f = 0; f < 6; f++) {
        Face face = static_cast<Face>(f);
        for (int u = 0; u < GRID_N; u++) {
            for (int v = 0; v < GRID_N; v++) {
                GridPos candidate = { face, u, v };
                if (!isOccupiedBySnake(candidate)) {
                    freeCells.push_back(candidate);
                }
            }
        }
    }

    if (freeCells.empty()) {
        // Player filled the entire cube!
        m_state = GameState::GAME_OVER;
        return;
    }

    int idx = std::rand() % freeCells.size();
    m_food = freeCells[idx];
}

bool GameManager::tick() {
    if (m_state != GameState::PLAYING) {
        return false;
    }

    // 1. Apply buffered input (re-check opposite against currentDir)
    if (!isOpposite(m_snake.pendingDir, m_snake.currentDir)) {
        m_snake.currentDir = m_snake.pendingDir;
    }

    // 2. Compute candidate new head position
    GridPos nextHead;
    Dir nextDir;
    stepPosition(m_snake.body.front(), m_snake.currentDir, nextHead, nextDir);

    // 3. Self-collision check
    // Notice Section 10 of implementation plan:
    // If growPending is true: tail does NOT move away, check against entire body.
    // If growPending is false: tail will pop away this frame, so collision against tail segment is safe.
    size_t checkCount = m_snake.growPending ? m_snake.body.size() : (m_snake.body.size() - 1);
    for (size_t i = 0; i < checkCount; i++) {
        if (m_snake.body[i] == nextHead) {
            m_state = GameState::GAME_OVER;
            return false;
        }
    }

    // 4. Update direction
    m_snake.currentDir = nextDir;
    m_snake.pendingDir = nextDir;

    // 5. Push new head
    m_snake.body.push_front(nextHead);

    // 6. Check food consumption
    if (nextHead == m_food) {
        m_score += 10;
        if (m_score > m_highScore) {
            m_highScore = m_score;
        }
        m_snake.growPending = true;

        // Speed ramp
        if (m_tickIntervalMs > MIN_TICK_MS) {
            m_tickIntervalMs = std::max(MIN_TICK_MS, m_tickIntervalMs - SPEED_RAMP_MS);
        }

        spawnFood();
    } else {
        // Pop tail if not growing
        m_snake.body.pop_back();
        m_snake.growPending = false;
    }

    return true;
}
