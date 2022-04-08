//

#include "sacw_api.h"
#include "sacw_extern.h"
#include "render_extern.h"

#include "tjd_conversions.h"
#include "tjd_shapefile.h"
#include "tjd_radar.h"

#ifndef __ANDROID__
#include "tjd_win32_download.h"
#endif

#include "nws_info.h"
#include <cstring>

#include "windows.h"

MapViewInfo g_MapViewInfo;
GeoTextRenderInfo g_GeoTextRenderInfo;
RdaSiteInfo g_RdaSiteInfo;
NexradProduct* g_CurrentProduct;
RdaSite* g_CurrentSite;

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
    char* DefaultWSR = "KMXX";

    char* siteName = DefaultWSR;
    printf("Site name: %s\n", siteName);

    CurrentProduct = GetProductInfo(DefaultProduct);
    RdaSite wsrInfo = {};

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

bool readWsrList()
{
    std::string filepath = R"(.\resources\wsrlist)";
    std::string file_contents;

    FILE* fp = fopen(filepath.c_str(), "r");

    if (fp == nullptr)
    {
        LOGERR("Failed to open wsrlist.\n");
        return false;
    }

    u32 size = 0;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    file_contents.resize(size);
    auto temp_buffer = (char*)malloc(size);
    fread(temp_buffer, 1, size, fp);

    file_contents = temp_buffer;
    free(temp_buffer);

    // parse stuff
    std::vector<RdaSite> temp_sites;
    char line[512];
    u32 idx = 0;
    u32 pos = 0;
    while (idx < size)
    {
        RdaSite this_site = {};

        pos = file_contents.find_first_of('|', idx + 1);
        std::string rdaName = file_contents.substr(idx, pos - idx);
        idx += rdaName.length() + 1;
        strcpy(this_site.name, rdaName.c_str());

        pos = file_contents.find_first_of('|', idx + 1);
        std::string name = file_contents.substr(idx, pos - idx);
        idx += name.length() + 1;
        strcpy(this_site.displayName, name.c_str());

        pos = file_contents.find_first_of('|', idx + 1);
        std::string lon = file_contents.substr(idx, pos - idx);
        idx += lon.length() + 1;

        f32 f_lon = atof(lon.c_str());
        this_site.location.lon = f_lon;

        pos = file_contents.find_first_of('\n', idx + 1);
        std::string lat = file_contents.substr(idx, pos - idx);
        idx += lat.length() + 1;

        f32 f_lat = atof(lat.c_str());
        this_site.location.lat = f_lat;

        temp_sites.push_back(this_site);
    }

    g_RdaSiteInfo.count = temp_sites.size();
    g_RdaSiteInfo.sites = (RdaSite*)malloc(temp_sites.size() * sizeof(RdaSite));
    for (int i = 0; i < temp_sites.size(); i++)
    {
        g_RdaSiteInfo.sites[i] = temp_sites.at(i);
    }

    return true;
}

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
    s32 shape_file_count = 5;
    g_MapViewInfo.shapeFileCount = shape_file_count;
    g_MapViewInfo.renderInfo = (ShapeRenderInfo*)calloc(shape_file_count, sizeof(ShapeRenderInfo));

    std::string filename = "";
    color4 line_color = {};

    ShapeFileInfo states = {};
    filename = R"(.\resources\shapes\st_us)";
    line_color = { ColorHexToFloat(0x73), ColorHexToFloat(0x73), ColorHexToFloat(0x73), 1.0f };
    ShapeFileInit(&states, filename, line_color, true, "NAME");
    states.lineWidth = 1.2f;
    states.category = SHAPE_STATES;

    ShapeFileInfo counties = {};
    filename = R"(.\resources\shapes\cnt_us)";
    line_color = { ColorHexToFloat(0x25), ColorHexToFloat(0x30), ColorHexToFloat(0x28), 1.0f };
    ShapeFileInit(&counties, filename, line_color);
    counties.lineWidth = 1.2f;
    counties.category = SHAPE_COUNTIES;
    counties.showPastScale = 0.075f;

    ShapeFileInfo roads = {};
    filename = R"(.\resources\shapes\hways)";
    line_color = { ColorHexToFloat(0x3a), ColorHexToFloat(0x2c), ColorHexToFloat(0x28), 1.0f };
    ShapeFileInit(&roads, filename, line_color, true, "SIGN1");
    roads.category = SHAPE_ROADS;
    roads.lineWidth = 1.2f;
    roads.showPastScale = 0.075f;

    ShapeFileInfo rivers = {};
    filename = R"(.\resources\shapes\maj_riv)";
    line_color = { ColorHexToFloat(0x00), ColorHexToFloat(0x00), ColorHexToFloat(0x10), 1.0f };
    ShapeFileInit(&rivers, filename, line_color);
    rivers.category = SHAPE_RIVERS;
    rivers.lineWidth = 1.0f;
    rivers.showPastScale = 0.4f;

    ShapeFileInfo lakes = {};
    filename = R"(.\resources\shapes\lk_us)";
    line_color = { ColorHexToFloat(0x00), ColorHexToFloat(0x00), ColorHexToFloat(0x10), 1.0f };
    ShapeFileInit(&lakes, filename, line_color);
    lakes.category = SHAPE_RIVERS;
    lakes.lineWidth = 1.0f;
    lakes.showPastScale = 0.4f;


    g_MapViewInfo.renderInfo[0].shapeFile = counties;
    g_MapViewInfo.renderInfo[1].shapeFile = roads;
    g_MapViewInfo.renderInfo[2].shapeFile = rivers;
    g_MapViewInfo.renderInfo[3].shapeFile = lakes;
    g_MapViewInfo.renderInfo[4].shapeFile = states;

    for (int i = 0; i < shape_file_count; i++)
    {
        ShapeFileInfo* this_file = &g_MapViewInfo.renderInfo[i].shapeFile;

        ReadShapeFile(this_file, this_file->filename);
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

    g_MapViewInfo.scaleFactor = 1.0f; //80.0f;
    g_MapViewInfo.xScale = 1.0f;
    g_MapViewInfo.yScale = 1.0f;
    g_MapViewInfo.xPan = 0.0f; // -AdjustLonForProjection(-85.790f);
    g_MapViewInfo.yPan = 0.0f; // -AdjustLatForProjection(32.537f);

    // centerMapAt(-85.790f, 32.537f);


    InitNexradProducts();

    readWsrList();
    g_CurrentSite = FindSiteByName("KMXX");

    g_CurrentProduct = GetProductInfo(94);
    StartDownload("KMXX", g_CurrentProduct);

    shapeFileInit();

    RenderInit(window);
}

