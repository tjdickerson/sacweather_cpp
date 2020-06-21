// 

#include <cstdio>
#include <cstring>
#include <cmath>
#include <cassert>

#include "tjd_conversions.h"
#include "tjd_radar.h"

#define RADIAL_COUNT    360
#define BIN_COUNT       230

static NexradProduct* CurrentProduct;
RangeBin* RangeBins;

s32* StartsArray;
s32* CountsArray;

bool ParseNexradRadarFile(const char* filename, WSR88DInfo* wsrInfo);
void CalcRangeBinLocations(f32 cx, f32 cy, f32 range);
RangeBin* GetRangeBin(s32 radialIndex, s32 binIndex);
void SetRangeBin(s32 radialIndex, s32 binIndex, s32 colorIndex);


void tjd_RadarInit()
{    
    int binCount = BIN_COUNT * RADIAL_COUNT;
    RangeBins = (RangeBin*)malloc(binCount * sizeof(RangeBin));

    StartsArray = (s32*)malloc(binCount * sizeof(*StartsArray));
    CountsArray = (s32*)malloc(binCount * sizeof(*CountsArray));

    for(int i = 0; i < binCount; i++)
    {
        RangeBins[i] = {};
        StartsArray[i] = i * 4;
        CountsArray[i] = 4;
    }     
    
}


void tjd_GetRadarRenderData(RenderBufferData* rbd, RenderVertData* rvd)
{
    rbd->vertexCount = BIN_COUNT * RADIAL_COUNT * 4;
    rbd->vertices = (f32*)malloc(rbd->vertexCount * 6 * sizeof(f32));

    int vi = 0;
    for(int i = 0; i < BIN_COUNT * RADIAL_COUNT; i++)
    {
        rbd->vertices[vi++] = RangeBins[i].p1.x;
        rbd->vertices[vi++] = RangeBins[i].p1.y;
        rbd->vertices[vi++] = RangeBins[i].color.r;
        rbd->vertices[vi++] = RangeBins[i].color.g;
        rbd->vertices[vi++] = RangeBins[i].color.b;
        rbd->vertices[vi++] = RangeBins[i].color.a;

        rbd->vertices[vi++] = RangeBins[i].p2.x;
        rbd->vertices[vi++] = RangeBins[i].p2.y;
        rbd->vertices[vi++] = RangeBins[i].color.r;
        rbd->vertices[vi++] = RangeBins[i].color.g;
        rbd->vertices[vi++] = RangeBins[i].color.b;
        rbd->vertices[vi++] = RangeBins[i].color.a;

        rbd->vertices[vi++] = RangeBins[i].p3.x;
        rbd->vertices[vi++] = RangeBins[i].p3.y;
        rbd->vertices[vi++] = RangeBins[i].color.r;
        rbd->vertices[vi++] = RangeBins[i].color.g;
        rbd->vertices[vi++] = RangeBins[i].color.b;
        rbd->vertices[vi++] = RangeBins[i].color.a;

        rbd->vertices[vi++] = RangeBins[i].p4.x;
        rbd->vertices[vi++] = RangeBins[i].p4.y;
        rbd->vertices[vi++] = RangeBins[i].color.r;
        rbd->vertices[vi++] = RangeBins[i].color.g;
        rbd->vertices[vi++] = RangeBins[i].color.b;
        rbd->vertices[vi++] = RangeBins[i].color.a;        
    }

    rvd->numParts = BIN_COUNT * RADIAL_COUNT;
    rvd->starts = (s32*)malloc(rvd->numParts * sizeof(s32));
    rvd->counts = (s32*)malloc(rvd->numParts * sizeof(s32));

    for (int i = 0; i < rvd->numParts; i++)
    {
        rvd->starts[i] = (i * 4);
        rvd->counts[i] = 4;
    }
}


