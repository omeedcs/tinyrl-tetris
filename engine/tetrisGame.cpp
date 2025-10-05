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

    // Initialize queue with random pieces
    queue.resize(queue_size);
    for (int i = 0; i < queue_size; i++) {
        queue[i] = rand() % 7;
    }

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
    // Clear the board
    for (int y = 0; y < Observation::BoardH; y++) {
        for (int x = 0; x < Observation::BoardW; x++) {
            obs.board[y][x] = 0;
            obs.active_tetromino[y][x] = 0;
        }
    }
    
    // Clear holder
    for (int y = 0; y < Tetris::PIECE_SIZE; y++) {
        for (int x = 0; x < Tetris::PIECE_SIZE; x++) {
            obs.holder[y][x] = 0;
        }
    }
    
    // Reset game state
    score = 0;
    scored = 0;
    game_over = false;
    holder_type = 7;
    clearing_lines.clear();
    
    // Reset queue with new random pieces
    for (int i = 0; i < queue_size; i++) {
        queue[i] = rand() % 7;
    }
    queue_index = 0;
    
    // Spawn first piece
    rotation = 0;
    current_x = (Tetris::BOARD_WIDTH / 2);
    current_y = Tetris::BOARD_HEIGHT - 1;
    current_piece_type = getNextPiece();
    
    if (checkCollision()) {
        game_over = true;
    }
    
    updateObservation();
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
    return next_piece;
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
    int old_x = current_x;
    int old_y = current_y;
    uint8_t old_rotation = rotation;
    
    switch(action) {
        case Action::LEFT:
            current_x -= 1;
            if (checkCollision()) {
                current_x = old_x;  // Revert if collision
            }
            break;
        case Action::RIGHT:
            current_x += 1;
            if (checkCollision()) {
                current_x = old_x;  // Revert if collision
            }
            break;
        case Action::DOWN:
            current_y -= 1;
            if (checkCollision()) {
                current_y = old_y;  // Revert if collision
            }
            break;
        case Action::CW:
            rotation = (rotation + 1) % 4;
            if (checkCollision()) {
                rotation = old_rotation;  // Revert if collision
            }
            break;
        case Action::CCW:
            rotation = (rotation - 1 + 4) % 4;
            if (checkCollision()) {
                rotation = old_rotation;  // Revert if collision
            }
            break;
        case Action::DROP:
            while (!checkCollision()) {
                current_y -= 1;
            }
            current_y += 1;  // Back up to last valid position
            // Lock the piece immediately after hard drop
            lockPiece();
            scored = clearLines();
            score += scored;
            spawnPiece();
            if (checkCollision()) {
                game_over = true;
            }
            break;
        case Action::SWAP:
            // Swap with hold piece
            if (holder_type == 7) {
                // No piece in holder, just move current piece there
                holder_type = current_piece_type;
                current_piece_type = getNextPiece();
                current_x = (Tetris::BOARD_WIDTH / 2);
                current_y = Tetris::BOARD_HEIGHT - 1;
                rotation = 0;
            } else {
                // Swap with held piece
                uint8_t temp = current_piece_type;
                current_piece_type = holder_type;
                holder_type = temp;
                current_x = (Tetris::BOARD_WIDTH / 2);
                current_y = Tetris::BOARD_HEIGHT - 1;
                rotation = 0;
            }
            if (checkCollision()) {
                // Can't swap - revert (shouldn't happen in normal play)
                game_over = true;
            }
            break;
        default:
            // invalid action or NOOP
            break;
    }
}

void TetrisGame::updateGameState() {
    // need to lock piece and clear lines if we have reached a point of no return
    current_y -= 1;
    if (checkCollision()) {
        current_y += 1;  // Restore position before locking
        // lock piece
        lockPiece();
        // clear lines
        scored = clearLines();
        score += scored;
        // spawn new piece
        spawnPiece();
        if (checkCollision()) {
            game_over = true;
        }
    }
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
                int board_y = current_y + y;
                int board_x = current_x + x;
                // Bounds check before writing
                if (board_y >= 0 && board_y < Observation::BoardH &&
                    board_x >= 0 && board_x < Observation::BoardW) {
                    obs.active_tetromino[board_y][board_x] = 1;
                }
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
            if (Tetris::PIECES[current_piece_type][rotation][y][x]) {
                int board_x = current_x + x;
                int board_y = current_y + y;
                
                // Check boundaries - use actual playable board size
                if (board_x < 0 || board_x >= Tetris::BOARD_WIDTH ||
                    board_y < 0 || board_y >= Observation::BoardH) {
                    return true;
                }
                
                // Check collision with existing pieces
                if (obs.board[board_y][board_x]) {
                    return true;
                }
            }
        }
    }
    return false;
}

void TetrisGame::spawnPiece() {
    // get next piece
    current_piece_type = getNextPiece();
    current_x = (Tetris::BOARD_WIDTH / 2);
    current_y = Tetris::BOARD_HEIGHT - 1;
    rotation = 0;
}

void TetrisGame::lockPiece() {
    // lock piece (store piece_type + 1 so 0 remains empty)
    for (int y = 0; y < Tetris::PIECE_SIZE; y++) {
        for (int x = 0; x < Tetris::PIECE_SIZE; x++) {
            if (Tetris::PIECES[current_piece_type][rotation][y][x]) {
                int board_y = current_y + y;
                int board_x = current_x + x;
                // Bounds check before writing
                if (board_y >= 0 && board_y < Observation::BoardH &&
                    board_x >= 0 && board_x < Observation::BoardW) {
                    obs.board[board_y][board_x] = current_piece_type + 1;
                }
            }
        }
    }
}

int TetrisGame::clearLine(uint8_t row) {
    // Shift all rows above down by one
    for (int y = row; y < Tetris::BOARD_HEIGHT - 1; y++) {
        for (int x = 0; x < Tetris::BOARD_WIDTH; x++) {
            obs.board[y][x] = obs.board[y + 1][x];
        }
    }
    // Clear the top row
    for (int x = 0; x < Tetris::BOARD_WIDTH; x++) {
        obs.board[Tetris::BOARD_HEIGHT - 1][x] = 0;
    }
    return 1;
}

int TetrisGame::clearLines() {
    clearing_lines.clear();
    
    // Check rows where the current piece was placed
    for (int y = 0; y < Tetris::PIECE_SIZE; y++) {
        int row = current_y + y;
        if (row < 0 || row >= Tetris::BOARD_HEIGHT) continue;
        
        // Check if this row is full
        bool is_full = true;
        for (int x = 0; x < Tetris::BOARD_WIDTH; x++) {
            if (obs.board[row][x] == 0) {
                is_full = false;
                break;
            }
        }
        
        if (is_full) {
            clearing_lines.push_back(row);
        }
    }

    // Return number of lines to clear (actual clearing happens after animation)
    return clearing_lines.size();
}

void TetrisGame::completeClearLines() {
    // Actually clear the marked lines
    for (int row : clearing_lines) {
        clearLine(row);
    }
    clearing_lines.clear();
}

#ifndef NO_TERMINAL_LOOP
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
#endif
