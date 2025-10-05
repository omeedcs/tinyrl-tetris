#include "tetrisGame.h"
#include "timeManager.h"
#include "sdl_renderer.h"
#include "constants.h"
#include <iostream>

int main() {
    if (!SDLRenderer::init()) {
        std::cerr << "Failed to initialize SDL" << std::endl;
        return 1;
    }
    
    TetrisGame game(TimeManager::REALTIME, 3);
    double accumulate = 0.0;
    bool quit = false;
    
    // Main game loop
    while (!quit && !game.isGameOver()) {
        accumulate += game.tm.getDeltaTime();
        
        // Handle input - process immediately for responsive controls
        Action action = SDLRenderer::handleInput();
        if (action == static_cast<Action>(255)) {  // QUIT
            quit = true;
            break;
        }
        
        // Apply user input immediately (except NOOP)
        if (action != Action::NOOP) {
            game.applyAction(static_cast<uint8_t>(action));
            game.updateObservation();
        }
        
        // Apply gravity on tick
        while (accumulate >= Tetris::TICK_RATE) {
            game.updateGameState();  // Just gravity, no user action
            game.updateObservation();  // Update visual after gravity
            
            // If lines were cleared, show animation
            if (!game.clearing_lines.empty()) {
                // Show flashing animation for 400ms
                for (int i = 0; i < 4; i++) {
                    SDLRenderer::render(game.obs, game.score, game.game_over, game.clearing_lines);
                    SDL_Delay(100);
                }
                // Actually clear the lines now
                game.completeClearLines();
                game.updateObservation();
            }
            
            accumulate -= Tetris::TICK_RATE;
        }
        
        // Render at full frame rate
        SDLRenderer::render(game.obs, game.score, game.game_over, game.clearing_lines);
    }
    
    // Game over screen
    if (game.isGameOver() && !quit) {
        SDLRenderer::render(game.obs, game.score, true);
        
        // Wait for quit
        while (!quit) {
            Action action = SDLRenderer::handleInput();
            if (action == static_cast<Action>(255)) {
                quit = true;
            }
            SDL_Delay(100);
        }
    }
    
    SDLRenderer::cleanup();
    return 0;
}
