//

#ifndef _TJD_SHAPEFILE_H_

// @TODO:
//  - Replace vector?


#include "tjd_share.h"
#include <vector>

constexpr char* SF_INDEX_EXT = "shx";
constexpr char* SF_SHAPE_EXT = "shp";
constexpr char* SF_DBASE_EXT = "dbf";

constexpr short FieldDescriptorStartOffset = 68; // ? is this valid @todo
constexpr uint8_t DBF_FIELD_DESC_ARRAY_TERM = 0x0D;
constexpr uint8_t DBF_END_OF_FILE = 0x1A; // sometimes maybe @todo


/*
https://www.esri.com/library/whitepapers/pdfs/shapefile.pdf

The offset of a record in the main file is the number of 16-bit words from the start of the
main file to the first byte of the record header for the record.  Thus, the offset for the
firstrecord in the main file is 50, given the 100-byte header.
*/

typedef struct ShapeFile_2D_BR_t
{
    f64 minX;
    f64 minY;
    f64 maxX;
    f64 maxY;
} ShapeFile2DBR;

typedef struct ShapeFile_MBR_t 
{
    f64 minX;
    f64 minY;
    f64 maxX;
    f64 maxY;
    f64 minZ;
    f64 maxZ;
    f64 minM;
    f64 maxM;
} ShapeFile_MBR;

typedef struct ShapeFileHeader_t 
{
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

typedef struct ShapeIndexRecord_t
{
    s32 offset;
    s32 contentLength;
} ShapeIndexRecord;

typedef struct ShapeFileRecHeader_t 
{
    s32 recordNumber;
    s32 recordLength;
} ShapeFileRecHeader;

typedef struct TextMarker_t
{
    char name[128];
    v2f64 location;
} TextMarker;

typedef struct ShapeData_t
{
    s32 numParts;
    s32 numPoints;

    std::vector<TextMarker> marker;    

    std::vector<s32> parts;
    std::vector<s32> counts;
    std::vector<v2f64> points;
} ShapeData;


typedef struct DBaseFileHeader_t 
{
    unsigned char version_info;
    unsigned char last_updated_year;
    unsigned char last_updated_month;
    unsigned char last_updated_day;
    unsigned int num_records;
    unsigned short num_bytes_in_header;
    unsigned short num_bytes_in_record;
    unsigned char reserved_1[20];
} DBaseFileHeader;

typedef struct FieldDescriptor_t 
{
    unsigned char field_name[11];
    unsigned char field_type;
    uint32_t field_data_address;
    unsigned char field_length;
    unsigned char field_decimal_count;
    unsigned char reserved_1[2];
    unsigned char work_area_id;
    unsigned char reserved_2[2];
    unsigned char set_fields_flag;
    unsigned char reserved_3[8];
} FieldDescriptor;

typedef struct FieldPropertiesStructure_t 
{
    unsigned short num_props;
    unsigned short start_of_standard;
    unsigned short num_custom_props;
    unsigned short start_of_custom;
    unsigned short num_ref_integrity;
    unsigned short start_of_ref_integrity;
    unsigned short start_of_data;
    unsigned short struct_size;
} FieldPropertiesStructure;

typedef struct StandardPropArray_t 
{
    unsigned short generational_number;
    unsigned short table_field_offset;
    unsigned char property_type;
    unsigned char field_type;
    unsigned char constraint_check;
    unsigned char reserved_1[4];
    unsigned short offset_from_start_to_data;
    unsigned short width_of_field;
} StandardPropArray;


// 
//
bool ReadShapeFile(ShapeData* shapeData, const char* name);



#define _TJD_SHAPEFILE_H_
#endif