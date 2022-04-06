//

#include "nws_info.h"
#include <cstdlib>
#include <cstring>

NexradProductInfo* g_NexradProducts;

void InitNexradProducts()
{
    static NexradProductInfo npi = {};
    g_NexradProducts = &npi;

    // Base Reflectivity (248nmi)
    {
        const char* name = "Base Reflectivity\0";
        const char* dir  = "DS.p94r0";

        NexradProduct br = {};
        br.productCode = 94;      
        br.range = 248.0f;

        strncpy(br.name, name, strlen(name));
        strncpy(br.dir, dir, strlen(dir));

        AddProduct(g_NexradProducts, br);
    }

    // Base Radial Velocity - 162 nmi Range
    {
        const char* name = "Base Velocity\0";
        const char* dir  = "DS.p99v0";

        NexradProduct br = {};
        br.productCode = 99;      
        br.range = 162.0f;

        strncpy(br.name, name, strlen(name));
        strncpy(br.dir, dir, strlen(dir));

        AddProduct(g_NexradProducts, br);
    }    

    // // Composite Reflectivity - 124
    // {
    //     const char* name = "Composite Reflectivity 124\0";
    //     const char* dir  = "DS.p37cr";

    //     NexradProduct br = {};
    //     br.productCode = 37;      
    //     br.range = 124.0f;
    //     br.resolution = 0.54f;

    //     strncpy(br.name, name, strlen(name));
    //     strncpy(br.dir, dir, strlen(dir));

    //     AddProduct(g_NexradProducts, br);
    // }
    
    // // Composite Reflectivity - 248
    // {
    //     const char* name = "Composite Reflectivity 248\0";
    //     const char* dir  = "DS.p38cr";

    //     NexradProduct br = {};
    //     br.productCode = 38;      
    //     br.range = 248.0f;
    //     br.resolution = 2.2f;

    //     strncpy(br.name, name, strlen(name));
    //     strncpy(br.dir, dir, strlen(dir));

    //     AddProduct(g_NexradProducts, br);
    // }
}


void AddProduct(NexradProductInfo* productInfo, NexradProduct product)
{
    productInfo->nexradProducts[productInfo->count] = product;
    productInfo->count += 1;
}


NexradProduct* GetProductInfo(int productCode)
{    
    for (int i = 0; i < g_NexradProducts->count; i++)
    {
        if (g_NexradProducts->nexradProducts[i].productCode == productCode)
            return &(g_NexradProducts->nexradProducts[i]);
    }

    return NULL;
}