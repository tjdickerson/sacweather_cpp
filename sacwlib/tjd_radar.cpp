// 

#include <cstdio>
#include <cstring>
#include <cmath>
#include <cassert>

#include "bzlib.h"

#include "tjd_conversions.h"
#include "tjd_radar.h"

s16 RadialCount = 1;
s16 BinCount = 1;

static NexradProduct* CurrentProduct;

s32* StartsArray;
s32* CountsArray;

RangeBin* RangeBins;
void CalcRangeBinLocations(f32 cx, f32 cy, f32 range);
void SetRangeBin(s32 radialIndex, s32 binIndex, s32 colorIndex);
RangeBin* GetRangeBin(s32 radialIndex, s32 binIndex);

// 

struct BufferInfo
{
    unsigned char* buffer;
    s32 position;
};

void readFromBuffer(void* dest, struct BufferInfo* bi, s32 length)
{
    memcpy(dest, &bi->buffer[bi->position], length);
    bi->position += length;
}

void seekBuffer(struct BufferInfo* bi, s32 jmp)
{
    bi->position += jmp;
}

void setBufferPos(struct BufferInfo* bi, s32 pos)
{
    bi->position = pos;
}

unsigned char peekBuffer(struct BufferInfo* bi, s32 jmp)
{
    return bi->buffer[bi->position];
}

//

bool RadialImagePacket(
    unsigned char* buffer,
    u32 bp,
    NexradProduct* nexradProduct,
    WSR88DInfo* wsrInfo,
    ProductDescription* pd,
    s16 packetCode
);

void SetRasterCell(f32 cx, f32 cy, s32 rowCount, s32 ix, s32 iy, f32 res, s32 colorIndex);

bool RasterImagePacket(
    unsigned char* buffer,
    u32 bp,
    NexradProduct* nexradProduct,
    WSR88DInfo* wsrInfo,
    ProductDescription* pd,
    s16 packetCode
);

void tjd_GetRadarRenderData(RenderBufferData* rbd)
{
    rbd->vertexCount = BinCount * RadialCount * 6;
    rbd->vertices = (f32*)malloc(rbd->vertexCount * 3 * sizeof(f32));

    int vi = 0;
    for (int i = 0; i < BinCount * RadialCount; i++)
    {
        rbd->vertices[vi++] = RangeBins[i].p1.x;
        rbd->vertices[vi++] = RangeBins[i].p1.y;
        rbd->vertices[vi++] = RangeBins[i].colorIndex;

        rbd->vertices[vi++] = RangeBins[i].p2.x;
        rbd->vertices[vi++] = RangeBins[i].p2.y;
        rbd->vertices[vi++] = RangeBins[i].colorIndex;

        rbd->vertices[vi++] = RangeBins[i].p3.x;
        rbd->vertices[vi++] = RangeBins[i].p3.y;
        rbd->vertices[vi++] = RangeBins[i].colorIndex;

        //

        rbd->vertices[vi++] = RangeBins[i].p3.x;
        rbd->vertices[vi++] = RangeBins[i].p3.y;
        rbd->vertices[vi++] = RangeBins[i].colorIndex;

        rbd->vertices[vi++] = RangeBins[i].p2.x;
        rbd->vertices[vi++] = RangeBins[i].p2.y;
        rbd->vertices[vi++] = RangeBins[i].colorIndex;

        rbd->vertices[vi++] = RangeBins[i].p4.x;
        rbd->vertices[vi++] = RangeBins[i].p4.y;
        rbd->vertices[vi++] = RangeBins[i].colorIndex;
    }
}

void CalcRangeBinLocation(
    s32 radialIndex,
    s32 binIndex,
    f32 cx,
    f32 cy,
    f32 range,
    f32 angleStart,
    f32 angleDelta
)
{
    f32 dx1, dy1, dx2, dy2;

    RangeBin* bin = GetRangeBin(radialIndex, binIndex);
    f32 sweepCenterLeft = angleStart - (angleDelta * 0.5f);
    f32 sweepCenterRight = angleStart + (angleDelta * 0.5f);

    dx1 = (binIndex + 1) * (range / (float)BinCount) / (cos(cy * PI / 180.0f));
    dy1 = (binIndex + 1) * (range / (float)BinCount);

    dx2 = (binIndex + 2) * (range / (float)BinCount) / (cos(cy * PI / 180.0f));
    dy2 = (binIndex + 2) * (range / (float)BinCount);


    // "bottom" left
    bin->p1.x = dx1 * sin(DegToRad(sweepCenterLeft));
    bin->p1.y = dy1 * cos(DegToRad(sweepCenterLeft));

    // "bottom" right
    bin->p2.x = dx1 * sin(DegToRad(sweepCenterRight));
    bin->p2.y = dy1 * cos(DegToRad(sweepCenterRight));

    // "top" left
    bin->p3.x = dx2 * sin(DegToRad(sweepCenterLeft));
    bin->p3.y = dy2 * cos(DegToRad(sweepCenterLeft));

    // "top" right
    bin->p4.x = dx2 * sin(DegToRad(sweepCenterRight));
    bin->p4.y = dy2 * cos(DegToRad(sweepCenterRight));




    // Convert the nautical mile results into lon/lat degree offsets
    bin->p1.x = (cx + (bin->p1.x / 60.0f));
    bin->p1.y = (cy + (bin->p1.y / 60.0f));

    bin->p2.x = (cx + (bin->p2.x / 60.0f));
    bin->p2.y = (cy + (bin->p2.y / 60.0f));

    bin->p3.x = (cx + (bin->p3.x / 60.0f));
    bin->p3.y = (cy + (bin->p3.y / 60.0f));

    bin->p4.x = (cx + (bin->p4.x / 60.0f));
    bin->p4.y = (cy + (bin->p4.y / 60.0f));


    // Convert to screen coordinates.
    bin->p1.x = ConvertLonToScreen(bin->p1.x);
    bin->p1.y = ConvertLatToScreen(bin->p1.y);

    bin->p2.x = ConvertLonToScreen(bin->p2.x);
    bin->p2.y = ConvertLatToScreen(bin->p2.y);

    bin->p3.x = ConvertLonToScreen(bin->p3.x);
    bin->p3.y = ConvertLatToScreen(bin->p3.y);

    bin->p4.x = ConvertLonToScreen(bin->p4.x);
    bin->p4.y = ConvertLatToScreen(bin->p4.y);
}

RangeBin* GetRangeBin(s32 radialIndex, s32 binIndex)
{
    RangeBin* bin = &(RangeBins[radialIndex + (binIndex * RadialCount)]);
    return bin;
}

