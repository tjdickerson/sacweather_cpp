//

#ifndef _render_extern_h_

#include "tjd_share.h"


bool RenderInit();

void Render();

bool RenderCleanup();

void RenderViewportUpdate(f32 width, f32 height);


#define _render_extern_h_
#endif