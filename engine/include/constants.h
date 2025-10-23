#pragma once

namespace Tetris {

    const double FRAMERATE = 60.0;
    const double RENDER_RATE = 1.0 / FRAMERATE;  // Render at 60 FPS
    const double TICK_RATE = 0.5;  // Gravity happens every 0.5 seconds (twice per second)
    const int BOARD_HEIGHT = 20;
    const int BOARD_WIDTH = 10;
    const int PIECE_SIZE = 4;
}
