#include "matrix.h"


//-----------------------------------------------------------------------------------------------------
//                                          Matrix
//-----------------------------------------------------------------------------------------------------

float degreesToRadians(float degrees) {
    return degrees * (PI / 180.0f);
}

float radiansToDegrees(float radians) {
    return radians * (180.0f / PI);
}

struct Vec2 vec2(float x, float y) {
    struct Vec2 v = { x, y };
    return v;
}

struct Vec2 addVec2(struct Vec2 a, struct Vec2 b) {
    return vec2(a.x + b.x, a.y + b.y);
}

struct Vec2 subtractVec2(struct Vec2 a, struct Vec2 b) {
    return vec2(a.x - b.x, a.y - b.y);
}

float dotVec2(struct Vec2 a, struct Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

float magnitudeVec2(struct Vec2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

struct Vec2 normalizeVec2(struct Vec2 v) {
    float mag = magnitudeVec2(v);
    if (mag != 0.0f) {
        v.x /= mag;
        v.y /= mag;
    }
    return v;
}

struct Vec3 vec3(float x, float y, float z) {
    struct Vec3 v = { x, y, z };
    return v;
}

struct Vec3 addVec3(struct Vec3 a, struct Vec3 b) {
    return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

struct Vec3 subtractVec3(struct Vec3 a, struct Vec3 b) {
    return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

float magnitudeVec3(struct Vec3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

struct Vec3 normalizeVec3(struct Vec3 v) {
    float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length != 0) {
        v.x /= length;
        v.y /= length;
        v.z /= length;
    }
    return v;
}

struct Vec3 crossVec3(struct Vec3 a, struct Vec3 b) {
    struct Vec3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

float dotVec3(struct Vec3 a, struct Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

struct Vec4 vec4(float x, float y, float z, float w) {
    struct Vec4 v = { x, y, z, w };
    return v;
}

struct Vec4 addVec4(struct Vec4 a, struct Vec4 b) {
    return vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

struct Vec4 subtractVec4(struct Vec4 a, struct Vec4 b) {
    return vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

float dotVec4(struct Vec4 a, struct Vec4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float magnitudeVec4(struct Vec4 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

struct Vec4 normalizeVec4(struct Vec4 v) {
    float mag = magnitudeVec4(v);
    if (mag != 0.0f) {
        v.x /= mag;
        v.y /= mag;
        v.z /= mag;
        v.w /= mag;
    }
    return v;
}

struct mat4 createMat4(float n) {
//struct mat4 createMat4(float n) {
    struct mat4 result;
    for (int i = 0; i < 16; i++) {
        result.elements[i] = 0.0f;
    }

    // Set diagonal elements to 'n' (identity matrix when n = 1)
    result.elements[0] = n;
    result.elements[5] = n;
    result.elements[10] = n;
    result.elements[15] = n;

    return result;
}

struct mat4 mat4Identity() {
    return createMat4(1.0f);
}


// Now define multiplyMat4
struct mat4 multiplyMat4(struct mat4 a, struct mat4 b) {
    struct mat4 result;
    
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.elements[row * 4 + col] = 
                a.elements[row * 4 + 0] * b.elements[0 * 4 + col] +
                a.elements[row * 4 + 1] * b.elements[1 * 4 + col] +
                a.elements[row * 4 + 2] * b.elements[2 * 4 + col] +
                a.elements[row * 4 + 3] * b.elements[3 * 4 + col];
        }
    }
    return result;
}

struct mat4 transposeMat4(struct mat4 m) {
    struct mat4 result;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.elements[row * 4 + col] = m.elements[col * 4 + row];
        }
    }
    return result;
}

struct Vec3 mat4_multiply_vec3(struct mat4 m, struct Vec3 v) {
    struct Vec3 result;
    result.x = m.elements[0] * v.x + m.elements[4] * v.y + m.elements[8]  * v.z + m.elements[12];
    result.y = m.elements[1] * v.x + m.elements[5] * v.y + m.elements[9]  * v.z + m.elements[13];
    result.z = m.elements[2] * v.x + m.elements[6] * v.y + m.elements[10] * v.z + m.elements[14];
    return result;
}

struct Vec3 mat4_multiply_vec3_normal(struct mat4 m, struct Vec3 v) {
    // No translation — normals are direction vectors only
    struct Vec3 result;
    result.x = m.elements[0] * v.x + m.elements[4] * v.y + m.elements[8]  * v.z;
    result.y = m.elements[1] * v.x + m.elements[5] * v.y + m.elements[9]  * v.z;
    result.z = m.elements[2] * v.x + m.elements[6] * v.y + m.elements[10] * v.z;
    return result;
}

struct mat4 applyTranslation(struct mat4 inputMatrix, struct Vec3 v) {
    struct mat4 translationMatrix = createMat4(1.0f); // Identity matrix
    
    // Set translation values
    translationMatrix.elements[12] = v.x;
    translationMatrix.elements[13] = v.y;
    translationMatrix.elements[14] = v.z;

    // Multiply inputMatrix by translationMatrix
    return multiplyMat4(inputMatrix, translationMatrix);
}

struct mat4 applyRotation(struct mat4 inputMatrix, float angle, struct Vec3 axis) {
    struct mat4 rotationMatrix = createMat4(1.0f);  // Identity matrix

    float radians = angle;// * (PI / 180.0f);  // Convert degrees to radians
    float c = cosf(radians);
    float s = sinf(radians);
    float oneMinusC = 1.0f - c;

    // Normalize axis (in case it's not already a unit vector)
    float length = sqrtf(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
    if (length > 0.0f) {
        axis.x /= length;
        axis.y /= length;
        axis.z /= length;
    }

    float x = axis.x, y = axis.y, z = axis.z;

    // Rotation matrix (column-major order for Vulkan)
    rotationMatrix.elements[0] = c + x * x * oneMinusC;
    rotationMatrix.elements[1] = y * x * oneMinusC + z * s;
    rotationMatrix.elements[2] = z * x * oneMinusC - y * s;
    rotationMatrix.elements[3] = 0.0f;

    rotationMatrix.elements[4] = x * y * oneMinusC - z * s;
    rotationMatrix.elements[5] = c + y * y * oneMinusC;
    rotationMatrix.elements[6] = z * y * oneMinusC + x * s;
    rotationMatrix.elements[7] = 0.0f;

    rotationMatrix.elements[8] = x * z * oneMinusC + y * s;
    rotationMatrix.elements[9] = y * z * oneMinusC - x * s;
    rotationMatrix.elements[10] = c + z * z * oneMinusC;
    rotationMatrix.elements[11] = 0.0f;

    rotationMatrix.elements[12] = 0.0f;
    rotationMatrix.elements[13] = 0.0f;
    rotationMatrix.elements[14] = 0.0f;
    rotationMatrix.elements[15] = 1.0f;

    // Multiply rotation matrix by input matrix
    return multiplyMat4(inputMatrix, rotationMatrix);
}

struct mat4 applyScaling(struct mat4 inputMatrix, struct Vec3 v) {
    struct mat4 scalingMatrix = createMat4(1.0f); // Identity matrix

    // Set scaling values
    scalingMatrix.elements[0] = v.x;  // Scale X
    scalingMatrix.elements[5] = v.y;  // Scale Y
    scalingMatrix.elements[10] = v.z; // Scale Z

    // Multiply inputMatrix by scalingMatrix
    //return multiplyMat4(inputMatrix, scalingMatrix);

        return multiplyMat4(scalingMatrix, inputMatrix);  // Apply scaling first

}

struct mat4 createLookAtMatrix(struct Vec3 eye, struct Vec3 center, struct Vec3 up) {
    struct Vec3 f, s, u; // Forward, Side, and Up vectors
    
    // Compute forward vector (normalize center - eye)
    f.x = center.x - eye.x;
    f.y = center.y - eye.y;
    f.z = center.z - eye.z;
    f = normalizeVec3(f);  

    // Compute right vector (cross product of forward and up, then normalize)
    s = crossVec3(f, up);
    s = normalizeVec3(s);

    // Compute new up vector (cross product of right and forward)
    u = crossVec3(s, f);

    struct mat4 result = createMat4(1.0f); // Identity matrix

    // Set rotation part
    result.elements[0] = s.x;
    result.elements[1] = u.x;
    result.elements[2] = -f.x;
    
    result.elements[4] = s.y;
    result.elements[5] = u.y;
    result.elements[6] = -f.y;
    
    result.elements[8] = s.z;
    result.elements[9] = u.z;
    result.elements[10] = -f.z;

    // Set translation part
    result.elements[12] = -dotVec3(s, eye);
    result.elements[13] = -dotVec3(u, eye);
    result.elements[14] = dotVec3(f, eye);

    return result;
}

struct mat4 createPerspectiveMatrix(float fov, float aspectRatio, float near, float far) {
    struct mat4 result;
    
    // Clear matrix
    for (int i = 0; i < 16; i++) {
        result.elements[i] = 0.0f;
    }

    float tanHalfFov = tanf(fov / 2.0f);

    result.elements[0] = 1.0f / (aspectRatio * tanHalfFov);
    result.elements[5] = 1.0f / tanHalfFov;
    result.elements[10] = -(far + near) / (far - near);
    result.elements[11] = -1.0f;
    result.elements[14] = -(2.0f * far * near) / (far - near);

    return result;
}

struct mat4 createOrthographicMatrix(float left, float right, float bottom, float top, float near, float far) {
    struct mat4 result;

    // Clear matrix
    for (int i = 0; i < 16; i++) {
        result.elements[i] = 0.0f;
    }

    /*// Set orthographic projection values
    result.elements[0] = 2.0f / (right - left);
    result.elements[5] = 2.0f / (top - bottom);
    result.elements[10] = -2.0f / (far - near);
    result.elements[12] = -(right + left) / (right - left);
    result.elements[13] = -(top + bottom) / (top - bottom);
    result.elements[14] = -(far + near) / (far - near);
    result.elements[15] = 1.0f;

    */

    // Orthographic projection formula
    result.elements[0] = 2.0f / (right - left);
    result.elements[5] = 2.0f / (top - bottom);
    result.elements[10] = 1.0f / (near - far);  // Vulkan: [0, 1] depth range
    result.elements[12] = -(right + left) / (right - left);
    result.elements[13] = -(top + bottom) / (top - bottom);
    result.elements[14] = near / (near - far);
    result.elements[15] = 1.0f;

    return result;
}

struct mat4 invertYAxis(struct mat4 matrix) {
    // Invert the Y-axis for Vulkan (flip the Y values)
    matrix.elements[5] *= -1.0f;  // Flipping the Y-axis element (row 1, column 1)

    return matrix;
}