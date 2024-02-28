// DSTAddWorktreeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxdialogex.h"

#include "AppUtils.h"
#include "UnicodeUtils.h"
#include "GitStatusListCtrl.h"

#include "DSTWorktreeDlg.h"

namespace dst
{

std::string WorkTreeDirName(const std::string& branch_name);

}

// DSTAddWorktreeDlg dialog

IMPLEMENT_DYNAMIC(DstAddWorktreeDlg, CResizableStandAloneDialog)

DstAddWorktreeDlg::DstAddWorktreeDlg(CWnd* pParent /*=nullptr*/)
	: CResizableStandAloneDialog(IDD_DST_ADDWORKTREE, pParent)
	, m_strBranchName(_T(""))
	, m_strWorktreePath(_T(""))
{
}

DstAddWorktreeDlg::~DstAddWorktreeDlg()
{
}

void DstAddWorktreeDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableStandAloneDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_BRANCHNAME, m_strBranchName);
	DDX_Text(pDX, IDC_EDIT_WORKTREEPATH, m_strWorktreePath);
	DDX_Control(pDX, IDC_EDIT_BRANCHNAME, m_edtBranchName);
	DDX_Control(pDX, IDC_EDIT_WORKTREEPATH, m_edtWorktreePath);
}


BEGIN_MESSAGE_MAP(DstAddWorktreeDlg, CResizableStandAloneDialog)
	ON_EN_CHANGE(IDC_EDIT_BRANCHNAME, &DstAddWorktreeDlg::Update)
END_MESSAGE_MAP()


// DSTAddWorktreeDlg message handlers

BOOL DstAddWorktreeDlg::OnInitDialog()
{
	CResizableStandAloneDialog::OnInitDialog();

	m_edtBranchName.Init(-1);
	m_edtBranchName.SetFont(L"Segoe UI", 9);

	m_edtWorktreePath.Init(-1);
	m_edtWorktreePath.SetFont(L"Segoe UI", 9);

	if (!m_strBranchName.IsEmpty())
	{
		m_edtBranchName.SetText(m_strBranchName);
		m_edtBranchName.Call(SCI_EMPTYUNDOBUFFER);
	}
	AddAnchor(IDC_EDIT_BRANCHNAME, TOP_LEFT, TOP_RIGHT);

	if (!m_strWorktreePath.IsEmpty())
	{
		m_edtWorktreePath.SetText(m_strWorktreePath);
		m_edtWorktreePath.Call(SCI_EMPTYUNDOBUFFER);
	}
	AddAnchor(IDC_EDIT_WORKTREEPATH, TOP_LEFT, TOP_RIGHT);

	AddAnchor(IDC_BTN_BRANCHNAME_BROWSE, TOP_RIGHT, TOP_RIGHT);
	AddAnchor(IDC_BTN_WORKTREEPATH_BROWSE, TOP_RIGHT, TOP_RIGHT);

	AddAnchor(IDOK, BOTTOM_RIGHT, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT, BOTTOM_RIGHT);

	Update();

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DstAddWorktreeDlg::Update()
{
	m_strBranchName = m_edtBranchName.GetText();
	m_strBranchName.Trim();
	auto utf8_branch_name = CUnicodeUtils::GetUTF8(m_main_repo_path + "-" + m_strBranchName);
	auto dir_name = dst::WorkTreeDirName((LPCSTR)utf8_branch_name);
	m_strWorktreePath = CString(dir_name.c_str());
	m_edtWorktreePath.SetText(m_strWorktreePath);
	GetDlgItem(IDOK)->EnableWindow(!m_strBranchName.IsEmpty());
}

DstDropWorktreeDlg::DstDropWorktreeDlg(CWnd* pParent)
	: CCommitDlg(pParent, IDD_DSTDROPWORKTREE)
{
}

void DstDropWorktreeDlg::EnableSaveRestore(LPCWSTR, bool bRectOnly)
{
	CCommitDlg::EnableSaveRestore(L"DropWorktreeDlg", bRectOnly);
}

bool DstDropWorktreeDlg::RunStartCommitHook()
{
	return true;
}

BOOL DstDropWorktreeDlg::OnInitDialog()
{
	CRect rcWorktreePath;
	auto pedtWorktreePath = (CEdit*)GetDlgItem(IDC_EDIT_WORKTREEPATH);
	pedtWorktreePath->GetWindowRect(&rcWorktreePath);
	ScreenToClient(&rcWorktreePath);

	CCommitDlg::OnInitDialog();

	m_ListCtrl.SetContextMenuBit(GITSLC_POPSKIPWORKTREE, false);
	m_ListCtrl.SetContextMenuBit(GITSLC_POPASSUMEVALID, false);
	m_ListCtrl.SetContextMenuBit(GITSLC_POPRESTORE, false);
	m_ListCtrl.SetContextMenuBit(GITSLC_POPCHANGELISTS, false);

	pedtWorktreePath->MoveWindow(rcWorktreePath);
	pedtWorktreePath->SetWindowText(g_Git.m_CurrentDir);
	GetDlgItem(IDOK)->SetWindowText(L"Remove");

	if (auto pDeleteBranch = (CButton*)GetDlgItem(IDC_DELETEBRANCH))
	{
		AddAnchor(*pDeleteBranch, TOP_LEFT);
		pDeleteBranch->SetCheck(m_bDeleteBranch ? BST_CHECKED : BST_UNCHECKED);
	}
	if (auto pBranchName = (CEdit*)GetDlgItem(IDC_BRANCH_NAME))
	{
		pBranchName->SetWindowText(g_Git.GetCurrentBranch());
	}

	AddAnchor(IDC_EDIT_WORKTREEPATH, TOP_LEFT, TOP_LEFT);

	return TRUE;
}

void DstDropWorktreeDlg::OnOK()
{
	if (auto pDeleteBranch = (CButton*)GetDlgItem(IDC_DELETEBRANCH))
		m_bDeleteBranch = pDeleteBranch->GetCheck() == BST_CHECKED;

	CResizableStandAloneDialog::OnOK();
}

void DstDropWorktreeDlg::PrepareOkButton()
{
}

afx_msg LRESULT DstDropWorktreeDlg::OnUpdateOKButton(WPARAM, LPARAM)
{
	DialogEnableWindow(IDOK, TRUE);
	return 0;
}

BEGIN_MESSAGE_MAP(DstDropWorktreeDlg, CCommitDlg)
	ON_REGISTERED_MESSAGE(WM_UPDATEOKBUTTON, OnUpdateOKButton)
END_MESSAGE_MAP()
