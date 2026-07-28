#include "dynamic_fan_batch.h"



int batchVertexCount = 0;
int batchIndexCount  = 0;
int segVertexCount   = 0;
int segIndexCount    = 0;

AtlasRegion atlas[MAX_ATLAS_TEXTURES];


GraphicsObject dynamic_batch_object_create(VulkanContext* context, Pipeline* pipeline, Texture texture, Vertex* vertices, uint16_t* indices, size_t vertexCount, size_t indexCount) {
    return init_graphics_object(context, pipeline, texture, vertices, indices, vertexCount, indexCount);
}



unsigned char* dynamic_batch_build_atlas(PixelBuffer* pixelArray, int count, int* outWidth, int* outHeight, AtlasRegion* atlas) {
    const int MAX_GPU_WIDTH = 16384; // The limit from your error
    int atlasWidth = 0;
    int atlasHeight = 0;

    // 1. Determine Dimensions with Wrapping
    int currentRowWidth = 0;
    int maxRowHeight = 0;
    int totalHeight = 0;

    for (int i = 0; i < count; i++) {
        if (currentRowWidth + pixelArray[i].width > MAX_GPU_WIDTH) {
            // We need a new row
            if (currentRowWidth > atlasWidth) atlasWidth = currentRowWidth;
            totalHeight += maxRowHeight;
            currentRowWidth = pixelArray[i].width;
            maxRowHeight = pixelArray[i].height;
        } else {
            currentRowWidth += pixelArray[i].width;
            if (pixelArray[i].height > maxRowHeight) maxRowHeight = pixelArray[i].height;
        }
    }
    // Add the final row
    if (currentRowWidth > atlasWidth) atlasWidth = currentRowWidth;
    totalHeight += maxRowHeight;

    atlasHeight = totalHeight;
    unsigned char* atlasPixels = (unsigned char*)calloc(atlasWidth * atlasHeight * 4, 1);
    if (!atlasPixels) LOG_FATAL("Failed to allocate atlas memory.");

    // 2. Copy Pixels and Set UVs
    int currentX = 0;
    int currentY = 0;
    int rowTallest = 0;

    for (int i = 0; i < count; i++) {
        int w = pixelArray[i].width;
        int h = pixelArray[i].height;

        // Check if we need to wrap to the next row
        if (currentX + w > MAX_GPU_WIDTH) {
            currentX = 0;
            currentY += rowTallest;
            rowTallest = 0;
        }

        if (h > rowTallest) rowTallest = h;

        // The Memory Copy logic (Now accounts for Y offset)
        for (int row = 0; row < h; row++) {
            size_t destOffset = ((size_t)(currentY + row) * atlasWidth + currentX) * 4;
            size_t srcOffset = (size_t)row * w * 4;
            memcpy(atlasPixels + destOffset, pixelArray[i].pixels + srcOffset, w * 4);
        }

        // Calculate UVs based on the new grid position
        atlas[i].u0 = (float)currentX / atlasWidth;
        atlas[i].v0 = (float)currentY / atlasHeight;
        atlas[i].u1 = (float)(currentX + w) / atlasWidth;
        atlas[i].v1 = (float)(currentY + h) / atlasHeight;

        currentX += w;
    }

    *outWidth = atlasWidth;
    *outHeight = atlasHeight;
    return atlasPixels;
}

