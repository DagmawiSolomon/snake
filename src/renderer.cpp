#include "renderer.h"
#include "palette.h"
#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>

Renderer::Renderer() {}

void Renderer::initGL() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Flat retro arcade shading: disable 3D lighting so exact palette colors render true
    glDisable(GL_LIGHTING);
    glShadeModel(GL_FLAT);

    glClearColor(Palette::Black.r, Palette::Black.g, Palette::Black.b, 1.0f);
}

void Renderer::renderCubeAndGrids() {
    // 1. Draw solid black cube faces (#000000)
    glColor3f(Palette::Black.r, Palette::Black.g, Palette::Black.b);
    glBegin(GL_QUADS);
    for (int f = 0; f < 6; f++) {
        Face face = static_cast<Face>(f);

        Vec3 c0 = cellCornerToWorld(face, 0, 0, 0.0f, 0.0f);
        Vec3 c1 = cellCornerToWorld(face, GRID_N - 1, 0, 1.0f, 0.0f);
        Vec3 c2 = cellCornerToWorld(face, GRID_N - 1, GRID_N - 1, 1.0f, 1.0f);
        Vec3 c3 = cellCornerToWorld(face, 0, GRID_N - 1, 0.0f, 1.0f);

        glVertex3f(c0.x, c0.y, c0.z);
        glVertex3f(c1.x, c1.y, c1.z);
        glVertex3f(c2.x, c2.y, c2.z);
        glVertex3f(c3.x, c3.y, c3.z);
    }
    glEnd();

    // 2. Draw interior grid lines for each face in Dim Green (#115511)
    glLineWidth(1.0f);
    glColor3f(Palette::DimGreen.r, Palette::DimGreen.g, Palette::DimGreen.b);

    glBegin(GL_LINES);
    for (int f = 0; f < 6; f++) {
        Face face = static_cast<Face>(f);
        const float eps = 0.003f; // slight offset to prevent z-fighting

        for (int i = 0; i <= GRID_N; i++) {
            // Horizontal lines
            Vec3 h0 = cellCornerToWorld(face, 0, i < GRID_N ? i : GRID_N - 1, 0.0f, i < GRID_N ? 0.0f : 1.0f, eps);
            Vec3 h1 = cellCornerToWorld(face, GRID_N - 1, i < GRID_N ? i : GRID_N - 1, 1.0f, i < GRID_N ? 0.0f : 1.0f, eps);
            glVertex3f(h0.x, h0.y, h0.z);
            glVertex3f(h1.x, h1.y, h1.z);

            // Vertical lines
            Vec3 v0 = cellCornerToWorld(face, i < GRID_N ? i : GRID_N - 1, 0, i < GRID_N ? 0.0f : 1.0f, 0.0f, eps);
            Vec3 v1 = cellCornerToWorld(face, i < GRID_N ? i : GRID_N - 1, GRID_N - 1, i < GRID_N ? 0.0f : 1.0f, 1.0f, eps);
            glVertex3f(v0.x, v0.y, v0.z);
            glVertex3f(v1.x, v1.y, v1.z);
        }
    }
    glEnd();

    // 3. Draw outer cube edges in Dim Green (#115511)
    glLineWidth(2.0f);
    glColor3f(Palette::DimGreen.r, Palette::DimGreen.g, Palette::DimGreen.b);
    glutWireCube(CUBE_SIZE + 0.005f);
}

void Renderer::renderCellBox(Face face, int u, int v, float scale, float height, float r, float g, float b) {
    const FaceBasis& basis = getFaceBasis(face);
    Vec3 center = localToWorld(face, u, v, 0.005f);

    float halfW = (CELL_SIZE * scale) * 0.5f;
    Vec3 uVec = basis.uAxis * halfW;
    Vec3 vVec = basis.vAxis * halfW;
    Vec3 nVec = basis.normal * height;

    // 8 vertices of the raised tile block
    Vec3 b00 = center - uVec - vVec;
    Vec3 b10 = center + uVec - vVec;
    Vec3 b11 = center + uVec + vVec;
    Vec3 b01 = center - uVec + vVec;

    Vec3 t00 = b00 + nVec;
    Vec3 t10 = b10 + nVec;
    Vec3 t11 = b11 + nVec;
    Vec3 t01 = b01 + nVec;

    // Flat saturated retro color - no lighting falloff or gradients
    glColor3f(r, g, b);

    glBegin(GL_QUADS);
    // Top face
    glVertex3f(t00.x, t00.y, t00.z);
    glVertex3f(t10.x, t10.y, t10.z);
    glVertex3f(t11.x, t11.y, t11.z);
    glVertex3f(t01.x, t01.y, t01.z);

    // Front (along +vAxis)
    glVertex3f(t01.x, t01.y, t01.z);
    glVertex3f(t11.x, t11.y, t11.z);
    glVertex3f(b11.x, b11.y, b11.z);
    glVertex3f(b01.x, b01.y, b01.z);

    // Back (along -vAxis)
    glVertex3f(t00.x, t00.y, t00.z);
    glVertex3f(b00.x, b00.y, b00.z);
    glVertex3f(b10.x, b10.y, b10.z);
    glVertex3f(t10.x, t10.y, t10.z);

    // Right (along +uAxis)
    glVertex3f(t10.x, t10.y, t10.z);
    glVertex3f(b10.x, b10.y, b10.z);
    glVertex3f(b11.x, b11.y, b11.z);
    glVertex3f(t11.x, t11.y, t11.z);

    // Left (along -uAxis)
    glVertex3f(t00.x, t00.y, t00.z);
    glVertex3f(t01.x, t01.y, t01.z);
    glVertex3f(b01.x, b01.y, b01.z);
    glVertex3f(b00.x, b00.y, b00.z);

    // Bottom (along -normal)
    glVertex3f(b00.x, b00.y, b00.z);
    glVertex3f(b01.x, b01.y, b01.z);
    glVertex3f(b11.x, b11.y, b11.z);
    glVertex3f(b10.x, b10.y, b10.z);
    glEnd();
}

