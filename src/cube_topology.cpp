#include "cube_topology.h"

// Initialize the 6 FaceBasis definitions
static const FaceBasis s_faceBases[6] = {
    // PX (+X face): u along -Z, v along +Y, normal is +X
    // Center at x = +HALF_CUBE. (u=0 is z=+HALF_CUBE, u=N-1 is z=-HALF_CUBE)
    {
        Vec3(HALF_CUBE, -HALF_CUBE, HALF_CUBE), // origin (u=0, v=0 corner)
        Vec3(0, 0, -1),                         // uAxis
        Vec3(0, 1, 0),                          // vAxis
        Vec3(1, 0, 0)                           // normal
    },
    // NX (-X face): u along +Z, v along +Y, normal is -X
    // Center at x = -HALF_CUBE. (u=0 is z=-HALF_CUBE, u=N-1 is z=+HALF_CUBE)
    {
        Vec3(-HALF_CUBE, -HALF_CUBE, -HALF_CUBE),
        Vec3(0, 0, 1),
        Vec3(0, 1, 0),
        Vec3(-1, 0, 0)
    },
    // PY (+Y face): u along +X, v along -Z, normal is +Y
    // Center at y = +HALF_CUBE. (u=0 is x=-HALF_CUBE, v=0 is z=+HALF_CUBE)
    {
        Vec3(-HALF_CUBE, HALF_CUBE, HALF_CUBE),
        Vec3(1, 0, 0),
        Vec3(0, 0, -1),
        Vec3(0, 1, 0)
    },
    // NY (-Y face): u along +X, v along +Z, normal is -Y
    // Center at y = -HALF_CUBE. (u=0 is x=-HALF_CUBE, v=0 is z=-HALF_CUBE)
    {
        Vec3(-HALF_CUBE, -HALF_CUBE, -HALF_CUBE),
        Vec3(1, 0, 0),
        Vec3(0, 0, 1),
        Vec3(0, -1, 0)
    },
    // PZ (+Z face): u along +X, v along +Y, normal is +Z
    // Center at z = +HALF_CUBE. (u=0 is x=-HALF_CUBE, v=0 is y=-HALF_CUBE)
    {
        Vec3(-HALF_CUBE, -HALF_CUBE, HALF_CUBE),
        Vec3(1, 0, 0),
        Vec3(0, 1, 0),
        Vec3(0, 0, 1)
    },
    // NZ (-Z face): u along -X, v along +Y, normal is -Z
    // Center at z = -HALF_CUBE. (u=0 is x=+HALF_CUBE, v=0 is y=-HALF_CUBE)
    {
        Vec3(HALF_CUBE, -HALF_CUBE, -HALF_CUBE),
        Vec3(-1, 0, 0),
        Vec3(0, 1, 0),
        Vec3(0, 0, -1)
    }
};

const FaceBasis& getFaceBasis(Face f) {
    return s_faceBases[static_cast<int>(f)];
}

Vec3 localToWorld(Face face, int u, int v, float normalOffset) {
    const FaceBasis& b = getFaceBasis(face);
    float cu = (u + 0.5f) * CELL_SIZE;
    float cv = (v + 0.5f) * CELL_SIZE;
    return b.origin + (b.uAxis * cu) + (b.vAxis * cv) + (b.normal * normalOffset);
}

Vec3 cellCornerToWorld(Face face, int u, int v, float cornerU, float cornerV, float normalOffset) {
    const FaceBasis& b = getFaceBasis(face);
    float cu = (u + cornerU) * CELL_SIZE;
    float cv = (v + cornerV) * CELL_SIZE;
    return b.origin + (b.uAxis * cu) + (b.vAxis * cv) + (b.normal * normalOffset);
}

bool isOpposite(Dir a, Dir b) {
    if (a == Dir::UP && b == Dir::DOWN) return true;
    if (a == Dir::DOWN && b == Dir::UP) return true;
    if (a == Dir::LEFT && b == Dir::RIGHT) return true;
    if (a == Dir::RIGHT && b == Dir::LEFT) return true;
    return false;
}

const char* faceToString(Face f) {
    switch (f) {
        case Face::PX: return "+X";
        case Face::NX: return "-X";
        case Face::PY: return "+Y";
        case Face::NY: return "-Y";
        case Face::PZ: return "+Z";
        case Face::NZ: return "-Z";
    }
    return "?";
}

const char* dirToString(Dir d) {
    switch (d) {
        case Dir::UP: return "UP";
        case Dir::DOWN: return "DOWN";
        case Dir::LEFT: return "LEFT";
        case Dir::RIGHT: return "RIGHT";
    }
    return "?";
}

