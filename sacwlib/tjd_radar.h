//

#ifndef __tjd_radar_h__

#include "tjd_share.h"
#include "sacw_api.h"
#include "nws_info.h"


#define TJD_RADAR_OK    0


constexpr s64 NEXRAD_BASE_MS = 86400000;

// Converts nexrad date/time format to standard millisecond timestamp
static s64 NexradToTimestamp(u32 nexradDate, u32 nexradTime)
{
    // @todo
    // Do the nexrad times always indicate seconds or do they sometimes indicate milliseconds?
    // In theory:
    //  s32 multi = 1;
    //  if (isSeconds) multi = 1000

    return (nexradDate * NEXRAD_BASE_MS) + (nexradTime * 1000);
}


typedef struct RangeBin_t
{
    v2f32 p1;
    v2f32 p2;
    v2f32 p3;
    v2f32 p4;
    f32 colorIndex;
} RangeBin;


typedef struct RangeBinInfo_t
{
    s32 db;
    RangeBin* bin;
} RangeBinInfo;


typedef struct WSR88DInfo_t
{
    f32     lon;
    f32     lat;    
    char    name[5];
} WSR88DInfo;


extern RangeBin* RangeBins;


static color4 ReflectivityMap[] =
{
    { 0.20f,  0.20f,  0.20f,  0.20f },   // 0 - black transparent
    { ColorHexToFloat(0x04), ColorHexToFloat(0xe9), ColorHexToFloat(0xe7), 0.20f },   
    { ColorHexToFloat(0x01), ColorHexToFloat(0x9f), ColorHexToFloat(0xf4), 0.90f },   
    { ColorHexToFloat(0x03), ColorHexToFloat(0x00), ColorHexToFloat(0xf4), 0.99f },   
    { ColorHexToFloat(0x02), ColorHexToFloat(0xfd), ColorHexToFloat(0x02), 0.99f },   
    { ColorHexToFloat(0x01), ColorHexToFloat(0xc5), ColorHexToFloat(0x01), 0.99f },   
    { ColorHexToFloat(0x00), ColorHexToFloat(0x8e), ColorHexToFloat(0x00), 0.99f },   
    { ColorHexToFloat(0xfd), ColorHexToFloat(0xf8), ColorHexToFloat(0x02), 0.99f },   
    { ColorHexToFloat(0xe5), ColorHexToFloat(0xbc), ColorHexToFloat(0x00), 0.99f },   
    { ColorHexToFloat(0xfd), ColorHexToFloat(0x95), ColorHexToFloat(0.00), 0.99f },   
    { ColorHexToFloat(0xfd), ColorHexToFloat(0.00), ColorHexToFloat(0.00), 0.99f },   
    { ColorHexToFloat(0xd4), ColorHexToFloat(0.00), ColorHexToFloat(0.00), 0.99f },   
    { ColorHexToFloat(0xbc), ColorHexToFloat(0.00), ColorHexToFloat(0.00), 0.99f },   
    { ColorHexToFloat(0xf8), ColorHexToFloat(0x00), ColorHexToFloat(0xfd), 0.99f },   
    { ColorHexToFloat(0x98), ColorHexToFloat(0x54), ColorHexToFloat(0xc6), 0.99f },   
    { ColorHexToFloat(0xff), ColorHexToFloat(0xff), ColorHexToFloat(0xff), 0.99f },      
};


