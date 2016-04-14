#ifndef __APP_H__
#define __APP_H__

#include <string>
#include "fihimgproc.h"



ImgProcMFC* app_GetImgProcHandler(void);

void app_ImageProcess(FIH_IMGPROC_CMD, FIH_IMGPROC_Params *);

#endif
