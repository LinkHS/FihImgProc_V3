#include "app.h"
#include "fihimgproc.h"
#include "opencv.hpp"

using namespace cv;

ImgProcMFC imgProcMain;

ImgProcMFC* app_GetImgProcHandler(void)
{
    return &imgProcMain;
}


