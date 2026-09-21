#include <GL/freeglut.h>
#include "cube_topology.h"
#include "game_state.h"
#include "camera.h"
#include "renderer.h"

static GameManager g_game;
static OrbitCamera g_camera;
static Renderer g_renderer;

static int g_windowWidth = 960;
static int g_windowHeight = 720;
static float g_animTime = 0.0f;

// Forward declarations
static void onDisplay();
static void onReshape(int w, int h);
static void onKeyboard(unsigned char key, int x, int y);
static void onSpecialKey(int key, int x, int y);
static void onRenderTimer(int val);
static void onGameTickTimer(int val);

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(g_windowWidth, g_windowHeight);
    glutCreateWindow("Snake on a Cube - OpenGL 3D");

    g_renderer.initGL();

    glutDisplayFunc(onDisplay);
    glutReshapeFunc(onReshape);
    glutKeyboardFunc(onKeyboard);
    glutSpecialFunc(onSpecialKey);

    // Start 60fps render loop
    glutTimerFunc(16, onRenderTimer, 0);

    // Start game tick loop
    glutTimerFunc(g_game.getTickIntervalMs(), onGameTickTimer, 0);

    glutMainLoop();
    return 0;
}

static void onDisplay() {
    g_renderer.render(g_game, g_camera, g_windowWidth, g_windowHeight, g_animTime);
}

static void onReshape(int w, int h) {
    if (h == 0) h = 1;
    g_windowWidth = w;
    g_windowHeight = h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, static_cast<float>(w) / static_cast<float>(h), 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

static void onRenderTimer(int val) {
    g_animTime += 0.016f;

    // Smoothly orbit camera towards target face
    g_camera.update(0.016f);

    // In menu, gently rotate camera around the cube as a showcase turntable
    if (g_game.getState() == GameState::MENU) {
        g_camera.addManualRotation(0.0f, 0.006f);
    }

    glutPostRedisplay();
    glutTimerFunc(16, onRenderTimer, 0);
}

static void onGameTickTimer(int val) {
    if (g_game.getState() == GameState::PLAYING) {
        bool moved = g_game.tick();
        if (moved) {
            // Keep camera locked onto active face where snake's head is
            g_camera.setTargetFace(g_game.getActiveFace());
        }
    }

    // Reschedule tick based on current dynamic game speed
    glutTimerFunc(g_game.getTickIntervalMs(), onGameTickTimer, 0);
}

static void handleDirInput(Dir d) {
    if (g_game.getState() == GameState::PLAYING) {
        g_game.requestDirection(d);
    }
}

static void onKeyboard(unsigned char key, int x, int y) {
    GameState state = g_game.getState();

    switch (key) {
        case 27: // ESC
            exit(0);
            break;

        case ' ':
        case 13: // ENTER
            if (state == GameState::MENU) {
                g_game.startGame();
                g_camera.setTargetFace(g_game.getActiveFace());
            } else if (state == GameState::PLAYING || state == GameState::PAUSED) {
                g_game.togglePause();
            }
            break;

        case 'w':
        case 'W':
            handleDirInput(Dir::UP);
            break;

        case 's':
        case 'S':
            handleDirInput(Dir::DOWN);
            break;

        case 'a':
        case 'A':
            handleDirInput(Dir::LEFT);
            break;

        case 'd':
        case 'D':
            handleDirInput(Dir::RIGHT);
            break;

        case 'p':
        case 'P':
            g_game.togglePause();
            break;

        case 'r':
        case 'R':
            g_game.reset();
            g_camera.setTargetFace(g_game.getActiveFace());
            break;

        case 'm':
        case 'M':
            if (state == GameState::GAME_OVER) {
                g_game.setState(GameState::MENU);
            }
            break;
    }
}

static void onSpecialKey(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            handleDirInput(Dir::UP);
            break;
        case GLUT_KEY_DOWN:
            handleDirInput(Dir::DOWN);
            break;
        case GLUT_KEY_LEFT:
            handleDirInput(Dir::LEFT);
            break;
        case GLUT_KEY_RIGHT:
            handleDirInput(Dir::RIGHT);
            break;
    }
}
