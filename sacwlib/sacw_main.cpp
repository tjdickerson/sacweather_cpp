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
static ProductDescription gProductDescription;

v2f32 ConvertScreenToCoords(s32 x, s32 y);

// temp?


#ifndef __ANDROID__

// @todo
// need to move this out of the library into the application layer.

void DownloadRadarFile()
{
    int DefaultProduct = 94;
    char* DefaultWSR = "kmxx";

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

    //const char* filename = "C:\\shapes\\KIND_20210526_1423";
    //const char* filename = "C:\\tmp\\test_vel.last";
    const char* filename = "C:\\tmp\\testing_radar.nx3";   

    sacw_RadarInit(filename, CurrentProduct->productCode);  
}

#endif

void sacw_Init(void* window)
{    
    radarIsStale = false;
    MapViewInfo = {};    

    InitNexradProducts();

    MapViewInfo.scaleFactor = 80.0f;
    MapViewInfo.xPan = -ConvertLonToScreen(-85.790f);
    MapViewInfo.yPan = -ConvertLatToScreen(32.537f);

    RenderInit(window);

    #ifndef __ANDROID__
    DownloadRadarFile();
    #endif
}


bool sacw_RadarInit(const char* filename, s16 productCode)
{
    bool success = false;

    NexradProduct* np = GetProductInfo(productCode);

    canRenderRadar = false;
    radarIsStale = false;   

    WSR88DInfo wsrInfo = {};
    gProductDescription = {};
    success = ParseNexradRadarFile(filename, &wsrInfo, np, &gProductDescription);

    if (success)
    {
        MapViewInfo.xPan = -ConvertLonToScreen(wsrInfo.lon);
        MapViewInfo.yPan = -ConvertLatToScreen(wsrInfo.lat);

        canRenderRadar = success;
        radarIsStale = success;   
    }

    return success; 
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

    f32 speed = 0.5f;
    f32 dir = zoom > 0 ? 1 : -1;

    f32 delta = (MapViewInfo.scaleFactor * 0.1f) * dir * speed;
    f32 targetScale = MapViewInfo.scaleFactor + delta;

    if (targetScale < 10) targetScale = 10 + 1;
    else if (targetScale > 250) targetScale = 250 - 1;

    MapViewInfo.scaleFactor = targetScale;
}


void sacw_PanMap(f32 x, f32 y)
{
    MapViewInfo.xPan += (x / MapViewInfo.scaleFactor) * -0.002f;
    MapViewInfo.yPan += (y / MapViewInfo.scaleFactor) * 0.002f;
}


void sacw_GetRadarRenderData(RenderBufferData* rbd)
{
    tjd_GetRadarRenderData(rbd);
}

s64 sacw_GetScanTime()
{
    s64 idunno = 86400000;
    s32 scanDate = gProductDescription.productDate - 1;
    s32 scanTime = gProductDescription.productTime;

    s64 whateven = (scanDate * idunno) + (scanTime * 1000);

    LOGINF("Scan Time: %d %d %lld\n", scanDate, scanTime, whateven);
    return whateven;
}

void sacw_GetPolarFromScreen(f32 x, f32 y, f32* points)
{
    // ?
    v2f32 convertedPoint = ConvertScreenToCoords(x, y);
    points[0] = convertedPoint.x;
    points[1] = convertedPoint.y;
}


void sacw_GetMapRenderData(RenderBufferData* rbd, RenderVertData* states, RenderVertData* counties)
{

    // @todo
    // handle this differently
    #ifdef __ANDROID__
    const char* stateShapeFile = "/data/user/0/com.tjdickerson.sacweather/files/data/st_us";
    const char* countyShapeFile = "/data/user/0/com.tjdickerson.sacweather/files/data/cnt_us";
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

v2f32 ConvertScreenToCoords(s32 x, s32 y) 
{
    /* 

    By shifting the longitude value to the "east" by 180 it essentially becomes a 0 - 360
    screen width. Once the relateive sceen coordinate has been scaled down, then we can just
    shift the values back "west" by 180. This makes it trivial to convert from a screen 
    coordinate to a longitute.

    The same can be applied to latitude. The mercator projection can be applied after this
    translation to map it to real coordinates.

    0 -------------x1-------------  1000 w1 = client screen width
    0 -------------x2-------------   360 w2 = adjusted lon coord width

    x1 = input = 500
    x2 = target = 180 ( - 180 = 0)

    ((x1 * w2) / w1) - 180 = x2

    0 -----x1---------------------  1000 w1 = client screen width
    0 -----x2---------------------   360 w2 = adjusted lon coord width

    x1 = input = 250
    x2 = target = 90 ( - 180 = -90)

    ((x1 * w2) / w1) - 180 = x2

    */
    
    // TODO: This may not work when the screen is smaller than 360x360.

    v2f32 coords = {};

    f32 width = MapViewInfo.mapWidthPixels; 
    f32 height = MapViewInfo.mapHeightPixels;
    // width = 984;
    // height = 961;

    f32 totalWidth = 360.0f;
    f32 totalHeight = 360.0f;
    f32 xRadius = 180.0f;
    f32 yRadius = 180.0f;
    f32 xPan = MapViewInfo.xPan * (MapViewInfo.scaleFactor / MapViewInfo.xScale);
    f32 yPan = MapViewInfo.yPan * (MapViewInfo.scaleFactor / MapViewInfo.yScale);

    f32 xAdjust = MapViewInfo.scaleFactor / MapViewInfo.xScale;
    f32 yAdjust = MapViewInfo.scaleFactor / MapViewInfo.yScale;

    totalWidth  /= xAdjust;
    totalHeight /= yAdjust;
    xRadius /= xAdjust;
    yRadius /= yAdjust;
    xPan /= xAdjust;
    yPan /= yAdjust;
   
    coords.x = ((x * totalWidth) / (width)) - xRadius;
    coords.x = xPan * 180.0f - coords.x;
    coords.x *= -1.0f;

    coords.y = ((y * totalHeight) / (height)) - yRadius;         
    coords.y = yPan * 180.0f + coords.y;
    coords.y *= -1.0f;

    coords.y = ScreenToY(coords.y);

    LOGINF("Lon Lat: %2.4f %2.4f\n", coords.x, coords.y);

    return coords;
}