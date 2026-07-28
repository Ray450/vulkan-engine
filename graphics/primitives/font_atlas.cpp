#include "font_atlas.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// #define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"   // already in your project via load_font_bitmap

bool font_atlas_build(FontAtlas* atlas, const char* fontPath, int fontSize, VulkanContext* context) {
    memset(atlas, 0, sizeof(FontAtlas));
    atlas->fontSize   = fontSize;
    atlas->lineHeight = fontSize;

    // --- Load TTF file ---
    FILE* f = fopen(fontPath, "rb");
    if (!f) { fprintf(stderr, "font_atlas_build: cannot open %s\n", fontPath); return false; }
    fseek(f, 0, SEEK_END);
    long ttfSize = ftell(f);
    rewind(f);
    unsigned char* ttfBuffer = (unsigned char*)malloc(ttfSize);
    fread(ttfBuffer, 1, ttfSize, f);
    fclose(f);

    stbtt_fontinfo fontInfo;
    if (!stbtt_InitFont(&fontInfo, ttfBuffer, stbtt_GetFontOffsetForIndex(ttfBuffer, 0))) {
        fprintf(stderr, "font_atlas_build: stbtt_InitFont failed\n");
        free(ttfBuffer);
        return false;
    }

    float scale = stbtt_ScaleForPixelHeight(&fontInfo, (float)fontSize);

    // Ascent/descent for baseline calculation
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
    atlas->lineHeight = (int)((ascent - descent + lineGap) * scale);

    // --- Allocate RGBA atlas pixels (initialized to transparent) ---
    int atlasW = FONT_ATLAS_SIZE;
    int atlasH = FONT_ATLAS_SIZE;
    unsigned char* atlasPixels = (unsigned char*)calloc(atlasW * atlasH * 4, 1);

    int cursorX = 1, cursorY = 1, rowHeight = 0;

    for (int ci = FONT_FIRST_CHAR; ci <= FONT_LAST_CHAR; ci++) {
        int glyphIndex = ci - FONT_FIRST_CHAR;

        // Get glyph metrics
        int advanceWidth, leftBearing;
        stbtt_GetCodepointHMetrics(&fontInfo, ci, &advanceWidth, &leftBearing);

        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&fontInfo, ci, scale, scale, &x0, &y0, &x1, &y1);
        int gw = x1 - x0;
        int gh = y1 - y0;

        // Wrap to next row if needed
        if (cursorX + gw + 1 >= atlasW) {
            cursorX = 1;
            cursorY += rowHeight + 1;
            rowHeight = 0;
        }
        if (cursorY + gh + 1 >= atlasH) {
            fprintf(stderr, "font_atlas_build: atlas too small for font size %d\n", fontSize);
            break;
        }

        // Render single-channel bitmap
        unsigned char* glyphBitmap = (unsigned char*)calloc(gw * gh, 1);
        stbtt_MakeCodepointBitmap(&fontInfo, glyphBitmap, gw, gh, gw, scale, scale, ci);

        // Copy into atlas as RGBA (white glyph, alpha from bitmap)
        for (int gy = 0; gy < gh; gy++) {
            for (int gx = 0; gx < gw; gx++) {
                int atlasIdx = ((cursorY + gy) * atlasW + (cursorX + gx)) * 4;
                unsigned char alpha = glyphBitmap[gy * gw + gx];
                atlasPixels[atlasIdx + 0] = 255;   // R
                atlasPixels[atlasIdx + 1] = 255;   // G
                atlasPixels[atlasIdx + 2] = 255;   // B
                atlasPixels[atlasIdx + 3] = alpha;  // A — shape of the glyph
            }
        }
        free(glyphBitmap);

        // Store glyph info
        atlas->glyphs[glyphIndex].u0      = (float)cursorX / atlasW;
        atlas->glyphs[glyphIndex].v0      = (float)cursorY / atlasH;
        atlas->glyphs[glyphIndex].u1      = (float)(cursorX + gw) / atlasW;
        atlas->glyphs[glyphIndex].v1      = (float)(cursorY + gh) / atlasH;
        atlas->glyphs[glyphIndex].width   = gw;
        atlas->glyphs[glyphIndex].height  = gh;
        atlas->glyphs[glyphIndex].xoff    = x0;
        atlas->glyphs[glyphIndex].yoff    = y0 + (int)(ascent * scale);  // baseline-relative
        atlas->glyphs[glyphIndex].advance = (int)(advanceWidth * scale);

        cursorX += gw + 1;
        if (gh > rowHeight) rowHeight = gh;
    }

    free(ttfBuffer);

    // Upload to GPU — same path as your atlas texture
    atlas->atlasTexture = create_texture(context, atlasPixels, atlasW, atlasH, VK_FORMAT_R8G8B8A8_UNORM);
    free(atlasPixels);

    atlas->loaded = true;
    return true;
}

void font_atlas_destroy(FontAtlas* atlas, VulkanContext* context) {
    if (!atlas->loaded) return;
    // destroy_texture(context->device, &atlas->atlasTexture);  // your existing cleanup
    atlas->loaded = false;
}

const GlyphInfo* font_atlas_glyph(const FontAtlas* atlas, char c) {
    if ((unsigned char)c < FONT_FIRST_CHAR || (unsigned char)c > FONT_LAST_CHAR) return NULL;
    return &atlas->glyphs[(unsigned char)c - FONT_FIRST_CHAR];
}