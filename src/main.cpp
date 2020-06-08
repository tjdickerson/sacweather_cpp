#include <cstdio>
#include "sacw_api.h"
#include "sacw_extern.h"
#include "tjd_ftp.h"

// Shapefile locations 
constexpr char* BaseShapefile = "C:\\shapes\\weather\\st_us.shp";
constexpr char* CountiesShapefile = "C:\\shapes\\weather\\cnt_us.shp";

int main(int argc, char** argv) 
{
    printf("SAC Weather\n");    
    sacw_Init();

    int result = DownloadFile();
    if (result != 0)
    {
    	printf("Error in download file. %d\n", result);
    }

    return 0;
}


void ShowError(const char* message)
{
	printf("Error Message: %s\n", message);
}


const char* GetBaseShapefileLocation()
{
	return BaseShapefile;
}


const char* GetCountiesShapefileLocation()
{
	return CountiesShapefile;
}














