void SetRangeBin(s32 radialIndex, s32 binIndex, s32 colorIndex)
{
    // int ci = (colorIndex * 4);
    // f32* colorMap = VelocityMap;
    RangeBin* bin = &(RangeBins[radialIndex + (binIndex * RadialCount)]);
    bin->colorIndex = colorIndex;
    // bin->color.r = colorMap[ci];
    // bin->color.g = colorMap[ci + 1];
    // bin->color.b = colorMap[ci + 2];
    // bin->color.a = colorMap[ci + 3];
}

void SetRasterCell(f32 cx, f32 cy, s32 rowCount, s32 ix, s32 iy, f32 res, s32 colorIndex)
{
    // moveme @todo
    f32 halfRes = res / 2.0f;
    f32 x, y;

    RangeBin* cell = &(RangeBins[iy + (ix * rowCount)]);


    // bottom left
    cell->p1.x = ((ix * res) - halfRes) - ((RadialCount * res) / 2.0f);
    cell->p1.y = ((iy * res) - halfRes) - ((RadialCount * res) / 2.0f);

    // bottom right
    cell->p2.x = ((ix * res) + halfRes) - ((RadialCount * res) / 2.0f);
    cell->p2.y = ((iy * res) - halfRes) - ((RadialCount * res) / 2.0f);

    // top left
    cell->p3.x = ((ix * res) - halfRes) - ((RadialCount * res) / 2.0f);
    cell->p3.y = ((iy * res) + halfRes) - ((RadialCount * res) / 2.0f);

    // top right
    cell->p4.x = ((ix * res) + halfRes) - ((RadialCount * res) / 2.0f);
    cell->p4.y = ((iy * res) + halfRes) - ((RadialCount * res) / 2.0f);



    // x adsjut
    cell->p1.x /= (cos(cy * PI / 180.0f));
    cell->p2.x /= (cos(cy * PI / 180.0f));
    cell->p3.x /= (cos(cy * PI / 180.0f));
    cell->p4.x /= (cos(cy * PI / 180.0f));


    // convert to lat/lon from nautical miles
    cell->p1.x = (cx + (cell->p1.x / 60.0f));
    cell->p1.y = (cy - (cell->p1.y / 60.0f));

    cell->p2.x = (cx + (cell->p2.x / 60.0f));
    cell->p2.y = (cy - (cell->p2.y / 60.0f));

    cell->p3.x = (cx + (cell->p3.x / 60.0f));
    cell->p3.y = (cy - (cell->p3.y / 60.0f));

    cell->p4.x = (cx + (cell->p4.x / 60.0f));
    cell->p4.y = (cy - (cell->p4.y / 60.0f));


    // convert to screen coords
    cell->p1.x = ConvertLonToScreen(cell->p1.x);
    cell->p1.y = ConvertLatToScreen(cell->p1.y);

    cell->p2.x = ConvertLonToScreen(cell->p2.x);
    cell->p2.y = ConvertLatToScreen(cell->p2.y);

    cell->p3.x = ConvertLonToScreen(cell->p3.x);
    cell->p3.y = ConvertLatToScreen(cell->p3.y);

    cell->p4.x = ConvertLonToScreen(cell->p4.x);
    cell->p4.y = ConvertLatToScreen(cell->p4.y);

    //cell->color = ReflectivityMap[colorIndex];
    cell->colorIndex = (f32)colorIndex;
    // int ci = (colorIndex * 4);
    // cell->color.r = ReflectivityMap[ci];
    // cell->color.g = ReflectivityMap[ci + 1];
    // cell->color.b = ReflectivityMap[ci + 2];
    // cell->color.a = ReflectivityMap[ci + 3];

}


/*void readFromBuffer(void* dest, unsigned char* src, s32* bufptr, s32 length)
{
    memcpy(dest, &src[*bufptr], length);
    *bufptr += length;    
}*/


/*void this_might_be_Product_Request_Message(unsigned char* buffer, s32* bufptr)
{
    // Reference 2620001Y.pdf
    // Figure 3-4. Product Request Message (Sheet 1)

    // skip divider
    *bufptr += 2;

    // Number of bytes in block, including block divider, in the Product Description Block 
    s16 msg_size;
    readFromBuffer(&msg_size, buffer, bufptr, 2);

    // Internal NEXRAD product code correspondi ng to a weather product in Table I 
    s16 product_code;
    readFromBuffer(&product_code, buffer, bufptr, 2);

    // Bit # Value Meaning 0 1 High Priority 0 0 Low Priority 1 1 Map Requested (Bit 0=MSB)
    s16 flags;
    readFromBuffer(&flags, buffer, bufptr, 2);

    // Monotonically increase for tracking of request
    s16 seq_num;
    readFromBuffer(&seq_num, buffer, bufptr, 2);        

    // -1 for continuous (RPS) product transmissio n. 1 to 9 for one-time requests, when 
    // Volume Scan Start Time of Product (halfwords 18, 19) is = - 1 (equivalent to PUP Repeat Count). 
    // NOTE: For RPS requests, the number of products requested is determined from the Number of Blocks 
    // fields of the Message Header.
    s16 prod_count;
    readFromBuffer(&prod_count, buffer, bufptr, 2);   
    
}*/

struct MessageHeaderInfo
{
    unsigned char type;
    s32 size;
};

MessageHeaderInfo readMessageHeader(struct BufferInfo* bi)
{
    u16 msg_size_hw;
    unsigned char c_msg_size_hw[2];
    readFromBuffer(c_msg_size_hw, bi, 2);

    msg_size_hw = c_msg_size_hw[0] << 8 | c_msg_size_hw[1];

    unsigned char rda_channel;
    readFromBuffer(&rda_channel, bi, 1);

    unsigned char msg_type;
    readFromBuffer(&msg_type, bi, 1);

    s16 seq_num;
    readFromBuffer(&seq_num, bi, 2);

    s16 nexrad_date;
    readFromBuffer(&nexrad_date, bi, 2);
    nexrad_date = swapBytes(nexrad_date);

    s32 nexrad_time;
    readFromBuffer(&nexrad_time, bi, 4);
    nexrad_time = swapBytes(nexrad_time);

    /** 2620002T.pdf
     * 6. A size value 65535 indicates that byte locations 12-15 are used to specify the message size, in bytes.  
     * This accommodates messages larger than 65534 halfwords.   This method of specifying size assumes the message is one segment.  
     * See note 7 . 7.   When the size field (byte location 0 and 1) value is 65535, bytes 12 and 13 denote the Most Significant 
     * Halfword of the message size while bytes 14 and 15 denote the Least Significant Halfword of the message size.   
     * The message is assumed one (1) segment with size expressed in bytes .  
     * */

    s16 seg_count;
    readFromBuffer(&seg_count, bi, 2);
    seg_count = swapBytes(seg_count);

    s16 seg_num;
    readFromBuffer(&seg_num, bi, 2);
    seg_num = swapBytes(seg_num);

    s32 msg_size = 0;

    if (msg_size_hw == 65535)
    {
        msg_size = seg_count << 16 | seg_num;
    }
    else if (msg_size_hw == 0)
    {
        msg_size = -1;
    }
    else
    {
        msg_size = msg_size_hw * 2;
    }

    return { msg_type, msg_size };
}

