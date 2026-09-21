#ifndef UI_H
#define UI_H

#include "game_state.h"

class RetroArcadeUI {
public:
    RetroArcadeUI();

    void update(float deltaTime);
    void triggerLossEffects();

    // Main 2D overlay entry point
    void render(const GameManager& game, int windowWidth, int windowHeight, float animTime);

    // Pixel text drawing helpers
    static void drawText(const char* str, float x, float y, float pixelSize,
                         float r, float g, float b, bool centered = false, int windowWidth = 0);
    static float getTextWidth(const char* str, float pixelSize);

private:
    void renderMenu(const GameManager& game, int width, int height, float animTime);
    void renderPlayingHUD(const GameManager& game, int width, int height);
    void renderPaused(int width, int height, float animTime);
    void renderGameOver(const GameManager& game, int width, int height, float animTime);

    void renderScanlines(int width, int height);
    void renderCRTBezel(int width, int height);
    void renderScreenFlash(int width, int height);

    float m_blinkTimer;
    float m_flashTimer;
    float m_shakeTimer;
};

#endif // UI_H
