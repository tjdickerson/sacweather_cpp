

#ifndef __tjd_font_h__

#include "tjd_share.h"

void InitFont();
void RenderText();
void LoadTextBuffer(f32 x, f32 y, int count, const char* text);

#define __tjd_font_h__
#endif