void dynamic_batch_render(GraphicsObject* obj, Vec2 xOffset, unsigned int color, unsigned int stroke,
                 Vec2* axVertices, int iNumVertices, uint32_t textureId)
{
    if (!axVertices || iNumVertices < 3) return;
    if (batchVertexCount + (iNumVertices - 2) * 3 >= MAX_SHAPE_VERTS) return;

    AtlasRegion reg = atlas[textureId];

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8)  & 0xFF) / 255.0f;
    float b = ((color >> 0)  & 0xFF) / 255.0f;
    float a = ((color >> 24) & 0xFF) / 255.0f;
    if (a == 0.0f) a = 1.0f;

    float minX = axVertices[0].x, maxX = axVertices[0].x;
    float minY = axVertices[0].y, maxY = axVertices[0].y;
    for (int i = 1; i < iNumVertices; i++) {
        if (axVertices[i].x < minX) minX = axVertices[i].x;
        if (axVertices[i].x > maxX) maxX = axVertices[i].x;
        if (axVertices[i].y < minY) minY = axVertices[i].y;
        if (axVertices[i].y > maxY) maxY = axVertices[i].y;
    }
    float rangeX = (maxX - minX);
    float rangeY = (maxY - minY);
    if (rangeX < 0.0001f) rangeX = 0.0001f;
    if (rangeY < 0.0001f) rangeY = 0.0001f;

    int triCount = iNumVertices - 2;

    for (int i = 0; i < triCount; i++) {
        int base = batchVertexCount;

        Vec2 v0 = axVertices[0];
        Vec2 v1 = axVertices[i + 1];
        Vec2 v2 = axVertices[i + 2];

        float u0 = (v0.x - minX) / rangeX;  float tv0 = (v0.y - minY) / rangeY;
        float u1 = (v1.x - minX) / rangeX;  float tv1 = (v1.y - minY) / rangeY;
        float u2 = (v2.x - minX) / rangeX;  float tv2 = (v2.y - minY) / rangeY;

        u0  = reg.u0 + u0 * (reg.u1 - reg.u0);  tv0 = reg.v0 + tv0 * (reg.v1 - reg.v0);
        u1  = reg.u0 + u1 * (reg.u1 - reg.u0);  tv1 = reg.v0 + tv1 * (reg.v1 - reg.v0);
        u2  = reg.u0 + u2 * (reg.u1 - reg.u0);  tv2 = reg.v0 + tv2 * (reg.v1 - reg.v0);

        obj->vertices[base]   = (Vertex){{v0.x + xOffset.x, v0.y + xOffset.y, 0.f}, {r,g,b,a}, {u0, tv0}, {0,0,1}};
        obj->vertices[base+1] = (Vertex){{v1.x + xOffset.x, v1.y + xOffset.y, 0.f}, {r,g,b,a}, {u1, tv1}, {0,0,1}};
        obj->vertices[base+2] = (Vertex){{v2.x + xOffset.x, v2.y + xOffset.y, 0.f}, {r,g,b,a}, {u2, tv2}, {0,0,1}};

        obj->indices[batchIndexCount]   = base;
        obj->indices[batchIndexCount+1] = base + 1;
        obj->indices[batchIndexCount+2] = base + 2;

        batchVertexCount += 3;
        batchIndexCount  += 3;
    }
}

void dynamic_batch_render_shape_3d(GraphicsObject* obj, struct Vec3 xOffset, unsigned int color,
                   struct Vec3* axVertices, int iNumVertices, uint32_t textureId)
{
    if (!axVertices || iNumVertices < 3) return;
    if (batchVertexCount + (iNumVertices - 2) * 3 >= MAX_SHAPE_VERTS) return;

