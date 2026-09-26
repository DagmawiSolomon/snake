"""
Cube topology and geometry for Snake on a Cube 3D.
"""

from enum import IntEnum
import math
from typing import NamedTuple, Tuple

class Face(IntEnum):
    PX = 0
    NX = 1
    PY = 2
    NY = 3
    PZ = 4
    NZ = 5

class Dir(IntEnum):
    UP = 0
    DOWN = 1
    LEFT = 2
    RIGHT = 3

GRID_N: int = 10
CUBE_SIZE: float = 6.0
CELL_SIZE: float = CUBE_SIZE / float(GRID_N)
HALF_CUBE: float = CUBE_SIZE * 0.5

class Vec3:
    __slots__ = ('x', 'y', 'z')

    def __init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0):
        self.x = float(x)
        self.y = float(y)
        self.z = float(z)

    def __add__(self, o: 'Vec3') -> 'Vec3':
        return Vec3(self.x + o.x, self.y + o.y, self.z + o.z)

    def __sub__(self, o: 'Vec3') -> 'Vec3':
        return Vec3(self.x - o.x, self.y - o.y, self.z - o.z)

    def __mul__(self, s: float) -> 'Vec3':
        return Vec3(self.x * s, self.y * s, self.z * s)

    def cross(self, o: 'Vec3') -> 'Vec3':
        return Vec3(
            self.y * o.z - self.z * o.y,
            self.z * o.x - self.x * o.z,
            self.x * o.y - self.y * o.x
        )

    def length(self) -> float:
        return math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z)

    def normalized(self) -> 'Vec3':
        l = self.length()
        if l < 1e-6:
            return Vec3(0, 0, 1)
        return Vec3(self.x / l, self.y / l, self.z / l)

    def to_tuple(self) -> Tuple[float, float, float]:
        return (self.x, self.y, self.z)

    def __repr__(self) -> str:
        return f"Vec3({self.x:.3f}, {self.y:.3f}, {self.z:.3f})"

class GridPos(NamedTuple):
    face: Face
    u: int
    v: int

class FaceBasis:
    __slots__ = ('origin', 'u_axis', 'v_axis', 'normal')

    def __init__(self, origin: Vec3, u_axis: Vec3, v_axis: Vec3, normal: Vec3 = None):
        self.origin = origin
        self.u_axis = u_axis
        self.v_axis = v_axis
        if normal is None:
            self.normal = u_axis.cross(v_axis)
        else:
            self.normal = normal

_FACE_BASES = [
    # PX (+X face): u along -Z, v along +Y, normal is +X
    FaceBasis(
        Vec3(HALF_CUBE, -HALF_CUBE, HALF_CUBE),
        Vec3(0, 0, -1),
        Vec3(0, 1, 0),
        Vec3(1, 0, 0)
    ),
    # NX (-X face): u along +Z, v along +Y, normal is -X
    FaceBasis(
        Vec3(-HALF_CUBE, -HALF_CUBE, -HALF_CUBE),
        Vec3(0, 0, 1),
        Vec3(0, 1, 0),
        Vec3(-1, 0, 0)
    ),
    # PY (+Y face): u along +X, v along -Z, normal is +Y
    FaceBasis(
        Vec3(-HALF_CUBE, HALF_CUBE, HALF_CUBE),
        Vec3(1, 0, 0),
        Vec3(0, 0, -1),
        Vec3(0, 1, 0)
    ),
    # NY (-Y face): u along +X, v along +Z, normal is -Y
    FaceBasis(
        Vec3(-HALF_CUBE, -HALF_CUBE, -HALF_CUBE),
        Vec3(1, 0, 0),
        Vec3(0, 0, 1),
        Vec3(0, -1, 0)
    ),
    # PZ (+Z face): u along +X, v along +Y, normal is +Z
    FaceBasis(
        Vec3(-HALF_CUBE, -HALF_CUBE, HALF_CUBE),
        Vec3(1, 0, 0),
        Vec3(0, 1, 0),
        Vec3(0, 0, 1)
    ),
    # NZ (-Z face): u along -X, v along +Y, normal is -Z
    FaceBasis(
        Vec3(HALF_CUBE, -HALF_CUBE, -HALF_CUBE),
        Vec3(-1, 0, 0),
        Vec3(0, 1, 0),
        Vec3(0, 0, -1)
    )
]

# Ensure normal = u_axis x v_axis for consistent winding
for _b in _FACE_BASES:
    _b.normal = _b.u_axis.cross(_b.v_axis)

def get_face_basis(face: Face) -> FaceBasis:
    return _FACE_BASES[int(face)]

def local_to_world(face: Face, u: int, v: int, normal_offset: float = 0.0) -> Vec3:
    b = get_face_basis(face)
    cu = (u + 0.5) * CELL_SIZE
    cv = (v + 0.5) * CELL_SIZE
    return b.origin + (b.u_axis * cu) + (b.v_axis * cv) + (b.normal * normal_offset)

def cell_corner_to_world(face: Face, u: int, v: int, corner_u: float, corner_v: float, normal_offset: float = 0.0) -> Vec3:
    b = get_face_basis(face)
    cu = (u + corner_u) * CELL_SIZE
    cv = (v + corner_v) * CELL_SIZE
    return b.origin + (b.u_axis * cu) + (b.v_axis * cv) + (b.normal * normal_offset)

