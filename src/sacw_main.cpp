//

#include "sacw_api.h"
#include "sacw_extern.h"
#include "render_extern.h"

#include "tjd_shapefile.h"
#include "tjd_conversions.h"

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



void GetMapBufferData(RenderBufferData* rbd, RenderVertData* rvd)
{
    ShapeData mapData = {};
    ReadShapeFile(&mapData, "st_us");

    rbd->vertexCount = mapData.numPoints;
    s32 arraySize = rbd->vertexCount * sizeof(f32);
    rbd->vertices = (f32*)malloc(arraySize);

    for(int i = 0; i < mapData.numPoints; i += 2)
    {
        v2f64 point = mapData.points.at(i);
        rbd->vertices[i] = ConvertLonToScreen(point.x);
        rbd->vertices[i + 1] = ConvertLatToScreen(point.y);
    }    

    rvd->numParts = mapData.numParts;
    rvd->starts = (s32*)malloc(rvd->numParts * sizeof(s32));
    rvd->counts = (s32*)malloc(rvd->numParts * sizeof(s32));

    for(int i = 0; i < rvd->numParts; i++)
    {
        rvd->starts[i] = mapData.parts.at(i);
        rvd->counts[i] = mapData.counts.at(i);
    }

}