void readMessage15(struct BufferInfo* buffer)
{

    s16 nexrad_date;
    readFromBuffer(&nexrad_date, buffer, 2);
    nexrad_date = swapBytes(nexrad_date);

    s16 nexrad_time;
    readFromBuffer(&nexrad_time, buffer, 2);
    nexrad_time = swapBytes(nexrad_time);

    s16 elevation_segments;
    readFromBuffer(&elevation_segments, buffer, 2);
    elevation_segments = swapBytes(elevation_segments);

    /**
     * 
     * Number of Elevation Segments
     *  Repeat for each Elevation Segment
     *      Repeat for each Azimuth Segment
     *          Number of Range Zones 
     *              Range Zone
     *               -Op Code
     *               -End Range
     *                  Same as R1 & R2 for Range Zone 1 
     *                      ...
     *                  Same as R1 & R2 for # of Range Zones specified 
     * 
     */

    // may can allocation smaller amount of these based on elevation_segments.
    // need to figure out how to do this for real.

    for (int i = 0; i < elevation_segments; i++)
    {
        for (int j = 0; j < 360; j++)
        {
            s16 range_zones;
            readFromBuffer(&range_zones, buffer, 2);
            range_zones = swapBytes(range_zones);

            for (int k = 0; k < range_zones; k++)
            {
                s16 op_code;
                readFromBuffer(&op_code, buffer, 2);

                s16 end_range;
                readFromBuffer(&end_range, buffer, 2);
            }
        }
    }
}

void readMessage18(struct BufferInfo* buffer)
{
    // Some (most?) of the data in this message doesn't seem important. Might can skip some of it.
    // seekBuffer(buffer, 9467);
}

void readMessage3(struct BufferInfo* buffer)
{
    // seekBuffer(buffer, 480 * 2);
}

void readMessage5(struct BufferInfo* buffer)
{
    // Number of halfwords in message.
    s16 msg_size;
    readFromBuffer(&msg_size, buffer, 2);
    msg_size = swapBytes(msg_size);

    // @todo
    // seekBuffer(buffer, msg_size);

}

void readMessage2(struct BufferInfo* buffer)
{
    seekBuffer(buffer, 60 * 2);
}

const s32 VOLUME_DATA_PTR = 0;
const s32 ELEVATION_DATA_PTR = 1;
const s32 RADIAL_DATA_PTR = 2;
const s32 DM_REF_PTR = 3;
const s32 DM_VEL_PTR = 4;
const s32 DM_SW_PTR = 5;
const s32 DM_ZDR_PTR = 6;
const s32 DM_PHI_PTR = 7;
const s32 DM_RHO_PTR = 8;
const s32 DM_CFP_PTR = 9;

void readDataMoment(struct BufferInfo* buffer)
{
    unsigned char data_block_type;
    readFromBuffer(&data_block_type, buffer, 1);

    unsigned char data_moment_name[3];
    readFromBuffer(&data_moment_name, buffer, 3);

    u32 reserved;
    readFromBuffer(&reserved, buffer, 4);

    u16 data_moment_gates;
    readFromBuffer(&data_moment_gates, buffer, 2);
    data_moment_gates = swapBytes(data_moment_gates);

    // Range to center of first range gate
    // Scaled Int, range from 0 to 32768 (0.0 .. 32.768 after scaling back)
    u16 data_moment_range;
    readFromBuffer(&data_moment_range, buffer, 2);
    data_moment_range = swapBytes(data_moment_range);

    // Size of data moment sample interval
    // 0.25 .. 4.0 after scaling back
    u16 dmr_sample_interval;
    readFromBuffer(&dmr_sample_interval, buffer, 2);
    dmr_sample_interval = swapBytes(dmr_sample_interval);

    // Threshold parameter which specifies the minimum difference in echo power between two
    // resolution gates for them not to be labeled "overlayed"
    // 0.0 .. 20.0
    u16 t_over;
    readFromBuffer(&t_over, buffer, 2);
    t_over = swapBytes(t_over);

    // SNR threshold for valid data
    // -12.0 .. 20.0
    s16 snr_threshold;
    readFromBuffer(&snr_threshold, buffer, 2);
    snr_threshold = swapBytes(snr_threshold);

    // 0 = none
    // 1 = recombined azimuthal radials
    // 2 = recombined range gates
    // 3 = recombined radials and range gates to legacy resolution
    unsigned char control_flags;
    readFromBuffer(&control_flags, buffer, 1);

    // 8 or 16
    unsigned char dm_gate_bit_count;
    readFromBuffer(&dm_gate_bit_count, buffer, 1);

    // Scale value used to convert Data Moments from integer to floating point data
    // > 0 .. 65535
    u32 scale;
    readFromBuffer(&scale, buffer, 4);
    scale = swapBytes(scale);

    // Offset value used to convert Data Moments from integer to floating point data
    // 2.0 .. 65535
    u32 offset;
    readFromBuffer(&offset, buffer, 4);
    offset = swapBytes(offset);

    // ??
    unsigned char* moments = nullptr;

    if (strncmp((const char*)data_moment_name, "REF", 3) == 0)
    {
        moments = (unsigned char*)malloc(data_moment_gates * sizeof(unsigned char));
        for (int i = 0; i < data_moment_gates; i++)
        {
            readFromBuffer(&moments[i], buffer, 1);
        }
    }

    int break_here = 1;
}

