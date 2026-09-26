"""
Orbit Camera with exponential smoothing and target face tracking for 3D Snake.
"""

import math
from OpenGL.GLU import gluLookAt
from cube_topology import Face, Vec3, get_face_basis

class OrbitCamera:
    def __init__(self, distance: float = 13.5, lerp_speed: float = 7.0):
        self.distance: float = distance
        self.lerp_speed: float = lerp_speed
        self.active_face: Face = Face.PZ

        self.cur_eye: Vec3 = Vec3(0, 0, distance)
        self.cur_up: Vec3 = Vec3(0, 1, 0)
        self.target_eye: Vec3 = Vec3(0, 0, distance)
        self.target_up: Vec3 = Vec3(0, 1, 0)

        self.set_target_face(Face.PZ)
        self.cur_eye = self.target_eye
        self.cur_up = self.target_up

    def compute_target_vectors(self, face: Face):
        basis = get_face_basis(face)
        # Slightly elevate camera along v_axis (up) and normal
        # so adjacent sides of the 3D cube are visible for depth perception
        eye_dir = (basis.normal * 1.0) + (basis.v_axis * 0.28) + (basis.u_axis * 0.15)
        eye_dir = eye_dir.normalized()

        target_eye = eye_dir * self.distance
        target_up = basis.v_axis
        return target_eye, target_up

    def set_target_face(self, face: Face) -> None:
        self.active_face = face
        self.target_eye, self.target_up = self.compute_target_vectors(face)

    def update(self, delta_time: float) -> None:
        # Frame-rate independent exponential smoothing
        blend = 1.0 - math.exp(-self.lerp_speed * delta_time)
        if blend > 1.0:
            blend = 1.0

        self.cur_eye = self.cur_eye + ((self.target_eye - self.cur_eye) * blend)
        self.cur_up = self.cur_up + ((self.target_up - self.cur_up) * blend)

        # Maintain constant spherical radius
        self.cur_eye = self.cur_eye.normalized() * self.distance

        # Orthonormalize Up vector against Eye vector
        look_dir = Vec3(-self.cur_eye.x, -self.cur_eye.y, -self.cur_eye.z).normalized()
        right = look_dir.cross(self.cur_up)
        if right.length() > 1e-4:
            right = right.normalized()
            self.cur_up = right.cross(look_dir).normalized()

    def apply_view(self) -> None:
        gluLookAt(
            self.cur_eye.x, self.cur_eye.y, self.cur_eye.z,
            0.0, 0.0, 0.0,
            self.cur_up.x, self.cur_up.y, self.cur_up.z
        )

    def add_manual_rotation(self, delta_pitch: float, delta_yaw: float) -> None:
        cos_y = math.cos(delta_yaw)
        sin_y = math.sin(delta_yaw)
        new_x = self.cur_eye.x * cos_y - self.cur_eye.z * sin_y
        new_z = self.cur_eye.x * sin_y + self.cur_eye.z * cos_y
        self.cur_eye = Vec3(new_x, self.cur_eye.y, new_z).normalized() * self.distance
