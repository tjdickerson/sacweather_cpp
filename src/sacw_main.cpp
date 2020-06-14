//

#include "sacw_api.h"
#include "sacw_extern.h"

#include "tjd_shapefile.h"

void sacw_Init()
{
    ShapeData mapData = {};
    ReadShapeFile(&mapData, "st_us");
}


