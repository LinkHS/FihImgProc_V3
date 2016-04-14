#ifndef __CV_OTHERS_H__
#define __CV_OTHERS_H__

#include "opencv.hpp"

#define PfmTest_StartTimer(t)       do{ t = (double)cv::getTickCount(); } while(0)
/* Times passed in seconds */
#define PfmTest_StopTimer(t)        do{ t = ((double)cv::getTickCount() - t)/cv::getTickFrequency(); } while(0)

#define PfmTest_GetFunCostTime(fun, funName)                           \
        do{                                                      \
            double t = (double)getTickCount();                   \
            fun;                                                 \
            t = ((double)getTickCount() - t)/getTickFrequency(); \
            MY_LOGD("%s() cost %lfs", funName, t);               \
        } while(0)

void Mat_ResizeDown(cv::Mat &mSrc, cv::Mat &mDst, int rect_size);
void Mat_ResizeUp(cv::Mat &mSrc, cv::Mat &mDst, int rect_size);

void Color_Saturation(cv::Mat& mSrc, int increment);

void ColorCvt_YUYV2BGR(uchar* pYUYV, uchar* pBGR, int length);
void ColorCvt_YUYV2BGR(uchar* pYUYV, cv::Mat& mBGR, int width, int height);
void ColorCvt_BGR2YUYV(cv::Mat& mBGR, uchar* pYUYV);

void Img_GuidedFilter(cv::Mat &mGuide, cv::Mat &mSrc, cv::Mat &mDst, int radius, float eps, int dDepth=-1);
#endif
