// 

#include <cstdio>
#include <cstring>
#include <cmath>
#include <cassert>

#include "bzlib.h"

#include "tjd_conversions.h"
#include "tjd_radar.h"

s16 RadialCount = 1;
s16 BinCount = 1;


static NexradProduct* CurrentProduct;

s32* StartsArray;
s32* CountsArray;

RangeBin* RangeBins;
void CalcRangeBinLocations(f32 cx, f32 cy, f32 range);
void SetRangeBin(s32 radialIndex, s32 binIndex, s32 colorIndex);
RangeBin* GetRangeBin(s32 radialIndex, s32 binIndex);


bool RadialImagePacket(
    unsigned char* buffer, 
    u32 bp, 
    NexradProduct* nexradProduct, 
    WSR88DInfo* wsrInfo,
    ProductDescription* pd,
    s16 packetCode);


void SetRasterCell(f32 cx, f32 cy, s32 rowCount, s32 ix, s32 iy, f32 res, s32 colorIndex);

bool RasterImagePacket(
    unsigned char* buffer, 
    u32 bp, 
    NexradProduct* nexradProduct, 
    WSR88DInfo* wsrInfo,
    ProductDescription* pd,
    s16 packetCode);


void tjd_GetRadarRenderData(RenderBufferData* rbd)
{
    rbd->vertexCount = BinCount * RadialCount * 6;
    rbd->vertices = (f32*)malloc(rbd->vertexCount * 3 * sizeof(f32));

    int vi = 0;
    for(int i = 0; i < BinCount * RadialCount; i++)
    {
        rbd->vertices[vi++] = RangeBins[i].p1.x;
        rbd->vertices[vi++] = RangeBins[i].p1.y;
        rbd->vertices[vi++] = RangeBins[i].colorIndex;

        rbd->vertices[vi++] = RangeBins[i].p2.x;
        rbd->vertices[vi++] = RangeBins[i].p2.y;
        rbd->vertices[vi++] = RangeBins[i].colorIndex;

        rbd->vertices[vi++] = RangeBins[i].p3.x;
        rbd->vertices[vi++] = RangeBins[i].p3.y;
        rbd->vertices[vi++] = RangeBins[i].colorIndex;

        //

        rbd->vertices[vi++] = RangeBins[i].p3.x;
        rbd->vertices[vi++] = RangeBins[i].p3.y;
        rbd->vertices[vi++] = RangeBins[i].colorIndex;

        rbd->vertices[vi++] = RangeBins[i].p2.x;
        rbd->vertices[vi++] = RangeBins[i].p2.y;
        rbd->vertices[vi++] = RangeBins[i].colorIndex;


        rbd->vertices[vi++] = RangeBins[i].p4.x;
        rbd->vertices[vi++] = RangeBins[i].p4.y;
        rbd->vertices[vi++] = RangeBins[i].colorIndex;       
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

    dx1 = (binIndex + 1) * (range / (float)BinCount) / (cos(cy * PI / 180.0f));
    dy1 = (binIndex + 1) * (range / (float)BinCount);

    dx2 = (binIndex + 2) * (range / (float)BinCount) / (cos(cy * PI / 180.0f));
    dy2 = (binIndex + 2) * (range / (float)BinCount);


    // "bottom" left
    bin->p1.x = dx1 * sin(DegToRad(sweepCenterLeft));
    bin->p1.y = dy1 * cos(DegToRad(sweepCenterLeft));

    // "bottom" right
    bin->p2.x = dx1 * sin(DegToRad(sweepCenterRight));
    bin->p2.y = dy1 * cos(DegToRad(sweepCenterRight));   

    // "top" left
    bin->p3.x = dx2 * sin(DegToRad(sweepCenterLeft));
    bin->p3.y = dy2 * cos(DegToRad(sweepCenterLeft));

    // "top" right
    bin->p4.x = dx2 * sin(DegToRad(sweepCenterRight));
    bin->p4.y = dy2 * cos(DegToRad(sweepCenterRight));      

   

    
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
    RangeBin* bin = &(RangeBins[radialIndex + (binIndex * RadialCount)]);
    return bin;
}


void SetRangeBin(s32 radialIndex, s32 binIndex, s32 colorIndex)
{
    // int ci = (colorIndex * 4);
    // f32* colorMap = VelocityMap;
    RangeBin* bin = &(RangeBins[radialIndex + (binIndex * RadialCount)]);        
    bin->colorIndex = colorIndex;
    // bin->color.r = colorMap[ci];
    // bin->color.g = colorMap[ci + 1];
    // bin->color.b = colorMap[ci + 2];
    // bin->color.a = colorMap[ci + 3];
}


void SetRasterCell(f32 cx, f32 cy, s32 rowCount, s32 ix, s32 iy, f32 res, s32 colorIndex)
{
    // moveme @todo
    f32 halfRes = res / 2.0f;
    f32 x, y;

    RangeBin* cell = &(RangeBins[iy + (ix * rowCount)]);


    // bottom left
    cell->p1.x = ((ix * res) - halfRes) - ((RadialCount * res) / 2.0f);
    cell->p1.y = ((iy * res) - halfRes) - ((RadialCount * res) / 2.0f);

    // bottom right
    cell->p2.x = ((ix * res) + halfRes) - ((RadialCount * res) / 2.0f);
    cell->p2.y = ((iy * res) - halfRes) - ((RadialCount * res) / 2.0f);    

    // top left
    cell->p3.x = ((ix * res) - halfRes) - ((RadialCount * res) / 2.0f);
    cell->p3.y = ((iy * res) + halfRes) - ((RadialCount * res) / 2.0f);

    // top right
    cell->p4.x = ((ix * res) + halfRes) - ((RadialCount * res) / 2.0f);
    cell->p4.y = ((iy * res) + halfRes) - ((RadialCount * res) / 2.0f);
  


    // x adsjut
    cell->p1.x /= (cos(cy * PI / 180.0f));
    cell->p2.x /= (cos(cy * PI / 180.0f));
    cell->p3.x /= (cos(cy * PI / 180.0f));
    cell->p4.x /= (cos(cy * PI / 180.0f));


    // convert to lat/lon from nautical miles
    cell->p1.x = (cx + (cell->p1.x / 60.0f));
    cell->p1.y = (cy - (cell->p1.y / 60.0f));

    cell->p2.x = (cx + (cell->p2.x / 60.0f));
    cell->p2.y = (cy - (cell->p2.y / 60.0f));

    cell->p3.x = (cx + (cell->p3.x / 60.0f));
    cell->p3.y = (cy - (cell->p3.y / 60.0f));

    cell->p4.x = (cx + (cell->p4.x / 60.0f));
    cell->p4.y = (cy - (cell->p4.y / 60.0f));


    // convert to screen coords
    cell->p1.x = ConvertLonToScreen(cell->p1.x);
    cell->p1.y = ConvertLatToScreen(cell->p1.y);

    cell->p2.x = ConvertLonToScreen(cell->p2.x);
    cell->p2.y = ConvertLatToScreen(cell->p2.y);

    cell->p3.x = ConvertLonToScreen(cell->p3.x);
    cell->p3.y = ConvertLatToScreen(cell->p3.y);

    cell->p4.x = ConvertLonToScreen(cell->p4.x);
    cell->p4.y = ConvertLatToScreen(cell->p4.y);

    //cell->color = ReflectivityMap[colorIndex];
    cell->colorIndex = (f32)colorIndex;
    // int ci = (colorIndex * 4);
    // cell->color.r = ReflectivityMap[ci];
    // cell->color.g = ReflectivityMap[ci + 1];
    // cell->color.b = ReflectivityMap[ci + 2];
    // cell->color.a = ReflectivityMap[ci + 3];

}


bool ParseNexradRadarFile(
    const char* filename, 
    WSR88DInfo* wsrInfo, 
    NexradProduct* nexradProduct,
    ProductDescription* pd)
{
    u32 bp = 0;

    FILE* fp = NULL;
    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        // error reading file.
        LOGERR("Failed to open radar file %s\n", filename);
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
        LOGERR("Failed to read radar file %s\n", filename);

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

    int messageHeaderLength = 18;    
    u32 productLength = 0;

    {
        int p = 8;
        memcpy(&productLength, &messageHeaderBuffer[p], 4);
        productLength = swapBytes(productLength);
    }

    
    // skipping the divider
    bp += 2;


    // The latitude and longitude of the radar site is stored as two 4 byte integers. They are 
    // treated as having a scale of three.
    memcpy(&pd->lat, &buffer[bp], 4);
    bp += 4;
    pd->lat = swapBytes(pd->lat);

    memcpy(&pd->lon, &buffer[bp], 4);
    bp += 4;
    pd->lon = swapBytes(pd->lon);

    f32 siteLat = pd->lat * 0.001f;
    f32 siteLon = pd->lon * 0.001f;

    wsrInfo->lat = siteLat;
    wsrInfo->lon = siteLon;


    // Height of the radar site in feet above sea level.
    memcpy(&pd->height, &buffer[bp], 2);
    bp += 2;
    pd->height = swapBytes(pd->height);    


    // Product Code referencing which NEXRAD product is contained in this file.
    memcpy(&pd->productCode, &buffer[bp], 2);
    bp += 2;
    pd->productCode = swapBytes(pd->productCode);


    // Operational Mode of the WSR-88D
    // 0 - Maintenance
    // 1 - Clean Air
    // 2 - Precipitation/Severe Weather
    memcpy(&pd->operationalMode, &buffer[bp], 2);
    bp += 2;
    pd->operationalMode = swapBytes(pd->operationalMode);


    // RDA Volume Coverage Pattern
    // @todo Document the VCP types and meanings
    memcpy(&pd->vcp, &buffer[bp], 2);
    bp += 2;
    pd->vcp = swapBytes(pd->vcp);


    // Sequence Number may not be useful.
    // Refer to Figure 3-4 of 2620001X.pdf
    memcpy(&pd->sequenceNum, &buffer[bp], 2);
    bp += 2;
    pd->sequenceNum = swapBytes(pd->sequenceNum);


    // This is a counter of volume scans for the radar site, it rolls back to one every 80 
    // volume scans. Not sure if this can be used in any meaningful way, but might be neat to
    // show if it's populated.
    memcpy(&pd->volScanNum, &buffer[bp], 2);
    bp += 2;
    pd->volScanNum = swapBytes(pd->volScanNum);


    // Volume Scan Date is stored as number of days since 1 Jan 1970.
    memcpy(&pd->volScanDate, &buffer[bp], 2);
    bp += 2;
    pd->volScanDate = swapBytes(pd->volScanDate);


    // Volume Scan Time is stored as number of seconds since Midnight.
    memcpy(&pd->volScanTime, &buffer[bp], 4);
    bp += 4;
    pd->volScanTime = swapBytes(pd->volScanTime);


    // @todo
    // Figure out the diference between volume scan date/time and product date/time
    memcpy(&pd->productDate, &buffer[bp], 2);
    bp += 2;
    pd->productDate = swapBytes(pd->productDate);

    memcpy(&pd->productTime, &buffer[bp], 4);
    bp += 4;
    pd->productTime = swapBytes(pd->productTime);


    // Reference TABLE V for Product dependant parameters 1 and 2.
    memcpy(&pd->h27_28, &buffer[bp], 4);
    bp += 4;
    

    // Elevation number within volume scan has range of 0-20
    memcpy(&pd->elevationNum, &buffer[bp], 2);        
    bp += 2;
    pd->elevationNum = swapBytes(pd->elevationNum);


    // Halfword 30 changes based on product, see TABLE V for parameter 3
    if (pd->productCode == 19 || pd->productCode == 94) 
    {
        memcpy(&pd->elevationAngle, &buffer[bp], 2);
        bp += 2;
        pd->elevationAngle = swapBytes(pd->elevationAngle);
    } 
    else 
    {
        memcpy(&pd->h30, &buffer[bp], 2);
        bp += 2;
    }


    // @todo
    memcpy(&pd->thresholdData, &buffer[bp], 32);
    bp += 32;


    // TABLE V parameters 4 - 10 @todo
    memcpy(&pd->h47_53, &buffer[bp], 14);
    bp += 14;

    bool compressed = false;
    if (pd->productCode == 94 || pd->productCode == 99)
    {
        compressed = !(pd->br.compressionMethod[0] == 0 && pd->br.compressionMethod[1] == 0);
    }


    // Product version number, might be interesting to show on screen.
    memcpy(&pd->version, &buffer[bp], 1);
    bp += 1;


    // Spot Blanking?
    // 1 - Spot Blanking ON
    // 0 - Spot Blanking OFF
    memcpy(&pd->spotBlank, &buffer[bp], 1);
    bp += 1;



    memcpy(&pd->symbologyOffset, &buffer[bp], 4);
    pd->symbologyOffset = swapBytes(pd->symbologyOffset);
    bp += 4;

    memcpy(&pd->graphicOffset, &buffer[bp], 4);
    pd->graphicOffset = swapBytes(pd->graphicOffset);
    bp += 4;

    memcpy(&pd->tabularOffset, &buffer[bp], 4);
    pd->tabularOffset = swapBytes(pd->tabularOffset);
    bp += 4;


    if (compressed)
    {
        LOGINF("Compressed data...");

        // @todo .... why no work
        //unsigned int srcLen = productLength - (messageHeaderLength + sizeof(pd->);
        unsigned int srcLen = fileLength - bp;

        /*u16 hi = 0;
        u16 lo = 0;
        memcpy(&hi, &pd->br.hiUncompProdSize[0], 2);
        memcpy(&lo, &pd->br.loUncompProdSize[0], 2);

        hi = swapBytes(hi);
        lo = swapBytes(lo);       

        u32 len = (hi << 16) | lo;*/

        u32 temp = 0;
        memcpy(&temp, &pd->br.hiUncompProdSize[0], 4);
        u32 len = swapBytes(temp);

        char* compBuffer = (char*)malloc(len * sizeof(char));    
        unsigned int* plen = &len;
        int ret = BZ2_bzBuffToBuffDecompress(&compBuffer[0], plen, (char*)&buffer[bp], srcLen, 0, 4);

        if (ret != BZ_OK)
        {
            LOGERR("Something went wrong trying to decompress the file: %d\n", ret);
            // @todo
            // potential big boi leak here if application doesn't exit...

            bp = 0;
            if(buffer)
            {
                free(buffer);
                buffer = NULL;
            }
            
            return false;
        }

        bp = 0;
        free(buffer);
        buffer = NULL;

        unsigned char* tempBuffer = NULL;
        tempBuffer = (unsigned char*)realloc(buffer, len * sizeof(char));    
        if (tempBuffer != NULL)
        {
            buffer = tempBuffer;
        }
        else
        {
            LOGINF("Failed to reallocate buffer.\n");
            return false;
        }

        memcpy(buffer, compBuffer, len); 

        if (compBuffer) free(compBuffer);
    }

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

    bool result = false;
    if ((packetCode & 0xffff) == 0xaf1f ||
        (packetCode & 0xffff) == 16)
    {
        result = RadialImagePacket(buffer, bp, nexradProduct, wsrInfo, pd, packetCode);
    }    
    else if ((packetCode & 0xffff) == 0xba07) 
    {
        result = RasterImagePacket(buffer, bp, nexradProduct, wsrInfo, pd, packetCode);
    }
    else
    {
        // @todo
        // log error of unsupported data packet and exit @todo
        // big boi leaks @todo
        // @todo
        LOGINF("Unsupported packet type: %d\n", packetCode);
        return false;
    }

    return result;
}


s32 GetColorFromDbz(u8 level, f32 minDbz, f32 incDbz)
{
    f32 dbz = minDbz + (level * incDbz);
    s8 color = 0;
    if (dbz < 5) color = 0;
    else if (dbz < 10) color = 1;
    else if (dbz < 15) color = 2;
    else if (dbz < 20) color = 3;
    else if (dbz < 25) color = 4;
    else if (dbz < 30) color = 5;
    else if (dbz < 35) color = 6;
    else if (dbz < 40) color = 7;
    else if (dbz < 45) color = 8;
    else if (dbz < 50) color = 9;
    else if (dbz < 55) color = 10;
    else if (dbz < 60) color = 11;
    else if (dbz < 65) color = 12;
    else if (dbz < 70) color = 13;
    else if (dbz < 75) color = 14;
    else if (dbz >= 75) color = 15;

    return color;
}

// @todo
// Clean this up
s32 GetColorFromSpeed(u8 level, f32 minVal, f32 inc)
{
    s8 color = 0;
    if (level == 0)
    {
        color = 0;
    }

    else if (level == 1)
    {
        color = 15;
    }

    else
    {
        f32 vel = minVal + (level * inc);

        if (vel <= -99) color = 1;
        else if (vel <= -80) color = 2;
        else if (vel <= -60) color = 3;
        else if (vel <= -45) color = 4;
        else if (vel <= -20) color = 5;
        else if (vel <= -5) color = 6;
        //else if (vel <= -5) color = 7;???
        else if (vel <= 0) color = 8;
        else if (vel <= 5) color = 9;
        else if (vel <= 20) color = 10;
        else if (vel <= 45) color = 11;
        else if (vel <= 60) color = 12;
        else if (vel <= 80) color = 13;
        else if (vel <= 99) color = 14;        
    }

    return color;
}


bool RadialImagePacket(
    unsigned char* buffer, 
    u32 bp, 
    NexradProduct* nexradProduct, 
    WSR88DInfo* wsrInfo,
    ProductDescription* pd,
    s16 packetCode)
{
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

    LOGINF("Bin Count: %d\n", binCount);
    LOGINF("Radial Count: %d\n", radialCount);

    BinCount = binCount;
    RadialCount = radialCount;   

    // @todo
    f32 minDbz = swapBytes(pd->reflectivityThreshold.minimumDbz) * 0.1f;
    f32 incDbz = swapBytes(pd->reflectivityThreshold.dbzIncrement) * 0.1f;
    f32 dbzLevels = swapBytes(pd->reflectivityThreshold.levelCount);

    // init stuff
    int totalBinCount = BinCount * RadialCount;
    RangeBins = (RangeBin*)malloc(totalBinCount * sizeof(RangeBin));

    StartsArray = (s32*)malloc(totalBinCount * sizeof(*StartsArray));
    CountsArray = (s32*)malloc(totalBinCount * sizeof(*CountsArray));

    for(int i = 0; i < totalBinCount; i++)
    {
        RangeBins[i] = {};
        StartsArray[i] = i * 4;
        CountsArray[i] = 4;
    }     

    // page 3-109 IC Doc
    // scale factor is 230 / binCount, avoid fp errors by just making it 1 unless it needs to 
    // be something else.
    f32 fScaleFactor = 1.0f;
    if (binCount != 230)
        fScaleFactor = scaleFactor * 0.001f;

    LOGINF("Scale factor: %2.8f\n", fScaleFactor);

    u8 run_color;
    s8 run, colorIndex;
    color4 color;
    s16 rleCount;
    s16 i_angleStart, i_angleDelta;
    f32 f_angleStart, f_angleDelta;
    f32 f_elevationAngle = pd->elevationAngle * 0.1f;

    
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

        if ((packetCode & 0xffff) == 0xaf1f)
        {
            for (int j = 0; j < rleCount * 2; j++) 
            {            
                run_color = (u8)(buffer[bp]);
                bp += 1;

                run = (run_color & 0xf0) >> 4;
                colorIndex = (run_color & 0x0f);

                for (s8 k = 0; k < run; k++) 
                {               
                    CalcRangeBinLocation(
                        i, binIndex, wsrInfo->lon, wsrInfo->lat, 
                        nexradProduct->range, f_angleStart, f_angleDelta);

                    // color = ReflectivityMap[colorIndex];
                    SetRangeBin(i, binIndex, colorIndex);
                    binIndex += 1;
                }
            }
        }

        else
        {
            for (int j = 0; j < rleCount; j++)
            {
                run_color = (u8)buffer[bp];
                bp += 1;

                CalcRangeBinLocation(
                        i, binIndex, wsrInfo->lon, wsrInfo->lat, 
                        nexradProduct->range, f_angleStart, f_angleDelta);

                s32 color;

                if(nexradProduct->productCode == 99)
                    color = GetColorFromSpeed(run_color, minDbz, incDbz);
                else
                    color = GetColorFromDbz(run_color, minDbz, incDbz);
                

                

                SetRangeBin(i, binIndex, color);
                binIndex += 1;
            }
        }

    }

    return true;
}


