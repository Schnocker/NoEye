#include "ServiceInstaller.hpp"
#include "ServiceConnection.hpp"
#include "Options.hpp"

namespace CMain
{
	BOOL OnAttach(HMODULE hDll)
	{
		
		std::array<char, MAX_PATH> szError;
		BOOL Status = false;
		if (GetModuleHandleA("BEService.exe"))
		{
			Status = BE::Kernelmode::XDriver::GetInstance()->Init();
			if (Status != TRUE)
			{
				sprintf_s(szError.data(), szError.size(), "Failed to initialize BattlEye Bypass (errorcode : %i)", GetLastError());
				OutputDebugStringA(szError.data());
			}
		}
		
		return Status;
	}
	BOOL OnDetach()
	{
		
		std::array<char, MAX_PATH> szError;
		BOOL Status = false;
		if (GetModuleHandleA("BEService.exe"))
		{
			Status = BE::Kernelmode::XDriver::GetInstance()->Uninit();
			if (Status != TRUE)
			{
				sprintf_s(szError.data(), szError.size(), "Failed to uninitialize BattlEye Bypass (errorcode : %i)", GetLastError());
				OutputDebugStringA(szError.data());
			}
		}
		
		return Status;
	}
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
		CMain::OnAttach(hModule);
	if (dwReason == DLL_PROCESS_DETACH)
		CMain::OnDetach();
	return TRUE;
}