//

#include "tjd_gl.h"
#include "tjd_ui.h"
#include "tjd_font.h"
#include "tjd_conversions.h"
#include "render_extern.h"
#include "sacw_api.h"
#include "sacw_extern.h"
#include <cstdio>
#include <cstdlib>

#ifndef __ANDROID__
/*PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLMULTIDRAWARRAYSPROC glMultiDrawArrays;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLUNIFORM4FVPROC glUniform4fv;
PFNGLACTIVETEXTUREPROC glActiveTexture;*/
#endif

static f32 ReflectivityMap[] =
    {
        0.20f, 0.20f, 0.20f, 0.20f,   // 0 - black transparent
        ColorHexToFloat(0x04), ColorHexToFloat(0xe9), ColorHexToFloat(0xe7), 0.20f,
        ColorHexToFloat(0x01), ColorHexToFloat(0x9f), ColorHexToFloat(0xf4), 0.90f,
        ColorHexToFloat(0x03), ColorHexToFloat(0x00), ColorHexToFloat(0xf4), 0.99f,
        ColorHexToFloat(0x02), ColorHexToFloat(0xfd), ColorHexToFloat(0x02), 0.99f,
        ColorHexToFloat(0x01), ColorHexToFloat(0xc5), ColorHexToFloat(0x01), 0.99f,
        ColorHexToFloat(0x00), ColorHexToFloat(0x8e), ColorHexToFloat(0x00), 0.99f,
        ColorHexToFloat(0xfd), ColorHexToFloat(0xf8), ColorHexToFloat(0x02), 0.99f,
        ColorHexToFloat(0xe5), ColorHexToFloat(0xbc), ColorHexToFloat(0x00), 0.99f,
        ColorHexToFloat(0xfd), ColorHexToFloat(0x95), ColorHexToFloat(0.00), 0.99f,
        ColorHexToFloat(0xfd), ColorHexToFloat(0.00), ColorHexToFloat(0.00), 0.99f,
        ColorHexToFloat(0xd4), ColorHexToFloat(0.00), ColorHexToFloat(0.00), 0.99f,
        ColorHexToFloat(0xbc), ColorHexToFloat(0.00), ColorHexToFloat(0.00), 0.99f,
        ColorHexToFloat(0xf8), ColorHexToFloat(0x00), ColorHexToFloat(0xfd), 0.99f,
        ColorHexToFloat(0x98), ColorHexToFloat(0x54), ColorHexToFloat(0xc6), 0.99f,
        ColorHexToFloat(0xff), ColorHexToFloat(0xff), ColorHexToFloat(0xff), 0.99f,
    };

static f32 VelocityMap[] =
    {
         0.20f, 0.20f, 0.20f, 0.20f,   // 0 - black transparent
         ColorHexToFloat(0x90), ColorHexToFloat(0x00), ColorHexToFloat(0xa0), 0.99f ,

         // #1ebdbb
         ColorHexToFloat(0x1e), ColorHexToFloat(0xbd), ColorHexToFloat(0xbb), 0.99f ,

         // #7dffac
         ColorHexToFloat(0x7d), ColorHexToFloat(0xff), ColorHexToFloat(0xac), 0.99f ,

         // #26e628
         ColorHexToFloat(0x26), ColorHexToFloat(0xe6), ColorHexToFloat(0x28), 0.99f ,

         // #1cb81f
         ColorHexToFloat(0x1c), ColorHexToFloat(0xb8), ColorHexToFloat(0x1f), 0.99f ,

         // #149617
         ColorHexToFloat(0x14), ColorHexToFloat(0x96), ColorHexToFloat(0x17), 0.99f ,

         // #368a37
         ColorHexToFloat(0x36), ColorHexToFloat(0x8a), ColorHexToFloat(0x37), 0.99f ,

         // #a3b2a4
         ColorHexToFloat(0xa3), ColorHexToFloat(0xb2), ColorHexToFloat(0xa4), 0.99f ,

         // #854458
         ColorHexToFloat(0x85), ColorHexToFloat(0x44), ColorHexToFloat(0x58), 0.99f ,

         // #8e0008
         ColorHexToFloat(0x8e), ColorHexToFloat(0x00), ColorHexToFloat(0.08), 0.99f ,

         // #b7000e
         ColorHexToFloat(0xb7), ColorHexToFloat(0x00), ColorHexToFloat(0x0e), 0.99f ,

         // #e40014
         ColorHexToFloat(0xe4), ColorHexToFloat(0x00), ColorHexToFloat(0x14), 0.99f ,

         // #fc4522
         ColorHexToFloat(0xfc), ColorHexToFloat(0x45), ColorHexToFloat(0x22), 0.99f ,

         // #f0ef31
         ColorHexToFloat(0xf0), ColorHexToFloat(0xef), ColorHexToFloat(0x31), 0.99f ,

         // #e9e995
         ColorHexToFloat(0xe9), ColorHexToFloat(0xe9), ColorHexToFloat(0x95), 0.99f ,
         
    };

