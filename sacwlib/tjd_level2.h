//
// Created by tjdic on 06/03/2021.
//

#ifndef _TJD_LEVEL2_H_
#define _TJD_LEVEL2_H_

#include "tjd_share.h"

struct RadialData
{
    s32 rangeBinCount;
    f32 angle;
    f32 * dbz;
};

struct L2Volume
{
    int radialCount;
    RadialData * radials;
};


void ReadLevel2File(BufferInfo* mainBuffer);

#endif //_TJD_LEVEL2_H_
