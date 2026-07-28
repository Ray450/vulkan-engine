/**
 * Vulkan Engine Testbed
 * ---------------------
 * Portfolio demo showcasing:
 *   1. Instanced static polygon batch (triangle → circle)
 *   2. CPU-side procedural texture drawing
 *   3. Dynamic vertex batching
 *   4. Static model batching (OBJ + texture atlas)
 *   5. Font atlas + text rendering
 *   6. Compute shader
 *   7. MIDI-driven guitar audio
 *
 * Controls:
 *   WASD  – move instanced primitives
 *   M     – start / restart MIDI sequence
 *   0-9   – pluck individual guitar strings
 *   P     – take screenshot
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "../../graphics/primitives/primitives.h"
#include "../../audio/midi.h"
#include <audio_util.h>

// -----------------------------------------------------------------------------
// Constants & simple types
// -----------------------------------------------------------------------------

typedef struct {
    GraphicsObject* Obj;
    Vec3  pos;
    Vec3  scale;
    Vec4  color;
} Primitive;

typedef struct {
    float r, g, b;
} ColorRGB;

static const ColorRGB palette[] = {
    {1.0f, 0.2f, 0.2f}, // Soft Red
    {0.2f, 1.0f, 0.2f}, // Lime Green
    {0.2f, 0.5f, 1.0f}, // Sky Blue
    {1.0f, 0.8f, 0.0f}, // Golden Yellow
    {0.8f, 0.2f, 1.0f}, // Hot Pink
    {0.0f, 1.0f, 0.8f}, // Cyan
    {1.0f, 0.5f, 0.0f}, // Orange
    {0.9f, 0.9f, 0.9f}  // Near White
};

struct InstanceData {
    Vec4 instanceOffsets;
};

// Simple unit quad used by the CPU-texture demo
static Vertex vertices_rectangle[] = {
    {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
    {{0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}
};
static uint16_t indices_rectangle[] = {0, 1, 2, 2, 3, 0};

// -----------------------------------------------------------------------------
// Global state (kept minimal)
// -----------------------------------------------------------------------------

float xPos = 0.0f, yPos = 0.0f;   // moved by WASD
float x = 0.0f;                   // timer for one-shot texture demo
bool  updatedToQuad = false;
bool  is3D = true;
static bool screenshotTaken = false;

struct Guitar guitar = {0};

Pipeline standard_pipeline;

// Four demo objects:
//   [0] Instanced polygon batch (primitiveVertices)
//   [1] CPU-drawn texture quad
//   [2] Dynamic batch
//   [3] Static batched viking-room model
Primitive primitives3D[4];
const size_t num3DPrimitives = 4;

Texture textureArray[10];
Texture emptyTexture = {0};
Texture atlasTexture = {0};
PixelBuffer pixelBuffers[7];
const int maximum_images = 7;
unsigned char* atlasPixels = NULL;

const char* filepaths[] = {
    "../../assets/2k_earth_daymap.jpg",
    "../../assets/2k_mars.jpg",
    "../../assets/2k_mercury.jpg",
    "../../assets/2k_jupiter.jpg",
    "../../assets/2k_saturn.jpg",
    "../../assets/2k_venus_surface.jpg",
    "../../assets/viking_room.png"
};


// Dynamic-batch scratch buffers
Vertex*   dynamic_batch_vertices = NULL;
uint16_t* dynamic_batch_indices  = NULL;

// Static-batch scratch buffers
Vertex*   static_batch_vertices = NULL;
uint16_t* static_batch_indices  = NULL;

// Loaded model
Vertex*   viking_room_vertices  = NULL;
int       viking_room_vertexCount = 0;
uint16_t* viking_room_indices   = NULL;
int       viking_room_indexCount  = 0;

// Font
FontAtlas     fontAtlas;
GraphicsObject fontBatchObj;
Vertex*       fontVerts   = NULL;
uint16_t*     fontIndices = NULL;

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

void processInput() {
    if (sys_get_key_state(KEY_W) == KEY_PRESSED) yPos += 0.1f;
    if (sys_get_key_state(KEY_S) == KEY_PRESSED) yPos -= 0.1f;
    if (sys_get_key_state(KEY_A) == KEY_PRESSED) xPos -= 0.1f;
    if (sys_get_key_state(KEY_D) == KEY_PRESSED) xPos += 0.1f;
}

void processInput2(struct Guitar* g) {
    static int previousStates[10] = {KEY_RELEASED};
    static int prevM = KEY_RELEASED;

    // Press M to (re)load and start the MIDI sequence
    int mState = sys_get_key_state(KEY_M);
    if (mState == KEY_PRESSED && prevM == KEY_RELEASED) {
        printf("Starting MIDI sequence...\n");
        process_midi(g, midi_data, get_midi_data_size());
    }
    prevM = mState;

    // Keys 0-9 pluck individual guitar strings
    for (int i = 0; i < 10; i++) {
        enum KeyNames key = (enum KeyNames)(KEY_0 + i);
        if (sys_get_key_state(key) == KEY_PRESSED && previousStates[i] == KEY_RELEASED) {
            int stringIndex = note_strings[i];
            printf("Playing note %d (string %d)\n", i, stringIndex);
            guitar_play_string(g, stringIndex);
        }
        previousStates[i] = sys_get_key_state(key);
    }
    guitar_cleanup_finished_sounds(g);
}

void printDeviceLimits(VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    VkPhysicalDeviceLimits limits = properties.limits;

    LOG_INFO("Device Limits:");
    LOG_INFO("  Max Image Dimension 2D: %u", limits.maxImageDimension2D);
    LOG_INFO("  Max Storage Buffer Range: %llu", limits.maxStorageBufferRange);
    LOG_INFO("  Max Uniform Buffer Range: %llu", limits.maxUniformBufferRange);
    LOG_INFO("  Max Push Constants Size: %u", limits.maxPushConstantsSize);
    LOG_INFO("  Max Sampler Anisotropy: %f", limits.maxSamplerAnisotropy);
    LOG_INFO("  Max Memory Allocation Count: %u", limits.maxMemoryAllocationCount);
}

void printResourceUsage(VulkanContext* ctx) {
    LOG_INFO("Resource Usage:");
    LOG_INFO("  Staging Buffer Size: %llu", ctx->stagingBuffer.allocated_size);
    LOG_INFO("  Total Buffers: %u", ctx->totalBuffers);
    LOG_INFO("  Total Images: %u", ctx->totalImages);
    LOG_INFO("  Total Pipelines: %u", ctx->totalPipelines);
    LOG_INFO("  Total Memory Allocations: %u", ctx->totalAllocations);
    LOG_INFO("  Total Bytes Allocated: %llu", ctx->totalBytesAllocated);
}

// -----------------------------------------------------------------------------
// Setup
// -----------------------------------------------------------------------------

void setup_texture() {
    for (int i = 0; i < maximum_images; i++) {
        pixelBuffers[i].pixels = load_image_pixels(
        filepaths[i], &pixelBuffers[i].width, &pixelBuffers[i].height);
        textureArray[i] = create_texture(
        &context, pixelBuffers[i].pixels,
        pixelBuffers[i].width, pixelBuffers[i].height,
        VK_FORMAT_R8G8B8A8_SRGB);
    }

    create_empty_texture(&context, &emptyTexture, 800, 800, VK_FORMAT_R8G8B8A8_SRGB);

    int atlasWidth, atlasHeight;
    atlasPixels = dynamic_batch_build_atlas(
        pixelBuffers, maximum_images, &atlasWidth, &atlasHeight, atlas);
    atlasTexture = create_texture(
        &context, atlasPixels, atlasWidth, atlasHeight, VK_FORMAT_R8G8B8A8_SRGB);

    font_atlas_build(&fontAtlas, "../../fonts/Ubuntu/Ubuntu-Regular.ttf", 24, &context);
}

void setup_pipelines() {
    create_graphics_pipeline(
        &standard_pipeline,
        "../../shaders/compiled/standard.vert.spv",
        "../../shaders/compiled/standard.frag.spv",
        VERTEX_FORMAT_POS_COLOR_TEX_NORM,
        VK_CULL_MODE_NONE,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_COMPARE_OP_LESS);
}

void setup_vertices() {
    // Scratch buffers for dynamic & static batches
    dynamic_batch_vertices = (Vertex*)calloc(MAX_SHAPE_VERTS, sizeof(Vertex));
    dynamic_batch_indices  = (uint16_t*)calloc(MAX_SHAPE_INDICES, sizeof(uint16_t));
    static_batch_vertices  = (Vertex*)calloc(MAX_SHAPE_VERTS, sizeof(Vertex));
    static_batch_indices   = (uint16_t*)calloc(MAX_SHAPE_INDICES, sizeof(uint16_t));

    // Font batch
    fontVerts   = (Vertex*)calloc(MAX_SHAPE_VERTS, sizeof(Vertex));
    fontIndices = (uint16_t*)calloc(MAX_SHAPE_INDICES, sizeof(uint16_t));
    font_batch_init(&context, &fontBatchObj, &fontAtlas, &standard_pipeline,
                    fontVerts, fontIndices, MAX_SHAPE_VERTS, MAX_SHAPE_INDICES);

    // Load OBJ model
    load_model(&viking_room_vertices, &viking_room_vertexCount,
               &viking_room_indices,  &viking_room_indexCount,
               "../../assets/viking_room.obj");

    // [0] Instanced polygon batch – uses the shared primitiveVertices / indices
    primitives3D[0].Obj = create_graphics_object(
        &context, &standard_pipeline, textureArray[0],
        primitiveVertices, primitiveIndices,
        sizeof(primitiveVertices) / sizeof(primitiveVertices[0]),
        sizeof(primitiveIndices) / sizeof(primitiveIndices[0]));


        

    // [1] Simple quad that will receive CPU-drawn content
    primitives3D[1].Obj = create_graphics_object(
        &context, &standard_pipeline, emptyTexture,
        vertices_rectangle, indices_rectangle, 4, 6);

    clearTexture(primitives3D[1].Obj, 155, 155, 155, 255);
            vk_update_texture_fast(&context, &primitives3D[1].Obj->texture,
                                   VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_VIEW_TYPE_2D);

    // [2] Dynamic batch (vertices filled at runtime)
    primitives3D[2].Obj = create_graphics_object(
        &context, &standard_pipeline, atlasTexture,
        dynamic_batch_vertices, dynamic_batch_indices,
        MAX_SHAPE_VERTS, MAX_SHAPE_INDICES);

    // [3] Static model batch (viking room baked once)
    primitives3D[3].Obj = create_graphics_object(
        &context, &standard_pipeline, atlasTexture,
        static_batch_vertices, static_batch_indices,
        MAX_SHAPE_VERTS, MAX_SHAPE_INDICES);

    struct mat4 world = createMat4(1.0f);
    world = applyTranslation(world, (struct Vec3){-0.5f, 0.5f, 0.0f});
    world = applyScaling(world, (struct Vec3){0.5f, 0.5f, 0.5f});

    static_batch_init_atlas(atlas, maximum_images);
    static_batch_append(primitives3D[3].Obj,
                        viking_room_vertices, viking_room_vertexCount,
                        viking_room_indices,  viking_room_indexCount,
                        world, 6);
    static_batch_upload(primitives3D[3].Obj);
}

void setup() {
    srand((unsigned)time(NULL));
    init_graphics(WIDTH, HEIGHT);
    setup_texture();
    setup_pipelines();
    setup_vertices();
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

int main() {
    LOG_INFO("Vulkan Engine Testbed starting...");

    setup();

    // Random colours / scales and a simple 2-D grid layout
    const int numColors = (int)(sizeof(palette) / sizeof(palette[0]));
    const int gridCols = 5, gridRows = 4;
    const float cellW = 2.0f / gridCols;
    const float cellH = 2.0f / gridRows;

    for (size_t i = 0; i < num3DPrimitives; i++) {
        int c = rand() % numColors;
        primitives3D[i].color = (Vec4){palette[c].r, palette[c].g, palette[c].b, 1.0f};

        float s = 0.1f + (float)rand() / RAND_MAX * 0.2f;
        primitives3D[i].scale = (Vec3){s, s, s};

        int col = (int)i % gridCols;
        int row = (int)i / gridCols;
        primitives3D[i].pos = (Vec3){
            -1.0f + (col + 0.5f) * cellW,
            -1.0f + (row + 0.5f) * cellH,
            0.0f
        };
    }

    loadFont("../../fonts/Ubuntu/Ubuntu-Regular.ttf", 16);
    set_rendering_mode(&context, is3D);

    // Audio (MIDI sequence starts when you press M)
    init_sound();
    guitar_setup(&guitar);

    // Compute demo
    ComputeObject computeObj;
    create_compute_Object(
        &computeObj, "../../shaders/compiled/test2.comp.spv",
        sizeof(StorageBufferObject), sizeof(StorageBufferObject),
        4, 4, 1);

    printResourceUsage(&context);
    printDeviceLimits(context.physicalDevice);

    sys_start_time();
    while (sys_process_events()) {
        uint32_t imageIndex = vk_acquire_next_image();
        if (imageIndex == UINT32_MAX) continue;

        processInput();
        processInput2(&guitar);

        begin_frame(imageIndex);
        vk_set_viewport(0.0f, 0.0f,
                        (float)vk_get_width(), (float)vk_get_height(),
                        0.0f, 1.0f);

        set_rendering_mode(&context, true);

        // ---------------------------------------------------------------------
        // 1. Instanced polygon batch (triangle, rect, pentagon … circle)
        // ---------------------------------------------------------------------
        {
            GraphicsObject* obj = primitives3D[0].Obj;
            obj->ubo.model = createMat4(1.0f);
            obj->ubo.model = applyScaling(obj->ubo.model, primitives3D[0].scale);
            obj->ubo.model = applyTranslation(obj->ubo.model, primitives3D[0].pos);
            obj->pushConstants.color   = primitives3D[0].color;
            obj->pushConstants.offset  = (Vec4){0.0f, 0.0f, 0.0f, 0.0f};
            obj->pushConstants.padding = (Vec4){2.0f, 0.0f, 0.0f, 0.0f};

            graphics_bind(obj);

            // Hardware instance offsets
            Vec4* offsets = (Vec4*)obj->instanceBuffer2.mapped;
            for (int f = 0; f < 10; f++)
                offsets[f] = (Vec4){-0.9f + f * 0.2f, 0.0f, 0.0f, 0.0f};

            for (int f = 0; f < PRIMITIVE_COUNT; f++) {
                DrawRange range = primitiveRanges[f];
                float currentX = -0.9f + f * 0.2f + xPos;
                graphics_draw_range(obj, 4,
                                    range.firstIndex, range.indexCount,
                                    currentX, yPos);
            }
        }

        // ---------------------------------------------------------------------
        // 2. CPU-side procedural texture (one-shot after a short delay)
        // ---------------------------------------------------------------------
        if (!updatedToQuad && x > 1.0f) {
            GraphicsObject* obj = primitives3D[1].Obj;
            clearTexture(obj, 155, 155, 155, 255);

            setColor(obj, 0, 255, 255, 255);
            rect(obj, 10, 10, obj->texture.width / 4, obj->texture.height / 4);

            setColor(obj, 255, 255, 0, 255);
            line(obj, 10, 10, obj->texture.width - 1, obj->texture.height - 1);

            setColor(obj, 255, 0, 0, 255);
            circle(obj, obj->texture.width / 2, obj->texture.height / 2, 10);

            setColor(obj, 0, 255, 0, 255);
            polygon(obj, 6, obj->texture.width * 3 / 4, obj->texture.height / 4, 10, 0, 1);

            setColor(obj, 0, 0, 0, 255);
            text(obj, "CPU Primitives", 10, obj->texture.height / 2, 1.0f, 1.2f);

            vk_update_texture_fast(&context, &obj->texture,
                                   VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_VIEW_TYPE_2D);
            updatedToQuad = true;
        }
        x += 0.01f;

        {
            GraphicsObject* obj = primitives3D[1].Obj;
            obj->ubo.model = createMat4(1.0f);
            obj->ubo.model = applyScaling(obj->ubo.model,
                (Vec3){primitives3D[1].scale.x * 4,
                       primitives3D[1].scale.y * 4,
                       primitives3D[1].scale.z * 4});
            obj->ubo.model = applyTranslation(obj->ubo.model, primitives3D[1].pos);
            obj->pushConstants.color   = primitives3D[1].color;
            obj->pushConstants.offset  = (Vec4){0.0f, 0.0f, 0.0f, 0.0f};
            obj->pushConstants.padding = (Vec4){2.0f, 0.0f, 0.0f, 0.0f};

            graphics_bind(obj);
            graphics_draw_range(obj, 1, 0, obj->indexCount, 0.0f, 0.0f);
        }

        // ---------------------------------------------------------------------
        // 3. Dynamic batch (runtime geometry)
        // ---------------------------------------------------------------------
        {
            GraphicsObject* obj = primitives3D[2].Obj;

            Vec2 verts[4] = {
                {-0.5f, -0.5f}, {0.5f, -0.5f},
                { 0.5f,  0.5f}, {-0.5f,  0.5f}
            };
            dynamic_batch_render(obj, (Vec2){0.7f, 0.0f},
                                 0xFFFFFFFF, 0xFFFFFFFF, verts, 4, 5);
            dynamic_batch_render_cube(obj, (Vec3){0.5f, 0.5f, 0.0f}, 0.5f, 0);

            obj->ubo.model = createMat4(1.0f);
            obj->pushConstants.color   = (Vec4){1.0f, 1.0f, 1.0f, 1.0f};
            obj->pushConstants.offset  = (Vec4){0.0f, 0.0f, 0.0f, 0.0f};
            obj->pushConstants.padding = (Vec4){2.0f, 0.0f, 0.0f, 0.0f};

            Vec4* offsets = (Vec4*)obj->instanceBuffer2.mapped;
            for (int f = 0; f < 4; f++)
                offsets[f] = (Vec4){-0.9f + f * 0.2f, 0.0f, 0.0f, 0.0f};

            dynamic_batch_flush(obj, 4);
        }

        // ---------------------------------------------------------------------
        // 4. Static model batch (viking room – transforms already baked)
        // ---------------------------------------------------------------------
        {
            GraphicsObject* obj = primitives3D[3].Obj;
            obj->ubo.model = createMat4(1.0f);
            obj->ubo.model = applyTranslation(obj->ubo.model, (Vec3){-0.5f, 0.5f, 0.0f});
            obj->ubo.model = applyScaling(obj->ubo.model, (Vec3){0.5f, 0.5f, 0.5f});
            obj->pushConstants.color   = (Vec4){1.0f, 1.0f, 1.0f, 1.0f};
            obj->pushConstants.offset  = (Vec4){0.0f, 0.0f, 0.0f, 0.0f};
            obj->pushConstants.padding = (Vec4){2.0f, 0.0f, 0.0f, 0.0f};

            InstanceData* offsets = (InstanceData*)obj->instanceBuffer2.mapped;
            for (int f = 0; f < 4; f++)
                offsets[f].instanceOffsets = (Vec4){-0.9f + f * 0.2f, 0.0f, 0.0f, 0.0f};

            graphics_bind(obj);
            graphics_draw_range(obj, 4, 0, obj->indexCount, 0.0f, 0.0f);
        }

        // ---------------------------------------------------------------------
        // 5. Font rendering (screen-space)
        // ---------------------------------------------------------------------
        set_rendering_mode(&context, false);
        fontBatchObj.pushConstants.color   = (Vec4){1.0f, 1.0f, 1.0f, 1.0f};
        fontBatchObj.pushConstants.offset  = (Vec4){0.0f, 0.0f, 0.0f, 0.0f};
        fontBatchObj.pushConstants.padding = (Vec4){2.0f, 0.0f, 0.0f, 0.0f};

        font_batch_begin();
        font_batch_draw_text(&fontBatchObj, &fontAtlas,
                             "Hello, world!", -0.9f, -0.9f,
                             2.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        font_batch_flush(&fontBatchObj, &context);
        set_rendering_mode(&context, true);

        // ---------------------------------------------------------------------
        // 6. Compute shader demo (runs once, prints results)
        // ---------------------------------------------------------------------
        {
            StorageBufferObject inputData = {};
            for (int i = 0; i < 256; ++i) inputData.data[i] = (float)i;

            update_compute_input(&computeObj, &inputData, sizeof(StorageBufferObject), 0);

            begin_compute();
            ComputePushConstantData push = {.numInputs = 128};
            record_compute_dispatch(&computeObj, &push, sizeof(push));
            end_compute();
            submit_compute();

            StorageBufferObject* output = (StorageBufferObject*)get_compute_output(&computeObj);
            static bool shown = false;
            if (!shown) {
                printf("=== Compute Shader Results ===\n");
                for (int i = 0; i < 10; ++i) {
                    printf("Input[%d]=%.1f + Input[%d]=%.1f => Output[%d]=%.1f\n",
                           i, inputData.data[i],
                           i + 128, inputData.data[i + 128],
                           i, output->data[i]);
                }
                shown = true;
            }
        }

        end_frame();

        if (!screenshotTaken && sys_get_key_state(KEY_P) == KEY_PRESSED) {
            vk_take_screenshot(imageIndex, "screenshot.png");
            screenshotTaken = true;
        }
        vk_present_image(imageIndex);
    }




    vkDeviceWaitIdle(context.device);

    vk_cleanup_pipeline(context.device, &standard_pipeline.pipelineLayout, &standard_pipeline.graphicsPipeline);
    
    for(int i = 0; i < maximum_images; i++) {
       vk_destroy_texture(context.device, &textureArray[i]);
    }


    
    free(dynamic_batch_vertices);
    free(dynamic_batch_indices);
    free(static_batch_vertices);
    free(static_batch_indices);
    free(fontVerts);
    free(fontIndices);
    free(viking_room_vertices);
    free(viking_room_indices);
    
    // Destroy texture resources
    vk_destroy_texture(context.device, &atlasTexture);
    vk_destroy_texture(context.device, &emptyTexture);

    free(atlasPixels);
    for (int i = 0; i < maximum_images; i++) {
        free(pixelBuffers[i].pixels);
    }

    
    destroy_shape(&fontBatchObj);
    

    if (fontBatchObj.instanceBuffer2.handle != VK_NULL_HANDLE) {
        vkUnmapMemory(context.device, fontBatchObj.instanceBuffer2.memory);
        vkDestroyBuffer(context.device, fontBatchObj.instanceBuffer2.handle, NULL);
        vkFreeMemory(context.device, fontBatchObj.instanceBuffer2.memory, NULL);
    }

    // Destroy font atlas
    font_atlas_destroy(&fontAtlas, &context);
    
    // Clean up audio
    guitar_cleanup(&guitar);

    // Clean up all GraphicsObjects
    for (int i = 0; i < num3DPrimitives; i++) {
        if (primitives3D[i].Obj != NULL) {
            // Destroy instance buffer before destroy_shape
            if (primitives3D[i].Obj->instanceBuffer2.handle != VK_NULL_HANDLE) {
                vkUnmapMemory(context.device, primitives3D[i].Obj->instanceBuffer2.memory);
                vkDestroyBuffer(context.device, primitives3D[i].Obj->instanceBuffer2.handle, NULL);
                context.totalBuffers--;
                vkFreeMemory(context.device, primitives3D[i].Obj->instanceBuffer2.memory, NULL);
                context.totalAllocations--;
                primitives3D[i].Obj->instanceBuffer2.handle = VK_NULL_HANDLE;
                primitives3D[i].Obj->instanceBuffer2.memory = VK_NULL_HANDLE;
            }

            destroy_shape(primitives3D[i].Obj);  // Destroy Vulkan resources
            
            // If you used malloc:
            free(primitives3D[i].Obj);
            primitives3D[i].Obj = NULL;
        }
    }

    
    cleanup_compute_resources(&context, &computeObj);

    // Destroy textures to prevent validation errors on exit
    for (int i = 0; i < 7; i++) {
        if (textureArray[i].sampler != VK_NULL_HANDLE) vkDestroySampler(context.device, textureArray[i].sampler, NULL);
        if (textureArray[i].imageView != VK_NULL_HANDLE) vkDestroyImageView(context.device, textureArray[i].imageView, NULL);
        if (textureArray[i].image != VK_NULL_HANDLE) {
            vkDestroyImage(context.device, textureArray[i].image, NULL);
            context.totalImages--;
        }
        if (textureArray[i].imageMemory != VK_NULL_HANDLE) {
            vkFreeMemory(context.device, textureArray[i].imageMemory, NULL);
            context.totalAllocations--;
        }
    }

    if (atlasTexture.sampler != VK_NULL_HANDLE) vkDestroySampler(context.device, atlasTexture.sampler, NULL);
    if (atlasTexture.imageView != VK_NULL_HANDLE) vkDestroyImageView(context.device, atlasTexture.imageView, NULL);
    if (atlasTexture.image != VK_NULL_HANDLE) {
        vkDestroyImage(context.device, atlasTexture.image, NULL);
        context.totalImages--;
    }
    if (atlasTexture.imageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(context.device, atlasTexture.imageMemory, NULL);
        context.totalAllocations--;
    }

    // Destroy font atlas texture
    if (fontBatchObj.texture.sampler != VK_NULL_HANDLE) vkDestroySampler(context.device, fontBatchObj.texture.sampler, NULL);
    if (fontBatchObj.texture.imageView != VK_NULL_HANDLE) vkDestroyImageView(context.device, fontBatchObj.texture.imageView, NULL);
    if (fontBatchObj.texture.image != VK_NULL_HANDLE) {
        vkDestroyImage(context.device, fontBatchObj.texture.image, NULL);
        context.totalImages--;
    }
    if (fontBatchObj.texture.imageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(context.device, fontBatchObj.texture.imageMemory, NULL);
        context.totalAllocations--;
    }


    printResourceUsage(&context);


    cleanup_graphics();
    
    return EXIT_SUCCESS;
}