bool RasterImagePacket(
    unsigned char* buffer, 
    u32 bp, 
    NexradProduct* nexradProduct, 
    WSR88DInfo* wsrInfo,
    ProductDescription* pd,
    s16 packetCode)
{
    s16 packetCode1, packetCode2;

    memcpy(&packetCode1, &buffer[bp], 2); 
    bp += 2;
    packetCode1 = swapBytes(packetCode1);    

    memcpy(&packetCode2, &buffer[bp], 2); 
    bp += 2;
    packetCode2 = swapBytes(packetCode2);    

    assert((packetCode1 & 0xffff) == 0x8000);
    assert((packetCode2 & 0xffff) == 0x00c0);


    s16 iStart, jStart;
    memcpy(&iStart, &buffer[bp], 2); 
    bp += 2;
    iStart = swapBytes(iStart);

    memcpy(&jStart, &buffer[bp], 2); 
    bp += 2;
    jStart = swapBytes(jStart);


    s16 xScale, yScale;
    memcpy(&xScale, &buffer[bp], 2); 
    bp += 2;
    xScale = swapBytes(xScale);

    // skip xScale fractional
    bp += 2;
    
    memcpy(&yScale, &buffer[bp], 2); 
    bp += 2;
    yScale = swapBytes(yScale);

    // skip xScale fractional
    bp += 2;


    s16 numberOfRows;
    memcpy(&numberOfRows, &buffer[bp], 2);
    bp += 2;
    numberOfRows = swapBytes(numberOfRows);


    s16 packingDescriptor;
    memcpy(&packingDescriptor, &buffer[bp], 2);
    bp += 2;
    packingDescriptor = swapBytes(packingDescriptor);

    assert(packingDescriptor == 2);


    // @todo
    // change this for realz
    BinCount = 920;
    RadialCount = numberOfRows;

    // init stuff
    // @todo
    // There may be a more efficient way to determine this size
    int gridCellCount = RadialCount * BinCount;
    RangeBins = (RangeBin*)malloc(gridCellCount * sizeof(RangeBin));

    StartsArray = (s32*)malloc(gridCellCount * sizeof(*StartsArray));
    CountsArray = (s32*)malloc(gridCellCount * sizeof(*CountsArray));

    for(int i = 0; i < gridCellCount; i++)
    {
        RangeBins[i] = {};
        StartsArray[i] = i * 4;
        CountsArray[i] = 4;
    }



    u8 run_color;
    s8 run, color;

    s32 colIndex;
    s16 rowBytes;
    for(int i = 0; i < numberOfRows; i++)
    {
        colIndex = 0;

        memcpy(&rowBytes, &buffer[bp], 2);
        bp += 2;
        rowBytes = swapBytes(rowBytes);

        for(int j = 0; j < rowBytes; j++)
        {
            run_color = (u8)buffer[bp];
            bp += 1;

            run = (run_color & 0xf0) >> 4;
            color = (run_color & 0x0f);

            for (int k = 0; k < run; k++)
            {
                SetRasterCell(wsrInfo->lon, wsrInfo->lat, 
                    numberOfRows, colIndex, i, nexradProduct->resolution, color);          

                colIndex += 1;
            }

        }
    }


    return true;
}