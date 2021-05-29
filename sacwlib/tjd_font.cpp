

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


    for (int row = TMP_BMP_HEIGHT; row > 0; row--)
    {
        for (int col = 0; col < TMP_BMP_WIDTH; col++)
        {
            tmp_flipped_bitmap[col + (TMP_BMP_WIDTH * (TMP_BMP_HEIGHT - row))] = 
                tmp_work_bitmap[col + (TMP_BMP_WIDTH * row)];
        }
    }

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
        tmp_flipped_bitmap);

    // free tmp_work_bitmap?

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
}


void LoadTextBuffer(RenderBufferData* textBuffer, f32 x, f32 y, int count, const char* text)
{
    textBuffer->vertexCount = count * 1 * 6;
    textBuffer->vertices = (f32*)malloc(textBuffer->vertexCount * 4 * sizeof(f32));
   
    f32 xoff = 0.0f;
    f32 glw = 1.0f / 512.0f;
    f32 tempx = 0.0f, tempy = 0.0f;
    f32 charHeight = 32.0f;
    int vi = 0;
    for (int i = 0; i < count; i++)
    {
        const char* c = &text[i];
        if (*c >= 32 && *c < 128)
        {              
            const stbtt_bakedchar *b = cdata + *c - 32;

            
            f32 botlx, botly, toplx, toply, botrx, botry, toprx, topry;
            f32 cw = b->x1 - b->x0;
            f32 ch = b->y1 - b->y0;
            f32 fy0 = TMP_BMP_HEIGHT - b->y0;
            f32 fy1 = TMP_BMP_HEIGHT - b->y1;

            botlx = tempx - (cw * 0.5f);
            botly = tempy - (ch * 0.5f);

            toplx = tempx - (cw * 0.5f);
            toply = tempy + (ch * 0.5f);

            botrx = tempx + (cw * 0.5f);
            botry = tempy - (ch * 0.5f);

            toprx = tempx + (cw * 0.5f);
            topry = tempy + (ch * 0.5f);

            tempx += b->xadvance;

            botlx *= glw;
            botly *= glw;
            toplx *= glw;
            toply *= glw;
            botrx *= glw;
            botry *= glw;
            toprx *= glw;
            topry *= glw;

            // botlx = ScreenToX(botlx);
            // botly = ScreenToY(botly);
            // toplx = ScreenToX(toplx);
            // toply = ScreenToY(toply);
            // botrx = ScreenToX(botrx);
            // botry = ScreenToY(botry);
            // toprx = ScreenToX(toprx);
            // topry = ScreenToY(topry);

            botlx = ConvertLonToScreen(x) + botlx;
            botly = -ConvertLatToScreen(y) + botly;
            toplx = ConvertLonToScreen(x) + toplx;
            toply = -ConvertLatToScreen(y) + toply;
            botrx = ConvertLonToScreen(x) + botrx;
            botry = -ConvertLatToScreen(y) + botry;
            toprx = ConvertLonToScreen(x) + toprx;
            topry = -ConvertLatToScreen(y) + topry;


            // bottom left
            textBuffer->vertices[vi++] = botlx;
            textBuffer->vertices[vi++] = botly;
            textBuffer->vertices[vi++] = b->x0 * glw;
            textBuffer->vertices[vi++] = fy1 * glw;

            // top left
            textBuffer->vertices[vi++] = toplx;
            textBuffer->vertices[vi++] = toply;
            textBuffer->vertices[vi++] = b->x0 * glw;
            textBuffer->vertices[vi++] = fy0 * glw;

            // bottom right
            textBuffer->vertices[vi++] = botrx;
            textBuffer->vertices[vi++] = botry;
            textBuffer->vertices[vi++] = b->x1 * glw;
            textBuffer->vertices[vi++] = fy1 * glw;

            // top left
            textBuffer->vertices[vi++] = toplx;
            textBuffer->vertices[vi++] = toply;
            textBuffer->vertices[vi++] = b->x0 * glw;
            textBuffer->vertices[vi++] = fy0 * glw;

            // top right
            textBuffer->vertices[vi++] = toprx;
            textBuffer->vertices[vi++] = topry;
            textBuffer->vertices[vi++] = b->x1 * glw;
            textBuffer->vertices[vi++] = fy0 * glw;

            // bottom right
            textBuffer->vertices[vi++] = botrx;
            textBuffer->vertices[vi++] = botry;
            textBuffer->vertices[vi++] = b->x1 * glw;
            textBuffer->vertices[vi++] = fy1 * glw;           

        }
    }
}

