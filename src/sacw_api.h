//

#ifndef _SACW_API_H_


#include "tjd_share.h"

typedef struct RenderBufferData_t
{
    f32* vertices;
    s32  vertexCount;
} RenderBufferData;

void sacw_Init();

void sacw_MainLoop();

void sacw_Cleanup();

void GetMapBufferData(RenderBufferData* rbd);


#define _SACW_API_H_
#endif