void CalcRangeBinLocation(
        s32 radialIndex,
        s32 binIndex,
        f32 cx, 
        f32 cy, 
        f32 range,
        f32 angleStart,
        f32 angleDelta)
{
    f32 dx1, dy1, dx2, dy2;

    RangeBin* bin = GetRangeBin(radialIndex, binIndex);
    f32 sweepCenterLeft  = angleStart - (angleDelta * 0.5f);
    f32 sweepCenterRight = angleStart + (angleDelta * 0.5f);

    dx1 = (binIndex + 1) * (range / (float)BIN_COUNT) / (cos(cy * PI / 180.0f));
    dy1 = (binIndex + 1) * (range / (float)BIN_COUNT);

    dx2 = (binIndex + 2) * (range / (float)BIN_COUNT) / (cos(cy * PI / 180.0f));
    dy2 = (binIndex + 2) * (range / (float)BIN_COUNT);

    // "bottom" left
    bin->p1.x = dx1 * sin(DegToRad(sweepCenterLeft));
    bin->p1.y = dy1 * cos(DegToRad(sweepCenterLeft));

    // "top" left
    bin->p2.x = dx2 * sin(DegToRad(sweepCenterLeft));
    bin->p2.y = dy2 * cos(DegToRad(sweepCenterLeft));

    // "top" right
    bin->p3.x = dx2 * sin(DegToRad(sweepCenterRight));
    bin->p3.y = dy2 * cos(DegToRad(sweepCenterRight));            

    // "bottom" right
    bin->p4.x = dx1 * sin(DegToRad(sweepCenterRight));
    bin->p4.y = dy1 * cos(DegToRad(sweepCenterRight));            

    
    // Convert the nautical mile results into lon/lat degree offsets
    bin->p1.x = (cx + (bin->p1.x / 60.0f));
    bin->p1.y = (cy + (bin->p1.y / 60.0f));

    bin->p2.x = (cx + (bin->p2.x / 60.0f));
    bin->p2.y = (cy + (bin->p2.y / 60.0f));

    bin->p3.x = (cx + (bin->p3.x / 60.0f));
    bin->p3.y = (cy + (bin->p3.y / 60.0f));

    bin->p4.x = (cx + (bin->p4.x / 60.0f));
    bin->p4.y = (cy + (bin->p4.y / 60.0f));


    // Convert to screen coordinates.
    bin->p1.x = ConvertLonToScreen(bin->p1.x);
    bin->p1.y = ConvertLatToScreen(bin->p1.y);

    bin->p2.x = ConvertLonToScreen(bin->p2.x);
    bin->p2.y = ConvertLatToScreen(bin->p2.y);

    bin->p3.x = ConvertLonToScreen(bin->p3.x);
    bin->p3.y = ConvertLatToScreen(bin->p3.y);

    bin->p4.x = ConvertLonToScreen(bin->p4.x);
    bin->p4.y = ConvertLatToScreen(bin->p4.y);
}


RangeBin* GetRangeBin(s32 radialIndex, s32 binIndex)
{
    RangeBin* bin = &(RangeBins[radialIndex + (binIndex * 360)]);
    return bin;
}


void SetRangeBin(s32 radialIndex, s32 binIndex, s32 colorIndex)
{
    int ci = (colorIndex * 4);
    RangeBin* bin = &(RangeBins[radialIndex + (binIndex * 360)]);        
    bin->color.r = ReflectivityMap[ci];
    bin->color.g = ReflectivityMap[ci + 1];
    bin->color.b = ReflectivityMap[ci + 2];
    bin->color.a = ReflectivityMap[ci + 3];
}


