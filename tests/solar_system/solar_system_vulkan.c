// solar_system_vulkan.c
// Physics + rendering for a 7-body solar system in 3D.
// Saturn is rendered as a sphere + a separate flat ring disc.
//
// Atlas slot order (must match load order in main.c):
//   0 = Sun
//   1 = Mercury
//   2 = Venus
//   3 = Earth
//   4 = Mars
//   5 = Jupiter
//   6 = Saturn (planet)
//   7 = Saturn ring

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

// ---- constants -------------------------------------------------------------

#define SS_NUM_BODIES     9
#define SS_G              6.67e-11
#define SS_DT             25000.0
#define SS_STEPS_FRAME    1

#define SS_SPHERE_STACKS  16
#define SS_SPHERE_SLICES  16
#define SS_RING_SLICES    64   // smoothness of the ring disc

// Body indices
#define SS_SUN      0
#define SS_MERCURY  1
#define SS_VENUS    2
#define SS_EARTH    3
#define SS_MARS     4
#define SS_JUPITER  5
#define SS_SATURN   6
#define SS_URANUS   7
#define SS_NEPTUNE  8

// On-screen radius per body in NDC units (artistic)
static const float SS_DRAW_RADIUS[SS_NUM_BODIES] = {
    0.09f,   // Sun
    0.02f,   // Mercury
    0.03f,   // Venus
    0.03f,   // Earth
    0.025f,  // Mars
    0.055f,  // Jupiter
    0.045f,  // Saturn
    0.035f,  // Uranus
    0.035f,  // Neptune
};

// Saturn ring inner/outer radius relative to the planet radius
#define SS_RING_INNER  1.3f
#define SS_RING_OUTER  2.4f

// Atlas texture slot per body (planet sphere)
static const uint32_t SS_TEXTURE_ID[SS_NUM_BODIES] = {
    0,  // Sun
    1,  // Mercury
    2,  // Venus
    3,  // Earth
    4,  // Mars
    5,  // Jupiter
    6,  // Saturn
    7,  // Uranus
    8,  // Neptune
};
#define SS_RING_TEXTURE_ID  9   // Saturn ring atlas slot

// ---- mesh types ------------------------------------------------------------

typedef struct {
    Vertex*   vertices;
    uint16_t* indices;
    int       vertexCount;
    int       indexCount;
} Mesh;

static Mesh g_spheres[SS_NUM_BODIES];
static Mesh g_saturnRing;

// ---- sphere generation -----------------------------------------------------

static Mesh sphere_generate(float radius)
{
    int stacks = SS_SPHERE_STACKS;
    int slices = SS_SPHERE_SLICES;

    int vertexCount = (stacks + 1) * (slices + 1);
    int indexCount  = stacks * slices * 6;

    Vertex*   verts   = (Vertex*)  malloc(vertexCount * sizeof(Vertex));
    uint16_t* indices = (uint16_t*)malloc(indexCount  * sizeof(uint16_t));

    int v = 0;
    for (int stack = 0; stack <= stacks; stack++)
    {
        float phi = (float)stack / stacks * (float)PI;
        for (int slice = 0; slice <= slices; slice++)
        {
            float theta = (float)slice / slices * 2.0f * (float)PI;

            float x = sinf(phi) * cosf(theta);
            float y = cosf(phi);
            float z = sinf(phi) * sinf(theta);

            verts[v].pos      = (Vec3){ x * radius, y * radius, z * radius };
            verts[v].color    = (Vec4){ 1.0f, 1.0f, 1.0f, 1.0f };
            verts[v].texCoord = (Vec2){ (float)slice / slices, (float)stack / stacks };
            verts[v].normal   = (Vec3){ x, y, z };
            v++;
        }
    }

    int idx = 0;
    for (int stack = 0; stack < stacks; stack++)
    {
        for (int slice = 0; slice < slices; slice++)
        {
            uint16_t tl = (uint16_t)( stack      * (slices + 1) + slice    );
            uint16_t tr = (uint16_t)( stack      * (slices + 1) + slice + 1);
            uint16_t bl = (uint16_t)((stack + 1) * (slices + 1) + slice    );
            uint16_t br = (uint16_t)((stack + 1) * (slices + 1) + slice + 1);

            indices[idx++] = tl;
            indices[idx++] = bl;
            indices[idx++] = tr;
            indices[idx++] = tr;
            indices[idx++] = bl;
            indices[idx++] = br;
        }
    }

    return (Mesh){ verts, indices, vertexCount, indexCount };
}

// ---- ring generation -------------------------------------------------------
// Flat disc (XZ plane) with inner and outer radius.
// The ring texture is mapped radially: u=0 at inner edge, u=1 at outer edge.