def is_opposite(a: Dir, b: Dir) -> bool:
    return (a == Dir.UP and b == Dir.DOWN) or \
           (a == Dir.DOWN and b == Dir.UP) or \
           (a == Dir.LEFT and b == Dir.RIGHT) or \
           (a == Dir.RIGHT and b == Dir.LEFT)

def face_to_string(f: Face) -> str:
    names = {
        Face.PX: "+X",
        Face.NX: "-X",
        Face.PY: "+Y",
        Face.NY: "-Y",
        Face.PZ: "+Z",
        Face.NZ: "-Z"
    }
    return names.get(f, "?")

def dir_to_string(d: Dir) -> str:
    names = {
        Dir.UP: "UP",
        Dir.DOWN: "DOWN",
        Dir.LEFT: "LEFT",
        Dir.RIGHT: "RIGHT"
    }
    return names.get(d, "?")

def step_position(current: GridPos, dir_: Dir) -> Tuple[GridPos, Dir]:
    nu = current.u
    nv = current.v

    if dir_ == Dir.UP:
        nv += 1
    elif dir_ == Dir.DOWN:
        nv -= 1
    elif dir_ == Dir.LEFT:
        nu -= 1
    elif dir_ == Dir.RIGHT:
        nu += 1

    # In-bounds check
    if 0 <= nu < GRID_N and 0 <= nv < GRID_N:
        return GridPos(current.face, nu, nv), dir_

    # Boundary crossing (24 exact transitions)
    cur_face = current.face
    next_face = cur_face
    next_dir = dir_
    res_u = nu
    res_v = nv
    N1 = GRID_N - 1

    if cur_face == Face.PZ:
        if dir_ == Dir.LEFT:
            next_face = Face.NX; next_dir = Dir.LEFT; res_u = N1; res_v = current.v
        elif dir_ == Dir.RIGHT:
            next_face = Face.PX; next_dir = Dir.RIGHT; res_u = 0; res_v = current.v
        elif dir_ == Dir.UP:
            next_face = Face.PY; next_dir = Dir.UP; res_u = current.u; res_v = 0
        elif dir_ == Dir.DOWN:
            next_face = Face.NY; next_dir = Dir.DOWN; res_u = current.u; res_v = N1

    elif cur_face == Face.PX:
        if dir_ == Dir.LEFT:
            next_face = Face.PZ; next_dir = Dir.LEFT; res_u = N1; res_v = current.v
        elif dir_ == Dir.RIGHT:
            next_face = Face.NZ; next_dir = Dir.RIGHT; res_u = 0; res_v = current.v
        elif dir_ == Dir.UP:
            next_face = Face.PY; next_dir = Dir.LEFT; res_u = N1; res_v = current.u
        elif dir_ == Dir.DOWN:
            next_face = Face.NY; next_dir = Dir.LEFT; res_u = N1; res_v = N1 - current.u

    elif cur_face == Face.NZ:
        if dir_ == Dir.LEFT:
            next_face = Face.PX; next_dir = Dir.LEFT; res_u = N1; res_v = current.v
        elif dir_ == Dir.RIGHT:
            next_face = Face.NX; next_dir = Dir.RIGHT; res_u = 0; res_v = current.v
        elif dir_ == Dir.UP:
            next_face = Face.PY; next_dir = Dir.DOWN; res_u = N1 - current.u; res_v = N1
        elif dir_ == Dir.DOWN:
            next_face = Face.NY; next_dir = Dir.UP; res_u = N1 - current.u; res_v = 0

    elif cur_face == Face.NX:
        if dir_ == Dir.LEFT:
            next_face = Face.NZ; next_dir = Dir.LEFT; res_u = N1; res_v = current.v
        elif dir_ == Dir.RIGHT:
            next_face = Face.PZ; next_dir = Dir.RIGHT; res_u = 0; res_v = current.v
        elif dir_ == Dir.UP:
            next_face = Face.PY; next_dir = Dir.RIGHT; res_u = 0; res_v = N1 - current.u
        elif dir_ == Dir.DOWN:
            next_face = Face.NY; next_dir = Dir.RIGHT; res_u = 0; res_v = current.u

    elif cur_face == Face.PY:
        if dir_ == Dir.LEFT:
            next_face = Face.NX; next_dir = Dir.DOWN; res_u = N1 - current.v; res_v = N1
        elif dir_ == Dir.RIGHT:
            next_face = Face.PX; next_dir = Dir.DOWN; res_u = current.v; res_v = N1
        elif dir_ == Dir.UP:
            next_face = Face.NZ; next_dir = Dir.DOWN; res_u = N1 - current.u; res_v = N1
        elif dir_ == Dir.DOWN:
            next_face = Face.PZ; next_dir = Dir.DOWN; res_u = current.u; res_v = N1

    elif cur_face == Face.NY:
        if dir_ == Dir.LEFT:
            next_face = Face.NX; next_dir = Dir.UP; res_u = current.v; res_v = 0
        elif dir_ == Dir.RIGHT:
            next_face = Face.PX; next_dir = Dir.UP; res_u = N1 - current.v; res_v = 0
        elif dir_ == Dir.UP:
            next_face = Face.PZ; next_dir = Dir.UP; res_u = current.u; res_v = 0
        elif dir_ == Dir.DOWN:
            next_face = Face.NZ; next_dir = Dir.UP; res_u = N1 - current.u; res_v = 0

    return GridPos(next_face, res_u, res_v), next_dir
