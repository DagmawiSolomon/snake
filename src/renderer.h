#ifndef RENDERER_H
#define RENDERER_H

#include "game_state.h"
#include "camera.h"

class Renderer {
public:
    Renderer();

    void initGL();
    void render(const GameManager& game, const OrbitCamera& camera, int windowWidth, int windowHeight, float animTime);

private:
    void renderCubeAndGrids();
    void renderSnake(const Snake& snake, float animTime);
    void renderFood(const GridPos& food, float animTime);
    void renderCellBox(Face face, int u, int v, float scale, float height, float r, float g, float b);
    void renderHUD(const GameManager& game, int width, int height);

    void drawText2D(float x, float y, const char* str, void* font = nullptr, float r = 1.0f, float g = 1.0f, float b = 1.0f);
};

#endif // RENDERER_H
