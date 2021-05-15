//

#include "sacw_api.h"
#include "sacw_extern.h"
#include "render_extern.h"

#include "tjd_conversions.h"
#include "tjd_shapefile.h"
#include "tjd_radar.h"

#ifndef __ANDROID__
#include "tjd_ftp.h"
#endif

#include "nws_info.h"

MapViewState MapViewInfo;
bool canRenderRadar;
bool radarIsStale;

static NexradProduct* CurrentProduct;

// temp?


#ifndef __ANDROID__

// @todo
// need to move this out of the library into the application layer.

void DownloadRadarFile()
{
    int DefaultProduct = 37;
    char* DefaultWSR = "kama";

    char* siteName = DefaultWSR;
    printf("Site name: %s\n", siteName);

    CurrentProduct = GetProductInfo(DefaultProduct);
    WSR88DInfo wsrInfo = {};  

    // download file
    char remoteFile[512];
    memset(remoteFile, 0, 512);
    
    strcat(remoteFile, NWS_NOAA_RADAR_DIR);
    strcat(remoteFile, "/");
    strcat(remoteFile, CurrentProduct->dir);
    strcat(remoteFile, "/");
    strcat(remoteFile, "SI.");
    strcat(remoteFile, siteName);
    strcat(remoteFile, "/");
    strcat(remoteFile, "sn.last\0");

    printf("Download file: %s\n", remoteFile);

    DownloadFile(NWS_NOAA_HOSTNAME, remoteFile);

    //const char* filename = "C:\\tmp\\nor.last";
    //const char* filename = "C:\\tmp\\test_vel.last";
    const char* filename = "C:\\tmp\\testing_radar.nx3";   

    sacw_RadarInit(filename, CurrentProduct->productCode);  
}

#endif

void sacw_Init()
{    
    radarIsStale = false;
    MapViewInfo = {};    

    InitNexradProducts();

    MapViewInfo.scaleFactor = 80.0f;
    MapViewInfo.xPan = -ConvertLonToScreen(-85.790f);
    MapViewInfo.yPan = -ConvertLatToScreen(32.537f);

    RenderInit();

    #ifndef __ANDROID__
    DownloadRadarFile();
    #endif
}


void sacw_RadarInit(const char* filename, s16 productCode)
{
    bool success = false;

    NexradProduct* np = GetProductInfo(productCode);

    WSR88DInfo wsrInfo = {};
    success = ParseNexradRadarFile(filename, &wsrInfo, np);

    MapViewInfo.xPan = -ConvertLonToScreen(wsrInfo.lon);
    MapViewInfo.yPan = -ConvertLatToScreen(wsrInfo.lat);

    canRenderRadar = success;
    radarIsStale = true;    
}


void sacw_MainLoop()
{
    if (radarIsStale)
    {
        radarIsStale = false;
        LoadLatestRadarData();
    }

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
    if (zoom == 0) return;

    f32 speed = 5.0f;
    f32 dir = zoom > 0 ? 1 : -1;

    f32 delta = (speed + (MapViewInfo.scaleFactor * 0.1f)) * dir;
    f32 targetScale = MapViewInfo.scaleFactor + delta;

    if (targetScale < 25) targetScale = 25 + 1;
    else if (targetScale > 200) targetScale = 200 - 1;

    MapViewInfo.scaleFactor = targetScale;
}


void sacw_PanMap(f32 x, f32 y)
{
    MapViewInfo.xPan += (x / MapViewInfo.scaleFactor) * -0.002f;
    MapViewInfo.yPan += (y / MapViewInfo.scaleFactor) * 0.002f;
}


void sacw_GetRadarRenderData(RenderBufferData* rbd, RenderVertData* rvd)
{
    tjd_GetRadarRenderData(rbd, rvd);
}


void sacw_GetMapRenderData(RenderBufferData* rbd, RenderVertData* states, RenderVertData* counties)
{

    // @todo
    // handle this differently
    #ifdef __ANDROID__
    const char* stateShapeFile = "/data/user/0/com.tjdickerson.sacweather/files/shapes/st_us";
    const char* countyShapeFile = "/data/user/0/com.tjdickerson.sacweather/files/shapes/cnt_us";
    #else
    const char* stateShapeFile = "C:\\shapes\\weather\\st_us";
    const char* countyShapeFile = "C:\\shapes\\weather\\cnt_us";
    #endif

    ShapeData stateData = {};
    ReadShapeFile(&stateData, stateShapeFile);

    ShapeData countyData = {};
    ReadShapeFile(&countyData, countyShapeFile);

    rbd->vertexCount = stateData.numPoints + countyData.numPoints;
    s32 arraySize = rbd->vertexCount * 2 * sizeof(f32);
    rbd->vertices = (f32*)malloc(arraySize);

    int countiesOffset = stateData.numPoints;
    int idx = 0;

    {
        
        for(int i = 0; i < stateData.numPoints; i++)
        {
            v2f64 point = stateData.points.at(i);
            rbd->vertices[idx] = ConvertLonToScreen(point.x);
            rbd->vertices[idx + 1] = ConvertLatToScreen(point.y);
            idx += 2;
        }    

        states->numParts = stateData.numParts;
        states->starts = (s32*)malloc(states->numParts * sizeof(s32));
        states->counts = (s32*)malloc(states->numParts * sizeof(s32));

        for(int i = 0; i < states->numParts; i++)
        {
            states->starts[i] = stateData.parts.at(i);
            states->counts[i] = stateData.counts.at(i);
        }
    }

    {
        for(int i = 0; i < countyData.numPoints; i++)
        {
            v2f64 point = countyData.points.at(i);
            rbd->vertices[idx] = ConvertLonToScreen(point.x);
            rbd->vertices[idx + 1] = ConvertLatToScreen(point.y);
            idx += 2;
        }    

        counties->numParts = countyData.numParts;
        counties->starts = (s32*)malloc(counties->numParts * sizeof(s32));
        counties->counts = (s32*)malloc(counties->numParts * sizeof(s32));

        for(int i = 0; i < counties->numParts; i++)
        {
            counties->starts[i] = countiesOffset + countyData.parts.at(i);
            counties->counts[i] = countyData.counts.at(i);
        }
    }    

}
