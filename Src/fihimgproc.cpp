#include "fihimgproc_inc.h"
#include "fihimgproc.h"

#include "beta/beta.h"

#ifdef GLOBAL_USE_OPENCV
  #include "opencv.hpp"
  #include "modules_ocv/cvOthers.h"
  using namespace cv;
#endif


#if _WIN32
  #define float double
#endif

#define MY_PRINT               FIH_PRINT
#define MY_LOGD(fmt, ...)      FIH_LOGD("[%s] " fmt, __FUNCTION__, __VA_ARGS__)
#define MY_LOGW                MY_LOGD
#define MY_LOGE                MY_LOGD

struct ImgProcMFCPrivate
{
    Mat                       mView;
	std::string               fileNameWithPath;
};

ImgProcMFC::ImgProcMFC(void)
{
    myPrivate = new struct ImgProcMFCPrivate;
}

ImgProcMFC::~ImgProcMFC(void)
{
    delete myPrivate;
}

int ImgProcMFC::OpenPicture(std::string &filename)
{
    MY_LOGD("Open \"%s\"", filename.c_str());

    myPrivate->mView = imread(filename);

    if( myPrivate->mView.data == NULL )
    {
        MY_LOGE("Open %s failed\n", filename.c_str());
        return -1;
    }

	/* std::cout << filename << std::endl; */
    MY_LOGD("Open \"%s\" OK", filename.c_str());
    MY_LOGD("image: height %d, width %d", myPrivate->mView.rows, myPrivate->mView.cols);

    myPrivate->fileNameWithPath = filename;
    return 0;
}

void ImgProcMFC::SavePicture(std::string &dstname)
{
    if( myPrivate->mView.data == NULL )
        return;

    imwrite(dstname, myPrivate->mView);
}


void* ImgProcMFC::PrepareDispData(void) 
{
    Mat mResize, mDst;
    IplImage* piDst;

    if ( myPrivate->mView.data == NULL )
    {
        return NULL;
    }

    int srcWidth = myPrivate->mView.cols;
    int srcHeight = myPrivate->mView.rows;

    /* Scale down the image */
    if ( (srcWidth > dispW) || (srcHeight > dispH) )
    {
        srcWidth = dispW;
        srcHeight = dispH;

        cv::resize(myPrivate->mView, mResize, cv::Size(srcWidth, srcHeight));
    }
    else
    {
        mResize = myPrivate->mView;
    }

    /* Vertical flipping of the image to use it on Microsoft Windows */
    flip( mResize, mDst, 0 );

    piDst = cvCreateImage( mDst.size(), 8, mDst.channels() );    

    IplImage iTemp = mDst;
    cvCopy( &iTemp, piDst );

    return piDst;
}

/*
    Compute new display image size when display area size changes
*/
void ImgProcMFC::SetDispSize(int dstWidth, int dstHeight)
{
    int srcWidth = myPrivate->mView.cols;
    int srcHeight = myPrivate->mView.rows;

    /* Scale down the image size if it is bigger than display area of the window */
    if ( (srcWidth > dstWidth) || (srcHeight > dstHeight) )
    {
        double scalew = (double)srcWidth / (double)dstWidth;
        double scaleh = (double)srcHeight / (double)dstHeight;

    	if( scalew >= scaleh )
    	{
    		dispH = dstWidth;  
    		dispW = (int)((double)srcHeight / scalew);  
    	}else
    	{
    		dispW = (int)((double)srcWidth / scaleh);  
    		dispH = dstHeight;  
    	}
    }
    else
    {
        dispW = srcWidth;
        dispH = srcHeight;
    }
}

void storeMatAsTxt(Mat &mSrc, char *pName)
{
    MY_LOGD("fopen %s", pName);
	FILE *fp;
    int res = fopen_s(&fp, pName, "w+");
    if( NULL == fp )
    {
        MY_LOGD("fopen failed");
        return;
    }

    for( int i=0; i<mSrc.rows; i++ )
    {  
        uchar* pSrc = mSrc.ptr<uchar>(i);

        for( int j=0; j<mSrc.cols; j++)
        {
            char buffer[10];
            sprintf_s(buffer, "%d ", pSrc[j]);
            fwrite(buffer, 1, strlen(buffer), fp);
        }
        fwrite("\n", 1, 1, fp);
    }
    fclose(fp);
}

