#pragma once
#include "Includes.hpp"

namespace Process
{
	class CProcess
	{
	public:
		CProcess();
		CProcess(DWORD dwProcessId, DWORD dwDesiredAccess = PROCESS_ALL_ACCESS);
		CProcess(std::string ProcessName);
		CProcess(HANDLE hProcess);
		~CProcess();


		bool Wait(uint32_t Interval);

		bool SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);

		bool Suspend();
		bool Resume();
		bool Kill();
		bool Open(DWORD dwDesiredAccess = PROCESS_ALL_ACCESS);
		bool Close();

		MODULEENTRY32 GetModule(char* Module);

		HANDLE GetHandle();
		DWORD GetPid();
		DWORD GetParentPid();
		BOOL InjectDll(LPCSTR lpPath);
		int Is64(PBOOL Is64);
		bool IsValidProcess();

	protected:
		std::map<std::string, std::uint32_t> ProcessList;
	private:
		std::string ProcessName;
		HANDLE hProcess;
		std::map<std::string, std::uint32_t> GetProcessList();
	};
}