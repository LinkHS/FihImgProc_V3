#include "fihimgproc_inc.h"
#include <algorithm>

#if _WIN32
#define float double
#endif


void ColorCvt_YUYV2BGR(uchar* pYUYV, uchar* pBGR, int height, int width)
{
#if GLOBAL_USE_OPENMP
	int subLen = height*width*3/8;
	#pragma omp parallel for
	for( int i=0; i<8; i++ ) /* const int cpus = 8; */
	{
	    uchar *pSrc = pYUYV + height*width*i/4; /* height * width * 2 / 8 * i */
		uchar *pDst = pBGR + height*width*3*i/8; /* height * width * 3 / 8 * i */
		
		for( int j = 0; j < subLen; )
		{
			uchar Y1 = *pSrc++; 
			uchar U  = *pSrc++; 
			uchar Y2 = *pSrc++; 
			uchar V  = *pSrc++; 
			
			int C1 = Y1 - 16;
			int C2 = Y2 - 16;
			int D  = U - 128;
			int E  = V - 128;

			pDst[j++] = FIH::saturate_cast<uchar>(( 298 * C1 + 516 * D + 128) >> 8);           //B1
			pDst[j++] = FIH::saturate_cast<uchar>(( 298 * C1 - 100 * D - 208 * E + 128) >> 8); //G1
			pDst[j++] = FIH::saturate_cast<uchar>(( 298 * C1 + 409 * E + 128) >> 8);           //R1

			pDst[j++] = FIH::saturate_cast<uchar>(( 298 * C2 + 516 * D + 128) >> 8);           //B2
			pDst[j++] = FIH::saturate_cast<uchar>(( 298 * C2 - 100 * D - 208 * E + 128) >> 8); //G2
			pDst[j++] = FIH::saturate_cast<uchar>(( 298 * C2 + 409 * E + 128) >> 8);           //R2
		}
	}
#else
    for( int j = 0; j < height*width*3; )
    {
        uchar Y1 = *pYUYV++; 
        uchar U  = *pYUYV++; 
        uchar Y2 = *pYUYV++; 
        uchar V  = *pYUYV++; 
        
        int C1 = Y1 - 16;
        int C2 = Y2 - 16;
        int D  = U - 128;
        int E  = V - 128;

        pBGR[j++] = FIH::saturate_cast<uchar>(( 298 * C1 + 516 * D + 128) >> 8);           //B1
        pBGR[j++] = FIH::saturate_cast<uchar>(( 298 * C1 - 100 * D - 208 * E + 128) >> 8); //G1
        pBGR[j++] = FIH::saturate_cast<uchar>(( 298 * C1 + 409 * E + 128) >> 8);           //R1

        pBGR[j++] = FIH::saturate_cast<uchar>(( 298 * C2 + 516 * D + 128) >> 8);           //B2
        pBGR[j++] = FIH::saturate_cast<uchar>(( 298 * C2 - 100 * D - 208 * E + 128) >> 8); //G2
        pBGR[j++] = FIH::saturate_cast<uchar>(( 298 * C2 + 409 * E + 128) >> 8);           //R2
    }
#endif
}

void ColorCvt_BGR2YUYV(uchar* pBGR, uchar* pYUYV, int height, int width)
{
    int channels = 3;
    int nRows = height;
    int nCols = width * channels;

#if GLOBAL_USE_OPENMP
    int subLen = nRows*nCols/8;
	#pragma omp parallel for
    for( int i=0; i<8; i++ )
	{
	    uchar *pSrc = pBGR + subLen*i; /* height * width * 3 / 8 * i */
	    uchar *pDst = pYUYV + subLen/3*2*i;
		
        for( int j = 0; j < subLen; )
        {
            int B = pSrc[j++];
            int G = pSrc[j++];
            int R = pSrc[j++];
                        
            *pDst++ = ((66*R + 129*G + 25*B + 128) >> 8) + 16;
            *pDst++ = (j&0x01) ? (((-38*R - 74*G + 112*B + 128) >> 8) + 128) : (((112 * R - 94 * G - 18 * B + 128) >> 8) + 128);
        }
    }
#endif
}

