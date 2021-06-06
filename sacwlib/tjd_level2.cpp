//
// Created by tjdic on 06/03/2021.
//

#include <cstring>
#include <cstdlib>
#include <cassert>

#include "bzlib.h"

#include "tjd_level2.h"

L2Volume g_L2Volume;

struct MessageHeaderInfo
{
    unsigned char type;
    s32 size;
};

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

MessageHeaderInfo readMessageHeader(struct BufferInfo* bi)
{
    u16 msg_size_hw;
    unsigned char c_msg_size_hw[2];
    ReadFromBuffer(c_msg_size_hw, bi, 2);

    msg_size_hw = c_msg_size_hw[0] << 8 | c_msg_size_hw[1];

    unsigned char rda_channel;
    ReadFromBuffer(&rda_channel, bi, 1);

    unsigned char msg_type;
    ReadFromBuffer(&msg_type, bi, 1);

    s16 seq_num;
    ReadFromBuffer(&seq_num, bi, 2);

    s16 nexrad_date;
    ReadFromBuffer(&nexrad_date, bi, 2);
    nexrad_date = SwapBytes(nexrad_date);

    s32 nexrad_time;
    ReadFromBuffer(&nexrad_time, bi, 4);
    nexrad_time = SwapBytes(nexrad_time);

    /** 2620002T.pdf
     * 6. A size value 65535 indicates that byte locations 12-15 are used to specify the message size, in bytes.
     * This accommodates messages larger than 65534 halfwords.   This method of specifying size assumes the message is one segment.
     * See note 7 . 7.   When the size field (byte location 0 and 1) value is 65535, bytes 12 and 13 denote the Most Significant
     * Halfword of the message size while bytes 14 and 15 denote the Least Significant Halfword of the message size.
     * The message is assumed one (1) segment with size expressed in bytes .
     * */

    s16 seg_count;
    ReadFromBuffer(&seg_count, bi, 2);
    seg_count = SwapBytes(seg_count);

    s16 seg_num;
    ReadFromBuffer(&seg_num, bi, 2);
    seg_num = SwapBytes(seg_num);

    s32 msg_size = 0;

    if (msg_size_hw == 65535)
    {
        msg_size = (seg_count << 16 | seg_num) + CTM_HEADER_SIZE;
    }

    else if (msg_type == 31)
    {
        msg_size = (msg_size_hw * 2) + CTM_HEADER_SIZE;
    }

    else
    {
        msg_size = TOTAL_SEG_SIZE;
    }


    return { msg_type, msg_size };
}

void readMessage15(struct BufferInfo* buffer)
{

    s16 nexrad_date;
    ReadFromBuffer(&nexrad_date, buffer, 2);
    nexrad_date = SwapBytes(nexrad_date);

    s16 nexrad_time;
    ReadFromBuffer(&nexrad_time, buffer, 2);
    nexrad_time = SwapBytes(nexrad_time);

    s16 elevation_segments;
    ReadFromBuffer(&elevation_segments, buffer, 2);
    elevation_segments = SwapBytes(elevation_segments);

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
            ReadFromBuffer(&range_zones, buffer, 2);
            range_zones = SwapBytes(range_zones);

            for (int k = 0; k < range_zones; k++)
            {
                s16 op_code;
                ReadFromBuffer(&op_code, buffer, 2);

                s16 end_range;
                ReadFromBuffer(&end_range, buffer, 2);
            }
        }
    }
}

void readMessage18(struct BufferInfo* buffer)
{
    // Some (most?) of the data in this message doesn't seem important. Might can skip some of it.
    // seekBuffer(data, 9467);
}

void readMessage3(struct BufferInfo* buffer)
{
    // seekBuffer(data, 480 * 2);
}

void readMessage5(struct BufferInfo* buffer)
{
    // Number of halfwords in message.
    s16 msg_size;
    ReadFromBuffer(&msg_size, buffer, 2);
    msg_size = SwapBytes(msg_size);

    // @todo
    // seekBuffer(data, msg_size);

}

void readMessage2(struct BufferInfo* buffer)
{
    seekBuffer(buffer, 60 * 2);
}

