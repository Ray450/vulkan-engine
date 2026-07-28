#ifndef SHAPE_BATCH_H
#define SHAPE_BATCH_H


#include "../vulkan/vulkan_graphics.h"

#define MAX_ATLAS_TEXTURES 64
typedef struct {
    float u0, v0;
    float u1, v1;
} AtlasRegion;

extern AtlasRegion atlas[MAX_ATLAS_TEXTURES];

// #define MAX_SHAPE_VERTS   10000
// #define MAX_SHAPE_INDICES 10000
// #define MAX_SEG_VERTS     1000
// #define MAX_SEG_INDICES   1000

#define MAX_SHAPE_VERTS   65536
#define MAX_SHAPE_INDICES 65536
#define MAX_SEG_VERTS     10000
#define MAX_SEG_INDICES   10000

extern int batchVertexCount;
extern int batchIndexCount;
extern int segVertexCount;
extern int segIndexCount;

GraphicsObject dynamic_batch_object_create(VulkanContext* context, Pipeline* pipeline, Texture texture, Vertex* vertices, uint16_t* indices, size_t vertexCount, size_t indexCount);
unsigned char* dynamic_batch_build_atlas(PixelBuffer* pixelArray, int count, int* outWidth, int* outHeight, AtlasRegion* atlas);
void dynamic_batch_render(GraphicsObject* obj, Vec2 xOffset, unsigned int color, unsigned int stroke,
                 Vec2* axVertices, int iNumVertices, uint32_t textureId);
void dynamic_batch_render_shape_3d(GraphicsObject* obj, struct Vec3 xOffset, unsigned int color,
                   struct Vec3* axVertices, int iNumVertices, uint32_t textureId);
void dynamic_batch_render_cube(GraphicsObject* obj, struct Vec3 pos, float size, uint32_t textureId);
void dynamic_batch_render_mesh(GraphicsObject* obj,
                struct Vec3 offset,
                unsigned int color,
                const Vertex* srcVertices,    // from loadModel
                int srcVertexCount,
                const uint16_t* srcIndices,   // from loadModel
                int srcIndexCount,
                uint32_t textureId);
void dynamic_batch_flush(GraphicsObject* obj, uint32_t instanceCount);

#endif // SHAPE_BATCH_H