void Color_Saturation(uchar *pBGR, int height, int width, int increment)
{
    int channels = 3;
    int nRows = height;
    int nCols = width * channels;
	
#if GLOBAL_USE_OPENMP
	#pragma omp parallel for
    for( int i=0; i<8; i++ )
	{  
		int subLen = nRows*nCols/8;
	    uchar *pSrc = pBGR + subLen*i; /* height * width * 3 / 8 * i */
		
        for( int j=0; j<subLen; j+=3)
        {
    		int t1 = pSrc[j+0];
    		int t2 = pSrc[j+1];
    		int t3 = pSrc[j+2];
    		int minVal = FIH::min(FIH::min(t1,t2), t3);  
    		int maxVal = FIH::max(FIH::max(t1,t2), t3);
			
            //float delta = maxVal - minVal;  
            //float L = 0.5*(maxVal + minVal);  
            //float S = max(0.5*delta/L, 0.5*delta/(255-L)) * 100;
    		float delta = (float)(maxVal-minVal);  
    		float L = (float)(maxVal+minVal);  
    		float S = FIH::max(delta/L, delta/(510-L));  

            if (increment > 0)
    		{  
    			float alpha = FIH::max(S, 1-(float)increment/100);  
    			alpha = (float)1.0/alpha - 1;  
    			pSrc[j+0] = (uchar)(t1 + (t1 - L/2.0) * alpha);  
    			pSrc[j+1] = (uchar)(t2 + (t2 - L/2.0) * alpha);  
    			pSrc[j+2] = (uchar)(t3 + (t3 - L/2.0) * alpha);  
    		}  
			else  
    		{  
    			//alpha = increment;  
    			//pSrc[j+0] = (L + (t1 - L) * (1+alpha));  
    			//pSrc[j+1] = (L + (t2 - L) * (1+alpha));  
    			//pSrc[j+2] = (L + (t3 - L) * (1+alpha));  
    		} 
        } 
	}  
#endif
}

void ImgData_CopyYFromYUYV(uchar *pYUYV, uchar *pY, int height, int width)
{
#if 0
	for( int row=0; row<height; row++ )
	{
		for( int col=0; col<width; col++ )
		{
			*pY++ = *pYUYV;
			pYUYV += 2;
		}
	}
#else
	#pragma omp parallel for
	for( int cpuIdx=0; cpuIdx<8; cpuIdx++ )
	{
		int subLen = height*width/8;
		uchar *pSrc = pYUYV + subLen*cpuIdx*2;
		uchar *pDst = pY + subLen*cpuIdx;
		for( int i=0; i<subLen; i++ )
		{
			*pDst++ = *pSrc;
			pSrc += 2;
		}
	}
#endif
}

void ImgData_CopyYToYUYV(uchar *pYUYV, uchar *pY, int height, int width)
{
#if 0
	for( int row=0; row<height; row++ )
	{
		for( int col=0; col<width; col++ )
		{
			*pYUYV = *pY++;
			pYUYV += 2;
		}
	}
#else
	#pragma omp parallel for
	for( int cpuIdx=0; cpuIdx<8; cpuIdx++ )
	{
		int subLen = height*width/8;
		uchar *pDst = pYUYV + subLen*cpuIdx*2;
		uchar *pSrc = pY + subLen*cpuIdx;
		for( int i=0; i<subLen; i++ )
		{
			*pDst = *pSrc++;
			pDst += 2;
		}
	}
#endif
}

extern int CLAHE_main (uchar* pImage, int xRes, int yRes, int xGrids, int yGrids, float fCliplimit);

void App_BGR_YUYV(uchar *pBGR, int height, int width)
{
	uchar *pYUYV = new uchar[height*width*2];
	uchar *pY = new uchar[height*width];

	ColorCvt_BGR2YUYV( pBGR, pYUYV, height, width );

    ImgData_CopyYFromYUYV( pYUYV, pY, height, width );
	CLAHE_main(pY, width, height, 8, 8, 0.5);
	ImgData_CopyYToYUYV( pYUYV, pY, height, width );

	ColorCvt_YUYV2BGR( pYUYV, pBGR, height, width );
	
	Color_Saturation( pBGR, height, width, 25 );

	delete pYUYV;
	delete pY;
}