void InitGLExtensions();
static GLuint CreateShader(GLenum shaderType, GLchar* source);
GLuint GLCreateShaderProgram(const GLchar* vertexSource, const GLchar* fragSource);

static GLchar* MapFragShaderSource();
static GLchar* MapVertShaderSource();
static GLchar* RadarFragShaderSource();
static GLchar* RadarVertShaderSource();
static GLchar* TextVertShaderSource();
static GLchar* TextFragShaderSource();
static GLchar* UiVertShaderSource();
static GLchar* UiFragShaderSource();
static GLchar* ObjVertShaderSource();
static GLchar* ObjFragShaderSource();

static GLuint RadarVao;
static GLuint RadarVbo;
static GLuint RadarShader;
static GLint g_radarVertexAttr;
static GLint g_radarColorIdxAttr;
static GLint g_radarColorMap;
static GLint g_radarTransPos;
static GLint g_radarScalePos;
static GLint g_radarRotPos;

static UiRenderInfo g_UiRenderInfo = {};

static GLuint FontAtlasTexture;

static GLfloat g_transMatrix[] =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

static GLfloat g_scaleMatrix[] =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

static GLfloat g_rotMatrix[] =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

static GLfloat g_identMatrix[] =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

static v4f32 g_clearColor = {
    //0x1d2025
    ColorHexToFloat(0x0d),
    ColorHexToFloat(0x10),
    ColorHexToFloat(0x15),
    1.0f
};

static RenderVertData StateVertData;
static RenderVertData CountyVertData;

static RenderBufferData RadarBufferData;

void logGLString(const char* text, GLenum key)
{
    const char* out = (const char*)glGetString(key);
    LOGINF("GL %s :: %s\n", text, out);
}

void adjScaleMatrix(f32 x, f32 y)
{
    g_scaleMatrix[0] = x;
    g_scaleMatrix[5] = y;
}

void adjTranslationMatrix(f32 x, f32 y)
{
    g_transMatrix[12] = x;
    g_transMatrix[13] = y;
}

void useWorldSpace()
{
    f32 x_scale = (g_MapViewInfo.scaleFactor / g_MapViewInfo.xScale);
    f32 y_scale = (g_MapViewInfo.scaleFactor / g_MapViewInfo.yScale);

    f32 trans_x = g_MapViewInfo.xPan;
    f32 trans_y = g_MapViewInfo.yPan;

    adjScaleMatrix(x_scale, y_scale);
    adjTranslationMatrix(trans_x, trans_y);
}

void useScreenSpace()
{
    glViewport(0, 0, g_MapViewInfo.mapWidthPixels, g_MapViewInfo.mapHeightPixels);

    f32 x_scale = 1.0f / g_MapViewInfo.xScale;
    f32 y_scale = 1.0f / g_MapViewInfo.yScale;

    adjScaleMatrix(1.0f / g_MapViewInfo.mapWidthPixels, -1.0f / g_MapViewInfo.mapHeightPixels);

    adjTranslationMatrix(
        (2.0f / g_MapViewInfo.mapWidthPixels) - g_MapViewInfo.mapWidthPixels,
        (2.0f / g_MapViewInfo.mapHeightPixels) - g_MapViewInfo.mapHeightPixels
    );
}

