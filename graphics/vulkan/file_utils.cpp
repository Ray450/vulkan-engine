#include "../logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION

#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb/stb_image_write.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"
#include "file_utils.h"

char* read_file(const char* filename, size_t* fileSize) {
    FILE* file = fopen(filename, "rb");

    if (!file) {
        logMessage(LOG_LEVEL_ERROR, "Failed to open file: %s", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(*fileSize);
    if (!buffer) {
        logMessage(LOG_LEVEL_ERROR, "Failed to allocate %zu bytes for file %s", *fileSize, filename);
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, *fileSize, file);
    if (bytesRead != *fileSize) {
        logMessage(LOG_LEVEL_WARNING, "Read only %zu of %zu bytes from %s", bytesRead, *fileSize, filename);
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);

    return buffer;
}

int parse_obj(ModelData* model, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        logMessage(LOG_LEVEL_ERROR, "Failed to open file: %s", filename);
        return 0;
    }

    char line[256];
    size_t vCount = 0, vtCount = 0, vnCount = 0, fCount = 0;
    size_t vCapacity = 128, vtCapacity = 128, vnCapacity = 128, fCapacity = 256;

    model->positions = (Vec3*)malloc(vCapacity * sizeof(Vec3));
    model->texcoords = (Vec2*)malloc(vtCapacity * sizeof(Vec2));
    model->normals = (Vec3*)malloc(vnCapacity * sizeof(Vec3));
    model->indices = (FaceIndex*)malloc(fCapacity * sizeof(FaceIndex));

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            if (vCount >= vCapacity) {
                vCapacity *= 2;
                model->positions = (Vec3*)realloc(model->positions, vCapacity * sizeof(Vec3));
            }
            if (sscanf(line, "v %f %f %f", &model->positions[vCount].x, &model->positions[vCount].y, &model->positions[vCount].z) == 3) {
                vCount++;
            }
        } else if (strncmp(line, "vt ", 3) == 0) {
            if (vtCount >= vtCapacity) {
                vtCapacity *= 2;
                model->texcoords = (Vec2*)realloc(model->texcoords, vtCapacity * sizeof(Vec2));
            }
            if (sscanf(line, "vt %f %f", &model->texcoords[vtCount].x, &model->texcoords[vtCount].y) == 2) {
                vtCount++;
            }
        } else if (strncmp(line, "vn ", 3) == 0) {
            if (vnCount >= vnCapacity) {
                vnCapacity *= 2;
                model->normals = (Vec3*)realloc(model->normals, vnCapacity * sizeof(Vec3));
            }
            if (sscanf(line, "vn %f %f %f", &model->normals[vnCount].x, &model->normals[vnCount].y, &model->normals[vnCount].z) == 3) {
                vnCount++;
            }
        } else if (strncmp(line, "f ", 2) == 0) {
            if (fCount + 3 > fCapacity) {
                fCapacity *= 2;
                model->indices = (FaceIndex*)realloc(model->indices, fCapacity * sizeof(FaceIndex));
            }
            int v1, vt1, vn1, v2, vt2, vn2, v3, vt3, vn3;
            if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", 
                    &v1, &vt1, &vn1, 
                    &v2, &vt2, &vn2, 
                    &v3, &vt3, &vn3) == 9) {
                model->indices[fCount++] = (FaceIndex){(uint16_t)(v1 - 1), (uint16_t)(vt1 - 1), (uint16_t)(vn1 - 1)};
                model->indices[fCount++] = (FaceIndex){(uint16_t)(v2 - 1), (uint16_t)(vt2 - 1), (uint16_t)(vn2 - 1)};
                model->indices[fCount++] = (FaceIndex){(uint16_t)(v3 - 1), (uint16_t)(vt3 - 1), (uint16_t)(vn3 - 1)};
            }
        }
    }

    fclose(file);

    model->vertexCount = vCount;
    model->indexCount = fCount;

    return 1;
}

