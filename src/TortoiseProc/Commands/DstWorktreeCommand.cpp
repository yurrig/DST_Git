#include "stdafx.h"

#include "AppUtils.h"
#include "UnicodeUtils.h"
#include "InputDlg.h"
#include "ProgressDlg.h"
#include "DstWorktreeCommand.h"
#include "DstWorktreeDlg.h"

#include <wincred.h>
#include <atlenc.h>

#include <vector>
#include <regex>

#include <filesystem>
namespace fs = std::filesystem;

namespace dst
{
char TargetName[] = "AzureDevOpsPAT";
const char ClientName[] = "Mozilla/5.0";
const std::string address = "dev.azure.com";
const std::string UrlPrefix = "https://" + address + "/";
const std::string OrganizationName = "downstreamtech";
const std::string ProjectName = "PCBProjects";

BOOL StoreAccessToken(LPBYTE pszToken, size_t size)
{
	CREDENTIALA credential = {};
	credential.TargetName = TargetName;
	credential.Type = CRED_TYPE_GENERIC;
	credential.Persist = CRED_PERSIST_LOCAL_MACHINE;
	credential.CredentialBlob = pszToken;
	credential.CredentialBlobSize = (int)size;

	return CredWriteA(&credential, 0);
}

std::string ToBase64(const std::string& src, size_t size)
{
	auto bytes = (const BYTE*)src.c_str();
	int byteLength = static_cast<int>(size);

	int base64Length = Base64EncodeGetRequiredLength(byteLength);
	std::vector<char> base64(base64Length + 1);

	auto s = (char*)base64.data();
	Base64Encode(bytes, byteLength, s, &base64Length);
	s[base64Length] = 0;

	return std::string(s);
}

std::string InvokeRestMethod(const std::string& aurl, std::string pat)
{
	std::string retval, url = aurl;
	if (HINTERNET hInternet = InternetOpenA(ClientName, INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0))
	{
		auto pos = url.find(UrlPrefix);
		if (pos != url.npos)
		{
			url = url.substr(pos + UrlPrefix.length());
		}
		HINTERNET hConnect = InternetConnectA(hInternet, address.c_str(),
			INTERNET_DEFAULT_HTTPS_PORT, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
		if (hConnect)
		{
			HINTERNET hRequest = HttpOpenRequestA(hConnect, "GET", url.c_str(), nullptr,
												 nullptr, nullptr, INTERNET_FLAG_SECURE, 0);
			if (hRequest)
			{
				pat = ":" + pat;
				std::string header = "Authorization: Basic " + ToBase64(pat.c_str(), pat.length());
				if (HttpAddRequestHeadersA(hRequest, header.c_str(), (DWORD)header.length(), 0))
				{
					if (HttpSendRequest(hRequest, nullptr, 0, nullptr, 0))
					{
						DWORD dwStatusCode = 0;
						DWORD dwLength = sizeof(dwStatusCode);
						HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwLength, nullptr);

						if (dwStatusCode == 200)
						{
							for (DWORD dwSize = 1, dwDownloaded = 0; dwSize > 0;)
							{
								dwSize = 0;
								if (InternetQueryDataAvailable(hRequest, &dwSize, 0, 0))
								{
									std::vector<char> out_buffer;
									out_buffer.resize(dwSize + 1);
									if (InternetReadFile(hRequest, (LPVOID)out_buffer.data(), dwSize, &dwDownloaded))
									{
										if (dwDownloaded > 0)
										{
											out_buffer[dwDownloaded] = 0;
											retval += out_buffer.data();
										}
									}
								}
							}
						}
					}
				}
				InternetCloseHandle(hRequest);
			}
			InternetCloseHandle(hConnect);
		}
		InternetCloseHandle(hInternet);
	}
	return retval;
}

std::string ReadAccessToken()
{
	PCREDENTIALA pCredential = nullptr;

	BOOL result = CredReadA(TargetName, CRED_TYPE_GENERIC, 0, &pCredential);
	if (result)
	{
		// Access token read successfully
		// You can access the access token using pCredential->CredentialBlob and pCredential->CredentialBlobSize
		return {
			(LPCSTR)pCredential->CredentialBlob,
			pCredential->CredentialBlobSize / sizeof(wchar_t)
		};
	}
	else
	{
		// Failed to read access token
	}

	if (pCredential != nullptr)
	{
		CredFree(pCredential);
	}

	return {};
}

void ReplaceAll(std::string& str, const std::string& from, const std::string& to)
{
	if (from.empty())
	{
		return;
	}
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

std::string GetClipText()
{
	std::string text;
	if (OpenClipboard(nullptr))
	{
		if (auto hData = GetClipboardData(CF_TEXT))
		{
			if (auto pszText = (LPCSTR)GlobalLock(hData))
			{
				text = pszText;
				GlobalUnlock(hData);
			}
		}
		CloseClipboard();
	}
	return text;
}

BranchDesc BranchDescFromUrl(const std::string& url, const std::string& Pat)
{
	std::string responce = InvokeRestMethod(url, Pat);

	auto to_cstring = [](const std::string& str) -> CString {
		return CUnicodeUtils::GetUnicode(str.c_str());
	};
	auto remove_bad_chars = [&](const CString& str) -> CString {
		std::wstring retval;
		for (int i = 0; i < str.GetLength(); ++i)
		{
			auto c = str[i];
			if (iswalnum(c))
				retval += towlower(c);
			else if (retval.back() != L'_')
				retval += L'_';
		}
		return retval.c_str();
	};

	std::regex re_defect("<title>(.+) ([0-9]+): (.+)</title>");
	std::match_results<std::string::const_iterator> match_defect;
	if (std::regex_search(responce, match_defect, re_defect))
	{
		std::string product, type = match_defect[1], id = match_defect[2], title = match_defect[3];
		if (type == "CAM350Defect")
			product = "cam350";
		else if (type == "BluePrintDefect")
			product = "blueprint";
		if (!product.empty())
		{
			CString branch_name = to_cstring(id) + L" " + to_cstring(title);
			return {
				BranchDesc::Defect,
				L"bugfix/" + to_cstring(product) + L"/" + remove_bad_chars(branch_name),
				remove_bad_chars(to_cstring(title)),
				to_cstring(id),
				to_cstring(product)
			};
		}
	}

	std::regex re_pr("\"pullRequestId\":([0-9]+)");
	std::match_results<std::string::const_iterator> match_pr;
	if (std::regex_search(responce, match_pr, re_pr))
	{
		std::string id = match_pr[1];
		if (!id.empty())
		{
			std::regex re_branch("\"sourceRefName\":\"refs\\/heads\\/(.*?)\"");
			std::match_results<std::string::const_iterator> match_branch;
			if (std::regex_search(responce, match_branch, re_branch))
			{
				std::string branch_name = match_branch[1];
				std::regex re_title("\"title\":\"(.*?)\"");
				if (std::regex_search(responce, match_branch, re_title))
				{
					std::string title = match_branch[1];
					return {
						BranchDesc::PR,
						to_cstring(branch_name),
						remove_bad_chars(to_cstring(title)),
						to_cstring(id)
					};
				};
			}
		}
	}

	return {};
}

BranchDesc BranchDescFromClipboard()
{
	dst::BranchDesc desc;
	auto clip_text = dst::GetClipText();
	std::string key = dst::UrlPrefix + dst::OrganizationName + "/" + dst::ProjectName + "/";
	auto pos = clip_text.find(key);
	if (pos == 0)
	{
		auto token = dst::ReadAccessToken();
		desc = dst::BranchDescFromUrl(clip_text, token);
	}
	return desc;
}

CString WorkTreeDirName(const BranchDesc& branch_desc)
{
	// need to limit output paths, as there may be very deep directories

	CString file_list;
	g_Git.Run(L"git ls-tree -r --name-only HEAD", &file_list, CP_UTF8);

	int position = 0, maxlen = INT_MIN;
	auto strToken = file_list.Tokenize(L"\n", position);
	while (!strToken.IsEmpty())
	{
		maxlen = std::max(maxlen, strToken.GetLength());
		strToken = file_list.Tokenize(L"\n", position);
	}

	int max_dirname_len = MAX_PATH - maxlen;
	auto branch_name = branch_desc.main_repo_path;
	if (branch_desc.type == BranchDesc::PR)
		branch_name += L"-PR_" + CString(branch_desc.id) + L"_" + branch_desc.title;
	else if (branch_desc.type == BranchDesc::Defect)
		branch_name += L"-" + branch_desc.name;
	CString dir_name = branch_name.Left(max_dirname_len - 2);

	dir_name.Replace(L'/', L'_');

	return dir_name;
}

void ImportClipboard(::DstAddWorktreeDlg *pDlg)
{
	auto main_worktree = pDlg->m_branch_desc.main_repo_path;
	pDlg->m_branch_desc = BranchDescFromClipboard();
	pDlg->m_branch_desc.main_repo_path = main_worktree;
}

std::string GetCredentials()
{
	CInputDlg dlg;
	dlg.m_sTitle = _T("Credentials");
	dlg.m_sHintText = _T("Personal Access Token:");
	std::string token;
	if (dlg.DoModal() == IDOK)
	{
		token = CUnicodeUtils::GetUTF8(dlg.m_sInputText);
		/*BOOL rc = */ dst::StoreAccessToken((LPBYTE)token.c_str(), token.size());
	}
	return token;
}

static fs::path CheckSolution(const fs::path& solution, const fs::path& dir)
{
	fs::path path = fs::current_path();

	fs::path subpath = path / solution;
	if (fs::exists(subpath))
		return subpath;

	subpath = path / dir / solution;
	if (fs::exists(subpath))
		return subpath;

	return {};
}

} // namespace dst

bool DstWorktreeCommand::Execute()
{
	auto token = dst::ReadAccessToken();
	if (token.empty())
	{
		token = dst::GetCredentials().c_str();
		if (token.empty())
			return false;
	}

	dst::BranchDesc desc;
	if (parser.HasKey(L"branch"))
		desc.name = CUnicodeUtils::GetUTF8(parser.GetVal(L"branch"));

	if (!m_bRemove && desc.name.IsEmpty())
		desc = dst::BranchDescFromClipboard();

	auto current_path = fs::current_path();

	fs::path path = parser.GetVal(L"path");
	desc.main_repo_path = path.c_str();

	std::error_code ec;
	if (!fs::exists(path, ec))
		return false;

	fs::current_path(path);
	g_Git.m_CurrentDir = path.c_str();

	if (m_bRemove)
	{
		try
		{
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

			if (fs::is_directory(path / ".git"))
			{
				MessageBox(*pExplorerWnd, L"Cannot remove main worktree", L"TortoiseGit", MB_ICONERROR);
				return false;
			}

			CString branch_name;
			g_Git.Run(L"git rev-parse --abbrev-ref HEAD", &branch_name, CP_UTF8);
			branch_name.Trim();

			DstDropWorktreeDlg dlg(pExplorerWnd);
			//dlg.m_pathList.AddPath(CString(path.c_str()));
			if (dlg.DoModal() == IDCANCEL)
				return false;

			fs::current_path(fs::path((LPCWSTR)main_worktree));
			g_Git.m_CurrentDir = (fs::current_path() / "").c_str();

			CProgressDlg progress(nullptr);
			progress.m_GitCmdList.push_back((L"cmd /c rmdir /s /q " + path.wstring()).c_str());
			progress.m_GitCmdList.push_back(L"git worktree prune -v");

			if (dlg.DeleteBranch())
				progress.m_GitCmdList.push_back(L"git branch -d " + branch_name);

			progress.DoModal();

			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	fs::path main_worktree_dir_name = path.filename();

	DstAddWorktreeDlg dlg;
	dlg.m_branch_desc = desc;

	if (dlg.DoModal() == IDCANCEL)
		return false;

	desc = dlg.m_branch_desc;
	fs::path worktree_path = (LPCWSTR)dlg.m_strWorktreePath;

	CString parent_branch;
	if (g_Git.Run(L"git rev-parse --abbrev-ref HEAD", &parent_branch, CP_UTF8))
		return false;
	parent_branch.Trim();

	if (g_Git.Run(L"git fetch", nullptr, CP_UTF8))
		return false;

	CString mergeRev;
	CProgressDlg progress(nullptr);

	auto exists = [](const fs::path& path) -> bool {
		try
		{
			switch (const auto type = fs::status(path).type())
			{
			case fs::file_type::none:
			case fs::file_type::not_found:
				return false;
			default:
				return true;
			}
		}
		catch (...)
		{
			return false;
		}
	};

	if (exists(worktree_path))
	{
		progress.m_PreFailText = (L"Path \"" + worktree_path.wstring() + L"\" already exists").c_str();
		progress.DoModal();
		return false;
	}

	progress.m_GitCmdList.push_back(L"git checkout " + parent_branch);
	progress.m_GitCmdList.push_back(L"git pull");

	auto branch_exists = [&](const CString& branch) -> bool {
		CString res;
		CString cmd = L"git show-ref --verify " + branch;
		return g_Git.Run(cmd, &res, CP_UTF8) == 0;
	};

	auto add_worktree = [&](const CString& branch, bool create = false, bool track = false) {
		CString cmd = (L"git worktree add \"" + worktree_path.wstring() + L"\" ").c_str();
		if (create)
			cmd += L"-b ";
		cmd += branch + L" ";
		if (track)
			cmd += L"--track origin/" + branch + L" ";
		progress.m_GitCmdList.push_back(cmd);
	};

	if (branch_exists(L"refs/heads/" + desc.name))
		add_worktree(desc.name);
	else if (branch_exists(L"refs/remotes/origin/" + desc.name))
		add_worktree(desc.name, true, true);
	else
		add_worktree(desc.name, true);

	 progress.m_PostCmdCallback = [&](DWORD status, PostCmdList& postCmdList) {
		if (status)
			return;
		postCmdList.emplace_back(L"Open CAM350 solution", [&] {
			auto path = worktree_path / "CAM350";
			ShellExecute(NULL, L"open", (path / "cam350.sln").c_str(), nullptr, path.c_str(), SW_SHOW);
		});
		postCmdList.emplace_back(L"Open BluePrint solution", [&] {
			auto path = worktree_path / "BluePrint";
			ShellExecute(NULL, L"open", (path / "BluePrint.sln").c_str(), nullptr, path.c_str(), SW_SHOW);
		});
	};

	auto cmd = "robocopy /s /np /ndl /njh /njs . " + worktree_path.string() + " .suo";
	progress.m_GitCmdList.push_back(CUnicodeUtils::GetUnicode(cmd.c_str()));

	[[maybe_unused]] auto rc = progress.DoModal();

	return false;
}

bool DstOpenSolutionCommand::Execute()
{
	auto solution = dst::CheckSolution(m_name, m_dir);
	ShellExecute(NULL, L"open", solution.c_str(), nullptr, solution.parent_path().c_str(), SW_SHOW);
	return true;
}
