// 

#ifndef _TJD_SHARE_H_

#include <cstdint>


typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef double      f64;
typedef float       f32;


typedef struct Vector4f32_t
{
	union 
	{
		f32 x, y, z, w;
		f32 r, g, b, a;
	};
	
} v4f32;

typedef struct Vector2s32_t
{
	s32 x, y;
} v2s32;

typedef struct Vector2f32_t
{
	s32 x, y;
} v2f32;


typedef v4f32 color4;


#define _TJD_SHARE_H_
#endif