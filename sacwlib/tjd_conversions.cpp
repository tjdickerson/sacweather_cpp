#include "tjd_conversions.h"

inline f32 DegToRad(f32 deg)
{
    return (deg * PI) / 180.0f;
}


inline f32 RadToDeg(f32 rad)
{
    return (rad * 180.0f) / PI;
}


f32 ConvertLonToScreen(f32 lon)
{
    return lon / 180.0f;
}

f32 ConvertLatToScreen(f32 lat)
{
    // Converts latitude into radians to apply Mercator projection scaling, then converts back
    // to degrees to scale down for screen coordinates.
    f32 result = DegToRad(lat);    
    result = log(tan(QUARTER_PI + (0.5f * result)));
    result = RadToDeg(result);

    return result / 180.0f;
}