#include "fihimgproc_inc.h"
#include "LMVFilter.h"
#include <string.h>

class LMVFilterImpl
{
public:
    virtual                     ~LMVFilterImpl() {}
    void                        filter( uchar *pDst, int r, int level );

protected:
    int                         Idepth;
    bool                        initWithData;
    
private:
    virtual void                filterSingleChannel( uchar *_pSrc, uchar *_pDst, int r, int level) = 0;
};

class LMVFilterMono : public LMVFilterImpl
{
public:
                                LMVFilterMono( uchar *pSrc, int cols, int rows );
	virtual                    ~LMVFilterMono();

public:
	/* for omp use */
	int                         IcpuNum;    // number of cpu
	int                         IblkHeight; // height of sub-block 

private:
    void                        filterSingleChannel( uchar *_pSrc, uchar *_pDst, int r, int level );
    void                        cptSppANDSqrSqq( bool padded );
	void                        cptLocalSumANDSqrSum( int *pCurLSum, int *pCurLSqrSum, int rowIdx, int r );
    //void                        test(void);

private:
	/* image info */
	uchar                       *IpSrc;
	int                         ICols;
    int                         IRows;

    int                         *IpMyMem;
    int                         *IpPreLSum;     //previous local sum
    int                         *IpPreLSqrSum;  //previous local square sum
    int                         *IpCurLSum;     //current local sum
    int                         *IpCurLSqrSum;  //current local square sum
	int                         *IpSpp;
	int                         *IpSqrSpp;
};

/* overrigde data in mSrc */
void LMVFilterImpl::filter( uchar *pDst, int r, int level )
{
    filterSingleChannel( NULL, pDst, r, level );
}

LMVFilterMono::LMVFilterMono( uchar *pSrc, int cols, int rows )
{
    ICols = cols;
    IRows = rows;
	IpSrc = pSrc;

	IcpuNum = 8;
	IblkHeight = IRows/IcpuNum;

	/* alloc memery */
	int memsize = cols*4*IcpuNum + cols*rows*2;
//MY_PRINT("memsize: %d %d\n", memsize, memsize*IcpuNum);
    IpMyMem = new int[memsize];
//MY_PRINT("IpMyMem: %x\n", IpMyMem);
    memset(IpMyMem, 0, cols*4*IcpuNum*sizeof(int));

    IpPreLSum    = IpMyMem + cols*0*IcpuNum;  //previous local sum
    IpPreLSqrSum = IpMyMem + cols*1*IcpuNum;  //previous local square sum
    IpCurLSum    = IpMyMem + cols*2*IcpuNum;  //current local sum
    IpCurLSqrSum = IpMyMem + cols*3*IcpuNum;  //current local square sum

	IpSpp    = IpMyMem + cols*4*IcpuNum;
	IpSqrSpp = IpSpp + cols*rows;	

	if ( IpSrc != NULL )
        cptSppANDSqrSqq( 0 );
}

LMVFilterMono::~LMVFilterMono(void)
{
    delete IpMyMem;
}


/* 
  Compute superposed results of each pixel and its square vaule in each row.
 
    Src:    12, 23, 32, 11, 223, 32,  231, 34,  ......
  Superposed result:
    Padded:  0, 12, 35, 67, 78,  301, 333, 564, 598, ......
    no pad: 12, 35, 67, 78, 301, 333, 564, 598, ......
*/
void LMVFilterMono::cptSppANDSqrSqq( bool padded )
{
    int nRows = IRows;
    int nCols = (padded == 1) ? ICols+1 : ICols;

    if ( padded == 1 )
    {
        for (int i=0; i<nRows; i++)
        {  
            uchar* pSrc = IpSrc + i*nCols;
            int *pSppRow    = IpSpp + + i*nCols;
            int *pSppSqrRow = IpSqrSpp + + i*nCols;

            for( int j=1; j<nCols; j++)
            {
                pSppRow[j]    = pSppRow[j-1]    + pSrc[j-1];
                pSppSqrRow[j] = pSppSqrRow[j-1] + pSrc[j-1]*pSrc[j-1];
            }
        }
    }else
    {
        for (int i=0; i<nRows; i++)
        {  
            uchar* pSrc = IpSrc + i*nCols;
            int *pSppRow    = IpSpp + i*nCols;
            int *pSppSqrRow = IpSqrSpp + i*nCols;

            pSppRow[0]    = pSrc[0];
            pSppSqrRow[0] = pSrc[0]*pSrc[0];

            for( int j=1; j<nCols; j++)
            {
                pSppRow[j]    = pSppRow[j-1]    + pSrc[j];
                pSppSqrRow[j] = pSppSqrRow[j-1] + pSrc[j]*pSrc[j];
                //pSppSqrRow[j] = pSppSqrRow[j-1] + table_u8Squares[pSrc[j]];
            }
        }
    }
}

