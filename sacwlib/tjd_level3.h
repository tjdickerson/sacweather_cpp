//
// Created by tjdic on 06/05/2021.
//

#ifndef _TJD_LEVEL3_H_
#define _TJD_LEVEL3_H_

#include "tjd_share.h"

struct L3Radial
{
    f32 startAngle;
    f32 angleDelta;
    s16 levelCount;
    s32 radialStartIndex;
};

struct L3RadialData
{
    s16 firstGate;
    s16 gateCount;
    s16 iCenter;
    s16 jCenter;
    f32 rangeScaleFactor;
    s16 radialCount;
    f32 minDbz;
    f32 incDbz;

    bool radialsValid;

    unsigned char* levels;
    L3Radial* radials;
};

struct Level3Archive
{
    f32 centerLat;
    f32 centerLon;

    bool valid;

    L3RadialData radial;
};

extern Level3Archive g_L3Archive;

void ReadLevel3File(BufferInfo* buffer);

#endif //_TJD_LEVEL3_H_
