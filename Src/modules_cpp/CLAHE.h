#ifndef __CLAHE_H__
#define __CLAHE_H__

typedef unsigned char uchar;
typedef unsigned int uint;

/******** Prototype of CLAHE function. Put this in a separate include file. *****/
int CLAHE_main (uchar* pImage, int xRes, int yRes, int xGrids, int yGrids, float fCliplimit);
#endif