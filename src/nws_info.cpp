//

#include "nws_info.h"
#include <cstdlib>


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
        strcpy(br.name, name, strlen(name));
        strcpy(br.dir, dir, strlen(dir));

        AddProduct(NexradInfo, br);
    }
    
}


void AddProduct(NexradProductInfo* productInfo, NexradProduct product)
{
    productInfo->nexradProducts[productInfo->count] = product;
    productInfo->count += 1;
}

