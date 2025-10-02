// Tetris game logic implementation
#include <cstdint>
#include <vector>
#include <chrono>
#include "timeManager.h"
#include "constants.h"
#include <iostream>

// action space: move left, move right, move down, rotate_cw, rotate_ccw, hard_drop, swap, no_op
// observation space: board(24x18), current tetromino mask, stored piece, upcoming piece
// rewards: +1 clear line, -2 when the game is over, .001 per step alive?

enum Action : uint8_t {
   LEFT, RIGHT, DOWN, CW, CCW, DROP, SWAP, NOOP
};

class TetrisGame {
public:
    TetrisGame(TimeManager::Mode m, uint8_t queue_size);

    void reset();
    bool step(int action);
    float getReward();
    int getAction();
    bool isGameOver();
    void loop();

private:
    void spawnPiece();
    bool checkCollision();
    void lockPiece();
    int clearLines();

    std::vector<std::vector<uint8_t> > board;
    std::vector<std::vector<uint8_t> > active_tetromino;
    std::vector<std::vector<uint8_t> > holder;
    std::vector<std::vector<uint8_t> > queue;
    int score;
    bool game_over;
    TimeManager tm;
};

TetrisGame::TetrisGame(TimeManager::Mode m, uint8_t queue_size = 3): tm(TimeManager(m)) {
    board.resize(Tetris::BOARD_HEIGHT, std::vector<uint8_t>(Tetris::BOARD_WIDTH, 0));
    active_tetromino.resize(Tetris::BOARD_HEIGHT, std::vector<uint8_t>(Tetris::BOARD_WIDTH, 0));
    holder.resize(Tetris::PIECE_SIZE, std::vector<uint8_t>(Tetris::PIECE_SIZE, 0));
    queue.resize(queue_size * Tetris::PIECE_SIZE, std::vector<uint8_t>(Tetris::PIECE_SIZE, 0));
}

bool TetrisGame::step(int action) {
    // TODO
    return true;
}

bool TetrisGame::isGameOver() {
    // TODO
    return false;
}

int TetrisGame::getAction() {
    // get user input and map
    std::string input;
    std::cin >> input;

    if (input == "j") {
        return LEFT;
    } else if (input == "l") {
        return RIGHT;
    } else if (input == "k") {
        return DOWN;
    } else if (input == "o") {
        return CW;
    } else if (input == "u") {
        return CCW;
    } else if (input == "K") {
        return DROP;
    } else if (input == ",") {
        return SWAP;
    } else {
        return NOOP;
    }
}

void render() {
    std::cout << "tick" << std::endl;
}


void TetrisGame::loop() {
    double accumulate = 0.0;
    // this is our main game loop
    while (!isGameOver()) {
       accumulate += tm.getDeltaTime();
       while (accumulate >= Tetris::TICK_RATE) {
           // get action
           int action = getAction();
           // update game
           step(action);
           accumulate -= Tetris::TICK_RATE;

           // render if needed
           if (tm.needRendering()) {
               render();
           }
       }
    }
}

int main() {
    TetrisGame game(TimeManager::REALTIME);
    game.loop();
    return 0;
}
