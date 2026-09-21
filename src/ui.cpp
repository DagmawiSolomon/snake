#include "ui.h"
#include "palette.h"
#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include <cctype>

// 5x7 bitmap font definitions (each row has 5 bits, MSB is column 0)
static const unsigned char FONT_5X7[][7] = {
    // Space (32)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    // ! (33)
    { 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04 },
    // " (34)
    { 0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00 },
    // # (35)
    { 0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A },
    // $ (36)
    { 0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04 },
    // % (37)
    { 0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03 },
    // & (38)
    { 0x08, 0x14, 0x14, 0x08, 0x15, 0x12, 0x0D },
    // ' (39)
    { 0x0C, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00 },
    // ( (40)
    { 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02 },
    // ) (41)
    { 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08 },
    // * (42)
    { 0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00 },
    // + (43)
    { 0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00 },
    // , (44)
    { 0x00, 0x00, 0x00, 0x00, 0x0C, 0x04, 0x08 },
    // - (45)
    { 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00 },
    // . (46)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C },
    // / (47)
    { 0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00 },
    // 0 (48)
    { 0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E },
    // 1 (49)
    { 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E },
    // 2 (50)
    { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F },
    // 3 (51)
    { 0x1E, 0x01, 0x01, 0x06, 0x01, 0x01, 0x1E },
    // 4 (52)
    { 0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02 },
    // 5 (53)
    { 0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E },
    // 6 (54)
    { 0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E },
    // 7 (55)
    { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 },
    // 8 (56)
    { 0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E },
    // 9 (57)
    { 0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C },
    // : (58)
    { 0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00 },
    // ; (59)
    { 0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x04, 0x08 },
    // < (60)
    { 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02 },
    // = (61)
    { 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00 },
    // > (62)
    { 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08 },
    // ? (63)
    { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04 },
    // @ (64)
    { 0x0E, 0x11, 0x01, 0x0D, 0x15, 0x15, 0x0E },
    // A (65)
    { 0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 },
    // B (66)
    { 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E },
    // C (67)
    { 0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E },
    // D (68)
    { 0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C },
    // E (69)
    { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F },
    // F (70)
    { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10 },
    // G (71)
    { 0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F },
    // H (72)
    { 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 },
    // I (73)
    { 0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E },
    // J (74)
    { 0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C },
    // K (75)
    { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 },
    // L (76)
    { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F },
    // M (77)
    { 0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11 },
    // N (78)
    { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 },
    // O (79)
    { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E },
    // P (80)
    { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10 },
    // Q (81)
    { 0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D },
    // R (82)
    { 0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11 },
    // S (83)
    { 0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E },
    // T (84)
    { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 },
    // U (85)
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E },
    // V (86)
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04 },
    // W (87)
    { 0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A },
    // X (88)
    { 0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11 },
    // Y (89)
    { 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04 },
    // Z (90)
    { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F },
    // [ (91)
    { 0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E },
    // \ (92)
    { 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00 },
    // ] (93)
    { 0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E }
};

RetroArcadeUI::RetroArcadeUI()
    : m_blinkTimer(0.0f),
      m_flashTimer(0.0f),
      m_shakeTimer(0.0f)
{
}

void RetroArcadeUI::update(float deltaTime) {
    m_blinkTimer += deltaTime;
    if (m_flashTimer > 0.0f) {
        m_flashTimer -= deltaTime;
        if (m_flashTimer < 0.0f) m_flashTimer = 0.0f;
    }
    if (m_shakeTimer > 0.0f) {
        m_shakeTimer -= deltaTime;
        if (m_shakeTimer < 0.0f) m_shakeTimer = 0.0f;
    }
}

void RetroArcadeUI::triggerLossEffects() {
    m_flashTimer = 0.45f;
    m_shakeTimer = 0.35f;
}

float RetroArcadeUI::getTextWidth(const char* str, float pixelSize) {
    if (!str) return 0.0f;
    int len = 0;
    for (const char* c = str; *c != '\0'; c++) len++;
    return static_cast<float>(len * 6) * pixelSize;
}

void RetroArcadeUI::drawText(const char* str, float x, float y, float pixelSize,
                             float r, float g, float b, bool centered, int windowWidth)
{
    if (!str) return;

    if (centered && windowWidth > 0) {
        float w = getTextWidth(str, pixelSize);
        x = (windowWidth - w) * 0.5f;
    }

    // Flat saturated pixel quads - no anti-aliased or muddy blending
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    float curX = x;
    float curY = y;
    for (const char* p = str; *p != '\0'; p++) {
        char ch = std::toupper(*p);
        if (ch >= 32 && ch <= 93) {
            const unsigned char* glyph = FONT_5X7[ch - 32];
            for (int row = 0; row < 7; row++) {
                unsigned char rowBits = glyph[row];
                for (int col = 0; col < 5; col++) {
                    if (rowBits & (1 << (4 - col))) {
                        float px = curX + col * pixelSize;
                        float py = curY + row * pixelSize;
                        glVertex2f(px, py);
                        glVertex2f(px + pixelSize, py);
                        glVertex2f(px + pixelSize, py + pixelSize);
                        glVertex2f(px, py + pixelSize);
                    }
                }
            }
        }
        curX += 6.0f * pixelSize;
    }
    glEnd();
}