bool UiBufferInit(UiRenderInfo* renderInfo)
{
    GLuint shader = GLCreateShaderProgram(UiVertShaderSource(), UiFragShaderSource());
    renderInfo->shader = shader;

    glGenVertexArrays(1, &renderInfo->vao);
    glBindVertexArray(renderInfo->vao);

    f32 thing[] = {
        // title bar
        -1.0f, 1.0f, 0.3f, 0.3f, 0.35f, 1.0f,
        -1.0f, 0.95f, 0.3f, 0.3f, 0.35f, 1.0f,
        1.0f, 1.0f, 0.3f, 0.3f, 0.35f, 1.0f,

        1.0f, 1.0f, 0.3f, 0.3f, 0.35f, 1.0f,
        -1.0f, 0.95f, 0.3f, 0.3f, 0.35f, 1.0f,
        1.0f, 0.95f, 0.3f, 0.3f, 0.35f, 1.0f,

        // close button?
        0.0f, 0.9, 1.0f, 0.0f, 1.0f, 1.0f,
        0.9f, 0.96f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.96f, 0.99f, 0.0f, 0.0f, 1.0f, 1.0f,

        0.96f, 0.99f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.9f, 0.96f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.96f, 0.96f, 1.0f, 0.0f, 0.0f, 1.0f,

        // box
        0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        100.0f, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,

        100.0f, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        100.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,

    };

    glGenBuffers(1, &renderInfo->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderInfo->vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(thing),
        thing,
        GL_STATIC_DRAW
    );

    renderInfo->transUniLoc = glGetUniformLocation(renderInfo->shader, "translation");
    renderInfo->rotUniLoc = glGetUniformLocation(renderInfo->shader, "rotation");
    renderInfo->scaleUniLoc = glGetUniformLocation(renderInfo->shader, "scale");

    renderInfo->vertexAttribute = glGetAttribLocation(renderInfo->shader, "Position");
    glEnableVertexAttribArray(renderInfo->vertexAttribute);
    glVertexAttribPointer(
        renderInfo->vertexAttribute,
        2,
        GL_FLOAT,
        GL_FALSE,
        6 * (sizeof(f32)),
        nullptr
    );

    renderInfo->colorAttribute = glGetAttribLocation(renderInfo->shader, "Color");
    glEnableVertexAttribArray(renderInfo->colorAttribute);
    glVertexAttribPointer(
        renderInfo->colorAttribute,
        4,
        GL_FLOAT,
        GL_FALSE,
        6 * (sizeof(f32)),
        (void*)(2 * (sizeof(f32)))
    );

    return true;
}

bool GeoTextRenderInit(GeoTextRenderInfo* renderInfo)
{
    GLuint shader = GLCreateShaderProgram(TextVertShaderSource(), TextFragShaderSource());
    renderInfo->shader = shader;
    renderInfo->vertexAttribute = glGetAttribLocation(shader, "vertex");
    renderInfo->offsetUniLoc = glGetUniformLocation(shader, "offset");
    renderInfo->colorUniLoc = glGetUniformLocation(shader, "color");
    renderInfo->transUniLoc = glGetUniformLocation(shader, "translation");
    renderInfo->rotUniLoc = glGetUniformLocation(shader, "rotation");
    renderInfo->scaleUniLoc = glGetUniformLocation(shader, "scale");
    glGenVertexArrays(1, &renderInfo->vao);
    glGenBuffers(1, &renderInfo->vbo);

    //
    u32 featureWithTextCount = 0;
    for (int i = 0; i < g_MapViewInfo.shapeFileCount; i++)
    {
        ShapeFileInfo* shapeFile = &g_MapViewInfo.renderInfo[i].shapeFile;
        featureWithTextCount += shapeFile->featuresHaveText ? shapeFile->numFeatures : 0;
    }

    renderInfo->markerCount = featureWithTextCount + g_RdaSiteInfo.count;
    renderInfo->markers = (GeoTextMarker*)calloc(renderInfo->markerCount, sizeof(GeoTextMarker));

    int idx = 0;

    // shape files
    for (int i = 0; i < g_MapViewInfo.shapeFileCount; i++)
    {
        ShapeFileInfo* shapeFile = &g_MapViewInfo.renderInfo[i].shapeFile;
        if (shapeFile->featuresHaveText)
        {
            for (int j = 0; j < shapeFile->numFeatures; j++)
            {
                ShapeData* feature = &shapeFile->features[j];
                GeoTextMarker* marker = &renderInfo->markers[idx++];

                marker->position = feature->featureName.location;
                marker->textLength = feature->featureName.textLength;
                marker->scale = 0.75f;

                marker->color = {
                    ColorHexToFloat(0xa4),
                    ColorHexToFloat(0xb6),
                    ColorHexToFloat(0xb5),
                    1.0f
                };

                memcpy(marker->text, feature->featureName.text, feature->featureName.textLength);
            }
        }
    }

    // rda sites
    for (int i = 0; i < g_RdaSiteInfo.count; i++)
    {
        RdaSite* site = &g_RdaSiteInfo.sites[i];
        GeoTextMarker* marker = &renderInfo->markers[idx++];

        marker->position = v2f32 { site->location.lon, site->location.lat };
        marker->textLength = 4;
        marker->color = {
            ColorHexToFloat(0x34),
            ColorHexToFloat(0xa6),
            ColorHexToFloat(0xc5),
            1.0f
        };

        marker->scale = 0.4f;

        strcpy(marker->text, site->name);
    }

    return true;
}