    AtlasRegion reg = atlas[textureId];

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8)  & 0xFF) / 255.0f;
    float b = ((color >> 0)  & 0xFF) / 255.0f;
    float a = ((color >> 24) & 0xFF) / 255.0f;
    if (a == 0.0f) a = 1.0f;

    float minX = axVertices[0].x, maxX = axVertices[0].x;
    float minY = axVertices[0].y, maxY = axVertices[0].y;
    for (int i = 1; i < iNumVertices; i++) {
        if (axVertices[i].x < minX) minX = axVertices[i].x;
        if (axVertices[i].x > maxX) maxX = axVertices[i].x;
        if (axVertices[i].y < minY) minY = axVertices[i].y;
        if (axVertices[i].y > maxY) maxY = axVertices[i].y;
    }
    float rangeX = (maxX - minX); if (rangeX < 0.0001f) rangeX = 0.0001f;
    float rangeY = (maxY - minY); if (rangeY < 0.0001f) rangeY = 0.0001f;

    int triCount = iNumVertices - 2;
    for (int i = 0; i < triCount; i++) {
        int base = batchVertexCount;

        struct Vec3 v0 = axVertices[0];
        struct Vec3 v1 = axVertices[i + 1];
        struct Vec3 v2 = axVertices[i + 2];

        float u0 = reg.u0 + ((v0.x - minX) / rangeX) * (reg.u1 - reg.u0);
        float u1 = reg.u0 + ((v1.x - minX) / rangeX) * (reg.u1 - reg.u0);
        float u2 = reg.u0 + ((v2.x - minX) / rangeX) * (reg.u1 - reg.u0);
        float tv0 = reg.v0 + ((v0.y - minY) / rangeY) * (reg.v1 - reg.v0);
        float tv1 = reg.v0 + ((v1.y - minY) / rangeY) * (reg.v1 - reg.v0);
        float tv2 = reg.v0 + ((v2.y - minY) / rangeY) * (reg.v1 - reg.v0);

        obj->vertices[base]   = (Vertex){{v0.x + xOffset.x, v0.y + xOffset.y, v0.z + xOffset.z}, {r,g,b,a}, {u0, tv0}, {0,0,1}};
        obj->vertices[base+1] = (Vertex){{v1.x + xOffset.x, v1.y + xOffset.y, v1.z + xOffset.z}, {r,g,b,a}, {u1, tv1}, {0,0,1}};
        obj->vertices[base+2] = (Vertex){{v2.x + xOffset.x, v2.y + xOffset.y, v2.z + xOffset.z}, {r,g,b,a}, {u2, tv2}, {0,0,1}};

        obj->indices[batchIndexCount]   = base;
        obj->indices[batchIndexCount+1] = base + 1;
        obj->indices[batchIndexCount+2] = base + 2;

        batchVertexCount += 3;
        batchIndexCount  += 3;
    }
}

// Called ONCE at end of draw() - single pipeline bind, single flush, N draw calls
void dynamic_batch_flush(GraphicsObject* obj, uint32_t instanceCount) {
    if (batchIndexCount == 0) return;

    VkMappedMemoryRange ranges[2] = {
        {VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, NULL, obj->vertexBuffer2.memory, 0, VK_WHOLE_SIZE},
        {VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, NULL, obj->indexBuffer2.memory,  0, VK_WHOLE_SIZE}
    };
    vkFlushMappedMemoryRanges(context.device, 2, ranges);

    
    /*obj->ubo.model = createMat4(1.0f);
    obj->ubo.model = applyScaling(obj->ubo.model, (struct Vec3){1.0f, 1.0f, 1.0f});
    obj->ubo.model = applyTranslation(obj->ubo.model, (struct Vec3){0.0f, 0.0f, 0.0f});
            

    obj->pushConstants.color = (Vec4){1.0f, 1.0f, 1.0f, 1.0f};
    obj->pushConstants.offset        = (Vec4){0.0f, 0.0f, 0.0f, 0.0f};
    obj->pushConstants.padding       = (Vec4){2.0f, 0.0f, 0.0f, 0.0f};*/

    vk_update_uniform_buffer(obj);

    // Bind ONCE
    vk_bind_pipeline(obj);
    vk_bind_descriptor_sets(obj);
    vk_bind_vertex_buffer(obj);
    vk_bind_index_buffer(obj);
    vk_push_constants(obj);

    // Draw everything in one call
    vk_draw(obj, batchIndexCount, instanceCount, 0, 0, 0);

    // Reset for next frame
    batchVertexCount = 0;
    batchIndexCount  = 0;
}


