"""
Retro Arcade 2D UI, CRT scanlines, and 5x7 bitmap font rendering for Snake on a Cube.
"""

from OpenGL.GL import (
    glEnable, glDisable, glBlendFunc, glMatrixMode, glPushMatrix, glPopMatrix,
    glLoadIdentity, glBegin, glEnd, glVertex2f, glColor3f, glColor4f, glLineWidth,
    GL_DEPTH_TEST, GL_LIGHTING, GL_BLEND, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
    GL_QUADS, GL_LINES, GL_LINE_LOOP, GL_PROJECTION, GL_MODELVIEW
)
from OpenGL.GLU import gluOrtho2D
from .palette import BLACK, NEON_GREEN, AMBER, CYAN, RED, DIM_GREEN, SCANLINE_ALPHA
from .cube_topology import face_to_string
from .game_state import GameState, GameManager

FONT_5X7 = [
    # Space (32)
    (0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00),
    # ! (33)
    (0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04),
    # " (34)
    (0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00),
    # # (35)
    (0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A),
    # $ (36)
    (0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04),
    # % (37)
    (0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03),
    # & (38)
    (0x08, 0x14, 0x14, 0x08, 0x15, 0x12, 0x0D),
    # ' (39)
    (0x0C, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00),
    # ( (40)
    (0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02),
    # ) (41)
    (0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08),
    # * (42)
    (0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00),
    # + (43)
    (0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00),
    # , (44)
    (0x00, 0x00, 0x00, 0x00, 0x0C, 0x04, 0x08),
    # - (45)
    (0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00),
    # . (46)
    (0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C),
    # / (47)
    (0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00),
    # 0 (48)
    (0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E),
    # 1 (49)
    (0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E),
    # 2 (50)
    (0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F),
    # 3 (51)
    (0x1E, 0x01, 0x01, 0x06, 0x01, 0x01, 0x1E),
    # 4 (52)
    (0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02),
    # 5 (53)
    (0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E),
    # 6 (54)
    (0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E),
    # 7 (55)
    (0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08),
    # 8 (56)
    (0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E),
    # 9 (57)
    (0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C),
    # : (58)
    (0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00),
    # ; (59)
    (0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x04, 0x08),
    # < (60)
    (0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02),
    # = (61)
    (0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00),
    # > (62)
    (0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08),
    # ? (63)
    (0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04),
    # @ (64)
    (0x0E, 0x11, 0x01, 0x0D, 0x15, 0x15, 0x0E),
    # A (65)
    (0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11),
    # B (66)
    (0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E),
    # C (67)
    (0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E),
    # D (68)
    (0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C),
    # E (69)
    (0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F),
    # F (70)
    (0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10),
    # G (71)
    (0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F),
    # H (72)
    (0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11),
    # I (73)
    (0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E),
    # J (74)
    (0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C),
    # K (75)
    (0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11),
    # L (76)
    (0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F),
    # M (77)
    (0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11),
    # N (78)
    (0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11),
    # O (79)
    (0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E),
    # P (80)
    (0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10),
    # Q (81)
    (0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D),
    # R (82)
    (0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11),
    # S (83)
    (0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E),
    # T (84)
    (0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04),
    # U (85)
    (0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E),
    # V (86)
    (0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04),
    # W (87)
    (0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A),
    # X (88)
    (0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11),
    # Y (89)
    (0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04),
    # Z (90)
    (0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F),
    # [ (91)
    (0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E),
    # \ (92)
    (0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00),
    # ] (93)
    (0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E)
]

def get_text_width(text: str, pixel_size: float) -> float:
    return float(len(text) * 6) * pixel_size

def draw_text(text: str, x: float, y: float, pixel_size: float,
              r: float, g: float, b: float, centered: bool = False, window_width: int = 0) -> None:
    if not text:
        return

    if centered and window_width > 0:
        w = get_text_width(text, pixel_size)
        x = (window_width - w) * 0.5

    glColor3f(r, g, b)
    glBegin(GL_QUADS)
    cur_x = x
    cur_y = y
    for ch in text.upper():
        code = ord(ch)
        if 32 <= code <= 93:
            glyph = FONT_5X7[code - 32]
            for row in range(7):
                row_bits = glyph[row]
                for col in range(5):
                    if row_bits & (1 << (4 - col)):
                        px = cur_x + col * pixel_size
                        py = cur_y + row * pixel_size
                        glVertex2f(px, py)
                        glVertex2f(px + pixel_size, py)
                        glVertex2f(px + pixel_size, py + pixel_size)
                        glVertex2f(px, py + pixel_size)
        cur_x += 6.0 * pixel_size
    glEnd()

