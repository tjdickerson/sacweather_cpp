//

#include "sacw_api.h"
#include "sacw_extern.h"
#include "render_extern.h"

#include "tjd_shapefile.h"
#include "tjd_conversions.h"

MapViewState MapViewInfo;

void sacw_Init()
{    
    MapViewInfo = {};    
    MapViewInfo.scaleFactor = 1.0f;

    RenderInit();
}


void sacw_MainLoop()
{
    Render();
}


void sacw_Cleanup()
{
    RenderCleanup();
}


void sacw_UpdateViewport(f32 width, f32 height)
{
    RenderViewportUpdate(width, height);    
}


void sacw_ZoomMap(f32 zoom)
{
    MapViewInfo.scaleFactor += zoom * (0.1f);
}


void GetMapBufferData(RenderBufferData* rbd, RenderVertData* rvd)
{
    ShapeData mapData = {};
    ReadShapeFile(&mapData, "st_us");

    rbd->vertexCount = mapData.numPoints;
    s32 arraySize = rbd->vertexCount * 2 * sizeof(f32);
    rbd->vertices = (f32*)malloc(arraySize);

    int idx = 0;
    for(int i = 0; i < mapData.numPoints; i++)
    {
        v2f64 point = mapData.points.at(i);
        rbd->vertices[idx] = ConvertLonToScreen(point.x);
        rbd->vertices[idx + 1] = ConvertLatToScreen(point.y);
        idx += 2;
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
