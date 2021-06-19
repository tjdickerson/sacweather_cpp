//

#include "tjd_shapefile.h"
#include "sacw_api.h"
#include "tjd_conversions.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <assert.h>

static unsigned char* INDEX_BUFFER;

constexpr u32 MA_INIT_ALLOCATION = 100 * 1024 * 1024;
constexpr u32 MAX_ALLOCATION = 250 * 1024 * 1024;
struct MemoryArena
{
    u32 currentAllocation;
};

struct ArrayInfo
{
    u32 _itemSize;
    u32 _totalAllocation;

    u32 length;
    unsigned char* data;
};

static void ArrayInit(ArrayInfo* array, u32 itemSize, u32 maxItemCount)
{
    array->_itemSize = itemSize;
    array->_totalAllocation = itemSize * maxItemCount;
    array->length = 0;

    array->data = (unsigned char*)malloc(array->_totalAllocation + 1);
}

static void ArrayFree(ArrayInfo* array)
{
    if (array->_totalAllocation > 0 && array->data != nullptr)
    {
        //free(array->data);
        array->_totalAllocation == 0;
        array->_itemSize = 0;
        array->length = 0;
    }
}

static void ArrayAdd(ArrayInfo* array, void* obj)
{
    if (array->data != nullptr && array->_itemSize + (array->length * array->_itemSize) < array->_totalAllocation)
    {
        memcpy(&array->data[array->length], obj, array->_itemSize);
        array->length += 1;
    }
}
// @todo
// I could store the directory in an application info struct.
// const char* ShapeFileDir = R"(C:\shapes\weather\)";

std::string generateFilename(const char* filepath, const char* ext)
{
    std::string result = filepath;
    result.append(".");
    result.append(ext);

    return result;
}

void GenerateFilename(char* buffer, const char* filepath, const char* ext)
{
    strcat(buffer, filepath);
    strcat(buffer, ".");
    strcat(buffer, ext);
    strcat(buffer, "\0");
}

bool ReadIndexFile(BufferInfo* buffer, const char* filepath)
{
    bool result = true;

    // read in shapefile index
    char index_filename[256];
    index_filename[0] = '\0';
    GenerateFilename(index_filename, filepath, SF_INDEX_EXT);

    FILE* fp = nullptr;
    fp = fopen(index_filename, "rb");
    if (!fp)
    {
        printf("Error opening file %s\n", index_filename);
        return false;
    }

    size_t file_length = 0;
    fseek(fp, 0, SEEK_END);
    file_length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    auto data = (unsigned char*)malloc(file_length * sizeof(unsigned char));
    assert(data != nullptr);

    int bytes_read = fread(data, 1, file_length, fp);
    assert(bytes_read == file_length);

    InitBuffer(buffer, data, bytes_read);

    fclose(fp);
    return result;
}

bool GenerateShapeBufferData(ShapeFileInfo* shapeFileInfo, RenderBufferData* renderData)
{
    if (!shapeFileInfo->pointsInitted)
    {
        LOGERR("Shape File data not initialized!\n");
        return false;
    }

    s32 i = 0;
    while (i < shapeFileInfo->totalNumPoints * 2)
    {
        renderData->vertices[i] = AdjustLonForProjection(shapeFileInfo->points[i]);
        renderData->vertices[i + 1] = AdjustLatForProjection(shapeFileInfo->points[i + 1]);
        i += 2;
    }

    return true;
}

static u32 GetFileLength(FILE* fp)
{
    u32 result = 0;
    if (fp != nullptr)
    {
        fseek(fp, 0, SEEK_END);
        result = ftell(fp);
        fseek(fp, 0, SEEK_SET);
    }

    return result;
}

