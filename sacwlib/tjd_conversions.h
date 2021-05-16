//


#ifndef _tjd_conversions_h_

#include <cmath>
#include "tjd_share.h"

constexpr double PI = 3.1415926535897932384626433832795;
constexpr double QUARTER_PI = 0.785398163397448309616;
constexpr double HALF_PI = 1.57079632679;

inline f32 DegToRad(f32 deg)
{
    return (deg * PI) / 180.0f;
}


inline f32 RadToDeg(f32 rad)
{
    return (rad * 180.0f) / PI;
}


f32 ConvertLonToScreen(f32 lon);

f32 ConvertLatToScreen(f32 lat);

f32 ScreenToY (f32 value);

f32 ScreenToX (f32 value);

v2f32 AdjustCoordinatesForMap(f32 x, f32 y);


#define _tjd_conversions_h_
#endif