#include "tetrisGame.h"
#include "pieces.h"
#include "tetrisGame.h"
#include "pieces.h"
#include "constants.h"
#include "input.h"
#include "renderer.h"
#include <cstdint>
#include <cstdlib>
#include <iostream>

TetrisGame::TetrisGame(TimeManager::Mode m, uint8_t queue_size)
    : tm(TimeManager(m)), queue_size(queue_size), score(0), game_over(false) {
    obs.board.resize(Observation::BoardH, std::vector<uint8_t>(Observation::BoardW, 0));
    obs.active_tetromino.resize(Observation::BoardH, std::vector<uint8_t>(Observation::BoardW, 0));
    obs.holder.resize(Tetris::PIECE_SIZE, std::vector<uint8_t>(Tetris::PIECE_SIZE, 0));
    obs.queue.resize(queue_size * Tetris::PIECE_SIZE, std::vector<uint8_t>(Tetris::PIECE_SIZE, 0));

    // update current piece data
    rotation = 0;
    queue_index = 0;
    holder_type = 7; // out of range meaning not populated
    current_x = (Tetris::BOARD_WIDTH / 2);
    current_y = Tetris::BOARD_HEIGHT - 1;
    current_piece_type = getNextPiece(); // pop from the queue

    if (checkCollision()) {
        game_over = true;
    }

    // adjusts current matrix with newly minted values
    updateActiveMask();
}

void TetrisGame::reset() {
    // TODO: Implement reset logic
}

// get val from queue
// replace current index with random
// update queue_index
//  1 2 3
//  7 2 3
//  1 2 3
uint8_t TetrisGame::getNextPiece() {
    // update tetris game variables
    uint8_t next_piece = queue[queue_index];
    queue[queue_index] = rand() % 7;
    queue_index = (queue_index + 1) % queue_size;
    // For now, return random piece (0-6)
    return rand() % 7;
}

uint8_t TetrisGame::setLastPiece(uint8_t val) {
    // update tetris game variables
    queue_index = (queue_index - 1 + queue_size) % queue_size;
    queue[queue_index] = val;
    // For now, return random piece (0-6)
    return val;
}

void TetrisGame::updateActiveMask() {
    // update active mask based on current piece type and rotation
    for (int i = 0; i < Tetris::PIECE_SIZE; i++) {
        for (int j = 0; j < Tetris::PIECE_SIZE; j++) {
            if (current_piece_type == 0) {
                obs.active_tetromino[current_y + i][current_x + j] =
                    Tetris::PIECES[current_piece_type][rotation][i][j];
            }
        }
    }
}

void TetrisGame::applyAction(uint8_t action) {
    // LEFT, RIGHT, DOWN, CW, CCW, DROP, SWAP, NOOP
    // need to transform the current tetrimino
    switch(action) {
        case Action::LEFT:
            current_x += -1;
            current_y += -1; // always move down one row
            break;
        case Action::RIGHT:
            current_x += 1;
            current_y += -1;
            break;
        case Action::DOWN:
            current_y+= -1;
            break;
        case Action::CW:
            rotation = (rotation + 1) % 4;
            break;
        case Action::CCW:
            rotation = (rotation - 1 + 4) % 4;
            break;
        case Action::DROP:
            while (!checkCollision()) {
                current_y += -1;
            }
            current_y += 1;
            break;
        case Action::SWAP:
            // need to check if swapping will cause collision
            holder_type = current_piece_type;
            current_piece_type = getNextPiece();
            if (checkCollision()) {
                // revert swap
                current_piece_type = holder_type;
                holder_type = setLastPiece(current_piece_type);
            }
        default:
            // invalid action
            break;
    }
}

void TetrisGame::updateGameState() {
    // need to lock piece and clear lines if we have reached a point of no return
    current_y -= 1;
    if (checkCollision()) {
        // lock piece
        lockPiece();
        // clear lines
        scored += clearLines();
        score += scored;
        return;
    }
    current_y += 1;
}

