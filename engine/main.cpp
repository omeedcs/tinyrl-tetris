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

enum class Action : uint8_t {
   LEFT, RIGHT, DOWN, CW, CCW, DROP, SWAP, NOOP
};

struct Observation {
   static constexpr int BoardW = 18;
   static constexpr int BoardH = 24;

    std::vector<std::vector<uint8_t> > board;
    std::vector<std::vector<uint8_t> > active_tetromino;
    std::vector<std::vector<uint8_t> > holder;
    std::vector<std::vector<uint8_t> > queue;
};

class TetrisGame {
public:
    TetrisGame(TimeManager::Mode m, uint8_t queue_size = 3);

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

    std::vector<std::vector<uint8_t>> board;
    std::vector<std::vector<uint8_t>> active_tetromino;
    std::vector<std::vector<uint8_t>> holder;
    std::vector<std::vector<uint8_t>> queue;
    
    int score;
    bool game_over;
    TimeManager tm;
    Observation obs;
};

TetrisGame::TetrisGame(TimeManager::Mode m, uint8_t queue_size): tm(TimeManager(m)) {
    board.resize(Observation::BoardH, std::vector<uint8_t>(Observation::BoardW, 0));
    active_tetromino.resize(Observation::BoardH, std::vector<uint8_t>(Observation::BoardW, 0));
    // holder.resize(Tetris::PIECE_SIZE, std::vector<uint8_t>(Tetris::PIECE_SIZE, 0));
    // queue.resize(queue_size * Tetris::PIECE_SIZE, std::vector<uint8_t>(Tetris::PIECE_SIZE, 0));
}

bool TetrisGame::step(int action) {
    // TODO: needs to essentially
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
        return static_cast<int>(Action::LEFT);
    } else if (input == "l") {
        return static_cast<int>(Action::RIGHT);
    } else if (input == "k") {
        return static_cast<int>(Action::DOWN);
    } else if (input == "o") {
        return static_cast<int>(Action::CW);
    } else if (input == "u") {
        return static_cast<int>(Action::CCW);
    } else if (input == "K") {
        return static_cast<int>(Action::DROP);
    } else if (input == ",") {
        return static_cast<int>(Action::SWAP);
    } else {
        return static_cast<int>(Action::NOOP);
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
