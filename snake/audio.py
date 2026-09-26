"""
Synthesized 8-bit Retro Arcade Audio Engine for Snake on a Cube.
Generates authentic chiptune waveforms without external audio files.
"""

import math
import struct
import pygame

SAMPLE_RATE = 44100

def _generate_square_tone(freq: float, duration: float, volume: float = 0.3) -> pygame.mixer.Sound:
    """Generate a square wave sound buffer."""
    num_samples = int(SAMPLE_RATE * duration)
    buf = bytearray()
    period = SAMPLE_RATE / max(freq, 1.0)
    amplitude = int(32767 * volume)

    for i in range(num_samples):
        # Square wave with duty cycle 0.5
        val = amplitude if (i % period) < (period * 0.5) else -amplitude
        # Apply slight linear decay envelope to eliminate clicking
        decay = 1.0 - (i / num_samples) * 0.4
        val = int(val * decay)
        buf.extend(struct.pack('<hh', val, val))

    return pygame.mixer.Sound(buffer=bytes(buf))

def _generate_arpeggio(freqs: list, note_duration: float, volume: float = 0.35) -> pygame.mixer.Sound:
    """Generate an ascending/descending retro arpeggio."""
    buf = bytearray()
    amplitude = int(32767 * volume)

    for idx, freq in enumerate(freqs):
        num_samples = int(SAMPLE_RATE * note_duration)
        period = SAMPLE_RATE / max(freq, 1.0)
        for i in range(num_samples):
            val = amplitude if (i % period) < (period * 0.5) else -amplitude
            decay = 1.0 - (i / num_samples) * 0.3
            val = int(val * decay)
            buf.extend(struct.pack('<hh', val, val))

    return pygame.mixer.Sound(buffer=bytes(buf))

def _generate_frequency_sweep(start_freq: float, end_freq: float, duration: float, volume: float = 0.35) -> pygame.mixer.Sound:
    """Generate a continuous frequency sweep (buzz/deflate or laser)."""
    num_samples = int(SAMPLE_RATE * duration)
    buf = bytearray()
    phase = 0.0
    amplitude = int(32767 * volume)

    for i in range(num_samples):
        t = i / num_samples
        cur_freq = start_freq + (end_freq - start_freq) * t
        phase += 2.0 * math.pi * cur_freq / SAMPLE_RATE

        # Mixed square wave + harmonics for arcade bite
        val = amplitude if (math.sin(phase) >= 0) else -amplitude
        # Envelope decay
        envelope = (1.0 - t * 0.8)
        val = int(val * envelope)
        buf.extend(struct.pack('<hh', val, val))

    return pygame.mixer.Sound(buffer=bytes(buf))

def _generate_noise_crash(duration: float, volume: float = 0.4) -> pygame.mixer.Sound:
    """Generate an 8-bit arcade explosion / impact crash."""
    import random
    num_samples = int(SAMPLE_RATE * duration)
    buf = bytearray()
    amplitude = int(32767 * volume)

    cur_val = amplitude
    step = 0
    freq_div = 8

    for i in range(num_samples):
        t = i / num_samples
        step += 1
        if step >= freq_div:
            step = 0
            cur_val = random.randint(-amplitude, amplitude)
            # Pitch drops over time (larger division)
            freq_div = int(8 + t * 40)

        envelope = max(0.0, 1.0 - t)
        val = int(cur_val * envelope)
        buf.extend(struct.pack('<hh', val, val))

    return pygame.mixer.Sound(buffer=bytes(buf))

class SoundManager:
    """Manages retro 8-bit sound effects."""

    def __init__(self):
        self.enabled = False
        try:
            if not pygame.mixer.get_init():
                pygame.mixer.init(SAMPLE_RATE, -16, 2, 512)
            self.enabled = True
        except Exception as e:
            print(f"[SoundManager] Warning: Audio init failed ({e}), running silent.")
            return

        self._snd_eat = None
        self._snd_shrink = None
        self._snd_warn = None
        self._snd_game_over = None
        self._snd_start = None
        self._snd_turn = None
        self._snd_pause = None

        self._build_sounds()

    def _build_sounds(self):
        if not self.enabled:
            return
        try:
            # 1. Eat food: High 2-tone arcade chirp (587Hz -> 880Hz)
            self._snd_eat = _generate_arpeggio([587.3, 880.0, 1174.6], 0.045, volume=0.35)

            # 2. Halve snake penalty: Deflating pitch dive (650Hz -> 110Hz)
            self._snd_shrink = _generate_frequency_sweep(650.0, 110.0, 0.38, volume=0.45)

            # 3. Urgent timer warning tick: Short 987Hz arcade blip
            self._snd_warn = _generate_square_tone(987.77, 0.055, volume=0.3)

            # 4. Game over: 8-bit crash explosion
            self._snd_game_over = _generate_noise_crash(0.55, volume=0.45)

            # 5. Game start: 4-note ascending arcade fanfare
            self._snd_start = _generate_arpeggio([392.0, 523.25, 659.25, 783.99], 0.07, volume=0.35)

            # 6. Direction turn: Subtle 220Hz click
            self._snd_turn = _generate_square_tone(220.0, 0.02, volume=0.15)

            # 7. Pause toggle: Mid blip
            self._snd_pause = _generate_arpeggio([440.0, 554.3], 0.04, volume=0.25)
        except Exception as e:
            print(f"[SoundManager] Warning: Failed to synthesize sounds ({e}).")
            self.enabled = False

    def play_eat(self):
        if self.enabled and self._snd_eat:
            self._snd_eat.play()

    def play_shrink(self):
        if self.enabled and self._snd_shrink:
            self._snd_shrink.play()

    def play_warn(self):
        if self.enabled and self._snd_warn:
            self._snd_warn.play()

    def play_game_over(self):
        if self.enabled and self._snd_game_over:
            self._snd_game_over.play()

    def play_start(self):
        if self.enabled and self._snd_start:
            self._snd_start.play()

    def play_turn(self):
        if self.enabled and self._snd_turn:
            self._snd_turn.play()

    def play_pause(self):
        if self.enabled and self._snd_pause:
            self._snd_pause.play()
