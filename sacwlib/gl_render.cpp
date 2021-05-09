//

#include "gl.h"
#include "render_extern.h"
#include "sacw_api.h"
#include "sacw_extern.h"
#include <cstdio>

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
PFNGLACTIVETEXTUREPROC glActiveTexture;

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
static RenderVertData RadarVertData;


bool RenderInit()
{    
    printf("Render init...\n");
    InitGLExtensions();
  
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    

    // init map    
    RenderBufferData mapBufferData = {};    
    StateVertData = {};
    CountyVertData = {};
    sacw_GetMapRenderData(&mapBufferData, &StateVertData, &CountyVertData);

    MapShader = GLCreateShaderProgram(MapVertShaderSource(), MapFragShaderSource());

    glGenVertexArrays(1, &MapVao);
    glBindVertexArray(MapVao);

    glGenBuffers(1, &MapVbo);
    glBindBuffer(GL_ARRAY_BUFFER, MapVbo);
    glBufferData(
        GL_ARRAY_BUFFER, 
        mapBufferData.vertexCount * 2 * sizeof(f32), 
        mapBufferData.vertices, 
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


    // init radar
    RenderBufferData radarBufferData = {};
    RadarVertData = {};
    sacw_GetRadarRenderData(&radarBufferData, &RadarVertData);

    RadarShader = GLCreateShaderProgram(RadarVertShaderSource(), RadarFragShaderSource());
    
    glGenVertexArrays(1, &RadarVao);
    glBindVertexArray(RadarVao);

    glGenBuffers(1, &RadarVbo);
    glBindBuffer(GL_ARRAY_BUFFER, RadarVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        radarBufferData.vertexCount * 6 * sizeof(f32),
        radarBufferData.vertices,
        GL_DYNAMIC_DRAW);

    RadarShaderPositionAttribute = glGetAttribLocation(RadarShader, "Position");
    glEnableVertexAttribArray(RadarShaderPositionAttribute);
    glVertexAttribPointer(
        RadarShaderPositionAttribute,
        2,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(f32),
        NULL);

    RadarShaderColorAttribute = glGetAttribLocation(RadarShader, "Color");
    glEnableVertexAttribArray(RadarShaderColorAttribute);
    glVertexAttribPointer(
        RadarShaderColorAttribute,
        4, 
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(f32),
        (void*)(2 * sizeof(f32)));

    RadarShaderModelAttribute = glGetUniformLocation(RadarShader, "Model");


    // cleanup
    if(mapBufferData.vertices) free(mapBufferData.vertices);   
    if(radarBufferData.vertices) free(radarBufferData.vertices);

    return true;
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

    // render map
    {
        glUseProgram(MapShader);
        glBindVertexArray(MapVao);
        glUniformMatrix4fv(MapShaderModelAttribute, 1, GL_FALSE, ModelMatrix);


        // counties
/*        glUniform4f(MapShaderColorAttribute, 0.5f, 0.5f, 0.5f, 0.5f);
        glMultiDrawArrays(
            GL_LINE_STRIP, CountyVertData.starts, CountyVertData.counts, CountyVertData.numParts);*/

        // states  #6c6c58
        glUniform4f(MapShaderColorAttribute, 1.0f, 1.0f, 1.0f, 1.0f);
        /*glMultiDrawArrays(
            GL_LINE_STRIP, StateVertData.starts, StateVertData.counts, StateVertData.numParts);*/

        for (int i = 0; i < StateVertData.numParts; i++)
        {
            //int offset = (StateVertData.numParts * i);
            glDrawArrays(GL_LINE_STRIP, StateVertData.starts[i], StateVertData.counts[i]);
        }

    }


    // radar data
    {
        glUseProgram(RadarShader);
        glBindVertexArray(RadarVao);
        glUniformMatrix4fv(RadarShaderModelAttribute, 1, GL_FALSE, ModelMatrix);

        glMultiDrawArrays(
            GL_TRIANGLE_FAN, RadarVertData.starts, RadarVertData.counts, RadarVertData.numParts);
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
    GLuint shader = glCreateShader(shaderType);

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // ensure the shader compiled
    GLint retLen = 0;
    GLchar errorBuffer[512];
    glGetShaderInfoLog(shader, 512, &retLen, errorBuffer);

    if(retLen > 0) 
    {
        char errorMessage[250];
        sprintf(errorMessage, "Shader failed to compile: %s", errorBuffer);
        ShowError(errorMessage);

        return -1;
    }

    printf("%d shader compiled.\n", shader);

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

    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);

    return shaderProgram;
}

void RenderViewportUpdate(f32 width, f32 height)
{
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
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)GLGetProcAddress("glActiveTexture");    

    glEnableVertexAttribArray = 
        (PFNGLENABLEVERTEXATTRIBARRAYPROC)GLGetProcAddress("glEnableVertexAttribArray");
    glBindFragDataLocation = 
        (PFNGLBINDFRAGDATALOCATIONPROC)GLGetProcAddress("glBindFragDataLocation");
}

static GLchar* MapVertShaderSource() 
{
    const char* source = R"glsl(
        #version 150

        in vec2 Position;

        uniform mat4 Model;

        void main() {
            gl_Position = Model * vec4(Position, 0.0, 1.0);
        }
    )glsl";

    return (GLchar*)source;
}

static GLchar* MapFragShaderSource() 
{
    const char* source = R"glsl(
        #version 150

        out vec4 outColor;

        uniform vec4 Color;

        void main() {
            outColor = Color;
        }
    )glsl";

    return (GLchar*)source;
}

static GLchar* RadarVertShaderSource() 
{
    const char* source = R"glsl(
        #version 150

        in vec2 Position;
        in vec4 Color;

        uniform mat4 Model;

        out vec4 FragColor;

        void main() {
            FragColor = Color;
            gl_Position = Model * vec4(Position, 0.0, 1.0);
        }
    )glsl";

    return (GLchar*)source;
}

static GLchar* RadarFragShaderSource() 
{
    const char* source = R"glsl(
        #version 150

        in vec4 FragColor;
        out vec4 outColor;

        void main() {
            outColor = FragColor;
        }
    )glsl";

    return (GLchar*)source;
}