void readDataMoment(struct BufferInfo* buffer, s32 radialIndex)
{
    unsigned char data_block_type;
    ReadFromBuffer(&data_block_type, buffer, 1);

    unsigned char data_moment_name[3];
    ReadFromBuffer(&data_moment_name, buffer, 3);

    u32 reserved;
    ReadFromBuffer(&reserved, buffer, 4);

    u16 data_moment_gates;
    ReadFromBuffer(&data_moment_gates, buffer, 2);
    data_moment_gates = swapBytes(data_moment_gates);
    g_L2Volume.radials[radialIndex].gateCount = data_moment_gates;

    // Range to center of first range gate
    // Scaled Int, range from 0 to 32768 (0.0 .. 32.768 after scaling back)
    u16 data_moment_range;
    ReadFromBuffer(&data_moment_range, buffer, 2);
    data_moment_range = swapBytes(data_moment_range);
    g_L2Volume.radials[radialIndex].rangeToFirstGate = (f32)data_moment_range * 0.001f;

    // Size of data moment sample interval
    // 0.25 .. 4.0 after scaling back
    u16 dmr_sample_interval;
    ReadFromBuffer(&dmr_sample_interval, buffer, 2);
    dmr_sample_interval = swapBytes(dmr_sample_interval);

    // Threshold parameter which specifies the minimum difference in echo power between two
    // resolution gates for them not to be labeled "overlayed"
    // 0.0 .. 20.0
    u16 t_over;
    ReadFromBuffer(&t_over, buffer, 2);
    t_over = swapBytes(t_over);

    // SNR threshold for valid data
    // -12.0 .. 20.0
    s16 snr_threshold;
    ReadFromBuffer(&snr_threshold, buffer, 2);
    snr_threshold = SwapBytes(snr_threshold);

    // 0 = none
    // 1 = recombined azimuthal radials
    // 2 = recombined range gates
    // 3 = recombined radials and range gates to legacy resolution
    unsigned char control_flags;
    ReadFromBuffer(&control_flags, buffer, 1);

    // 8 or 16
    unsigned char dm_gate_bit_count;
    ReadFromBuffer(&dm_gate_bit_count, buffer, 1);

    // Scale value used to convert Data Moments from integer to floating point data
    // > 0 .. 65535
    f32 scale;
    unsigned char c_scale[4];
    ReadFromBuffer(c_scale, buffer, 4);
    scale = convertIEEE754(c_scale);

    // Offset value used to convert Data Moments from integer to floating point data
    // 2.0 .. 65535   // 42 84 00 00
    f32 offset;
    unsigned char c_offset[4];
    ReadFromBuffer(c_offset, buffer, 4);
    offset = convertIEEE754(c_offset);

    if (strncmp((const char*)data_moment_name, "REF", 3) == 0)
    {
        g_L2Volume.radials[radialIndex].dbz = (f32*)malloc(data_moment_gates * sizeof(f32));

        unsigned char value;
        f32 adjusted_value;

        for (int i = 0; i < data_moment_gates; i++)
        {
            ReadFromBuffer(&value, buffer, 1);

            // @todo
            // A scale value of 0 indicates floating point moment data for each range gate.

            // F = (N - OFFSET) / SCALE
            adjusted_value = ((f32)value - offset) / scale;
            g_L2Volume.radials[radialIndex].dbz[i] = adjusted_value;
        }
    }

    int break_here = 1;
}