unsigned char *load_font_bitmap(const char *font_path, char character, int pixel_height, 
                                int *out_width, int *out_height, int *x_offset, int *y_offset) {
    FILE *fontFile = fopen(font_path, "rb");
    if (!fontFile) {
        logMessage(LOG_LEVEL_ERROR, "Failed to open font file: %s", font_path);
        return NULL;
    }

    fseek(fontFile, 0, SEEK_END);
    long size = ftell(fontFile);
    fseek(fontFile, 0, SEEK_SET);

    unsigned char *fontBuffer = (unsigned char *)malloc(size);
    if (!fontBuffer) {
        fclose(fontFile);
        logMessage(LOG_LEVEL_ERROR, "Memory allocation failed for font buffer");
        return NULL;
    }
    fread(fontBuffer, size, 1, fontFile);
    fclose(fontFile);

    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontBuffer, stbtt_GetFontOffsetForIndex(fontBuffer, 0))) {
        logMessage(LOG_LEVEL_ERROR, "Failed to initialize font");
        free(fontBuffer);
        return NULL;
    }

    float scale = stbtt_ScaleForPixelHeight(&font, pixel_height);
    unsigned char *bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, character, 
                                                     out_width, out_height, x_offset, y_offset);

    free(fontBuffer);

    return bitmap;
}


unsigned char* load_image_pixels(const char* filepath, int* width, int* height) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filepath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    
    if (!pixels) {
        LOG_ERROR("Failed to load texture image %s", filepath);
        return NULL;
    }

    size_t imageSize = (size_t)(texWidth * texHeight * 4);
    unsigned char* pixelData = (unsigned char*)malloc(imageSize);
    if (!pixelData) {
        LOG_ERROR("Failed to allocate pixel data");
        stbi_image_free(pixels);
        return NULL;
    }
    
    memcpy(pixelData, pixels, imageSize);
    stbi_image_free(pixels);
    
    *width = texWidth;
    *height = texHeight;
    
    return pixelData;
}

void save_image_png(const char* filename, unsigned char* raw_pixels, uint32_t width, uint32_t height, bool is_bgra) {
    uint32_t bufferSize = width * height * 4;
    unsigned char* formatted_pixels = (unsigned char*)malloc(bufferSize);

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t src_index = ((height - 1 - y) * width + x) * 4;
            uint32_t dst_index = (y * width + x) * 4;

            if (is_bgra) {
                formatted_pixels[dst_index + 0] = raw_pixels[src_index + 2]; // R <- B
                formatted_pixels[dst_index + 1] = raw_pixels[src_index + 1]; // G
                formatted_pixels[dst_index + 2] = raw_pixels[src_index + 0]; // B <- R
                formatted_pixels[dst_index + 3] = raw_pixels[src_index + 3]; // A
            } else {
                formatted_pixels[dst_index + 0] = raw_pixels[src_index + 0];
                formatted_pixels[dst_index + 1] = raw_pixels[src_index + 1];
                formatted_pixels[dst_index + 2] = raw_pixels[src_index + 2];
                formatted_pixels[dst_index + 3] = raw_pixels[src_index + 3];
            }
        }
    }

    if (!stbi_write_png(filename, width, height, 4, formatted_pixels, width * 4)) {
        logMessage(LOG_LEVEL_ERROR, "Failed to write PNG: %s", filename);
    } else {
        logMessage(LOG_LEVEL_INFO, "Screenshot saved: %s", filename);
    }

    free(formatted_pixels);
}

void write_float_array_to_csv(

    const char* path,

    const float* data,

    size_t rows,

    size_t cols,

    bool overwrite

) {

    if (!path || !data) {

        fprintf(stderr, "Error: Invalid parameters for CSV write\n");

        return;

    }



    // Open file

    const char* mode = overwrite ? "w" : "a";

    FILE* file = fopen(path, mode);

    if (!file) {

        fprintf(stderr, "Error: Failed to open file '%s'\n", path);

        return;

    }



    // Write header (if overwriting)

    if (overwrite) {

        for (size_t col = 0; col < cols; col++) {

            fprintf(file, "col%zu", col);

            if (col < cols - 1) fprintf(file, ",");

        }

        fprintf(file, "\n");

    }



    // Write data rows

    for (size_t row = 0; row < rows; row++) {

        for (size_t col = 0; col < cols; col++) {

            size_t index = row * cols + col;

            fprintf(file, "%.6f", data[index]);

            if (col < cols - 1) fprintf(file, ",");

        }

        fprintf(file, "\n");

    }



    fclose(file);

}