#if 0
#ifndef CPU_NUM
const int cpus = 8;
#else
const int cpus = CPU_NUM; 
#endif

/*
* Allocate memory
*/
int *Mem = new int[xRes*yRes*2]; // size of SumTbl and SqrSumTbl
int *SumTbl    = Mem;
int *SqrSumTbl = Mem + xRes*yRes;

/*
* Calculate accumulated sum/sqrSum(sum of squares) of each row
*/
for(int cpu=0; cpu<cpus; cpu++) 
{
	int blkH = yRes / cpus;
	int blkSize = blkH * xRes;
	uchar *pData = pSrc + blkSize*cpu;
	int *pSum = SumTbl + blkSize*cpu;
	int *pSqrSum = SqrSumTbl + blkSize*cpu; 

	if( cpu == (cpus-1) ) //last block
		blkH += yRes - blkH*cpus; // avoid yRes is no multiple of cpus

	for (int row=0; row<blkH; row++)
	{
		int value = *pData++; 
		*pSum++ = value;
		*pSqrSum++ = value*value;
		for (int col=1; col<xRes; col++)
		{
			value = *pData++; 
			*pSum++ = *(pSum-1) + value;
			*pSqrSum++ = *(pSqrSum-1) + value*value;
		}
	}
}

//free memory
delete Mem;

#endif

#if 0 // TEST: SumTbl and SqrSumTbl check
for (int row=0; row<yRes; row++)
{
	MY_PRINT("row %d: ", row);
	for (int col=0; col<xRes; col++)
	{
		MY_PRINT("%d, ", SumTbl[row*xRes + col]);
	}
	MY_PRINT("\n");
}
#endif


/* 
'rowIdx' should not smaller than 'r' 
 
e.g. cptLocalSumANDSqrSum(5,4) 
     '-' are origin values, '*' are computed values, '~' need to be used
    0: ----------------------------------
    1: ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    2: ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    3: ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    4: ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    5: ~~~~**************************~~~~
    6: ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    7: ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    8: ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    9: ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   10: ----------------------------------
*/
void LMVFilterMono::cptLocalSumANDSqrSum( int *pCurLSum, int *pCurLSqrSum, int rowIdx, int r )
{
    int kh = r*2 + 1;  //kernel height
    int kw = r*2 + 1;  //kernel width

    int firstRow = rowIdx - r;
    if ( firstRow < 0 )
        firstRow = 0;
    else if ( (firstRow+r) >= IRows )
        firstRow = IRows - kh;

    /* radius+1 section to radius+1 last section, only compute offset value */
    /* ----*******************************---- */
    for( int i=firstRow; i<firstRow+kh; i++ )
    {
        int *pSppRow    = IpSpp + ICols*i;
        int *pSqrSppRow = IpSqrSpp + ICols*i;

        /* compute r col independently. As spp is not padded, spp[0] is not zero. */
        pCurLSum[r]    += pSppRow[kw-1];
        pCurLSqrSum[r] += pSqrSppRow[kw-1];

        for( int j=r+1; j<ICols-r; j++)
        {
            pCurLSum[j]    += pSppRow[j+r]    - pSppRow[j-r-1];
            pCurLSqrSum[j] += pSqrSppRow[j+r] - pSqrSppRow[j-r-1];
        }
    }

#define COPY 0
#if 0
    /* Copy to 'first col to radius-1 col' */
    for( int j=0; j<r; j++)
    {
        IpCurLSum[j]    = IpCurLSum[r];
        pCurLSqrSum[j] = pCurLSqrSum[r];
    }
    /* Copy to last radius cols */
    for( int j=nCols-r; j<nCols; j++)
    {
        IpCurLSum[j]    = IpCurLSum[nCols-r-1];
        pCurLSqrSum[j] = pCurLSqrSum[nCols-r-1];
    }
#endif
#undef COPY
}

#if 0
void LMVFilterMono::filterRowHead( uchar* pSrc, uchar* pDst, int lMean, int lSdv, int r )
{

    pDst[j] = (uchar)(alpha*mean/(lSdv+alpha) + lSdv*pSrc[j]/(lSdv+alpha));
}
#endif

