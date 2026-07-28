#include "texture.h"
#include <string.h>
#include <math.h>
#include "font_data.h"

//-----------------------------------------------------------------------------------------------------
//                                      Texture Primitives
//-----------------------------------------------------------------------------------------------------



int red = 255;
int green = 255;
int blue = 255;
int alpha = 255;

int current_r = 255;
int current_g = 255;
int current_b = 255;
int current_a = 255;

int width2;
int height2;

void setColor(GraphicsObject* quad, int r, int g, int b, int a) {
    red = r;
    green = g;
    blue = b;
    alpha = a;
}

void setPixel(GraphicsObject* quad, int x, int y) {
    if (x < 0 || y < 0 || x >= quad->texture.width || y >= quad->texture.height) {
        // fprintf(stderr, "setPixel: Out-of-bounds (%d, %d)\n", x, y);
        return; // skip instead of crashing
    }
    vk_set_texture_pixel(&quad->texture, x, y, red, green, blue, alpha, VK_FORMAT_R8G8B8A8_UNORM);
}

void clearTexture(GraphicsObject* quad, int r, int g, int b, int a) {
    if (!quad->texture.pixelData) {
        return;
    }
    // Assuming pixelData is a contiguous buffer of RGBA bytes (4 bytes per pixel)
    uint32_t pixelCount = quad->texture.width * quad->texture.height;
    uint8_t clearColor[4] = {(uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a};
    for (uint32_t i = 0; i < pixelCount; i++) {
        memcpy(&quad->texture.pixelData[i * 4], clearColor, 4);
    }
}

void rect(GraphicsObject* quad, float x, float y, float w, float h) {
    // Calculate rectangle bounds
    int x1 = (int)x;
    int y1 = (int)y;
    int x2 = (int)(x + w);
    int y2 = (int)(y + h);

    // Clamp rectangle bounds to within the screen boundaries
    x1 = (x1 < 0) ? 0 : (x1 >= WIDTH) ? WIDTH - 1 : x1;
    y1 = (y1 < 0) ? 0 : (y1 >= HEIGHT) ? HEIGHT - 1 : y1;
    x2 = (x2 < 0) ? 0 : (x2 >= WIDTH) ? WIDTH - 1 : x2;
    y2 = (y2 < 0) ? 0 : (y2 >= HEIGHT) ? HEIGHT - 1 : y2;

    // Draw the rectangle
    for (int i = x1; i <= x2; ++i) {
        for (int j = y1; j <= y2; ++j) {
            //if(i>780) printf("(%d, %d)\n", i, j);
            setPixel(quad, i, j);
        }
    }
    //exit(0);

    //puts("return: rect");
}

void line(GraphicsObject* quad, int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = 0;
    int sy = 0;
    int err = 0;
    if (x0 < x1)
    {
        sx = 1;
    }
    else
    {
        sx = -1;
    }
    if (y0 < y1)
    {
        sy = 1;
    }
    else
    {
        sy = -1;
    }
    if (dx > dy)
    {
        err = dx/2;
    }
    else
    {
        err = -dy/2;
    }
    while (true)
    {
        setPixel(quad, (int)x0, (int)y0);
        if (x0 == x1 && y0 == y1)
        {
            break;
        }
        float e2 = err;
        if (e2 > -dx)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void circle(GraphicsObject* quad, int x0, int y0, int radius) {
    int x = radius;
    int y = 0;
    int xChange = 1 - (radius << 1);
    int yChange = 0;
    int radiusError = 0;
    while (x >= y)
    {
        for (int i = x0 - x; i <= x0 + x; i++)
        {
            //drawPixels(renderer, centreX + x, centreY - y,1,1,color);
            setPixel(quad, i, y0 + y);
            setPixel(quad, i, y0 - y);
        }
        for (int i = x0 - y; i <= x0 + y; i++)
        {
            setPixel(quad, i, y0 + x);
            setPixel(quad, i, y0 - x);
        }
        y++;
        radiusError += yChange;
        yChange += 2;
        if (((radiusError << 1) + xChange) > 0)
        {
            x--;
            radiusError += xChange;
            xChange += 2;
        }
    }
}

void scanlineFillPolygon(GraphicsObject* quad, int sides, float centerX, float centerY, float radius, float rotation,int fill)
{
    float angle = 2 * M_PI / sides;
    float x, y, previousX, previousY;
    float minX = width2, minY = height2, maxX = 0, maxY = 0;

    for (int i = 0; i < sides; i++)
    {
        float rotatedAngle = i * angle;// + rotation;
        x = centerX + radius * cos(rotatedAngle);
        y = centerY + radius * sin(rotatedAngle);

        // Apply rotation transformation to the vertex
        float rotatedX = cos(rotation) * (x - centerX) - sin(rotation) * (y - centerY) + centerX;
        float rotatedY = sin(rotation) * (x - centerX) + cos(rotation) * (y - centerY) + centerY;

        if (i > 0)
        {
            line(quad, (int)previousX, (int)previousY, (int)rotatedX, (int)rotatedY);
        }

        // Update min/max values for bounds calculation
        if (rotatedX < minX)
            minX = rotatedX;
        if (rotatedX > maxX)
            maxX = rotatedX;
        if (rotatedY < minY)
            minY = rotatedY;
        if (rotatedY > maxY)
            maxY = rotatedY;

        previousX = rotatedX;
        previousY = rotatedY;
    }

    // Connect the last point with the first point
    line(quad, (int)previousX, (int)previousY, (int)(centerX + radius * cos(rotation)), (int)(centerY + radius * sin(rotation)));

    // Fill the polygon with the specified color
    if(fill) {
    for (int y = minY; y <= maxY; y++)
    {
        int leftX = width2;
        int rightX = 0;
        for (int x = minX; x <= maxX; x++)
        {
            if (x >= 0 && x < width2 && y >= 0 && y < height2)
            {
                    int index = (x + y * width2) * 4;
    GLubyte r2 = quad->texture.pixelData[index];
    GLubyte g2 = quad->texture.pixelData[index + 1];
    GLubyte b2 = quad->texture.pixelData[index + 2];
    GLubyte a2 = quad->texture.pixelData[index + 3];

    if (r2 == current_r && g2 == current_g && b2 == current_b && a2 == current_a)
                    {
                        if (x < leftX)
                            leftX = x;
                        if (x > rightX)
                            rightX = x;
                    }
                }
        }

        if (leftX <= rightX)
            line(quad, leftX, y, rightX, y);


        }
    }
}

void edgeIntersectionFillPolygon(GraphicsObject* quad, int sides, float centerX, float centerY, float radius, float rotation, int fill)
{
    //centerX += offsetX;
    //centerY += offsetY;

    float angle = 2 * M_PI / sides;
    float x, y, previousX, previousY;
    float minX = width2, minY = height2, maxX = 0, maxY = 0;

    for (int i = 0; i < sides; i++)
    {
        float rotatedAngle = i * angle;// + rotation;
        x = centerX + radius * cos(rotatedAngle);
        y = centerY + radius * sin(rotatedAngle);

        // Apply rotation transformation to the vertex
        float rotatedX = cos(rotation) * (x - centerX) - sin(rotation) * (y - centerY) + centerX;
        float rotatedY = sin(rotation) * (x - centerX) + cos(rotation) * (y - centerY) + centerY;

        if (i > 0)
        {
            line(quad, (int)previousX, (int)previousY, (int)rotatedX, (int)rotatedY);
        }

        // Update min/max values for bounds calculation
        if (rotatedX < minX)
            minX = rotatedX;
        if (rotatedX > maxX)
            maxX = rotatedX;
        if (rotatedY < minY)
            minY = rotatedY;
        if (rotatedY > maxY)
            maxY = rotatedY;

        previousX = rotatedX;
        previousY = rotatedY;
    }

    // Connect the last point with the first point
    line(quad, (int)previousX, (int)previousY, (int)(centerX + radius * cos(rotation)), (int)(centerY + radius * sin(rotation)));

    // Fill the polygon with the specified color
    if (fill) {
        int scanlineY;
        float* intersections = (float*)malloc((height2 * 2) * sizeof(float));  // Dynamic array to store intersection points

        // Initialize the intersection points array with maximum and minimum X values
        for (int i = 0; i < height2 * 2; i++)
        {
            intersections[i] = width2;
        }

        // Scan each scanline within the polygon bounds
        for (scanlineY = minY; scanlineY <= maxY; scanlineY++)
        {
            int numIntersections = 0;

            // Find the intersection points of the scanline with the polygon edges
            for (int i = 0; i < sides; i++)
            {
                float rotatedAngle = i * angle;// + rotation;
                x = centerX + radius * cos(rotatedAngle);
                y = centerY + radius * sin(rotatedAngle);

                // Apply rotation transformation to the vertex
                float rotatedX = cos(rotation) * (x - centerX) - sin(rotation) * (y - centerY) + centerX;
                float rotatedY = sin(rotation) * (x - centerX) + cos(rotation) * (y - centerY) + centerY;

                float nextRotatedAngle = ((i + 1) % sides) * angle;// + rotation;
                float nextX = centerX + radius * cos(nextRotatedAngle);
                float nextY = centerY + radius * sin(nextRotatedAngle);

                // Apply rotation transformation to the next vertex
                float nextRotatedX = cos(rotation) * (nextX - centerX) - sin(rotation) * (nextY - centerY) + centerX;
                float nextRotatedY = sin(rotation) * (nextX - centerX) + cos(rotation) * (nextY - centerY) + centerY;

                if ((rotatedY <= scanlineY && nextRotatedY > scanlineY) || (rotatedY > scanlineY && nextRotatedY <= scanlineY))
                {
                    float intersectionX = rotatedX + (scanlineY - rotatedY) / (nextRotatedY - rotatedY) * (nextRotatedX - rotatedX);
                    intersections[numIntersections++] = intersectionX;
                }
            }

            // Sort the intersection points in ascending order
            for (int i = 0; i < numIntersections - 1; i++)
            {
                for (int j = i + 1; j < numIntersections; j++)
                {
                    if (intersections[i] > intersections[j])
                    {
                        float temp = intersections[i];
                        intersections[i] = intersections[j];
                        intersections[j] = temp;
                    }
                }
            }

            // Fill the pixels between the intersection points
            for (int i = 0; i < numIntersections - 1; i += 2)
            {
                int leftX = (int)intersections[i];
                int rightX = (int)intersections[i + 1];

                if (leftX <= rightX)
                    line(quad, leftX, scanlineY, rightX, scanlineY);
            }
        }

        free(intersections);  // Free the dynamically allocated memory
    }
}

void polygon(GraphicsObject* quad, int sides, float centerX, float centerY, float radius, float rotation, int fill) {
    edgeIntersectionFillPolygon(quad, sides, centerX, centerY, radius, rotation, fill);
}

// Render a character using the font map
void printCharacter(char c) {
    const int* grid = fontMap[(unsigned char)c];
    if (grid == NULL) {
        // Handle unsupported characters
        return;
    }

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            //putchar((grid[i] & (1 << (7 - j))) ? '1' : '0');
            putchar((grid[i] & (1 << (7 - j))) ? '#' : ' ');
        }
        putchar('\n');
    }
}

void renderCharacter(GraphicsObject* quad, char c, float x, float y, float scale) {
    const int* grid = fontMap[(unsigned char)c];
    if (grid == NULL) {
        // Handle unsupported characters
        return;
    }
rect(quad, x, y, scale, scale);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (grid[i] & (1 << (7 - j))) {
                // If the pixel is set (1), draw a scaled rectangle at (x + j * scale, y + i * scale)
                rect(quad,x + j * scale, y + i * scale, scale, scale);
            }
        }
    }
}/**/


