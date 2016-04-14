#include "fihimgproc_inc.h"
#include "beta.h"

#ifdef GLOBAL_USE_OPENCV
  #include "opencv.hpp"
  #include "modules_ocv/cvOthers.h"
  using namespace cv;
#endif

void Mat_DrawGridlines(Mat &mSrc, int xBlocks, int yBlocks)
{
	int subW = mSrc.cols / xBlocks;
	int subH = mSrc.rows / yBlocks;

	for( int i=1; i<yBlocks; i++ )
	{
		mSrc.row(i*subH).setTo(0);
	}

	for( int i=1; i<xBlocks; i++ )
	{
		mSrc.col(i*subW).setTo(0);
	}
}

void MatData_CptVar(Mat &mGray, float *pMean, float *pStddev, int xBlocks, int yBlocks)
{
	int subw = mGray.cols / xBlocks;
	int subh = mGray.rows / yBlocks;
	Scalar mean, stddev;

	for(int j=0; j<yBlocks; j++)
	{
		for(int i=0; i<xBlocks; i++)
		{
			Mat mSrc(mGray, Rect(i*subw, j*subh, subw, subh));
			meanStdDev(mSrc, mean, stddev);
			*pStddev++ = stddev[0];
			*pMean++ = mean[0];
		}
	}
}

#if _WIN32
  #pragma message("32-bit system")
  const int MAX_INT = 0x7FFFFFFF;
#elif _WIN64
  #pragma message("64-bit system")
  const int MAX_INT = 0x7FFFFFFFFFFFFFFF;
#endif

int MatData_CptVar(uchar *pSrc, int xRes, int yRes, float *pMean, float *pStddev, int xBlocks, int yBlocks)
{
#ifndef CPU_NUM
	const int cpus = 8;
#else
    const int cpus = CPU_NUM; 
#endif
	const int MAX_SUM = MAX_INT - 0xFF;      // MAX_INT - 255
	const int MAX_SQRSUM = MAX_INT - 0xFE01; // MAX_INT - 255*255

	if( (xRes%xBlocks) || (yRes%yBlocks)) return -1; // xRes/yRes is no multiple of xBlocks/yBlocks
	if( (pSrc==NULL) || (pMean==NULL) || (pStddev==NULL) ) return -2; //fatal error
	if( (xBlocks == 0) || (yBlocks == 0) ) return -3;

//MY_PRINT("MAX_SQRSUM = %x", MAX_SQRSUM);

	/*
	 * Calculate mean and stddev of each block(total block number is xBlocks*yBlocks)
	 */	
	int subw = xRes / xBlocks; //sub width
	int subh = yRes / yBlocks; //sub height
	int subsize = subw * subh;
	for(int tblV=0; tblV<yBlocks; tblV++) //tblV: table vertical
	{
		int mem[16*3];
		int *pSum = mem;
		int *pSqrMeanQuot = mem + xBlocks; //square sum mean quotient
		int *pSqrMeanRema = pSqrMeanQuot + xBlocks; //square sum mean remain
		//int *pSqrSum = mem + 16;
		if( xBlocks > 16 ) //mem[32] is not enough
		{
			pSum = new int[xBlocks * 3];
			//pSqrSum = pSum + xBlocks;
			pSqrMeanQuot = pSum + xBlocks;
			pSqrMeanRema = pSqrMeanQuot + xBlocks;
		}

		/* clear mem */
		for(int i=0; i<xBlocks; i++)
		{
			pSum[i] = 0;
			//pSqrSum[i] = 0;
			pSqrMeanQuot[i] = 0;
			pSqrMeanRema[i] = 0;
		}

		uchar *pData = pSrc + subh*xRes*tblV;
		for(int row=0; row<subh; row++)
		{
			for (int xblk=0; xblk<xBlocks; xblk++)
		    {
				int sum = 0;
				int sqrsum = 0;
				int sqrmeanquot = 0; //square sum mean quotient
				int sqrmeanrema = 0; //square sum mean remain

				for(int x=0; x<subw; x++)
				{
					int value = *pData++; 
					sum += value;
					sqrsum += value*value;
					if( sqrsum >= MAX_SQRSUM )
					{
//MY_PRINT("error, sqrsum=%d\n", sqrsum);
						sqrmeanquot += sqrsum / subsize;
						sqrmeanrema += sqrsum % subsize;
						sqrsum = 0;
					}
				}
//MY_PRINT("tblV=%d, xblk=%d, sqrsum=%d, quot=%d rema=%d, \n", tblV, xblk, sqrsum, pSqrMeanQuot[xblk], pSqrMeanRema[xblk]);
				pSum[xblk] += sum;
				//pSqrSum[xblk] += sqrsum;
				pSqrMeanQuot[xblk] += sqrmeanquot + sqrsum/subsize;
				pSqrMeanRema[xblk] += sqrmeanrema + sqrsum%subsize;;
			}
		}	

		/* mean and stddev */
		for (int xblk=0; xblk<xBlocks; xblk++)
		{
//MY_PRINT("f: tblV=%d, xblk=%d, quot=%d rema=%d, \n", tblV, xblk, pSqrMeanQuot[xblk], pSqrMeanRema[xblk]);
			float mean = pSum[xblk] / (float)subsize;
			float sqrmean = pSqrMeanQuot[xblk] + pSqrMeanRema[xblk]/(float)subsize;
			pMean[tblV*xBlocks + xblk] = mean;
			pStddev[tblV*xBlocks + xblk] = sqrt(sqrmean - mean*mean);

			//int sqrmean = pSqrMeanQuot[xblk] + (pSqrMeanRema[xblk]+subsize/2)/subsize;
			//pMean[tblV*xBlocks + xblk] = mean;
			//pStddev[tblV*xBlocks + xblk] = sqrmean - pSum[xblk]*pSum[xblk]);
		}

#if 0 // TEST: pSum and pSqrSum check
	MY_PRINT("blk %d: ", tblV);
	for (int xblk=0; xblk<xBlocks; xblk++)
	{
		MY_PRINT("%d, ", pSum[xblk]);
	}
	MY_PRINT("\n");
	for (int xblk=0; xblk<xBlocks; xblk++)
	{
		MY_PRINT("%d, ", pSqrSum[xblk]);
	}
	MY_PRINT("\n");
	for (int xblk=0; xblk<xBlocks; xblk++)
	{
		MY_PRINT("%d, ", pMean[tblV*xBlocks + xblk]);
	}
	MY_PRINT("\n");
	for (int xblk=0; xblk<xBlocks; xblk++)
	{
		MY_PRINT("%d, ", pStddev[tblV*xBlocks + xblk]);
	}
	MY_PRINT("\n\n");
#endif		
	    if( xBlocks > 16 ) delete pSum;
	}

	return 0;
}