bool GeoTextLoadBuffer()
{
    RenderBufferData bufferData = {};
    GeoTextRenderInfo* renderInfo = &g_GeoTextRenderInfo;

    CreateGeoTextVertices(renderInfo, &bufferData);

    glBindVertexArray(renderInfo->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderInfo->vbo);

    glEnableVertexAttribArray(renderInfo->vertexAttribute);
    glVertexAttribPointer(
        renderInfo->vertexAttribute,
        4,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(f32),
        nullptr
    );

    glBindBuffer(GL_ARRAY_BUFFER, renderInfo->vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        bufferData.vertexCount * 4 * sizeof(f32),
        bufferData.vertices,
        GL_STATIC_DRAW
    );

    if (bufferData.vertices) free(bufferData.vertices);

    return true;
}

bool ShapeRenderInit(ShapeRenderInfo* renderInfo)
{
    GLuint shader = GLCreateShaderProgram(MapVertShaderSource(), MapFragShaderSource());
    renderInfo->shader = shader;
    renderInfo->vertexAttribute = glGetAttribLocation(shader, "vertex");
    renderInfo->colorUniLoc = glGetUniformLocation(shader, "color");
    renderInfo->transUniLoc = glGetUniformLocation(shader, "translation");
    renderInfo->rotUniLoc = glGetUniformLocation(shader, "rotation");
    renderInfo->scaleUniLoc = glGetUniformLocation(shader, "scale");
    glGenVertexArrays(1, &renderInfo->vao);
    glGenBuffers(1, &renderInfo->vbo);

    return true;
}

bool ShapeRenderLoadBuffer(ShapeRenderInfo* renderInfo)
{
    RenderBufferData renderData = {};

    ShapeFileInfo* shapeFile = &renderInfo->shapeFile;

    renderData.vertexCount = shapeFile->totalNumPoints;
    renderData.vertices = (f32*)malloc(renderData.vertexCount * 2 * sizeof(f32));

    GenerateShapeBufferData(shapeFile, &renderData);

    free(shapeFile->points);
    shapeFile->points = nullptr;
    shapeFile->pointsInitted = false;

    glBindVertexArray(renderInfo->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderInfo->vbo);

    glEnableVertexAttribArray(renderInfo->vertexAttribute);
    glVertexAttribPointer(
        renderInfo->vertexAttribute,
        2,
        GL_FLOAT,
        GL_FALSE,
        2 * (sizeof(f32)),
        nullptr
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        renderData.vertexCount * 2L * (s64)sizeof(f32),
        renderData.vertices,
        GL_STATIC_DRAW
    );

    // cleanup
    if (renderData.vertices) free(renderData.vertices);

    return true;
}

bool LoadMapBufferData()
{
    for (int i = 0; i < g_MapViewInfo.shapeFileCount; i++)
    {
        ShapeRenderInfo* renderInfo = &g_MapViewInfo.renderInfo[i];
        ShapeRenderLoadBuffer(renderInfo);
    }

    return true;
}

void MapInit()
{
    glLineWidth(2.0f);
    for (int i = 0; i < g_MapViewInfo.shapeFileCount; i++)
    {
        ShapeRenderInfo* renderInfo = &g_MapViewInfo.renderInfo[i];
        ShapeRenderInit(renderInfo);
    }
}

bool LoadLatestRadarData()
{
    // @todo
    // Reset these or something to free memory
    RadarBufferData = {};
    sacw_GetRadarRenderData(&RadarBufferData);

    glBindVertexArray(RadarVao);
    glBindBuffer(GL_ARRAY_BUFFER, RadarVbo);

    glEnableVertexAttribArray(g_radarVertexAttr);
    glVertexAttribPointer(
        g_radarVertexAttr,
        2,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(f32),
        NULL
    );

    glEnableVertexAttribArray(g_radarColorIdxAttr);
    glVertexAttribPointer(
        g_radarColorIdxAttr,
        1,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(f32),
        (void*)(2 * sizeof(f32)));

    glBindBuffer(GL_ARRAY_BUFFER, RadarVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        RadarBufferData.vertexCount * 3 * sizeof(f32),
        RadarBufferData.vertices,
        GL_STATIC_DRAW
    );

    if (RadarBufferData.vertices) free(RadarBufferData.vertices);

    return true;
}

