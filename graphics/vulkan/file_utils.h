#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

// #include "math_api.h"

// #include "vulkan_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../math/matrix.h"


// #include "vulkan_utils.h"
// #include "vulkan_types.h"

typedef struct {
    uint16_t v_idx;
    uint16_t vt_idx;
    uint16_t vn_idx;
} FaceIndex;

typedef struct {
    Vec3* positions;
    Vec2* texcoords;
    Vec3* normals;
    FaceIndex* indices;
    size_t vertexCount;
    size_t indexCount;
} ModelData;

char* read_file(const char* filename, size_t* fileSize);

int parse_obj(ModelData* model, const char* filename);


unsigned char *load_font_bitmap(const char *font_path, char character, int pixel_height, 
                                int *out_width, int *out_height, int *x_offset, int *y_offset);

unsigned char* load_image_pixels(const char* filepath, int* width, int* height);

void save_image_png(const char* filename, unsigned char* raw_pixels, uint32_t width, uint32_t height, bool is_bgra);

void write_float_array_to_csv(
    const char* path,
    const float* data,
    size_t rows,
    size_t cols,
    bool overwrite
);

#endif