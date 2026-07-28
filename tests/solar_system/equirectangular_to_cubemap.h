// equirectangular_to_cubemap.h
// Converts a single equirectangular (2:1 lat-long) RGBA image into 6 cubemap faces.
// Face order: +X, -X, +Y, -Y, +Z, -Z  (matches Vulkan VkCubeMapFace order)
// Each output face is faceSize x faceSize RGBA (4 bytes per pixel).
// Caller is responsible for freeing each faces[i].

#ifndef EQUIRECTANGULAR_TO_CUBEMAP_H
#define EQUIRECTANGULAR_TO_CUBEMAP_H

#include <stdlib.h>
#include <math.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

// Given a cube face index and pixel (u,v) in [0,1], returns the 3D direction vector.
static void cube_uv_to_dir(int face, float u, float v,
                            float* dx, float* dy, float* dz)
{
    // Remap [0,1] -> [-1,1]
    float uc = 2.0f * u - 1.0f;
    float vc = 2.0f * v - 1.0f;

    switch (face)
    {
        case 0: *dx =  1.0f; *dy =  vc;   *dz = -uc;   break; // +X
        case 1: *dx = -1.0f; *dy =  vc;   *dz =  uc;   break; // -X
        case 2: *dx =  uc;   *dy =  1.0f; *dz = -vc;   break; // +Y
        case 3: *dx =  uc;   *dy = -1.0f; *dz =  vc;   break; // -Y
        case 4: *dx =  uc;   *dy =  vc;   *dz =  1.0f; break; // +Z
        case 5: *dx = -uc;   *dy =  vc;   *dz = -1.0f; break; // -Z
    }
}

static void equirectangular_to_cubemap(const unsigned char* src,
                                       int srcW, int srcH,
                                       int faceSize,
                                       unsigned char* faces[6])
{
    for (int face = 0; face < 6; face++)
    {
        faces[face] = (unsigned char*)malloc(faceSize * faceSize * 4);

        for (int py = 0; py < faceSize; py++)
        {
            for (int px = 0; px < faceSize; px++)
            {
                float u = (px + 0.5f) / faceSize;
                float v = (py + 0.5f) / faceSize;

                float dx, dy, dz;
                cube_uv_to_dir(face, u, v, &dx, &dy, &dz);

                // Normalise direction
                float len = sqrtf(dx*dx + dy*dy + dz*dz);
                dx /= len; dy /= len; dz /= len;

                // Direction -> spherical coordinates
                float phi   = atan2f(dz, dx);           // [-PI,  PI]
                float theta = acosf(dy);                 // [0,    PI]

                // Spherical -> equirectangular UV
                float eu = (phi   + (float)PI) / (2.0f * (float)PI);
                float ev = theta  / (float)PI;

                // Bilinear sample from source
                float sx = eu * (srcW - 1);
                float sy = ev * (srcH - 1);

                int x0 = (int)sx;
                int y0 = (int)sy;
                int x1 = x0 + 1; if (x1 >= srcW) x1 = srcW - 1;
                int y1 = y0 + 1; if (y1 >= srcH) y1 = srcH - 1;

                float fx = sx - x0;
                float fy = sy - y0;

                for (int c = 0; c < 4; c++)
                {
                    float p00 = src[(y0 * srcW + x0) * 4 + c];
                    float p10 = src[(y0 * srcW + x1) * 4 + c];
                    float p01 = src[(y1 * srcW + x0) * 4 + c];
                    float p11 = src[(y1 * srcW + x1) * 4 + c];

                    float val = p00 * (1-fx)*(1-fy)
                              + p10 *    fx *(1-fy)
                              + p01 * (1-fx)*   fy
                              + p11 *    fx *    fy;

                    faces[face][(py * faceSize + px) * 4 + c] = (unsigned char)val;
                }
            }
        }
    }
}

#endif // EQUIRECTANGULAR_TO_CUBEMAP_H