bool RenderInit(void* window)
{
    LOGINF("GL Init\n");

#ifndef __ANDRDOID__

    int err = gladLoadGL();
    LOGINF("Loaded GL error code %d\n", err);

    InitGui(window);

#endif

    logGLString("Version", GL_VERSION);
    logGLString("Vendor", GL_VENDOR);
    logGLString("Renderer", GL_RENDERER);
    // logGLString("Extensions", GL_EXTENSIONS);

    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    MapInit();
    GeoTextRenderInit(&g_GeoTextRenderInfo);

    RadarShader = GLCreateShaderProgram(RadarVertShaderSource(), RadarFragShaderSource());
    g_radarVertexAttr = glGetAttribLocation(RadarShader, "position");
    g_radarColorIdxAttr = glGetAttribLocation(RadarShader, "colorIndex");
    g_radarColorMap = glGetUniformLocation(RadarShader, "colorMap");
    g_radarTransPos = glGetUniformLocation(RadarShader, "translation");
    g_radarRotPos = glGetUniformLocation(RadarShader, "rotation");
    g_radarScalePos = glGetUniformLocation(RadarShader, "scale");

    glGenVertexArrays(1, &RadarVao);
    glGenBuffers(1, &RadarVbo);

    glGenTextures(1, &FontAtlasTexture);
    InitFont(FontAtlasTexture);

    UiBufferInit(&g_UiRenderInfo);

    return true;
}

void RenderUi(UiRenderInfo* renderInfo)
{
    useScreenSpace();
    glUseProgram(renderInfo->shader);
    glBindVertexArray(renderInfo->vao);

    glUniformMatrix4fv(renderInfo->transUniLoc, 1, GL_FALSE, g_transMatrix);
    glUniformMatrix4fv(renderInfo->rotUniLoc, 1, GL_FALSE, g_rotMatrix);
    glUniformMatrix4fv(renderInfo->scaleUniLoc, 1, GL_FALSE, g_scaleMatrix);

    glDrawArrays(GL_TRIANGLES, 0, 18);
}

void renderShapeFile(ShapeRenderInfo* renderInfo)
{
    glUseProgram(renderInfo->shader);
    glBindVertexArray(renderInfo->vao);

    glUniformMatrix4fv(renderInfo->transUniLoc, 1, GL_FALSE, g_transMatrix);
    glUniformMatrix4fv(renderInfo->rotUniLoc, 1, GL_FALSE, g_rotMatrix);
    glUniformMatrix4fv(renderInfo->scaleUniLoc, 1, GL_FALSE, g_scaleMatrix);

    ShapeFileInfo* shape_file = &renderInfo->shapeFile;

    if (shape_file->showPastScale != 0 && g_MapViewInfo.scaleFactor < shape_file->showPastScale)
        return;

    glUniform4f(
        renderInfo->colorUniLoc,
        shape_file->lineColor.r,
        shape_file->lineColor.g,
        shape_file->lineColor.b,
        shape_file->lineColor.a
    );

    GLint draw_type = GL_LINE_LOOP;
    if (shape_file->type == SHAPE_TYPE_POLYLINE)
        draw_type = GL_LINE_STRIP;

    glLineWidth(shape_file->lineWidth);

    s32 idx = 0;

    for (int j = 0; j < shape_file->numFeatures; j++)
    {
        ShapeData* feature = &shape_file->features[j];
        for (int k = 0; k < feature->numParts; k++)
        {
            bool draw =
                feature->boundingBox.maxX > g_MapViewInfo.worldScreenBounds.min_x &&
                feature->boundingBox.minX < g_MapViewInfo.worldScreenBounds.max_x &&
                feature->boundingBox.maxY > g_MapViewInfo.worldScreenBounds.min_y &&
                feature->boundingBox.minY < g_MapViewInfo.worldScreenBounds.max_y;

            if (draw)
            {
                idx = feature->partsIndex + k;
                glDrawArrays(
                    draw_type,
                    shape_file->starts[idx],
                    shape_file->counts[idx]
                );
            }
        }
    }

}

void renderMap()
{
    for (int i = 0; i < g_MapViewInfo.shapeFileCount; i++)
    {
        renderShapeFile(&g_MapViewInfo.renderInfo[i]);
    }
}