void RetroArcadeUI::renderScanlines(int width, int height) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Black scanlines at ~25% alpha (#000000 @ 0.25 alpha)
    glColor4f(Palette::Black.r, Palette::Black.g, Palette::Black.b, Palette::ScanlineAlpha);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int y = 0; y < height; y += 3) {
        glVertex2f(0.0f, static_cast<float>(y));
        glVertex2f(static_cast<float>(width), static_cast<float>(y));
    }
    glEnd();

    glDisable(GL_BLEND);
}

void RetroArcadeUI::renderCRTBezel(int width, int height) {
    // Cabinet outer border in pure black (#000000)
    float w = static_cast<float>(width);
    float h = static_cast<float>(height);
    float border = 8.0f;

    glColor3f(Palette::Black.r, Palette::Black.g, Palette::Black.b);
    glBegin(GL_QUADS);
    // Top
    glVertex2f(0, 0); glVertex2f(w, 0); glVertex2f(w, border); glVertex2f(0, border);
    // Bottom
    glVertex2f(0, h - border); glVertex2f(w, h - border); glVertex2f(w, h); glVertex2f(0, h);
    // Left
    glVertex2f(0, 0); glVertex2f(border, 0); glVertex2f(border, h); glVertex2f(0, h);
    // Right
    glVertex2f(w - border, 0); glVertex2f(w, 0); glVertex2f(w, h); glVertex2f(w - border, h);
    glEnd();

    // Dim green bezel outline (#115511)
    glColor3f(Palette::DimGreen.r, Palette::DimGreen.g, Palette::DimGreen.b);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(border, border);
    glVertex2f(w - border, border);
    glVertex2f(w - border, h - border);
    glVertex2f(border, h - border);
    glEnd();
}

void RetroArcadeUI::renderScreenFlash(int width, int height) {
    if (m_flashTimer <= 0.0f) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float alpha = (m_flashTimer / 0.45f) * 0.5f;
    glColor4f(Palette::Red.r, Palette::Red.g, Palette::Red.b, alpha);

    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(static_cast<float>(width), 0.0f);
    glVertex2f(static_cast<float>(width), static_cast<float>(height));
    glVertex2f(0.0f, static_cast<float>(height));
    glEnd();

    glDisable(GL_BLEND);
}

void RetroArcadeUI::renderMenu(const GameManager& game, int width, int height, float animTime) {
    // Solid flat black start screen per spec - no 3D scene, pure 80s arcade feel

    // 1. Title: Neon Green (#33FF33)
    drawText("SNAKE ON A CUBE", 0, height * 0.22f, 5.0f,
             Palette::NeonGreen.r, Palette::NeonGreen.g, Palette::NeonGreen.b, true, width);

    // 2. High score line: Amber (#FFB000)
    char highBuf[64];
    std::snprintf(highBuf, sizeof(highBuf), "HIGH SCORE  %05d", game.getHighScore());
    drawText(highBuf, 0, height * 0.42f, 3.0f,
             Palette::Amber.r, Palette::Amber.g, Palette::Amber.b, true, width);

    // 3. Blinking prompt: Amber (#FFB000) (500ms toggle)
    bool blinkOn = static_cast<int>(m_blinkTimer * 2.0f) % 2 == 0;
    if (blinkOn) {
        drawText("PRESS SPACE TO START", 0, height * 0.62f, 3.2f,
                 Palette::Amber.r, Palette::Amber.g, Palette::Amber.b, true, width);
    }

    // 4. Controls footer: Amber (#FFB000)
    drawText("[ARROW KEYS / WASD] MOVE", 0, height * 0.84f, 2.0f,
             Palette::Amber.r, Palette::Amber.g, Palette::Amber.b, true, width);
}

