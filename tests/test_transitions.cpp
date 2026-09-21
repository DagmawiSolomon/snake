#include <iostream>
#include <cassert>
#include <vector>
#include <cstdlib>
#include "../src/cube_topology.h"

// Helper to get opposite direction
Dir getOpposite(Dir d) {
    switch (d) {
        case Dir::UP: return Dir::DOWN;
        case Dir::DOWN: return Dir::UP;
        case Dir::LEFT: return Dir::RIGHT;
        case Dir::RIGHT: return Dir::LEFT;
    }
    return Dir::UP;
}

void testReversibility() {
    std::cout << "[Test 1] Testing Reversibility across all boundary edges...\n";
    int tested = 0;
    for (int f = 0; f < 6; f++) {
        Face face = static_cast<Face>(f);
        // Test all 4 edges
        for (int coord = 0; coord < GRID_N; coord++) {
            // Test stepping UP from top edge
            {
                GridPos start = { face, coord, GRID_N - 1 };
                GridPos next;
                Dir nextDir;
                stepPosition(start, Dir::UP, next, nextDir);

                // Step backward from next in opposite of nextDir
                GridPos back;
                Dir backDir;
                stepPosition(next, getOpposite(nextDir), back, backDir);

                assert(back == start);
                assert(backDir == Dir::DOWN);
                tested++;
            }
            // Test stepping DOWN from bottom edge
            {
                GridPos start = { face, coord, 0 };
                GridPos next;
                Dir nextDir;
                stepPosition(start, Dir::DOWN, next, nextDir);

                GridPos back;
                Dir backDir;
                stepPosition(next, getOpposite(nextDir), back, backDir);

                assert(back == start);
                assert(backDir == Dir::UP);
                tested++;
            }
            // Test stepping LEFT from left edge
            {
                GridPos start = { face, 0, coord };
                GridPos next;
                Dir nextDir;
                stepPosition(start, Dir::LEFT, next, nextDir);

                GridPos back;
                Dir backDir;
                stepPosition(next, getOpposite(nextDir), back, backDir);

                assert(back == start);
                assert(backDir == Dir::RIGHT);
                tested++;
            }
            // Test stepping RIGHT from right edge
            {
                GridPos start = { face, GRID_N - 1, coord };
                GridPos next;
                Dir nextDir;
                stepPosition(start, Dir::RIGHT, next, nextDir);

                GridPos back;
                Dir backDir;
                stepPosition(next, getOpposite(nextDir), back, backDir);

                assert(back == start);
                assert(backDir == Dir::LEFT);
                tested++;
            }
        }
    }
    std::cout << "  Passed! Tested " << tested << " boundary transitions with 100% reversibility.\n";
}

void testGreatCircles() {
    std::cout << "[Test 2] Testing Great Circle Closed Loops (walking straight tours 4 faces)...\n";
    // Walking straight in any direction along any track on the cube should return to the start
    // in exactly 4 * GRID_N steps.
    const int loopLength = 4 * GRID_N;

    for (int f = 0; f < 6; f++) {
        Face startFace = static_cast<Face>(f);
        for (int d = 0; d < 4; d++) {
            Dir startDir = static_cast<Dir>(d);
            for (int u = 0; u < GRID_N; u += 3) {
                for (int v = 0; v < GRID_N; v += 3) {
                    GridPos cur = { startFace, u, v };
                    Dir curDir = startDir;

                    for (int step = 0; step < loopLength; step++) {
                        GridPos next;
                        Dir nextDir;
                        stepPosition(cur, curDir, next, nextDir);
                        cur = next;
                        curDir = nextDir;
                    }

                    // After exactly 4 * GRID_N steps, must be back at original cell and direction!
                    assert(cur.face == startFace);
                    assert(cur.u == u);
                    assert(cur.v == v);
                    assert(curDir == startDir);
                }
            }
        }
    }
    std::cout << "  Passed! All straight-line paths form perfect closed 4-face geodesic loops.\n";
}

void testSpatialContinuity() {
    std::cout << "[Test 3] Testing 3D Spatial Adjacency across boundaries...\n";
    // When transitioning from cell A to cell B across an edge, their 3D world positions
    // should be approximately CELL_SIZE distance apart (at most ~1.42 * CELL_SIZE for corner bevel).
    for (int f = 0; f < 6; f++) {
        Face face = static_cast<Face>(f);
        for (int d = 0; d < 4; d++) {
            Dir dir = static_cast<Dir>(d);
            for (int u = 0; u < GRID_N; u++) {
                for (int v = 0; v < GRID_N; v++) {
                    GridPos start = { face, u, v };
                    GridPos next;
                    Dir nextDir;
                    stepPosition(start, dir, next, nextDir);

                    Vec3 posA = localToWorld(start.face, start.u, start.v);
                    Vec3 posB = localToWorld(next.face, next.u, next.v);

                    float dx = posA.x - posB.x;
                    float dy = posA.y - posB.y;
                    float dz = posA.z - posB.z;
                    float dist = std::sqrt(dx*dx + dy*dy + dz*dz);

                    // Adjacent cell centers should never be farther than ~1.5 * CELL_SIZE
                    // (and definitely not jumping across the cube diameter CUBE_SIZE = 6.0)
                    if (dist > CELL_SIZE * 1.6f) {
                        std::cerr << "Distance anomaly: " << dist << " between "
                                  << faceToString(start.face) << "(" << start.u << "," << start.v << ") and "
                                  << faceToString(next.face) << "(" << next.u << "," << next.v << ")\n";
                        assert(false);
                    }
                }
            }
        }
    }
    std::cout << "  Passed! All transitions preserve physical 3D distance continuity.\n";
}

void testRandomWalkSimulation() {
    std::cout << "[Test 4] Testing 100,000 steps of random walk without boundary violation...\n";
    GridPos cur = { Face::PZ, 5, 5 };
    Dir curDir = Dir::UP;
    srand(42);

    for (int step = 0; step < 100000; step++) {
        // Occasionally turn left or right
        int r = rand() % 5;
        if (r == 0) {
            // Turn 90 deg relative to curDir
            if (curDir == Dir::UP || curDir == Dir::DOWN) curDir = Dir::LEFT;
            else curDir = Dir::UP;
        } else if (r == 1) {
            if (curDir == Dir::UP || curDir == Dir::DOWN) curDir = Dir::RIGHT;
            else curDir = Dir::DOWN;
        }

        GridPos next;
        Dir nextDir;
        stepPosition(cur, curDir, next, nextDir);

        // Sanity assertions
        assert(static_cast<int>(next.face) >= 0 && static_cast<int>(next.face) < 6);
        assert(next.u >= 0 && next.u < GRID_N);
        assert(next.v >= 0 && next.v < GRID_N);

        cur = next;
        curDir = nextDir;
    }
    std::cout << "  Passed! 100,000 random walk steps executed with zero errors.\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "   RUNNING CUBE TOPOLOGY UNIT TESTS     \n";
    std::cout << "========================================\n";

    testReversibility();
    testGreatCircles();
    testSpatialContinuity();
    testRandomWalkSimulation();

    std::cout << "\n>>> ALL TOPOLOGY & TRANSITION TESTS PASSED SUCCESSFULLY! <<<\n\n";
    return 0;
}
