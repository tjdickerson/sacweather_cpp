//

#include "sacw_api.h"
#include "sacw_extern.h"
#include "render_extern.h"

#include "tjd_conversions.h"
#include "tjd_shapefile.h"
#include "tjd_radar.h"
#include "tjd_ftp.h"
#include "nws_info.h"

MapViewState MapViewInfo;

static NexradProduct* CurrentProduct;

// temp?
constexpr int DefaultProduct = 19;
constexpr char* DefaultWSR = "kmxx";


void sacw_Init(char* args)
{    
    MapViewInfo = {};    

    char* siteName = DefaultWSR;
    if (args) siteName = args;
    printf("Site name: %s\n", siteName);

    InitNexradProducts();
    CurrentProduct = GetProductInfo(DefaultProduct);
    WSR88DInfo wsrInfo = {};  


    // download file
    char remoteFile[512];
    strcat(remoteFile, NWS_NOAA_RADAR_DIR);
    strcat(remoteFile, "/");
    strcat(remoteFile, CurrentProduct->dir);
    strcat(remoteFile, "/");
    strcat(remoteFile, "SI.");
    strcat(remoteFile, siteName);
    strcat(remoteFile, "/");
    strcat(remoteFile, "sn.last");

    printf("Download file: %s\n", remoteFile);

    DownloadFile(NWS_NOAA_HOSTNAME, remoteFile);



    tjd_RadarInit();

    //const char* filename = "C:\\tmp\\nor.last";
    const char* filename = "C:\\tmp\\testing_radar.nx3";
    //const char* filename = "C:\\tmp\\testing_radar1.nx3";
    ParseNexradRadarFile(filename, &wsrInfo, CurrentProduct);

    MapViewInfo.scaleFactor = 45.0f;
    MapViewInfo.xPan = -ConvertLonToScreen(wsrInfo.lon);
    MapViewInfo.yPan = -ConvertLatToScreen(wsrInfo.lat);

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


void sacw_PanMap(f32 x, f32 y)
{
    MapViewInfo.xPan += x;
    MapViewInfo.yPan += y;
}


void sacw_GetRadarRenderData(RenderBufferData* rbd, RenderVertData* rvd)
{
    tjd_GetRadarRenderData(rbd, rvd);
}


void sacw_GetMapRenderData(RenderBufferData* rbd, RenderVertData* rvd)
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