bool readDbf(const char* filepath, std::string propName, std::vector<std::string>* names)
{
    BufferInfo _dbf_buffer = {};
    BufferInfo* dbf_buffer = &_dbf_buffer;

    std::vector<std::string> shapeNames;

    std::string filename = generateFilename(filepath, SF_DBASE_EXT);
    u32 items_read = 0;

    FILE* fp = fopen(filename.c_str(), "rb");
    if (fp == nullptr)
    {
        LOGINF("No DBF file found (%s)\n", filename.c_str());
        return false;
    }

    u32 file_length = GetFileLength(fp);
    auto data = (unsigned char*)calloc(file_length, sizeof(unsigned char));
    fread(&data[0], file_length, 1, fp);
    fclose(fp);

    InitBuffer(dbf_buffer, data, file_length);

    DBaseFileHeader dbf_header = {};
    ReadFromBuffer(&dbf_header, dbf_buffer, sizeof(dbf_header));

    std::vector<FieldDescriptor> field_descriptors;

    while (dbf_buffer->position < dbf_buffer->length)
    {
        FieldDescriptor field_descriptor = {};
        bool success = ReadFromBuffer(&field_descriptor, dbf_buffer, sizeof(field_descriptor));
        if (!success) break;

        field_descriptors.push_back(field_descriptor);

        if (PeekBuffer(dbf_buffer, 0) == 0x0D)
        {
            SeekBuffer(dbf_buffer, 1);
            break;
        }
    }

    SetBufferPos(dbf_buffer, dbf_header.num_bytes_in_header);
    auto record = (unsigned char*)calloc(dbf_header.num_bytes_in_record, 1);
    auto content = (unsigned char*)calloc(dbf_header.num_bytes_in_record, 1);
    for (int i = 0; i < dbf_header.num_records; i++)
    {
        ReadFromBuffer(record, dbf_buffer, dbf_header.num_bytes_in_record);

        unsigned char deletion_marker;
        memcpy(&deletion_marker, record, 1);
        bool deleted = (deletion_marker == 0x2A);

        void* record_cursor = &record[0];
        record_cursor = (void*)((char*)record_cursor + 1);

        for (auto field_descriptor : field_descriptors)
        {
            std::string field_name = reinterpret_cast<const char*>(field_descriptor.field_name);

            if (propName.compare((const char*)field_descriptor.field_name) == 0)
            {
                memcpy(content, record_cursor, field_descriptor.field_length);
                std::string shapeName = (const char*)content;
                shapeName.erase(shapeName.find_last_not_of(0x20) + 1);
                shapeNames.push_back(shapeName);
            }

            record_cursor = (void*)((char*)record_cursor + field_descriptor.field_length);
        }
    }

    names->assign(shapeNames.begin(), shapeNames.end());

    free(content);
    free(record);
    FreeBuffer(dbf_buffer);
    return true;
}

