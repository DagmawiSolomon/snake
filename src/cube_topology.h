#ifndef CUBE_TOPOLOGY_H
#define CUBE_TOPOLOGY_H

#include <cmath>

enum class Face { PX = 0, NX = 1, PY = 2, NY = 3, PZ = 4, NZ = 5 };
enum class Dir { UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3 };

constexpr int GRID_N = 10;
constexpr float CUBE_SIZE = 6.0f;
constexpr float CELL_SIZE = CUBE_SIZE / (float)GRID_N;
constexpr float HALF_CUBE = CUBE_SIZE * 0.5f;

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }

    Vec3 cross(const Vec3& o) const {
        return Vec3(
            y * o.z - z * o.y,
            z * o.x - x * o.z,
            x * o.y - y * o.x
        );
    }
};

struct GridPos {
    Face face;
    int u, v; // 0 .. GRID_N - 1

    bool operator==(const GridPos& o) const {
        return face == o.face && u == o.u && v == o.v;
    }
    bool operator!=(const GridPos& o) const {
        return !(*this == o);
    }
};

struct FaceBasis {
    Vec3 origin;  // World pos of cell (0, 0) center or corner
    Vec3 uAxis;   // Unit vector for +u
    Vec3 vAxis;   // Unit vector for +v
    Vec3 normal;  // Outward unit normal
};

// Returns FaceBasis for given face
const FaceBasis& getFaceBasis(Face f);

// Returns world coordinates for center of cell (u, v) on face
Vec3 localToWorld(Face face, int u, int v, float normalOffset = 0.0f);

// Returns world coordinates for corner of cell (u, v) on face:
// cornerU, cornerV in [0, 1] relative to the cell
Vec3 cellCornerToWorld(Face face, int u, int v, float cornerU, float cornerV, float normalOffset = 0.0f);

// Given current position and step direction, returns the new position and new direction.
// Handles crossing boundaries between any adjacent faces.
void stepPosition(const GridPos& current, Dir dir, GridPos& outNext, Dir& outNextDir);

// Helper to check if two directions are opposite
bool isOpposite(Dir a, Dir b);

// Helper string names for debugging
const char* faceToString(Face f);
const char* dirToString(Dir d);

#endif // CUBE_TOPOLOGY_H
