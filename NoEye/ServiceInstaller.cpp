#include "ServiceInstaller.hpp"
#include "Options.hpp"

namespace ServiceInstaller
{
	NTSTATUS NTAPI InstallService(LPCSTR lpServiceName, LPCSTR lpDisplayName, LPCSTR lpPath)
	{
		
		NTSTATUS result = NULL;
		SC_HANDLE hSC = NULL, hService = NULL;
		SERVICE_STATUS ss;
		LPCSTR Args[1];
		ZeroMemory(&ss, sizeof(SERVICE_STATUS));
		ZeroMemory(Args, 1);
		hSC = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
		if (!hSC)
		{
			result = 1;
			goto END;
		}
		hService = OpenServiceA(hSC, lpServiceName, SERVICE_ALL_ACCESS);
		if (hService == NULL)
		{
			hService = CreateServiceA(hSC, lpServiceName, lpDisplayName, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, lpPath, NULL, NULL, NULL, NULL, NULL);
			if (hService == NULL)
			{
				result = 2;
				goto END;
			}
		}

		if (!QueryServiceStatus(hService, &ss))
		{
			result = 3;
			goto END;
		}
		const_cast<char*>(lpPath)[(strlen(lpPath) - 3)] = 'd';
		const_cast<char*>(lpPath)[(strlen(lpPath) - 2)] = 'l';
		const_cast<char*>(lpPath)[(strlen(lpPath) - 1)] = 'l';
		Args[0] = lpPath;
		if (ss.dwCurrentState == SERVICE_STOPPED && 
			!StartServiceA(hService, 1, Args))
		{
			result = 4;
			goto END;
		}
	END:
		if (hSC)
			CloseServiceHandle(hSC);
		if (hService)
			CloseServiceHandle(hService);
		
		return result;
	}
	NTSTATUS NTAPI UninstallService(LPCSTR lpServiceName)
	{
		
		NTSTATUS result = NULL;
		SC_HANDLE hSC = NULL, hService = NULL;
		SERVICE_STATUS ss, sw;
		ZeroMemory(&ss, sizeof(SERVICE_STATUS));
		ZeroMemory(&sw, sizeof(SERVICE_STATUS));
		hSC = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
		if (!hSC)
		{
			result = 1;
			goto END;
		}
		hService = OpenServiceA(hSC, lpServiceName, SERVICE_ALL_ACCESS);
		if (hService == NULL)
			goto END;
		if (!QueryServiceStatus(hService, &ss))
		{
			result = 2;
			goto END;
		}
		if (ss.dwCurrentState == SERVICE_STOPPED)
			goto DELETE_SERVICE;
		if (!ControlService(hService, SERVICE_CONTROL_STOP, &sw))
		{
			result = 3;
			goto END;
		}
		do
		{
			if (!QueryServiceStatus(hService, &ss))
			{
				result = 2;
				goto END;
			}
		}
		while (ss.dwCurrentState != SERVICE_STOPPED);
	DELETE_SERVICE:
		if (!DeleteService(hService))
		{
			result = 4;
			goto END;
		}
	END:
		if (hSC)
			CloseServiceHandle(hSC);
		if (hService)
			CloseServiceHandle(hService);
		
		return result;
	}
	NTSTATUS NTAPI GetServiceStatus(LPCSTR lpServiceName)
	{
		
		NTSTATUS result = NULL;
		SC_HANDLE hSC = NULL, hService = NULL;
		SERVICE_STATUS ss, sw;
		ZeroMemory(&ss, sizeof(SERVICE_STATUS));
		ZeroMemory(&sw, sizeof(SERVICE_STATUS));
		hSC = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
		if (!hSC)
		{
			result = 0;
			goto END;
		}
		hService = OpenServiceA(hSC, lpServiceName, SERVICE_ALL_ACCESS);
		if (hService == NULL)
		{
			result = -1;
			goto END;
		}
		if (!QueryServiceStatus(hService, &ss))
		{
			result = 0;
			goto END;
		}
		result = ss.dwCurrentState;
	END:
		if (hSC)
			CloseServiceHandle(hSC);
		if (hService)
			CloseServiceHandle(hService);
		
		return result;
	}
}


extern "C"
{

	__declspec(dllexport) NTSTATUS ServiceInstaller_InstallService(LPCSTR lpServiceName, LPCSTR lpDisplayName, LPCSTR lpPath)
	{
		return ServiceInstaller::InstallService(lpServiceName, lpDisplayName, lpPath);
	}
	__declspec(dllexport) NTSTATUS ServiceInstaller_UninstallService(LPCSTR lpServiceName)
	{
		return ServiceInstaller::UninstallService(lpServiceName);
	}
	__declspec(dllexport) NTSTATUS ServiceInstaller_GetServiceStatus(LPCSTR lpServiceName)
	{
		return ServiceInstaller::GetServiceStatus(lpServiceName);
	}
}