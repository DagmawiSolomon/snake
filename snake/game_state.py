"""
Game logic and state management for Snake on a Cube 3D.
Features dynamic aggressive urgency countdown timers and snake size halving penalties.
"""

from collections import deque
from enum import Enum, auto
import random
from typing import Deque, List, Tuple
from .cube_topology import Face, Dir, GridPos, GRID_N, step_position, is_opposite

class GameState(Enum):
    MENU = auto()
    PLAYING = auto()
    PAUSED = auto()
    GAME_OVER = auto()
    HELP = auto()

class Snake:
    def __init__(self):
        self.body: Deque[GridPos] = deque()
        self.current_dir: Dir = Dir.RIGHT
        self.pending_dir: Dir = Dir.RIGHT
        self.grow_pending: bool = False

INITIAL_TICK_MS: int = 160
MIN_TICK_MS: int = 75
SPEED_RAMP_MS: int = 3

# Progressive countdown difficulty ramp (scales aggressively the longer you survive)
INITIAL_FOOD_TIMER: float = 12.0          # Starting generous window (12.0 seconds)
MIN_FOOD_TIMER: float = 4.0               # Floor cap for extreme arcade frenzy (4.0 seconds)
FOOD_TIMER_DECAY_PER_APPLE: float = 0.35  # Tighter timer for every apple eaten
FOOD_TIMER_DECAY_PER_SEC: float = 0.045   # Time decay: -1.0s every ~22 seconds survived!

