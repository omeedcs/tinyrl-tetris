#include "tetrisGame.h"
#include "timeManager.h"

int main() {
    TetrisGame game(TimeManager::REALTIME);
    game.loop();
    return 0;
}
