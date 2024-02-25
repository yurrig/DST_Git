#include "stdafx.h"

#include "resource.h"
#include "CommitDlg.h"

#include "DstDropWorktreeCommand.h"

#include <filesystem>
namespace fs = std::filesystem;

static std::vector<UINT> CtrlsToHide = {
	IDC_CHECK_NEWBRANCH,
	IDC_BUGIDLABEL,
	IDC_BUGID,
	IDC_BUGTRAQBUTTON,
	IDC_MESSAGEGROUP,
	IDC_LOGMESSAGE,
	IDC_COMMIT_AMEND,
	IDC_COMMIT_SETDATETIME,
	IDC_COMMIT_SETAUTHOR,
	IDC_COMMIT_AMENDDIFF,
	IDC_COMMIT_DATEPICKER,
	IDC_COMMIT_TIMEPICKER,
	IDC_COMMIT_AS_COMMIT_DATE,
	IDC_COMMIT_AUTHORDATA,
	IDC_SIGNOFF,
	IDC_SPLITTER,
	IDC_STAGINGSUPPORT,
	IDC_NOAUTOSELECTSUBMODULES,
	IDC_WHOLE_PROJECT,
	IDC_COMMIT_MESSAGEONLY,
	IDC_VIEW_PATCH,
	IDC_PARTIAL_STAGING,
	IDC_PARTIAL_UNSTAGING,
	IDI_MERGEACTIVE,
	IDC_COMMIT_TO,
};

class DstDropWorktreeDlg : public CCommitDlg
{
public:
	DstDropWorktreeDlg(CWnd *pParent = nullptr)
		: CCommitDlg(pParent, IDD_DSTDROPWORKTREE)
	{
	}

	void EnableSaveRestore(LPCWSTR, bool bRectOnly = FALSE) override
	{
		CCommitDlg::EnableSaveRestore(L"DropWorktreeDlg", bRectOnly);
	}
	bool RunStartCommitHook() override
	{
		return true;
	}
	BOOL OnInitDialog() override
	{
		CRect rcWorktreePath;
		auto pedtWorktreePath = (CEdit*)GetDlgItem(IDC_EDIT_WORKTREEPATH);
		pedtWorktreePath->GetWindowRect(&rcWorktreePath);
		ScreenToClient(&rcWorktreePath);

		CCommitDlg::OnInitDialog();

		for (auto id : CtrlsToHide)
		{
			if (auto pCtrl = GetDlgItem(id))
				pCtrl->ShowWindow(SW_HIDE);
		}
		GetDlgItem(IDC_COMMITLABEL)->SetWindowText(L"Worktree path: ");

		pedtWorktreePath->MoveWindow(rcWorktreePath);
		pedtWorktreePath->SetWindowText(m_pathList[0].GetDisplayString());
		GetDlgItem(IDOK)->SetWindowText(L"Remove");

		AddAnchor(IDC_EDIT_WORKTREEPATH, TOP_LEFT, TOP_LEFT);

		return TRUE;
	}
	void OnOK() override
	{
		CResizableStandAloneDialog::OnOK();
	}

	void PrepareOkButton() override
	{
	}

	afx_msg LRESULT OnUpdateOKButton(WPARAM, LPARAM)
	{
		DialogEnableWindow(IDOK, TRUE);
		return 0;
	}

	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(DstDropWorktreeDlg, CCommitDlg)
	ON_REGISTERED_MESSAGE(WM_UPDATEOKBUTTON, OnUpdateOKButton)
END_MESSAGE_MAP()

bool DstDropWorktreeCommand::Execute()
{
	try
	{
		fs::path path = parser.GetVal(L"path");
		fs::path current_path = fs::current_path();
		fs::current_path(path);
		g_Git.m_CurrentDir = path.c_str();

		auto pExplorerWnd = CWnd::FromHandle(GetExplorerHWND());

		CString main_worktree;
		if (g_Git.Run(L"git rev-parse --path-format=absolute --git-common-dir", &main_worktree, CP_UTF8))
			main_worktree = CString();

		main_worktree.Trim();
		if (main_worktree.IsEmpty())
		{
			MessageBox(*pExplorerWnd, L"Could not find main worktree", L"TortoiseGit", MB_ICONERROR);
			return false;
		}

		CString branch_name;
		g_Git.Run(L"git rev-parse --abbrev-ref HEAD", &branch_name, CP_UTF8);
		branch_name.Trim();

		DstDropWorktreeDlg dlg(pExplorerWnd);
		dlg.m_pathList.AddPath(CString(path.c_str()));
		if (dlg.DoModal() == IDCANCEL)
			return false;

		fs::current_path(fs::path((LPCWSTR)main_worktree).parent_path());
		g_Git.m_CurrentDir = fs::current_path().c_str();

		CProgressDlg progress(nullptr);
		progress.m_GitCmdList.push_back((L"git worktree remove " + path.wstring()).c_str());
		progress.m_GitCmdList.push_back(L"git branch -d " + branch_name);

		progress.DoModal();

		return true;
	}
	catch (...)
	{
		return false;
	}
}