bool ReadShapeFile(ShapeFileInfo* shapeFileInfo, const char* filepath)
{
    bool result = true;

    BufferInfo index_buffer = {};

    if (!ReadIndexFile(&index_buffer, filepath) || index_buffer.length == 0)
    {
        LOGERR("Failed to load parts_offset file.\n");
        return false;
    }

    ShapeFileHeader index_header = {};
    ShapeIndexRecord index_record = {};
    ShapeIndexRecord shape_index_record = {};

    // @todo
    // This may not be a good way to do this. This might have issues with different systems.
    // Read in two chunks to get around struct alignment problems.
    ReadFromBuffer(&index_header, &index_buffer, sizeof(s32) * 9);
    ReadFromBuffer(&index_header.mbr, &index_buffer, sizeof(f64) * 8);

    index_header.fileLength = SwapBytes(index_header.fileLength);
    index_header.fileCode = SwapBytes(index_header.fileCode);
    assert(index_header.fileCode == 9994);

    u32 buffer_pos = index_buffer.position;
    u32 content_length = 0;
    s32 feature_count = 0;
    while (index_buffer.position < index_buffer.length)
    {
        ReadFromBuffer(&index_record, &index_buffer, sizeof(index_record));
        content_length += SwapBytes(index_record.contentLength);
        feature_count += 1;
    }

    SetBufferPos(&index_buffer, buffer_pos);


    // @todo
    // Maybe I should bite the bullet and just use string for these types of situations.
    char shapeFilename[256];
    shapeFilename[0] = '\0';
    GenerateFilename(shapeFilename, filepath, SF_SHAPE_EXT);

    FILE* fp = fopen(shapeFilename, "rb");
    if (fp == nullptr)
    {
        LOGERR("Failed to open shapefile...\n");
        return false;
    }

    u32 file_length = 0;
    fseek(fp, 0, SEEK_END);
    file_length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    BufferInfo _shape_buffer = {};
    BufferInfo* shape_buffer = &_shape_buffer;

    auto shape_file_data = (unsigned char*)malloc(file_length * sizeof(unsigned char));
    fread(shape_file_data, sizeof(unsigned char), file_length, fp);
    InitBuffer(shape_buffer, shape_file_data, file_length);

    shapeFileInfo->numFeatures = feature_count;
    shapeFileInfo->features = (ShapeData*)malloc(feature_count * sizeof(ShapeData));
    shapeFileInfo->totalNumPoints = 0;
    shapeFileInfo->totalNumParts = 0;

    // @todo
    // Memory Arena?
    shapeFileInfo->points = (f32*)malloc(file_length * sizeof(unsigned char));
    shapeFileInfo->starts = (s32*)malloc(file_length * sizeof(unsigned char));
    shapeFileInfo->counts = (s32*)malloc(file_length * sizeof(unsigned char));
    shapeFileInfo->pointsInitted = true;

    ShapeData* feature;
    s32 points_offset = 0;
    s32 point_accum = 0;
    s32 parts_offset = 0;
    s32 actual_feature_count = 0;

    // set color stuff here?
    shapeFileInfo->type = index_header.shapeType;

    bool dbfValid = false;
    std::vector<std::string> shapeNames;
    if (shapeFileInfo->displayNames)
    {
        dbfValid = readDbf(filepath, shapeFileInfo->propName, &shapeNames);
        shapeFileInfo->displayNames = dbfValid;
    }

    for (int i = 0; i < feature_count; i++)
    {


        ReadFromBuffer(&index_record, &index_buffer, sizeof(index_record));
        index_record.offset = SwapBytes(index_record.offset) * 2;

        // @todo
        // Temp hack for roads until I handle dbf properties better
        if (shapeFileInfo->category == SHAPE_ROADS)
        {
            shapeFileInfo->displayNames = false;
            std::string type = shapeNames.at(i);

/*            if (type == "I" || type == "Sthy")
            {
                int break_here = 1;
            }
            else
            {
                continue;
            }*/
        }

        SetBufferPos(shape_buffer, index_record.offset);

        feature = &shapeFileInfo->features[actual_feature_count];
        feature->partsIndex = parts_offset;
        actual_feature_count += 1;

        // Read in a feature.

        // @todo
        // Do I want to just skip these bytes? Nothing here seems useful except maybe record number if that
        // turns out to be needed. The DBF file should be sorted where the record from the parts_offset file itself
        // matches the same record number for both the shape file and the dbf file.
        ReadFromBuffer(&shape_index_record, shape_buffer, sizeof(index_record));

        s32 record_shape_type;
        ReadFromBuffer(&record_shape_type, shape_buffer, 4);
        assert(record_shape_type == index_header.shapeType);

        f64 bounding_box[4];
        ReadFromBuffer(bounding_box, shape_buffer, sizeof(f64) * 4);
        feature->boundingBox = {};
        feature->boundingBox.minX = (f32)bounding_box[0];
        feature->boundingBox.minY = (f32)bounding_box[1];
        feature->boundingBox.maxX = (f32)bounding_box[2];
        feature->boundingBox.maxY = (f32)bounding_box[3];

        if (shapeFileInfo->displayNames)
        {
            if (i < shapeFileInfo->numFeatures)
            {
                shapeFileInfo->featuresHaveText = true;

                std::string name = shapeNames.at(i);
                v2f32 location = v2f32
                    {
                        feature->boundingBox.minX + ((feature->boundingBox.maxX - feature->boundingBox.minX) / 2.0f),
                        feature->boundingBox.minY + ((feature->boundingBox.maxY - feature->boundingBox.minY) / 2.0f)
                    };
                feature->featureName.location = location;

                memcpy(&feature->featureName.text[0], name.c_str(), name.length() + 1);
                feature->featureName.textLength = name.length();
            }
        }

        s32 num_parts = 0;
        ReadFromBuffer(&num_parts, shape_buffer, 4);
        feature->numParts = num_parts;
        shapeFileInfo->totalNumParts += num_parts;

        s32 num_points = 0;
        ReadFromBuffer(&num_points, shape_buffer, 4);
        feature->numPoints = num_points;
        shapeFileInfo->totalNumPoints += num_points;

        for (int j = 0; j < num_parts; j++)
        {
            s32 this_start;
            ReadFromBuffer(&this_start, shape_buffer, 4);

            this_start += points_offset;
            shapeFileInfo->starts[parts_offset] = this_start;
            parts_offset += 1;
        }


        // set the counts
        for (int j = 0; j < num_parts; j++)
        {
            s32 idx = (parts_offset - num_parts) + j;
            s32 count_value = 0;

            if (j < (num_parts - 1))
            {
                count_value = shapeFileInfo->starts[idx + 1] - shapeFileInfo->starts[idx];
            }
            else
            {
                count_value = num_points - (shapeFileInfo->starts[idx] - points_offset);
            }

            shapeFileInfo->counts[idx] = count_value;
        }

        for (int j = 0; j < num_points; j++)
        {
            v2f64 point;
            ReadFromBuffer(&point, shape_buffer, sizeof(point));

            s32 point_index = point_accum;
            shapeFileInfo->points[point_index] = (f32)point.x;
            shapeFileInfo->points[point_index + 1] = (f32)point.y;
            point_accum += 2;
        }

        points_offset += num_points;
    }

    shapeFileInfo->numFeatures = actual_feature_count;

    // @todo
    // Handle if realloc returns 0/nullptr
    s32* newStarts = (s32*)realloc(shapeFileInfo->starts, shapeFileInfo->totalNumParts * sizeof(s32));
    shapeFileInfo->starts = newStarts;

    s32* newCounts = (s32*)realloc(shapeFileInfo->counts, shapeFileInfo->totalNumParts * sizeof(s32));
    shapeFileInfo->counts = newCounts;

    FreeBuffer(shape_buffer);

#if 0








    ShapeFileRecHeader sf_rec_header = {};
    ShapeFile2DBR br = {};
    int bytes_read = 0;
    int record_shapte_type = 0;




    // @todo
    // This loop seems to have some issues. It breaks out when the read for the record header fails, but
    // there should be a cleaner way to handle this.

    // fileLength is stored as the length of the file in 16-bit words
    while (bp < (index_header.fileLength * 2))
    {
        ReadFromBuffer(&index_record, &index_buffer, sizeof(index_record));
        index_record.offset = SwapBytes(index_record.offset);

        // here is where dbf file can be processed, it is small enough to be read entirely 
        // and then could be accessed here in relation to the shapefile itself

        fseek(fp, index_record.offset * 2, SEEK_SET);

        // This may be something we can skip entirely... @todo
        bytes_read = fread(&sf_rec_header, sizeof(s32), 2, fp);
        if (bytes_read != 2)
        {
            LOGERR("Error while reading shapefile record header %d.\n", index_record.offset);
            result = false;
            break;
        }

        bytes_read = fread(&record_shapte_type, sizeof(int), 1, fp);
        assert(bytes_read == 1 && record_shapte_type == index_header.shapeType);

        // bounding rectangle .. unused at the moment.
        bytes_read = fread(&br, sizeof(f64), 4, fp);
        assert(bytes_read == 4);

        // number of parts
        int nParts = 0;
        bytes_read = fread(&nParts, sizeof(s32), 1, fp);
        assert(bytes_read == 1);

        // number of points
        int nPoints = 0;
        bytes_read = fread(&nPoints, sizeof(s32), 1, fp);
        assert(bytes_read == 1);

        // change me if you stop using vector... @todo
        for(int i = 0; i < nParts; i++)
        {
            s32 pidx = 0;
            bytes_read = fread(&pidx, sizeof(s32), 1, fp);
            assert(bytes_read == 1);

            // We want all the shapefile data to be one long array of information for all shapes
            // instead of per-shape. Offset the starting by the current start points.            
            pidx += shapeData->numPoints;

            shapeData->parts.push_back(pidx);
        }

        // change me if you stop using vector... @todo
        for(int i = 0; i < nPoints; i++)
        {
            v2f64 point = {};
            bytes_read = fread(&point, sizeof(v2f64), 1, fp);
            assert(bytes_read == 1);

            shapeData->points.push_back(point);
        }     

        shapeData->numParts += nParts;
        shapeData->numPoints += nPoints;
    }

    // Now there's enough data to generate the counts per-parts_offset.
    for (int i = 0; i < shapeData->numParts; i++)
    {
        if (i < (shapeData->numParts - 1))
            shapeData->counts.push_back(shapeData->parts.at(i + 1) - shapeData->parts.at(i));
        else
            shapeData->counts.push_back(shapeData->numPoints - shapeData->parts.at(i));
    }

    fclose(fp);
    FreeBuffer(&index_buffer);
    LOGINF("Shapefile Done. (%s, %s)\n", shapeFilename, filepath);
#endif
    return result;
}