void processVolumeDataType(BufferInfo* buffer)
{
    unsigned char data_type;
    readFromBuffer(&data_type, buffer, 1);

    assert(data_type == 'R');

    unsigned char data_name[3];
    readFromBuffer(&data_name, buffer, 3);

    u16 size_of_block;
    readFromBuffer(&size_of_block, buffer, 2);
    size_of_block = swapBytes(size_of_block);

    unsigned char version_maj;
    readFromBuffer(&version_maj, buffer, 1);

    unsigned char version_min;
    readFromBuffer(&version_min, buffer, 1);

    s32 lat;
    readFromBuffer(&lat, buffer, 4);

    s32 lon;
    readFromBuffer(&lon, buffer, 4);

    s16 site_height;
    readFromBuffer(&site_height, buffer, 2);
    site_height = swapBytes(site_height);

    u16 feedhorn_height;
    readFromBuffer(&feedhorn_height, buffer, 2);
    feedhorn_height = swapBytes(feedhorn_height);

    s32 calibration_constant;
    readFromBuffer(&calibration_constant, buffer, 4);

    s32 horiz_shv_tx_power;
    readFromBuffer(&horiz_shv_tx_power, buffer, 4);
    horiz_shv_tx_power = swapBytes(horiz_shv_tx_power);

    s32 vert_shv_tx_power;
    readFromBuffer(&vert_shv_tx_power, buffer, 4);
    vert_shv_tx_power = swapBytes(vert_shv_tx_power);

    s32 system_diff_ref;
    readFromBuffer(&system_diff_ref, buffer, 4);
    system_diff_ref = swapBytes(system_diff_ref);

    u16 vcp_num;
    readFromBuffer(&vcp_num, buffer, 2);
    vcp_num = swapBytes(vcp_num);

    u16 processing_status;
    readFromBuffer(&processing_status, buffer, 2);
    processing_status = swapBytes(processing_status);

}

void processElevationDataType(BufferInfo* buffer)
{
    unsigned char data_type;
    readFromBuffer(&data_type, buffer, 1);

    unsigned char data_name[3];
    readFromBuffer(&data_name, buffer, 3);

    u16 block_size;
    readFromBuffer(&block_size, buffer, 2);
    block_size = swapBytes(block_size);

    // -0.02 .. 0.002;
    s16 atmos;
    readFromBuffer(&atmos, buffer, 2);
    atmos = swapBytes(atmos);

    // Scaling constant used by the Signal Processor for this elevation to calculate reflectivity
    s32 calibration_constant;
    readFromBuffer(&calibration_constant, buffer, 4);
}

void processRadialDataType(BufferInfo* buffer)
{
    unsigned char data_type;
    readFromBuffer(&data_type, buffer, 1);

    unsigned char data_name[3];
    readFromBuffer(&data_name, buffer, 3);

    u16 block_size;
    readFromBuffer(&block_size, buffer, 2);
    block_size = swapBytes(block_size);

    // 115 .. 511 km
    u16 unamb_range;
    readFromBuffer(&unamb_range, buffer, 2);
    unamb_range = swapBytes(unamb_range);

    s32 horiz_noise_level;
    readFromBuffer(&horiz_noise_level, buffer, 4);

    s32 vert_noise_level;
    readFromBuffer(&vert_noise_level, buffer, 4);

    // 8 .. 35.61
    u16 nyquist_vel;
    readFromBuffer(&nyquist_vel, buffer, 2);
    nyquist_vel = swapBytes(nyquist_vel);

    u16 radial_flags;
    readFromBuffer(&radial_flags, buffer, 2);
    radial_flags = swapBytes(radial_flags);

    // -99.0 .. 99.0
    s32 horiz_calibration;
    readFromBuffer(&horiz_calibration, buffer, 4);

    // -99.0 .. 99.0
    s32 vert_calibration;
    readFromBuffer(&vert_calibration, buffer, 4);
}

void processDataBlocks(BufferInfo* buffer, const s32* offsetPointers, s32 pointerCount, s32 blockStartPos)
{
    for (int i = 0; i < pointerCount; i++)
    {
        s32 off_pos = blockStartPos + offsetPointers[i];
        setBufferPos(buffer, off_pos);

        unsigned char data_type = peekBuffer(buffer, 1);
        unsigned char data_name = peekBuffer(buffer, 2);

        if (data_type == 'R')
        {
            if (data_name == 'V')
                processVolumeDataType(buffer);

            if (data_name == 'E')
                processElevationDataType(buffer);

            if (data_name == 'R')
                processRadialDataType(buffer);
        }

        else if (data_type == 'D')
        {
            readDataMoment(buffer);
        }
    }

}

void readMessage31(BufferInfo* buffer)
{
    // 2620002T
    // Table XVII Digital Radar Data Generic Format Blocks (Message Type 31)

    s32 header_block_start = buffer->position;

    char radar_id[4];
    readFromBuffer(radar_id, buffer, 4);

    // Radial data collection time in milliseconds past midnight GMT
    u32 collection_time;
    readFromBuffer(&collection_time, buffer, 4);
    collection_time = swapBytes(collection_time);

    u16 nexrad_date;
    readFromBuffer(&nexrad_date, buffer, 2);
    nexrad_date = swapBytes(nexrad_date);

    s16 azimuth_num;
    readFromBuffer(&azimuth_num, buffer, 2);
    azimuth_num = swapBytes(azimuth_num);

    // @todo
    // This isn't how this works. It doesn't work to read into a 4 byte integer then swap it.
    // Range should be 0 .. 359.956055
    s32 b_azimuth_angle;
    readFromBuffer(&b_azimuth_angle, buffer, 4);
    b_azimuth_angle = swapBytes(b_azimuth_angle);

    f64 azimuth_angle = b_azimuth_angle * 0.01;
    LOGINF("Azimuth angle: %2.4f\n", azimuth_angle);

    // 0 = uncompressed
    // 1 = compressed using BZIP2
    // 2 = compressed using zlib
    // 3 = future use
    unsigned char compression_indicator;
    readFromBuffer(&compression_indicator, buffer, 1);

    unsigned char spare;
    readFromBuffer(&spare, buffer, 1);

    u16 radial_byte_length;
    readFromBuffer(&radial_byte_length, buffer, 2);
    radial_byte_length = swapBytes(radial_byte_length);

    unsigned char azimuth_res_scaling;
    readFromBuffer(&azimuth_res_scaling, buffer, 1);

    unsigned char radial_status;
    readFromBuffer(&radial_status, buffer, 1);

    unsigned char elevation_num;
    readFromBuffer(&elevation_num, buffer, 1);

    unsigned char cut_sector_num;
    readFromBuffer(&cut_sector_num, buffer, 1);

    u32 i_elevation_angle;
    readFromBuffer(&i_elevation_angle, buffer, 4);
    i_elevation_angle = swapBytes(i_elevation_angle);

    f64 elevation_angle = i_elevation_angle * 0.01;

    unsigned char spot_blanking;
    readFromBuffer(&spot_blanking, buffer, 1);

    // 0 = no indexing
    // 1 to 100 means indexing angle of 0.01 to 1.00
    unsigned char azimuth_indexing_mode;
    readFromBuffer(&azimuth_indexing_mode, buffer, 1);

    u16 data_block_count;
    readFromBuffer(&data_block_count, buffer, 2);
    data_block_count = swapBytes(data_block_count);

    // My interpretation of the reference document is that there is at least 4 and at most 10.
    // I'm not sure if you always read all 10 pointers, or if there are only "data_block_count"
    // number of pointers in the data.
    s32 data_block_ptrs[10];
    for (s32 & data_block_ptr : data_block_ptrs)
    {
        readFromBuffer(&data_block_ptr, buffer, 4);
        data_block_ptr = swapBytes(data_block_ptr);
    }


    processDataBlocks(buffer, data_block_ptrs, data_block_count, header_block_start);

}

