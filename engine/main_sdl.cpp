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
        
        // Apply user input immediately (except NOOP and DOWN which are gravity)
        if (action != Action::NOOP && action != Action::DOWN) {
            game.applyAction(static_cast<uint8_t>(action));
            game.updateObservation();
        }
        
        // Apply gravity/down movement on tick
        while (accumulate >= Tetris::TICK_RATE) {
            // Apply DOWN if user pressed it, otherwise just gravity
            if (action == Action::DOWN) {
                game.step(static_cast<int>(action));
            } else {
                game.updateGameState();  // Just gravity, no user action
            }
            accumulate -= Tetris::TICK_RATE;
        }
        
        // Render at full frame rate
        SDLRenderer::render(game.obs, game.score, game.game_over);
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
