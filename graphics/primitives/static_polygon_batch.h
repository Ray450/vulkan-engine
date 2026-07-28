#ifndef STATIC_POLYGON_BATCH_H
#define STATIC_POLYGON_BATCH_H


#include "../vulkan/vulkan_graphics.h"
#include "dynamic_fan_batch.h"

#define PRIMITIVE_VERTEX_COUNT 44
#define PRIMITIVE_INDEX_COUNT 96
#define PRIMITIVE_TRIANGLE   0
#define PRIMITIVE_RECTANGLE  1
#define PRIMITIVE_PENTAGON   2
#define PRIMITIVE_HEXAGON    3
#define PRIMITIVE_HEPTAGON   4
#define PRIMITIVE_OCTAGON    5
#define PRIMITIVE_CIRCLE     6
#define PRIMITIVE_COUNT      7

#define PRIMITIVE_BATCH_MAX_INSTANCES 100

// Inside static_polygon_batch.h
typedef struct {
    uint32_t firstIndex;   // Offset into the shared index buffer
    uint32_t indexCount;   // Number of indices for this specific appended mesh
} DrawRange;

static const DrawRange primitiveRanges[PRIMITIVE_COUNT] = {
    {0,  3},   // Triangle
    {3,  6},   // Rectangle
    {9,  9},   // Pentagon
    {18, 12},  // Hexagon
    {30, 15},  // Heptagon
    {45, 18},  // Octagon
    {63, 33},  // Circle (11-sided approximation)
};

static Vertex primitiveVertices[PRIMITIVE_VERTEX_COUNT] = {
    // Triangle (green) at (0.0, 0.0)
    {{ 0.0f,  0.2f, 0.0f}, {0,1,0,1}, {0.5f,1},  {0,0,1}},
    {{-0.1f,  0.0f, 0.0f}, {0,1,0,1}, {0,0},      {0,0,1}},
    {{ 0.1f,  0.0f, 0.0f}, {0,1,0,1}, {1,0},      {0,0,1}},

    // Rectangle (red) at (-0.6, 0.0)
    {{-0.6f,  0.0f, 0.0f}, {1,0,0,1}, {0,0},      {0,0,1}},
    {{-0.4f,  0.0f, 0.0f}, {1,0,0,1}, {1,0},      {0,0,1}},
    {{-0.4f,  0.2f, 0.0f}, {1,0,0,1}, {1,1},      {0,0,1}},
    {{-0.6f,  0.2f, 0.0f}, {1,0,0,1}, {0,1},      {0,0,1}},

    // Pentagon (blue) at (0.5, 0.0)
    {{ 0.5f,  0.2f, 0.0f}, {0,0,1,1}, {0.5f,1},   {0,0,1}},
    {{ 0.4f,  0.1f, 0.0f}, {0,0,1,1}, {0,0.5f},   {0,0,1}},
    {{ 0.45f, 0.0f, 0.0f}, {0,0,1,1}, {0.2f,0},   {0,0,1}},
    {{ 0.55f, 0.0f, 0.0f}, {0,0,1,1}, {0.8f,0},   {0,0,1}},
    {{ 0.6f,  0.1f, 0.0f}, {0,0,1,1}, {1,0.5f},   {0,0,1}},

    // Hexagon (cyan) at (-0.2, -0.4)
    {{-0.2f, -0.2f, 0.0f}, {0,1,1,1}, {0.5f,1},   {0,0,1}},
    {{-0.3f, -0.3f, 0.0f}, {0,1,1,1}, {0,0.75f},  {0,0,1}},
    {{-0.3f, -0.5f, 0.0f}, {0,1,1,1}, {0,0.25f},  {0,0,1}},
    {{-0.2f, -0.6f, 0.0f}, {0,1,1,1}, {0.5f,0},   {0,0,1}},
    {{-0.1f, -0.5f, 0.0f}, {0,1,1,1}, {1,0.25f},  {0,0,1}},
    {{-0.1f, -0.3f, 0.0f}, {0,1,1,1}, {1,0.75f},  {0,0,1}},

    // Heptagon (magenta) at (0.3, -0.4)
    {{ 0.3f,  -0.2f,  0.0f}, {1,0,1,1}, {0.5f,1},     {0,0,1}},
    {{ 0.18f, -0.28f, 0.0f}, {1,0,1,1}, {0.1f,0.75f}, {0,0,1}},
    {{ 0.16f, -0.45f, 0.0f}, {1,0,1,1}, {0.1f,0.25f}, {0,0,1}},
    {{ 0.26f, -0.58f, 0.0f}, {1,0,1,1}, {0.35f,0},    {0,0,1}},
    {{ 0.38f, -0.58f, 0.0f}, {1,0,1,1}, {0.65f,0},    {0,0,1}},
    {{ 0.46f, -0.45f, 0.0f}, {1,0,1,1}, {0.9f,0.25f}, {0,0,1}},
    {{ 0.42f, -0.28f, 0.0f}, {1,0,1,1}, {0.9f,0.75f}, {0,0,1}},

    // Octagon (orange) at (-0.5, -0.4)
    {{-0.5f,  -0.2f,  0.0f}, {1,0.5f,0,1}, {0.5f,1},      {0,0,1}},
    {{-0.6f,  -0.28f, 0.0f}, {1,0.5f,0,1}, {0.15f,0.85f}, {0,0,1}},
    {{-0.65f, -0.4f,  0.0f}, {1,0.5f,0,1}, {0,0.5f},      {0,0,1}},
    {{-0.6f,  -0.52f, 0.0f}, {1,0.5f,0,1}, {0.15f,0.15f}, {0,0,1}},
    {{-0.5f,  -0.6f,  0.0f}, {1,0.5f,0,1}, {0.5f,0},      {0,0,1}},
    {{-0.4f,  -0.52f, 0.0f}, {1,0.5f,0,1}, {0.85f,0.15f}, {0,0,1}},
    {{-0.35f, -0.4f,  0.0f}, {1,0.5f,0,1}, {1,0.5f},      {0,0,1}},
    {{-0.4f,  -0.28f, 0.0f}, {1,0.5f,0,1}, {0.85f,0.85f}, {0,0,1}},

    // Circle (yellow) at (0.0, -0.4) — 11-sided approximation
    {{ 0.0f,  -0.4f,  0.0f}, {1,1,0,1}, {0.5f,0.5f},  {0,0,1}},  // center (index 33)
    {{ 0.0f,  -0.3f,  0.0f}, {1,1,0,1}, {0.5f,1},     {0,0,1}},
    {{ 0.05f, -0.32f, 0.0f}, {1,1,0,1}, {0.75f,0.9f}, {0,0,1}},
    {{ 0.09f, -0.37f, 0.0f}, {1,1,0,1}, {0.95f,0.65f},{0,0,1}},
    {{ 0.09f, -0.43f, 0.0f}, {1,1,0,1}, {0.95f,0.35f},{0,0,1}},
    {{ 0.05f, -0.48f, 0.0f}, {1,1,0,1}, {0.75f,0.1f}, {0,0,1}},
    {{ 0.0f,  -0.5f,  0.0f}, {1,1,0,1}, {0.5f,0},     {0,0,1}},
    {{-0.05f, -0.48f, 0.0f}, {1,1,0,1}, {0.25f,0.1f}, {0,0,1}},
    {{-0.09f, -0.43f, 0.0f}, {1,1,0,1}, {0.05f,0.35f},{0,0,1}},
    {{-0.09f, -0.37f, 0.0f}, {1,1,0,1}, {0.05f,0.65f},{0,0,1}},
    {{-0.05f, -0.32f, 0.0f}, {1,1,0,1}, {0.25f,0.9f}, {0,0,1}},
};

