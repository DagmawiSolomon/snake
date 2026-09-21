#include "renderer.h"
#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include <string>

Renderer::Renderer() {}

void Renderer::initGL() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Enable smooth shading and basic color material
    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    GLfloat light0_pos[] = { 10.0f, 15.0f, 12.0f, 1.0f };
    GLfloat light0_diff[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    GLfloat light0_amb[]  = { 0.25f, 0.25f, 0.3f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0_diff);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light0_amb);

    GLfloat light1_pos[] = { -12.0f, -10.0f, -10.0f, 1.0f };
    GLfloat light1_diff[] = { 0.4f, 0.4f, 0.5f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  light1_diff);

    glEnable(GL_NORMALIZE);
    glClearColor(0.07f, 0.08f, 0.12f, 1.0f); // Sleek modern dark blue-gray
}

void Renderer::renderCubeAndGrids() {
    // 1. Draw solid cube faces with subtle distinct tints
    glBegin(GL_QUADS);
    for (int f = 0; f < 6; f++) {
        Face face = static_cast<Face>(f);
        const FaceBasis& b = getFaceBasis(face);
        glNormal3f(b.normal.x, b.normal.y, b.normal.z);

        // Subtle dark slate tints
        switch (face) {
            case Face::PZ: glColor3f(0.12f, 0.14f, 0.20f); break;
            case Face::PX: glColor3f(0.13f, 0.15f, 0.22f); break;
            case Face::NZ: glColor3f(0.11f, 0.13f, 0.19f); break;
            case Face::NX: glColor3f(0.14f, 0.16f, 0.23f); break;
            case Face::PY: glColor3f(0.15f, 0.17f, 0.25f); break;
            case Face::NY: glColor3f(0.10f, 0.12f, 0.17f); break;
        }

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

    // 2. Draw interior grid lines for each face
    glDisable(GL_LIGHTING);
    glLineWidth(1.0f);
    glColor4f(0.24f, 0.30f, 0.42f, 0.8f);

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

    // 3. Draw highlighted outer cube edges
    glLineWidth(2.5f);
    glColor3f(0.40f, 0.55f, 0.80f);
    glutWireCube(CUBE_SIZE + 0.005f);

    glEnable(GL_LIGHTING);
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

    glColor3f(r, g, b);

    glBegin(GL_QUADS);
    // Top face
    glNormal3f(basis.normal.x, basis.normal.y, basis.normal.z);
    glVertex3f(t00.x, t00.y, t00.z);
    glVertex3f(t10.x, t10.y, t10.z);
    glVertex3f(t11.x, t11.y, t11.z);
    glVertex3f(t01.x, t01.y, t01.z);

    // Darken side faces slightly for 3D depth
    glColor3f(r * 0.75f, g * 0.75f, b * 0.75f);

    // Front (along +vAxis)
    glNormal3f(basis.vAxis.x, basis.vAxis.y, basis.vAxis.z);
    glVertex3f(t01.x, t01.y, t01.z);
    glVertex3f(t11.x, t11.y, t11.z);
    glVertex3f(b11.x, b11.y, b11.z);
    glVertex3f(b01.x, b01.y, b01.z);

    // Back (along -vAxis)
    glNormal3f(-basis.vAxis.x, -basis.vAxis.y, -basis.vAxis.z);
    glVertex3f(t00.x, t00.y, t00.z);
    glVertex3f(b00.x, b00.y, b00.z);
    glVertex3f(b10.x, b10.y, b10.z);
    glVertex3f(t10.x, t10.y, t10.z);

    // Right (along +uAxis)
    glNormal3f(basis.uAxis.x, basis.uAxis.y, basis.uAxis.z);
    glVertex3f(t10.x, t10.y, t10.z);
    glVertex3f(b10.x, b10.y, b10.z);
    glVertex3f(b11.x, b11.y, b11.z);
    glVertex3f(t11.x, t11.y, t11.z);

    // Left (along -uAxis)
    glNormal3f(-basis.uAxis.x, -basis.uAxis.y, -basis.uAxis.z);
    glVertex3f(t00.x, t00.y, t00.z);
    glVertex3f(t01.x, t01.y, t01.z);
    glVertex3f(b01.x, b01.y, b01.z);
    glVertex3f(b00.x, b00.y, b00.z);
    glEnd();
}

void Renderer::renderSnake(const Snake& snake, float animTime) {
    if (snake.body.empty()) return;

    size_t bodyLen = snake.body.size();

    // 1. Draw body segments
    for (size_t i = 1; i < bodyLen; i++) {
        float progress = static_cast<float>(i) / static_cast<float>(bodyLen);
        // Gradient from vibrant emerald to deeper forest jade
        float r = 0.12f + progress * 0.05f;
        float g = 0.85f - progress * 0.35f;
        float b = 0.45f - progress * 0.20f;

        float scale = 0.88f - (progress * 0.18f); // slight taper toward tail
        float height = 0.22f - (progress * 0.06f);

        renderCellBox(snake.body[i].face, snake.body[i].u, snake.body[i].v, scale, height, r, g, b);
    }

    // 2. Draw snake head (larger, glowing cyan-emerald)
    const GridPos& head = snake.body.front();
    renderCellBox(head.face, head.u, head.v, 0.92f, 0.28f, 0.15f, 0.95f, 0.70f);

    // 3. Draw cute eyes on the snake head
    const FaceBasis& basis = getFaceBasis(head.face);
    Vec3 center = localToWorld(head.face, head.u, head.v, 0.29f);

    // Determine forward and sideways directions based on snake.currentDir
    Vec3 fwd(0, 0, 0);
    Vec3 side(0, 0, 0);
    switch (snake.currentDir) {
        case Dir::UP:    fwd = basis.vAxis; side = basis.uAxis; break;
        case Dir::DOWN:  fwd = basis.vAxis * -1.0f; side = basis.uAxis; break;
        case Dir::LEFT:  fwd = basis.uAxis * -1.0f; side = basis.vAxis; break;
        case Dir::RIGHT: fwd = basis.uAxis; side = basis.vAxis; break;
    }

    float eyeOffsetFwd = CELL_SIZE * 0.22f;
    float eyeOffsetSide = CELL_SIZE * 0.25f;
    float eyeRadius = CELL_SIZE * 0.085f;

    Vec3 eyeL = center + (fwd * eyeOffsetFwd) - (side * eyeOffsetSide);
    Vec3 eyeR = center + (fwd * eyeOffsetFwd) + (side * eyeOffsetSide);

    // Draw eye whites
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(eyeL.x, eyeL.y, eyeL.z);
    glutSolidSphere(eyeRadius, 8, 8);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(eyeR.x, eyeR.y, eyeR.z);
    glutSolidSphere(eyeRadius, 8, 8);
    glPopMatrix();

    // Draw pupils
    glColor3f(0.05f, 0.05f, 0.1f);
    Vec3 pupilL = eyeL + (fwd * (eyeRadius * 0.5f));
    Vec3 pupilR = eyeR + (fwd * (eyeRadius * 0.5f));

    glPushMatrix();
    glTranslatef(pupilL.x, pupilL.y, pupilL.z);
    glutSolidSphere(eyeRadius * 0.55f, 6, 6);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(pupilR.x, pupilR.y, pupilR.z);
    glutSolidSphere(eyeRadius * 0.55f, 6, 6);
    glPopMatrix();
}

void Renderer::renderFood(const GridPos& food, float animTime) {
    const FaceBasis& basis = getFaceBasis(food.face);

    // Floating bobbing motion
    float bob = 0.25f + 0.08f * std::sin(animTime * 5.0f);
    Vec3 center = localToWorld(food.face, food.u, food.v, bob);

    glPushMatrix();
    glTranslatef(center.x, center.y, center.z);

    // Spin food
    float spinAngle = animTime * 120.0f;
    glRotatef(spinAngle, basis.normal.x, basis.normal.y, basis.normal.z);

    // Pulsing scale
    float pulse = 1.0f + 0.12f * std::sin(animTime * 8.0f);
    float foodRadius = (CELL_SIZE * 0.32f) * pulse;

    // Glowing ruby/amber color
    glColor3f(1.0f, 0.22f, 0.28f);
    glutSolidSphere(foodRadius, 14, 14);

    // Golden leaf/stem
    glColor3f(0.95f, 0.85f, 0.2f);
    Vec3 stemTop = basis.normal * (foodRadius * 1.2f);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(stemTop.x, stemTop.y, stemTop.z);
    glEnd();

    glPopMatrix();
}

void Renderer::drawText2D(float x, float y, const char* str, void* font, float r, float g, float b) {
    if (!font) font = GLUT_BITMAP_HELVETICA_18;

    // Draw dark shadow first
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(x + 1.5f, y + 1.5f);
    for (const char* c = str; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }

    // Draw main text
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (const char* c = str; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void Renderer::renderHUD(const GameManager& game, int width, int height) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, width, height, 0); // Top-left is (0, 0)

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    char buf[128];

    // Top banner: Title & Score
    std::snprintf(buf, sizeof(buf), "SCORE: %d", game.getScore());
    drawText2D(25, 35, buf, GLUT_BITMAP_HELVETICA_18, 1.0f, 0.9f, 0.2f);

    std::snprintf(buf, sizeof(buf), "HIGH SCORE: %d", game.getHighScore());
    drawText2D(25, 60, buf, GLUT_BITMAP_HELVETICA_12, 0.8f, 0.8f, 0.85f);

    std::snprintf(buf, sizeof(buf), "FACE: %s  |  SPEED: %d ms",
                  faceToString(game.getActiveFace()), game.getTickIntervalMs());
    drawText2D(25, 82, buf, GLUT_BITMAP_HELVETICA_12, 0.6f, 0.8f, 1.0f);

    // Controls hint (bottom)
    drawText2D(25, height - 20, "[WASD / ARROWS] Turn   [P] Pause   [R] Restart",
               GLUT_BITMAP_HELVETICA_12, 0.7f, 0.75f, 0.85f);

    // State overlays
    GameState state = game.getState();
    if (state == GameState::MENU) {
        // Dark translucent overlay panel
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.05f, 0.06f, 0.10f, 0.82f);
        glBegin(GL_QUADS);
        glVertex2f(width * 0.15f, height * 0.22f);
        glVertex2f(width * 0.85f, height * 0.22f);
        glVertex2f(width * 0.85f, height * 0.78f);
        glVertex2f(width * 0.15f, height * 0.78f);
        glEnd();
        glDisable(GL_BLEND);

        drawText2D(width * 0.28f, height * 0.35f, "SNAKE ON A CUBE",
                   GLUT_BITMAP_TIMES_ROMAN_24, 0.2f, 0.95f, 0.65f);
        drawText2D(width * 0.23f, height * 0.43f, "Guide your snake across the 6 faces of the 3D cube!",
                   GLUT_BITMAP_HELVETICA_18, 0.9f, 0.9f, 0.95f);
        drawText2D(width * 0.26f, height * 0.50f, "Edges seamlessly connect all faces - no walls!",
                   GLUT_BITMAP_HELVETICA_12, 0.65f, 0.85f, 1.0f);

        drawText2D(width * 0.30f, height * 0.62f, "Press SPACE or ENTER to Start",
                   GLUT_BITMAP_HELVETICA_18, 1.0f, 0.85f, 0.2f);
    } else if (state == GameState::PAUSED) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.04f, 0.05f, 0.08f, 0.75f);
        glBegin(GL_QUADS);
        glVertex2f(width * 0.25f, height * 0.38f);
        glVertex2f(width * 0.75f, height * 0.38f);
        glVertex2f(width * 0.75f, height * 0.62f);
        glVertex2f(width * 0.25f, height * 0.62f);
        glEnd();
        glDisable(GL_BLEND);

        drawText2D(width * 0.42f, height * 0.48f, "PAUSED",
                   GLUT_BITMAP_TIMES_ROMAN_24, 1.0f, 0.85f, 0.2f);
        drawText2D(width * 0.35f, height * 0.55f, "Press P to Resume Game",
                   GLUT_BITMAP_HELVETICA_18, 0.85f, 0.9f, 1.0f);
    } else if (state == GameState::GAME_OVER) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.12f, 0.03f, 0.05f, 0.85f);
        glBegin(GL_QUADS);
        glVertex2f(width * 0.20f, height * 0.30f);
        glVertex2f(width * 0.80f, height * 0.30f);
        glVertex2f(width * 0.80f, height * 0.72f);
        glVertex2f(width * 0.20f, height * 0.72f);
        glEnd();
        glDisable(GL_BLEND);

        drawText2D(width * 0.38f, height * 0.42f, "GAME OVER",
                   GLUT_BITMAP_TIMES_ROMAN_24, 1.0f, 0.25f, 0.3f);

        std::snprintf(buf, sizeof(buf), "Final Score: %d", game.getScore());
        drawText2D(width * 0.40f, height * 0.50f, buf,
                   GLUT_BITMAP_HELVETICA_18, 1.0f, 0.9f, 0.25f);

        drawText2D(width * 0.32f, height * 0.60f, "Press R to Play Again or M for Menu",
                   GLUT_BITMAP_HELVETICA_18, 0.85f, 0.9f, 1.0f);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void Renderer::render(const GameManager& game, const OrbitCamera& camera, int windowWidth, int windowHeight, float animTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    camera.applyView();

    renderCubeAndGrids();
    renderSnake(game.getSnake(), animTime);
    renderFood(game.getFood(), animTime);

    renderHUD(game, windowWidth, windowHeight);

    glutSwapBuffers();
}
