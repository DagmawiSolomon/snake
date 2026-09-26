"""
Main executable entry point for Snake on a Cube - PyOpenGL 3D.
"""

import sys
import pygame
from pygame.locals import (
    DOUBLEBUF, OPENGL, RESIZABLE,
    QUIT, KEYDOWN, VIDEORESIZE,
    K_ESCAPE, K_SPACE, K_RETURN,
    K_w, K_s, K_a, K_d,
    K_UP, K_DOWN, K_LEFT, K_RIGHT,
    K_p, K_r, K_m,
    K_1, K_2, K_3, K_4, K_5, K_6
)
from OpenGL.GL import glViewport, glMatrixMode, glLoadIdentity, GL_PROJECTION, GL_MODELVIEW
from OpenGL.GLU import gluPerspective

from cube_topology import Face, Dir
from game_state import GameState, GameManager
from camera import OrbitCamera
from renderer import Renderer

def setup_perspective(width: int, height: int) -> None:
    if height == 0:
        height = 1
    glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45.0, float(width) / float(height), 0.1, 100.0)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

def main():
    pygame.init()
    pygame.font.init()

    window_width = 960
    window_height = 720

    pygame.display.set_caption("Snake on a Cube - PyOpenGL 3D")
    pygame.display.set_mode((window_width, window_height), DOUBLEBUF | OPENGL | RESIZABLE)

    setup_perspective(window_width, window_height)

    game = GameManager()
    camera = OrbitCamera()
    renderer = Renderer()
    renderer.init_gl()

    clock = pygame.time.Clock()
    anim_time = 0.0
    last_tick_time = pygame.time.get_ticks()

    running = True
    while running:
        current_time = pygame.time.get_ticks()

        # 1. Event Handling
        for event in pygame.event.get():
            if event.type == QUIT:
                running = False
                break

            elif event.type == VIDEORESIZE:
                window_width, window_height = event.size
                setup_perspective(window_width, window_height)

            elif event.type == KEYDOWN:
                if event.key == K_ESCAPE:
                    running = False
                    break

                # Start / Pause / Resume / Back to menu
                if event.key in (K_SPACE, K_RETURN):
                    if game.state == GameState.MENU:
                        game.start_game()
                        camera.set_target_face(game.get_active_face())
                        last_tick_time = current_time
                    elif game.state == GameState.GAME_OVER:
                        game.state = GameState.MENU
                    elif game.state in (GameState.PLAYING, GameState.PAUSED):
                        game.toggle_pause()
                        if game.state == GameState.PLAYING:
                            last_tick_time = current_time

                elif event.key == K_p:
                    if game.state in (GameState.PLAYING, GameState.PAUSED):
                        game.toggle_pause()
                        if game.state == GameState.PLAYING:
                            last_tick_time = current_time

                elif event.key == K_r:
                    game.reset()
                    camera.set_target_face(game.get_active_face())
                    last_tick_time = current_time

                elif event.key == K_m:
                    if game.state in (GameState.GAME_OVER, GameState.PAUSED):
                        game.state = GameState.MENU

                # Directions (WASD and Arrow keys)
                elif event.key in (K_w, K_UP):
                    game.request_direction(Dir.UP)
                elif event.key in (K_s, K_DOWN):
                    game.request_direction(Dir.DOWN)
                elif event.key in (K_a, K_LEFT):
                    game.request_direction(Dir.LEFT)
                elif event.key in (K_d, K_RIGHT):
                    game.request_direction(Dir.RIGHT)

                # Quick camera inspection keys (1-6)
                elif event.key == K_1: camera.set_target_face(Face.PX)
                elif event.key == K_2: camera.set_target_face(Face.NX)
                elif event.key == K_3: camera.set_target_face(Face.PY)
                elif event.key == K_4: camera.set_target_face(Face.NY)
                elif event.key == K_5: camera.set_target_face(Face.PZ)
                elif event.key == K_6: camera.set_target_face(Face.NZ)

        # 2. Game Tick Loop
        if game.state == GameState.PLAYING:
            if current_time - last_tick_time >= game.tick_interval_ms:
                last_tick_time = current_time
                moved = game.tick()
                if moved:
                    camera.set_target_face(game.get_active_face())
                else:
                    renderer.trigger_loss_effects()

        # 3. Render and Animation Update (60 FPS)
        dt = clock.tick(60) / 1000.0
        dt = min(dt, 0.05)  # clamp to avoid huge delta on freeze/drag
        anim_time += dt

        renderer.update(dt)

        if game.state in (GameState.PLAYING, GameState.PAUSED):
            camera.update(dt)

        renderer.render(game, camera, window_width, window_height, anim_time)
        pygame.display.flip()

    pygame.quit()
    sys.exit(0)

if __name__ == '__main__':
    main()
