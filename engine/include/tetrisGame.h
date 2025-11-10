#pragma once
#include <vector>
#include <cstdint>
#include <random>
#include "timeManager.h"

enum Action : uint8_t {
    LEFT, RIGHT, DOWN, CW, CCW, DROP, SWAP, NOOP
};

struct Observation {
    static constexpr int BoardW = 18;
    static constexpr int BoardH = 24;
    
    std::vector<std::vector<uint8_t>> board; // 0-9 representing all forms of tetrominoes
    std::vector<std::vector<uint8_t>> active_tetromino; // 0,1 mask for where the piece is
    std::vector<std::vector<uint8_t>> holder;
    std::vector<std::vector<uint8_t>> queue;
};

struct StepResult {
    Observation obs;
    float reward;
    bool terminated;
};

class TetrisGame {
public:
    TetrisGame(TimeManager::Mode m, uint8_t queue_size = 3, uint32_t seed = std::random_device{}());
    void reset();
    StepResult step(int action);
    float getReward();
    bool isGameOver();
    uint8_t getNextPiece();
    uint8_t setLastPiece(uint8_t val);
    void updateGameState();
    void updateActiveMask();
    void updateObservation();
    void applyAction(uint8_t action);
#ifndef NO_TERMINAL_LOOP
    void loop();
#endif

    // for random sampling
    uint8_t samplePiece();
    std::mt19937 rng_;

    // Made public for testing - consider friend class for production
    void spawnPiece();
    bool checkCollision();
    void lockPiece();
    int clearLines();
    void completeClearLines();
    int clearLine(uint8_t row);

    // general board state data
    int score;
    int scored; // points accumulated in one cycle
    bool game_over;
    int8_t queue_size;
    std::vector<int> clearing_lines;  // Lines currently being cleared (for animation)
    TimeManager tm;
    Observation obs;
    std::vector<uint8_t> queue;
    uint8_t queue_index; // circular buffer
    uint8_t holder_type;

    // current piece data
    int8_t current_x;
    int8_t current_y;
    uint8_t current_piece_type;
    uint8_t rotation; // 0-3 possible options
};
