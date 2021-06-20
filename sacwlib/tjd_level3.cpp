//
// Created by tjdic on 06/05/2021.
//

#include <cassert>
#include "tjd_level3.h"
#include "tjd_radar.h"
#include "bzlib.h"

// 2620001Y.pdf

Level3Archive g_L3Archive;

void readProductMessageHeader(BufferInfo* buffer)
{
    unsigned char message_header[18];
    ReadFromBuffer(message_header, buffer, 18);

    BufferInfo b = {};
    b.data = message_header;
    b.position = 0;
    b.length = 18;

    u16 message_code;
    ReadFromBuffer(&message_code, &b, 2);
    message_code = swapBytes(message_code);

    u16 nexrad_date;
    ReadFromBuffer(&nexrad_date, &b, 2);
    nexrad_date = swapBytes(nexrad_date);

    u32 nexrad_time;
    ReadFromBuffer(&nexrad_time, &b, 4);
    nexrad_time = swapBytes(nexrad_time);

    // the calculation to end of file seems more accurate than having to subtract header length?
    u32 message_length;
    ReadFromBuffer(&message_length, &b, 4);
    message_length = swapBytes(message_length);

    u16 source_id;
    ReadFromBuffer(&source_id, &b, 2);
    source_id = swapBytes(source_id);

    u16 destination_id;
    ReadFromBuffer(&destination_id, &b, 2);
    destination_id = swapBytes(destination_id);

    u16 num_blocks;
    ReadFromBuffer(&num_blocks, &b, 2);
    num_blocks = swapBytes(num_blocks);
}

void parseHalfWordForProduct(ProductDescription* pd, s32 hw)
{
    if (hw == 47)
    {
        BufferInfo temp = {};
        temp.data = pd->h47_53;
        temp.position = 0;
        temp.length = 14;

        if(pd->productCode == 94 || pd->productCode == 99)
        {
            pd->brda = {};

            // @todo
            // Verify this somehow
            ReadFromBuffer(pd->brda.c_maxReflectivity, &temp, 6);
            ReadFromBuffer(pd->brda.c_deltaTime, &temp, 2);

            ReadFromBuffer(&pd->brda.compressionMethod, &temp, 2);
            pd->brda.compressionMethod = SwapBytes(pd->brda.compressionMethod);

            ReadFromBuffer(&pd->brda.uncompressedSize, &temp, 4);
            pd->brda.uncompressedSize = SwapBytes(pd->brda.uncompressedSize);
        }
    }

    else if (hw == 31)
    {
        BufferInfo temp = {};
        temp.data = pd->thresholdData;
        temp.position = 0;
        temp.length = 32;

        if(pd->productCode == 94 || pd->productCode == 99)
        {
            g_L3Archive.radial.minDbz = (f32)SwapBytes(pd->reflectivityThreshold.minimumDbz) * 0.1f;
            g_L3Archive.radial.incDbz = (f32)SwapBytes(pd->reflectivityThreshold.dbzIncrement) * 0.1f;
        }
    }
}

