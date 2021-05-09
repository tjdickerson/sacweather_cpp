//


#ifndef _tjd_conversions_h_

#include <cmath>
#include "tjd_share.h"

constexpr double PI = 3.1415926535897932384626433832795;
constexpr double QUARTER_PI = 0.785398163397448309616;
constexpr double HALF_PI = 1.57079632679;

inline f32 DegToRad(f32 deg);

inline f32 RadToDeg(f32 rad);

f32 ConvertLonToScreen(f32 lon);

f32 ConvertLatToScreen(f32 lat);


#define _tjd_conversions_h_
#endif