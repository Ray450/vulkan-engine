#include "ui.h"
#include "dynamic_fan_batch.h"
#include "font_batch.h"
#include "../vulkan/sys.h"
#include <cstring>
#include <cstdlib>

// Reserved atlas slot for the UI's solid-white texture, so its UV region
// (0,0)-(1,1) never collides with slots used by the game's own texture
// atlas (built via dynamic_batch_build_atlas / static_batch_init_atlas,
// which populate low indices starting at 0). Kept far away at the top
// of the range on purpose.
#define UI_ATLAS_SLOT (MAX_ATLAS_TEXTURES - 1)

#define MAX_UI_VERTS   4096
#define MAX_UI_INDICES 6144

// Buttons queue their label here during ui_button(); ui_end() draws all
// of them in one pass, AFTER the button rectangles have been flushed.
// This is required, not a style choice: dynamic_batch_render (rects) and
// font_batch_draw_text (labels) write into two different GraphicsObjects
// but share the same global batchVertexCount/batchIndexCount counters.
// Interleaving calls to both within one begin/flush cycle would advance
// the shared counter past what either object's vertex array actually has
// written into it that frame, corrupting whichever one flushes second.
#define MAX_UI_LABELS 64
struct QueuedLabel {
    float ndcX, ndcY;
    char text[32];
};
static QueuedLabel labelQueue[MAX_UI_LABELS];
static int labelQueueCount = 0;

static VulkanContext* uiContext   = nullptr;
static FontAtlas*      uiFontAtlas = nullptr;
static GraphicsObject* uiFontBatchObj = nullptr;

static GraphicsObject uiBatchObject;
static Vertex*   uiVerts   = nullptr;
static uint16_t* uiIndices = nullptr;
static bool uiInitialized = false;

static bool prevMouseDown = false;

void pixel_to_ndc(double pixelX, double pixelY, float* ndcX, float* ndcY) {
    float w      = (float)get_graphics_width();
    float h      = (float)get_graphics_height();
    float aspect = w / h;

    *ndcX =  ((float)pixelX / w) * 2.0f * aspect - aspect;
    *ndcY =  ((float)pixelY / h) * 2.0f - 1.0f;
}

void ui_init(VulkanContext* context, Pipeline* pipeline,
             FontAtlas* fontAtlas, GraphicsObject* fontBatchObj) {
    uiContext      = context;
    uiFontAtlas    = fontAtlas;
    uiFontBatchObj = fontBatchObj;

    // Solid opaque white, small (4x4) since it's just tinted by vertex color.
    unsigned char whitePixels[4 * 4 * 4];
    memset(whitePixels, 255, sizeof(whitePixels));
    Texture whiteTexture = create_texture(context, whitePixels, 4, 4, VK_FORMAT_R8G8B8A8_SRGB);

    uiVerts   = (Vertex*)calloc(MAX_UI_VERTS, sizeof(Vertex));
    uiIndices = (uint16_t*)calloc(MAX_UI_INDICES, sizeof(uint16_t));

    uiBatchObject = dynamic_batch_object_create(context, pipeline, whiteTexture,
                                                 uiVerts, uiIndices,
                                                 MAX_UI_VERTS, MAX_UI_INDICES);

    // Full-texture UV region — no packing needed for a single solid color.
    atlas[UI_ATLAS_SLOT] = (AtlasRegion){0.0f, 0.0f, 1.0f, 1.0f};

    uiInitialized = true;
}

void ui_begin(void) {
    labelQueueCount = 0;
    batchVertexCount = 0;
    batchIndexCount  = 0;
}

bool ui_button(float x, float y, float w, float h, const char* label) {
    if (!uiInitialized) return false;

    double mouseX, mouseY;
    sys_get_mouse_position(&mouseX, &mouseY);
    bool mouseDown = sys_get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT) == KEY_PRESSED;

    bool hovered = (mouseX >= x && mouseX <= x + w &&
                    mouseY >= y && mouseY <= y + h);

    // Click registers on release, only if still hovered — lets the user
    // cancel a press by dragging off the button before releasing.
    bool clicked = hovered && !mouseDown && prevMouseDown;

    unsigned int color = 0xFFCCCCCC; // base light gray
    if (hovered && mouseDown) {
        color = 0xFF888888; // pressed: darker
    } else if (hovered) {
        color = 0xFFEEEEEE; // hover: lighter
    }

    float ndcX0, ndcY0, ndcX1, ndcY1;
    pixel_to_ndc(x,     y,     &ndcX0, &ndcY0);
    pixel_to_ndc(x + w, y + h, &ndcX1, &ndcY1);

    Vec2 quad[4] = {
        {ndcX0, ndcY0},
        {ndcX1, ndcY0},
        {ndcX1, ndcY1},
        {ndcX0, ndcY1},
    };

    dynamic_batch_render(&uiBatchObject, (Vec2){0.0f, 0.0f}, color, 0, quad, 4, UI_ATLAS_SLOT);

    // Queue the label instead of drawing it now — see the comment on
    // MAX_UI_LABELS above for why.
    if (label && uiFontAtlas && uiFontBatchObj && labelQueueCount < MAX_UI_LABELS) {
        float labelNdcX, labelNdcY;
        // Left-padded, roughly vertically centered (first pass — not
        // measuring string width for true centering yet).
        pixel_to_ndc(x + 6.0f, y + h * 0.5f, &labelNdcX, &labelNdcY);

        QueuedLabel& q = labelQueue[labelQueueCount++];
        q.ndcX = labelNdcX;
        q.ndcY = labelNdcY;
        strncpy(q.text, label, sizeof(q.text) - 1);
        q.text[sizeof(q.text) - 1] = '\0';
    }

    prevMouseDown = mouseDown;
    return clicked;
}

void ui_end(void) {
    if (!uiInitialized) return;

    // Pass 1: flush all button rectangles as their own complete cycle.
    //dynamic_batch_flush(&uiBatchObject, 1);

    // Pass 2: draw + flush all queued labels as a separate complete cycle.
    if (uiFontAtlas && uiFontBatchObj && labelQueueCount > 0) {
        font_batch_begin();
        for (int i = 0; i < labelQueueCount; i++) {
            font_batch_draw_text(uiFontBatchObj, uiFontAtlas, labelQueue[i].text,
                                  labelQueue[i].ndcX, labelQueue[i].ndcY,
                                  1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        }
        font_batch_flush(uiFontBatchObj, uiContext);
    }
}