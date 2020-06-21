//

#ifndef __NWS_INFO_H__



constexpr char* NWS_NOAA_HOSTNAME = "tgftp.nws.noaa.gov\0";
constexpr char* NWS_NOAA_RADAR_DIR = "/SL.us008001/DF.of/DC.radar\0";


typedef struct NexradProduct_t
{
    int productCode;
    float range;
    char name[128];
    char dir[16];
} NexradProduct;


typedef struct NexradProductInfo_t
{
    int count;
    NexradProduct nexradProducts[255];
} NexradProductInfo;


extern NexradProductInfo* NexradInfo;


// 
void InitNexradProducts();
void AddProduct(NexradProductInfo* productInfo, NexradProduct product);
NexradProduct* GetProductInfo(int productCode);


#define __NWS_INFO_H__
#endif
