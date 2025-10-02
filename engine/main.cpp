// Tetris game logic implementation
#include <cstdint>
#include <vector>
#include <chrono>
#include "timeManager.h"
#include "constants.h"
#include <iostream>


class TetrisGame {
public:
    TetrisGame(TimeManager::Mode m);

    void reset();
    bool step(int action);
    float getReward();
    bool isGameOver();
    void loop();

private:
    void spawnPiece();
    bool checkCollision();
    void lockPiece();
    int clearLines();

    std::vector<std::vector<uint8_t> > board;
    int score;
    bool game_over;
    TimeManager tm;
};

bool TetrisGame::step(int action) {
    // TODO
    return true;
}

bool TetrisGame::isGameOver() {
    // TODO
    return false;
}

void render() {
    std::cout << "tick" << std::endl;
}

int getAction() {
    //TODO
    return 0;
}

TetrisGame::TetrisGame(TimeManager::Mode m): tm(TimeManager(m)) {
    // TODO
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
