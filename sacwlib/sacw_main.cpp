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
#include <cstring>

#include "windows.h"

MapViewInfo g_MapViewInfo;
GeoTextRenderInfo g_GeoTextRenderInfo;

bool canRenderRadar;
bool radarIsStale;
bool g_mapIsStale;

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
    char* DefaultWSR = "KDDC";

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

    // DownloadFile(NWS_NOAA_HOSTNAME, remoteFile);
    const char* file_url = "https://tgftp.nws.noaa.gov/SL.us008001/DF.of/DC.radar/DS.p94r0/SI.kmxx/sn.last";


    //const char* filename = "C:\\shapes\\KIND_20210526_1423";
    //const char* filename = "C:\\tmp\\test_vel.last";
    const char* filename = "C:\\tmp\\testing_radar.nx3";
//    URLDownloadToFile(nullptr, file_url, filename, 0, nullptr);
//
    sacw_RadarInit(filename, CurrentProduct->productCode);  
}

#endif

void centerMapAt(f32 lon, f32 lat)
{
    f32 screen_x = AdjustLonForProjection(lon);
    f32 screen_y = AdjustLatForProjection(lat);

    g_MapViewInfo.scaleFactor = 0.4f;
    g_MapViewInfo.xPan = -screen_x;
    g_MapViewInfo.yPan = -screen_y;
}

void shapeFileInit()
{
    s32 shape_file_count = 2;
    g_MapViewInfo.shapeFileCount = shape_file_count;
    g_MapViewInfo.renderInfo = (ShapeRenderInfo*)calloc(shape_file_count, sizeof(ShapeRenderInfo));

    ShapeFileInfo states = {};
    states.filename = R"(C:\shapes\weather\st_us)";
    states.lineColor = color4 {1.0f, 1.0f, 1.0f, 1.0f};

    ShapeFileInfo counties = {};
    counties.filename = R"(C:\shapes\weather\cnt_us)";
    counties.lineColor = color4 {0.4f, 0.4f, 0.4f, 1.0f};

    /*ShapeFileInfo roads = {};
    roads.filename = R"(C:\shapes\weather\roads)";
    roads.lineColor = color4 {0.4f, 0.4f, 1.0f, 1.0f};*/

    g_MapViewInfo.renderInfo[0].shapeFile = counties;
    g_MapViewInfo.renderInfo[1].shapeFile = states;
    // g_MapViewInfo.renderInfo[2].shapeFile = roads;

    for(int i = 0; i <  shape_file_count; i++)
    {
        ShapeFileInfo* this_file = &g_MapViewInfo.renderInfo[i].shapeFile;

        ReadShapeFile(this_file, this_file->filename.c_str());
        this_file->needsRefresh = true;
    }

    g_mapIsStale = true;
}