bool sacw_RadarInit(const char* filename, s16 productCode)
{
    bool success = false;

    NexradProduct* np = GetProductInfo(productCode);

    canRenderRadar = false;
    radarIsStale = false;

    RdaSite wsrInfo = {};
    gProductDescription = {};
    success = ParseNexradRadarFile(filename, &wsrInfo, np, &gProductDescription);

    if (success)
    {
        // g_MapViewInfo.xPan = -AdjustLonForProjection(wsrInfo.lon);
        // g_MapViewInfo.yPan = -AdjustLatForProjection(wsrInfo.lat);

        centerMapAt(wsrInfo.location.lon, wsrInfo.location.lat);

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

void sacw_UpdateViewport(f32 width, f32 height)
{
    RenderViewportUpdate(width, height);
    AdjustWorldScreenBounds();
}

void LogMapInfo()
{
#if 0
    LOGINF("MapInfo:    xPan %2.4f  yPan %2.4f  scaleFactor %2.4f   xScale %2.4f    yScale %2.4f\n",
        g_MapViewInfo.xPan, g_MapViewInfo.yPan, g_MapViewInfo.scaleFactor, g_MapViewInfo.xScale, g_MapViewInfo.yScale);

    LOGINF("            minx %2.4f  miny %2.4f  maxx %2.4f  maxy %2.4f\n",
        g_MapViewInfo.worldScreenBounds.min_x,
        g_MapViewInfo.worldScreenBounds.min_y,
        g_MapViewInfo.worldScreenBounds.max_x,
        g_MapViewInfo.worldScreenBounds.max_y);
#endif
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

    LOGINF("Scale: %.4f\n", g_MapViewInfo.scaleFactor);

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
    tjd_GetRadarRenderData(rbd, g_CurrentProduct);
}

s64 sacw_GetScanTime()
{
    return gProductDescription.volScanTimestamp;
}

void sacw_GetPolarFromScreen(f32 x, f32 y, f32* points)
{
    // ?
    v2f32 convertedPoint = ConvertScreenToCoords(&g_MapViewInfo, x, y);
    LOGINF("Scale: %.4f\n", g_MapViewInfo.scaleFactor);
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


RdaSite* FindClosestRda(v2f32 coords)
{
    RdaSite* closestRda = nullptr;

    f32 shortest_distance = 400.0f;
    for (int i = 0; i < g_RdaSiteInfo.count; i++)
    {
        RdaSite* site = &g_RdaSiteInfo.sites[i];
        f32 distance = abs(DistanceBetween(site->location, coords));
        if (distance < shortest_distance)
        {
            shortest_distance = distance;
            closestRda = site;
        }
    }

    return closestRda;
}

RdaSite* FindClosestRdaFromScreen(s32 screenX, s32 screenY)
{
    v2f32 coords = ConvertScreenToCoords(&g_MapViewInfo, screenX, screenY);
    return FindClosestRda(coords);
}

RdaSite* FindSiteByName(const char* name)
{
    RdaSite* result = nullptr;
    for (int i = 0; i < g_RdaSiteInfo.count; i++)
    {
        if (strncmp(g_RdaSiteInfo.sites[i].name, name, 4) == 0)
            result = &g_RdaSiteInfo.sites[i];
    }

    return result;
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