#include "modules/dehazing/dehazing.h"
#include "basic/myDehazing.h"
#include "basic/LMVFilter.h"
extern void Test_DataTraversal(Mat &mSrc);

void ImgProcMFC::ImageProcess(FIH_IMGPROC_CMD cmd, FIH_IMGPROC_Params *pParams)
{
	Mat &mView = myPrivate->mView;
    if (mView.data == NULL)
        return;

    Mat mDispSize;
    resize(mView, mDispSize, Size(dispW, dispH));

    switch (cmd)
    {
    /* Show mView's gray image in a new window */
    case IMGPROC_ShowGrayChannel:
		{
        Mat mGray;
        cvtColor(mDispSize, mGray, CV_BGR2GRAY);
        imshow("test", mGray);
		}
        break;
		
    case IMGPROC_Dehazing:
		{
		int dehazing_blk_size = 16;
        int nWid = mView.cols - (mView.cols % dehazing_blk_size);
        int nHei = mView.rows - (mView.rows % dehazing_blk_size);

        if( (nWid == 0) || (nHei == 0) )
        {
            MY_LOGW("Image size is too small for dehazing. Exit");
        }

    	dehazing dehazingImg(nWid, nHei, 16, false, false, 5.0f, 1.0f, 40);

        Mat mResult, mResize;
		resize( mView, mResize, Size(nWid, nHei) );
    	dehazingImg.ImageHazeRemoval(mResize, mResult);
        mView = mResult;
		}
        break;

    case IMGPROC_MyDehazing:
        {
            Mat mGray, mDst;
			int dehazing_blk_size = 16;
			int nWid = mDispSize.cols - (mDispSize.cols % dehazing_blk_size);
			int nHei = mDispSize.rows - (mDispSize.rows % dehazing_blk_size);
			
#if 0
            cvtColor( mDispSize, mGray, CV_BGR2GRAY );
			resize( mGray, mDst, Size(nWid, nHei) ); mGray = mDst;
            Dehazing dehaze(mGray);
#else
            Mat mResize, mYCrCb, mChannels[3];
            resize( mDispSize, mResize, Size(nWid, nHei) );
            cvtColor( mResize, mYCrCb, CV_BGR2YCrCb );
            split( mYCrCb, mChannels );
            Dehazing dehaze( mChannels[0] );
            dehaze.doDehazing( mChannels[0] );
            merge( mChannels, 3, mYCrCb );
            cvtColor( mYCrCb, mDst , CV_YCrCb2BGR );
            imshow("dehazing", mDst);
            Color_Saturation(mDst, 30);
            imshow("saturation", mDst);
#endif
        }
        break;

    case IMGPROC_ComptAveragePic:
        {
            Mat mSrcs[4];
            mSrcs[0] = imread("D://KuaiPan//FIH_Nanjing//Photos for test//others/obstacle-1.jpg");
            //mSrcs[1] = imread("D://KuaiPan//FIH_Nanjing//Photos for test//others/obstacle-2.jpg");
            mSrcs[2] = imread("D://KuaiPan//FIH_Nanjing//Photos for test//others/obstacle-3.jpg");
            mSrcs[3] = imread("D://KuaiPan//FIH_Nanjing//Photos for test//others/obstacle-4.jpg");
            
			Mat mMask;
            Mat mDsts[3];
            add(mSrcs[0], mSrcs[2], mDsts[0], mMask, CV_32SC3);
            add(mDsts[0], mSrcs[3], mDsts[1], mMask, CV_32SC3);
            //add(mDsts[0], mDsts[1], mDsts[2], mMask, CV_32SC3);

            Mat mDst = mDsts[1] / 3;
            mDst.convertTo(mView, CV_8UC3);
        }
        break;

	case IMGPROC_PermTest:
		//Test_DataTraversal(mView);
		extern void Test_myBoxfilter(Mat &mSrc, Mat &mDst, int r, int alpha);
        extern void Compute_Sum_SqrSum_InFirstRow(Mat &mSrc, int r);
        extern void Compute_Acc_Sum_SqrSum_InFirstBlock(Mat &mSrc, int r);
        extern int RowsFIFO_test(void);
		extern void FloatMulti_IntDivide(float a, int b);
		extern void TableRead_vs_Cpt(void);

        {
			//FloatMulti_IntDivide(0.121, 10);
			TableRead_vs_Cpt();
        }
		break;

    case IMGPROC_LMSFilter:
        {  
#if 0
			Mat mGray;
            cvtColor(mDispSize, mGray, CV_BGR2GRAY);
            imshow("test", mGray);
            //storeMatAsTxt(mGray, "d:/src.txt");
            double t;
            PfmTest_StartTimer(t);
            LMVFilter lmsFiler( mGray );
            lmsFiler.filter( NULL, 10, 4 );
            PfmTest_StopTimer(t);
            MY_LOGD("%s() cost %lfs", "filter", t);  
            imshow("result", mGray);
#else
/*
  ISO 100-200: Radius 15, Level 4
  ISO 200-800: Radius 15, Level 3
  ISO 800-: do not process
*/
			Mat mYCrCb, mChns[3], mRes, mY;
			cvtColor(mView, mYCrCb, CV_BGR2YCrCb);
			split(mYCrCb, mChns);
			//mChns[0].copyTo(mY);
			//LMVFilter lmsFiler( mChns[0].data,  );
			LMVFilter lmsFiler( mChns[0].data, mView.rows, mView.cols );
			lmsFiler.filter( mChns[0].data, 15, 4 );
			merge(mChns, 3, mYCrCb);
			cvtColor(mYCrCb, mRes, CV_YCrCb2BGR);
			imwrite("d:/temp/res_R15L4.jpg", mRes);
			break;
#endif
        }
		{
			Mat mGray;
            cvtColor(mDispSize, mGray, CV_BGR2GRAY);
            Mat mDst;
            double t;
            PfmTest_StartTimer(t);
			boxFilter( mGray, mDst, -1, Size(10, 10) );
			boxFilter( mGray.mul(mGray), mDst, -1, Size(10, 10) );
            PfmTest_StopTimer(t);
			MY_LOGD("%s() cost %lfs", "boxfilter", t);  

			Mat mMean, mSqrMean;
            PfmTest_StartTimer(t);
#pragma omp parallel for
			for(int i=0; i<2; i++)
			{
				if( i== 0 )
					boxFilter( mGray, mMean, -1, Size(10, 10) );
				else
					boxFilter( mGray.mul(mGray), mSqrMean, -1, Size(10, 10) );
			}
            PfmTest_StopTimer(t);
            MY_LOGD("%s() cost %lfs", "boxfilter omp", t);  
        }
        break;

	    case IMGPROC_CLAHE:
           {
			double t;
			Mat mGray_opencv;

			uchar *pGray = (uchar *)malloc(sizeof(uchar)*mView.rows*mView.cols);
			Mat mGray(mView.rows, mView.cols, CV_8UC1, pGray);
			cvtColor(mView, mGray, CV_BGR2GRAY);
			mGray.copyTo(mGray_opencv);
			imwrite("d:/src_gray.jpg", mGray);
			PfmTest_StartTimer(t);
			//int res = CLAHE_main(pGray, mView.cols, mView.rows, 8, 8, 0.5);
			PfmTest_StopTimer(t);
			MY_PRINT("time cost %f\n", t);
			imwrite("d:/res_gray.jpg", mGray);

			Mat mDst;
			Ptr<CLAHE> clahe = createCLAHE(0.5, cv::Size(8,8));
			//Ptr<CLAHE> clahe = createCLAHE(0.5);
			//clahe->setClipLimit(1);
			PfmTest_StartTimer(t);
			clahe->apply(mGray_opencv,mDst);
			PfmTest_StopTimer(t);
			MY_PRINT("time cost %f\n", t); 
			imwrite("d:/res_opencv.jpg", mDst);

			free(pGray);
		}
		break;

	    case IMGPROC_SHDR_CPP:
extern void App_BGR_YUYV(uchar *pBGR, int height, int width);
		{
			MY_LOGD("height %d, width %d", mView.rows, mView.cols);

			//uchar *pView = new uchar[mView.rows*mView.cols*3];
			//Mat mViewC(mView.rows, mView.cols, CV_8UC3, pView);
			//mView.copyTo(mViewC);

			App_BGR_YUYV(mView.data, mView.rows, mView.cols);

			imwrite("d:/SHDR_res.jpg", mView);
			//delete pView;

			//MY_LOGD("done");
	    }
		break;

		case IMGPROC_Var:
		{
            const int blkX = 16;
			const int blkY = 16;
			Mat mGray;
			float Mean[blkX*blkY]; 
			float Stddev[blkX*blkY];
#if 0
			uchar testData[15 * 12] = {
				0,1,2,3,4,5,6,7,8,9,10,11, 
				1,2,3,4,5,6,7,8,9,10,11,13,
				1,2,3,4,5,6,7,8,9,10,11,12,
				1,2,3,4,5,6,7,8,9,10,11,12,
				1,2,3,4,5,6,7,8,9,10,11,12,
				1,2,3,4,5,6,7,8,9,10,11,12,
				1,2,3,4,5,6,7,8,9,10,11,12,
				1,2,3,4,5,6,7,8,9,10,11,12, 
				1,2,3,4,5,6,7,8,9,10,11,12,     
				1,2,3,4,5,6,7,8,9,10,11,12,
				1,2,3,4,5,6,7,8,9,10,11,12,
				1,2,3,4,5,6,7,8,9,10,11,12,
				1,2,3,4,5,6,7,8,9,10,11,12,
				255,255,255,255,255,255,255,255,255,255,255,255,
				1,2,3,4,5,6,7,8,9,10,11,12
			};

			MatData_CptVar(testData, 12, 15, Mean, Stddev, 2, 3);
			for(int i=0; i<2*3; i++)
				MY_PRINT("%d %d, ", Mean[i], Stddev[i]);
			MY_PRINT("\n");
#endif
#if 0
			cvtColor(mView, mGray, CV_BGR2GRAY);

			MatData_CptVar(mGray, Mean, Stddev, blkX, blkY);
			for(int i=0; i<blkX*blkX; i++)
				MY_PRINT("%.2f %.2f, ", Mean[i], Stddev[i]);

			MY_PRINT("\n\n");
			MatData_CptVar(mGray.data, mGray.cols, mGray.rows, Mean, Stddev, blkX, blkY);
			for(int i=0; i<blkX*blkX; i++)
				MY_PRINT("%.2f %.2f, ", Mean[i], Stddev[i]);
			MY_PRINT("\n");
#endif
	    }
		break;
	 }
}