class RetroArcadeUI:
    def __init__(self):
        self.blink_timer: float = 0.0
        self.flash_timer: float = 0.0
        self.shake_timer: float = 0.0

    def update(self, delta_time: float) -> None:
        self.blink_timer += delta_time
        if self.flash_timer > 0.0:
            self.flash_timer = max(0.0, self.flash_timer - delta_time)
        if self.shake_timer > 0.0:
            self.shake_timer = max(0.0, self.shake_timer - delta_time)

    def trigger_loss_effects(self) -> None:
        self.flash_timer = 0.45
        self.shake_timer = 0.35

    def render_scanlines(self, width: int, height: int) -> None:
        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glColor4f(BLACK[0], BLACK[1], BLACK[2], SCANLINE_ALPHA)
        glLineWidth(1.0)

        glBegin(GL_LINES)
        for y in range(0, height, 3):
            glVertex2f(0.0, float(y))
            glVertex2f(float(width), float(y))
        glEnd()

        glDisable(GL_BLEND)

    def render_crt_bezel(self, width: int, height: int) -> None:
        w = float(width)
        h = float(height)
        border = 8.0

        glColor3f(*BLACK)
        glBegin(GL_QUADS)
        # Top
        glVertex2f(0, 0); glVertex2f(w, 0); glVertex2f(w, border); glVertex2f(0, border)
        # Bottom
        glVertex2f(0, h - border); glVertex2f(w, h - border); glVertex2f(w, h); glVertex2f(0, h)
        # Left
        glVertex2f(0, 0); glVertex2f(border, 0); glVertex2f(border, h); glVertex2f(0, h)
        # Right
        glVertex2f(w - border, 0); glVertex2f(w, 0); glVertex2f(w, h); glVertex2f(w - border, h)
        glEnd()

        glColor3f(*DIM_GREEN)
        glLineWidth(1.5)
        glBegin(GL_LINE_LOOP)
        glVertex2f(border, border)
        glVertex2f(w - border, border)
        glVertex2f(w - border, h - border)
        glVertex2f(border, h - border)
        glEnd()

    def render_screen_flash(self, width: int, height: int) -> None:
        if self.flash_timer <= 0.0:
            return

        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)

        alpha = (self.flash_timer / 0.45) * 0.5
        glColor4f(RED[0], RED[1], RED[2], alpha)

        glBegin(GL_QUADS)
        glVertex2f(0.0, 0.0)
        glVertex2f(float(width), 0.0)
        glVertex2f(float(width), float(height))
        glVertex2f(0.0, float(height))
        glEnd()

        glDisable(GL_BLEND)

    def render_menu(self, game: GameManager, width: int, height: int) -> None:
        draw_text("SNAKE ON A CUBE", 0, height * 0.22, 5.0,
                  *NEON_GREEN, centered=True, window_width=width)

        draw_text(f"HIGH SCORE  {game.high_score:05d}", 0, height * 0.42, 3.0,
                  *AMBER, centered=True, window_width=width)

        blink_on = (int(self.blink_timer * 2.0) % 2) == 0
        if blink_on:
            draw_text("PRESS SPACE TO START", 0, height * 0.62, 3.2,
                      *AMBER, centered=True, window_width=width)

        draw_text("[ARROW KEYS / WASD] MOVE", 0, height * 0.84, 2.0,
                  *AMBER, centered=True, window_width=width)

    def render_playing_hud(self, game: GameManager, width: int, height: int) -> None:
        draw_text(f"SCORE {game.score:05d}", 24, 20, 2.4, *AMBER)
        draw_text(f"HIGH {game.high_score:05d}", 0, 20, 2.4, *AMBER, centered=True, window_width=width)

        face_str = f"FACE {face_to_string(game.get_active_face())}"
        face_w = get_text_width(face_str, 2.4)
        draw_text(face_str, width - face_w - 24, 20, 2.4, *CYAN)

        draw_text("[P] PAUSE", 24, height - 28, 1.8, *AMBER)

    def render_paused(self, width: int, height: int) -> None:
        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glColor4f(BLACK[0], BLACK[1], BLACK[2], 0.85)
        glBegin(GL_QUADS)
        glVertex2f(0, 0)
        glVertex2f(float(width), 0)
        glVertex2f(float(width), float(height))
        glVertex2f(0, float(height))
        glEnd()
        glDisable(GL_BLEND)

        draw_text("PAUSED", 0, height * 0.40, 5.5, *AMBER, centered=True, window_width=width)

        blink_on = (int(self.blink_timer * 2.0) % 2) == 0
        if blink_on:
            draw_text("PRESS P TO RESUME", 0, height * 0.54, 2.8, *AMBER, centered=True, window_width=width)

    def render_game_over(self, game: GameManager, width: int, height: int) -> None:
        draw_text("GAME OVER", 0, height * 0.28, 5.5, *RED, centered=True, window_width=width)
        draw_text(f"FINAL SCORE  {game.score:05d}", 0, height * 0.46, 3.2, *AMBER, centered=True, window_width=width)

        if game.score > 0 and game.score >= game.high_score:
            flash = (int(self.blink_timer * 4.0) % 2) == 0
            if flash:
                draw_text("** NEW HIGH SCORE **", 0, height * 0.56, 2.6, *NEON_GREEN, centered=True, window_width=width)

        blink = (int(self.blink_timer * 2.0) % 2) == 0
        if blink:
            draw_text("PRESS SPACE FOR MENU", 0, height * 0.68, 3.0, *AMBER, centered=True, window_width=width)

        draw_text("PRESS R TO RESTART", 0, height * 0.80, 2.0, *AMBER, centered=True, window_width=width)

    def render(self, game: GameManager, window_width: int, window_height: int, anim_time: float) -> None:
        glDisable(GL_LIGHTING)
        glDisable(GL_DEPTH_TEST)

        glMatrixMode(GL_PROJECTION)
        glPushMatrix()
        glLoadIdentity()
        gluOrtho2D(0, window_width, window_height, 0)

        glMatrixMode(GL_MODELVIEW)
        glPushMatrix()
        glLoadIdentity()

        state = game.state
        if state == GameState.MENU:
            self.render_menu(game, window_width, window_height)
        elif state == GameState.PLAYING:
            self.render_playing_hud(game, window_width, window_height)
        elif state == GameState.PAUSED:
            self.render_playing_hud(game, window_width, window_height)
            self.render_paused(window_width, window_height)
        elif state == GameState.GAME_OVER:
            self.render_game_over(game, window_width, window_height)

        self.render_screen_flash(window_width, window_height)
        self.render_scanlines(window_width, window_height)
        self.render_crt_bezel(window_width, window_height)

        glPopMatrix()
        glMatrixMode(GL_PROJECTION)
        glPopMatrix()
        glMatrixMode(GL_MODELVIEW)

        glEnable(GL_DEPTH_TEST)
