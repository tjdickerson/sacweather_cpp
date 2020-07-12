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
constexpr int DefaultProduct = 99;
constexpr char* DefaultWSR = "klot";


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

    //const char* filename = "C:\\tmp\\nor.last";
    //const char* filename = "C:\\tmp\\test_vel.last";
    const char* filename = "C:\\tmp\\testing_radar.nx3";
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


void sacw_GetMapRenderData(RenderBufferData* rbd, RenderVertData* states, RenderVertData* counties)
{
    ShapeData stateData = {};
    ReadShapeFile(&stateData, "st_us");

    ShapeData countyData = {};
    ReadShapeFile(&countyData, "cnt_us");

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
