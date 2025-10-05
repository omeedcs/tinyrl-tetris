#pragma once
#include "tetrisGame.h"
#include <SDL.h>
#include <SDL_ttf.h>

// Add special actions to enum
enum ActionSDL {
    RESET = 254,
    QUIT = 255
};

namespace SDLRenderer {
    bool init();
    void cleanup();
    void render(const Observation& obs, int score = 0, bool game_over = false, const std::vector<int>& clearing_lines = {});
    Action handleInput();
}
