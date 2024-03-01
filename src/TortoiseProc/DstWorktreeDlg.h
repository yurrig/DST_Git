
#pragma once

#include "resource.h"
#include "CommitDlg.h"
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

class DstDropWorktreeDlg : public CCommitDlg
{
	BOOL m_bDeleteBranch = TRUE;

public:
	DstDropWorktreeDlg(CWnd* pParent = nullptr);

	void EnableSaveRestore(LPCWSTR, bool bRectOnly = FALSE) override;
	bool RunStartCommitHook() override;
	BOOL OnInitDialog() override;
	void OnOK() override;

	void PrepareOkButton() override;
	BOOL DeleteBranch() const { return m_bDeleteBranch; }

protected:
	afx_msg void OnTimer(UINT_PTR);
	afx_msg LRESULT OnUpdateOKButton(WPARAM, LPARAM);
	afx_msg LRESULT OnAutoListReady(WPARAM, LPARAM);
	afx_msg void OnBnClickedExplore();

	DECLARE_MESSAGE_MAP()
};
