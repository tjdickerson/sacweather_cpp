// 

#ifndef _TJD_SHARE_H_
#define _TJD_SHARE_H_

#include <cstdint>
#include <cmath>

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


static s16 SwapBytes(s16 value)
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

static s32 SwapBytes(s32 value)
{
    return 
        (value & 0xff)          << 24   | 
        (value & 0xff0000)      >>  8   | 
        (value & 0xff00)        <<  8   | 
        (value & 0xff000000)    >> 24;
}

static u32 SwapBytes(u32 value)
{
    return
        (value & 0xff)          << 24   |
        (value & 0xff0000)      >>  8   |
        (value & 0xff00)        <<  8   |
        (value & 0xff000000)    >> 24;
}


// @todo
// how to manage that this is actually 4 bytes passed in
static f32 convertIEEE754(unsigned char* ieee754)
{
    u8 sign = ieee754[0] & 0x80;
    u8 expo = ((((ieee754[0] & 0x7f) << 1)) | (((ieee754[1] & 0x80) >> 7) & 0xff)) - 127;
    u32 frac = 0x800000 | (((ieee754[1] & 0x7f) << 16)) | ((ieee754[2] & 0xff) << 8) | (ieee754[3] & 0xFF);

    f32 result = 0.0f;
    f32 div;
    for (int i = 23; i >= 0; i--)
    {
        u32 check = (frac >> i) & 0x01;
        if (check == 1)
        {
            div = (f32)pow(2, 23 - i);
            result += 1.0f / div;
        }
    }

    result = result * (f32)pow(2, expo) * (sign == 0 ? 1.0f : -1.0f);
    return result;
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
    union { f32 min_x; f32 x; f32 r; };
    union { f32 min_y; f32 y; f32 g; };
    union { f32 max_x; f32 z; f32 b; };
    union { f32 max_y; f32 w; f32 a; };
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
    union { f32 x; f32 lon; };
    union { f32 y; f32 lat; };
} v2f32;


typedef v4f32 color4;

struct BufferInfo
{
    void* data;
    s32 position;
    u32 length;
};

static void InitBuffer(BufferInfo* buffer, void* data, u32 length)
{
    buffer->data = nullptr;
    buffer->position = 0;
    buffer->length = 0;

    if (data != nullptr)
    {
        buffer->data = data;
        buffer->length = length;
    }
}

static v2f32 MultiplyVectorV2f(v2f32 v1, v2f32 v2)
{
    v2f32 r = {};
    r.x = v1.x * v2.x;
    r.y = v1.y * v2.y;

    return r;
}

static f32 DistanceBetween(v2f32 v1, v2f32 v2)
{
    return (f32)sqrt(pow(v2.x - v1.x, 2) + pow(v2.y - v1.y, 2));
}

static v2f32 SubtractVectorV2f(v2f32 v1, v2f32 v2)
{
    v2f32 r = {};
    r.x = v2.x - v1.x;
    r.y = v2.y - v1.y;

    return r;
}

static v2f32 AddVectorV2f(v2f32 v1, v2f32 v2)
{
    v2f32 r = {};
    r.x = v2.x + v1.x;
    r.y = v2.y + v1.y;

    return r;
}


static void* GetBufferMarker(struct BufferInfo* buffer)
{
    return (void*)((char*)buffer->data + buffer->position);
}

static bool ReadFromBuffer(void* dest, struct BufferInfo* buffer, s32 length)
{
    if (buffer->position + length <= buffer->length)
    {
        memcpy(dest, (void*)((char*)buffer->data + buffer->position), length);
        buffer->position += length;

        return true;
    }

    return false;
}

static bool SeekBuffer(struct BufferInfo* buffer, s32 jmp)
{
    if (buffer->position + jmp < buffer->length)
    {
        buffer->position += jmp;
        return true;
    }

    return false;
}

static bool SetBufferPos(struct BufferInfo* buffer, s32 pos)
{
    if (pos >= 0 && pos < buffer->length)
    {
        buffer->position = pos;
        return true;
    }

    return false;
}

static unsigned char PeekBuffer(struct BufferInfo* buffer, s32 jmp)
{
    if (buffer->data != nullptr && (buffer->position + jmp) < buffer->length)
    {
        return *(((unsigned char*)buffer->data + buffer->position) + jmp);
    }

    return 0;
}

static void FreeBuffer(struct BufferInfo* buffer)
{
    if (buffer->data != nullptr)
    {
        free(buffer->data);
        buffer->data = nullptr;
    }
}

#endif