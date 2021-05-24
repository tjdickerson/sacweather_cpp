//

#include "tjd_gl.h"
#include "render_extern.h"
#include "sacw_api.h"
#include "sacw_extern.h"
#include <cstdio>
#include <cstdlib>


#ifndef __ANDROID__
PFNGLCREATESHADERPROC glCreateShader;
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
PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif


static f32 ReflectivityMap[] =
{
    0.20f,  0.20f,  0.20f,  0.20f,   // 0 - black transparent
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


void InitGLExtensions();
static GLuint CreateShader(GLenum shaderType, GLchar* source);
GLuint GLCreateShaderProgram(const GLchar* vertexSource, const GLchar* fragSource);


static GLchar* MapFragShaderSource();
static GLchar* MapVertShaderSource();
static GLchar* RadarFragShaderSource();
static GLchar* RadarVertShaderSource();


static GLuint MapVao;
static GLuint MapVbo;
static GLuint MapShader;
static GLint  MapShaderPositionAttribute;
static GLint  MapShaderModelAttribute;
static GLint  MapShaderColorAttribute;

static GLuint RadarVao;
static GLuint RadarVbo;
static GLuint RadarShader;
static GLint  RadarShaderPositionAttribute;
static GLint  RadarShaderModelAttribute;
static GLint  RadarShaderColorAttribute;


static GLfloat ModelMatrix[] = 
{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
};

static GLfloat UiScaleMatrix[] = 
{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
};

static GLfloat IdentityMatrix[] = 
{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
};


static v4f32 ClearColor = {20.0f/255.0f, 20.0f/255.0f, 20.0f/255.0f, 1.0f};


static RenderVertData StateVertData;
static RenderVertData CountyVertData;

static RenderBufferData RadarBufferData;
static RenderBufferData MapBufferData;

void logGLString(const char* text, GLenum key)
{
    const char* out = (const char*) glGetString(key);
    LOGINF("GL %s :: %s\n", text, out);
}


bool MapInit()
{
    MapBufferData = {};
    StateVertData = {};
    CountyVertData = {};
    sacw_GetMapRenderData(&MapBufferData, &StateVertData, &CountyVertData);

    MapShader = GLCreateShaderProgram(MapVertShaderSource(), MapFragShaderSource());

    glGenVertexArrays(1, &MapVao);
    glBindVertexArray(MapVao);

    glGenBuffers(1, &MapVbo);
    glBindBuffer(GL_ARRAY_BUFFER, MapVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        MapBufferData.vertexCount * 2 * sizeof(f32),
        MapBufferData.vertices,
        GL_STATIC_DRAW);

    MapShaderPositionAttribute = glGetAttribLocation(MapShader, "Position");
    glEnableVertexAttribArray(MapShaderPositionAttribute);
    glVertexAttribPointer(
        MapShaderPositionAttribute,
        2,
        GL_FLOAT,
        GL_FALSE,
        2 * (sizeof(f32)),
        NULL);

    MapShaderModelAttribute = glGetUniformLocation(MapShader, "Model");
    MapShaderColorAttribute = glGetUniformLocation(MapShader, "Color");

    // cleanup
    if(MapBufferData.vertices) free(MapBufferData.vertices);

    return true;
}


bool LoadLatestRadarData()
{
    // @todo
    // Reset these or something to free memory
    RadarBufferData = {};
    sacw_GetRadarRenderData(&RadarBufferData);

    glGenVertexArrays(1, &RadarVao);
    glBindVertexArray(RadarVao);

    glGenBuffers(1, &RadarVbo);
    glBindBuffer(GL_ARRAY_BUFFER, RadarVbo);

    glEnableVertexAttribArray(RadarShaderPositionAttribute);
    glVertexAttribPointer(
        RadarShaderPositionAttribute,
        2,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(f32),
        NULL);

    glEnableVertexAttribArray(RadarShaderColorAttribute);
    glVertexAttribPointer(
        RadarShaderColorAttribute,
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
        GL_STATIC_DRAW);

    if(RadarBufferData.vertices) free(RadarBufferData.vertices);

    return true;
}

bool RenderInit()
{
    LOGINF("GL Init\n");

    #ifndef __ANDRDOID__
    InitGLExtensions();
    #endif

    logGLString("Version", GL_VERSION);
    logGLString("Vendor", GL_VENDOR);
    logGLString("Renderer", GL_RENDERER);
    // logGLString("Extensions", GL_EXTENSIONS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    MapInit();

    RadarShader = GLCreateShaderProgram(RadarVertShaderSource(), RadarFragShaderSource());
    RadarShaderPositionAttribute = glGetAttribLocation(RadarShader, "Position");
    RadarShaderColorAttribute = glGetUniformLocation(RadarShader, "colorMap");
    RadarShaderModelAttribute = glGetUniformLocation(RadarShader, "Model");

    return true;
}


void RenderStates()
{
    glUseProgram(MapShader);
    glBindVertexArray(MapVao);
    glUniformMatrix4fv(MapShaderModelAttribute, 1, GL_FALSE, ModelMatrix);

    // states  #6c6c58
    glUniform4f(MapShaderColorAttribute, 1.0f, 1.0f, 1.0f, 1.0f);
    for (int i = 0; i < StateVertData.numParts; i++)
    {
        glDrawArrays(GL_LINE_STRIP, StateVertData.starts[i], StateVertData.counts[i]);
    }
}

void RenderCounties()
{
    glUseProgram(MapShader);
    glBindVertexArray(MapVao);
    glUniformMatrix4fv(MapShaderModelAttribute, 1, GL_FALSE, ModelMatrix);


    // counties
    glUniform4f(MapShaderColorAttribute, 0.2f, 0.2f, 0.2f, 1.0f);
    for (int i = 0; i < CountyVertData.numParts; i++)
    {
        glDrawArrays(GL_LINE_STRIP, CountyVertData.starts[i], CountyVertData.counts[i]);
    }
}

void RenderRadar()
{
    glUseProgram(RadarShader);
    glBindVertexArray(RadarVao);
    glUniformMatrix4fv(RadarShaderModelAttribute, 1, GL_FALSE, ModelMatrix);

    glUniform4fv(RadarShaderColorAttribute, 16, ReflectivityMap);
    glDrawArrays(GL_TRIANGLES, 0, RadarBufferData.vertexCount);
}


void DoRender()
{
    if (canRenderRadar)
    {
        RenderRadar();
    }

    RenderCounties();
    RenderStates();
}


void Render()
{
    // 
    glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ModelMatrix[0] = (MapViewInfo.scaleFactor / MapViewInfo.xScale);
    ModelMatrix[5] = (MapViewInfo.scaleFactor / MapViewInfo.yScale);

    ModelMatrix[12] = MapViewInfo.xPan * ModelMatrix[0];
    ModelMatrix[13] = MapViewInfo.yPan * ModelMatrix[5];

    bool splitScreen = false;
    if (splitScreen)
    {
        ModelMatrix[0] = (MapViewInfo.scaleFactor * 0.5f) / MapViewInfo.xScale;
        ModelMatrix[12] = MapViewInfo.xPan * ModelMatrix[0];

        // bottom view
        {
            glViewport(
                0, 
                0, 
                MapViewInfo.mapWidthPixels, 
                MapViewInfo.mapHeightPixels * 0.5f);
            
            DoRender();
        }

        // top view
        {
            glViewport(
            0, 
            MapViewInfo.mapHeightPixels * 0.5f, 
            MapViewInfo.mapWidthPixels, 
            MapViewInfo.mapHeightPixels * 0.5f);

            DoRender();
        }       

    }

    else
    {       
        glViewport(
                0, 
                0, 
                MapViewInfo.mapWidthPixels, 
                MapViewInfo.mapHeightPixels);

        DoRender();
    }

    glUseProgram(0);
    glFlush();
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
        MapViewInfo.xScale = width / height;
        MapViewInfo.yScale = 1.0f;
    }

    else
    {
        MapViewInfo.xScale = 1.0f;
        MapViewInfo.yScale = height / width;
    }

    MapViewInfo.mapWidthPixels = width;
    MapViewInfo.mapHeightPixels = height;

    glViewport(0, 0, width, height);
}


void InitGLExtensions() 
{
#ifndef __ANDROID__    
    glCreateShader = (PFNGLCREATESHADERPROC)GLGetProcAddress("glCreateShader");
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
        (PFNGLBINDFRAGDATALOCATIONPROC)GLGetProcAddress("glBindFragDataLocation");
#endif        
}

static GLchar* MapVertShaderSource() 
{
#ifdef __ANDROID__
    const char* source = R"glsl(#version 300 es
        precision mediump float;
        in vec2 Position;

        uniform mat4 Model;

        void main() {
            gl_Position = Model * vec4(Position, 0.0, 1.0);
        }
    )glsl";
#else    
    const char* source = R"glsl(
        #version 150

        in vec2 Position;

        uniform mat4 Model;

        void main() {
            gl_Position = Model * vec4(Position, 0.0, 1.0);
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

        uniform vec4 Color;

        void main() {
            outColor = Color;
        }
    )glsl";
#else
    const char* source = R"glsl(
        #version 150

        out vec4 outColor;

        uniform vec4 Color;

        void main() {
            outColor = Color;
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