void dynamic_batch_render_cube(GraphicsObject* obj, struct Vec3 pos, float size, uint32_t textureId) {
    float h = size * 0.5f;

    struct Vec3 front[4]  = {{-h,-h, h},{h,-h, h},{h, h, h},{-h, h, h}};
    struct Vec3 back[4]   = {{ h,-h,-h},{-h,-h,-h},{-h, h,-h},{ h, h,-h}};
    struct Vec3 left[4]   = {{-h,-h,-h},{-h,-h, h},{-h, h, h},{-h, h,-h}};
    struct Vec3 right[4]  = {{ h,-h, h},{ h,-h,-h},{ h, h,-h},{ h, h, h}};
    struct Vec3 top[4]    = {{-h, h, h},{ h, h, h},{ h, h,-h},{-h, h,-h}};
    struct Vec3 bottom[4] = {{-h,-h,-h},{ h,-h,-h},{ h,-h, h},{-h,-h, h}};

    dynamic_batch_render_shape_3d(obj, pos, 0xFFFFFFFF, front,  4, textureId);
    dynamic_batch_render_shape_3d(obj, pos, 0xFFFFFFFF, back,   4, textureId);
    dynamic_batch_render_shape_3d(obj, pos, 0xFFFFFFFF, left,   4, textureId);
    dynamic_batch_render_shape_3d(obj, pos, 0xFFFFFFFF, right,  4, textureId);
    dynamic_batch_render_shape_3d(obj, pos, 0xFFFFFFFF, top,    4, textureId);
    dynamic_batch_render_shape_3d(obj, pos, 0xFFFFFFFF, bottom, 4, textureId);
}


// Add this new function (place it near RenderShape3D / RenderCube)

// Renders any pre-triangulated mesh (from loadModel) into the batch
// - vertices & indices must come from your loadModel function
// - offset = world position to place the model
// - color = global tint multiplier (0xFFFFFFFF = no tint)
// - textureId = which atlas slot to use (UVs will be remapped into that region)
void dynamic_batch_render_mesh(GraphicsObject* obj,
                struct Vec3 offset,
                unsigned int color,
                const Vertex* srcVertices,    // from loadModel
                int srcVertexCount,
                const uint16_t* srcIndices,   // from loadModel
                int srcIndexCount,
                uint32_t textureId)
{
    if (srcVertexCount <= 0 || srcIndexCount <= 0) return;

    // Safety check against batch overflow
    if (batchVertexCount + srcVertexCount >= MAX_SHAPE_VERTS ||
        batchIndexCount  + srcIndexCount  >= MAX_SHAPE_INDICES) {  // define MAX_SHAPE_INDICES if missing, e.g. 65536*4 or similar
        // Optional: log warning "Batch full - skipping mesh"
        LOG_ERROR("RenderMesh: verts=%d/%d, indices=%d/%d\n",
         batchVertexCount + srcVertexCount, MAX_SHAPE_VERTS,
         batchIndexCount + srcIndexCount, MAX_SHAPE_INDICES);
        return;
    }

    AtlasRegion reg = atlas[textureId];

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >>  8) & 0xFF) / 255.0f;
    float b = ((color >>  0) & 0xFF) / 255.0f;
    float a = ((color >> 24) & 0xFF) / 255.0f;
    if (a == 0.0f) a = 1.0f;

    int baseVertex = batchVertexCount;

    // Copy & transform all vertices
    for (int i = 0; i < srcVertexCount; i++) {
        Vertex v = srcVertices[i];

        // Apply position offset
        v.pos.x += offset.x;
        v.pos.y += offset.y;
        v.pos.z += offset.z;

        // Optional tint: multiply vertex color (useful if model has baked lighting / vertex colors)
        // If model has no per-vertex color yet, set it fully here
        v.color.r *= r;
        v.color.g *= g;
        v.color.b *= b;
        v.color.a *= a;

        // Remap model's [0,1] UVs into the chosen atlas region
        // (most OBJs use 0-1 UVs per material — this packs it into your atlas slot)
        float u = reg.u0 + v.texCoord.x * (reg.u1 - reg.u0);
        float v_tex = reg.v0 + v.texCoord.y * (reg.v1 - reg.v0);
        v.texCoord.x = u;
        v.texCoord.y = v_tex;

        // Normal stays as-is (assuming model normals are correct)

        obj->vertices[batchVertexCount++] = v;
    }

    // Copy indices, offset by our base in the batch
    for (int i = 0; i < srcIndexCount; i++) {
        obj->indices[batchIndexCount++] = (uint16_t)(baseVertex + srcIndices[i]);
    }
}