// Tetris game logic implementation
#include <cstdint>
#include <vector>
#include "timeManager.h"
#include "constants.h"
#include <iostream>

// expand tetris namespace
namespace Tetris {
    // Piece shapes: [piece_type][rotation][row][col]
    // 7 pieces × 4 rotations × 4 rows × 4 cols
    constexpr uint8_t PIECES[7][4][4][4] = {
        // I-piece
        {
            {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},  // rot 0
            {{0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0}},  // rot 1
            {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},  // rot 2
            {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}}   // rot 3
        },
        // O-piece
        {
            {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
            {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
            {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
            {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}
        },
        // T-piece
        {
            {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},  // rot 0
            {{0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},  // rot 1
            {{0,0,0,0}, {1,1,1,0}, {0,1,0,0}, {0,0,0,0}},  // rot 2
            {{0,1,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}   // rot 3
        },
        // S-piece
        {
            {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},
            {{0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,0,0}},
            {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
            {{1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}
        },
        // Z-piece
        {
            {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
            {{0,0,1,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
            {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
            {{0,1,0,0}, {1,1,0,0}, {1,0,0,0}, {0,0,0,0}}
        },
        // J-piece
        {
            {{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
            {{0,1,1,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}},
            {{0,0,0,0}, {1,1,1,0}, {0,0,1,0}, {0,0,0,0}},
            {{0,1,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,0,0}}
        },
        // L-piece
        {
            {{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
            {{0,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,0,0}},
            {{0,0,0,0}, {1,1,1,0}, {1,0,0,0}, {0,0,0,0}},
            {{1,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}}
        }
    };
}


// action space: move left, move right, move down, rotate_cw, rotate_ccw, hard_drop, swap, no_op
// observation space: board(24x18), current tetromino mask, stored piece, upcoming piece
// rewards: +1 clear line, -2 when the game is over, .001 per step alive?

enum class Action : uint8_t {
   LEFT, RIGHT, DOWN, CW, CCW, DROP, SWAP, NOOP
};

struct Observation {
   static constexpr int BoardW = 18;
   static constexpr int BoardH = 24;

    std::vector<std::vector<uint8_t> > board; // 0-9 representing all forms of tetrominoes
    std::vector<std::vector<uint8_t> > active_tetromino; // 0,1 mask for where the piece is
    std::vector<std::vector<uint8_t> > holder;
    std::vector<std::vector<uint8_t> > queue;
};

struct StepResult {
    Observation obs;
    float reward;
    bool terminated;
};

class TetrisGame {
public:
    TetrisGame(TimeManager::Mode m, uint8_t queue_size = 3);

    void reset();
    StepResult step(int action);
    float getReward();
    int getAction();
    bool isGameOver();
    uint8_t getNextPiece();
    void updateActiveMask();
    void applyAction(int action);
    void loop();

private:
    void spawnPiece();
    bool checkCollision();
    void lockPiece();
    int clearLines();

    //general board state data
    int score;
    bool game_over;
    int8_t queue_size;
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

uint8_t TetrisGame::getNextPiece() {
   // update tetris game variables
   uint8_t next_piece = queue[queue_index];
   queue[queue_index] = rand() % 7;
   queue_index = (queue_index + 1) % queue_size;
   // For now, return random piece (0-6)
   return rand() % 7;
}

void TetrisGame::updateActiveMask() {
    // update active mask based on current piece type and rotation
    for (int i = 0; i < Tetris::PIECE_SIZE; i++) {
        for (int j = 0; j < Tetris::PIECE_SIZE; j++) {
            if (current_piece_type == 0) {
                obs.active_tetromino[current_y + i][current_x + j] = Tetris::PIECES[current_piece_type][rotation][i][j];
            }
        }
    }
}

TetrisGame::TetrisGame(TimeManager::Mode m, uint8_t queue_size): tm(TimeManager(m)) {
    obs.board.resize(Observation::BoardH, std::vector<uint8_t>(Observation::BoardW, 0));
    obs.active_tetromino.resize(Observation::BoardH, std::vector<uint8_t>(Observation::BoardW, 0));
    obs.holder.resize(Tetris::PIECE_SIZE, std::vector<uint8_t>(Tetris::PIECE_SIZE, 0));
    obs.queue.resize(queue_size * Tetris::PIECE_SIZE, std::vector<uint8_t>(Tetris::PIECE_SIZE, 0));

    // update current piece data
    rotation = 0;
    queue_index = 0;
    holder_type = 7; // out of range meaning not populated
    current_x = (Tetris::BOARD_WIDTH / 2);
    current_y =  Tetris::BOARD_HEIGHT - 1;
    current_piece_type = getNextPiece(); // pop from the queue

   // TODO: check for collision here
   if (checkCollision()) {
       game_over = true;
   }

   // adjusts current matrix with newly minted values
   updateActiveMask();
}


void TetrisGame::applyAction(int action) {
    // need to transform the current tetrimino
    // switch(action) {
    //     case Action::LEFT:
    //         // move left
    //         obs.active_tetromino =
    //         break;
    //     case Action::RIGHT:
    //         // move right
    //         break;
    //     case Action::DOWN:
    //         // move down
    //         break;
    //     case Action::ROTATE_CW:
    //         // rotate clockwise
    //         break;
    //     case Action::ROTATE_CCW:
    //         // rotate counterclockwise
    //         break;
    //     case Action::HOLD:
    //         // hold piece
    //         break;
    //     default:
    //         // invalid action
    //         break;
    // }
}

StepResult TetrisGame::step(int action) {
    // This is the pattern
    // apply action
    applyAction(action);
    // update gravity, mechanics, collision, lock, clear lines
    // updateGameState();
    // // compute reward based on the above
    // return StepResult{
    //     updateObservation(),
    //     calculateReward(),
    //     game_over
    // };
    return StepResult{
        obs,
        0,
        game_over
    };
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