static color4 VelocityMap[] = 
{
    { 0.20f,  0.20f,  0.20f,  0.20f },   // 0 - black transparent
    { ColorHexToFloat(0x02), ColorHexToFloat(0xfc), ColorHexToFloat(0x02), 0.99f },   
    { ColorHexToFloat(0x01), ColorHexToFloat(0xe4), ColorHexToFloat(0x01), 0.99f },   
    { ColorHexToFloat(0x01), ColorHexToFloat(0xc5), ColorHexToFloat(0x01), 0.99f },   
    { ColorHexToFloat(0x07), ColorHexToFloat(0xac), ColorHexToFloat(0x04), 0.99f },   
    { ColorHexToFloat(0x06), ColorHexToFloat(0x8f), ColorHexToFloat(0x03), 0.99f },   
    { ColorHexToFloat(0x04), ColorHexToFloat(0x72), ColorHexToFloat(0x02), 0.99f },   
    { ColorHexToFloat(0x7c), ColorHexToFloat(0x97), ColorHexToFloat(0x7b), 0.99f },   
    { ColorHexToFloat(0x98), ColorHexToFloat(0x77), ColorHexToFloat(0x77), 0.99f },   
    { ColorHexToFloat(0x89), ColorHexToFloat(0x00), ColorHexToFloat(0.00), 0.99f },   
    { ColorHexToFloat(0xa2), ColorHexToFloat(0.00), ColorHexToFloat(0.00), 0.99f },   
    { ColorHexToFloat(0xb9), ColorHexToFloat(0.00), ColorHexToFloat(0.00), 0.99f },   
    { ColorHexToFloat(0xd8), ColorHexToFloat(0.00), ColorHexToFloat(0.00), 0.99f },   
    { ColorHexToFloat(0xef), ColorHexToFloat(0x00), ColorHexToFloat(0x00), 0.99f },   
    { ColorHexToFloat(0xfe), ColorHexToFloat(0x00), ColorHexToFloat(0x00), 0.99f },   
    { ColorHexToFloat(0x90), ColorHexToFloat(0x00), ColorHexToFloat(0xa0), 0.99f },   
};






//  Unsure about everything below this line
// -------------------------------------------------------------

struct BaseReflectivityDataArray_47
{
    unsigned char c_maxReflectivity[6];

    // @todo
    // Bits 5-15 (0-800)
    //      Bits 5-15 contains the delta time, in seconds, btween the last radial in the elevation scan used to
    //      create the product and the start of the volume scan.
    // Bits 0-4:
    //      0 – Non Supplemental Scan
    //      1 – SAILS Scan
    //      2 – MRLE Scan
    //
    unsigned char c_deltaTime[2];

    // halfword 51 contains 0 if no compression is applied
    // halfword 51 contains 1 if the data are compressed using bzip2
    s16 compressionMethod;

    u32 uncompressedSize;
};

typedef struct ReflectivityThreshold_t
{
    s16 minimumDbz;
    s16 dbzIncrement;
    s16 levelCount;
    unsigned char reserved[26];
} ReflectivityThreshold;

// Product Codes (Table III) 2620001X.pdf
// code 19: NTR 1, Name Base Reflectivity, Res .54 x 1, Range 124, Data Level 16, Radial Image
// Data Structure Figure 3-6 sheet 2,6, 7
constexpr s32 RANGE = 124;
typedef struct ProductDescription_t {
    s16 divider;
    f32 lat;
    f32 lon;
    s16 height;
    s16 productCode;
    s16 operationalMode;
    s16 vcp;
    s16 sequenceNum;
    s16 volScanNum;
    s64 volScanTimestamp;
    s64 prodGenTimestamp;

    // half words 27-28
    union
    {
        unsigned char h27_28[4];
    };

    s16 elevationNum;

    // half word 30
    union 
    {
        s16 elevationAngle;

        // stored as 16-bit integer to make endian reversal easier.
        u16 h30;
    };

    union 
    {
        unsigned char thresholdData[32];
        ReflectivityThreshold reflectivityThreshold;
    };

    // half words 47-53
    union 
    {
        unsigned char h47_53[14];

        // Can't do this because of padding issues. 14 bytes doesn't play nice.
        // BaseReflectivityDataArray_47 brda;
    };

    unsigned char version;
    unsigned char spotBlank;
    s32 symbologyOffset;
    s32 graphicOffset;
    s32 tabularOffset;

    //

    BaseReflectivityDataArray_47 brda;

} ProductDescription;


typedef struct SymbologyHeader_t {
    s16 divider;
    s16 blockId;
    s32 blockLength;
    s16 layers;
    s16 layerDivider;
    s16 layerLength;
} SymbologyHeader;



void tjd_RadarInit();
s32 tjd_GetRadarRenderData(RenderBufferData* rbd);
bool ParseNexradRadarFile(
    const char* filename, 
    WSR88DInfo* wsrInfo, 
    NexradProduct* nexradProduct, 
    ProductDescription* pd);



#define __tjd_radar_h__
#endif