//

#ifndef _SACW_API_H_

#include <glad/glad.h>
#include "tjd_share.h"
#include "tjd_shapefile.h"
#include "nws_info.h"

struct RenderBufferData
{
    f32* vertices;
    s64  vertexCount;
};


typedef struct RenderVertData_t
{
    s32 numParts;
    s32* starts;
    s32* counts;
} RenderVertData;

struct MapIcon
{
    GLuint texId;
    v2f32 position;
};

struct MapIconInfo
{

};

#define GEO_TEXT_STATE      1
#define GEO_TEXT_RDA        2

struct GeoTextMarker
{
    v2f32 position;
    u32 textLength;
    char text[256];
    color4 color;

    f32 scale {1.0f};
    s32 type;

    f32 renderWidth;
    u32 startIndex;
    u32 count;
};

struct GeoTextRenderInfo
{
    GLuint vao{};
    GLuint vbo{};
    GLuint shader{};

    GLint vertexAttribute{};

    GLint transUniLoc{};
    GLint rotUniLoc{};
    GLint scaleUniLoc{};
    GLint colorUniLoc{};
    GLint offsetUniLoc{};

    u32 markerCount;
    GeoTextMarker* markers;
};

struct UiRenderInfo
{
    GLuint vao{};
    GLuint vbo{};
    GLuint shader{};

    GLint vertexAttribute{};
    GLint colorAttribute{};

    GLint transUniLoc{};
    GLint rotUniLoc{};
    GLint scaleUniLoc{};
};

struct ShapeRenderInfo
{
    GLuint vao{};
    GLuint vbo{};
    GLuint shader{};

    GLint vertexAttribute{};

    GLint transUniLoc{};
    GLint rotUniLoc{};
    GLint scaleUniLoc{};
    GLint colorUniLoc{};

    ShapeFileInfo shapeFile;
};


struct RdaSite
{
    v2f32   location;
    char    name[5];
    char    displayName[256];
};

struct RdaSiteInfo
{
    u32 count;
    RdaSite* sites;
};

struct MapViewInfo
{
    f32 mapWidthPixels{};
    f32 mapHeightPixels{};

    f32 scaleFactor{};
    f32 xPan{};
    f32 yPan{};
    f32 xScale{};
    f32 yScale{};

    v4f32 worldScreenBounds{};

    s32 shapeFileCount{};
    ShapeRenderInfo* renderInfo{};
};


extern MapViewInfo g_MapViewInfo;
extern GeoTextRenderInfo g_GeoTextRenderInfo;
extern RdaSiteInfo g_RdaSiteInfo;
extern NexradProduct* g_CurrentProduct;

bool GenerateShapeBufferData(ShapeFileInfo* shapeFileInfo, RenderBufferData* renderData);


extern bool canRenderRadar;


void sacw_Init(void* window);

bool sacw_RadarInit(const char* filename, s16 productCode);

void sacw_MainLoop();

void sacw_Cleanup();

void sacw_UpdateViewport(f32 width, f32 height);

void sacw_ZoomMap(f32 zoom);

void sacw_PanMap(f32 x, f32 y);

s64 sacw_GetScanTime();

void sacw_GetPolarFromScreen(f32 x, f32 y, f32* points);

void sacw_GetMapRenderData(RenderBufferData* rbd, RenderVertData* states, RenderVertData* counties);

void sacw_GetRadarRenderData(RenderBufferData* rbd);

v2f32 ConvertScreenToCoords(MapViewInfo* map, s32 x, s32 y);

RdaSite* FindClosestRdaFromScreen(s32 x, s32 y);
RdaSite* FindClosestRda(v2f32 coords);

#define _SACW_API_H_
#endif
