#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>

#define PI 3.14159265358979323846


struct Vec2 {
    float x;
    float y;
};

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Vec4 {
    // float x;
    // float y;
    // float z;
    // float w;

    union {
        struct { float x, y, z, w; };  // Generic names
        struct { float r, g, b, a; };  // Color-specific names
    };

};

// Define your C struct for mat4 and UniformBufferObject
struct mat4 {
    float elements[16]; // 1D array for column-major layout
};

struct Vec2 vec2(float x, float y);
struct Vec3 vec3(float x, float y, float z);
struct Vec4 vec4(float x, float y, float z, float w);

// Core matrix functions
struct mat4 createMat4(float n);
struct mat4 mat4Identity();
struct mat4 multiplyMat4(struct mat4 a, struct mat4 b);
struct mat4 transposeMat4(struct mat4 m);
struct Vec3 mat4_multiply_vec3(struct mat4 m, struct Vec3 v);
struct Vec3 mat4_multiply_vec3_normal(struct mat4 m, struct Vec3 v);

float degreesToRadians(float degrees);

float radiansToDegrees(float radians);

// Transformation functions
struct mat4 applyTranslation(struct mat4 inputMatrix, struct Vec3 v);
struct mat4 applyRotation(struct mat4 inputMatrix, float angle, struct Vec3 axis);
struct mat4 applyScaling(struct mat4 inputMatrix, struct Vec3 v);
struct mat4 invertYAxis(struct mat4 matrix);

// Projection functions
struct mat4 createLookAtMatrix(struct Vec3 eye, struct Vec3 center, struct Vec3 up);
struct mat4 createPerspectiveMatrix(float fov, float aspectRatio, float near, float far);
struct mat4 createOrthographicMatrix(float left, float right, float bottom, float top, float near, float far);

// Utility functions
// void translate(GraphicsObject* object, float x, float y, float z);
// void rotate(GraphicsObject* object, float angle, float x, float y, float z);

// Pure math transformation functions (no GraphicsObject dependency)
struct mat4 translateMatrix(struct mat4 inputMatrix, float x, float y, float z);
struct mat4 rotateMatrix(struct mat4 inputMatrix, float angle, float x, float y, float z);
struct mat4 scaleMatrix(struct mat4 inputMatrix, float x, float y, float z);


#endif