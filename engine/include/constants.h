#pragma once

namespace Tetris {

    const double FRAMERATE = 60.0;
    const double RENDER_RATE = 1.0 / FRAMERATE;  // Render at 60 FPS
    const double TICK_RATE = 1.0;  // Gravity happens every 1.0 seconds (once per second)
    const int BOARD_HEIGHT = 20;
    const int BOARD_WIDTH = 10;
    const int PIECE_SIZE = 4;
}