bool ParseNexradRadarFile(
    const char* filename,
    WSR88DInfo* wsrInfo,
    NexradProduct* nexradProduct,
    ProductDescription* pd
)
{
    u32 bp = 0;

    FILE* fp = NULL;
    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        // error reading file.
        LOGERR("Failed to open radar file %s\n", filename);
        return false;
    }

    fseek(fp, 0, SEEK_END);
    u32 fileLength = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Reading the whole file into a buffer and then parsing out the contents
    unsigned char* buffer = (unsigned char*)malloc(fileLength * sizeof(unsigned char));
    int bytesRead = fread(buffer, 1, fileLength, fp);
    fclose(fp);

    if (bytesRead != fileLength)
    {
        LOGERR("Failed to read radar file %s\n", filename);

        if (buffer) free(buffer);
        return false;
    }

    // Reference 2620010H.pdf 
    if (buffer[0] == 'A' && buffer[1] == 'R' && buffer[2] == '2')
    {
        // This is a level 2 radar file...

        // colume header
        char file_header[25];
        memcpy(file_header, &buffer[bp], 24);
        bp += 24;
        file_header[24] = '\0';

        { // Parse out the volume header info. Might be useful.
            unsigned char version[10];
            memcpy(version, &file_header[0], 9);
            version[9] = '\0';

            unsigned char ext_num[4];
            memcpy(ext_num, &file_header[9], 3);
            ext_num[3] = '\0';

            s32 nexrad_date;
            memcpy(&nexrad_date, &file_header[12], 4);

            s32 nexrad_time;
            memcpy(&nexrad_time, &file_header[16], 4);

            unsigned char icao[5];
            memcpy(icao, &file_header[20], 4);
            icao[4] = '\0';
        }



        // skip CTM Header
        /*char test1[3];
        memcpy(test1, &buffer[bp], 2); 
        test1[2] = '/0';


        bp += 12;
        char test2[3];
        memcpy(test2, &buffer[bp], 2); 
        test2[2] = '/0';*/

        // bp += 12;

        // 7.35 260010h.pdf  
        // 2432

        // is this just the metadata?
        u32 metadata_size = 325888;


        // assume the result cant be larger than 45mB?
        u32 uncompressed_size = 47185920;

        s32 compressed_size;
        memcpy(&compressed_size, &buffer[bp], 4);
        bp += 4;

        compressed_size = swapBytes(compressed_size);
        compressed_size = abs(compressed_size);

        unsigned char* uncompressed_data = (unsigned char*)malloc(uncompressed_size * sizeof(unsigned char));
        u32* p_len = &uncompressed_size;
        s32 ret = BZ2_bzBuffToBuffDecompress((char*)&uncompressed_data[0],
            p_len,
            (char*)&buffer[bp],
            compressed_size,
            0,
            4
        );

        bp += compressed_size;

        if (ret != BZ_OK)
        {
            LOGERR("Failed to decompress the Level-II radar data.\n");
        }

        else
        {
            const s32 TOTAL_SEG_SIZE = 2432;

            // From the 2620010H document:
            // The Archive II raw data format contains a 28-byte header.
            // The first 12 bytes are empty, which means the "Message Size" does not begin until byte 13 (halfword
            // 7 or full word 4). This 12 byte offset is due to legacy compliance (previously known as the "CTM
            // header").
            const s32 CTM_HEADER_SIZE = 12;

            // From what I can gather each message is supposed to be 2432 bytes. If we take the 12 CTM bytes,
            // and add what usually comes back from the Message Size Halfwords, there are 4 bytes left over.
            // I can't seem to find this in any of the reference documentation. This implies each message has
            // some padding between the message and the next message header.
            const s32 PADDING_BYTES = 4;

            //
            // 2620002T for message data definitions
            //

            BufferInfo msg_buffer = {};
            msg_buffer.buffer = uncompressed_data;
            msg_buffer.position = 0;

            MessageHeaderInfo msg_info = {};
            s32 pre_msg_buf_pos = 0, post_msg_buf_pos = 0;

            while (msg_buffer.position < uncompressed_size)
            {
                pre_msg_buf_pos = msg_buffer.position;
                seekBuffer(&msg_buffer, CTM_HEADER_SIZE);

                msg_info = readMessageHeader(&msg_buffer);

                if (msg_info.type == 15)
                {
                    readMessage15(&msg_buffer);
                }

                else if (msg_info.type == 18)
                {
                    readMessage18(&msg_buffer);
                }

                else if (msg_info.type == 3)
                {
                    readMessage3(&msg_buffer);
                }

                else if (msg_info.type == 5)
                {
                    readMessage5(&msg_buffer);
                }

                else if (msg_info.type == 2)
                {
                    readMessage2(&msg_buffer);
                }
                /**
                 * From 2620010H:
                 *  The message size defined in the message header is expressed using an alternate
                 *  method for specifying size for messages larger than 65534 halfwords (see
                 *  Interface Control Document for RDA/RPG, 2620002)
                 *
                 * From 2620002T:
                 *  Starting in Build 19, messages exchanged between the RDA and RPG are no longer segmented.
                 *  For messages smaller than 65534 halfwords, the number of message segments and message segment
                 *  numbers are set to 1.  For messages larger than 65534 halfwords, an alternate form
                 *  of message size definition is specified.
                 *
                 *  Number of Message Segments If the message size is less than 65534 halfwords, the number of
                 *  message segments is set to 1.  Otherwise, halfwords 12-15 specify the size of the
                 *  message, in bytes.
                 *
                 *  Message Segment Number. If the [message] size is less than 65534 halfwords, the message segment
                 *  number is set to 1.  Otherwise, halfwords 12-15 specify the size of the message, in bytes.
                 */

                s32 forward_size = TOTAL_SEG_SIZE;

                // @todo
                // Need to test/handle this...

                //if (msg_info.size < 0)
                //    forward_size = TOTAL_SEG_SIZE;

                //else
                //    forward_size = msg_info.size + PADDING_BYTES;


                setBufferPos(&msg_buffer, pre_msg_buf_pos + forward_size);
            }

            // might can free uncompressed_data here. @todo
            if (uncompressed_data) free(uncompressed_data);

            // is this now message 31?
            {
                // assume the result cant be larger than 45mB?
                uncompressed_size = 47185920;

                compressed_size = 0;
                memcpy(&compressed_size, &buffer[bp], 4);
                bp += 4;

                compressed_size = swapBytes(compressed_size);
                compressed_size = abs(compressed_size);

                uncompressed_data = (unsigned char*)malloc(uncompressed_size * sizeof(unsigned char));

                p_len = &uncompressed_size;
                ret = BZ2_bzBuffToBuffDecompress((char*)&uncompressed_data[0],
                    p_len,
                    (char*)&buffer[bp],
                    compressed_size,
                    0,
                    4
                );

                if (ret != BZ_OK)
                {
                    LOGERR("Failed to decompress the Level-II radar data.\n");
                }

                bp += compressed_size;


                BufferInfo radial_buffer = {};
                radial_buffer.buffer = uncompressed_data;
                radial_buffer.position = 0;

                // @todo
                // need to loop this and read all the messages

                seekBuffer(&radial_buffer, CTM_HEADER_SIZE);
                msg_info = readMessageHeader(&radial_buffer);
                if (msg_info.type == 31)
                {
                    readMessage31(&radial_buffer);
                }
            }


        }
    }

    // The file data for radar files starts off with a 30 bytes text header
    // @todo, can use this to verify it is a valid file?
    char textHeader[31];
    memcpy(textHeader, &buffer[bp], 30);
    bp += 30;
    textHeader[30] = '\0';


    // The next 18 bytes is the MessageHeader
    unsigned char messageHeaderBuffer[18];
    memcpy(messageHeaderBuffer, &buffer[bp], 18);
    bp += 18;

    int messageHeaderLength = 18;
    u32 productLength = 0;

    {
        int p = 8;
        memcpy(&productLength, &messageHeaderBuffer[p], 4);
        productLength = swapBytes(productLength);
    }


    // skipping the divider
    bp += 2;


    // The latitude and longitude of the radar site is stored as two 4 byte integers. They are 
    // treated as having a scale of three.
    memcpy(&pd->lat, &buffer[bp], 4);
    bp += 4;
    pd->lat = swapBytes(pd->lat);

    memcpy(&pd->lon, &buffer[bp], 4);
    bp += 4;
    pd->lon = swapBytes(pd->lon);

    f32 siteLat = pd->lat * 0.001f;
    f32 siteLon = pd->lon * 0.001f;

    wsrInfo->lat = siteLat;
    wsrInfo->lon = siteLon;


    // Height of the radar site in feet above sea level.
    memcpy(&pd->height, &buffer[bp], 2);
    bp += 2;
    pd->height = swapBytes(pd->height);


    // Product Code referencing which NEXRAD product is contained in this file.
    memcpy(&pd->productCode, &buffer[bp], 2);
    bp += 2;
    pd->productCode = swapBytes(pd->productCode);


    // Operational Mode of the WSR-88D
    // 0 - Maintenance
    // 1 - Clean Air
    // 2 - Precipitation/Severe Weather
    memcpy(&pd->operationalMode, &buffer[bp], 2);
    bp += 2;
    pd->operationalMode = swapBytes(pd->operationalMode);


    // RDA Volume Coverage Pattern
    // @todo Document the VCP types and meanings
    memcpy(&pd->vcp, &buffer[bp], 2);
    bp += 2;
    pd->vcp = swapBytes(pd->vcp);


    // Sequence Number may not be useful.
    // Refer to Figure 3-4 of 2620001X.pdf
    memcpy(&pd->sequenceNum, &buffer[bp], 2);
    bp += 2;
    pd->sequenceNum = swapBytes(pd->sequenceNum);


    // This is a counter of volume scans for the radar site, it rolls back to one every 80 
    // volume scans. Not sure if this can be used in any meaningful way, but might be neat to
    // show if it's populated.
    memcpy(&pd->volScanNum, &buffer[bp], 2);
    bp += 2;
    pd->volScanNum = swapBytes(pd->volScanNum);


    // Volume Scan Date is stored as number of days since 1 Jan 1970.
    memcpy(&pd->volScanDate, &buffer[bp], 2);
    bp += 2;
    pd->volScanDate = swapBytes(pd->volScanDate);


    // Volume Scan Time is stored as number of seconds since Midnight.
    memcpy(&pd->volScanTime, &buffer[bp], 4);
    bp += 4;
    pd->volScanTime = swapBytes(pd->volScanTime);


    // @todo
    // Figure out the diference between volume scan date/time and product date/time
    memcpy(&pd->productDate, &buffer[bp], 2);
    bp += 2;
    pd->productDate = swapBytes(pd->productDate);

    memcpy(&pd->productTime, &buffer[bp], 4);
    bp += 4;
    pd->productTime = swapBytes(pd->productTime);


    // Reference TABLE V for Product dependant parameters 1 and 2.
    memcpy(&pd->h27_28, &buffer[bp], 4);
    bp += 4;


    // Elevation number within volume scan has range of 0-20
    memcpy(&pd->elevationNum, &buffer[bp], 2);
    bp += 2;
    pd->elevationNum = swapBytes(pd->elevationNum);


    // Halfword 30 changes based on product, see TABLE V for parameter 3
    if (pd->productCode == 19 || pd->productCode == 94)
    {
        memcpy(&pd->elevationAngle, &buffer[bp], 2);
        bp += 2;
        pd->elevationAngle = swapBytes(pd->elevationAngle);
    }
    else
    {
        memcpy(&pd->h30, &buffer[bp], 2);
        bp += 2;
    }


    // @todo
    memcpy(&pd->thresholdData, &buffer[bp], 32);
    bp += 32;


    // TABLE V parameters 4 - 10 @todo
    memcpy(&pd->h47_53, &buffer[bp], 14);
    bp += 14;

    bool compressed = false;
    if (pd->productCode == 94 || pd->productCode == 99)
    {
        compressed = !(pd->br.compressionMethod[0] == 0 && pd->br.compressionMethod[1] == 0);
    }


    // Product version number, might be interesting to show on screen.
    memcpy(&pd->version, &buffer[bp], 1);
    bp += 1;


    // Spot Blanking?
    // 1 - Spot Blanking ON
    // 0 - Spot Blanking OFF
    memcpy(&pd->spotBlank, &buffer[bp], 1);
    bp += 1;

    memcpy(&pd->symbologyOffset, &buffer[bp], 4);
    pd->symbologyOffset = swapBytes(pd->symbologyOffset);
    bp += 4;

    memcpy(&pd->graphicOffset, &buffer[bp], 4);
    pd->graphicOffset = swapBytes(pd->graphicOffset);
    bp += 4;

    memcpy(&pd->tabularOffset, &buffer[bp], 4);
    pd->tabularOffset = swapBytes(pd->tabularOffset);
    bp += 4;

    if (compressed)
    {
        LOGINF("Compressed data...");

        // @todo .... why no work
        //unsigned int srcLen = productLength - (messageHeaderLength + sizeof(pd->);
        unsigned int srcLen = fileLength - bp;

        /*u16 hi = 0;
        u16 lo = 0;
        memcpy(&hi, &pd->br.hiUncompProdSize[0], 2);
        memcpy(&lo, &pd->br.loUncompProdSize[0], 2);

        hi = swapBytes(hi);
        lo = swapBytes(lo);       

        u32 len = (hi << 16) | lo;*/

        u32 temp = 0;
        memcpy(&temp, &pd->br.hiUncompProdSize[0], 4);
        u32 len = swapBytes(temp);

        char* compBuffer = (char*)malloc(len * sizeof(char));
        unsigned int* plen = &len;
        int ret = BZ2_bzBuffToBuffDecompress(&compBuffer[0], plen, (char*)&buffer[bp], srcLen, 0, 4);

        if (ret != BZ_OK)
        {
            LOGERR("Something went wrong trying to decompress the file: %d\n", ret);
            // @todo
            // potential big boi leak here if application doesn't exit...

            bp = 0;
            if (buffer)
            {
                free(buffer);
                buffer = NULL;
            }

            return false;
        }

        bp = 0;
        free(buffer);
        buffer = NULL;

        unsigned char* tempBuffer = nullptr;
        tempBuffer = (unsigned char*)realloc(buffer, len * sizeof(char));
        if (tempBuffer != nullptr)
        {
            buffer = tempBuffer;
        }
        else
        {
            LOGINF("Failed to reallocate buffer.\n");
            return false;
        }

        memcpy(buffer, compBuffer, len);

        if (compBuffer) free(compBuffer);
    }

    SymbologyHeader sh = {};
    memcpy(&sh, &buffer[bp], sizeof(sh));
    bp += sizeof(sh);

    sh.divider = swapBytes(sh.divider);
    sh.blockId = swapBytes(sh.blockId);
    sh.blockLength = swapBytes(sh.blockLength);
    sh.layers = swapBytes(sh.layers);
    sh.layerDivider = swapBytes(sh.layerDivider);
    sh.layerLength = swapBytes(sh.layerLength);

    s16 packetCode;
    memcpy(&packetCode, &buffer[bp], 2);
    packetCode = swapBytes(packetCode);
    bp += 2;

    bool result = false;
    if ((packetCode & 0xffff) == 0xaf1f ||
        (packetCode & 0xffff) == 16)
    {
        result = RadialImagePacket(buffer, bp, nexradProduct, wsrInfo, pd, packetCode);
    }
    else if ((packetCode & 0xffff) == 0xba07)
    {
        result = RasterImagePacket(buffer, bp, nexradProduct, wsrInfo, pd, packetCode);
    }
    else
    {
        // @todo
        // log error of unsupported data packet and exit @todo
        // big boi leaks @todo
        // @todo
        LOGINF("Unsupported packet type: %d\n", packetCode);
        return false;
    }

    return result;
}

