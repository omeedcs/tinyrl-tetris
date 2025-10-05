#include "sdl_renderer.h"
#include "constants.h"
#include <iostream>

namespace SDLRenderer {
    const int CELL_SIZE = 30;
    const int BOARD_OFFSET_X = 50;
    const int BOARD_OFFSET_Y = 50;
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 700;
    
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;
    TTF_Font* titleFont = nullptr;
    
    // Piece colors (Tetris standard colors)
    const SDL_Color COLORS[] = {
        {40, 40, 40, 255},      // Empty - dark gray
        {0, 240, 240, 255},     // I - Cyan
        {240, 240, 0, 255},     // O - Yellow
        {160, 0, 240, 255},     // T - Purple
        {0, 240, 0, 255},       // S - Green
        {240, 0, 0, 255},       // Z - Red
        {0, 0, 240, 255},       // J - Blue
        {240, 160, 0, 255},     // L - Orange
    };
    
    const SDL_Color BG_COLOR = {20, 20, 30, 255};
    const SDL_Color GRID_COLOR = {50, 50, 60, 255};
    const SDL_Color TEXT_COLOR = {200, 200, 200, 255};
    const SDL_Color ACTIVE_PIECE_COLOR = {255, 255, 100, 255};
    
    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
            return false;
        }
        
        if (TTF_Init() < 0) {
            std::cerr << "TTF init failed: " << TTF_GetError() << std::endl;
            return false;
        }
        
        window = SDL_CreateWindow(
            "Tetris - TinyRL",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN
        );
        
        if (!window) {
            std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
            return false;
        }
        
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer) {
            std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // Try to load system fonts
        font = TTF_OpenFont("/System/Library/Fonts/Supplemental/Arial.ttf", 18);
        if (!font) {
            font = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 18);
        }
        
        titleFont = TTF_OpenFont("/System/Library/Fonts/Supplemental/Arial Bold.ttf", 32);
        if (!titleFont) {
            titleFont = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 32);
        }
        
        return true;
    }
    
    void cleanup() {
        if (font) TTF_CloseFont(font);
        if (titleFont) TTF_CloseFont(titleFont);
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
    }
    
    void drawText(const char* text, int x, int y, TTF_Font* f = nullptr, SDL_Color color = TEXT_COLOR) {
        if (!f) f = font;
        if (!f) return;
        
        SDL_Surface* surface = TTF_RenderText_Blended(f, text, color);
        if (!surface) return;
        
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!texture) {
            SDL_FreeSurface(surface);
            return;
        }
        
        SDL_Rect destRect = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &destRect);
        
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
    
    void drawCell(int x, int y, const SDL_Color& color, bool isActive = false) {
        SDL_Rect rect = {
            BOARD_OFFSET_X + x * CELL_SIZE,
            BOARD_OFFSET_Y + (Tetris::BOARD_HEIGHT - 1 - y) * CELL_SIZE,
            CELL_SIZE - 2,
            CELL_SIZE - 2
        };
        
        if (isActive) {
            // Active piece - brighter with glow effect
            SDL_SetRenderDrawColor(renderer, ACTIVE_PIECE_COLOR.r, ACTIVE_PIECE_COLOR.g, 
                                   ACTIVE_PIECE_COLOR.b, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        }
        SDL_RenderFillRect(renderer, &rect);
        
        // Draw highlight for 3D effect
        SDL_SetRenderDrawColor(renderer, 
            std::min(255, color.r + 40), 
            std::min(255, color.g + 40), 
            std::min(255, color.b + 40), 255);
        SDL_Rect highlight = {rect.x + 2, rect.y + 2, rect.w - 4, 4};
        SDL_RenderFillRect(renderer, &highlight);
        
        // Draw shadow
        SDL_SetRenderDrawColor(renderer, 
            color.r / 2, color.g / 2, color.b / 2, 255);
        SDL_Rect shadow = {rect.x + 2, rect.y + rect.h - 6, rect.w - 4, 4};
        SDL_RenderFillRect(renderer, &shadow);
    }
    
    void drawGrid() {
        SDL_SetRenderDrawColor(renderer, GRID_COLOR.r, GRID_COLOR.g, GRID_COLOR.b, 255);
        
        // Vertical lines
        for (int x = 0; x <= Tetris::BOARD_WIDTH; x++) {
            SDL_RenderDrawLine(renderer,
                BOARD_OFFSET_X + x * CELL_SIZE,
                BOARD_OFFSET_Y,
                BOARD_OFFSET_X + x * CELL_SIZE,
                BOARD_OFFSET_Y + Tetris::BOARD_HEIGHT * CELL_SIZE
            );
        }
        
        // Horizontal lines
        for (int y = 0; y <= Tetris::BOARD_HEIGHT; y++) {
            SDL_RenderDrawLine(renderer,
                BOARD_OFFSET_X,
                BOARD_OFFSET_Y + y * CELL_SIZE,
                BOARD_OFFSET_X + Tetris::BOARD_WIDTH * CELL_SIZE,
                BOARD_OFFSET_Y + y * CELL_SIZE
            );
        }
    }
    
    void drawPreviewPiece(const std::vector<std::vector<uint8_t>>& preview, 
                         int startX, int startY, const char* label) {
        if (font) {
            drawText(label, startX, startY - 25);
        }
        
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (y < preview.size() && x < preview[y].size() && preview[y][x]) {
                    SDL_Rect rect = {
                        startX + x * 20,
                        startY + y * 20,
                        18,
                        18
                    };
                    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
    }
    
    void render(const Observation& obs, int score, bool game_over, const std::vector<int>& clearing_lines) {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, BG_COLOR.r, BG_COLOR.g, BG_COLOR.b, 255);
        SDL_RenderClear(renderer);
        
        // Draw title
        if (titleFont) {
            drawText("TETRIS", 500, 30, titleFont);
        }
        
        // Draw grid
        drawGrid();
        
        // Draw locked pieces
        for (int y = 0; y < Tetris::BOARD_HEIGHT; y++) {
            // Check if this line is being cleared
            bool is_clearing = false;
            for (int clear_y : clearing_lines) {
                if (clear_y == y) {
                    is_clearing = true;
                    break;
                }
            }
            
            for (int x = 0; x < Tetris::BOARD_WIDTH; x++) {
                uint8_t cell = obs.board[y][x];
                if (cell > 0 && cell < 8) {
                    if (is_clearing) {
                        // Flash white for clearing lines
                        SDL_Color white = {255, 255, 255, 255};
                        drawCell(x, y, white, false);
                    } else {
                        drawCell(x, y, COLORS[cell], false);
                    }
                }
            }
        }
        
        // Draw active piece on top (unless lines are clearing)
        if (clearing_lines.empty()) {
            for (int y = 0; y < Tetris::BOARD_HEIGHT; y++) {
                for (int x = 0; x < Tetris::BOARD_WIDTH; x++) {
                    if (obs.active_tetromino[y][x]) {
                        drawCell(x, y, ACTIVE_PIECE_COLOR, true);
                    }
                }
            }
        }
        
        // Draw score
        if (font) {
            char scoreText[64];
            snprintf(scoreText, sizeof(scoreText), "SCORE: %d", score);
            drawText(scoreText, 500, 80);
        }
        
        // Draw next piece
        int nextX = 500;
        int nextY = 150;
        drawPreviewPiece(obs.queue, nextX, nextY, "NEXT:");
        
        // Draw hold piece
        int holdX = 500;
        int holdY = 300;
        drawPreviewPiece(obs.holder, holdX, holdY, "HOLD:");
        
        // Draw controls
        if (font) {
            drawText("CONTROLS:", 500, 400);
            drawText("A/D - Move", 500, 430);
            drawText("W - Rotate", 500, 455);
            drawText("S - Soft Drop", 500, 480);
            drawText("SPACE - Hard Drop", 500, 505);
            drawText("C - Hold", 500, 530);
            drawText("R - Reset", 500, 555);
            drawText("ESC - Quit", 500, 580);
        }
        
        // Game over overlay
        if (game_over) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
            SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
            SDL_RenderFillRect(renderer, &overlay);
            
            if (titleFont) {
                SDL_Color red = {255, 50, 50, 255};
                drawText("GAME OVER", 250, 250, titleFont, red);
            }
            
            if (font) {
                char finalScore[64];
                snprintf(finalScore, sizeof(finalScore), "Final Score: %d", score);
                drawText(finalScore, 280, 320);
                drawText("Press R to restart", 265, 360);
                drawText("Press ESC to quit", 270, 390);
            }
        }
        
        SDL_RenderPresent(renderer);
    }
    
    Action handleInput() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return static_cast<Action>(255);  // QUIT
            }
            
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                    case SDLK_q:
                        return static_cast<Action>(255);  // QUIT
                    case SDLK_r:
                        return static_cast<Action>(254);  // RESET
                    case SDLK_a:
                    case SDLK_LEFT:
                        return Action::LEFT;
                    case SDLK_d:
                    case SDLK_RIGHT:
                        return Action::RIGHT;
                    case SDLK_s:
                    case SDLK_DOWN:
                        return Action::DOWN;
                    case SDLK_w:
                    case SDLK_UP:
                        return Action::CW;
                    case SDLK_e:
                        return Action::CCW;
                    case SDLK_SPACE:
                        return Action::DROP;
                    case SDLK_c:
                        return Action::SWAP;
                }
            }
        }
        return Action::NOOP;
    }
}
