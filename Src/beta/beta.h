#ifndef __BETA_H__
#define __BETA_H__

#include "opencv.hpp"

void Mat_DrawGridlines(cv::Mat &mSrc, int xStep, int yStep);
//void MatData_CptVar(cv::Mat &mGray, int *pStddev, int xBlocks, int yBlocks);
void MatData_CptVar(cv::Mat &mGray, float *pMean, float *pStddev, int xBlocks, int yBlocks);

int CLAHE_main (uchar* pImage, int xRes, int yRes, int xGrids, int yGrids, float fCliplimit);

int MatData_CptVar(uchar *pSrc, int xRes, int yRes, float *pMean, float *pStddev, int xBlocks, int yBlocks);

void SaveIntArray( char *pFilename, int *pArray, int rows, int cols );

#endif