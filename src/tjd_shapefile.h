//

#ifndef _TJD_SHAPEFILE_H_

// @TODO:
//  - Replace vector?


#include "tjd_share.h"
#include <vector>


/*
https://www.esri.com/library/whitepapers/pdfs/shapefile.pdf

The offset of a record in the main file is the number of 16-bit words from the start of the
main file to the first byte of the record header for the record.  Thus, the offset for the
firstrecord in the main file is 50, given the 100-byte header.
*/

typedef struct ShapeIndexRecord_t
{
    u32 offset;
    u32 contentLength;
} ShapeIndexRecord;

typedef struct ShapeFile_MBR_t {
    f64 minX;
    f64 minY;
    f64 maxX;
    f64 maxY;
    f64 minZ;
    f64 maxZ;
    f64 minM;
    f64 maxM;
} ShapeFile_MBR;

typedef struct ShapeFileHeader_t {
    s32 fileCode;
    u32 unused1;
    u32 unused2;
    u32 unused3;
    u32 unused4;
    u32 unused5;
    s32 fileLength;
    s32 version;
    s32 shapeType;
    ShapeFile_MBR mbr;
} ShapeFileHeader;

typedef struct ShapeFileRecHeader_t {
    s32 recordNumber;
    s32 recordLength;
} ShapeFileRecHeader;

typedef struct {
    f64 minX;
    f64 minY;
    f64 maxX;
    f64 maxY;
    s32 num_parts;
    s32 num_points;

    s32* parts;
    std::vector<Vec2f64> po
    ints;
} PolygonShape;

typedef struct ShapeFile_t {
    const char name[20];
    std::vector<PolygonShape> polygons;
} ShapeFile;



#define _TJD_SHAPEFILE_H_
#endif