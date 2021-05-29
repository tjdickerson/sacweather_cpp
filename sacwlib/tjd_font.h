

#ifndef __tjd_font_h__

#include "tjd_share.h"
#include "sacw_api.h"
#include "tjd_gl.h"

void InitFont(GLuint textureId);
void LoadTextBuffer(RenderBufferData* textBuffer, f32 lon, f32 lat, f32 scale, s32 count, const char* text);

#define __tjd_font_h__
#endif