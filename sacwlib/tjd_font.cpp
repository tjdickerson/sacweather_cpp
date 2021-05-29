

#include <stdio.h>
#include <stdlib.h>

#include "sacw_api.h"
#include "tjd_share.h"
#include "tjd_gl.h"
#include "tjd_font.h"
#include "tjd_conversions.h"

// http://nothings.org/stb/stb_truetype.h
#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h" 

// ascii 32..126
stbtt_bakedchar cdata[96];
GLuint FontAtlasTexture;

#define TMP_BMP_WIDTH 512
#define TMP_BMP_HEIGHT 512
unsigned char tmp_work_bitmap[TMP_BMP_WIDTH * TMP_BMP_HEIGHT];
unsigned char tmp_flipped_bitmap[TMP_BMP_WIDTH * TMP_BMP_HEIGHT];

static GLuint TextVao;
static GLuint TextVbo;

static GLuint TextShader;
static GLint  TextShaderVertexAttribute;
static GLint  TextShaderModelAttribute;

RenderBufferData TextBufferData;

// @todo
// This is duplicated, move to a central location...

static GLchar* TextVertShaderSource();
static GLchar* TextFragShaderSource();

/////
static GLuint xCreateShader(GLenum shaderType, const GLchar* source) 
{
    LOGINF("Creating shader\n");
    GLuint shader = glCreateShader(shaderType);

    if (shader == 0)
        LOGERR("Failed to create shader...\n");
    else
        LOGINF("Shader created... Compiling...\n");

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // ensure the shader compiled
    GLint retLen = 0;
    GLchar errorBuffer[512];
    glGetShaderInfoLog(shader, 512, &retLen, errorBuffer);

    if(retLen > 0) 
    {
        char errorMessage[250];
        sprintf(errorMessage, "Shader failed to compile: %s\n", errorBuffer);
        LOGERR("%s", errorMessage);

        return -1;
    }

    LOGINF("%d shader compiled.\n", shader);

    return shader;
}


GLuint xGLCreateShaderProgram(const GLchar* vertexSource, const GLchar* fragSource) 
{    
    GLuint vertShader = xCreateShader(GL_VERTEX_SHADER, vertexSource);
    if (vertShader < 0) return -1;    

    GLuint fragShader = xCreateShader(GL_FRAGMENT_SHADER, fragSource);
    if (fragShader < 0) return -1;

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    
    glLinkProgram(shaderProgram);

    LOGINF("Shader created..\n");
    return shaderProgram;
}

static GLfloat xModelMatrix[] = 
{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
};

/////

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

    glGenTextures(1, &FontAtlasTexture);
    glBindTexture(GL_TEXTURE_2D, FontAtlasTexture);
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

    TextShader = xGLCreateShaderProgram(TextVertShaderSource(), TextFragShaderSource());
    TextShaderVertexAttribute = glGetAttribLocation(TextShader, "vertex");    
    TextShaderModelAttribute = glGetUniformLocation(TextShader, "Model");

    glGenVertexArrays(1, &TextVao);
    glGenBuffers(1, &TextVbo);

    LOGINF ("Text  Vao %d, Vbo %d\n", TextVao, TextVbo);
}


void RenderText()
{
    glUseProgram(TextShader);
    glBindVertexArray(TextVao);

    /*xModelMatrix[12] = (MapViewInfo.xPan / MapViewInfo.scaleFactor) * MapViewInfo.xScale;
    xModelMatrix[13] = MapViewInfo.yPan;*/

    xModelMatrix[12] = MapViewInfo.xPan + (1.0f / (MapViewInfo.scaleFactor * MapViewInfo.xScale));
    xModelMatrix[13] = MapViewInfo.yPan + (1.0f / (MapViewInfo.scaleFactor * MapViewInfo.yScale));

    glUniformMatrix4fv(TextShaderModelAttribute, 1, GL_FALSE, xModelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, TextBufferData.vertexCount);
}


