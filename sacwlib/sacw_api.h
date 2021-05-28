//

#ifndef _SACW_API_H_


#include "tjd_share.h"

typedef struct RenderBufferData_t
{
    f32* vertices;
    s32  vertexCount;
} RenderBufferData;


typedef struct RenderVertData_t
{
    s32 numParts;    
    s32* starts;
    s32* counts;
} RenderVertData;


typedef struct MapViewState_t
{
    f32 mapWidthPixels;
    f32 mapHeightPixels;
    f32 scaleFactor;
    f32 xPan;
    f32 yPan;
    f32 xScale;
    f32 yScale;
} MapViewState;


extern MapViewState MapViewInfo;

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

#define _SACW_API_H_
#endif