s32 GetColorFromDbz(u8 level, f32 minDbz, f32 incDbz)
{
    f32 dbz = minDbz + (level * incDbz);
    s8 color = 0;
    if (dbz < 5) color = 0;
    else if (dbz < 10) color = 1;
    else if (dbz < 15) color = 2;
    else if (dbz < 20) color = 3;
    else if (dbz < 25) color = 4;
    else if (dbz < 30) color = 5;
    else if (dbz < 35) color = 6;
    else if (dbz < 40) color = 7;
    else if (dbz < 45) color = 8;
    else if (dbz < 50) color = 9;
    else if (dbz < 55) color = 10;
    else if (dbz < 60) color = 11;
    else if (dbz < 65) color = 12;
    else if (dbz < 70) color = 13;
    else if (dbz < 75) color = 14;
    else if (dbz >= 75) color = 15;

    return color;
}

// @todo
// Clean this up
s32 GetColorFromSpeed(u8 level, f32 minVal, f32 inc)
{
    s8 color = 0;
    if (level == 0)
    {
        color = 0;
    }

    else if (level == 1)
    {
        color = 15;
    }

    else
    {
        f32 vel = minVal + (level * inc);

        if (vel <= -99) color = 1;
        else if (vel <= -80) color = 2;
        else if (vel <= -60) color = 3;
        else if (vel <= -45) color = 4;
        else if (vel <= -20) color = 5;
        else if (vel <= -5) color = 6;
            //else if (vel <= -5) color = 7;???
        else if (vel <= 0) color = 8;
        else if (vel <= 5) color = 9;
        else if (vel <= 20) color = 10;
        else if (vel <= 45) color = 11;
        else if (vel <= 60) color = 12;
        else if (vel <= 80) color = 13;
        else if (vel <= 99) color = 14;
    }

    return color;
}