void processVolumeDataType(BufferInfo* buffer)
{
    unsigned char data_type;
    ReadFromBuffer(&data_type, buffer, 1);

    assert(data_type == 'R');

    unsigned char data_name[3];
    ReadFromBuffer(&data_name, buffer, 3);

    u16 size_of_block;
    ReadFromBuffer(&size_of_block, buffer, 2);
    size_of_block = swapBytes(size_of_block);

    unsigned char version_maj;
    ReadFromBuffer(&version_maj, buffer, 1);

    unsigned char version_min;
    ReadFromBuffer(&version_min, buffer, 1);

    f32 lat;
    unsigned char c_lat[4];
    ReadFromBuffer(c_lat, buffer, 4);
    lat = convertIEEE754(c_lat);
    g_L2Volume.lat = lat;

    f32 lon;
    unsigned char c_lon[4];
    ReadFromBuffer(&lon, buffer, 4);
    lon = convertIEEE754(c_lon);
    g_L2Volume.lon = lon;

    s16 site_height;
    ReadFromBuffer(&site_height, buffer, 2);
    site_height = SwapBytes(site_height);

    u16 feedhorn_height;
    ReadFromBuffer(&feedhorn_height, buffer, 2);
    feedhorn_height = swapBytes(feedhorn_height);

    s32 calibration_constant;
    ReadFromBuffer(&calibration_constant, buffer, 4);

    s32 horiz_shv_tx_power;
    ReadFromBuffer(&horiz_shv_tx_power, buffer, 4);
    horiz_shv_tx_power = SwapBytes(horiz_shv_tx_power);

    s32 vert_shv_tx_power;
    ReadFromBuffer(&vert_shv_tx_power, buffer, 4);
    vert_shv_tx_power = SwapBytes(vert_shv_tx_power);

    s32 system_diff_ref;
    ReadFromBuffer(&system_diff_ref, buffer, 4);
    system_diff_ref = SwapBytes(system_diff_ref);

    u16 vcp_num;
    ReadFromBuffer(&vcp_num, buffer, 2);
    vcp_num = swapBytes(vcp_num);

    u16 processing_status;
    ReadFromBuffer(&processing_status, buffer, 2);
    processing_status = swapBytes(processing_status);

}

void processElevationDataType(BufferInfo* buffer)
{
    unsigned char data_type;
    ReadFromBuffer(&data_type, buffer, 1);

    unsigned char data_name[3];
    ReadFromBuffer(&data_name, buffer, 3);

    u16 block_size;
    ReadFromBuffer(&block_size, buffer, 2);
    block_size = swapBytes(block_size);

    // -0.02 .. 0.002;
    s16 atmos;
    ReadFromBuffer(&atmos, buffer, 2);
    atmos = SwapBytes(atmos);

    // Scaling constant used by the Signal Processor for this elevation to calculate reflectivity
    s32 calibration_constant;
    ReadFromBuffer(&calibration_constant, buffer, 4);
}

void processRadialDataType(BufferInfo* buffer)
{
    unsigned char data_type;
    ReadFromBuffer(&data_type, buffer, 1);

    unsigned char data_name[3];
    ReadFromBuffer(&data_name, buffer, 3);

    u16 block_size;
    ReadFromBuffer(&block_size, buffer, 2);
    block_size = swapBytes(block_size);

    // 11.5 .. 51.1 km
    u16 unamb_range;
    ReadFromBuffer(&unamb_range, buffer, 2);
    unamb_range = swapBytes(unamb_range);

    s32 horiz_noise_level;
    ReadFromBuffer(&horiz_noise_level, buffer, 4);

    s32 vert_noise_level;
    ReadFromBuffer(&vert_noise_level, buffer, 4);

    // 8 .. 35.61
    u16 nyquist_vel;
    ReadFromBuffer(&nyquist_vel, buffer, 2);
    nyquist_vel = swapBytes(nyquist_vel);

    u16 radial_flags;
    ReadFromBuffer(&radial_flags, buffer, 2);
    radial_flags = swapBytes(radial_flags);

    // -99.0 .. 99.0
    s32 horiz_calibration;
    ReadFromBuffer(&horiz_calibration, buffer, 4);

    // -99.0 .. 99.0
    s32 vert_calibration;
    ReadFromBuffer(&vert_calibration, buffer, 4);
}

void processDataBlocks(
    BufferInfo* buffer,
    s32 radialIndex,
    const s32* offsetPointers,
    s32 pointerCount,
    s32 blockStartPos)
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
            readDataMoment(buffer, radialIndex);
        }
    }

}

