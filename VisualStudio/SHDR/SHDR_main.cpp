#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "SHDR.h"
#endif

#include "SHDRDoc.h"
#include <conio.h>
#include "opencv.hpp"
#include "includes/debug.h"
#include "modules_cpp/modules_cpp.h" 

using namespace cv;

// CSHDRDoc commands
BOOL CSHDRDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	std::string filePath = CT2A(lpszPathName);
	Mat mSrc = imread(filePath);
	if( mSrc.data == NULL )
	{
		FIH_LOGD("OnOpenDocument\n");
		return FALSE;
	}

	imshow("src", mSrc);

	Mat mGray;
	cvtColor(mSrc, mGray, CV_BGR2GRAY);
    CLAHE_main(mGray.data, mGray.cols, mGray.rows, 8, 8, 0.0f);
	imshow("dst", mGray);
	
	return TRUE;
}