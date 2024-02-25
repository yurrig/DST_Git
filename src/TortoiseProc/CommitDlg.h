﻿// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2003-2008 - TortoiseSVN
// Copyright (C) 2008-2023 - TortoiseGit

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#pragma once

#include "StandAloneDlg.h"
#include "GitStatusListCtrl.h"
#include "RegHistory.h"
#include "registry.h"
#include "SciEdit.h"
#include "SplitterControl.h"
#include "LinkControl.h"
#include "PathWatcher.h"
#include "BugTraqAssociations.h"
#include "../IBugTraqProvider/IBugTraqProvider_h.h"
#include "HyperLink.h"
#include "PatchViewDlg.h"
#include "MenuButton.h"
#include "EnableStagingTypes.h"
#include <regex>
#include "MassiveGitTask.h"

#define ENDDIALOGTIMER	100
#define REFRESHTIMER	101
#define FILLPATCHVTIMER	102

enum class Git_PostCommit_Cmd
{
	Nothing,
	ReCommit,
	Push,
	DCommit,
	Pull,
	CreateTag,
};


/**
 * \ingroup TortoiseProc
 * Dialog to enter log messages used in a commit.
 */
class CCommitDlg : public CResizableStandAloneDialog, public CSciEditContextMenuInterface, IHasPatchView
{
	DECLARE_DYNAMIC(CCommitDlg)

public:
	CCommitDlg(CWnd* pParent = nullptr, UINT nID = IDD); // standard constructor
	virtual ~CCommitDlg();

protected:
	// CSciEditContextMenuInterface
	void		InsertMenuItems(CMenu& mPopup, int& nCmd) override;
	bool		HandleMenuItemClick(int cmd, CSciEdit* pSciEdit) override;
	void		HandleSnippet(int type, const CString& text, CSciEdit* pSciEdit) override;

public:
	void ShowViewPatchText(bool b=true)
	{
		if(b)
			this->m_ctrlShowPatch.SetWindowText(CString(MAKEINTRESOURCE(IDS_PROC_COMMIT_SHOWPATCH)));
		else
			this->m_ctrlShowPatch.SetWindowText(CString(MAKEINTRESOURCE(IDS_PROC_COMMIT_HIDEPATCH)));

		m_ctrlShowPatch.Invalidate();
	}
	void ShowPartialStagingTextAndUpdateDisplayState(bool b = true)
	{
		if (b)
		{
			this->m_ctrlPartialStaging.SetWindowText(CString(MAKEINTRESOURCE(IDS_PROC_COMMIT_PARTIAL_STAGING)));
			m_stagingDisplayState |= SHOW_STAGING; // set
		}
		else
		{
			this->m_ctrlPartialStaging.SetWindowText(CString(MAKEINTRESOURCE(IDS_PROC_COMMIT_HIDE_STAGING)));
			m_stagingDisplayState &= ~SHOW_STAGING; // unset
		}
		m_ctrlPartialStaging.Invalidate();
	}
	void ShowPartialUnstagingTextAndUpdateDisplayState(bool b = true)
	{
		if (b)
		{
			this->m_ctrlPartialUnstaging.SetWindowText(CString(MAKEINTRESOURCE(IDS_PROC_COMMIT_PARTIAL_UNSTAGING)));
			m_stagingDisplayState |= SHOW_UNSTAGING; // set
		}
		else
		{
			this->m_ctrlPartialUnstaging.SetWindowText(CString(MAKEINTRESOURCE(IDS_PROC_COMMIT_HIDE_UNSTAGING)));
			m_stagingDisplayState &= ~SHOW_UNSTAGING; // unset
		}
		m_ctrlPartialUnstaging.Invalidate();
	}
	void SetAuthor(CString author)
	{
		m_bSetAuthor = TRUE;
		m_sAuthor = author;
	}
	void SetTime(CTime time)
	{
		m_bSetCommitDateTime = TRUE;
		m_wantCommitTime = time;
	}
private:
	CTime m_wantCommitTime;
	void ReloadHistoryEntries();
	static UINT StatusThreadEntry(LPVOID pVoid);
	UINT StatusThread();
	void FillPatchView(bool onlySetTimer = false);
	CWnd* GetPatchViewParentWnd() override { return this; }
	void TogglePatchView() override;
	void SetDlgTitle();
	CString GetSignedOffByLine();
	CString m_sTitle;
	void UpdateLogMsgByBugId(bool comapreExistingBugID);

// Dialog Data
	enum { IDD = IDD_COMMITDLG };

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	BOOL OnInitDialog() override;
	void OnOK() override;
	void PrepareIndexForCommitWithoutStagingSupport(int nListItems, bool& bAddSuccess, int& nchecked, CMassiveGitTask& mgtReAddAfterCommit, CMassiveGitTask& mgtReDelAfterCommit);
	void OnCancel() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;
	LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;
	afx_msg void OnBnClickedShowunversioned();
	afx_msg void OnBnClickedHistory();
	afx_msg void OnBnClickedBugtraqbutton();
	afx_msg void OnEnChangeLogmessage();
	afx_msg void OnFocusMessage();
	afx_msg void OnFocusFileList();
	afx_msg LRESULT OnGitStatusListCtrlItemCountChanged(WPARAM, LPARAM);
	afx_msg LRESULT OnGitStatusListCtrlNeedsRefresh(WPARAM, LPARAM);
	afx_msg LRESULT OnGitStatusListCtrlCheckChanged(WPARAM, LPARAM);
	afx_msg LRESULT OnGitStatusListCtrlItemChanged(WPARAM, LPARAM);