// Global font cache
typedef struct {
    unsigned char* bitmap;
    int width, height, xoff, yoff;
    bool loaded;
} CachedGlyph;
 
CachedGlyph fontCache[256];  // One slot for each ASCII character
 
// Load entire font once
void loadFont(const char* fontPath, int fontSize) {
    for (int i = 0; i < 256; i++) {
        fontCache[i].bitmap = load_font_bitmap(
            fontPath, (char)i, fontSize, 
            &fontCache[i].width, &fontCache[i].height,
            &fontCache[i].xoff, &fontCache[i].yoff);
        fontCache[i].loaded = (fontCache[i].bitmap != NULL);
    }
}

void renderCharacter2(GraphicsObject* quad, char c, float x, float y, float scale) {
    int index = (unsigned char)c;
    if (!fontCache[index].loaded) return;
    
    unsigned char* bmp = fontCache[index].bitmap;
    int w = fontCache[index].width;
    int h = fontCache[index].height;
    int xoff = fontCache[index].xoff;  // Add these
    int yoff = fontCache[index].yoff;  // Add these
    
    // Draw each pixel of the font bitmap
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            unsigned char pixel = bmp[i * w + j];
            if (pixel > 128) {  // If pixel is visible
                rect(quad, x + (j + xoff) * scale, y + (i + yoff) * scale, scale, scale);
            }
        }
    }
    
    // Remove this line: free(bmp);  // Don't free cached data!
}

void text(GraphicsObject* quad, const char* text, float x, float y, float scale, float spacing) {
    if (text == NULL) {
        // Handle NULL string
        return;
    }
    
    // initFontMap();

    
    int len = strlen(text);
    for (int i = 0; i < len; i++) {
        renderCharacter2(quad, text[i], x + i * (8 * scale + spacing), y, scale);
    }
}
