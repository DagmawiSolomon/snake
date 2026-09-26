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

    def test_food_timer_and_halving(self):
        """
        Tests that countdown timer running out halves the snake body in size.
        """
        game = GameManager()
        game.start_game()

        # Manually grow snake to length 10
        while len(game.snake.body) < 10:
            last = game.snake.body[-1]
            game.snake.body.append(GridPos(last.face, last.u, last.v))

        self.assertEqual(len(game.snake.body), 10)

        # Advance timer by 6 seconds (not expired yet)
        halved, _ = game.update_timers(6.0)
        self.assertFalse(halved)
        self.assertEqual(len(game.snake.body), 10)
        self.assertAlmostEqual(game.food_timer, 6.0, places=2)

        # Advance timer past expiration (remaining 6.5s)
        halved, _ = game.update_timers(6.5)
        self.assertTrue(halved)
        # 10 halved -> 5
        self.assertEqual(len(game.snake.body), 5)
        # Timer resets back to full
        self.assertEqual(game.food_timer, game.food_timer_max)

        # Halve again when length is 5 -> max(1, 5 // 2) = 2
        halved, _ = game.update_timers(12.5)
        self.assertTrue(halved)
        self.assertEqual(len(game.snake.body), 2)

        # Halve again when length is 2 -> max(1, 2 // 2) = 1 (head only)
        halved, _ = game.update_timers(12.5)
        self.assertTrue(halved)
        self.assertEqual(len(game.snake.body), 1)

        # Halve again when length is 1 -> Starvation triggers GAME_OVER
        halved, _ = game.update_timers(12.5)
        self.assertTrue(halved)
        self.assertEqual(game.state, GameState.GAME_OVER)

    def test_halving_without_eating_any_apple(self):
        """
        Tests that starting snake (length 3, 0 apples eaten) halves when countdown expires.
        """
        game = GameManager()
        game.start_game()
        self.assertEqual(len(game.snake.body), 3)
        self.assertEqual(game.apples_eaten, 0)

        # Countdown expires without eating any apple
        halved, _ = game.update_timers(game.food_timer + 0.1)
        self.assertTrue(halved)
        # 3 // 2 -> 1 segment (head only)
        self.assertEqual(len(game.snake.body), 1)

        # Expiration at length 1 triggers starvation Game Over
        halved, _ = game.update_timers(game.food_timer + 0.1)
        self.assertTrue(halved)
        self.assertEqual(game.state, GameState.GAME_OVER)

    def test_aggressive_timer_scaling(self):
        """
        Tests that food countdown timer window aggressively shrinks as play_time and apples_eaten increase.
        """
        game = GameManager()
        game.start_game()

        # At start: 12.0s
        t_start = game.compute_current_food_timer_max()
        self.assertEqual(t_start, 12.0)
        self.assertEqual(game.aggression_ratio, 0.0)

        # After surviving 40 seconds:
        game.play_time = 40.0
        t_40s = game.compute_current_food_timer_max()
        self.assertLess(t_40s, t_start)

        # After eating 6 apples and surviving 70 seconds:
        game.play_time = 70.0
        game.apples_eaten = 6
        t_mid = game.compute_current_food_timer_max()
        self.assertLess(t_mid, t_40s)

        # After extreme survival (150s + 15 apples): hits floor cap of 4.0s
        game.play_time = 150.0
        game.apples_eaten = 15
        t_frenzy = game.compute_current_food_timer_max()
        self.assertEqual(t_frenzy, 4.0)
        game.food_timer_max = t_frenzy
        self.assertAlmostEqual(game.aggression_ratio, 1.0, places=2)

if __name__ == '__main__':
    unittest.main()