void stepPosition(const GridPos& current, Dir dir, GridPos& outNext, Dir& outNextDir) {
    int nu = current.u;
    int nv = current.v;

    switch (dir) {
        case Dir::UP:    nv++; break;
        case Dir::DOWN:  nv--; break;
        case Dir::LEFT:  nu--; break;
        case Dir::RIGHT: nu++; break;
    }

    // If within face boundary, no transition needed
    if (nu >= 0 && nu < GRID_N && nv >= 0 && nv < GRID_N) {
        outNext.face = current.face;
        outNext.u = nu;
        outNext.v = nv;
        outNextDir = dir;
        return;
    }

    // Boundary crossing: 24 transitions
    Face curFace = current.face;
    Face nextFace = curFace;
    Dir nextDir = dir;
    int resU = nu;
    int resV = nv;
    const int N1 = GRID_N - 1;

    switch (curFace) {
        case Face::PZ:
            if (dir == Dir::LEFT) {
                nextFace = Face::NX; nextDir = Dir::LEFT; resU = N1; resV = current.v;
            } else if (dir == Dir::RIGHT) {
                nextFace = Face::PX; nextDir = Dir::RIGHT; resU = 0; resV = current.v;
            } else if (dir == Dir::UP) {
                nextFace = Face::PY; nextDir = Dir::UP; resU = current.u; resV = 0;
            } else if (dir == Dir::DOWN) {
                nextFace = Face::NY; nextDir = Dir::DOWN; resU = current.u; resV = N1;
            }
            break;

        case Face::PX:
            if (dir == Dir::LEFT) {
                nextFace = Face::PZ; nextDir = Dir::LEFT; resU = N1; resV = current.v;
            } else if (dir == Dir::RIGHT) {
                nextFace = Face::NZ; nextDir = Dir::RIGHT; resU = 0; resV = current.v;
            } else if (dir == Dir::UP) {
                nextFace = Face::PY; nextDir = Dir::LEFT; resU = N1; resV = current.u;
            } else if (dir == Dir::DOWN) {
                nextFace = Face::NY; nextDir = Dir::LEFT; resU = N1; resV = N1 - current.u;
            }
            break;

        case Face::NZ:
            if (dir == Dir::LEFT) {
                nextFace = Face::PX; nextDir = Dir::LEFT; resU = N1; resV = current.v;
            } else if (dir == Dir::RIGHT) {
                nextFace = Face::NX; nextDir = Dir::RIGHT; resU = 0; resV = current.v;
            } else if (dir == Dir::UP) {
                nextFace = Face::PY; nextDir = Dir::DOWN; resU = N1 - current.u; resV = N1;
            } else if (dir == Dir::DOWN) {
                nextFace = Face::NY; nextDir = Dir::UP; resU = N1 - current.u; resV = 0;
            }
            break;

        case Face::NX:
            if (dir == Dir::LEFT) {
                nextFace = Face::NZ; nextDir = Dir::LEFT; resU = N1; resV = current.v;
            } else if (dir == Dir::RIGHT) {
                nextFace = Face::PZ; nextDir = Dir::RIGHT; resU = 0; resV = current.v;
            } else if (dir == Dir::UP) {
                nextFace = Face::PY; nextDir = Dir::RIGHT; resU = 0; resV = N1 - current.u;
            } else if (dir == Dir::DOWN) {
                nextFace = Face::NY; nextDir = Dir::RIGHT; resU = 0; resV = current.u;
            }
            break;

        case Face::PY:
            if (dir == Dir::LEFT) {
                nextFace = Face::NX; nextDir = Dir::DOWN; resU = N1 - current.v; resV = N1;
            } else if (dir == Dir::RIGHT) {
                nextFace = Face::PX; nextDir = Dir::DOWN; resU = current.v; resV = N1;
            } else if (dir == Dir::UP) {
                nextFace = Face::NZ; nextDir = Dir::DOWN; resU = N1 - current.u; resV = N1;
            } else if (dir == Dir::DOWN) {
                nextFace = Face::PZ; nextDir = Dir::DOWN; resU = current.u; resV = N1;
            }
            break;

        case Face::NY:
            if (dir == Dir::LEFT) {
                nextFace = Face::NX; nextDir = Dir::UP; resU = current.v; resV = 0;
            } else if (dir == Dir::RIGHT) {
                nextFace = Face::PX; nextDir = Dir::UP; resU = N1 - current.v; resV = 0;
            } else if (dir == Dir::UP) {
                nextFace = Face::PZ; nextDir = Dir::UP; resU = current.u; resV = 0;
            } else if (dir == Dir::DOWN) {
                nextFace = Face::NZ; nextDir = Dir::UP; resU = N1 - current.u; resV = 0;
            }
            break;
    }

    outNext.face = nextFace;
    outNext.u = resU;
    outNext.v = resV;
    outNextDir = nextDir;
}
