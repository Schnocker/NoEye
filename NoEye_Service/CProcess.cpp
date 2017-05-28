#include "CProcess.hpp"

namespace Process
{
	CProcess::CProcess()
	{
		this->ProcessName = "";
		this->hProcess = GetCurrentProcess();
	}
	CProcess::CProcess(DWORD dwProcessId, DWORD dwDesiredAccess)
	{
		this->ProcessName = "";
		this->hProcess = OpenProcess(dwDesiredAccess, false, dwProcessId);
	}
	CProcess::CProcess(std::string ProcessName)
	{
		this->ProcessName = ProcessName;
		this->hProcess = 0;
	}
	CProcess::CProcess(HANDLE hProcess)
	{
		this->ProcessName = "";
		this->hProcess = hProcess;
	}
	CProcess::~CProcess()
	{

	}

	std::map<std::string, std::uint32_t> CProcess::GetProcessList()
	{
		std::map<std::string, uint32_t> ProcessList;
		PROCESSENTRY32 pe32;
		HANDLE hSnapshot = 0;
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE || hSnapshot == 0)
			goto EXIT;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (!Process32First(hSnapshot, &pe32))
			goto EXIT;
		do
		{
			ProcessList[pe32.szExeFile] = pe32.th32ProcessID;
		} while (Process32Next(hSnapshot, &pe32));
	EXIT:
		if(hSnapshot)
			CloseHandle(hSnapshot);
		return ProcessList;
	}

	bool CProcess::Wait(uint32_t Interval)
	{
		if (!this->ProcessName.length())
			return false;
		this->hProcess = 0;
		while (!this->ProcessList.count(ProcessName))
		{
			this->ProcessList = this->GetProcessList();
			Sleep(Interval);
		}
		return true;
	}

	bool CProcess::SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
	{
		TOKEN_PRIVILEGES priv = { 0,0,0,0 };
		HANDLE hToken = NULL;
		LUID luid = { 0,0 };
		BOOL Status = true;

		if (!OpenProcessToken(this->hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken))
		{
			Status = false;
			goto EXIT;
		}

		if (!LookupPrivilegeValueA(0, lpszPrivilege, &luid))
		{
			Status = false;
			goto EXIT;
		}

		priv.PrivilegeCount = 1;
		priv.Privileges[0].Luid = luid;
		priv.Privileges[0].Attributes = bEnablePrivilege ? SE_PRIVILEGE_ENABLED : SE_PRIVILEGE_REMOVED;

		if (!AdjustTokenPrivileges(hToken, false, &priv, 0, 0, 0))
		{
			Status = false;
			goto EXIT;
		}
	EXIT:
		if(hToken)
			CloseHandle(hToken);
		return Status;
	}

	bool CProcess::Suspend()
	{
		typedef NTSTATUS(WINAPI*_NtSuspendProcess)(HANDLE);
		static _NtSuspendProcess __NtSuspendProcess = reinterpret_cast<_NtSuspendProcess>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtSuspendProcess"));
		if (!__NtSuspendProcess)
			return false;
		__NtSuspendProcess(this->hProcess);
		return true;
	}

	bool CProcess::Resume()
	{
		typedef NTSTATUS(WINAPI*_NtResumeProcess)(HANDLE);
		static _NtResumeProcess __NtResumeProcess = reinterpret_cast<_NtResumeProcess>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtResumeProcess"));
		if (!__NtResumeProcess)
			return false;
		__NtResumeProcess(this->hProcess);
		return true;
	}

	bool CProcess::Kill()
	{
		return TerminateProcess(this->hProcess, 0);
	}

	bool CProcess::Open(DWORD dwDesiredAccess)
	{
		if (!this->ProcessName.length())
			return false;
		this->ProcessList = this->GetProcessList();
		if (this->ProcessList.count(ProcessName))
			this->hProcess = OpenProcess(dwDesiredAccess, false, this->ProcessList[ProcessName]);
		return IsValidProcess();
	}

	bool CProcess::Close()
	{
		return CloseHandle(this->hProcess);
	}

	HANDLE CProcess::GetHandle()
	{
		return this->hProcess;
	}

	DWORD CProcess::GetPid()
	{
		return GetProcessId(this->hProcess);
	}
	DWORD CProcess::GetParentPid()
	{
		ULONG_PTR pbi[6];
		ULONG ulSize = 0;
		typedef NTSTATUS(WINAPI *_NtQueryInformationProcess)(HANDLE ProcessHandle, ULONG ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
		static _NtQueryInformationProcess __NtQueryInformationProcess = reinterpret_cast<_NtQueryInformationProcess>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess"));;
		if (!__NtQueryInformationProcess)
			return 0;
		if (__NtQueryInformationProcess(this->hProcess, 0, &pbi, sizeof(pbi), &ulSize) >= 0 && ulSize == sizeof(pbi))
			return static_cast<DWORD>(pbi[5]);
		return 0;
	}

	int CProcess::Is64(PBOOL Is64)
	{
		int Status = 1;
		HANDLE hFile = INVALID_HANDLE_VALUE;
		LPVOID lpFile = 0;
		DWORD dwFileSize = 0, dwReaded = 0, dwSize = MAX_PATH;
		PIMAGE_NT_HEADERS NtHeaders = 0;
		char Path[MAX_PATH];
		if (!QueryFullProcessImageNameA(this->hProcess, 0, Path, &dwSize) ||
			!Is64)
		{
			Status = 2;
			goto EXIT;
		}
		hFile = CreateFileA(Path, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
		if (!hFile || hFile == INVALID_HANDLE_VALUE)
		{
			Status = 3;
			goto EXIT;
		}
		dwFileSize = GetFileSize(hFile, 0);
		lpFile = VirtualAlloc(0, dwFileSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!lpFile)
		{
			Status = 4;
			goto EXIT;
		};
		if (!ReadFile(hFile, lpFile, dwFileSize, &dwReaded, 0))
		{
			Status = 5;
			goto EXIT;
		}
		NtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>((reinterpret_cast<DWORD_PTR>(lpFile) + PIMAGE_DOS_HEADER(lpFile)->e_lfanew));
		if (!NtHeaders ||
			NtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			Status = 6;
			goto EXIT;
		}
		if (NtHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64 ||
			NtHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64)
		{
			*Is64 = true;
			Status = 7;
			goto EXIT;
		}
		if (NtHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
		{
			*Is64 = false;
			Status = 7;
			goto EXIT;
		}
	EXIT:
		if(hFile)
			CloseHandle(hFile);
		if(lpFile)
			VirtualFree(lpFile, dwFileSize, MEM_DECOMMIT);
		return true;
	}

	bool CProcess::IsValidProcess()
	{
		if (hProcess == INVALID_HANDLE_VALUE)
			return false;
		return (WaitForSingleObject(this->hProcess, 0) == WAIT_TIMEOUT);
	}
}
