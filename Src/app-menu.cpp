#include "stdafx.h"
#include "ImgProcDoc.h"
#include "ImgProcView.h"

#include "opencv.hpp"
#include "app.h"

#include <conio.h>
#include <stdlib.h>
#include <list>

void GetFileFromDir(CString csDirPath, std::list<std::string> &m_FileList);

void CImgProcView::OnShowGrayChannel()
{
    app_GetImgProcHandler()->ImageProcess(IMGPROC_ShowGrayChannel, NULL);

	/* 1. ((CImgProcView *)((CMainFrame *)AfxGetMainWnd())->GetActiveView())->Invalidate(); */
	/* 2. OnDraw( AfxGetMainWnd()->GetWindowDC() ); */
	Invalidate();
}

void CImgProcView::OnDehazing()
{
    app_GetImgProcHandler()->ImageProcess(IMGPROC_MyDehazing, NULL);

	Invalidate();
}

void CImgProcView::OnTest()
{
	//app_GetImgProcHandler()->ImageProcess(IMGPROC_CLAHE, NULL);IMGPROC_SHDR_CPP IMGPROC_PermTest IMGPROC_LMSFilter
    app_GetImgProcHandler()->ImageProcess(IMGPROC_SHDR_CPP, NULL);
}

//方差
void CImgProcView::On32786()
{
	app_GetImgProcHandler()->ImageProcess(IMGPROC_Var, NULL);
}

/**************************************************** multi image processing ***********************************************/
void CImgProcView::OnMulti_SHDR()
{
	CString str_path(_T("C:\\Users\\J6000275\\Desktop\\AdaptiveSHDR\\Original"));
	std::list<std::string> m_FileList;
	
	GetFileFromDir(str_path, m_FileList);
	app_GetImgProcHandler()->MultiImageProcess(m_FileList, IMGPROC_Var);
}



void GetFileFromDir(CString csDirPath, std::list<std::string> &m_FileList)
{
	const int MAX_NAME = 100;
	size_t len;
	HANDLE hFile;
	WIN32_FIND_DATA fileData;
	char filename[MAX_NAME];

	/* push back the path */
	wcstombs_s(&len, filename, csDirPath, MAX_NAME);
	m_FileList.push_back(filename); 

	csDirPath+="\\*.jpg";
	hFile = FindFirstFile(csDirPath.GetBuffer(), &fileData);
	wcstombs_s(&len, filename, fileData.cFileName, MAX_NAME);
	m_FileList.push_back(filename);
	
	BOOL bState = FALSE;
	bState = FindNextFile(hFile, &fileData);
	while(bState)
	{		
		wcstombs_s(&len, filename, fileData.cFileName, 100);
		m_FileList.push_back(filename);
		_cprintf("%s\n", filename);
		bState = FindNextFile(hFile, &fileData);
	}

	/* miuns 1 to remove the path */
	std::cout << "num of jpg files = " << m_FileList.size() - 1 << std::endl << std::endl;
}
