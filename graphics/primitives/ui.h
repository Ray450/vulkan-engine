#ifndef UI_H
#define UI_H

#include "../vulkan/vulkan_graphics.h"
#include "font_atlas.h"

// Converts a pixel coordinate (origin top-left, matching GLFW's mouse
// convention) into NDC, using the current window size and the Y-down
// convention established by set_rendering_mode(context, false).
//
// Takes the coordinate as parameters rather than reading the mouse
// directly, so it can be reused for any screen-space position (mouse,
// UI element corners, etc.), not just the current cursor.
void pixel_to_ndc(double pixelX, double pixelY, float* ndcX, float* ndcY);

// Call once at startup, after the standard pipeline and font atlas/font
// batch object are ready. Reuses your existing font batch object for
// drawing button labels rather than creating a second font resource.
void ui_init(VulkanContext* context, Pipeline* pipeline,
             FontAtlas* fontAtlas, GraphicsObject* fontBatchObj);

// Call once per frame, before any ui_button calls.
void ui_begin(void);

// Draws a button at the given pixel-space rect (x, y = top-left corner,
// matching mouse coordinates) with the given label. Returns true on the
// frame the button is released while still hovered (a completed click) —
// pressing and dragging off the button before releasing does not count.
bool ui_button(float x, float y, float w, float h, const char* label);

// Call once per frame after all ui_button calls. Issues the draw calls
// for both the button rectangles and their labels.
void ui_end(void);

#endif // UI_H