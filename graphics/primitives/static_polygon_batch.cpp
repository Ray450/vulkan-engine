#include "static_polygon_batch.h"
#include "dynamic_fan_batch.h"

static int staticVertexCount = 0;
static int staticIndexCount  = 0;
static bool staticBatchReady = false;

AtlasRegion staticAtlas[MAX_ATLAS_TEXTURES];

GraphicsObject create_static_polygon_batch(VulkanContext* context, Pipeline* pipeline, Texture texture, Vertex* vertices, uint16_t* indices, size_t vertexCount, size_t indexCount) {
    return init_graphics_object(context, pipeline, texture, vertices, indices, vertexCount, indexCount);
}

void static_batch_init_atlas(AtlasRegion* regions, int count) {
    for (int i = 0; i < count && i < MAX_ATLAS_TEXTURES; i++)
        staticAtlas[i] = regions[i];
}

/*void static_batch_append(GraphicsObject* obj,
                         Vertex* vertices, int vertexCount,
                         uint16_t* indices, int indexCount,
                         struct mat4 worldMatrix,
                         uint32_t textureId)          // <-- add this
{
    if (staticBatchReady) return;
    if (staticVertexCount + vertexCount > MAX_SHAPE_VERTS) return;
    if (staticIndexCount  + indexCount  > MAX_SHAPE_INDICES) return;

    AtlasRegion reg = staticAtlas[textureId];   // <-- look up region

    int baseVertex = staticVertexCount;

    for (int i = 0; i < vertexCount; i++) {
        Vertex v = vertices[i];
        v.pos    = mat4_multiply_vec3(worldMatrix, v.pos);
        v.normal = mat4_multiply_vec3_normal(worldMatrix, v.normal);

        // Remap UVs into atlas region, same as dynamic_batch_render_mesh
        v.texCoord.x = reg.u0 + v.texCoord.x * (reg.u1 - reg.u0);
        v.texCoord.y = reg.v0 + v.texCoord.y * (reg.v1 - reg.v0);

        obj->vertices[staticVertexCount++] = v;
    }

    for (int i = 0; i < indexCount; i++) {
        obj->indices[staticIndexCount++] = (uint16_t)(baseVertex + indices[i]);
    }
}*/


DrawRange static_batch_append(GraphicsObject* obj, Vertex* vertices, int vertexCount,
                                    uint16_t* indices, int indexCount,
                                    struct mat4 worldMatrix, uint32_t textureId)
{
    if (staticBatchReady) return (DrawRange){0, 0};
    if (staticVertexCount + vertexCount > MAX_SHAPE_VERTS) return (DrawRange){0, 0};
    if (staticIndexCount  + indexCount  > MAX_SHAPE_INDICES) return (DrawRange){0, 0};

    AtlasRegion reg = staticAtlas[textureId];

    int baseVertex = staticVertexCount;
    int baseIndex  = staticIndexCount;   // <-- this shape's firstIndex, captured BEFORE appending

    for (int i = 0; i < vertexCount; i++) {
        Vertex v = vertices[i];
        v.pos    = mat4_multiply_vec3(worldMatrix, v.pos);
        v.normal = mat4_multiply_vec3_normal(worldMatrix, v.normal);

        v.texCoord.x = reg.u0 + v.texCoord.x * (reg.u1 - reg.u0);
        v.texCoord.y = reg.v0 + v.texCoord.y * (reg.v1 - reg.v0);

        obj->vertices[staticVertexCount++] = v;
    }

    for (int i = 0; i < indexCount; i++) {
        obj->indices[staticIndexCount++] = (uint16_t)(baseVertex + indices[i]);
    }

    return (DrawRange){ (uint32_t)baseIndex, (uint32_t)indexCount };
}

// Called once after all appends — uploads to GPU
void static_batch_upload(GraphicsObject* obj)
{
    if (staticBatchReady) return;

    vk_update_vertex_buffer(&context, obj, obj->vertices, staticVertexCount, VERTEX_FORMAT_POS_COLOR_TEX_NORM);
    vk_update_index_buffer(&context, obj, obj->indices, staticIndexCount);

    staticBatchReady = true;
}

// Called every frame — single draw call, zero CPU work
void static_batch_draw(GraphicsObject* obj, uint32_t instanceCount)
{
    if (!staticBatchReady) return;

    

    vk_update_uniform_buffer(obj);
    vk_bind_pipeline(obj);
    vk_bind_descriptor_sets(obj);
    vk_bind_vertex_buffer(obj);
    vk_bind_index_buffer(obj);
    vk_push_constants(obj);

    vk_draw(obj, staticIndexCount, instanceCount, 0, 0, 0);
}

// Binds the graphics object to the GPU (Call this once before looping)
void static_batch_bind(GraphicsObject* obj) {
    vk_update_uniform_buffer(obj);
    vk_bind_pipeline(obj);
    vk_bind_descriptor_sets(obj);
    vk_bind_vertex_buffer(obj);
    vk_bind_index_buffer(obj);
}

// Draws a specific shape from the buffer (Call this inside your loop)
void static_batch_draw_range(GraphicsObject* obj, uint32_t instanceCount, 
                             uint32_t firstIndex, uint32_t indexCount, 
                             float xPos, float yPos) 
{
    // Push the dynamic position for this specific shape
    obj->pushConstants.offset = (Vec4){xPos, yPos, 0.0f, -1.0f};
    vk_push_constants(obj);
    
    // Draw just this chunk of the vertex buffer
    vk_draw(obj, indexCount, instanceCount, firstIndex, 0, 0);
}

// In static_polygon_batch.c
void static_batch_set_state(GraphicsObject* obj, struct mat4 finalTransform, struct Vec4 color, float mode) {
    // 1. Accept the exact matrix the user calculated
    obj->ubo.model = finalTransform;
    
    // 2. Sync with GPU
    vk_update_uniform_buffer(obj);
    
    // 3. Set global push constants
    obj->pushConstants.color = color;
    obj->pushConstants.padding = (struct Vec4){mode, 0.0f, 0.0f, 0.0f};
}