	afx_msg LRESULT OnCheck(WPARAM count, LPARAM);
	afx_msg LRESULT OnAutoListReady(WPARAM, LPARAM);
	afx_msg LRESULT OnUpdateOKButton(WPARAM, LPARAM);
	afx_msg LRESULT OnUpdateDataFalse(WPARAM, LPARAM);
	afx_msg LRESULT OnPartialStagingRefreshPatchView(WPARAM, LPARAM);
	afx_msg LRESULT OnFileDropped(WPARAM, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void Refresh();
	void StartStatusThread();
	void StopStatusThread();
	void GetAutocompletionList(std::map<CString, int>& autolist);
	void ScanFile(std::map<CString, int>& autolist, const CString& sFilePath, const CString& sRegex, const CString& sExt);
	void DoSize(int delta);
	void SetSplitterRange();
	void SaveSplitterPos();
	void UpdateCheckLinks();
	void ParseRegexFile(const CString& sFile, std::map<CString, CString>& mapRegex);
	void ParseSnippetFile(const CString& sFile, std::map<CString, CString>& mapSnippet);
	virtual bool RunStartCommitHook();
	void CreatePatchViewDlg();
	void DestroyPatchViewDlgIfOpen();
	void PrepareStagingSupport();

	DECLARE_MESSAGE_MAP()

public:
	CString				m_sLogMessage;
	BOOL				m_bDoNotAutoselectSubmodules;
	bool				m_bForceCommitAmend = false;
	BOOL				m_bCommitAmend;
	bool				m_bNoPostActions = false;
	bool				m_bSelectFilesForCommit = true;
	bool				m_bAutoClose = false;
	CString				m_AmendStr;
	CString				m_sBugID;
	BOOL				m_bWholeProject;
	BOOL				m_bWholeProject2;
	CTGitPathList		m_pathList;
	Git_PostCommit_Cmd	m_PostCmd = Git_PostCommit_Cmd::Nothing;
	BOOL				m_bAmendDiffToLastCommit;
	BOOL				m_bCommitMessageOnly;
	bool				m_bWarnDetachedHead = true;

protected:
	CTGitPathList		m_selectedPathList;
	CSciEdit			m_cLogMessage;
	INT_PTR				m_itemsCount = 0;
	CComPtr<IBugTraqProvider> m_BugTraqProvider;
	CString				m_NoAmendStr;
	BOOL				m_bCreateNewBranch;
	BOOL				m_bSetAuthor;
	CString				m_sAuthor;
	volatile bool		m_bDoNotStoreLastSelectedLine = true; // true on first start of commit dialog and set on recommit
	BOOL				m_bStagingSupport;
	volatile LONG		m_nBlockItemChangeHandler = FALSE; // used to make sure FillPatch is called only once after a partial staging

	int					CheckHeadDetach();

protected:
	CWinThread*			m_pThread = nullptr;
	std::map<CString, CString>	m_snippet;
	CGitStatusListCtrl	m_ListCtrl;
	BOOL				m_bShowUnversioned;
	volatile LONG		m_bBlock = FALSE;
	volatile LONG		m_bThreadRunning = FALSE;
	volatile LONG		m_bRunThread = FALSE;
	CRegDWORD			m_regAddBeforeCommit;
	CRegDWORD			m_regDoNotAutoselectSubmodules;
	CRegDWORD			m_regShowWholeProject;
	ProjectProperties	m_ProjectProperties;
	CString				m_sWindowTitle;
	static UINT			WM_AUTOLISTREADY;
	static UINT			WM_UPDATEOKBUTTON;
	static UINT			WM_UPDATEDATAFALSE;
	static UINT			WM_PARTIALSTAGINGREFRESHPATCHVIEW;
	int					m_nPopupPickCommitHash = 0;
	int					m_nPopupPickCommitMessage = 0;
	int					m_nPopupPasteListCmd = 0;
	int					m_nPopupPasteLastMessage = 0;
	int					m_nPopupRecentMessage = 0;
	CRegHistory			m_History;
	bool				m_bCancelled = false;
	CSplitterControl	m_wndSplitter;
	CRect				m_DlgOrigRect;
	CRect				m_LogMsgOrigRect;
	CPathWatcher		m_pathwatcher;
	CHyperLink			m_ctrlShowPatch;
	CHyperLink			m_ctrlPartialStaging;
	CHyperLink			m_ctrlPartialUnstaging;
	CPatchViewDlg		m_patchViewdlg;
	BOOL				m_bSetCommitDateTime;
	CDateTimeCtrl		m_CommitDate;
	CDateTimeCtrl		m_CommitTime;
	CButton				m_AsCommitDateCtrl;
	CLinkControl		m_linkControl;
	CString				m_sLogTemplate;
	CMenuButton			m_ctrlOkButton;
	CRegDWORD			m_regLastAction;
	virtual void		PrepareOkButton();
	struct ACCELLERATOR
	{
		int id;
		int cnt;
		int wmid;
	};
	std::map<wchar_t, ACCELLERATOR>	m_accellerators;
	HACCEL							m_hAccelOkButton = nullptr;

	CBugTraqAssociation	m_bugtraq_association;
	HACCEL				m_hAccel = nullptr;
	bool				RestoreFiles(bool doNotAsk = false, bool allowCancel = true);

	// State of the links "Partial Staging>>" and "Partial Unstaging>>" shown on the commit window
	enum PartialStagingDisplayState : unsigned char
	{
		// both unset should never happen
		SHOW_STAGING = 0x00000001, // unset when "Hide Staging"
		SHOW_UNSTAGING = 0x00000002, // unset when "Hide Unstaging"
	};
	unsigned char m_stagingDisplayState = SHOW_STAGING | SHOW_UNSTAGING;

protected:
	afx_msg void OnBnClickedSignOff();
	afx_msg void OnBnClickedCommitAmend();
	afx_msg void OnBnClickedCommitMessageOnly();
	afx_msg void OnBnClickedWholeProject();
	afx_msg void OnScnUpdateUI(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnStnClickedViewPatch();
	afx_msg void OnStnClickedPartialUnstaging();
	afx_msg void OnStnClickedPartialStaging();
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnHdnItemchangedFilelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCommitAmenddiff();
	afx_msg void OnBnClickedNoautoselectsubmodules();
	afx_msg void OnBnClickedCommitSetDateTime();
	afx_msg void OnBnClickedCommitAsCommitDate();
	afx_msg void OnBnClickedCheckNewBranch();
	afx_msg void OnBnClickedCommitSetauthor();
	afx_msg void OnBnClickedStagingSupport();

	CLinkControl		m_CheckAll;
	CLinkControl		m_CheckNone;
	CLinkControl		m_CheckUnversioned;
	CLinkControl		m_CheckVersioned;
	CLinkControl		m_CheckAdded;
	CLinkControl		m_CheckDeleted;
	CLinkControl		m_CheckModified;
	CLinkControl		m_CheckFiles;
	CLinkControl		m_CheckSubmodules;
};
