//

#ifndef _NWS_INFO_H_



constexpr char* NWS_NOAA_HOSTNAME = "tgftp.nws.noaa.gov\0";
constexpr char* NWS_NOAA_RADAR_DIR = "/SL.us008001/DF.of/DC.radar\0";


typedef struct NexradProduct_t
{
    int productCode;
    char name[128];
    char dir[16];
} NexradProduct;


typedef struct NexradProductInfo_t
{
    int count;
    NexradProduct nexradProducts[255];
} NexradProductInfo;


NexradProductInfo* NexradInfo;


// 
void InitNexradProducts();
void AddProduct(NexradProductInfo* productInfo, NexradProduct product);


#define _NWS_INFO_H_
#endif