static Mesh ring_generate(float innerRadius, float outerRadius)
{
    int slices = SS_RING_SLICES;

    // 2 verts per slice (inner + outer) = 2*(slices+1) verts
    int vertexCount = 2 * (slices + 1);
    int indexCount  = slices * 6;

    Vertex*   verts   = (Vertex*)  malloc(vertexCount * sizeof(Vertex));
    uint16_t* indices = (uint16_t*)malloc(indexCount  * sizeof(uint16_t));

    for (int i = 0; i <= slices; i++)
    {
        float angle = (float)i / slices * 2.0f * (float)PI;
        float cx = cosf(angle);
        float cz = sinf(angle);
        float u  = (float)i / slices;

        // inner vertex
        verts[i * 2].pos      = (Vec3){ cx * innerRadius, 0.0f, cz * innerRadius };
        verts[i * 2].color    = (Vec4){ 1.0f, 1.0f, 1.0f, 1.0f };
        verts[i * 2].texCoord = (Vec2){ u, 1.0f };   // v=1 → inner edge of ring texture
        verts[i * 2].normal   = (Vec3){ 0.0f, 1.0f, 0.0f };

        // outer vertex
        verts[i * 2 + 1].pos      = (Vec3){ cx * outerRadius, 0.0f, cz * outerRadius };
        verts[i * 2 + 1].color    = (Vec4){ 1.0f, 1.0f, 1.0f, 1.0f };
        verts[i * 2 + 1].texCoord = (Vec2){ u, 0.0f };   // v=0 → outer edge of ring texture
        verts[i * 2 + 1].normal   = (Vec3){ 0.0f, 1.0f, 0.0f };
    }

    int idx = 0;
    for (int i = 0; i < slices; i++)
    {
        uint16_t i0 = (uint16_t)(i * 2);
        uint16_t i1 = (uint16_t)(i * 2 + 1);
        uint16_t i2 = (uint16_t)((i + 1) * 2);
        uint16_t i3 = (uint16_t)((i + 1) * 2 + 1);

        indices[idx++] = i0;
        indices[idx++] = i2;
        indices[idx++] = i1;
        indices[idx++] = i1;
        indices[idx++] = i2;
        indices[idx++] = i3;
    }

    return (Mesh){ verts, indices, vertexCount, indexCount };
}

// ---- simulation state ------------------------------------------------------

typedef struct {
    double xPos[SS_NUM_BODIES];
    double yPos[SS_NUM_BODIES];
    double xVel[SS_NUM_BODIES];
    double yVel[SS_NUM_BODIES];
    double mass[SS_NUM_BODIES];
    double viewRadius;
    double zoomScale;
} SolarSystemState;

static SolarSystemState g_ss;

// ---- public API ------------------------------------------------------------

void solar_system_init(void)
{
    g_ss.viewRadius = 6.00e+12;   // Needs to be larger than Neptune's 4.5e+12 orbit
    
    g_ss.viewRadius = 1.5e+11;

    
    g_ss.zoomScale  = 1.0;        // Uniform camera zoom, applied in solar_system_draw()

    g_ss.xPos[SS_SUN]     = 0.0000e+00;
    g_ss.xPos[SS_MERCURY] = 5.7900e+10;
    g_ss.xPos[SS_VENUS]   = 1.0820e+11;
    g_ss.xPos[SS_EARTH]   = 1.4960e+11;
    g_ss.xPos[SS_MARS]    = 2.2790e+11;
    g_ss.xPos[SS_JUPITER] = 7.7830e+11;
    g_ss.xPos[SS_SATURN]  = 1.4270e+12;
    g_ss.xPos[SS_URANUS]  = 2.8710e+12;
    g_ss.xPos[SS_NEPTUNE] = 4.5000e+12;

    for (int i = 0; i < SS_NUM_BODIES; i++)
        g_ss.yPos[i] = 0.0;

    g_ss.yVel[SS_SUN]     = 0.0;
    g_ss.yVel[SS_MERCURY] = 4.7900e+04;
    g_ss.yVel[SS_VENUS]   = 3.5000e+04;
    g_ss.yVel[SS_EARTH]   = 2.9800e+04;
    g_ss.yVel[SS_MARS]    = 2.4100e+04;
    g_ss.yVel[SS_JUPITER] = 1.3100e+04;
    g_ss.yVel[SS_SATURN]  = 9.6900e+03;
    g_ss.yVel[SS_URANUS]  = 6.8100e+03;
    g_ss.yVel[SS_NEPTUNE] = 5.4300e+03;

    for (int i = 0; i < SS_NUM_BODIES; i++)
        g_ss.xVel[i] = 0.0;

    g_ss.mass[SS_SUN]     = 1.9890e+30;
    g_ss.mass[SS_MERCURY] = 3.3020e+23;
    g_ss.mass[SS_VENUS]   = 4.8690e+24;
    g_ss.mass[SS_EARTH]   = 5.9740e+24;
    g_ss.mass[SS_MARS]    = 6.4190e+23;
    g_ss.mass[SS_JUPITER] = 1.8980e+27;
    g_ss.mass[SS_SATURN]  = 5.6800e+26;
    g_ss.mass[SS_URANUS]  = 8.6800e+25;
    g_ss.mass[SS_NEPTUNE] = 1.0200e+26;

    // Generate sphere meshes
    for (int i = 0; i < SS_NUM_BODIES; i++)
        g_spheres[i] = sphere_generate(SS_DRAW_RADIUS[i]);

    // Generate Saturn ring: inner/outer radius relative to Saturn's sphere radius
    float rPlanet = SS_DRAW_RADIUS[SS_SATURN];
    g_saturnRing = ring_generate(rPlanet * SS_RING_INNER, rPlanet * SS_RING_OUTER);
}

