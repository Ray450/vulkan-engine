// skybox_pipeline.c
// Compile shaders:
//   glslc skybox.vert -o compiled/skybox.vert.spv
//   glslc skybox.frag -o compiled/skybox.frag.spv

#include "../../graphics/primitives/primitives.h"
// #include "../../math/matrix.h"
// #include "static_polygon_batch.h"
#include "equirectangular_to_cubemap.h"

// ---------------------------------------------------------------------------
// Unit cube — full Vertex layout, positions are the cubemap sample directions
// ---------------------------------------------------------------------------
static const Vertex SKYBOX_VERTS[8] = {
    {{-1,-1,-1}, {1,1,1,1}, {0,0}, {0,0,-1}},
    {{ 1,-1,-1}, {1,1,1,1}, {0,0}, {0,0,-1}},
    {{ 1, 1,-1}, {1,1,1,1}, {0,0}, {0,0,-1}},
    {{-1, 1,-1}, {1,1,1,1}, {0,0}, {0,0,-1}},
    {{-1,-1, 1}, {1,1,1,1}, {0,0}, {0,0, 1}},
    {{ 1,-1, 1}, {1,1,1,1}, {0,0}, {0,0, 1}},
    {{ 1, 1, 1}, {1,1,1,1}, {0,0}, {0,0, 1}},
    {{-1, 1, 1}, {1,1,1,1}, {0,0}, {0,0, 1}},
};

static const uint16_t SKYBOX_INDICES[36] = {
    0,2,1, 2,0,3,   // back
    4,5,6, 6,7,4,   // front
    0,4,7, 7,3,0,   // left
    1,6,5, 6,1,2,   // right
    0,1,5, 5,4,0,   // bottom
    3,6,2, 6,3,7,   // top
};

// ---------------------------------------------------------------------------

typedef struct {
    Pipeline       pipeline;
    GraphicsObject object;
    Texture        cubemap;
} SkyboxPass;

static SkyboxPass skybox;

// ---------------------------------------------------------------------------

void skybox_init(VulkanContext* ctx, const char* equirectangularPath)
{
        create_graphics_pipeline(
        &skybox.pipeline,
        "../../shaders/compiled/skybox.vert.spv",
        "../../shaders/compiled/skybox.frag.spv",
        VERTEX_FORMAT_POS_COLOR_TEX_NORM,
        VK_CULL_MODE_FRONT_BIT,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_COMPARE_OP_LESS_OR_EQUAL
    );

    // Load equirectangular image and convert to 6 cube faces
    int starsW, starsH;
    unsigned char* starsSrc = load_image_pixels(equirectangularPath, &starsW, &starsH);

    int faceSize = starsH / 2;
    unsigned char* faces[6];
    equirectangular_to_cubemap(starsSrc, starsW, starsH, faceSize, faces);
    free(starsSrc);

    vk_create_cube_map_from_pixels(ctx, &skybox.cubemap, faces, faceSize, faceSize, VK_FORMAT_R8G8B8A8_SRGB);
    vk_create_texture_image_view(ctx, &skybox.cubemap, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_CUBE);
    vk_create_texture_sampler(ctx, &skybox.cubemap);

    for (int i = 0; i < 6; i++) free(faces[i]);

    Vertex*   dummyVerts   = (Vertex*)  calloc(MAX_SHAPE_VERTS,   sizeof(Vertex));
    uint16_t* dummyIndices = (uint16_t*)calloc(MAX_SHAPE_INDICES, sizeof(uint16_t));

    skybox.object = create_static_polygon_batch(
        ctx, &skybox.pipeline, skybox.cubemap,
        dummyVerts, dummyIndices,
        MAX_SHAPE_VERTS, MAX_SHAPE_INDICES
    );

    free(dummyVerts);
    free(dummyIndices);

    // static_batch_init_atlas(atlas, 0);
    static_batch_append(&skybox.object, (Vertex*)SKYBOX_VERTS, 8, (uint16_t*)SKYBOX_INDICES, 36, createMat4(1.0f), 0);
    static_batch_upload(&skybox.object);
}

void skybox_draw(void)
{
    skybox.object.ubo.model = createMat4(1.0f);

    skybox.object.pushConstants.color = (Vec4){1.0f, 1.0f, 1.0f, 1.0f};
    skybox.object.pushConstants.offset        = (Vec4){0.0f, 0.0f, 0.0f, 0.0f};
    skybox.object.pushConstants.padding       = (Vec4){2.0f, 0.0f, 0.0f, 0.0f};

    static_batch_draw(&skybox.object, 1);
}

void skybox_cleanup(VkDevice device)
{
    if (skybox.object.instanceBuffer2.handle != VK_NULL_HANDLE) {
        vkUnmapMemory(device, skybox.object.instanceBuffer2.memory);
        vkDestroyBuffer(device, skybox.object.instanceBuffer2.handle, NULL);
        vkFreeMemory(device, skybox.object.instanceBuffer2.memory, NULL);
    }
    destroy_shape(&skybox.object);
    vk_destroy_texture(device, &skybox.cubemap);
    vk_cleanup_pipeline(device,
                     &skybox.pipeline.pipelineLayout,
                     &skybox.pipeline.graphicsPipeline);
}