void RenderRadar()
{
    glUseProgram(RadarShader);
    glBindVertexArray(RadarVao);

    glUniformMatrix4fv(g_radarTransPos, 1, GL_FALSE, g_transMatrix);
    glUniformMatrix4fv(g_radarRotPos, 1, GL_FALSE, g_rotMatrix);
    glUniformMatrix4fv(g_radarScalePos, 1, GL_FALSE, g_scaleMatrix);

    f32* colorMap = ReflectivityMap;
    if (g_CurrentProduct->productCode == 99)
    {
        colorMap = VelocityMap;
    }
    glUniform4fv(g_radarColorMap, 16, colorMap);
    glDrawArrays(GL_TRIANGLES, 0, RadarBufferData.vertexCount);
}

void renderGeoText()
{

    for (int i = 0; i < g_GeoTextRenderInfo.markerCount; i++)
    {
        GeoTextMarker* text_marker = &g_GeoTextRenderInfo.markers[i];

        f32 x = AdjustLonForProjection(text_marker->position.x);
        f32 y = AdjustLatForProjection(text_marker->position.y);

        v2f32 direction = {};
        direction.x = x > 0 ? -1.0f : 1.0f;
        direction.y = y > 0 ? 1.0f : -1.0f;

        f32 off_x = (x + g_MapViewInfo.xPan) * (g_MapViewInfo.scaleFactor * direction.x);
        f32 off_y = (y + g_MapViewInfo.yPan) * (g_MapViewInfo.scaleFactor * direction.y);

        f32 offset[2];
        offset[0] = off_x;
        offset[1] = off_y;

        glUseProgram(g_GeoTextRenderInfo.shader);
        glBindVertexArray(g_GeoTextRenderInfo.vao);

        // In order to let the text stay in a "geo-locked" position and not scale with the map,
        // the translation matrix needs to be zeroed out and the offset between the vertex coordinates
        // and the and the resulting product of the matrix multiplications needs to be passed to the shader.
        // A scale can still be applied to prevent the font from stretching after window resize, but the
        // square of the scale needs to be multiplied into the offset.
        adjScaleMatrix(1.0f / g_MapViewInfo.xScale, 1.0f / g_MapViewInfo.yScale);
        adjTranslationMatrix(0.0f, 0.0f);

        glUniformMatrix4fv(g_GeoTextRenderInfo.transUniLoc, 1, GL_FALSE, g_transMatrix);
        glUniformMatrix4fv(g_GeoTextRenderInfo.rotUniLoc, 1, GL_FALSE, g_rotMatrix);
        glUniformMatrix4fv(g_GeoTextRenderInfo.scaleUniLoc, 1, GL_FALSE, g_scaleMatrix);

        glUniform2fv(g_GeoTextRenderInfo.offsetUniLoc, 1, offset);
        glUniform4f(
            g_GeoTextRenderInfo.colorUniLoc,
            text_marker->color.r,
            text_marker->color.b,
            text_marker->color.g,
            text_marker->color.a
        );

        glDrawArrays(GL_TRIANGLES, text_marker->startIndex, text_marker->count);
    }
}

bool show_demo = true;

void renderLayers()
{
    useWorldSpace();

    if (canRenderRadar)
    {
        RenderRadar();
    }

    renderMap();
    renderGeoText();

    {
        // TestIMGUI(show_demo);
        //RenderUi(&g_UiRenderInfo);
    }

}

void Render()
{
    // 
    glClearColor(g_clearColor.x, g_clearColor.y, g_clearColor.z, g_clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, g_MapViewInfo.mapWidthPixels, g_MapViewInfo.mapHeightPixels);

    renderLayers();

    {
        BeginImGui();

        //RenderToolbar();
        InfoPanel();
        CheckEvents();

        EndImGui();
    }

    /*bool splitScreen = false;
    if (splitScreen)
    {
        ModelMatrix[0] = (g_MapViewInfo.scaleFactor * 0.5f) / g_MapViewInfo.xScale;
        ModelMatrix[12] = g_MapViewInfo.xPan * ModelMatrix[0];

        // bottom view
        {
            glViewport(
                0, 
                0, 
                g_MapViewInfo.mapWidthPixels,
                g_MapViewInfo.mapHeightPixels * 0.5f);
            
            renderLayers();
        }

        // top view
        {
            glViewport(
            0, 
            g_MapViewInfo.mapHeightPixels * 0.5f,
            g_MapViewInfo.mapWidthPixels,
            g_MapViewInfo.mapHeightPixels * 0.5f);

            renderLayers();
        }       

    }

    else
    {       
        glViewport(
                0, 
                0, 
                g_MapViewInfo.mapWidthPixels,
                g_MapViewInfo.mapHeightPixels);

        renderLayers();
    }*/


}

