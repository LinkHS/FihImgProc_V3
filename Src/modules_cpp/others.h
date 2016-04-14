#ifndef __OTHERS_H__
#define __OTHERS_H__

#include "fihimgproc_inc.h"

void ColorCvt_YUYV2BGR(uchar* pYUYV, uchar* pBGR, int height, int width);
void ColorCvt_BGR2YUYV(uchar* pBGR, uchar* pYUYV, int height, int width);
void Color_Saturation(uchar *pBGR, int height, int width, int increment);
void ImgData_CopyYFromYUYV(uchar *pYUYV, uchar *pY, int height, int width);
void ImgData_CopyYToYUYV(uchar *pYUYV, uchar *pY, int height, int width);

#endif
