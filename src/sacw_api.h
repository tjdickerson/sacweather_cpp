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


void sacw_Init();

void sacw_MainLoop();

void sacw_Cleanup();

void GetMapBufferData(RenderBufferData* rbd, RenderVertData* rvd);


#define _SACW_API_H_
#endif
