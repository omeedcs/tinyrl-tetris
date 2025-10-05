#pragma once
#include "tetrisGame.h"
#include <SDL.h>
#include <SDL_ttf.h>

// Add QUIT action to enum
enum ActionSDL {
    QUIT = 255
};

namespace SDLRenderer {
    bool init();
    void cleanup();
    void render(const Observation& obs, int score = 0, bool game_over = false, const std::vector<int>& clearing_lines = {});
    Action handleInput();
}
