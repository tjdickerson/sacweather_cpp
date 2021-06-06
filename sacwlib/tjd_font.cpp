

#include <stdio.h>
#include <stdlib.h>

#include "tjd_font.h"
#include "tjd_share.h"
#include "tjd_conversions.h"

// http://nothings.org/stb/stb_truetype.h
#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h" 

// ascii 32..126
stbtt_bakedchar cdata[96];

#define TMP_BMP_WIDTH 512
#define TMP_BMP_HEIGHT 512
unsigned char tmp_work_bitmap[TMP_BMP_WIDTH * TMP_BMP_HEIGHT];
unsigned char tmp_flipped_bitmap[TMP_BMP_WIDTH * TMP_BMP_HEIGHT];


void InitFont(GLuint textureId)
{
    const char* font_location = "C:\\code\\sacweather\\resources\\fonts\\WorkSans-Regular.ttf";
    long file_length;
    unsigned char* font_buffer;

    FILE* font_file = fopen(font_location, "rb");
    if (font_file == NULL)
    {
        LOGERR("Failed to open %s\n", font_location);
        return;
    }
    
    fseek(font_file, 0, SEEK_END);
    file_length = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);

    font_buffer = (unsigned char*) malloc(file_length * sizeof(unsigned char));

    fread(font_buffer, 1, file_length, font_file);
    fclose(font_file);

    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, font_buffer, 0))
    {
        LOGERR("Failed to load font %s\n", font_location);
    }

    stbtt_BakeFontBitmap(font_buffer, 0, 32.0, tmp_work_bitmap, TMP_BMP_WIDTH, TMP_BMP_HEIGHT, 32, 96, cdata);


/*    for (int row = TMP_BMP_HEIGHT; row > 0; row--)
    {
        for (int col = 0; col < TMP_BMP_WIDTH; col++)
        {
            tmp_flipped_bitmap[col + (TMP_BMP_WIDTH * (TMP_BMP_HEIGHT - row))] = 
                tmp_work_bitmap[col + (TMP_BMP_WIDTH * row)];
        }
    }*/

    // free font_buffer here?
    
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        GL_RED, 
        TMP_BMP_WIDTH, 
        TMP_BMP_HEIGHT, 
        0, 
        GL_RED, 
        GL_UNSIGNED_BYTE, 
        tmp_work_bitmap);

    // free tmp_work_bitmap?

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
}


void LoadTextBuffer(RenderBufferData* textBuffer, f32 lon, f32 lat, f32 scale, s32 count, const char* text)
{
    textBuffer->vertexCount = count * 1 * 6;
    textBuffer->vertices = (f32*)malloc(textBuffer->vertexCount * 4 * sizeof(f32));
   
    f32 bmp_h_factor = 1.0f / TMP_BMP_HEIGHT;
    f32 bmp_w_factor = 1.0f / TMP_BMP_WIDTH;        

    f32 screen_x = 0.0f; // AdjustLonForProjection(lon);
    f32 screen_y = 0.0f; //AdjustLatForProjection(lat);

    f32 temp_x = 0.0f;
    
    f32 botlx, botly, toplx, toply, botrx, botry, toprx, topry;
    f32 c_h_half, c_w_half, fy0, fy1;

    int vi = 0;
    for (int i = 0; i < count; i++)
    {
        const char* c = &text[i];
        if (*c >= 32 && *c < 128)
        {              
            const stbtt_bakedchar *b = cdata + *c - 32;
            
            c_w_half = (b->x1 - b->x0) * 0.5f;
            c_h_half = (b->y1 - b->y0) * 0.5f;

            c_w_half *= bmp_w_factor;
            c_h_half *= bmp_h_factor;

            fy0 = b->y0;
            fy1 = b->y1;

            botlx = temp_x + (screen_x - c_w_half);
            botly = screen_y - c_h_half;

            toplx = temp_x + (screen_x - c_w_half);
            toply = screen_y + c_h_half;

            botrx = temp_x + (screen_x + c_w_half);
            botry = screen_y - c_h_half;

            toprx = temp_x + (screen_x + c_w_half);
            topry = screen_y + c_h_half;

            temp_x += (b->xadvance * bmp_w_factor);


            // bottom left
            textBuffer->vertices[vi++] = botlx;
            textBuffer->vertices[vi++] = botly;
            textBuffer->vertices[vi++] = b->x0 * bmp_w_factor;
            textBuffer->vertices[vi++] = fy1 * bmp_h_factor;

            // top left
            textBuffer->vertices[vi++] = toplx;
            textBuffer->vertices[vi++] = toply;
            textBuffer->vertices[vi++] = b->x0 * bmp_w_factor;
            textBuffer->vertices[vi++] = fy0 * bmp_h_factor;

            // bottom right
            textBuffer->vertices[vi++] = botrx;
            textBuffer->vertices[vi++] = botry;
            textBuffer->vertices[vi++] = b->x1 * bmp_w_factor;
            textBuffer->vertices[vi++] = fy1 * bmp_h_factor;

            // top left
            textBuffer->vertices[vi++] = toplx;
            textBuffer->vertices[vi++] = toply;
            textBuffer->vertices[vi++] = b->x0 * bmp_w_factor;
            textBuffer->vertices[vi++] = fy0 * bmp_h_factor;

            // top right
            textBuffer->vertices[vi++] = toprx;
            textBuffer->vertices[vi++] = topry;
            textBuffer->vertices[vi++] = b->x1 * bmp_w_factor;
            textBuffer->vertices[vi++] = fy0 * bmp_h_factor;

            // bottom right
            textBuffer->vertices[vi++] = botrx;
            textBuffer->vertices[vi++] = botry;
            textBuffer->vertices[vi++] = b->x1 * bmp_w_factor;
            textBuffer->vertices[vi++] = fy1 * bmp_h_factor;           

        }
    }
}

