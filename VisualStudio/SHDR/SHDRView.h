
// SHDRView.h : interface of the CSHDRView class
//

#pragma once


class CSHDRView : public CListView
{
protected: // create from serialization only
	CSHDRView();
	DECLARE_DYNCREATE(CSHDRView)

// Attributes
public:
	CSHDRDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CSHDRView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in SHDRView.cpp
inline CSHDRDoc* CSHDRView::GetDocument() const
   { return reinterpret_cast<CSHDRDoc*>(m_pDocument); }
#endif