bool RenderCleanup()
{
    if (StateVertData.starts) free(StateVertData.starts);
    if (StateVertData.counts) free(StateVertData.counts);

    return true;
}

static GLuint CreateShader(GLenum shaderType, const GLchar* source)
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

    if (retLen > 0)
    {
        char errorMessage[250];
        sprintf(errorMessage, "Shader failed to compile: %s\n", errorBuffer);
        LOGERR("%s", errorMessage);

        return -1;
    }

    LOGINF("%d shader compiled.\n", shader);

    return shader;
}

GLuint GLCreateShaderProgram(const GLchar* vertexSource, const GLchar* fragSource)
{
    GLuint vertShader = CreateShader(GL_VERTEX_SHADER, vertexSource);
    if (vertShader < 0) return -1;

    GLuint fragShader = CreateShader(GL_FRAGMENT_SHADER, fragSource);
    if (fragShader < 0) return -1;

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);

    glLinkProgram(shaderProgram);

    LOGINF("Shader created..\n");
    return shaderProgram;
}

void RenderViewportUpdate(f32 width, f32 height)
{
    if (height == 0.0f) height = 1.0f;
    if (width == 0.0f) width = 1.0f;

    if (width > height)
    {
        g_MapViewInfo.xScale = width / height;
        g_MapViewInfo.yScale = 1.0f;
    }

    else
    {
        g_MapViewInfo.xScale = 1.0f;
        g_MapViewInfo.yScale = height / width;
    }

    g_MapViewInfo.mapWidthPixels = width;
    g_MapViewInfo.mapHeightPixels = height;

    glViewport(0, 0, width, height);
}

void InitGLExtensions()
{
#ifndef __ANDROID__

/*    glCreateShader = (PFNGLCREATESHADERPROC)GLGetProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)GLGetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)GLGetProcAddress("glCompileShader");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)GLGetProcAddress("glGetShaderInfoLog");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)GLGetProcAddress("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)GLGetProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)GLGetProcAddress("glLinkProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)GLGetProcAddress("glUseProgram");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)GLGetProcAddress("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)GLGetProcAddress("glBindVertexArray");
    glGenBuffers = (PFNGLGENBUFFERSPROC)GLGetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)GLGetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)GLGetProcAddress("glBufferData");
    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)GLGetProcAddress("glGetAttribLocation");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)GLGetProcAddress("glVertexAttribPointer");
    glMultiDrawArrays = (PFNGLMULTIDRAWARRAYSPROC)GLGetProcAddress("glMultiDrawArrays");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)GLGetProcAddress("glGetUniformLocation");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)GLGetProcAddress("glUniformMatrix4fv");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)GLGetProcAddress("glBufferSubData");
    glUniform3f = (PFNGLUNIFORM3FPROC)GLGetProcAddress("glUniform3f");
    glUniform4f = (PFNGLUNIFORM4FPROC)GLGetProcAddress("glUniform4f");
    glUniform4fv = (PFNGLUNIFORM4FVPROC)GLGetProcAddress("glUniform4fv");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)GLGetProcAddress("glActiveTexture");    

    glEnableVertexAttribArray = 
        (PFNGLENABLEVERTEXATTRIBARRAYPROC)GLGetProcAddress("glEnableVertexAttribArray");
    glBindFragDataLocation = 
        (PFNGLBINDFRAGDATALOCATIONPROC)GLGetProcAddress("glBindFragDataLocation");*/
#endif
}

static GLchar* MapVertShaderSource()
{
#ifdef __ANDROID__
    const char* source = R"glsl(#version 300 es
        precision mediump float;
        in vec2 vertex;

        uniform mat4 Model;

        void main() {
            gl_Position = Model * vec4(Position, 0.0, 1.0);
        }
    )glsl";
#else
    const char* source = R"glsl(
        #version 150

        in vec2 vertex;

        uniform mat4 translation;
        uniform mat4 rotation;
        uniform mat4 scale;

        void main() {

            // This calculation is backwards from what is typical, in which scale is applied first.
            // This is because the entire "map" is using this shader so the whole thing gets scaled and
            // we want the scale to happen from the translated position instead of the origin, so the 
            // translation needs to be applied before the scale.
            
            gl_Position = rotation * scale * translation * vec4(vertex, 0.0, 1.0);
        }
    )glsl";