const double clipMapTable[20] = {0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
                            //0    10   20   30   40   50   60   70   80   90   100  110  120  130

void MakeClipTable(int *pMeanTable, int *pStddevTable, double *pClipTable )
{
	std::cout << "MakeClipTable" << std::endl;

	for( int j=0; j<16; j+=2 )
	{
	    for( int i=0; i<16; i+=2 )
		{
			int sumStddev = pStddevTable[j*16+i] + pStddevTable[j*16+i+1];
			sumStddev += pStddevTable[(j+1)*16+i] + pStddevTable[(j+1)*16+i+1];
			int sumMean = pMeanTable[j*16+i] + pMeanTable[j*16+i+1];
			sumMean += pMeanTable[(j+1)*16+i] + pMeanTable[(j+1)*16+i+1];
			int brightness = (sumMean+2) / 4;
			int offset = 3 - brightness/50;

#if 0 //use log
			int clip = (sumStddev + 2) / 4;
			clip = (clip + 5) / 10;
			pClipTable[(j/2)*8 + i/2] = clipMapTable[clip] + (double)offset / 10.0;
#else
			int clip = (sumStddev + 2) / 4;
			pClipTable[(j/2)*8 + i/2] = (double)log10(clip+10) - 1.0 + (double)offset / 10.0;
#endif
			//std::cout << clipMapTable[clip]  << ',';
			std::cout << pClipTable[(j/2)*8 + i/2]  << "  ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;
}

#include <math.h>

void MakeClipTable(float *pMeanTable, float *pStddevTable, float *pClipTable, int blkX, int blkY)
{
	std::cout << "MakeClipTable" << std::endl;

	for( int j=0; j<blkY; j++ )
	{
	    for( int i=0; i<blkX; i++ )
		{
			float brightness = *pMeanTable++;
#if 0
			double offset = 3 - brightness/50;
			offset = offset / 10.0;
#else
		    //double offset = (8.0 - log10(brightness)*3.3) * 0.14;
			float offset = (8.0 - log10(brightness+10)*3.3) * 0.22;
			//std::cout << brightness << "*" << offset  << ',';
#endif

#if 0 //use log
			int clip = (*pStddevTable++) / 10;
			pClipTable[j*blkX + i] = clipMapTable[clip] + (double)offset / 10.0;
#else
			float clip = *pStddevTable++;
			pClipTable[j*blkX + i] = (float)log10(clip+10) - 1.0 + offset;
			std::cout << log10(clip+10)  << ',';
#endif 
			//std::cout << clipMapTable[clip]  << ',';
			std::cout << pClipTable[j*blkX + i]  << "  ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;
}

void SmoothClipTable(float *pClipTable, int blkX, int blkY)
{
	const int radius = 1;

	std::cout << "SmoothClipTable" << std::endl;

	for( int j=0; j<blkY; j++ )
	{
		int rowUp = j - 1;
		int rowCr = j ;
		int rowDn = j + 1;
		rowUp = (rowUp < 0) ? (j + 1) : rowUp;
		rowDn = (rowDn >= blkY) ? (j - 1) : rowDn;

	    for( int i=0; i<blkX; i++ )
		{
			int colLf = i - 1;
			int colCr = i ;
			int colRt = i + 1;
			colLf = (colLf < 0) ? (i + 1) : colLf;
			colRt = (colRt >= blkX) ? (i - 1) : colRt;

			float sum = pClipTable[rowUp*blkX + colLf] + pClipTable[rowUp*blkX + colCr] + pClipTable[rowUp*blkX + colRt];
			sum += pClipTable[rowCr*blkX + colLf] + pClipTable[rowCr*blkX + colCr] + pClipTable[rowCr*blkX + colRt];
			sum += pClipTable[rowDn*blkX + colLf] + pClipTable[rowDn*blkX + colCr] + pClipTable[rowDn*blkX + colRt];
			pClipTable[j*blkX + i] = sum / 9;
			std::cout << pClipTable[j*blkX + i]  << "  ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;
}

void WriteToFile(char *pFilename, const char *pData)
{
	FILE *fp;
	int res = fopen_s(&fp, pFilename, "a+");
    if( NULL == fp )
	{
        std::cout << "fopen failed: " << pFilename <<std::endl;
        return;
    }

    fwrite(pData, 1, strlen(pData), fp);

    fclose(fp);
}

void ImgProcMFC::MultiImageProcess(std::list<std::string> &m_FileList, FIH_IMGPROC_CMD cmd)
{
	std::string path = m_FileList.front();
	m_FileList.pop_front();
//#define SINGLE

#ifndef SINGLE
    while(!m_FileList.empty())
#endif
    {
		std::string filename = m_FileList.front();
        m_FileList.pop_front();
		Mat mSrc = imread(path+'\\'+filename);
		std::cout << filename << std::endl;

		const int blkX = 16;
		const int blkY = 16;
		float Stddev[blkX*blkY];
		float Mean[blkX*blkY];

		Mat mYUYV, mCnls[2];
		mYUYV.create(mSrc.rows, mSrc.cols, CV_8UC2);

		ColorCvt_BGR2YUYV(mSrc, mYUYV.data); //cvtColor(mView, mYCrCb, CV_BGR2YUV); //CV_BGR2YUV  CV_BGR2YCrCb
		split(mYUYV, mCnls);

#ifdef SINGLE
		Mat mDispSize;
		resize(mSrc, mDispSize, Size(dispW, dispH));
		Mat_DrawGridlines(mDispSize, 8, 8);
		imshow("blocks", mDispSize);
#endif

		float clipTable[64];
#if 0
		//MatData_CptVar(mCnls[0], Mean, Stddev, 8, 8);
		MatData_CptVar(mCnls[0].data, mCnls[0].cols, mCnls[0].rows, Mean, Stddev, blkX, blkY);
		MakeClipTable(Mean, Stddev, clipTable, 8, 8);
		SmoothClipTable(clipTable, 8, 8);
#elseif 0
		MatData_CptVar(mCnls[0], Mean, Stddev, 16, 16);
		MakeClipTable(Mean, Stddev, clipTable);
		SmoothClipTable(clipTable, 8, 8);
#endif
		//CLAHE_main(mCnls[0].data, mSrc.cols, mSrc.rows, 8, 8, (float)0.0);
		merge(mCnls, 2, mYUYV);

		cvtColor( mYUYV, mSrc, CV_YUV2BGR_YUY2 ); //CV_YCrCb2BGR CV_YUV2BGR
		//Color_Saturation( mSrc, 15 );

		imwrite(path+"\\res\\"+filename, mSrc);
    }
}
