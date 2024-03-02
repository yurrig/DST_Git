// DSTAddWorktreeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxdialogex.h"

#include "Git.h"
#include "AppUtils.h"
#include "UnicodeUtils.h"
#include "GitStatusListCtrl.h"

#include "DSTWorktreeDlg.h"

#include <filesystem>
namespace fs = std::filesystem;

namespace dst
{

CString WorkTreeDirName(const BranchDesc& branch_desc);
std::string GetCredentials();
void ImportClipboard(::DstAddWorktreeDlg *pDlg);

}

// DSTAddWorktreeDlg dialog

IMPLEMENT_DYNAMIC(DstAddWorktreeDlg, CResizableStandAloneDialog)

DstAddWorktreeDlg::DstAddWorktreeDlg(CWnd* pParent /*=nullptr*/)
	: CResizableStandAloneDialog(IDD_DST_ADDWORKTREE, pParent)
{
}

DstAddWorktreeDlg::~DstAddWorktreeDlg()
{
}

void DstAddWorktreeDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableStandAloneDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_BRANCHNAME, m_branch_desc.name);
	DDX_Text(pDX, IDC_EDIT_WORKTREEPATH, m_strWorktreePath);
	DDX_Control(pDX, IDC_EDIT_BRANCHNAME, m_edtBranchName);
	DDX_Control(pDX, IDC_EDIT_WORKTREEPATH, m_edtWorktreePath);
}

afx_msg void DstAddWorktreeDlg::OnCredentials()
{
	dst::GetCredentials();
}

afx_msg void DstAddWorktreeDlg::OnImportClipboard()
{
	dst::ImportClipboard(this);
	m_edtBranchName.SetText(m_branch_desc.name);
	Update();
}

BEGIN_MESSAGE_MAP(DstAddWorktreeDlg, CResizableStandAloneDialog)
	ON_BN_CLICKED(IDC_BUTTON_CREDENTIALS, &OnCredentials)
	ON_BN_CLICKED(IDC_BUTTON_CLIPBOARD, &OnImportClipboard)
	ON_EN_CHANGE(IDC_EDIT_BRANCHNAME, &Update)
END_MESSAGE_MAP()


// DSTAddWorktreeDlg message handlers

BOOL DstAddWorktreeDlg::OnInitDialog()
{
	CResizableStandAloneDialog::OnInitDialog();

	m_edtBranchName.Init(-1);
	m_edtBranchName.SetFont(L"Segoe UI", 9);

	m_edtWorktreePath.Init(-1);
	m_edtWorktreePath.SetFont(L"Segoe UI", 9);

	if (!m_branch_desc.name.IsEmpty())
	{
		m_edtBranchName.SetText(m_branch_desc.name);
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

afx_msg void DstAddWorktreeDlg::Update()
{
	auto strBranchName = m_edtBranchName.GetText().Trim();
	if (!strBranchName.IsEmpty())
		m_branch_desc.name = strBranchName;
	m_strWorktreePath = dst::WorkTreeDirName(m_branch_desc);
	m_edtWorktreePath.SetText(m_strWorktreePath);
	GetDlgItem(IDOK)->EnableWindow(!m_branch_desc.name.IsEmpty());
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
	auto pedtWorktreePath = (CEdit*)GetDlgItem(IDC_EDIT_WORKTREE_PATH);
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
	if (auto pBranchName = (CEdit*)GetDlgItem(IDC_EDIT_BRANCH_NAME))
	{
		pBranchName->SetWindowText(g_Git.GetCurrentBranch());
	}

	AddAnchor(IDC_EDIT_WORKTREE_PATH, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_BUTTON_EXPLORE, TOP_RIGHT, TOP_RIGHT);
	AddAnchor(IDC_EDIT_BRANCH_NAME, TOP_LEFT, TOP_RIGHT);

	DialogEnableWindow(IDOK, FALSE);

	SetTimer(1, 100, nullptr);

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

afx_msg void DstDropWorktreeDlg::OnBnClickedExplore()
{
	if (auto pidl = ILCreateFromPath(g_Git.m_CurrentDir))
	{
		SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
		ILFree(pidl);
	}
}

afx_msg LRESULT DstDropWorktreeDlg::OnAutoListReady(WPARAM wp, LPARAM lp)
{
	auto count = m_ListCtrl.GetItemCount();
	DialogEnableWindow(IDOK, !count);
	return CCommitDlg::OnAutoListReady(wp, lp);
}

afx_msg void DstDropWorktreeDlg::OnTimer(UINT_PTR)
{
	auto count = m_ListCtrl.GetItemCount();
	DialogEnableWindow(IDOK, !count);
}

BEGIN_MESSAGE_MAP(DstDropWorktreeDlg, CCommitDlg)
	ON_WM_TIMER()
	ON_REGISTERED_MESSAGE(WM_UPDATEOKBUTTON, OnUpdateOKButton)
	ON_REGISTERED_MESSAGE(WM_AUTOLISTREADY, OnAutoListReady)
	ON_BN_CLICKED(IDC_BUTTON_EXPLORE, OnBnClickedExplore)
END_MESSAGE_MAP()
