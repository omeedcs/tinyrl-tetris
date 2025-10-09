"""
Add the tinyrl_tetris engine to Python path.
Import this at the top of scripts that need the engine.
"""
import sys
from pathlib import Path

# Add engine build directory to path
# __file__ is in rl/src/setup_env.py, so parent.parent.parent gets to project root
project_root = Path(__file__).parent.parent.parent
engine_lib = project_root / "engine" / "build" / "lib"

if not engine_lib.exists():
    raise RuntimeError(
        f"Engine library not found at {engine_lib}. "
        f"Build the engine first with: cd engine && mkdir -p build && cd build && cmake .. && make"
    )

if str(engine_lib) not in sys.path:
    sys.path.insert(0, str(engine_lib))