void LMVFilterMono::filterSingleChannel( uchar *_pSrc, uchar *_pDst, int r, int level )
{
    if ( IpSrc == NULL )
    {
		IpSrc = _pSrc;
        cptSppANDSqrSqq( 0 );
    }

	int kh = r*2 + 1;  //kernel height
    int kw = r*2 + 1;  //kernel width
    int r1 = r+1;
    int area = kh*kw;
    int alpha = level * level * 5 + 10;

	#pragma omp parallel for
	for(int i=0; i<IcpuNum; i++)
	{
		int blkSta = (i==0) ? r : IblkHeight*i;
	    int blkEnd = (i==IcpuNum-1) ? IRows-r : IblkHeight*(i+1);
//MY_LOGD("cpu: %d, blkSta %d, blkEnd %d\n", i, blkSta, blkEnd);
		cptLocalSumANDSqrSum( IpCurLSum+ICols*i, IpCurLSqrSum+ICols*i, IblkHeight*i, r );

		/* Start: compute result in r row */
		uchar* pSrc = IpSrc + blkSta*ICols;
		uchar* pDst = _pDst + blkSta*ICols;
		int *pPreLSum    = IpPreLSum + ICols*i;        //previous local sum
		int *pPreLSqrSum = IpPreLSqrSum + ICols*i;  //previous local square sum
		int *pCurLSum    = IpCurLSum + ICols*i;        //current local sum
		int *pCurLSqrSum = IpCurLSqrSum + ICols*i;  //current local square sum

		for( int j=r; j<ICols-r; j++)
		{
			int lMean = pCurLSum[j] / area;
			int lSdv = (pCurLSqrSum[j] - (lMean * pCurLSum[j])) / area;

			pDst[j] = (uchar)(alpha*lMean/(lSdv+alpha) + lSdv*pSrc[j]/(lSdv+alpha));
		}
		/* End: compute result in r row  */

		/* Start: compute result from r+1 row to last r+1 row */
		for (int i=blkSta+1; i<blkEnd; i++)
		{  
			pSrc = IpSrc + ICols*i;
			pDst = _pDst + ICols*i;

			int *pSppRow_rm     = IpSpp + ICols*(i-r1);   // the row to be removed
			int *pSppSqrRow_rm  = IpSqrSpp + ICols*(i-r1); // the row to be removed
			int *pSppRow_add    = IpSpp + ICols*(i+r);     // the row to be added
			int *pSppSqrRow_add = IpSqrSpp + ICols*(i+r);  // the row to be added

			/* exchange buf */
			int *temp = pPreLSum; pPreLSum = pCurLSum; pCurLSum = temp;
			temp = pPreLSqrSum; pPreLSqrSum = pCurLSqrSum; pCurLSqrSum = temp;

			pCurLSum[r]    = pPreLSum[r]    + pSppRow_add[r*2]    - pSppRow_rm[r*2];
			pCurLSqrSum[r] = pPreLSqrSum[r] + pSppSqrRow_add[r*2] - pSppSqrRow_add[r*2];

			int mean    = pCurLSum[r] / area;
			int sqrmean = pCurLSqrSum[r] / area;

			//TODO
			pDst[r] = mean;

			for( int j=r1; j<ICols-r; j++)
			{
				int add    = pSppRow_add[j+r] - pSppRow_add[j-r1];
				int sqradd = pSppSqrRow_add[j+r] - pSppSqrRow_add[j-r1];
				int rm     = pSppRow_rm[j+r] - pSppRow_rm[j-r1];
				int sqrrm  = pSppSqrRow_rm[j+r] - pSppSqrRow_rm[j-r1];

				pCurLSum[j]    = pPreLSum[j] + add - rm;
				pCurLSqrSum[j] = pPreLSqrSum[j] + sqradd - sqrrm;

				mean    = pCurLSum[j] / area;
				//sqrmean = IpCurLSqrSum[j] / area;

				int lSdv = (pCurLSqrSum[j] - (mean * pCurLSum[j])) / area;
				//lSdv = 0;
				//TODO
				pDst[j] = (uchar)(alpha*mean/(lSdv+alpha) + lSdv*pSrc[j]/(lSdv+alpha));
				//pDst[j] = mean;
			}
		}
		/* End: compute result from r+1 row to last r+1 row */
	}
}

LMVFilter::LMVFilter( uchar *pSrc, int h, int w )
{
    impl_ = new LMVFilterMono( pSrc, w, h );
}

LMVFilter::~LMVFilter()
{
    delete impl_;
}

void LMVFilter::filter( uchar *pDst, int r, int level )
{
    impl_->filter( pDst, r, level );
}