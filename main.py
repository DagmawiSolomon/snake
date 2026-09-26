"""
Main executable entry point for Snake on a Cube - PyOpenGL 3D.
Features retro arcade audio, urgency countdown timer, snake halving mechanics, and fullscreen support.
"""

import sys
import pygame
from pygame.locals import (
    DOUBLEBUF, OPENGL, RESIZABLE, FULLSCREEN,
    QUIT, KEYDOWN, VIDEORESIZE,
    K_ESCAPE, K_SPACE, K_RETURN,
    K_w, K_s, K_a, K_d,
    K_UP, K_DOWN, K_LEFT, K_RIGHT,
    K_p, K_r, K_m,
    K_F11, K_f,
    K_1, K_2, K_3, K_4, K_5, K_6
)
from OpenGL.GL import glViewport, glMatrixMode, glLoadIdentity, GL_PROJECTION, GL_MODELVIEW
from OpenGL.GLU import gluPerspective

from snake.cube_topology import Face, Dir
from snake.game_state import GameState, GameManager
from snake.camera import OrbitCamera
from snake.renderer import Renderer
from snake.audio import SoundManager

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

    pygame.display.set_caption("Snake on a Cube - PyOpenGL 3D")

    renderer = Renderer()
    is_fullscreen = True
    window_width = 960
    window_height = 720

    def apply_display_mode(fullscreen: bool):
        nonlocal window_width, window_height, is_fullscreen
        is_fullscreen = fullscreen
        if is_fullscreen:
            info = pygame.display.Info()
            window_width = info.current_w if info.current_w > 0 else 1366
            window_height = info.current_h if info.current_h > 0 else 768
            flags = DOUBLEBUF | OPENGL | FULLSCREEN
        else:
            window_width = 960
            window_height = 720
            flags = DOUBLEBUF | OPENGL | RESIZABLE

        pygame.display.set_mode((window_width, window_height), flags)
        setup_perspective(window_width, window_height)
        renderer.init_gl()

    # Launch into full screen by default
    apply_display_mode(True)

    game = GameManager()
    camera = OrbitCamera()
    sound = SoundManager()

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

            elif event.type == VIDEORESIZE and not is_fullscreen:
                window_width, window_height = event.size
                setup_perspective(window_width, window_height)

            elif event.type == KEYDOWN:
                if event.key == K_ESCAPE:
                    running = False
                    break

                # Fullscreen Toggle (F11 or F)
                if event.key in (K_F11, K_f):
                    apply_display_mode(not is_fullscreen)

                # Start / Pause / Resume / Back to menu
                elif event.key in (K_SPACE, K_RETURN):
                    if game.state == GameState.MENU:
                        game.start_game()
                        camera.set_target_face(game.get_active_face())
                        last_tick_time = current_time
                        sound.play_start()
                    elif game.state == GameState.GAME_OVER:
                        game.state = GameState.MENU
                        sound.play_pause()
                    elif game.state in (GameState.PLAYING, GameState.PAUSED):
                        game.toggle_pause()
                        sound.play_pause()
                        if game.state == GameState.PLAYING:
                            last_tick_time = current_time

                elif event.key == K_p:
                    if game.state in (GameState.PLAYING, GameState.PAUSED):
                        game.toggle_pause()
                        sound.play_pause()
                        if game.state == GameState.PLAYING:
                            last_tick_time = current_time

                elif event.key == K_r:
                    game.reset()
                    camera.set_target_face(game.get_active_face())
                    last_tick_time = current_time
                    sound.play_start()

                elif event.key == K_m:
                    if game.state in (GameState.GAME_OVER, GameState.PAUSED):
                        game.state = GameState.MENU
                        sound.play_pause()

                # Directions (WASD and Arrow keys)
                elif event.key in (K_w, K_UP):
                    if game.request_direction(Dir.UP):
                        sound.play_turn()
                elif event.key in (K_s, K_DOWN):
                    if game.request_direction(Dir.DOWN):
                        sound.play_turn()
                elif event.key in (K_a, K_LEFT):
                    if game.request_direction(Dir.LEFT):
                        sound.play_turn()
                elif event.key in (K_d, K_RIGHT):
                    if game.request_direction(Dir.RIGHT):
                        sound.play_turn()

                # Quick camera inspection keys (1-6)
                elif event.key == K_1: camera.set_target_face(Face.PX)
                elif event.key == K_2: camera.set_target_face(Face.NX)
                elif event.key == K_3: camera.set_target_face(Face.PY)
                elif event.key == K_4: camera.set_target_face(Face.NY)
                elif event.key == K_5: camera.set_target_face(Face.PZ)
                elif event.key == K_6: camera.set_target_face(Face.NZ)

        # 2. Time & Frame Delta
        dt = clock.tick(60) / 1000.0
        dt = min(dt, 0.05)
        anim_time += dt

        # 3. Urgency Countdown & Halving Penalty
        halved, warn_tick = game.update_timers(dt)
        if warn_tick:
            sound.play_warn()
        if halved:
            sound.play_shrink()
            renderer.trigger_shrink_effects()

        # 4. Game Tick Loop
        if game.state == GameState.PLAYING:
            if current_time - last_tick_time >= game.tick_interval_ms:
                last_tick_time = current_time
                moved = game.tick()
                if moved:
                    camera.set_target_face(game.get_active_face())
                    if game.just_ate:
                        sound.play_eat()
                else:
                    sound.play_game_over()
                    renderer.trigger_loss_effects()

        # 5. Visual Updates & Rendering
        renderer.update(dt)

        if game.state in (GameState.PLAYING, GameState.PAUSED):
            camera.update(dt)

        renderer.render(game, camera, window_width, window_height, anim_time)
        pygame.display.flip()

    pygame.quit()
    sys.exit(0)

if __name__ == '__main__':
    main()