bool RadialImagePacket(
    unsigned char* buffer,
    u32 bp,
    NexradProduct* nexradProduct,
    WSR88DInfo* wsrInfo,
    ProductDescription* pd,
    s16 packetCode
)
{
    s16 firstBin;
    s16 iCenterSweep;
    s16 jCenterSweep;
    s16 binCount;
    s16 scaleFactor;
    s16 radialCount;

    memcpy(&firstBin, &buffer[bp], 2);
    bp += 2;
    memcpy(&binCount, &buffer[bp], 2);
    bp += 2;
    memcpy(&iCenterSweep, &buffer[bp], 2);
    bp += 2;
    memcpy(&jCenterSweep, &buffer[bp], 2);
    bp += 2;
    memcpy(&scaleFactor, &buffer[bp], 2);
    bp += 2;
    memcpy(&radialCount, &buffer[bp], 2);
    bp += 2;

    firstBin = swapBytes(firstBin);
    iCenterSweep = swapBytes(iCenterSweep);
    jCenterSweep = swapBytes(jCenterSweep);
    binCount = swapBytes(binCount);
    scaleFactor = swapBytes(scaleFactor);
    radialCount = swapBytes(radialCount);

    LOGINF("Bin Count: %d\n", binCount);
    LOGINF("Radial Count: %d\n", radialCount);

    BinCount = binCount;
    RadialCount = radialCount;

    // @todo
    f32 minDbz = swapBytes(pd->reflectivityThreshold.minimumDbz) * 0.1f;
    f32 incDbz = swapBytes(pd->reflectivityThreshold.dbzIncrement) * 0.1f;
    f32 dbzLevels = swapBytes(pd->reflectivityThreshold.levelCount);

    // init stuff
    int totalBinCount = BinCount * RadialCount;
    RangeBins = (RangeBin*)malloc(totalBinCount * sizeof(RangeBin));

    StartsArray = (s32*)malloc(totalBinCount * sizeof(*StartsArray));
    CountsArray = (s32*)malloc(totalBinCount * sizeof(*CountsArray));

    for (int i = 0; i < totalBinCount; i++)
    {
        RangeBins[i] = {};
        StartsArray[i] = i * 4;
        CountsArray[i] = 4;
    }

    // page 3-109 IC Doc
    // scale factor is 230 / binCount, avoid fp errors by just making it 1 unless it needs to 
    // be something else.
    f32 fScaleFactor = 1.0f;
    if (binCount != 230)
        fScaleFactor = scaleFactor * 0.001f;

    LOGINF("Scale factor: %2.8f\n", fScaleFactor);

    u8 run_color;
    s8 run, colorIndex;
    color4 color;
    s16 rleCount;
    s16 i_angleStart, i_angleDelta;
    f32 f_angleStart, f_angleDelta;
    f32 f_elevationAngle = pd->elevationAngle * 0.1f;

    int binIndex;
    for (int i = 0; i < radialCount; i++)
    {
        binIndex = 0;

        memcpy(&rleCount, &buffer[bp], sizeof(rleCount));
        bp += sizeof(rleCount);
        rleCount = swapBytes(rleCount);

        memcpy(&i_angleStart, &buffer[bp], sizeof(i_angleStart));
        bp += sizeof(i_angleStart);
        f_angleStart = swapBytes(i_angleStart) * 0.1f;

        memcpy(&i_angleDelta, &buffer[bp], sizeof(i_angleDelta));
        bp += sizeof(i_angleDelta);
        f_angleDelta = swapBytes(i_angleDelta) * 0.1f;

        if ((packetCode & 0xffff) == 0xaf1f)
        {
            for (int j = 0; j < rleCount * 2; j++)
            {
                run_color = (u8)(buffer[bp]);
                bp += 1;

                run = (run_color & 0xf0) >> 4;
                colorIndex = (run_color & 0x0f);

                for (s8 k = 0; k < run; k++)
                {
                    CalcRangeBinLocation(
                        i, binIndex, wsrInfo->lon, wsrInfo->lat,
                        nexradProduct->range, f_angleStart, f_angleDelta
                    );

                    // color = ReflectivityMap[colorIndex];
                    SetRangeBin(i, binIndex, colorIndex);
                    binIndex += 1;
                }
            }
        }

        else
        {
            for (int j = 0; j < rleCount; j++)
            {
                run_color = (u8)buffer[bp];
                bp += 1;

                CalcRangeBinLocation(
                    i, binIndex, wsrInfo->lon, wsrInfo->lat,
                    nexradProduct->range, f_angleStart, f_angleDelta
                );

                s32 color;

                if (nexradProduct->productCode == 99)
                    color = GetColorFromSpeed(run_color, minDbz, incDbz);
                else
                    color = GetColorFromDbz(run_color, minDbz, incDbz);

                SetRangeBin(i, binIndex, color);
                binIndex += 1;
            }
        }

    }

    return true;
}

