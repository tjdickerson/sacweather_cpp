#include "tjd_conversions.h"

#define math_e 2.718281828f

f32 AdjustLonForProjection(f32 lon)
{
    return lon;
}

f32 AdjustLatForProjection(f32 lat)
{
    // Converts latitude into radians to apply Mercator projection scaling, then converts back
    // to degrees
    f32 result = DegToRad(lat);    
    result = log(tan(QUARTER_PI + (0.5f * result)));
    result = RadToDeg(result);

    return result;
}

f32 ScreenToY (f32 value)
{
    value = (value * PI) / 180.0f;
    f32 lat = 2 * atan(pow(math_e, value)) - (PI / 2.0f);
    lat = (180.0f * lat) / PI;

    return lat;
}

f32 ScreenToX (f32 value)
{
    f32 lon = (180.0f * value) / PI;

    return lon;
}


v2f32 AdjustCoordinatesForMap(f32 x, f32 y) 
{
    v2f32 p = {};

    p.x = (x * PI) * (1.0f / 180.0f);
    p.x = (180.0f * p.x) / PI;

    p.y = (y * PI) * (1.0f / 180.0f);
    p.y = log(tan((f32)QUARTER_PI + (0.5f * p.y)));
    p.y = (180.0f * p.y) / PI;

    return p;
}