void SaveIntArray( char *pFilename, int *pArray, int rows, int cols )
{
	FILE *fp;
	int res = fopen_s(&fp, pFilename, "a+");
    if( NULL == fp )
	{
        std::cerr << "fopen failed: " << pFilename <<std::endl;
        return;
    }

    for( int i=0; i<rows; ++i )
	{  
        for( int j=0; j<cols; j++ )
        {
            char buffer[20];
            sprintf_s(buffer, "%4d ", *pArray++);
            fwrite(buffer, 1, strlen(buffer), fp);
        }
        fwrite("\n", 1, 1, fp);
    }

    fclose(fp);
}


#if 0
static bool
saveBufToFile(char const*const fname, char *const buf, int const size)
{
	int nw, cnt = 0;
	uint32_t written = 0;

	MY_LOGD("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
	MY_LOGD("opening file [%s]\n", fname);
	int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
	if (fd < 0) {
		CAM_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
		return false;
	}

	MY_LOGD("writing %d bytes to file [%s]\n", size, fname);
	while (written < size) {
		nw = ::write(fd,
			buf + written,
			size - written);
		if (nw < 0) {
			CAM_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
			break;
		}
		written += nw;
		cnt++;
	}
	MY_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
	::close(fd);
	return true;
}

/******************************************************************************
*   read the file to the buffer
*******************************************************************************/
static uint32_t
loadFileToBuf(char const*const fname, uint8_t*const buf, uint32_t size)
{
	int nr, cnt = 0;
	uint32_t readCnt = 0;

	MY_LOGD("opening file [%s] adr 0x%x\n", fname,buf);
	int fd = ::open(fname, O_RDONLY);
	if (fd < 0) {
		MY_LOGD("failed to create file [%s]: %s", fname, strerror(errno));
		return readCnt;
	}
	//
	if (size == 0) {
		size = ::lseek(fd, 0, SEEK_END);
		::lseek(fd, 0, SEEK_SET);
	}
	//
	MY_LOGD("read %d bytes from file [%s]\n", size, fname);
	while (readCnt < size) {
		nr = ::read(fd,
			buf + readCnt,
			size - readCnt);
		if (nr < 0) {
			CAM_LOGE("failed to read from file [%s]: %s",
				fname, strerror(errno));
			break;
		}
		readCnt += nr;
		cnt++;
	}
	MY_LOGD("done reading %d bytes to file [%s] in %d passes\n", size, fname, cnt);
	::close(fd);

	return readCnt;
}

#endif
