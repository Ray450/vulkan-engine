#ifndef TEXTURE_H
#define TEXTURE_H


#include "../vulkan/vulkan_graphics.h"

//-----------------------------------------------------------------------------------------------------
//                                      Texture Primitives
//-----------------------------------------------------------------------------------------------------

// GraphicsObject quad;
extern int red;
extern int green;
extern int blue;
extern int alpha;

extern int current_r;
extern int current_g;
extern int current_b;
extern int current_a;



void createQuadObject(GraphicsObject* quad, Pipeline pipeline, const char* texturePath, bool is3D, 
                      struct Vertex* customVertices, uint32_t vertexCount, 
                      uint16_t* customIndices, uint32_t indexCount);

void drawQuad(GraphicsObject* quad, float x, float y, float z, float width, float height, float r, float g, float b, float a);


void createModelObject(GraphicsObject* model, Pipeline pipeline, const char* modelPath, const char* texturePath, bool is3D);

void drawModel(GraphicsObject* model, float x, float y, float z, float scale, float r, float g, float b, float a, float rotationAngle);

void createWaveFormViewer(GraphicsObject* quad, Pipeline pipeline, const char* texturePath, bool is3D, 
                      struct Vertex* customVertices, uint32_t vertexCount, 
                      uint16_t* customIndices, uint32_t indexCount);

void drawWaveFormViewer(GraphicsObject* quad, float x, float y, float z, float width, float height, float r, float g, float b, float a);

void createScene2D(GraphicsObject* quad, Pipeline pipeline, const char* texturePath, bool is3D, 
                  struct Vertex* customVertices, uint32_t vertexCount, 
                  uint16_t* customIndices, uint32_t indexCount);

void drawScene2D(GraphicsObject* quad, float x, float y, float z, float width, float height, float r, float g, float b, float a);

void createScene3D(GraphicsObject* cube, Pipeline pipeline, const char* texturePath, bool is3D, 
                  struct Vertex* customVertices, uint32_t vertexCount, 
                  uint16_t* customIndices, uint32_t indexCount);

void drawScene3D(GraphicsObject* cube, float x, float y, float z, float sx, float sy, float sz, float r, float g, float b, float a);


void loadFont(const char* fontPath, int fontSize);


void setColor(GraphicsObject* quad, int r, int g, int b, int a);

void setPixel(GraphicsObject* quad, int x, int y);

void clearTexture(GraphicsObject* quad, int r, int g, int b, int a);

void createQuadObject2(GraphicsObject* quad, Pipeline pipeline, const char* texturePath, bool is3D,
                      struct Vertex* customVertices, uint32_t vertexCount,
                      uint16_t* customIndices, uint32_t indexCount);

void drawQuad2(GraphicsObject* quad, float x, float y, float z, float width, float height, float r, float g, float b, float a);


void rect(GraphicsObject* quad, float x, float y, float w, float h);

void line(GraphicsObject* quad, int x0, int y0, int x1, int y1);

void circle(GraphicsObject* quad, int x0, int y0, int radius);

void scanlineFillPolygon(GraphicsObject* quad, int sides, float centerX, float centerY, float radius, float rotation,int fill);

void edgeIntersectionFillPolygon(GraphicsObject* quad, int sides, float centerX, float centerY, float radius, float rotation, int fill);

void polygon(GraphicsObject* quad, int sides, float centerX, float centerY, float radius, float rotation, int fill);

// Render a character using the font map
void printCharacter(char c);
void renderCharacter(GraphicsObject* quad, char c, float x, float y, float scale);

void text(GraphicsObject* quad, const char* text, float x, float y, float scale, float spacing);


#endif // TEXTURE_H
