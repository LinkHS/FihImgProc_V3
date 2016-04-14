#include "stdafx.h"
#include "ImgProc.h" //CImgProcApp
#include "ImgProcDoc.h"
#include "ImgProcView.h"
#include "MainFrm.h"

#include "opencv.hpp"
#include "app.h"
#include "debug.h"

#define MY_PRINT               FIH_PRINT
#define MY_LOGD(fmt, ...)      FIH_LOGD("[%s] " fmt, __FUNCTION__, __VA_ARGS__)

void CImgProcView::OnDraw(CDC* pDC)
{
	CImgProcDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
    
	/* Get current window's size */
	CRect rect;
    GetClientRect(&rect);
	int nDstWidth = rect.Width();
	int nDstHeight = rect.Height();

    app_GetImgProcHandler()->SetDispSize(nDstWidth, nDstHeight);

    /* Get image size and Prepare the image data to display */
    IplImage *pIplImage = (IplImage *)(app_GetImgProcHandler()->PrepareDispData());
    if ( pIplImage == NULL )
        return;

    int nSrcWidth = pIplImage->width;
    int nSrcHeight = pIplImage->height;

    /* Initialize the BITMAPINFO structure */
    BITMAPINFO bitInfo;
    bitInfo.bmiHeader.biBitCount = 3*8;
    bitInfo.bmiHeader.biWidth = nSrcWidth;
    bitInfo.bmiHeader.biHeight = nSrcHeight;
    bitInfo.bmiHeader.biPlanes = 1;
    bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitInfo.bmiHeader.biCompression = BI_RGB;
    bitInfo.bmiHeader.biClrImportant = 0;
    bitInfo.bmiHeader.biClrUsed = 0;
    bitInfo.bmiHeader.biSizeImage = 0;
    bitInfo.bmiHeader.biXPelsPerMeter = 0;
    bitInfo.bmiHeader.biYPelsPerMeter = 0;

    int dstx = (nDstWidth - nSrcWidth)/2;
	int dsty = (nDstHeight - nSrcHeight)/2;

    /* Draw on the windows */
    pDC->SetStretchBltMode(COLORONCOLOR);
	HDC hDCDst = pDC->GetSafeHdc();
    StretchDIBits(hDCDst, dstx, dsty,
        nSrcWidth, nSrcHeight, 0, 0,
        nSrcWidth, nSrcHeight,
        pIplImage->imageData, &bitInfo, DIB_RGB_COLORS, SRCCOPY);

    MY_LOGD("dw %d, dh %d, sw %d, sh %d\r\n", nDstWidth, nDstHeight, nSrcWidth, nSrcHeight);

    cvReleaseImage( &pIplImage );
}

void CImgProcView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	app_GetImgProcHandler()->SetDispSize(cx, cy);
}

