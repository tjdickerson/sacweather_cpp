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