void readProductDescription(ProductDescription* pd, BufferInfo* buffer)
{
    s32 lat;
    ReadFromBuffer(&lat, buffer, 4);
    lat = SwapBytes(lat);

    s32 lon;
    ReadFromBuffer(&lon, buffer, 4);
    lon = SwapBytes(lon);

    f32 latitude = (f32)lat * 0.001f;
    f32 longitude = (f32)lon * 0.001f;

    pd->lat = latitude;
    pd->lon = longitude;

    ReadFromBuffer(&pd->height, buffer, 2);
    pd->height = SwapBytes(pd->height);

    ReadFromBuffer(&pd->productCode, buffer, 2);
    pd->productCode = SwapBytes(pd->productCode);

    // Operational Mode of the WSR-88D
    // 0 - Maintenance
    // 1 - Clean Air
    // 2 - Precipitation/Severe Weather
    ReadFromBuffer(&pd->operationalMode, buffer, 2);
    pd->operationalMode = SwapBytes(pd->operationalMode);

    ReadFromBuffer(&pd->vcp, buffer, 2);
    pd->vcp = SwapBytes(pd->vcp);

    ReadFromBuffer(&pd->sequenceNum, buffer, 2);
    pd->sequenceNum = SwapBytes(pd->sequenceNum);

    ReadFromBuffer(&pd->volScanNum, buffer, 2);
    pd->volScanNum = SwapBytes(pd->volScanNum);


    u16 nexrad_scan_date;
    ReadFromBuffer(&nexrad_scan_date, buffer, 2);
    nexrad_scan_date = swapBytes(nexrad_scan_date);

    u32 nexrad_scan_start_time;
    ReadFromBuffer(&nexrad_scan_start_time, buffer, 4);
    nexrad_scan_start_time = swapBytes(nexrad_scan_start_time);

    pd->volScanTimestamp = NexradToTimestamp(nexrad_scan_date, nexrad_scan_start_time);


    u16 nexrad_gen_date;
    ReadFromBuffer(&nexrad_gen_date, buffer, 2);
    nexrad_gen_date = swapBytes(nexrad_gen_date);

    u32 nexrad_gen_time;
    ReadFromBuffer(&nexrad_gen_time, buffer, 4);
    nexrad_gen_time = swapBytes(nexrad_gen_time);

    pd->prodGenTimestamp = NexradToTimestamp(nexrad_gen_date, nexrad_gen_time);


    // product dependant 27-28
    ReadFromBuffer(&pd->h27_28, buffer, 4);

    ReadFromBuffer(&pd->elevationNum, buffer, 2);
    pd->elevationNum = SwapBytes(pd->elevationNum);

    // product dependant 30
    ReadFromBuffer(&pd->h30, buffer, 2);
    pd->h30 = SwapBytes(pd->h30);

    // product dependant 31-46
    ReadFromBuffer(&pd->thresholdData, buffer, 32);
    parseHalfWordForProduct(pd, 31);

    //product dependant 47-53
    ReadFromBuffer(&pd->h47_53, buffer, 14);
    parseHalfWordForProduct(pd, 47);

    ReadFromBuffer(&pd->version, buffer, 1);
    ReadFromBuffer(&pd->spotBlank, buffer, 1);

    ReadFromBuffer(&pd->symbologyOffset, buffer, 4);
    pd->symbologyOffset = SwapBytes(pd->symbologyOffset);

    ReadFromBuffer(&pd->graphicOffset, buffer, 4);
    pd->graphicOffset = SwapBytes(pd->graphicOffset);

    ReadFromBuffer(&pd->tabularOffset, buffer, 4);
    pd->tabularOffset = SwapBytes(pd->tabularOffset);
}

s32 readPacketCode16(BufferInfo* buffer)
{
    // Packet Code has already been read from the buffer.

    L3RadialData* radialData = &g_L3Archive.radial;

    ReadFromBuffer(&radialData->firstGate, buffer, 2);
    g_L3Archive.radial.firstGate = SwapBytes(g_L3Archive.radial.firstGate);

    ReadFromBuffer(&radialData->gateCount, buffer, 2);
    radialData->gateCount = SwapBytes(radialData->gateCount);

    ReadFromBuffer(&radialData->iCenter, buffer, 2);
    radialData->iCenter = SwapBytes(radialData->iCenter);

    ReadFromBuffer(&radialData->jCenter, buffer, 2);
    radialData->jCenter = SwapBytes(radialData->jCenter);


    s16 i_scaled_factor;
    ReadFromBuffer(&i_scaled_factor, buffer, 2);
    i_scaled_factor = SwapBytes(i_scaled_factor);

    radialData->rangeScaleFactor = (f32)i_scaled_factor * 0.001f;


    ReadFromBuffer(&radialData->radialCount, buffer, 2);
    radialData->radialCount = SwapBytes(radialData->radialCount);


    radialData->radials = (L3Radial*)malloc(radialData->radialCount * sizeof(L3Radial));

    // @todo
    // I may need to check this inside the for loop to make sure that there's the correct amount of memory.
    //
    // The RPG clips radials to 70 kft.  This could result in an odd number of bins in a radialData.
    // However, the radialData will always be on a halfword boundary, so the number of bytes in a radialData may
    // be number of bins in a radialData + 1
    s32 totalGateCount = radialData->radialCount * (radialData->gateCount + 1);
    radialData->levels = (unsigned char*)malloc(totalGateCount * sizeof(unsigned char));

    L3Radial* radial;
    s16 last_offset = 0;
    s32 valid_count = 0;
    for (int i = 0; i < radialData->radialCount; i++)
    {
        radial = &radialData->radials[i];
        radial->radialStartIndex = 0;

        if (i != 0)
        {
            radial->radialStartIndex = radialData->radials[i - 1].radialStartIndex + last_offset;
        }

        ReadFromBuffer(&radial->levelCount, buffer, 2);
        radial->levelCount = SwapBytes(radial->levelCount);
        last_offset = radial->levelCount;

        s16 i_start_angle;
        ReadFromBuffer(&i_start_angle, buffer, 2);
        i_start_angle = SwapBytes(i_start_angle);

        radial->startAngle = (f32)i_start_angle * 0.1f;

        s16 i_angle_delta;
        ReadFromBuffer(&i_angle_delta, buffer, 2);
        i_angle_delta = SwapBytes(i_angle_delta);

        radial->angleDelta = (f32)i_angle_delta * 0.1f;

        for (int j = 0; j < radial->levelCount; j++)
        {
            unsigned char level;
            ReadFromBuffer(&level, buffer, 1);

            // @todo
            // For products 32, 94, 153, 193, and 195, data level codes 0 and 1 correspond to 
            // "Below Threshold" and "Missing", respectively.
            // if (level < 2) continue;

            radialData->levels[radial->radialStartIndex + j] = level;
            valid_count += 1;
        }
    }
    
    radialData->radialsValid = true;
    return 0;
}

