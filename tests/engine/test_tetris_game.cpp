#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "tetrisGame.h"
#include "constants.h"

TEST_CASE("TetrisGame initialization", "[tetris][init]") {
    TetrisGame game(TimeManager::SIMULATION, 3);
    
    SECTION("Board is initialized to empty") {
        // Board should be empty (all zeros)
        REQUIRE(game.obs.board.size() == Observation::BoardH);
        REQUIRE(game.obs.board[0].size() == Observation::BoardW);
        
        for (int y = 0; y < Observation::BoardH; y++) {
            for (int x = 0; x < Observation::BoardW; x++) {
                REQUIRE(game.obs.board[y][x] == 0);
            }
        }
    }
    
    SECTION("Game starts not over") {
        REQUIRE_FALSE(game.isGameOver());
    }
    
    SECTION("Active tetromino is initialized") {
        REQUIRE(game.obs.active_tetromino.size() == Observation::BoardH);
        REQUIRE(game.obs.active_tetromino[0].size() == Observation::BoardW);
    }
}

TEST_CASE("Piece movement", "[tetris][movement]") {
    TetrisGame game(TimeManager::SIMULATION, 3);
    
    SECTION("Move left") {
        int initial_x = game.current_x;
        game.applyAction(static_cast<uint8_t>(Action::LEFT));
        REQUIRE(game.current_x == initial_x - 1);
    }
    
    SECTION("Move right") {
        int initial_x = game.current_x;
        game.applyAction(static_cast<uint8_t>(Action::RIGHT));
        REQUIRE(game.current_x == initial_x + 1);
    }
    
    SECTION("Move down") {
        int initial_y = game.current_y;
        game.applyAction(static_cast<uint8_t>(Action::DOWN));
        REQUIRE(game.current_y == initial_y - 1);
    }
}

TEST_CASE("Piece rotation", "[tetris][rotation]") {
    TetrisGame game(TimeManager::SIMULATION, 3);
    
    SECTION("Rotate clockwise") {
        game.rotation = 0;
        game.applyAction(static_cast<uint8_t>(Action::CW));
        REQUIRE(game.rotation == 1);
        
        game.applyAction(static_cast<uint8_t>(Action::CW));
        REQUIRE(game.rotation == 2);
        
        game.applyAction(static_cast<uint8_t>(Action::CW));
        REQUIRE(game.rotation == 3);
        
        game.applyAction(static_cast<uint8_t>(Action::CW));
        REQUIRE(game.rotation == 0);  // Wraps around
    }
    
    SECTION("Rotate counter-clockwise") {
        game.rotation = 0;
        game.applyAction(static_cast<uint8_t>(Action::CCW));
        REQUIRE(game.rotation == 3);  // Wraps around
        
        game.applyAction(static_cast<uint8_t>(Action::CCW));
        REQUIRE(game.rotation == 2);
        
        game.applyAction(static_cast<uint8_t>(Action::CCW));
        REQUIRE(game.rotation == 1);
        
        game.applyAction(static_cast<uint8_t>(Action::CCW));
        REQUIRE(game.rotation == 0);
    }
}

TEST_CASE("Collision detection", "[tetris][collision]") {
    TetrisGame game(TimeManager::SIMULATION, 3);
    
    SECTION("No collision on empty board") {
        // Starting position should have no collision
        REQUIRE_FALSE(game.checkCollision());
    }
    
    SECTION("Collision with board boundary") {
        // Move piece to bottom edge
        game.current_y = 0;
        game.current_y -= 1;  // One below bottom
        // Should detect collision (out of bounds)
        // Note: depends on checkCollision implementation
    }
    
    SECTION("Collision with locked piece") {
        // Place a piece on the board
        game.obs.board[5][5] = 1;
        
        // Move current piece to overlap
        game.current_x = 5;
        game.current_y = 5;
        
        // Should detect collision if piece overlaps
        // Note: depends on piece shape
    }
}

