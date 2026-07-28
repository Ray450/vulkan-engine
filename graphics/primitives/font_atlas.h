#ifndef FONT_ATLAS_H
#define FONT_ATLAS_H

#include <stdbool.h>
#include <stdint.h>
#include "../vulkan/vulkan_graphics.h"

#define FONT_ATLAS_SIZE     512      // Atlas texture dimensions (512x512)
#define FONT_FIRST_CHAR     32       // Space
#define FONT_LAST_CHAR      126      // ~
#define FONT_GLYPH_COUNT    (FONT_LAST_CHAR - FONT_FIRST_CHAR + 1)

typedef struct {
    float u0, v0, u1, v1;   // UV coords in atlas texture
    int   width, height;     // Glyph bitmap size in pixels
    int   xoff, yoff;        // Bearing (offset from cursor baseline)
    int   advance;           // How far to move cursor after this glyph
} GlyphInfo;

typedef struct {
    GlyphInfo glyphs[FONT_GLYPH_COUNT];
    Texture   atlasTexture;
    int       fontSize;
    int       lineHeight;
    bool      loaded;
} FontAtlas;

// Build the atlas from a TTF file using stb_truetype
// Returns true on success
bool font_atlas_build(FontAtlas* atlas, const char* fontPath, int fontSize, VulkanContext* context);

// Destroy GPU resources
void font_atlas_destroy(FontAtlas* atlas, VulkanContext* context);

// Get glyph info for a character (returns NULL for unsupported chars)
const GlyphInfo* font_atlas_glyph(const FontAtlas* atlas, char c);

#endif // FONT_ATLAS_H