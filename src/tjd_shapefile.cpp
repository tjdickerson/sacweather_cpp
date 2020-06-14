//

#include "tjd_shapefile.h"
#include <cstdio>
#include <cstdlib>
#include <assert.h>

constexpr char* SHAPE_FILE_DIR = "C:\\shapes\\weather\\\0";

static unsigned char* INDEX_BUFFER;

void GenerateFilename(char* buffer, const char* name, const char* ext)
{   
    strcat(buffer, SHAPE_FILE_DIR);
    strcat(buffer, name);
    strcat(buffer, ".");
    strcat(buffer, ext);
    strcat(buffer, "\0");
}


bool ReadIndexFile(unsigned char* &buffer, const char* name)
{
    bool result = true;

    // read in shapefile index
    char indexFilename[256];
    indexFilename[0] = '\0';
    GenerateFilename(indexFilename, name, SF_INDEX_EXT);

    FILE* fp; 
    fp = fopen(indexFilename, "rb");
    if (!fp)
    {
        printf("Error opening file %s\n", indexFilename);
        return false;
    }    

    int fileLength = 0;
    fseek(fp, 0, SEEK_END);
    fileLength = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buffer = (unsigned char*)malloc(fileLength * sizeof(unsigned char));
    assert(buffer != NULL);

    int bytesRead = fread(buffer, 1, fileLength, fp);
    assert(bytesRead == fileLength);    

    if (fp) fclose(fp);    
    return result;
}


bool ReadShapeFile(ShapeData* shapeData, const char* name)
{
    bool result = true;
    unsigned char* sfIndex = NULL;
    unsigned int bp = 0;

    if (!ReadIndexFile(sfIndex, name) || sfIndex == NULL)
    {
        printf("Failed to load index file.\n");
        return false;
    }    

    ShapeFileHeader indexHeader = {};
    ShapeIndexRecord indexRec = {};

    memcpy(&indexHeader, sfIndex, sizeof(s32) * 9);
    bp += sizeof(s32) * 9;

    memcpy(&indexHeader.mbr, &sfIndex[bp], sizeof(f64) * 8);
    bp += sizeof(f64) * 8;

    indexHeader.fileLength = swapBytes(indexHeader.fileLength);
    indexHeader.fileCode = swapBytes(indexHeader.fileCode);
    assert(indexHeader.fileCode == 9994);

    if (indexHeader.shapeType != 3 && indexHeader.shapeType != 5)
    {
        printf("Unsupported shape type: %d\n", indexHeader.shapeType);
        return false;
    }    


    char shapeFilename[256];
    shapeFilename[0] = '\0';
    GenerateFilename(shapeFilename, name, SF_SHAPE_EXT);

    FILE *fp = fopen(shapeFilename, "rb");
    if (fp == NULL)
    {
        printf("Failed to open shapefile...\n");
        return false;
    }

    ShapeFileRecHeader sfRec = {};
    ShapeFile2DBR br = {};
    int bytesRead = 0;
    int recordShapeType = 0;    


    // fileLength is stored as the length of the file in 16-bit words
    while (bp < (indexHeader.fileLength * 2))
    {
        memcpy(&indexRec, &sfIndex[bp], sizeof(indexRec));
        bp += sizeof(indexRec);

        indexRec.offset = swapBytes(indexRec.offset);

        // here is where dbf file can be processed, it is small enough to be read entirely 
        // and then could be accessed here in relation to the shapefile itself

        fseek(fp, indexRec.offset * 2, SEEK_SET);

        // This may be something we can skip entirely... @todo
        bytesRead = fread(&sfRec, sizeof(s32), 2, fp);
        if (bytesRead != 2)
        {
            printf("Error while reading shapefile record header %d.\n", indexRec.offset);
            result = false;
            break;
        }

        bytesRead = fread(&recordShapeType, sizeof(int), 1, fp);
        assert(bytesRead == 1 && recordShapeType == indexHeader.shapeType);

        // bounding rectangle .. unused at the moment.
        bytesRead = fread(&br, sizeof(f64), 4, fp);
        assert(bytesRead == 4);

        // number of parts
        int nParts = 0;
        bytesRead = fread(&nParts, sizeof(s32), 1, fp);
        assert(bytesRead == 1);

        // number of points
        int nPoints = 0;
        bytesRead = fread(&nPoints, sizeof(s32), 1, fp);
        assert(bytesRead == 1);

        // change me if you stop using vector... @todo
        for(int i = 0; i < nParts; i++)
        {
            s32 pidx = 0;
            bytesRead = fread(&pidx, sizeof(s32), 1, fp);
            assert(bytesRead == 1);

            // We want all the shapefile data to be one long array of information for all shapes
            // instead of per-shape. Offset the starting by the current start points.
            pidx += shapeData->numPoints;

            shapeData->parts.push_back(pidx);
        }

        // change me if you stop using vector... @todo
        for(int i = 0; i < nPoints; i++)
        {
            v2f64 point = {};
            bytesRead = fread(&point, sizeof(v2f64), 1, fp);
            assert(bytesRead == 1);

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
            shapeData->counts.push_back(shapeData->numPoints * 2 - shapeData->parts.at(i));
    }


    if (fp) fclose(fp);
    if (sfIndex) free(sfIndex);
    printf("Shapefile Done.\n");

    return result;
}