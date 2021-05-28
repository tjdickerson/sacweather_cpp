

#include <stdio.h>
#include <stdlib.h>

#include "tjd_share.h"
#include "tjd_gl.h"
#include "tjd_font.h"

// http://nothings.org/stb/stb_truetype.h
#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h" 

// ascii 32..126
stbtt_bakedchar cdata[96];
GLuint FontAtlasTexture;

unsigned char tmp_work_bitmap[512 * 512];

// static RenderBufferData TextBufferData;

void InitFont()
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

    stbtt_BakeFontBitmap(font_buffer, 0, 32.0, tmp_work_bitmap, 512, 512, 32, 96, cdata);

    // free font_buffer here?

    glGenTextures(1, &FontAtlasTexture);
    glBindTexture(GL_TEXTURE_2D, FontAtlasTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, tmp_work_bitmap);

    // free tmp_work_bitmap?

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void LoadTextBuffer()
{

}


void RenderTextImmediate(f32 x, f32 y, char* text)
{
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);

    glBindTexture(GL_TEXTURE_2D, FontAtlasTexture);

    // does this have to be immediate mode? Could probably do buffer stuff here.
    glBegin(GL_QUADS);

    f32 xpos = x;
    f32 ypos = y;

    f32 ipw = 1.0f / 512;
    f32 iph = 1.0f / 512;

    while (*text)
    {
        if (*text >= 32 && *text < 128)
        {
            // stbtt_aligned_quad q;
            // stbtt_GetBakedQuad(cdata, 512, 512, *text - 32, &x, &y, &q, 1);

            const stbtt_bakedchar *c = cdata + *text - 32;

            f32 s0, s1, t0, t1, x0, x1, y0, y1;

            // f32 round_x = (xpos + (c->xoff / 512));
            // f32 round_y = (ypos + (c->yoff / 512));

            // x0 = round_x;
            // y0 = round_y;
            // x1 = round_x + c->x1 - c->x0;
            // y1 = round_y + c->y1 - c->y0;

            f32 adjx = xpos + (c->x0 * ipw) + (c->xoff * ipw);
            f32 adjy = ypos + (c->y0 * iph) + (c->yoff * iph);

            x0 = adjx;
            y0 = adjy;
            x1 = adjx + (c->x1 * ipw) - (c->x0 * ipw);
            y1 = adjy + (c->y1 * iph) - (c->y0 * iph);

            s0 = c->x0;
            t0 = c->y0;
            s1 = c->x1;
            t1 = c->y1;

            xpos += c->xadvance;



            glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
            
            glTexCoord2f(s0,t1); glVertex2f(x0,y0);
            glTexCoord2f(s1,t1); glVertex2f(x1,y0);
            glTexCoord2f(s1,t0); glVertex2f(x1,y1);
            glTexCoord2f(s0,t0); glVertex2f(x0,y1);

            
        }

        text += 1;
    }

    glEnd();
}