void Renderer::renderSnake(const Snake& snake, float animTime) {
    if (snake.body.empty()) return;

    size_t bodyLen = snake.body.size();

    // 1. Draw body segments: Flat Neon Green (#33CC33) across all segments per retro palette
    for (size_t i = 1; i < bodyLen; i++) {
        renderCellBox(snake.body[i].face, snake.body[i].u, snake.body[i].v,
                      0.85f, 0.22f,
                      Palette::BodyGreen.r, Palette::BodyGreen.g, Palette::BodyGreen.b);
    }

    // 2. Draw snake head: Flat Bright Cyan (#33FFFF)
    const GridPos& head = snake.body.front();
    renderCellBox(head.face, head.u, head.v,
                  0.90f, 0.28f,
                  Palette::Cyan.r, Palette::Cyan.g, Palette::Cyan.b);

    // 3. Directional eyes on snake head
    const FaceBasis& basis = getFaceBasis(head.face);
    Vec3 center = localToWorld(head.face, head.u, head.v, 0.29f);

    Vec3 fwd(0, 0, 0);
    switch (snake.currentDir) {
        case Dir::UP:    fwd = basis.vAxis; break;
        case Dir::DOWN:  fwd = basis.vAxis * -1.0f; break;
        case Dir::LEFT:  fwd = basis.uAxis * -1.0f; break;
        case Dir::RIGHT: fwd = basis.uAxis; break;
    }
    Vec3 side = fwd.cross(basis.normal);

    float eyeOffsetFwd = CELL_SIZE * 0.22f;
    float eyeOffsetSide = CELL_SIZE * 0.24f;
    float eyeRadius = CELL_SIZE * 0.08f;

    Vec3 eyeL = center + (fwd * eyeOffsetFwd) - (side * eyeOffsetSide);
    Vec3 eyeR = center + (fwd * eyeOffsetFwd) + (side * eyeOffsetSide);

    // Black pupils on cyan head (#000000)
    glColor3f(Palette::Black.r, Palette::Black.g, Palette::Black.b);
    glPushMatrix();
    glTranslatef(eyeL.x, eyeL.y, eyeL.z);
    glutSolidSphere(eyeRadius, 8, 8);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(eyeR.x, eyeR.y, eyeR.z);
    glutSolidSphere(eyeRadius, 8, 8);
    glPopMatrix();
}

void Renderer::renderFood(const GridPos& food, float animTime) {
    const FaceBasis& basis = getFaceBasis(food.face);

    float bob = 0.25f + 0.08f * std::sin(animTime * 5.0f);
    Vec3 center = localToWorld(food.face, food.u, food.v, bob);

    glPushMatrix();
    glTranslatef(center.x, center.y, center.z);

    // Spin food
    float spinAngle = animTime * 120.0f;
    glRotatef(spinAngle, basis.normal.x, basis.normal.y, basis.normal.z);

    float foodRadius = CELL_SIZE * 0.32f;

    // Flat Red (#FF3333) per retro palette
    glColor3f(Palette::Red.r, Palette::Red.g, Palette::Red.b);
    glutSolidSphere(foodRadius, 10, 10);

    glPopMatrix();
}

void Renderer::update(float deltaTime) {
    m_ui.update(deltaTime);
}

void Renderer::triggerLossEffects() {
    m_ui.triggerLossEffects();
}

void Renderer::render(const GameManager& game, const OrbitCamera& camera, int windowWidth, int windowHeight, float animTime) {
    GameState state = game.getState();

    glClearColor(Palette::Black.r, Palette::Black.g, Palette::Black.b, 1.0f);

    if (state == GameState::MENU || state == GameState::GAME_OVER) {
        // Section 1: Skip 3D render pass entirely for MENU and GAME_OVER!
        // Solid black background, 2D arcade overlay only.
        glClear(GL_COLOR_BUFFER_BIT);

        m_ui.render(game, windowWidth, windowHeight, animTime);
    } else {
        // PLAYING or PAUSED: Render 3D scene + HUD
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        camera.applyView();

        renderCubeAndGrids();
        renderSnake(game.getSnake(), animTime);
        renderFood(game.getFood(), animTime);

        // 2D HUD and CRT scanlines
        m_ui.render(game, windowWidth, windowHeight, animTime);
    }

    glutSwapBuffers();
}
