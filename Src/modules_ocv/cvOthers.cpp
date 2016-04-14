#include "fihimgproc_inc.h"

#ifdef GLOBAL_USE_OPENCV
  #include "opencv.hpp"
  #include "cvOthers.h"
  using namespace cv;
#endif

/*
 * Resize an image to a smaller one without changing the ratio. 
 * If the original size is smaller than the parameter(rect_size), just return.
 */
void Mat_ResizeDown(Mat &mSrc, Mat &mDst, int rect_size)
{
    int width  = mSrc.cols;
    int height = mSrc.rows;
    int max_dim = ( width >= height ) ? width : height;
    
    if( (max_dim > rect_size) && (rect_size > 0))
    {
        double scale = (double)max_dim / (double)rect_size;
        width  = (int)((double)width / scale);
        height = (int)((double)height / scale);
	    resize(mSrc, mDst, Size(width, height));
    }else
    {
        mDst = mSrc;
    }
}

/*
 * Resize an image to a bigger one without changing the ratio. 
 * If the original size is bigger than the parameter(rect_size), just return.
 */
void Mat_ResizeUp(Mat &mSrc, Mat &mDst, int rect_size)
{
    int width  = mSrc.cols;
    int height = mSrc.rows;
    int min_dim = ( width <= height ) ? width : height;
    
    if( (min_dim < rect_size) && (rect_size > 0) )
    {
        double scale = (double)rect_size / (double)min_dim;
        width  = (int)((double)width * scale);
        height = (int)((double)height * scale);
	    resize(mSrc, mDst, Size(width, height));
    }else
    {
        mDst = mSrc;
    }
}

/*
    'mDst' can be as same as 'mSrc'
    'increment' range from (-100.0, 100.0)  
*/
void Color_Saturation(Mat& mSrc, int increment)
{
    int channels = mSrc.channels();
    int nRows = mSrc.rows;
    int nCols = mSrc.cols * channels;
    if( mSrc.isContinuous() )
    {
        nCols *= nRows;
        nRows = 1;
    }

    for( int i=0; i<nRows; ++i )
	{  
        uchar* pSrc = mSrc.ptr<uchar>(i);

      #pragma omp parallel for
        for( int j=0; j<nCols; j+=3)
        {
    		int t1 = pSrc[j+0];
    		int t2 = pSrc[j+1];
    		int t3 = pSrc[j+2];
    		int minVal = std::min(std::min(t1,t2), t3);  
    		int maxVal = std::max(std::max(t1,t2), t3);

            //float delta = maxVal - minVal;  
            //float L = 0.5*(maxVal + minVal);  
            //float S = max(0.5*delta/L, 0.5*delta/(255-L)) * 100;
    		float delta = (float)(maxVal-minVal);  
    		float L = (float)(maxVal+minVal);  
    		float S = std::max(delta/L, delta/(510-L));  

            if (increment > 0)
    		{  
    			float alpha = max(S, 1-(float)increment/100);  
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
}

// Return the number of bits in each channel of the given Mat. ie: 8, 16, 32 or 64.
int Mat_GetBitDepth(const Mat &M)
{
    switch (CV_MAT_DEPTH(M.type())) {
        case CV_8U:
        case CV_8S:
            return 8;
        case CV_16U:
        case CV_16S:
            return 16;
        case CV_32S:
        case CV_32F:
            return 32;
        case CV_64F:
            return 64;
    }

    return -1;
}

void ColorCvt_YUYV2BGR(uchar* pYUYV, Mat& mBGR, int width, int height)
{
    Mat mSrcYUYV( height, width, CV_8UC2, pYUYV );

    cvtColor( mSrcYUYV, mBGR, CV_YUV2BGR_YUY2 );
}

void ColorCvt_BGR2YUYV(Mat& mBGR, uchar* pYUYV)
{
    int channels = mBGR.channels();
    int nRows = mBGR.rows;
    int nCols = mBGR.cols * channels;
    if( mBGR.isContinuous() )
    {
        nCols *= nRows;
        nRows = 1;
    }

    for( int i = 0; i < nRows; ++i)
    {
        uchar* pBGR = mBGR.ptr<uchar>(i);
        for( int j = 0; j < nCols; )
        {
            int B = pBGR[j++];
            int G = pBGR[j++];
            int R = pBGR[j++];
                        
            *pYUYV++ = ((66*R + 129*G + 25*B + 128) >> 8) + 16;
            *pYUYV++ = (j&0x01) ? (((-38*R - 74*G + 112*B + 128) >> 8) + 128) : (((112 * R - 94 * G - 18 * B + 128) >> 8) + 128);
        }
    }
}