class GameManager:
    def __init__(self):
        self.state: GameState = GameState.MENU
        self.snake: Snake = Snake()
        self.food: GridPos = GridPos(Face.PZ, 0, 0)
        self.score: int = 0
        self.high_score: int = 0
        self.tick_interval_ms: int = INITIAL_TICK_MS

        # Dynamic urgency timer mechanics
        self.apples_eaten: int = 0
        self.play_time: float = 0.0
        self.food_timer_max: float = INITIAL_FOOD_TIMER
        self.food_timer: float = INITIAL_FOOD_TIMER
        self.halved_recently: float = 0.0
        self.just_ate: bool = False
        self.warn_sound_pending: bool = False
        self._last_warn_tick: int = int(INITIAL_FOOD_TIMER)

        self.init_new_game()

    def compute_current_food_timer_max(self) -> float:
        """
        Calculates dynamic countdown duration based on apples eaten and play duration.
        Gets progressively tighter and more aggressive the longer the player lasts.
        """
        # Time survival decay: ramps up as you survive longer
        time_factor = (self.play_time * FOOD_TIMER_DECAY_PER_SEC) + (max(0.0, self.play_time - 30.0) * 0.02)
        apple_factor = self.apples_eaten * FOOD_TIMER_DECAY_PER_APPLE
        total_decay = time_factor + apple_factor
        return max(MIN_FOOD_TIMER, INITIAL_FOOD_TIMER - total_decay)

    @property
    def aggression_ratio(self) -> float:
        """Progressive aggression meter from 0.0 (start) to 1.0 (extreme frenzy)."""
        span = INITIAL_FOOD_TIMER - MIN_FOOD_TIMER
        if span <= 0:
            return 0.0
        return max(0.0, min(1.0, (INITIAL_FOOD_TIMER - self.food_timer_max) / span))

    @property
    def urgency_threshold(self) -> float:
        """Dynamic threshold when the screen starts pulsing red and warning beeps play."""
        return min(4.0, max(2.0, self.food_timer_max * 0.45))

    def init_new_game(self) -> None:
        self.snake.body.clear()
        start_u = GRID_N // 2
        start_v = GRID_N // 2

        self.snake.body.append(GridPos(Face.PZ, start_u, start_v))       # head
        self.snake.body.append(GridPos(Face.PZ, start_u - 1, start_v))   # mid
        self.snake.body.append(GridPos(Face.PZ, start_u - 2, start_v))   # tail

        self.snake.current_dir = Dir.RIGHT
        self.snake.pending_dir = Dir.RIGHT
        self.snake.grow_pending = False

        self.score = 0
        self.tick_interval_ms = INITIAL_TICK_MS

        self.apples_eaten = 0
        self.play_time = 0.0
        self.food_timer_max = INITIAL_FOOD_TIMER
        self.food_timer = INITIAL_FOOD_TIMER
        self.halved_recently = 0.0
        self.just_ate = False
        self.warn_sound_pending = False
        self._last_warn_tick = int(INITIAL_FOOD_TIMER)

        self.spawn_food()

    def reset(self) -> None:
        self.init_new_game()
        self.state = GameState.PLAYING

    def start_game(self) -> None:
        self.init_new_game()
        self.state = GameState.PLAYING

    def toggle_pause(self) -> None:
        if self.state == GameState.PLAYING:
            self.state = GameState.PAUSED
        elif self.state == GameState.PAUSED:
            self.state = GameState.PLAYING

    def request_direction(self, dir_: Dir) -> bool:
        """Requests a direction change. Returns True if accepted (not 180 reversal)."""
        if self.state != GameState.PLAYING:
            return False
        if not is_opposite(dir_, self.snake.current_dir) and dir_ != self.snake.current_dir:
            self.snake.pending_dir = dir_
            return True
        return False

    def is_occupied_by_snake(self, pos: GridPos) -> bool:
        return pos in self.snake.body

    def spawn_food(self) -> None:
        free_cells: List[GridPos] = []
        for f in range(6):
            face = Face(f)
            for u in range(GRID_N):
                for v in range(GRID_N):
                    candidate = GridPos(face, u, v)
                    if not self.is_occupied_by_snake(candidate):
                        free_cells.append(candidate)

        if not free_cells:
            self.state = GameState.GAME_OVER
            return

        self.food = random.choice(free_cells)

    def get_active_face(self) -> Face:
        if self.snake.body:
            return self.snake.body[0].face
        return Face.PZ

    def update_timers(self, dt: float) -> Tuple[bool, bool]:
        """
        Updates the food countdown timer and urgency alerts.
        Returns: (halved_penalty_triggered, warn_tick_triggered)
        """
        halved = False
        warn_tick = False

        if self.halved_recently > 0.0:
            self.halved_recently = max(0.0, self.halved_recently - dt)

        if self.state != GameState.PLAYING:
            return halved, warn_tick

        self.play_time += dt
        self.food_timer -= dt

        # Check for warning blips when timer <= urgency threshold
        # In critical final moments (<= 1.8s), trigger double-speed panic warnings (every 0.5s)
        if self.food_timer <= 1.8:
            cur_tick = int(self.food_timer * 2.0)
        else:
            cur_tick = int(self.food_timer)

        if self.food_timer <= self.urgency_threshold and cur_tick != self._last_warn_tick and self.food_timer >= 0.0:
            self._last_warn_tick = cur_tick
            warn_tick = True

        # Countdown expired: Halve the snake in size!
        if self.food_timer <= 0.0:
            cur_len = len(self.snake.body)
            # Retain minimum length of 3 segments
            new_len = max(3, cur_len // 2)
            while len(self.snake.body) > new_len:
                self.snake.body.pop()

            # Recalculate dynamic max timer and reset
            self.food_timer_max = self.compute_current_food_timer_max()
            self.food_timer = self.food_timer_max
            self._last_warn_tick = int(self.food_timer_max)
            self.halved_recently = 1.8  # show notification banner for 1.8s
            halved = True

        return halved, warn_tick

    def tick(self) -> bool:
        """
        Executes one game tick. Returns True if snake moved, False if collision / game over.
        """
        self.just_ate = False

        if self.state != GameState.PLAYING:
            return False

        # 1. Apply buffered input
        if not is_opposite(self.snake.pending_dir, self.snake.current_dir):
            self.snake.current_dir = self.snake.pending_dir

        # 2. Compute candidate head
        next_head, next_dir = step_position(self.snake.body[0], self.snake.current_dir)

        # 3. Collision check
        check_count = len(self.snake.body) if self.snake.grow_pending else (len(self.snake.body) - 1)
        for i in range(check_count):
            if self.snake.body[i] == next_head:
                self.state = GameState.GAME_OVER
                return False

        # 4. Update direction
        self.snake.current_dir = next_dir
        self.snake.pending_dir = next_dir

        # 5. Insert new head
        self.snake.body.appendleft(next_head)

        # 6. Check food
        if next_head == self.food:
            self.score += 10
            self.apples_eaten += 1
            if self.score > self.high_score:
                self.high_score = self.score
            self.snake.grow_pending = True
            self.just_ate = True

            # Recalculate dynamic aggressive timer window
            self.food_timer_max = self.compute_current_food_timer_max()
            self.food_timer = self.food_timer_max
            self._last_warn_tick = int(self.food_timer_max)

            # Speed ramp
            if self.tick_interval_ms > MIN_TICK_MS:
                self.tick_interval_ms = max(MIN_TICK_MS, self.tick_interval_ms - SPEED_RAMP_MS)

            self.spawn_food()
        else:
            self.snake.body.pop()
            self.snake.grow_pending = False

        return True
