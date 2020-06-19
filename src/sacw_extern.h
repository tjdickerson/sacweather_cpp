//

#ifndef _SACW_EXTERN_H_


void ShowError(const char* message);
int DownloadFile(const char* url, const char* dest);
const char* GetBaseShapefileLocation();
const char* GetCountiesShapefileLocation();


#define _SACW_EXTERN_H_
#endif