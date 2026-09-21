#include "camera.h"
#include <GL/freeglut.h>
#include <cmath>

static Vec3 normalizeVec(const Vec3& v) {
    float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len < 1e-6f) return Vec3(0, 0, 1);
    return Vec3(v.x / len, v.y / len, v.z / len);
}

static Vec3 crossVec(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

OrbitCamera::OrbitCamera()
    : m_distance(13.5f),
      m_lerpSpeed(7.0f),
      m_activeFace(Face::PZ)
{
    computeTargetVectors(Face::PZ, m_targetEye, m_targetUp);
    m_curEye = m_targetEye;
    m_curUp = m_targetUp;
}

void OrbitCamera::computeTargetVectors(Face face, Vec3& outEye, Vec3& outUp) {
    const FaceBasis& basis = getFaceBasis(face);

    // Slightly elevate camera along face's vAxis (up) and normal
    // so adjacent sides of the 3D cube are visible for depth perception
    Vec3 eyeDir = basis.normal * 1.0f + basis.vAxis * 0.28f + basis.uAxis * 0.15f;
    eyeDir = normalizeVec(eyeDir);

    outEye = eyeDir * m_distance;
    outUp = basis.vAxis;
}

void OrbitCamera::setTargetFace(Face face) {
    m_activeFace = face;
    computeTargetVectors(face, m_targetEye, m_targetUp);
}

void OrbitCamera::update(float deltaTime) {
    // Frame-rate independent exponential smoothing
    float blend = 1.0f - std::exp(-m_lerpSpeed * deltaTime);
    if (blend > 1.0f) blend = 1.0f;

    m_curEye = m_curEye + (m_targetEye - m_curEye) * blend;
    m_curUp = m_curUp + (m_targetUp - m_curUp) * blend;

    // Maintain constant spherical radius
    m_curEye = normalizeVec(m_curEye) * m_distance;

    // Orthonormalize Up vector against Eye vector
    Vec3 lookDir = normalizeVec(Vec3(-m_curEye.x, -m_curEye.y, -m_curEye.z));
    Vec3 right = crossVec(lookDir, m_curUp);
    if (std::sqrt(right.x * right.x + right.y * right.y + right.z * right.z) > 1e-4f) {
        right = normalizeVec(right);
        m_curUp = normalizeVec(crossVec(right, lookDir));
    }
}

void OrbitCamera::applyView() const {
    gluLookAt(
        m_curEye.x, m_curEye.y, m_curEye.z, // Eye
        0.0f, 0.0f, 0.0f,                   // LookAt center of cube
        m_curUp.x, m_curUp.y, m_curUp.z     // Up
    );
}

void OrbitCamera::addManualRotation(float deltaPitch, float deltaYaw) {
    // Rotate eye slightly for menu turntable
    float cosY = std::cos(deltaYaw);
    float sinY = std::sin(deltaYaw);
    float newX = m_curEye.x * cosY - m_curEye.z * sinY;
    float newZ = m_curEye.x * sinY + m_curEye.z * cosY;
    m_curEye.x = newX;
    m_curEye.z = newZ;
    m_curEye = normalizeVec(m_curEye) * m_distance;
}
