// 

#ifndef _TJD_SHARE_H_
#define _TJD_SHARE_H_

#include <cstdint>

#define TJD_LOG_ENABLED 1

// logging
#ifdef TJD_LOG_ENABLED

#define LOG_TAG "sacweather"

#ifdef __ANDROID__
#include <android/log.h>
#define LOGINF(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGERR(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#include <cstdio>
#include <cstring>

#define LOGINF(...) printf(__VA_ARGS__)
#define LOGERR(...) printf(__VA_ARGS__)
#endif

#endif

#define ColorHexToFloat(x) (x / (float)0xff)

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


static s16 swapBytes(s16 value) 
{
    return 
        (value & 0x00ff) << 8 | 
        (value & 0xff00) >> 8;
}

static u16 swapBytes(u16 value) 
{
    return 
        (value & 0x00ff) << 8 | 
        (value & 0xff00) >> 8;
}

static s32 swapBytes(s32 value) 
{
    return 
        (value & 0xff)          << 24   | 
        (value & 0xff0000)      >>  8   | 
        (value & 0xff00)        <<  8   | 
        (value & 0xff000000)    >> 24;
}

static u32 swapBytes(u32 value) 
{
    return 
        (value & 0xff)          << 24   | 
        (value & 0xff0000)      >>  8   | 
        (value & 0xff00)        <<  8   | 
        (value & 0xff000000)    >> 24;
}


typedef struct Vector4f32_t
{
    union { f32 x; f32 r; };
    union { f32 y; f32 g; };
    union { f32 z; f32 b; };
    union { f32 w; f32 a; };    
} v4f32;

typedef struct Vector2f64_t
{
    f64 x, y;
} v2f64;

typedef struct Vector2s32_t
{
    s32 x, y;
} v2s32;

typedef struct Vector2f32_t
{
    f32 x, y;
} v2f32;


typedef v4f32 color4;

struct BufferInfo
{
    unsigned char* buffer;
    s32 position;
    u32 totalSize;
};

static void readFromBuffer(void* dest, struct BufferInfo* bi, s32 length)
{
    memcpy(dest, &bi->buffer[bi->position], length);
    bi->position += length;
}

static void seekBuffer(struct BufferInfo* bi, s32 jmp)
{
    bi->position += jmp;
}

static void setBufferPos(struct BufferInfo* bi, s32 pos)
{
    bi->position = pos;
}

static unsigned char peekBuffer(struct BufferInfo* bi, s32 jmp)
{
    return bi->buffer[bi->position];
}

#endif