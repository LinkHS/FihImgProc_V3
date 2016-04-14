


/*
	Function: AirlightEstimation
	Description: estimate the atmospheric light value in a hazy image.
			     it divides the hazy image into 4 sub-block and selects the optimal block, 
				 which has minimum std-dev and maximum average value.
				 *Repeat* the dividing process until the size of sub-block is smaller than 
				 pre-specified threshold value. Then, We select the most similar value to
				 the pure white.
				 IT IS A RECURSIVE FUNCTION.
	Parameter: 
		imInput - input image (cv iplimage)
	Return:
		m_anAirlight: estimated atmospheric light value
 */
//void dehazing::AirlightEstimation(IplImage* imInput)
void DehazingColor::AirlightEstimation(int minSize)
{
MY_LOGD("+");
    int BlockTBLR[4];
    int bT, bB, bL, bR;
    int nCols;
    //int nMinDistance = 65536;
    int nMaxDistance = 0;

    LookForSubBlock_MiniStdDev(200, BlockTBLR);
    bT=BlockTBLR[0], bB=BlockTBLR[1], bL=BlockTBLR[2], bR=BlockTBLR[3];
    nCols = (bR-bL+1)*3; //RGB channel is 3
   
    /* select the atmospheric light value in the sub-block */
    for( int j=bT; j<bB; j++ )
    {
        uchar *pSrc = ImSrc.ptr<uchar>(j); // a pointer points to current row
        
        for( int i=bL; i<bL+nCols; i+=3 )
        {
            /* 255-r, 255-g, 255-b */
            //int nDistance = sqrt( (255-R)*(255-R) + (255-G)*(255-G) + (255-B)*(255-B) );                 
            int nDistance = pSrc[i] + pSrc[i+1] + pSrc[i+2];   

            if( nMaxDistance < nDistance )
            {
                nMaxDistance = nDistance;
                IAtmosLight[0] = pSrc[i+0]; //Blue
                IAtmosLight[1] = pSrc[i+1]; //Green
                IAtmosLight[2] = pSrc[i+2]; //Red
            }
        }
    }

#if DEBUG
    DEBUG_AirLigthtBlock[0] = BlockTBLR[0];
    DEBUG_AirLigthtBlock[1] = BlockTBLR[1];
    DEBUG_AirLigthtBlock[2] = BlockTBLR[2];
    DEBUG_AirLigthtBlock[3] = BlockTBLR[3];
#endif
}


float DehazingColor::NFTrsEstimation(int nStartX, int nStartY)
{
	int nCounter;	
	int nX, nY;		
	int nEndX;
	int nEndY;

	int nOutR, nOutG, nOutB;					
	int nSumofOuts;					
	int nSumofSquaredOuts;			
	float fTrans, fOptTrs;			
	int nTrans;						
	int nSumofSLoss;				
	float fCost, fMinCost, fMean;	
	int nNumberofPixels, nLossCount;

	nEndX = std::min(nStartX+ITranBlkSize, IWidth); 
	nEndY = std::min(nStartY+ITranBlkSize, IHeight); 

	nNumberofPixels = (nEndY-nStartY)*(nEndX-nStartX) * 3;	

	fTrans = 0.3f;	
	nTrans = 427;

	for(nCounter=0; nCounter<7; nCounter++)
	{
		nSumofSLoss = 0;
		nLossCount = 0;
		nSumofSquaredOuts = 0;
		nSumofOuts = 0;

		for(nY=nStartY; nY<nEndY; nY++)
		{
			/* uchar* pSrc = mSrc.ptr<uchar>(nY); */
			uchar* pSrc = (uchar*)(ImTransmission.data + ImTransmission.step.p[0] * nY);

			//for(nX=nStartX; nX<nEndX; nX++)
			for(nX=nStartX*3; nX<nEndX*3; nX+=3)
			{
				/* (I-A)/t + A --> ((I-A)*k*128 + A*128)/128 */
				nOutB = ((pSrc[nX+0] - IAtmosLight[0])*nTrans + 128*IAtmosLight[0]) >> 7;
				nOutG = ((pSrc[nX+1] - IAtmosLight[1])*nTrans + 128*IAtmosLight[1]) >> 7;
				nOutR = ((pSrc[nX+2] - IAtmosLight[2])*nTrans + 128*IAtmosLight[2]) >> 7;

				if(nOutR>255)
				{
				 nSumofSLoss += (nOutR - 255)*(nOutR - 255);
				 nLossCount++;
				}
				else if(nOutR < 0)
				{
				 nSumofSLoss += nOutR * nOutR;
				 nLossCount++;
				}
				if(nOutG>255)
				{
				 nSumofSLoss += (nOutG - 255)*(nOutG - 255);
				 nLossCount++;
				}
				else if(nOutG < 0)
				{
				 nSumofSLoss += nOutG * nOutG;
				 nLossCount++;
				}
				if(nOutB>255)
				{
				 nSumofSLoss += (nOutB - 255)*(nOutB - 255);
				 nLossCount++;
				}
				else if(nOutB < 0)
				{
				 nSumofSLoss += nOutB * nOutB;
				 nLossCount++;
				}
				nSumofSquaredOuts += nOutB * nOutB + nOutR * nOutR + nOutG * nOutG;;
				nSumofOuts += nOutR + nOutG + nOutB;
			}
		}
		fMean = (float)(nSumofOuts)/(float)(nNumberofPixels);  
		fCost = ILambda * (float)nSumofSLoss/(float)(nNumberofPixels) 
					- ((float)nSumofSquaredOuts/(float)nNumberofPixels - fMean*fMean); 

		if(nCounter==0 || fMinCost > fCost)
		{
			fMinCost = fCost;
			fOptTrs = fTrans;
		}

		fTrans += 0.1f;
		nTrans = (int)(1.0f/fTrans*128.0f);
	}

	return fOptTrs; 
}

