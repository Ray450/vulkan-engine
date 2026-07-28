#ifndef FONT_BATCH_H
#define FONT_BATCH_H

#include "font_atlas.h"
#include "../vulkan/vulkan_graphics.h"
#include "../graphics/primitives/dynamic_fan_batch.h"

// One GraphicsObject per font batch — pre-allocated, reused every frame
// Shares the same dynamic batch infrastructure you already have

void font_batch_init(VulkanContext* context, GraphicsObject* obj, FontAtlas* atlas, Pipeline* pipeline,
                     Vertex* vertexBuffer, uint16_t* indexBuffer,
                     size_t maxVerts, size_t maxIndices);

// Call once per frame before drawing any text
void font_batch_begin(void);

// Queue a string — x,y in your NDC [-1,1] coordinate space
// r,g,b,a: text color (0.0-1.0)
// scale: pixel size multiplier (1.0 = fontSize pixels)
void font_batch_draw_text(GraphicsObject* obj, const FontAtlas* atlas,
                          const char* text,
                          float x, float y, float scale,
                          float r, float g, float b, float a);

// Call once per frame after all text — single draw call for all text
void font_batch_flush(GraphicsObject* obj, VulkanContext* context);

#endif // FONT_BATCH_H