void sacw_Init(void* window)
{    
    radarIsStale = false;
    g_mapIsStale = false;

    g_MapViewInfo = {};
    g_MapViewInfo.worldScreenBounds = {};

    g_GeoTextRenderInfo = {};

    shapeFileInit();

    InitNexradProducts();

    g_MapViewInfo.scaleFactor = 1.0f; //80.0f;
    g_MapViewInfo.xScale = 1.0f;
    g_MapViewInfo.yScale = 1.0f;
    g_MapViewInfo.xPan = 0.0f; // -AdjustLonForProjection(-85.790f);
    g_MapViewInfo.yPan = 0.0f; // -AdjustLatForProjection(32.537f);

    RenderInit(window);

    centerMapAt(-85.790f, 32.537f);

    #ifndef __ANDROID__
    // DownloadRadarFile();
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
        // g_MapViewInfo.xPan = -AdjustLonForProjection(wsrInfo.lon);
        // g_MapViewInfo.yPan = -AdjustLatForProjection(wsrInfo.lat);

        centerMapAt(wsrInfo.lon, wsrInfo.lat);

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

    if (g_mapIsStale)
    {
        g_mapIsStale = false;
        LoadMapBufferData();
        GeoTextLoadBuffer();
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

void LogMapInfo()
{
    LOGINF("MapInfo:    xPan %2.4f  yPan %2.4f  scaleFactor %2.4f   xScale %2.4f    yScale %2.4f\n",
        g_MapViewInfo.xPan, g_MapViewInfo.yPan, g_MapViewInfo.scaleFactor, g_MapViewInfo.xScale, g_MapViewInfo.yScale);

    LOGINF("            minx %2.4f  miny %2.4f  maxx %2.4f  maxy %2.4f\n",
        g_MapViewInfo.worldScreenBounds.min_x,
        g_MapViewInfo.worldScreenBounds.min_y,
        g_MapViewInfo.worldScreenBounds.max_x,
        g_MapViewInfo.worldScreenBounds.max_y);
}

void AdjustWorldScreenBounds()
{
    // @todo
    // May need a bit of padding on this just to avoid weird cases with comparing floats.
    v2f32 top_left = ConvertScreenToCoords(&g_MapViewInfo, 0, 0);
    v2f32 bottom_right =
        ConvertScreenToCoords(&g_MapViewInfo, (s32)g_MapViewInfo.mapWidthPixels, (s32)g_MapViewInfo.mapHeightPixels);

    g_MapViewInfo.worldScreenBounds.min_x = top_left.x;
    g_MapViewInfo.worldScreenBounds.max_x = bottom_right.x;
    g_MapViewInfo.worldScreenBounds.min_y = bottom_right.y;
    g_MapViewInfo.worldScreenBounds.max_y = top_left.y;
}

void sacw_ZoomMap(f32 zoom)
{      
    if (zoom == 0) return;

    f32 speed = 1.0f;
    f32 dir = zoom > 0 ? 1 : -1;

    f32 delta = (g_MapViewInfo.scaleFactor * 0.1f) * dir * speed;
    f32 targetScale = g_MapViewInfo.scaleFactor + delta;

    // if (targetScale < 10) targetScale = 10 + 1;
    // else if (targetScale > 250) targetScale = 250 - 1;

    g_MapViewInfo.scaleFactor = targetScale;
    AdjustWorldScreenBounds();

    LogMapInfo();
}


void sacw_PanMap(f32 x, f32 y)
{
    g_MapViewInfo.xPan += (x / g_MapViewInfo.scaleFactor) * -0.002f;
    g_MapViewInfo.yPan += (y / g_MapViewInfo.scaleFactor) * 0.002f;

    AdjustWorldScreenBounds();
}


void sacw_GetRadarRenderData(RenderBufferData* rbd)
{
    tjd_GetRadarRenderData(rbd);
}

s64 sacw_GetScanTime()
{
    return gProductDescription.volScanTimestamp;
}

void sacw_GetPolarFromScreen(f32 x, f32 y, f32* points)
{
    // ?
    v2f32 convertedPoint = ConvertScreenToCoords(&g_MapViewInfo, x, y);

    points[0] = convertedPoint.x;
    points[1] = convertedPoint.y;
}


void sacw_GetMapRenderData(RenderBufferData* rbd, RenderVertData* states, RenderVertData* counties)
{
#if 0
    // @todo
    // handle this differently
    #ifdef __ANDROID__
    const char* stateShapeFile = "/data/user/0/com.tjdickerson.sacweather/files/data/st_us";
    const char* countyShapeFile = "/data/user/0/com.tjdickerson.sacweather/files/data/cnt_us";
    #else
    const char* stateShapeFile = "C:\\shapes\\weather\\st_us";
    const char* countyShapeFile = "C:\\shapes\\weather\\cnt_us";
    #endif

    ShapeFileInfo stateData = {};
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
            rbd->vertices[idx] = AdjustLonForProjection(point.x);
            rbd->vertices[idx + 1] = AdjustLatForProjection(point.y);
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
            rbd->vertices[idx] = AdjustLonForProjection(point.x);
            rbd->vertices[idx + 1] = AdjustLatForProjection(point.y);
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
#endif
}

v2f32 ConvertScreenToCoords(MapViewInfo* map, s32 x, s32 y)
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

    f32 width = map->mapWidthPixels;
    f32 height = map->mapHeightPixels;
    // width = 984;
    // height = 961;

    f32 totalWidth = 2.0f;
    f32 totalHeight = 2.0f;
    f32 xRadius = 1.0f;
    f32 yRadius = 1.0f;

    f32 fx = (map->scaleFactor / map->xScale);
    totalWidth /= fx;
    xRadius /= fx;

    f32 xPan = map->xPan;
    coords.x = ((x * totalWidth) / (width)) - xRadius;
    coords.x = coords.x - xPan;

    f32 fy = (map->scaleFactor / map->yScale);
    totalHeight /= fy;
    yRadius /= fy;

    f32 yPan = map->yPan;
    coords.y = ((y * totalHeight) / (height)) - yRadius;
    coords.y = coords.y + yPan;
    coords.y *= -1.0f;

    coords.y = ScreenToY(coords.y);
    return coords;
}