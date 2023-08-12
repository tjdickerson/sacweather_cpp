//
// Created by tjdic on 04/15/2022.
//

#ifndef _TJD_PRODUCT_DESC_H_
#define _TJD_PRODUCT_DESC_H_

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
typedef struct ReflProductDescription_t {
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

} ReflProductDescription;

#endif //_TJD_PRODUCT_DESC_H_