void TetrisGame::updateObservation() {
    // Clear active_tetromino first
    for (int y = 0; y < Observation::BoardH; y++) {
        for (int x = 0; x < Observation::BoardW; x++) {
            obs.active_tetromino[y][x] = 0;
        }
    }

    // Update active_tetromino at current position
    for (int y = 0; y < Tetris::PIECE_SIZE; y++) {
        for (int x = 0; x < Tetris::PIECE_SIZE; x++) {
            if (Tetris::PIECES[current_piece_type][rotation][y][x]) {
                obs.active_tetromino[current_y + y][current_x + x] = 1;
            }
        }
    }

    // Update holder piece display
    for (int y = 0; y < Tetris::PIECE_SIZE; y++) {
        for (int x = 0; x < Tetris::PIECE_SIZE; x++) {
            if (holder_type < 7) {
                obs.holder[y][x] = Tetris::PIECES[holder_type][0][y][x];
            } else {
                obs.holder[y][x] = 0;
            }
        }
    }

    // Update queue display - show upcoming pieces
    for (int i = 0; i < queue_size; i++) {
        uint8_t piece_type = queue[(queue_index + i) % queue_size];
        for (int y = 0; y < Tetris::PIECE_SIZE; y++) {
            for (int x = 0; x < Tetris::PIECE_SIZE; x++) {
                obs.queue[i * Tetris::PIECE_SIZE + y][x] = Tetris::PIECES[piece_type][0][y][x];
            }
        }
    }
}

StepResult TetrisGame::step(int action) {
    // This is the pattern
    // apply action
    applyAction(action);
    // update gravity, mechanics, collision, lock, clear lines
    updateGameState();
    updateObservation();

    // compute reward based on the above
    return StepResult{
        obs,
        getReward(),
        game_over
    };
}

bool TetrisGame::isGameOver() {
    return game_over;
}

float TetrisGame::getReward() {
    // TODO: need to make this more robust and interesting probably
    return scored * 1.0f;
}

bool TetrisGame::checkCollision() {
    for (int y = 0; y < Tetris::PIECE_SIZE; y++) {
        for (int x = 0; x < Tetris::PIECE_SIZE; x++) {
            if (Tetris::PIECES[current_piece_type][rotation][y][x] && obs.board[current_y + y][current_x + x]) {
                 return true;
            }
        }
    }
}

void TetrisGame::spawnPiece() {
    // get next piece
    current_piece_type = getNextPiece();
    current_x = (Tetris::BOARD_WIDTH / 2);
    current_y = Tetris::BOARD_HEIGHT - 1;
    rotation = 0;
}

void TetrisGame::lockPiece() {
    // lock piece
    for (int y = 0; y < Tetris::PIECE_SIZE; y++) {
        for (int x = 0; x < Tetris::PIECE_SIZE; x++) {
            if (Tetris::PIECES[current_piece_type][rotation][y][x]) {
                obs.board[current_y + y][current_x + x] = current_piece_type;
            }
        }
    }
}

int TetrisGame::clearLine(uint8_t row) {
    // need to copy the row above recursively
    for (int y = row; y > 0; y--) {
        for (int x = 0; x < Tetris::BOARD_WIDTH; x++) {
            obs.board[row][x] = obs.board[row - 1][x];
        }
    }
    return 1;
}

int TetrisGame::clearLines() {
    int scored = 0;
    for (int y = 0; y < Tetris::PIECE_SIZE; y++) {
        for (int x = 0; x < Tetris::BOARD_WIDTH; x++) {
            // find a space in the row
            if (!obs.board[current_y + y][current_x + x]) {
                break;
            }
            scored += clearLine(y);
        }
    }

    return scored;
}

void TetrisGame::loop() {
    double accumulate = 0.0;

    // this is our main game loop
    while (!isGameOver()) {
        accumulate += tm.getDeltaTime();

        while (accumulate >= Tetris::TICK_RATE) {
            // get action
            int action = Input::getAction();

            // update game
            step(action);
            accumulate -= Tetris::TICK_RATE;

            // render if needed
            if (tm.needRendering()) {
                Renderer::render(obs);
            }
        }
    }
}
