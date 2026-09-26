"""
3D and 2D OpenGL Renderer for Snake on a Cube.
"""

import math
from OpenGL.GL import (
    glEnable, glDisable, glClear, glClearColor, glShadeModel, glDepthFunc,
    glLoadIdentity, glPushMatrix, glPopMatrix, glTranslatef, glRotatef,
    glBegin, glEnd, glVertex3f, glColor3f, glLineWidth,
    GL_DEPTH_TEST, GL_LEQUAL, GL_LIGHTING, GL_FLAT,
    GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_QUADS, GL_LINES
)
from OpenGL.GLU import gluNewQuadric, gluSphere
from .palette import BLACK, DIM_GREEN, BODY_GREEN, CYAN, RED
from .cube_topology import (
    Face, Dir, GRID_N, CELL_SIZE, HALF_CUBE,
    Vec3, GridPos, get_face_basis, local_to_world, cell_corner_to_world
)
from .game_state import GameState, GameManager, Snake
from .camera import OrbitCamera
from .ui import RetroArcadeUI

class Renderer:
    def __init__(self):
        self.ui: RetroArcadeUI = RetroArcadeUI()
        self._quadric = None

    def init_gl(self) -> None:
        glEnable(GL_DEPTH_TEST)
        glDepthFunc(GL_LEQUAL)

        # Flat retro arcade shading: disable 3D lighting so exact palette colors render true
        glDisable(GL_LIGHTING)
        glShadeModel(GL_FLAT)

        glClearColor(BLACK[0], BLACK[1], BLACK[2], 1.0)
        self._quadric = gluNewQuadric()

    def update(self, delta_time: float) -> None:
        self.ui.update(delta_time)

    def trigger_loss_effects(self) -> None:
        self.ui.trigger_loss_effects()

    def trigger_shrink_effects(self) -> None:
        self.ui.trigger_shrink_flash()


    def render_cube_and_grids(self) -> None:
        # 1. Solid black cube faces (#000000)
        glColor3f(*BLACK)
        glBegin(GL_QUADS)
        for f in range(6):
            face = Face(f)
            c0 = cell_corner_to_world(face, 0, 0, 0.0, 0.0)
            c1 = cell_corner_to_world(face, GRID_N - 1, 0, 1.0, 0.0)
            c2 = cell_corner_to_world(face, GRID_N - 1, GRID_N - 1, 1.0, 1.0)
            c3 = cell_corner_to_world(face, 0, GRID_N - 1, 0.0, 1.0)

            glVertex3f(c0.x, c0.y, c0.z)
            glVertex3f(c1.x, c1.y, c1.z)
            glVertex3f(c2.x, c2.y, c2.z)
            glVertex3f(c3.x, c3.y, c3.z)
        glEnd()

        # 2. Interior grid lines in Dim Green (#115511)
        glLineWidth(1.0)
        glColor3f(*DIM_GREEN)

        eps = 0.003
        glBegin(GL_LINES)
        for f in range(6):
            face = Face(f)
            for i in range(GRID_N + 1):
                # Horizontal line
                row_idx = i if i < GRID_N else GRID_N - 1
                row_frac = 0.0 if i < GRID_N else 1.0
                h0 = cell_corner_to_world(face, 0, row_idx, 0.0, row_frac, eps)
                h1 = cell_corner_to_world(face, GRID_N - 1, row_idx, 1.0, row_frac, eps)
                glVertex3f(h0.x, h0.y, h0.z)
                glVertex3f(h1.x, h1.y, h1.z)

                # Vertical line
                col_idx = i if i < GRID_N else GRID_N - 1
                col_frac = 0.0 if i < GRID_N else 1.0
                v0 = cell_corner_to_world(face, col_idx, 0, col_frac, 0.0, eps)
                v1 = cell_corner_to_world(face, col_idx, GRID_N - 1, col_frac, 1.0, eps)
                glVertex3f(v0.x, v0.y, v0.z)
                glVertex3f(v1.x, v1.y, v1.z)
        glEnd()

        # 3. Outer cube edges in Dim Green (#115511)
        h = HALF_CUBE + 0.003
        glLineWidth(2.0)
        glColor3f(*DIM_GREEN)
        glBegin(GL_LINES)
        # 12 edges of the cube
        glVertex3f(-h, -h, -h); glVertex3f(h, -h, -h)
        glVertex3f(h, -h, -h);  glVertex3f(h, -h, h)
        glVertex3f(h, -h, h);   glVertex3f(-h, -h, h)
        glVertex3f(-h, -h, h);  glVertex3f(-h, -h, -h)

        glVertex3f(-h, h, -h);  glVertex3f(h, h, -h)
        glVertex3f(h, h, -h);   glVertex3f(h, h, h)
        glVertex3f(h, h, h);    glVertex3f(-h, h, h)
        glVertex3f(-h, h, h);   glVertex3f(-h, h, -h)

        glVertex3f(-h, -h, -h); glVertex3f(-h, h, -h)
        glVertex3f(h, -h, -h);  glVertex3f(h, h, -h)
        glVertex3f(h, -h, h);   glVertex3f(h, h, h)
        glVertex3f(-h, -h, h);  glVertex3f(-h, h, h)
        glEnd()

    def render_cell_box(self, face: Face, u: int, v: int, scale: float, height: float,
                        r: float, g: float, b: float) -> None:
        basis = get_face_basis(face)
        center = local_to_world(face, u, v, 0.005)

        half_w = (CELL_SIZE * scale) * 0.5
        u_vec = basis.u_axis * half_w
        v_vec = basis.v_axis * half_w
        n_vec = basis.normal * height

        b00 = center - u_vec - v_vec
        b10 = center + u_vec - v_vec
        b11 = center + u_vec + v_vec
        b01 = center - u_vec + v_vec

        t00 = b00 + n_vec
        t10 = b10 + n_vec
        t11 = b11 + n_vec
        t01 = b01 + n_vec

        glColor3f(r, g, b)

        glBegin(GL_QUADS)
        # Top face
        glVertex3f(t00.x, t00.y, t00.z)
        glVertex3f(t10.x, t10.y, t10.z)
        glVertex3f(t11.x, t11.y, t11.z)
        glVertex3f(t01.x, t01.y, t01.z)

        # Front (+v_axis)
        glVertex3f(t01.x, t01.y, t01.z)
        glVertex3f(t11.x, t11.y, t11.z)
        glVertex3f(b11.x, b11.y, b11.z)
        glVertex3f(b01.x, b01.y, b01.z)

        # Back (-v_axis)
        glVertex3f(t00.x, t00.y, t00.z)
        glVertex3f(b00.x, b00.y, b00.z)
        glVertex3f(b10.x, b10.y, b10.z)
        glVertex3f(t10.x, t10.y, t10.z)

        # Right (+u_axis)
        glVertex3f(t10.x, t10.y, t10.z)
        glVertex3f(b10.x, b10.y, b10.z)
        glVertex3f(b11.x, b11.y, b11.z)
        glVertex3f(t11.x, t11.y, t11.z)

        # Left (-u_axis)
        glVertex3f(t00.x, t00.y, t00.z)
        glVertex3f(t01.x, t01.y, t01.z)
        glVertex3f(b01.x, b01.y, b01.z)
        glVertex3f(b00.x, b00.y, b00.z)

        # Bottom (-normal)
        glVertex3f(b00.x, b00.y, b00.z)
        glVertex3f(b01.x, b01.y, b01.z)
        glVertex3f(b11.x, b11.y, b11.z)
        glVertex3f(b10.x, b10.y, b10.z)
        glEnd()

    def render_snake(self, snake: Snake, anim_time: float) -> None:
        if not snake.body:
            return

        body_len = len(snake.body)

        # 1. Body segments: Flat Body Green (#33CC33)
        for i in range(1, body_len):
            seg = snake.body[i]
            self.render_cell_box(seg.face, seg.u, seg.v, 0.85, 0.22, *BODY_GREEN)

        # 2. Snake head: Flat Bright Cyan (#33FFFF)
        head = snake.body[0]
        self.render_cell_box(head.face, head.u, head.v, 0.90, 0.28, *CYAN)

        # 3. Directional eyes on snake head
        basis = get_face_basis(head.face)
        center = local_to_world(head.face, head.u, head.v, 0.29)

        fwd = Vec3(0, 0, 0)
        if snake.current_dir == Dir.UP:
            fwd = basis.v_axis
        elif snake.current_dir == Dir.DOWN:
            fwd = basis.v_axis * -1.0
        elif snake.current_dir == Dir.LEFT:
            fwd = basis.u_axis * -1.0
        elif snake.current_dir == Dir.RIGHT:
            fwd = basis.u_axis

        side = fwd.cross(basis.normal)

        eye_offset_fwd = CELL_SIZE * 0.22
        eye_offset_side = CELL_SIZE * 0.24
        eye_radius = CELL_SIZE * 0.08

        eye_l = center + (fwd * eye_offset_fwd) - (side * eye_offset_side)
        eye_r = center + (fwd * eye_offset_fwd) + (side * eye_offset_side)

        # Black pupils on cyan head (#000000)
        glColor3f(*BLACK)
        if self._quadric:
            glPushMatrix()
            glTranslatef(eye_l.x, eye_l.y, eye_l.z)
            gluSphere(self._quadric, eye_radius, 8, 8)
            glPopMatrix()

            glPushMatrix()
            glTranslatef(eye_r.x, eye_r.y, eye_r.z)
            gluSphere(self._quadric, eye_radius, 8, 8)
            glPopMatrix()

    def render_food(self, food: GridPos, anim_time: float) -> None:
        basis = get_face_basis(food.face)

        bob = 0.25 + 0.08 * math.sin(anim_time * 5.0)
        center = local_to_world(food.face, food.u, food.v, bob)

        glPushMatrix()
        glTranslatef(center.x, center.y, center.z)

        spin_angle = anim_time * 120.0
        glRotatef(spin_angle, basis.normal.x, basis.normal.y, basis.normal.z)

        food_radius = CELL_SIZE * 0.32

        # Flat Red (#FF3333)
        glColor3f(*RED)
        if self._quadric:
            gluSphere(self._quadric, food_radius, 10, 10)

        glPopMatrix()

    def render(self, game: GameManager, camera: OrbitCamera,
               window_width: int, window_height: int, anim_time: float) -> None:
        state = game.state

        glClearColor(BLACK[0], BLACK[1], BLACK[2], 1.0)

        if state in (GameState.MENU, GameState.GAME_OVER):
            glClear(GL_COLOR_BUFFER_BIT)
            self.ui.render(game, window_width, window_height, anim_time)
        else:
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
            glLoadIdentity()

            camera.apply_view()

            self.render_cube_and_grids()
            self.render_snake(game.snake, anim_time)
            self.render_food(game.food, anim_time)

            self.ui.render(game, window_width, window_height, anim_time)
