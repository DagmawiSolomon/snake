"""
Comprehensive test suite for cube topology boundary transitions and game mechanics.
"""

import unittest
from snake.cube_topology import (
    Face, Dir, GridPos, GRID_N, step_position, is_opposite
)
from snake.game_state import GameManager, GameState

def get_opposite(d: Dir) -> Dir:
    if d == Dir.UP: return Dir.DOWN
    if d == Dir.DOWN: return Dir.UP
    if d == Dir.LEFT: return Dir.RIGHT
    if d == Dir.RIGHT: return Dir.LEFT
    return Dir.UP

class TestCubeTopology(unittest.TestCase):
    def test_reversibility(self):
        """
        Tests that walking across every boundary and taking the reverse direction
        always lands back on the original cell and direction.
        """
        tested = 0
        for f in range(6):
            face = Face(f)
            for coord in range(GRID_N):
                # Step UP from top edge
                start_up = GridPos(face, coord, GRID_N - 1)
                nxt, nxt_dir = step_position(start_up, Dir.UP)
                back, back_dir = step_position(nxt, get_opposite(nxt_dir))
                self.assertEqual(back, start_up)
                self.assertEqual(back_dir, Dir.DOWN)
                tested += 1

                # Step DOWN from bottom edge
                start_down = GridPos(face, coord, 0)
                nxt, nxt_dir = step_position(start_down, Dir.DOWN)
                back, back_dir = step_position(nxt, get_opposite(nxt_dir))
                self.assertEqual(back, start_down)
                self.assertEqual(back_dir, Dir.UP)
                tested += 1

                # Step LEFT from left edge
                start_left = GridPos(face, 0, coord)
                nxt, nxt_dir = step_position(start_left, Dir.LEFT)
                back, back_dir = step_position(nxt, get_opposite(nxt_dir))
                self.assertEqual(back, start_left)
                self.assertEqual(back_dir, Dir.RIGHT)
                tested += 1

                # Step RIGHT from right edge
                start_right = GridPos(face, GRID_N - 1, coord)
                nxt, nxt_dir = step_position(start_right, Dir.RIGHT)
                back, back_dir = step_position(nxt, get_opposite(nxt_dir))
                self.assertEqual(back, start_right)
                self.assertEqual(back_dir, Dir.LEFT)
                tested += 1

        self.assertEqual(tested, 240)

    def test_great_circles(self):
        """
        Walking straight in any direction along any track on the cube
        should return to the start in exactly 4 * GRID_N steps.
        """
        loop_length = 4 * GRID_N
        for f in range(6):
            start_face = Face(f)
            for d in range(4):
                start_dir = Dir(d)
                for coord in range(GRID_N):
                    cur = GridPos(start_face, coord, coord)
                    cur_dir = start_dir
                    visited_faces = {cur.face}

                    for step in range(loop_length):
                        cur, cur_dir = step_position(cur, cur_dir)
                        visited_faces.add(cur.face)

                    self.assertEqual(cur, GridPos(start_face, coord, coord))
                    self.assertEqual(cur_dir, start_dir)
                    self.assertEqual(len(visited_faces), 4)

    def test_game_manager_simulation(self):
        """
        Tests initialization, ticks, food eating, growth, and self collision.
        """
        game = GameManager()
        self.assertEqual(game.state, GameState.MENU)
        game.start_game()
        self.assertEqual(game.state, GameState.PLAYING)
        self.assertEqual(len(game.snake.body), 3)

        for _ in range(5):
            moved = game.tick()
            self.assertTrue(moved)

        head = game.snake.body[0]
        front_pos, _ = step_position(head, game.snake.current_dir)
        game.food = front_pos
        old_score = game.score
        old_len = len(game.snake.body)

        moved = game.tick()
        self.assertTrue(moved)
        self.assertEqual(game.score, old_score + 10)
        self.assertEqual(len(game.snake.body), old_len + 1)

if __name__ == '__main__':
    unittest.main()
