#ifndef CAMERA_H
#define CAMERA_H

#include "cube_topology.h"

class OrbitCamera {
public:
    OrbitCamera();

    void setTargetFace(Face face);
    void update(float deltaTime); // called every render frame (e.g. 60 FPS)

    void applyView() const;

    // Optional manual orbit adjustment (for menu / exploration)
    void addManualRotation(float deltaPitch, float deltaYaw);

private:
    void computeTargetVectors(Face face, Vec3& outEye, Vec3& outUp);

    Vec3 m_curEye;
    Vec3 m_curUp;
    Vec3 m_targetEye;
    Vec3 m_targetUp;
    float m_distance;
    float m_lerpSpeed;

    Face m_activeFace;
};

#endif // CAMERA_H
