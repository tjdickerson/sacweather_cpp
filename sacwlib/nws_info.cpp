//

#include "nws_info.h"
#include <cstdlib>
#include <cstring>

NexradProductInfo* NexradInfo;

void InitNexradProducts()
{
    NexradProductInfo npi = {};
    NexradInfo = &npi;

    // Base Reflectivity (124nmi)
    {
        const char* name = "Base reflectivity - 124 nmi Range\0";
        const char* dir  = "DS.p19r0";

        NexradProduct br = {};
        br.productCode = 19;      
        br.range = 124.0f;

        strncpy(br.name, name, strlen(name));
        strncpy(br.dir, dir, strlen(dir));

        AddProduct(NexradInfo, br);
    }

    // Base Reflectivity (248nmi)
    {
        const char* name = "Base reflectivity - 248 nmi Range\0";
        const char* dir  = "DS.p94r0";

        NexradProduct br = {};
        br.productCode = 94;      
        br.range = 248.0f;

        strncpy(br.name, name, strlen(name));
        strncpy(br.dir, dir, strlen(dir));

        AddProduct(NexradInfo, br);
    }

    // Base Radial Velocity - 162 nmi Range
    {
        const char* name = "Base Radial Velocity - 248 nmi Range\0";
        const char* dir  = "DS.p99v0";

        NexradProduct br = {};
        br.productCode = 99;      
        br.range = 162.0f;

        strncpy(br.name, name, strlen(name));
        strncpy(br.dir, dir, strlen(dir));

        AddProduct(NexradInfo, br);
    }    

    // Composite Reflectivity - 124
    {
        const char* name = "Composite Reflectivity - 124 nmi Range\0";
        const char* dir  = "DS.p37cr";

        NexradProduct br = {};
        br.productCode = 37;      
        br.range = 124.0f;
        br.resolution = 0.54f;

        strncpy(br.name, name, strlen(name));
        strncpy(br.dir, dir, strlen(dir));

        AddProduct(NexradInfo, br);
    }
    
    // Composite Reflectivity - 248
    {
        const char* name = "Composite Reflectivity - 248 nmi Range\0";
        const char* dir  = "DS.p38cr";

        NexradProduct br = {};
        br.productCode = 38;      
        br.range = 248.0f;
        br.resolution = 2.2f;

        strncpy(br.name, name, strlen(name));
        strncpy(br.dir, dir, strlen(dir));

        AddProduct(NexradInfo, br);
    }
}


void AddProduct(NexradProductInfo* productInfo, NexradProduct product)
{
    productInfo->nexradProducts[productInfo->count] = product;
    productInfo->count += 1;
}


NexradProduct* GetProductInfo(int productCode)
{    
    for (int i = 0; i < NexradInfo->count; i++)
    {
        if (NexradInfo->nexradProducts[i].productCode == productCode)
            return &(NexradInfo->nexradProducts[i]);
    }

    return NULL;
}