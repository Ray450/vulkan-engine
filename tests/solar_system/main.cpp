#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "../../graphics/primitives/primitives.h"
#include "solar_system_vulkan.c"
#include "skybox_pipeline.c"
#include "equirectangular_to_cubemap.h"


// ---- camera pan (arrow keys) -----------------------------------------------
static float xPos2 = 0.0f;
static float yPos2 = 0.0f;

// ---- pipeline --------------------------------------------------------------
Pipeline standard_pipeline;

// ---- input -----------------------------------------------------------------
void processInput(void)
{
    if (sys_get_key_state(KEY_ARROW_UP)    == KEY_PRESSED) yPos2 += 0.01f;
    if (sys_get_key_state(KEY_ARROW_DOWN)  == KEY_PRESSED) yPos2 -= 0.01f;
    if (sys_get_key_state(KEY_ARROW_LEFT)  == KEY_PRESSED) xPos2 -= 0.01f;
    if (sys_get_key_state(KEY_ARROW_RIGHT) == KEY_PRESSED) xPos2 += 0.01f;

    if (sys_get_key_state(KEY_Q) == KEY_PRESSED) solar_system_zoom(0.98f);
    if (sys_get_key_state(KEY_E) == KEY_PRESSED) solar_system_zoom(1.02f);
    if (sys_get_key_state(KEY_A) == KEY_PRESSED) solar_system_radius(0.98f);
    if (sys_get_key_state(KEY_D) == KEY_PRESSED) solar_system_radius(1.02f);
}

// ---- main ------------------------------------------------------------------
int main(void)
{
    init_graphics(WIDTH, HEIGHT);

        create_graphics_pipeline(
        &standard_pipeline,
        "../../shaders/compiled/standard.vert.spv",
        "../../shaders/compiled/standard.frag.spv",
        VERTEX_FORMAT_POS_COLOR_TEX_NORM,
        VK_CULL_MODE_NONE,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_COMPARE_OP_LESS
    );

    // ---- build texture atlas -----------------------------------------------
    // Slots 0-6: planet surfaces. Slot 7: Saturn ring.
    // Order must match SS_TEXTURE_ID[] and SS_RING_TEXTURE_ID in solar_system_vulkan.c
    const char* texturePaths[] = {
        "../../assets/2k_sun.jpg",               // 0  Sun
        "../../assets/2k_mercury.jpg",           // 1  Mercury
        "../../assets/2k_venus_surface.jpg",     // 2  Venus
        "../../assets/2k_earth_daymap.jpg",      // 3  Earth
        "../../assets/2k_mars.jpg",              // 4  Mars
        "../../assets/2k_jupiter.jpg",           // 5  Jupiter
        "../../assets/2k_saturn.jpg",            // 6  Saturn
        "../../assets/2k_uranus.jpg",            // 7  Uranus
        "../../assets/2k_neptune.jpg",           // 8  Neptune
        "../../assets/2k_saturn_ring_alpha.png", // 9  Saturn ring
    };

    const char* texturePaths2[] = {
        "../../assets/2k_stars.jpg",               // 0  Sun
    };

    
    int texCount = SS_NUM_BODIES;

    PixelBuffer pixelBuffers[SS_NUM_BODIES];
    for (int i = 0; i < texCount; i++) {
        pixelBuffers[i].pixels = NULL;
        pixelBuffers[i].width  = 0;
        pixelBuffers[i].height = 0;
        pixelBuffers[i].pixels = load_image_pixels(
            texturePaths[i],
            &pixelBuffers[i].width,
            &pixelBuffers[i].height
        );
    }

    int atlasWidth, atlasHeight;
    unsigned char* atlasPixels = dynamic_batch_build_atlas(
        pixelBuffers, texCount, &atlasWidth, &atlasHeight, atlas
    );

    Texture atlasTexture = create_texture(
        &context, atlasPixels, atlasWidth, atlasHeight, VK_FORMAT_R8G8B8A8_SRGB
    );

    // ---- dynamic batch -----------------------------------------------------
    Vertex*   batchVerts   = (Vertex*)  calloc(MAX_SHAPE_VERTS,   sizeof(Vertex));
    uint16_t* batchIndices = (uint16_t*)calloc(MAX_SHAPE_INDICES, sizeof(uint16_t));

    GraphicsObject batch = dynamic_batch_object_create(
        &context, &standard_pipeline, atlasTexture,
        batchVerts, batchIndices,
        MAX_SHAPE_VERTS, MAX_SHAPE_INDICES
    );

    // ---- init solar system -------------------------------------------------
    solar_system_init();




    

    // ---- init skybox -------------------------------------------------------
    skybox_init(&context, "../../assets/2k_stars.jpg");
    set_rendering_mode(&context, true);   // 3D perspective

    sys_start_time();
    while (sys_process_events())
    {
        uint32_t imageIndex = vk_acquire_next_image();
        if (imageIndex == UINT32_MAX) continue;

        processInput();

        begin_frame(imageIndex);
        vk_set_viewport(0.0f, 0.0f, (float)get_graphics_width(), (float)get_graphics_height(), 0.0f, 1.0f);

        solar_system_update();
        skybox_draw();
        solar_system_draw(&batch, xPos2, yPos2);

        end_frame();
        vk_present_image(imageIndex);
    }

    // ---- cleanup -----------------------------------------------------------
    vkDeviceWaitIdle(context.device);

    solar_system_cleanup();

    if (batch.instanceBuffer2.handle != VK_NULL_HANDLE) {
        vkUnmapMemory(context.device, batch.instanceBuffer2.memory);
        vkDestroyBuffer(context.device, batch.instanceBuffer2.handle, NULL);
        vkFreeMemory(context.device, batch.instanceBuffer2.memory, NULL);
    }

    destroy_shape(&batch);
    free(batchVerts);
    free(batchIndices);

    

    vk_destroy_texture(context.device, &atlasTexture);
    free(atlasPixels);
    for (int i = 0; i < texCount; i++)
        free(pixelBuffers[i].pixels);

    skybox_cleanup(context.device);

    vk_cleanup_pipeline(context.device,
                     &standard_pipeline.pipelineLayout,
                     &standard_pipeline.graphicsPipeline);
    cleanup_graphics();

    return EXIT_SUCCESS;
}