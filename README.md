# Snake on a Cube (PyOpenGL 3D)

A 3D Retro-Arcade Snake Game played across the 6 faces of an interconnected 3D cube, implemented in **pure Python** using **PyOpenGL** and **Pygame**.

Branch: `pyopengl`

---

## Documentation

For a comprehensive explanation of the mathematics behind the cube manifold, face bases, 24 boundary transitions, orbit camera smoothing, and retro arcade rendering pipeline, see:
- [Snake_on_a_Cube_Technical_Documentation.pdf](file:///d:/snake/docs/Snake_on_a_Cube_Technical_Documentation.pdf) (located in `docs/`)


---

## Features

- **True 3D Cube Topology**: Seamless movement across all 6 cube faces (+X, -X, +Y, -Y, +Z, -Z) with mathematically verified boundary transitions.
- **Dynamic Orbit Camera**: Automatically tracks the active face the snake is crawling on with smooth, frame-rate independent exponential interpolation (`lerp`).
- **Retro Arcade Aesthetics**:
  - CRT scanlines overlay.
  - CRT monitor bezel frame.
  - Custom 5x7 bitmap pixel font.
  - Palette faithful to vintage green/amber arcade displays: Neon Green (`#33FF33`), Amber (`#FFB000`), Cyan (`#33FFFF`), Body Green (`#33CC33`), Food Red (`#FF3333`), and Dim Green gridlines (`#115511`).
  - Screen impact flash on crash.
  - Bobbing and spinning 3D food items.
  - 3D snake head with directional pupil eyes.
- **Classic Arcade State Flow**: `MENU` -> `PLAYING` -> `PAUSED` -> `GAME OVER`.

---

## Project Structure

```
snake/
├── docs/                          # Detailed technical documentation
│   └── README.md
├── snake/                         # Core game package
│   ├── __init__.py                # Package exports
│   ├── camera.py                  # OrbitCamera with exponential smoothing
│   ├── cube_topology.py           # Vector math, face bases, 24 transitions
│   ├── game_state.py              # GameManager, Snake, collision, food logic
│   ├── palette.py                 # Retro arcade color constants
│   ├── renderer.py                # 3D OpenGL cube & snake rendering
│   └── ui.py                      # 2D HUD, 5x7 bitmap font, CRT scanlines
├── tests/                         # Test suite
│   ├── __init__.py
│   └── test_transitions.py       # Topology reversibility & loop unit tests
├── .gitignore                     # Python-specific gitignore rules
├── main.py                        # Executable entry point
├── requirements.txt               # Dependencies (PyOpenGL, pygame)
├── run.bat                        # Windows batch launcher
└── README.md                      # Project overview
```

---

## Installation & Running

### Requirements
- Python 3.8+
- PyOpenGL (`pip install PyOpenGL`)
- Pygame (`pip install pygame`)

### Quick Start
```bash
# 1. Install dependencies
pip install -r requirements.txt

# 2. Run the game
python main.py
```

Or on Windows:
```cmd
run.bat
```

---

## Controls

| Key | Action |
|---|---|
| **SPACE / ENTER** | Start Game / Pause & Resume / Return to Menu |
| **W / A / S / D** | Steer Snake (Up, Left, Down, Right) |
| **Arrow Keys** | Steer Snake (Up, Left, Down, Right) |
| **P** | Pause / Resume |
| **F11 / F** | Toggle Fullscreen / Windowed Mode |
| **R** | Restart game |
| **M** | Return to Menu (from Game Over or Pause) |
| **1 - 6** | Quick camera face inspection (+X, -X, +Y, -Y, +Z, -Z) |
| **ESC** | Exit game |


---

## Testing

Run the topology transition verification and game mechanics test suite:
```bash
python -m unittest discover tests
```
This tests:
1. **Reversibility**: Validates that all 24 boundary crossings can be traversed in reverse with 100% positional accuracy.
2. **Great Circle Closed Loops**: Confirms that continuous straight movement along any path completes a 4-face closed loop returning to the origin in `4 * GRID_N` steps.
3. **Game Mechanics**: Simulates food spawning, consumption, body growth, and scoring.