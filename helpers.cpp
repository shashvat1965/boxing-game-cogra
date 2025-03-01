#include "helpers.hpp"
#include <cmath>

void customRotatef(float angle, float axisX, float axisY, float axisZ) {
    // Convert degrees to radians
    float rad = angle * M_PI / 180.0f;
    float c = cos(rad);
    float s = sin(rad);
    float one_c = 1 - c;

    // Normalize the axis
    float length = sqrt(axisX * axisX + axisY * axisY + axisZ * axisZ);
    if (length < 0.0001f) return;  // Avoid division by zero
    float x = axisX / length;
    float y = axisY / length;
    float z = axisZ / length;

    // Build rotation matrix (column-major order for OpenGL)
    float m[16] = {
        x*x*one_c + c,     y*x*one_c + z*s,  z*x*one_c - y*s,  0.0f,
        x*y*one_c - z*s,   y*y*one_c + c,    z*y*one_c + x*s,  0.0f,
        x*z*one_c + y*s,   y*z*one_c - x*s,  z*z*one_c + c,    0.0f,
        0.0f,              0.0f,             0.0f,             1.0f
    };

    // Apply the rotation
    glMultMatrixf(m);
}

void customTranslatef(float x, float y, float z) {
    // Create translation matrix in column-major order for OpenGL
    float m[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        x,    y,    z,    1.0f
    };
    
    // Apply the translation
    glMultMatrixf(m);
}