u32 readMessage31(BufferInfo* buffer, s32 radialIndex)
{
    // 2620002T
    // Table XVII Digital Radar Data Generic Format Blocks (Message Type 31)
    u32 return_code = 0;

    s32 header_block_start = buffer->position;

    char radar_id[4];
    ReadFromBuffer(radar_id, buffer, 4);

    // Radial data collection time in milliseconds past midnight GMT
    u32 collection_time;
    ReadFromBuffer(&collection_time, buffer, 4);
    collection_time = swapBytes(collection_time);

    u16 nexrad_date;
    ReadFromBuffer(&nexrad_date, buffer, 2);
    nexrad_date = swapBytes(nexrad_date);

    s16 azimuth_num;
    ReadFromBuffer(&azimuth_num, buffer, 2);
    azimuth_num = SwapBytes(azimuth_num);
    g_L2Volume.radials[radialIndex].azimuthNumber = azimuth_num;

    f32 azimuth_angle;
    unsigned char caa[4];
    ReadFromBuffer(caa, buffer, 4);
    azimuth_angle = convertIEEE754(caa);
    g_L2Volume.radials[radialIndex].azimuth = azimuth_angle;

    // 0 = uncompressed
    // 1 = compressed using BZIP2
    // 2 = compressed using zlib
    // 3 = future use
    unsigned char compression_indicator;
    ReadFromBuffer(&compression_indicator, buffer, 1);

    unsigned char spare;
    ReadFromBuffer(&spare, buffer, 1);

    // Uncompressed length of the radial in bytes including the Data Header block length
    u16 radial_byte_length;
    ReadFromBuffer(&radial_byte_length, buffer, 2);
    radial_byte_length = swapBytes(radial_byte_length);

    unsigned char azimuth_res_scaling;
    ReadFromBuffer(&azimuth_res_scaling, buffer, 1);

    unsigned char radial_status;
    ReadFromBuffer(&radial_status, buffer, 1);

    //  Start of new Elevation 00
    //  Intermediate Radial Data 01
    //  End of Elevation 02
    //  Beginning of Volume Scan 03
    //  End of Volume Scan 04
    //  Start of new Elevation - Last Elevation in VCP 05
    if (radial_status == 0x02)
    {
        // @todo
        // For the moment just read the first one and get out.
        return_code = 1;
    }

    unsigned char elevation_num;
    ReadFromBuffer(&elevation_num, buffer, 1);

    unsigned char cut_sector_num;
    ReadFromBuffer(&cut_sector_num, buffer, 1);

    f32 elevation_angle;
    ReadFromBuffer(&elevation_angle, buffer, 4);

    unsigned char spot_blanking;
    ReadFromBuffer(&spot_blanking, buffer, 1);

    // 0 = no indexing
    // 1 to 100 means indexing angle of 0.01 to 1.00
    unsigned char azimuth_indexing_mode;
    ReadFromBuffer(&azimuth_indexing_mode, buffer, 1);

    u16 data_block_count;
    ReadFromBuffer(&data_block_count, buffer, 2);
    data_block_count = swapBytes(data_block_count);

    // My interpretation of the reference document is that there is at least 4 and at most 10.
    // I'm not sure if you always read all 10 pointers, or if there are only "data_block_count"
    // number of pointers in the data.
    s32 data_block_ptrs[10];
    for (s32& data_block_ptr : data_block_ptrs)
    {
        ReadFromBuffer(&data_block_ptr, buffer, 4);
        data_block_ptr = SwapBytes(data_block_ptr);
    }

    processDataBlocks(buffer, radialIndex, data_block_ptrs, data_block_count, header_block_start);

    return return_code;
}

void ProcessLdmRecords(BufferInfo* buffer)
{
//    // might can free uncompressed_data here. @todo
//    if (uncompressed_data != nullptr) free(uncompressed_data);
//
//    // is this now message 31?
//    {
//        // assume the result cant be larger than 45mB?
//        uncompressed_size = 47185920;
//
//        compressed_size = 0;
//        memcpy(&compressed_size, &data[bp], 4);
//        bp += 4;
//
//        compressed_size = SwapBytes(compressed_size);
//        compressed_size = abs(compressed_size);
//
//        uncompressed_data = (unsigned char*)malloc(uncompressed_size * sizeof(unsigned char));
//
//        p_len = &uncompressed_size;
//        ret = BZ2_bzBuffToBuffDecompress((char*)&uncompressed_data[0],
//            p_len,
//            (char*)&data[bp],
//            compressed_size,
//            0,
//            4
//        );
//
//        if (ret != BZ_OK)
//        {
//            LOGERR("Failed to decompress the Level-II radar data.\n");
//        }
//
//        bp += compressed_size;
//
//        L2Volume l2volume = {};
//        BufferInfo radial_buffer = {};
//        radial_buffer.data = uncompressed_data;
//        radial_buffer.position = 0;
//
//        // @todo
//        // need to loop this and read all the messages
//        int msg_31_count = 0;
//        seekBuffer(&radial_buffer, CTM_HEADER_SIZE);
//        while (true)
//        {
//            s32 start_pos = radial_buffer.position;
//
//            msg_info = readMessageHeader(&radial_buffer);
//            if (msg_info.type == 31)
//            {
//                msg_31_count += 1;
//                int skipBytes = readMessage31(&radial_buffer);
//                setBufferPos(&radial_buffer, start_pos + msg_info.size + CTM_HEADER_SIZE);
//            }
//            else
//            {
//                int break_here = 1;
//                int test = msg_31_count;
//                break;
//            }
//        }
//    }

}