void LoadTextBuffer(f32 x, f32 y, int count, const char* text)
{
    TextBufferData = {};

    TextBufferData.vertexCount = count * 1 * 6;
    TextBufferData.vertices = (f32*)malloc(TextBufferData.vertexCount * 4 * sizeof(f32));
   

    f32 xoff = 0.0f;
    f32 glw = 1.0f / 512.0f;
    f32 tempx = 1.0f, tempy = 1.0f;
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
            f32 fy0 = TMP_BMP_HEIGHT - b->y1;
            f32 fy1 = TMP_BMP_HEIGHT - b->y0;

            botlx = tempx - (cw * 0.5f);
            botly = tempy + (ch * 0.5f);

            toplx = tempx - (cw * 0.5f);
            toply = tempy - (ch * 0.5f);

            botrx = tempx + (cw * 0.5f);
            botry = tempy + (ch * 0.5f);

            toprx = tempx + (cw * 0.5f);
            topry = tempy - (ch * 0.5f);

            tempx += b->xadvance;

            botlx *= glw;
            botly *= glw;
            toplx *= glw;
            toply *= glw;
            botrx *= glw;
            botry *= glw;
            toprx *= glw;
            topry *= glw;


            // bottom left
            TextBufferData.vertices[vi++] = x + botlx;
            TextBufferData.vertices[vi++] = y + botly;
            TextBufferData.vertices[vi++] = b->x0 * glw;
            TextBufferData.vertices[vi++] = fy1 * glw;

            // top left
            TextBufferData.vertices[vi++] = x + toplx;
            TextBufferData.vertices[vi++] = y + toply;
            TextBufferData.vertices[vi++] = b->x0 * glw;
            TextBufferData.vertices[vi++] = fy0 * glw;

            // bottom right
            TextBufferData.vertices[vi++] = x + botrx;
            TextBufferData.vertices[vi++] = y + botry;
            TextBufferData.vertices[vi++] = b->x1 * glw;
            TextBufferData.vertices[vi++] = fy1 * glw;

            // bottom right
            TextBufferData.vertices[vi++] = x + botrx;
            TextBufferData.vertices[vi++] = y + botry;
            TextBufferData.vertices[vi++] = b->x1 * glw;
            TextBufferData.vertices[vi++] = fy1 * glw;

            // top left
            TextBufferData.vertices[vi++] = x + toplx;
            TextBufferData.vertices[vi++] = y + toply;
            TextBufferData.vertices[vi++] = b->x0 * glw;
            TextBufferData.vertices[vi++] = fy0 * glw;
            
            // top right
            TextBufferData.vertices[vi++] = x + toprx;
            TextBufferData.vertices[vi++] = y + topry;
            TextBufferData.vertices[vi++] = b->x1 * glw;
            TextBufferData.vertices[vi++] = fy0 * glw;

        }
    }
/*
    for (int i = 0; i < TextBufferData.vertexCount; i += 4)
    {
        TextBufferData.vertices[i] = x + TextBufferData.vertices[i];
        TextBufferData.vertices[i + 1] = y + TextBufferData.vertices[i + 1];
    }*/


    glBindVertexArray(TextVao);
    glBindBuffer(GL_ARRAY_BUFFER, TextVbo);

    glEnableVertexAttribArray(TextShaderVertexAttribute);
    glVertexAttribPointer(
        TextShaderVertexAttribute,
        4,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(f32),
        NULL);


    glBindBuffer(GL_ARRAY_BUFFER, TextVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        TextBufferData.vertexCount * 4 * sizeof(f32),
        TextBufferData.vertices,
        GL_STATIC_DRAW);    


    if(TextBufferData.vertices) free(TextBufferData.vertices);

}


static GLchar* TextVertShaderSource() 
{
#ifdef __ANDROID__
    const char* source = R"glsl(#version 300 es
        precision mediump float;
        in vec2 Position;
        in float colorIndex;

        uniform vec4[16] colorMap;

        uniform mat4 Model;

        out vec4 FragColor;

        void main() {
            FragColor = colorMap[int(colorIndex)];
            gl_Position = Model * vec4(Position, 0.0, 1.0);
        }
    )glsl";
#else    
    const char* source = R"glsl(
        #version 150
        // layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>

        in vec4 vertex;
        out vec2 TexCoords;

        uniform mat4 Model;

        void main()
        {
            gl_Position = Model * vec4(vertex.xy, 0.0, 1.0);
            TexCoords = vertex.zw;
        }  
    )glsl";
#endif    

    return (GLchar*)source;
}

static GLchar* TextFragShaderSource() 
{
#ifdef __ANDROID__
    const char* source = R"glsl(#version 300 es
        precision mediump float;
        in vec4 FragColor;
        out vec4 outColor;

        void main() {
            outColor = FragColor;
        }
    )glsl";
#else    
    const char* source = R"glsl(
        #version 150
        in vec2 TexCoords;
        out vec4 color;

        uniform sampler2D text;
        uniform vec3 textColor;

        void main()
        {    
            vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
            // color = vec4(textColor, 1.0) * sampled;
            color = vec4(1.0, 1.0, 1.0, 1.0) * sampled;
        }  
    )glsl";
#endif    

    return (GLchar*)source;
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