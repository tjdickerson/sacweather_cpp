//

#include "tjd_shapefile.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <assert.h>

static unsigned char* INDEX_BUFFER;

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


bool ReadShapeFile(ShapeData* shapeData, const char* filepath)
{
    bool result = true;
    unsigned int bp = 0;

    BufferInfo index_buffer = {};

    if (!ReadIndexFile(&index_buffer, filepath) || index_buffer.length == 0)
    {
        LOGERR("Failed to load index file.\n");
        return false;
    }    

    ShapeFileHeader index_header = {};
    ShapeIndexRecord index_record = {};

    // @todo
    // This may not be a good way to do this. This might have issues with different systems.
    // Read in two chunks to get around struct alignment problems.
    ReadFromBuffer(&index_header, &index_buffer, sizeof(s32) * 9);
    ReadFromBuffer(&index_header.mbr, &index_buffer, sizeof(f64) * 8);

    index_header.fileLength = SwapBytes(index_header.fileLength);
    index_header.fileCode = SwapBytes(index_header.fileCode);
    assert(index_header.fileCode == 9994);

    if (index_header.shapeType != 3 && index_header.shapeType != 5)
    {
        LOGERR("Unsupported shape type: %d\n", index_header.shapeType);
        return false;
    }    


    // @todo
    // Maybe I should bite the bullet and just use string for these types of situations.
    char shapeFilename[256];
    shapeFilename[0] = '\0';
    GenerateFilename(shapeFilename, filepath, SF_SHAPE_EXT);

    FILE *fp = fopen(shapeFilename, "rb");
    if (fp == nullptr)
    {
        LOGERR("Failed to open shapefile...\n");
        return false;
    }

    ShapeFileRecHeader sf_rec_header = {};
    ShapeFile2DBR br = {};
    int bytes_read = 0;
    int record_shapte_type = 0;


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

    // Now there's enough data to generate the counts per-index.
    for (int i = 0; i < shapeData->numParts; i++)
    {
        if (i < (shapeData->numParts - 1))
            shapeData->counts.push_back(shapeData->parts.at(i + 1) - shapeData->parts.at(i));
        else
            shapeData->counts.push_back(shapeData->numPoints - shapeData->parts.at(i));
    }


    if (fp) fclose(fp);
    if (index_buffer.length > 0) free(index_buffer.data);
    LOGINF("Shapefile Done. (%s, %s)\n", shapeFilename, filepath);

    return result;
}