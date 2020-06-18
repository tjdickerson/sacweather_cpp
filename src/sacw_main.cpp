//

#include "sacw_api.h"
#include "sacw_extern.h"
#include "render_extern.h"

#include "tjd_shapefile.h"

void sacw_Init()
{    

    //
    RenderInit();

}


void sacw_MainLoop()
{
    Render();
}


void sacw_Cleanup()
{
    //?
    RenderCleanup();
}



void GetMapBufferData(RenderBufferData* rbd)
{
    ShapeData mapData = {};
    ReadShapeFile(&mapData, "st_us");

    rbd->vertexCount = mapData.numPoints * 2;
    s32 arraySize = rbd->vertexCount * sizeof(f32);
    rbd->vertices = (f32*)malloc(arraySize);

    for(int i = 0; i < mapData.numPoints; i += 2)
    {
        v2f64 point = mapData.points.at(i);
        rbd->vertices[i] = point.x;
        rbd->vertices[i + 1] = point.y;
    }    

}



