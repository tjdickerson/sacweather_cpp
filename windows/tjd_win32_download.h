//
// Created by tjdic on 06/19/2021.
//

#ifndef _TJD_WIN32_DOWNLOAD_H_
#define _TJD_WIN32_DOWNLOAD_H_

#include <nws_info.h>
#include "sacw_api.h"

void StartDownload(const char* siteName, NexradProduct* nexradProduct);
void StartDownload(RdaSite* rdaSite, NexradProduct* nexradProduct);

#endif //_TJD_WIN32_DOWNLOAD_H_