s32 readSymbologyBlock(BufferInfo* buffer)
{
    // should be a divider right here
    s16 divider;
    ReadFromBuffer(&divider, buffer, 2);
    divider = SwapBytes(divider);

    // should always be 1
    s16 block_id;
    ReadFromBuffer(&block_id, buffer, 2);
    block_id = SwapBytes(block_id);

    s32 block_length;
    ReadFromBuffer(&block_length, buffer, 4);
    block_length = SwapBytes(block_length);

    s16 num_layers;
    ReadFromBuffer(&num_layers, buffer, 2);
    num_layers = SwapBytes(num_layers);

    for (int i = 0; i < num_layers; i++)
    {
        ReadFromBuffer(&divider, buffer, 2);
        divider = SwapBytes(divider);

        s32 data_length;
        ReadFromBuffer(&data_length, buffer, 4);
        data_length = SwapBytes(data_length);

        s16 packet_code;
        ReadFromBuffer(&packet_code, buffer, 2);
        packet_code = SwapBytes(packet_code);

        // @todo
        // I might need to check for packet code and what product type this is.
        // Digital Radial Data Array and some arrow whatever may both be 16 according to the ref docs.
        if ((packet_code & 0xffff) == 16)
        {
            readPacketCode16(buffer);
        }
        else
        {
            LOGERR("Unsupported Packet Type %04x\n", packet_code);
            return 1;
        }
    }

    return 0;
}

void ReadLevel3File(BufferInfo* buffer)
{
    g_L3Archive = {};
    g_L3Archive.radial = {};
    ProductDescription pd = {};

    unsigned char text_header[31];
    ReadFromBuffer(text_header, buffer, 30);
    text_header[30] = '\0';

    s32 product_messages_start = buffer->position;
    readProductMessageHeader(buffer);

    // should be a divider right here
    s16 divider;
    ReadFromBuffer(&divider, buffer, 2);
    divider = SwapBytes(divider);

    assert(divider == -1);

    readProductDescription(&pd, buffer);

    // @todo
    // Figure out how to organize this stuff below

    BufferInfo product_buffer = {};

    if (pd.productCode == 94 || pd.productCode == 99)
    {
        if (pd.brda.compressionMethod == 1)
        {
            s32 compressed_size = (s32)buffer->length - buffer->position;
            // unsigned char* buffer_entry = &buffer->data[buffer->position];
            auto buffer_entry = GetBufferMarker(buffer);
            auto t_product_buffer = (unsigned char*)malloc(pd.brda.uncompressedSize * sizeof(unsigned char));

            unsigned int* plen = &pd.brda.uncompressedSize;
            int ret =
                BZ2_bzBuffToBuffDecompress((char*)t_product_buffer, plen, (char*)buffer_entry, compressed_size, 0, 4);

            if (ret != BZ_OK)
            {
                LOGERR("Failed to decompress Level-III product. Return code: %d\n", ret);
            }

            else
            {
                product_buffer.data = t_product_buffer;
                product_buffer.position = 0;
                product_buffer.length = pd.brda.uncompressedSize;
            }
        }

        else
        {
            product_buffer.data = GetBufferMarker(buffer);
            product_buffer.position = 0;
        }

        g_L3Archive.centerLat = pd.lat;
        g_L3Archive.centerLon = pd.lon;

        g_L3Archive.valid = true;

        if (pd.symbologyOffset != 0)
        {
            // In most cases this seems to be 0, but just in case...
            s32 offset = (pd.symbologyOffset * 2) - (buffer->position - product_messages_start);
            SetBufferPos(&product_buffer, offset);

            readSymbologyBlock(&product_buffer);
        }

        //

    }
}