#endif

    return (GLchar*)source;
}

static GLchar* MapFragShaderSource()
{
#ifdef __ANDROID__
    const char* source = R"glsl(#version 300 es
        precision mediump float;
        out vec4 outColor;

        uniform vec4 color;

        void main() {
            outColor = color;
        }
    )glsl";
#else
    const char* source = R"glsl(
        #version 150

        out vec4 outColor;

        uniform vec4 color;

        void main() {
            outColor = color;
        }
    )glsl";
#endif

    return (GLchar*)source;
}

static GLchar* RadarVertShaderSource()
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

        in vec2 position;
        in float colorIndex;

        uniform vec4[16] colorMap;

        uniform mat4 translation;
        uniform mat4 rotation;
        uniform mat4 scale;

        out vec4 FragColor;

        vec4 color1;
        vec4 color2;
        float blend_amount;

        void main() {
            blend_amount = colorIndex - floor(colorIndex);
            color1 = colorMap[int(floor(colorIndex))];
            color2 = colorMap[int(ceil(colorIndex))];
            FragColor = mix(color1, color2, blend_amount);
            
            gl_Position = rotation * scale * translation * vec4(position, 0.0, 1.0);
        }
    )glsl";
#endif

    return (GLchar*)source;
}

static GLchar* RadarFragShaderSource()
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

        in vec4 FragColor;
        out vec4 outColor;

        void main() {
            outColor = FragColor;
        }
    )glsl";
#endif

    return (GLchar*)source;
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
            float blend_amount = colorIndex - floor(colorIndex);
            vec4 color1 = colorMap[int(floor(colorIndex))];
            vec4 color2 = colorMap[int(ceil(colorIndex))];
            FragColor = mix(color1, color2, blend_amount);
            gl_Position = Model * vec4(Position, 0.0, 1.0);
        }
    )glsl";
#else
    const char* source = R"glsl(
        #version 150
        // layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>

        in vec4 vertex;
        out vec2 TexCoords;

        uniform vec2 offset;

        uniform mat4 translation;
        uniform mat4 rotation;
        uniform mat4 scale;      

        void main()
        {
            vec2 temp = vertex.xy + offset;
            
            gl_Position = rotation * scale * translation * vec4(temp, 0.0, 1.0);
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
        out vec4 frag_color;

        uniform sampler2D text;
        uniform vec4 color;

        void main()
        {    
            vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
            frag_color = color * sampled;
        }  
    )glsl";
#endif

    return (GLchar*)source;
}

static GLchar* UiVertShaderSource()
{
#ifdef __ANDROID__
    const char* source = R"glsl(#version 300 es
        precision mediump float;
        in vec2 Position;
        in vec4 Color;

        uniform mat4 Model;

        out vec4 frag_color;

        void main() {
            frag_color = Color;
            gl_Position = Model * vec4(Position, 0.0, 1.0);
        }
    )glsl";
#else
    const char* source = R"glsl(
        #version 150

        in vec2 Position;
        in vec4 Color;

        uniform mat4 translation;
        uniform mat4 rotation;
        uniform mat4 scale;

        out vec4 frag_color;

        void main() {

            frag_color = Color;
            gl_Position = rotation * scale * translation * vec4(Position, 0.0, 1.0);
        }
    )glsl";
#endif

    return (GLchar*)source;
}

static GLchar* UiFragShaderSource()
{
#ifdef __ANDROID__
    const char* source = R"glsl(#version 300 es
        precision mediump float;
        out vec4 outColor;

        uniform vec4 Color;

        void main() {
            outColor = Color;
        }
    )glsl";
#else
    const char* source = R"glsl(
        #version 150

        out vec4 outColor;

        in vec4 frag_color;

        void main() {
            outColor = frag_color;
        }
    )glsl";
#endif

    return (GLchar*)source;
}

static GLchar* ObjVertShaderSource()
{
    const char* source = R"glsl(
        #version 150
        in vec2 position;
        in vec4 color;

        out vec4 frag_color;
        void main() {
            frag_color = color;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )glsl";

    return (GLchar*)source;
}

static GLchar* ObjFragShaderSource()
{
    const char* source = R"glsl(
        #version 150
        in  vec4 frag_color;
        out vec4 outColor;
        void main() {
            outColor = frag_color;
        }
    )glsl";

    return (GLchar*)source;
}