bool RasterImagePacket(
    unsigned char* buffer,
    u32 bp,
    NexradProduct* nexradProduct,
    WSR88DInfo* wsrInfo,
    ProductDescription* pd,
    s16 packetCode
)
{
    s16 packetCode1, packetCode2;

    memcpy(&packetCode1, &buffer[bp], 2);
    bp += 2;
    packetCode1 = swapBytes(packetCode1);

    memcpy(&packetCode2, &buffer[bp], 2);
    bp += 2;
    packetCode2 = swapBytes(packetCode2);

    assert((packetCode1 & 0xffff) == 0x8000);
    assert((packetCode2 & 0xffff) == 0x00c0);

    s16 iStart, jStart;
    memcpy(&iStart, &buffer[bp], 2);
    bp += 2;
    iStart = swapBytes(iStart);

    memcpy(&jStart, &buffer[bp], 2);
    bp += 2;
    jStart = swapBytes(jStart);

    s16 xScale, yScale;
    memcpy(&xScale, &buffer[bp], 2);
    bp += 2;
    xScale = swapBytes(xScale);

    // skip xScale fractional
    bp += 2;

    memcpy(&yScale, &buffer[bp], 2);
    bp += 2;
    yScale = swapBytes(yScale);

    // skip xScale fractional
    bp += 2;

    s16 numberOfRows;
    memcpy(&numberOfRows, &buffer[bp], 2);
    bp += 2;
    numberOfRows = swapBytes(numberOfRows);

    s16 packingDescriptor;
    memcpy(&packingDescriptor, &buffer[bp], 2);
    bp += 2;
    packingDescriptor = swapBytes(packingDescriptor);

    assert(packingDescriptor == 2);


    // @todo
    // change this for realz
    BinCount = 920;
    RadialCount = numberOfRows;

    // init stuff
    // @todo
    // There may be a more efficient way to determine this size
    int gridCellCount = RadialCount * BinCount;
    RangeBins = (RangeBin*)malloc(gridCellCount * sizeof(RangeBin));

    StartsArray = (s32*)malloc(gridCellCount * sizeof(*StartsArray));
    CountsArray = (s32*)malloc(gridCellCount * sizeof(*CountsArray));

    for (int i = 0; i < gridCellCount; i++)
    {
        RangeBins[i] = {};
        StartsArray[i] = i * 4;
        CountsArray[i] = 4;
    }

    u8 run_color;
    s8 run, color;

    s32 colIndex;
    s16 rowBytes;
    for (int i = 0; i < numberOfRows; i++)
    {
        colIndex = 0;

        memcpy(&rowBytes, &buffer[bp], 2);
        bp += 2;
        rowBytes = swapBytes(rowBytes);

        for (int j = 0; j < rowBytes; j++)
        {
            run_color = (u8)buffer[bp];
            bp += 1;

            run = (run_color & 0xf0) >> 4;
            color = (run_color & 0x0f);

            for (int k = 0; k < run; k++)
            {
                SetRasterCell(
                    wsrInfo->lon, wsrInfo->lat,
                    numberOfRows, colIndex, i, nexradProduct->resolution, color
                );

                colIndex += 1;
            }

        }
    }

    return true;
}