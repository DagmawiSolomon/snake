#ifndef PALETTE_H
#define PALETTE_H

// Defined Retro Arcade Color Palette
// Exact hex values from the spec:
// Black:       #000000 (Screen background)
// Neon Green:  #33FF33 (Title text)
// Amber:       #FFB000 (Blinking prompt, secondary text, scores)
// Bright Cyan: #33FFFF (Snake head)
// Body Green:  #33CC33 (Snake body)
// Red:         #FF3333 (Food, Game-over text)
// Dim Green:   #115511 (Cube face grid lines & outline)

namespace Palette {
    struct Color {
        float r, g, b;
    };

    constexpr Color Black       = { 0.000f, 0.000f, 0.000f }; // #000000
    constexpr Color NeonGreen   = { 0.200f, 1.000f, 0.200f }; // #33FF33
    constexpr Color Amber       = { 1.000f, 0.690f, 0.000f }; // #FFB000
    constexpr Color Cyan        = { 0.200f, 1.000f, 1.000f }; // #33FFFF
    constexpr Color BodyGreen   = { 0.200f, 0.800f, 0.200f }; // #33CC33
    constexpr Color Red         = { 1.000f, 0.200f, 0.200f }; // #FF3333
    constexpr Color DimGreen    = { 0.067f, 0.333f, 0.067f }; // #115511

    constexpr float ScanlineAlpha = 0.25f;                    // #000000 @ 0.25 alpha
}

#endif // PALETTE_H
