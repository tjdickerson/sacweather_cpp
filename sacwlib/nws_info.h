//

#include <string>
#include <algorithm>

#ifndef __NWS_INFO_H__


constexpr char NWS_NOAA_PROTOCOL[] = "https://\0";
constexpr char NWS_NOAA_HOSTNAME[] = "tgftp.nws.noaa.gov\0";
constexpr char NWS_NOAA_RADAR_DIR[] = "/SL.us008001/DF.of/DC.radar\0";
constexpr char NWS_NOAA_RADAR_DEFAULT_FILE[] = "sn.last";


struct NexradProduct
{
    int productCode;
    float range;
    float resolution;
    char name[128];
    char dir[16];
};


struct NexradProductInfo
{
    int count;
    NexradProduct nexradProducts[255];
};


extern NexradProductInfo* g_NexradProducts;

static void GetUrlForProduct(std::string* dest, const char* siteName, NexradProduct* product)
{
    dest->append(NWS_NOAA_PROTOCOL);
    dest->append(NWS_NOAA_HOSTNAME);
    dest->append(NWS_NOAA_RADAR_DIR);

    dest->append("/");
    dest->append(product->dir);

    std::string site_name = siteName;
    transform(site_name.begin(), site_name.end(), site_name.begin(), ::tolower);

    dest->append("/SI.");
    dest->append(site_name);

    dest->append("/");
    dest->append(NWS_NOAA_RADAR_DEFAULT_FILE);
}

// 
void InitNexradProducts();
void AddProduct(NexradProductInfo* productInfo, NexradProduct product);
NexradProduct* GetProductInfo(int productCode);


#define __NWS_INFO_H__
#endif