void RetroArcadeUI::renderPlayingHUD(const GameManager& game, int width, int height) {
    char buf[64];

    // Score HUD: Amber (#FFB000)
    std::snprintf(buf, sizeof(buf), "SCORE %05d", game.getScore());
    drawText(buf, 24, 20, 2.4f,
             Palette::Amber.r, Palette::Amber.g, Palette::Amber.b);

    // High Score: Amber (#FFB000)
    std::snprintf(buf, sizeof(buf), "HIGH %05d", game.getHighScore());
    drawText(buf, 0, 20, 2.4f,
             Palette::Amber.r, Palette::Amber.g, Palette::Amber.b, true, width);

    // Active Face: Bright Cyan (#33FFFF)
    std::snprintf(buf, sizeof(buf), "FACE %s", faceToString(game.getActiveFace()));
    float faceW = getTextWidth(buf, 2.4f);
    drawText(buf, width - faceW - 24, 20, 2.4f,
             Palette::Cyan.r, Palette::Cyan.g, Palette::Cyan.b);

    // Controls reminder at bottom: Amber (#FFB000)
    drawText("[P] PAUSE", 24, height - 28, 1.8f,
             Palette::Amber.r, Palette::Amber.g, Palette::Amber.b);
}

void RetroArcadeUI::renderPaused(int width, int height, float animTime) {
    // Translucent black backdrop over the paused 3D scene
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(Palette::Black.r, Palette::Black.g, Palette::Black.b, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(static_cast<float>(width), 0);
    glVertex2f(static_cast<float>(width), static_cast<float>(height));
    glVertex2f(0, static_cast<float>(height));
    glEnd();
    glDisable(GL_BLEND);

    // PAUSED: Amber (#FFB000)
    drawText("PAUSED", 0, height * 0.40f, 5.5f,
             Palette::Amber.r, Palette::Amber.g, Palette::Amber.b, true, width);

    // Blinking prompt: Amber (#FFB000)
    bool blinkOn = static_cast<int>(m_blinkTimer * 2.0f) % 2 == 0;
    if (blinkOn) {
        drawText("PRESS P TO RESUME", 0, height * 0.54f, 2.8f,
                 Palette::Amber.r, Palette::Amber.g, Palette::Amber.b, true, width);
    }
}

void RetroArcadeUI::renderGameOver(const GameManager& game, int width, int height, float animTime) {
    // Solid flat black background per spec - no 3D scene behind GAME OVER

    // 1. GAME OVER: Red (#FF3333)
    drawText("GAME OVER", 0, height * 0.28f, 5.5f,
             Palette::Red.r, Palette::Red.g, Palette::Red.b, true, width);

    // 2. Final score: Amber (#FFB000)
    char buf[64];
    std::snprintf(buf, sizeof(buf), "FINAL SCORE  %05d", game.getScore());
    drawText(buf, 0, height * 0.46f, 3.2f,
             Palette::Amber.r, Palette::Amber.g, Palette::Amber.b, true, width);

    // 3. New High Score alert: Neon Green (#33FF33)
    if (game.getScore() > 0 && game.getScore() >= game.getHighScore()) {
        bool flash = static_cast<int>(m_blinkTimer * 4.0f) % 2 == 0;
        if (flash) {
            drawText("** NEW HIGH SCORE **", 0, height * 0.56f, 2.6f,
                     Palette::NeonGreen.r, Palette::NeonGreen.g, Palette::NeonGreen.b, true, width);
        }
    }

    // 4. Prompt to return to MENU: Amber (#FFB000)
    bool blink = static_cast<int>(m_blinkTimer * 2.0f) % 2 == 0;
    if (blink) {
        drawText("PRESS SPACE FOR MENU", 0, height * 0.68f, 3.0f,
                 Palette::Amber.r, Palette::Amber.g, Palette::Amber.b, true, width);
    }

    // 5. Quick restart option: Amber (#FFB000)
    drawText("PRESS R TO RESTART", 0, height * 0.80f, 2.0f,
             Palette::Amber.r, Palette::Amber.g, Palette::Amber.b, true, width);
}

void RetroArcadeUI::render(const GameManager& game, int windowWidth, int windowHeight, float animTime) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, windowHeight, 0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    GameState state = game.getState();
    switch (state) {
        case GameState::MENU:
            renderMenu(game, windowWidth, windowHeight, animTime);
            break;
        case GameState::PLAYING:
            renderPlayingHUD(game, windowWidth, windowHeight);
            break;
        case GameState::PAUSED:
            renderPlayingHUD(game, windowWidth, windowHeight);
            renderPaused(windowWidth, windowHeight, animTime);
            break;
        case GameState::GAME_OVER:
            renderGameOver(game, windowWidth, windowHeight, animTime);
            break;
    }

    // Impact flash
    renderScreenFlash(windowWidth, windowHeight);

    // CRT scanlines (#000000 @ 0.25 alpha)
    renderScanlines(windowWidth, windowHeight);

    // CRT arcade cabinet bezel
    renderCRTBezel(windowWidth, windowHeight);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
}
