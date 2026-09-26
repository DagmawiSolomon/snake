"""
Palette definitions for Snake on a Cube (Retro Arcade Aesthetics).
"""

from typing import Tuple

Color = Tuple[float, float, float]

# Defined Retro Arcade Color Palette
# Black:       #000000 (Screen background)
# Neon Green:  #33FF33 (Title text)
# Amber:       #FFB000 (Blinking prompt, secondary text, scores)
# Bright Cyan: #33FFFF (Snake head)
# Body Green:  #33CC33 (Snake body)
# Red:         #FF3333 (Food, Game-over text)
# Dim Green:   #115511 (Cube face grid lines & outline)

BLACK: Color = (0.0, 0.0, 0.0)         # #000000
NEON_GREEN: Color = (0.2, 1.0, 0.2)    # #33FF33
AMBER: Color = (1.0, 0.69, 0.0)        # #FFB000
CYAN: Color = (0.2, 1.0, 1.0)          # #33FFFF
BODY_GREEN: Color = (0.2, 0.8, 0.2)    # #33CC33
RED: Color = (1.0, 0.2, 0.2)           # #FF3333
DIM_GREEN: Color = (0.067, 0.333, 0.067)  # #115511

SCANLINE_ALPHA: float = 0.25           # #000000 @ 0.25 alpha