bool ParseNexradRadarFile(const char* filename, WSR88DInfo* wsrInfo, NexradProduct* nexradProduct)
{
    u32 bp = 0;
    f32 maxRange = 124.0f;

    FILE* fp = NULL;
    int err = fopen_s(&fp, filename, "rb");
    if (err != 0)
    {
        // error reading file.
        printf("Failed to open radar file %s\n", filename);
        return false;
    }

   
    fseek(fp, 0, SEEK_END);
    u32 fileLength = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Reading the whole file into a buffer and then parsing out the contents
    unsigned char* buffer = (unsigned char*)malloc(fileLength * sizeof(unsigned char));    
    int bytesRead = fread(buffer, 1, fileLength, fp);
    fclose(fp);

    if (bytesRead != fileLength)
    {
        printf("Failed to read radar file %s\n", filename);

        if (buffer) free(buffer);
        return false;
    }


    // The file data for radar files starts off with a 30 bytes text header
    // @todo, can use this to verify it is a valid file?
    char textHeader[31];
    memcpy(textHeader, &buffer[bp], 30);
    bp += 30;
    textHeader[30] = '\0';


    // The next 18 bytes is the MessageHeader
    unsigned char messageHeaderBuffer[18];
    memcpy(messageHeaderBuffer, &buffer[bp], 18);
    bp += 18;


    // The product description block is 102 bytes long. Unfortunately because of my current
    // uncertainty with the struct padding, I'm going to pluck these out one at a time. The way
    // the data is laid out in the file isn't aligned very nicely.
    ProductDescription pd = {}; 

    // skipping the divider
    bp += 2;


    // The latitude and longitude of the radar site is stored as two 4 byte integers. They are 
    // treated as having a scale of three.
    memcpy(&pd.lat, &buffer[bp], 4);
    bp += 4;
    pd.lat = swapBytes(pd.lat);

    memcpy(&pd.lon, &buffer[bp], 4);
    bp += 4;
    pd.lon = swapBytes(pd.lon);


    // Height of the radar site in feet above sea level.
    memcpy(&pd.height, &buffer[bp], 2);
    bp += 2;
    pd.height = swapBytes(pd.height);


    // Product Code referencing which NEXRAD product is contained in this file.
    memcpy(&pd.productCode, &buffer[bp], 2);
    bp += 2;
    pd.productCode = swapBytes(pd.productCode);


    // Operational Mode of the WSR-88D
    // 0 - Maintenance
    // 1 - Clean Air
    // 2 - Precipitation/Severe Weather
    memcpy(&pd.operationalMode, &buffer[bp], 2);
    bp += 2;
    pd.operationalMode = swapBytes(pd.operationalMode);


    // RDA Volume Coverage Pattern
    // @todo Document the VCP types and meanings
    memcpy(&pd.vcp, &buffer[bp], 2);
    bp += 2;
    pd.vcp = swapBytes(pd.vcp);


    // Sequence Number may not be useful.
    // Refer to Figure 3-4 of 2620001X.pdf
    memcpy(&pd.sequenceNum, &buffer[bp], 2);
    bp += 2;
    pd.sequenceNum = swapBytes(pd.sequenceNum);


    // This is a counter of volume scans for the radar site, it rolls back to one every 80 
    // volume scans. Not sure if this can be used in any meaningful way, but might be neat to
    // show if it's populated.
    memcpy(&pd.volScanNum, &buffer[bp], 2);
    bp += 2;
    pd.volScanNum = swapBytes(pd.volScanNum);


    // Volume Scan Date is stored as number of days since 1 Jan 1970.
    memcpy(&pd.volScanDate, &buffer[bp], 2);
    bp += 2;
    pd.volScanDate = swapBytes(pd.volScanDate);


    // Volume Scan Time is stored as number of seconds since Midnight.
    memcpy(&pd.volScanTime, &buffer[bp], 4);
    bp += 4;
    pd.volScanTime = swapBytes(pd.volScanTime);


    // @todo
    // Figure out the diference between volume scan date/time and product date/time
    memcpy(&pd.productDate, &buffer[bp], 2);
    bp += 2;
    pd.productDate = swapBytes(pd.productDate);

    memcpy(&pd.productTime, &buffer[bp], 4);
    bp += 4;
    pd.productTime = swapBytes(pd.productTime);


    // Reference TABLE V for Product dependant parameters 1 and 2.
    memcpy(&pd.h27_28, &buffer[bp], 4);
    bp += 4;
    

    // Elevation number within volume scan has range of 0-20
    memcpy(&pd.elevationNum, &buffer[bp], 2);        
    bp += 2;
    pd.elevationNum = swapBytes(pd.elevationNum);


    // Halfword 30 changes based on product, see TABLE V for parameter 3
    if (pd.productCode == 19) 
    {
        memcpy(&pd.elevationAngle, &buffer[bp], 2);
        bp += 2;
        pd.elevationAngle = swapBytes(pd.elevationAngle);
    } 
    else 
    {
        memcpy(&pd.h30, &buffer[bp], 2);
        bp += 2;
    }


    // @todo
    memcpy(&pd.thresholdData, &buffer[bp], 32);
    bp += 32;


    // TABLE V parameters 4 - 10 @todo
    memcpy(&pd.h47_53, &buffer[bp], 14);
    bp += 14;


    // Product version number, might be interesting to show on screen.
    memcpy(&pd.version, &buffer[bp], 1);
    bp += 1;


    // Spot Blanking?
    // 1 - Spot Blanking ON
    // 0 - Spot Blanking OFF
    memcpy(&pd.spotBlank, &buffer[bp], 1);
    bp += 1;



    memcpy(&pd.symbologyOffset, &buffer[bp], 4);
    bp += 4;

    memcpy(&pd.graphicOffset, &buffer[bp], 4);
    bp += 4;

    memcpy(&pd.tabularOffset, &buffer[bp], 4);
    bp += 4;


    SymbologyHeader sh = {};
    memcpy(&sh, &buffer[bp], sizeof(sh));
    bp += sizeof(sh);

    sh.divider = swapBytes(sh.divider);
    sh.blockId = swapBytes(sh.blockId);
    sh.blockLength = swapBytes(sh.blockLength);
    sh.layers = swapBytes(sh.layers);
    sh.layerDivider = swapBytes(sh.layerDivider);
    sh.layerLength = swapBytes(sh.layerLength);

    s16 packetCode;
    memcpy(&packetCode, &buffer[bp], 2);
    packetCode = swapBytes(packetCode);
    bp += 2;

    if ((packetCode & 0xffff) != 0xaf1f) 
    {
        // @todo
        // log error of unsupported data packet and exit @todo
        // big boi leaks @todo
        // @todo
        printf("Unsupported packet type: %d\n", packetCode);
        return false;
    }    


    s16 firstBin;
    s16 iCenterSweep;
    s16 jCenterSweep;
    s16 binCount;
    s16 scaleFactor;
    s16 radialCount;

    memcpy(&firstBin, &buffer[bp], 2);       bp += 2;
    memcpy(&binCount, &buffer[bp], 2);       bp += 2;
    memcpy(&iCenterSweep, &buffer[bp], 2);   bp += 2;
    memcpy(&jCenterSweep, &buffer[bp], 2);   bp += 2;
    memcpy(&scaleFactor, &buffer[bp], 2);    bp += 2;
    memcpy(&radialCount, &buffer[bp], 2);    bp += 2;

    firstBin = swapBytes(firstBin);
    iCenterSweep = swapBytes(iCenterSweep);
    jCenterSweep = swapBytes(jCenterSweep);
    binCount = swapBytes(binCount);
    scaleFactor = swapBytes(scaleFactor);
    radialCount = swapBytes(radialCount);

    // ??? @todo
    assert(binCount == 230);
    assert(radialCount == 360);   


    // page 3-109 IC Doc
    // scale factor is 230 / binCount, avoid fp errors by just making it 1 unless it needs to 
    // be something else.
    f32 fScaleFactor = 1.0f;
    if (binCount != 230)
        fScaleFactor = scaleFactor * 0.001f;


    s8 run_color;
    s8 run, color;
    s16 rleCount;
    s16 i_angleStart, i_angleDelta;
    f32 f_angleStart, f_angleDelta;
    f32 f_elevationAngle = pd.elevationAngle * 0.1f;

    f32 siteLat = pd.lat * 0.001f;
    f32 siteLon = pd.lon * 0.001f;

    wsrInfo->lat = siteLat;
    wsrInfo->lon = siteLon;
    
    int binIndex;
    for (int i = 0; i < radialCount; i++)
    {
        binIndex = 0;

        memcpy(&rleCount, &buffer[bp], sizeof(rleCount));
        bp += sizeof(rleCount);
        rleCount = swapBytes(rleCount);

        memcpy(&i_angleStart, &buffer[bp], sizeof(i_angleStart));
        bp += sizeof(i_angleStart);
        f_angleStart = swapBytes(i_angleStart) * 0.1f;

        memcpy(&i_angleDelta, &buffer[bp], sizeof(i_angleDelta));
        bp += sizeof(i_angleDelta);        
        f_angleDelta = swapBytes(i_angleDelta) * 0.1f;

        for (int j = 0; j < rleCount * 2; j++) 
        {
            run_color = (s8)(buffer[bp]);
            bp += 1;

            run = (run_color & 0xf0) >> 4;
            color = (run_color & 0x0f);

            for (s8 k = 0; k < run; k++) 
            {               
                CalcRangeBinLocation(
                    i, binIndex, siteLon, siteLat, nexradProduct->range, f_angleStart, f_angleDelta);
                SetRangeBin(i, binIndex, color);
                binIndex += 1;
            }
        }

    }

    return true;
}