#ifndef RENDERER_H
#define RENDERER_H

#include "game_state.h"
#include "camera.h"
#include "ui.h"

class Renderer {
public:
    Renderer();

    void initGL();
    void update(float deltaTime);
    void triggerLossEffects();
    void render(const GameManager& game, const OrbitCamera& camera, int windowWidth, int windowHeight, float animTime);

private:
    void renderCubeAndGrids();
    void renderSnake(const Snake& snake, float animTime);
    void renderFood(const GridPos& food, float animTime);
    void renderCellBox(Face face, int u, int v, float scale, float height, float r, float g, float b);

    RetroArcadeUI m_ui;
};

#endif // RENDERER_H
