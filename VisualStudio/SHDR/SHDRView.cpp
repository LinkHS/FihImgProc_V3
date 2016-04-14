
// SHDRView.cpp : implementation of the CSHDRView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "SHDR.h"
#endif

#include "SHDRDoc.h"
#include "SHDRView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSHDRView

IMPLEMENT_DYNCREATE(CSHDRView, CListView)

BEGIN_MESSAGE_MAP(CSHDRView, CListView)
	ON_WM_STYLECHANGED()
END_MESSAGE_MAP()

// CSHDRView construction/destruction

CSHDRView::CSHDRView()
{
	// TODO: add construction code here

}

CSHDRView::~CSHDRView()
{
}

BOOL CSHDRView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CListView::PreCreateWindow(cs);
}

void CSHDRView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();


	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().
}


// CSHDRView diagnostics

#ifdef _DEBUG
void CSHDRView::AssertValid() const
{
	CListView::AssertValid();
}

void CSHDRView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CSHDRDoc* CSHDRView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSHDRDoc)));
	return (CSHDRDoc*)m_pDocument;
}
#endif //_DEBUG


// CSHDRView message handlers
void CSHDRView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	//TODO: add code to react to the user changing the view style of your window	
	CListView::OnStyleChanged(nStyleType,lpStyleStruct);	
}
