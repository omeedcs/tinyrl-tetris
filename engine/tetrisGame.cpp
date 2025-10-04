#include "tetrisGame.h"
#include "pieces.h"
#include "tetrisGame.h"
#include "pieces.h"
#include "constants.h"
#include "input.h"
#include "renderer.h"
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
    
    // TODO: check for collision here
    if (checkCollision()) {
        game_over = true;
    }
    
    // adjusts current matrix with newly minted values
    updateActiveMask();
}

void TetrisGame::reset() {
    // TODO: Implement reset logic
}

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
                obs.active_tetromino[current_y + i][current_x + j] = 
                    Tetris::PIECES[current_piece_type][rotation][i][j];
            }
        }
    }
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

float TetrisGame::getReward() {
    // TODO
    return 0.0f;
}

bool TetrisGame::checkCollision() {
    // TODO
    return false;
}

void TetrisGame::spawnPiece() {
    // TODO
}

void TetrisGame::lockPiece() {
    // TODO
}

int TetrisGame::clearLines() {
    // TODO
    return 0;
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
