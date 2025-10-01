// Tetris game logic implementation
#include <cstdint>
#include <vector>

class TetrisGame {
public:
    TetrisGame();
    ~TetrisGame();

    void reset();
    bool step(int action);
    float getReward();
    bool isGameOver();

private:
    void spawnPiece();
    bool checkCollision();
    void lockPiece();
    int clearLines();

    std::vector<std::vector<uint8_t>> board;
    int score;
    bool game_over;
};
