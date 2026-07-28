#include "font_batch.h"
#include <string.h>

// Reuses your existing batchVertexCount / batchIndexCount globals from dynamic_fan_batch
extern int batchVertexCount;
extern int batchIndexCount;

void font_batch_init(VulkanContext* context, GraphicsObject* obj, FontAtlas* atlas, Pipeline* pipeline,
                     Vertex* vertexBuffer, uint16_t* indexBuffer,
                     size_t maxVerts, size_t maxIndices) {
    // Bind the font atlas as this object's texture
    *obj = init_graphics_object(context, pipeline, atlas->atlasTexture,
                                 vertexBuffer, indexBuffer,
                                 maxVerts, maxIndices);
}

void font_batch_begin(void) {
    batchVertexCount = 0;
    batchIndexCount  = 0;
}

void font_batch_draw_text(GraphicsObject* obj, const FontAtlas* atlas,
                          const char* str,
                          float x, float y, float scale,
                          float r, float gr, float b, float a) {  // rename green to gr
    if (!str || !atlas->loaded) return;

    float ndcScaleX = (2.0f / (float)WIDTH)  * scale;
    float ndcScaleY = (2.0f / (float)HEIGHT) * scale;

    float cursorX = x;

    for (int i = 0; str[i] != '\0'; i++) {
        const GlyphInfo* glyph = font_atlas_glyph(atlas, str[i]);  // rename g to glyph
        if (!glyph) {
            cursorX += 8.0f * ndcScaleX;
            continue;
        }
        if (glyph->width == 0 || glyph->height == 0) {
            cursorX += glyph->advance * ndcScaleX;
            continue;
        }

        float gx0 = cursorX     + glyph->xoff   * ndcScaleX;
        float gy0 = y           + glyph->yoff   * ndcScaleY;
        float gx1 = gx0         + glyph->width  * ndcScaleX;
        float gy1 = gy0         + glyph->height * ndcScaleY;

        if (batchVertexCount + 4 >= MAX_SHAPE_VERTS ||
            batchIndexCount  + 6 >= MAX_SHAPE_INDICES) break;

        int base = batchVertexCount;

        obj->vertices[base + 0] = (Vertex){{gx0, gy0, 0.f}, {r,gr,b,a}, {glyph->u0, glyph->v0}, {0,0,1}};
        obj->vertices[base + 1] = (Vertex){{gx1, gy0, 0.f}, {r,gr,b,a}, {glyph->u1, glyph->v0}, {0,0,1}};
        obj->vertices[base + 2] = (Vertex){{gx1, gy1, 0.f}, {r,gr,b,a}, {glyph->u1, glyph->v1}, {0,0,1}};
        obj->vertices[base + 3] = (Vertex){{gx0, gy1, 0.f}, {r,gr,b,a}, {glyph->u0, glyph->v1}, {0,0,1}};

        obj->indices[batchIndexCount + 0] = base + 0;
        obj->indices[batchIndexCount + 1] = base + 1;
        obj->indices[batchIndexCount + 2] = base + 2;
        obj->indices[batchIndexCount + 3] = base + 2;
        obj->indices[batchIndexCount + 4] = base + 3;
        obj->indices[batchIndexCount + 5] = base + 0;

        batchVertexCount += 4;
        batchIndexCount  += 6;
        cursorX += glyph->advance * ndcScaleX;
    }
}

void font_batch_flush(GraphicsObject* obj, VulkanContext* context) {
    if (batchIndexCount == 0) return;

    VkMappedMemoryRange ranges[2] = {
        {VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, NULL, obj->vertexBuffer2.memory, 0, VK_WHOLE_SIZE},
        {VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, NULL, obj->indexBuffer2.memory,  0, VK_WHOLE_SIZE}
    };
    vkFlushMappedMemoryRanges(context->device, 2, ranges);

    obj->ubo.model = createMat4(1.0f);
    // padding.x == 2.0 tells your shader to use texSampler (texture path)
    obj->pushConstants.color = (Vec4){1.0f, 1.0f, 1.0f, 1.0f};
    obj->pushConstants.offset        = (Vec4){0.0f, 0.0f, 0.0f, 0.0f};
    obj->pushConstants.padding       = (Vec4){2.0f, 0.0f, 0.0f, 0.0f};

    vk_update_uniform_buffer(obj);
    vk_bind_pipeline(obj);
    vk_bind_descriptor_sets(obj);
    vk_bind_vertex_buffer(obj);
    vk_bind_index_buffer(obj);
    vk_push_constants(obj);
    vk_draw(obj, batchIndexCount, 1, 0, 0, 0);

    batchVertexCount = 0;
    batchIndexCount  = 0;
}