void solar_system_update(void)
{
    for (int step = 0; step < SS_STEPS_FRAME; step++)
    {
        for (int i = 1; i < SS_NUM_BODIES; i++)
        {
            double dx = g_ss.xPos[0] - g_ss.xPos[i];
            double dy = g_ss.yPos[0] - g_ss.yPos[i];
            double r  = sqrt(dx*dx + dy*dy);
            if (r < 1.0) continue;

            double F  = (SS_G * g_ss.mass[i] * g_ss.mass[0]) / (r * r);
            double ax = (F * dx / r) / g_ss.mass[i];
            double ay = (F * dy / r) / g_ss.mass[i];

            g_ss.xVel[i] += SS_DT * ax;
            g_ss.yVel[i] += SS_DT * ay;
            g_ss.xPos[i] += SS_DT * g_ss.xVel[i];
            g_ss.yPos[i] += SS_DT * g_ss.yVel[i];
        }
    }
}

void solar_system_draw(GraphicsObject* batch, float camOffsetX, float camOffsetY)
{
    for (int i = 0; i < SS_NUM_BODIES; i++)
    {
        float ndcX = (float)(g_ss.xPos[i] / g_ss.viewRadius) - camOffsetX;
        float ndcY = (float)(g_ss.yPos[i] / g_ss.viewRadius) - camOffsetY;

        // Draw planet sphere
        dynamic_batch_render_mesh(
            batch,
            (Vec3){ ndcX, ndcY, 0.0f },
            0xFFFFFFFF,
            g_spheres[i].vertices,
            g_spheres[i].vertexCount,
            g_spheres[i].indices,
            g_spheres[i].indexCount,
            SS_TEXTURE_ID[i]
        );

        // Draw Saturn ring on top of Saturn's sphere
        if (i == SS_SATURN)
        {
            dynamic_batch_render_mesh(
                batch,
                (Vec3){ ndcX, ndcY, 0.0f },
                0xFFFFFFFF,
                g_saturnRing.vertices,
                g_saturnRing.vertexCount,
                g_saturnRing.indices,
                g_saturnRing.indexCount,
                SS_RING_TEXTURE_ID
            );
        }
    }

    float zoom = (float)g_ss.zoomScale;
    batch->ubo.model                   = applyScaling(createMat4(1.0f), (struct Vec3){zoom, zoom, zoom});
    batch->pushConstants.color = (Vec4){1.0f, 1.0f, 1.0f, 1.0f};
    batch->pushConstants.offset        = (Vec4){0.0f, 0.0f, 0.0f, 0.0f};
    batch->pushConstants.padding       = (Vec4){2.0f, 0.0f, 0.0f, 0.0f};

    dynamic_batch_flush(batch, 1);
}

void solar_system_zoom(float factor)
{
    g_ss.zoomScale *= (double)factor;
}

void solar_system_radius(float factor)
{
    g_ss.viewRadius *= (double)factor;
    // printf("viewRadius: %f\n", g_ss.viewRadius);
}

void solar_system_cleanup(void)
{
    for (int i = 0; i < SS_NUM_BODIES; i++)
    {
        free(g_spheres[i].vertices);
        free(g_spheres[i].indices);
        g_spheres[i].vertices = NULL;
        g_spheres[i].indices  = NULL;
    }
    free(g_saturnRing.vertices);
    free(g_saturnRing.indices);
    g_saturnRing.vertices = NULL;
    g_saturnRing.indices  = NULL;
}