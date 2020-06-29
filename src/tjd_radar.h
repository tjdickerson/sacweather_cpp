//

#ifndef __tjd_radar_h__

#include "tjd_share.h"
#include "sacw_api.h"
#include "nws_info.h"

typedef struct RangeBin_t
{
    v2f32 p1;
    v2f32 p2;
    v2f32 p3;
    v2f32 p4;
    color4 color;
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


void tjd_RadarInit();
void tjd_GetRadarRenderData(RenderBufferData* rbd, RenderVertData* rvd);
bool ParseNexradRadarFile(const char* filename, WSR88DInfo* wsrInfo, NexradProduct* nexradProduct);



// nws color scheme
static f32 ReflectivityMap[] = 
{
    0.20f,  0.20f,  0.20f,  0.20f,   // 0 - black transparent
    ColorHexToFloat(0x04), ColorHexToFloat(0xe9), ColorHexToFloat(0xe7), 0.20f,   
    ColorHexToFloat(0x01), ColorHexToFloat(0x9f), ColorHexToFloat(0xf4), 0.90f,   
    ColorHexToFloat(0x03), ColorHexToFloat(0x00), ColorHexToFloat(0xf4), 0.99f,   
    ColorHexToFloat(0x02), ColorHexToFloat(0xfd), ColorHexToFloat(0x02), 0.99f,   
    ColorHexToFloat(0x01), ColorHexToFloat(0xc5), ColorHexToFloat(0x01), 0.99f,   
    ColorHexToFloat(0x00), ColorHexToFloat(0x8e), ColorHexToFloat(0x00), 0.99f,   
    ColorHexToFloat(0xfd), ColorHexToFloat(0xf8), ColorHexToFloat(0x02), 0.99f,   
    ColorHexToFloat(0xe5), ColorHexToFloat(0xbc), ColorHexToFloat(0x00), 0.99f,   
    ColorHexToFloat(0xfd), ColorHexToFloat(0x95), ColorHexToFloat(0.00), 0.99f,   
    ColorHexToFloat(0xfd), ColorHexToFloat(0.00), ColorHexToFloat(0.00), 0.99f,   
    ColorHexToFloat(0xd4), ColorHexToFloat(0.00), ColorHexToFloat(0.00), 0.99f,   
    ColorHexToFloat(0xbc), ColorHexToFloat(0.00), ColorHexToFloat(0.00), 0.99f,   
    ColorHexToFloat(0xf8), ColorHexToFloat(0x00), ColorHexToFloat(0xfd), 0.99f,   
    ColorHexToFloat(0x98), ColorHexToFloat(0x54), ColorHexToFloat(0xc6), 0.99f,   
    ColorHexToFloat(0xff), ColorHexToFloat(0xff), ColorHexToFloat(0xff), 0.99f,   
};

static f32 VelocityMap[] = 
{
    0.20f,  0.20f,  0.20f,  0.20f,   // 0 - black transparent
    ColorHexToFloat(0x02), ColorHexToFloat(0xfc), ColorHexToFloat(0x02), 0.20f,   
    ColorHexToFloat(0x01), ColorHexToFloat(0xe4), ColorHexToFloat(0x01), 0.90f,   
    ColorHexToFloat(0x01), ColorHexToFloat(0xc5), ColorHexToFloat(0x01), 0.99f,   
    ColorHexToFloat(0x07), ColorHexToFloat(0xac), ColorHexToFloat(0x04), 0.99f,   
    ColorHexToFloat(0x06), ColorHexToFloat(0x8f), ColorHexToFloat(0x03), 0.99f,   
    ColorHexToFloat(0x04), ColorHexToFloat(0x72), ColorHexToFloat(0x02), 0.99f,   
    ColorHexToFloat(0x7c), ColorHexToFloat(0x97), ColorHexToFloat(0x7b), 0.99f,   
    ColorHexToFloat(0x98), ColorHexToFloat(0x77), ColorHexToFloat(0x77), 0.99f,   
    ColorHexToFloat(0x89), ColorHexToFloat(0x00), ColorHexToFloat(0.00), 0.99f,   
    ColorHexToFloat(0xa2), ColorHexToFloat(0.00), ColorHexToFloat(0.00), 0.99f,   
    ColorHexToFloat(0xb9), ColorHexToFloat(0.00), ColorHexToFloat(0.00), 0.99f,   
    ColorHexToFloat(0xd8), ColorHexToFloat(0.00), ColorHexToFloat(0.00), 0.99f,   
    ColorHexToFloat(0xef), ColorHexToFloat(0x00), ColorHexToFloat(0x00), 0.99f,   
    ColorHexToFloat(0xfe), ColorHexToFloat(0x00), ColorHexToFloat(0x00), 0.99f,   
    ColorHexToFloat(0x90), ColorHexToFloat(0x00), ColorHexToFloat(0xa0), 0.99f,   
};






//  Unsure about everything below this line
// -------------------------------------------------------------

typedef struct brda_t
{
    unsigned char maxReflectivity[8];
    unsigned char compressionMethod [2];
    unsigned char hiUncompProdSize[2];
    unsigned char loUncompProdSize[2];                
} brda;

// Product Codes (Table III) 2620001X.pdf
// code 19: NTR 1, Name Base Reflectivity, Res .54 x 1, Range 124, Data Level 16, Radial Image
// Data Structure Figure 3-6 sheet 2,6, 7
constexpr s32 RANGE = 124;
typedef struct ProductDescription_t {
    s16 divider;
    s32 lat;
    s32 lon;
    s16 height;
    s16 productCode;
    s16 operationalMode;
    s16 vcp;
    s16 sequenceNum;
    s16 volScanNum;
    s16 volScanDate;
    s32 volScanTime;
    s16 productDate;
    s32 productTime;
    unsigned char h27_28[4];
    s16 elevationNum;

    union {
        s16 elevationAngle;
        unsigned char h30[2];
    };

    unsigned char thresholdData[32];

    union 
    {
        unsigned char h47_53[14];
        brda br;
    };

    unsigned char version;
    unsigned char spotBlank;
    s32 symbologyOffset;
    s32 graphicOffset;
    s32 tabularOffset;
} ProductDescription;


typedef struct SymbologyHeader_t {
    s16 divider;
    s16 blockId;
    s32 blockLength;
    s16 layers;
    s16 layerDivider;
    s16 layerLength;
} SymbologyHeader;



#define __tjd_radar_h__
#endif