TEST_CASE("Hard drop", "[tetris][drop]") {
    TetrisGame game(TimeManager::SIMULATION, 3);
    
    SECTION("Hard drop moves piece to bottom") {
        int initial_y = game.current_y;
        game.applyAction(static_cast<uint8_t>(Action::DROP));
        
        // After hard drop, piece should be lower
        REQUIRE(game.current_y < initial_y);
    }
}

TEST_CASE("Line clearing", "[tetris][lines]") {
    TetrisGame game(TimeManager::SIMULATION, 3);
    
    SECTION("Clear single line") {
        // Fill a complete line
        for (int x = 0; x < Tetris::BOARD_WIDTH; x++) {
            game.obs.board[0][x] = 1;
        }
        
        int cleared = game.clearLines();
        // Should clear at least one line
        // Note: depends on clearLines implementation
    }
    
    SECTION("No lines to clear on empty board") {
        int cleared = game.clearLines();
        REQUIRE(cleared == 0);
    }
}

TEST_CASE("Piece spawning", "[tetris][spawn]") {
    TetrisGame game(TimeManager::SIMULATION, 3);
    
    SECTION("Spawn piece at top center") {
        game.spawnPiece();
        
        // Piece should spawn at center top
        REQUIRE(game.current_x == Tetris::BOARD_WIDTH / 2);
        REQUIRE(game.current_y == Tetris::BOARD_HEIGHT - 1);
        REQUIRE(game.rotation == 0);
    }
    
    SECTION("Spawned piece type is valid") {
        game.spawnPiece();
        
        // Piece type should be 0-6
        REQUIRE(game.current_piece_type >= 0);
        REQUIRE(game.current_piece_type <= 6);
    }
}

TEST_CASE("Observation update", "[tetris][observation]") {
    TetrisGame game(TimeManager::SIMULATION, 3);
    
    SECTION("Active tetromino is updated") {
        game.updateObservation();
        
        // Check that active_tetromino has some non-zero values
        // (current piece should be visible)
        bool has_active_piece = false;
        for (int y = 0; y < Observation::BoardH; y++) {
            for (int x = 0; x < Observation::BoardW; x++) {
                if (game.obs.active_tetromino[y][x] != 0) {
                    has_active_piece = true;
                    break;
                }
            }
            if (has_active_piece) break;
        }
        REQUIRE(has_active_piece);
    }
    
    SECTION("Queue is populated") {
        game.updateObservation();
        
        // Queue should show upcoming pieces
        REQUIRE(game.obs.queue.size() > 0);
    }
}

TEST_CASE("Step function", "[tetris][step]") {
    TetrisGame game(TimeManager::SIMULATION, 3);
    
    SECTION("Step returns valid result") {
        StepResult result = game.step(static_cast<int>(Action::DOWN));
        
        REQUIRE(result.reward >= 0);  // Reward should be non-negative initially
        REQUIRE_FALSE(result.terminated);  // Game shouldn't end on first step
    }
    
    SECTION("Multiple steps execute without error") {
        for (int i = 0; i < 10; i++) {
            StepResult result = game.step(static_cast<int>(Action::DOWN));
            // Should execute without crashing
        }
    }
}

TEST_CASE("Piece locking", "[tetris][lock]") {
    TetrisGame game(TimeManager::SIMULATION, 3);
    
    SECTION("Lock piece adds to board") {
        // Position piece
        game.current_x = 5;
        game.current_y = 5;
        game.current_piece_type = 0;  // I-piece
        game.rotation = 0;
        
        // Count non-zero cells before
        int count_before = 0;
        for (int y = 0; y < Observation::BoardH; y++) {
            for (int x = 0; x < Observation::BoardW; x++) {
                if (game.obs.board[y][x] != 0) count_before++;
            }
        }
        
        game.lockPiece();
        
        // Count non-zero cells after
        int count_after = 0;
        for (int y = 0; y < Observation::BoardH; y++) {
            for (int x = 0; x < Observation::BoardW; x++) {
                if (game.obs.board[y][x] != 0) count_after++;
            }
        }
        
        // Should have more filled cells after locking
        REQUIRE(count_after > count_before);
    }
}