void ProcessMessages(BufferInfo* buffer)
{
    //
    // 2620002T for message data definitions
    //

    MessageHeaderInfo msg_info = {};
    s32 pre_msg_buf_pos = 0, post_msg_buf_pos = 0;

    s32 radialIndex = 0;

    seekBuffer(buffer, CTM_HEADER_SIZE);
    while (buffer->position < buffer->totalSize)
    {
        pre_msg_buf_pos = buffer->position;
        msg_info = readMessageHeader(buffer);

        if (msg_info.type == 15)
        {
            readMessage15(buffer);
        }

        else if (msg_info.type == 18)
        {
            readMessage18(buffer);
        }

        else if (msg_info.type == 3)
        {
            readMessage3(buffer);
        }

        else if (msg_info.type == 5)
        {
            readMessage5(buffer);
        }

        else if (msg_info.type == 2)
        {
            readMessage2(buffer);
        }

        else if (msg_info.type == 31)
        {
            u32 result = readMessage31(buffer, radialIndex);

            if (result == 1)
            {
                break;
            }

            radialIndex += 1;
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

//                if (msg_info.size < 0)
//                    forward_size = TOTAL_SEG_SIZE;
//
//                else
//                    forward_size = msg_info.size;


        setBufferPos(buffer, pre_msg_buf_pos + msg_info.size);
    }

}

void ReadLevel2File(BufferInfo* mainBuffer)
{
    // This is a level 2 radar file...

    // volume header
    char file_header[25];
    ReadFromBuffer(file_header, mainBuffer, 24);
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

    // assume the result cant be larger than 90mB?
    u32 max_uncompressed_size = 94371840;
    u32 uncompressed_size = max_uncompressed_size;

    s32 compressed_size;
    u32 total_buffer_size = 0;
    auto uncompressed_data = (unsigned char*)malloc(uncompressed_size * sizeof(unsigned char));

    while (mainBuffer->position < mainBuffer->totalSize)
    {
        compressed_size = 0;
        ReadFromBuffer(&compressed_size, mainBuffer, 4);
        compressed_size = SwapBytes(compressed_size);
        compressed_size = abs(compressed_size);

        unsigned char* buffer_entry = &mainBuffer->data[mainBuffer->position];

        // @todo
        // for safety keep up with the size of uncompressed data and how much has been put into it already.
        uncompressed_size = max_uncompressed_size - total_buffer_size;
        u32* p_len = &uncompressed_size;
        s32 ret = BZ2_bzBuffToBuffDecompress((char*)&uncompressed_data[total_buffer_size],
            p_len,
            (char*)buffer_entry,
            compressed_size,
            0,
            4
        );

        if (ret != BZ_OK)
        {
            LOGERR("Failed to decompress the Level-II LDM Record data.\n");
            break;
        }

        total_buffer_size += uncompressed_size;

        // Set the position past the compressed data block for after this data is processed.
        seekBuffer(mainBuffer, compressed_size);
    }

    // @todo
    // Here I might could deallocate the 90mb data after copying to a smaller data,
    // I'm not sure if the trade off is worth it since the data will go bye bye after everything
    // has been processed anyway.


    BufferInfo l2_buffer = {};
    l2_buffer.data = &uncompressed_data[0];
    l2_buffer.totalSize = total_buffer_size;
    l2_buffer.position = 0;

    g_L2Volume = {};
    g_L2Volume.radialCount = 720;
    g_L2Volume.radials = (RadialData*)malloc(g_L2Volume.radialCount * sizeof(RadialData));
    ProcessMessages(&l2_buffer);

    free(uncompressed_data);
}