//-----------------------------------------------------------------------------------------------------
//  Shared Index Data
//-----------------------------------------------------------------------------------------------------


static uint16_t primitiveIndices[PRIMITIVE_INDEX_COUNT] = {
    // Triangle (firstIndex=0, count=3)
    0,1,2,

    // Rectangle (firstIndex=3, count=6)
    3,4,5,
    3,5,6,

    // Pentagon fan from vertex 7 (firstIndex=9, count=9)
    7,8,9,
    7,9,10,
    7,10,11,

    // Hexagon fan from vertex 12 (firstIndex=18, count=12)
    12,13,14,
    12,14,15,
    12,15,16,
    12,16,17,

    // Heptagon fan from vertex 18 (firstIndex=30, count=15)
    18,19,20,
    18,20,21,
    18,21,22,
    18,22,23,
    18,23,24,

    // Octagon fan from vertex 25 (firstIndex=45, count=18)
    25,26,27,
    25,27,28,
    25,28,29,
    25,29,30,
    25,30,31,
    25,31,32,

    // Circle fan from center vertex 33 (firstIndex=63, count=33)
    33,34,35,
    33,35,36,
    33,36,37,
    33,37,38,
    33,38,39,
    33,39,40,
    33,40,41,
    33,41,42,
    33,42,43,
    33,43,44,
    33,44,34,   // wrap-around closes the fan
};


GraphicsObject create_static_polygon_batch(VulkanContext* context, Pipeline* pipeline, Texture texture, Vertex* vertices, uint16_t* indices, size_t vertexCount, size_t indexCount);
void destroy_static_polygon_batch(GraphicsObject* obj);
/*void static_batch_append(GraphicsObject* obj,
                         Vertex* vertices, int vertexCount,
                         uint16_t* indices, int indexCount,
                         struct mat4 worldMatrix, uint32_t textureId);*/

DrawRange static_batch_append(GraphicsObject* obj, Vertex* vertices, int vertexCount, uint16_t* indices, int indexCount, struct mat4 worldMatrix, uint32_t textureId);

void static_batch_init_atlas(AtlasRegion* regions, int count);
void static_batch_upload(GraphicsObject* obj);
void static_batch_draw(GraphicsObject* obj, uint32_t instanceCount);

#endif // STATIC_POLYGON_BATCH_H