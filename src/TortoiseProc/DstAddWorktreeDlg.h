﻿
#pragma once

#include "StandAloneDlg.h"
#include "SciEdit.h"


// DSTAddWorktreeDlg dialog

class DstAddWorktreeDlg : public CResizableStandAloneDialog
{
	DECLARE_DYNAMIC(DstAddWorktreeDlg)

public:
	DstAddWorktreeDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~DstAddWorktreeDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DST_ADDWORKTREE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	void Update();

public:
	CString m_main_repo_path;
	CString m_strBranchName;
	CString m_strWorktreePath;
	virtual BOOL OnInitDialog();
	CSciEdit m_edtBranchName;
	CSciEdit m_edtWorktreePath;
};
