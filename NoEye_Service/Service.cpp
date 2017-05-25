#include "Service.hpp"


int main(int argc, TCHAR *argv[])
{
	
	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{ SERVICE_NAME, reinterpret_cast<LPSERVICE_MAIN_FUNCTION>(ServiceMain) },
		{ NULL, NULL }
	};
	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
		return GetLastError();
	
	return 0;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	
	DWORD Status = E_FAIL;
	HANDLE hThread = INVALID_HANDLE_VALUE;
	if (argc == 1 || argc == 0)
	{
		OutputDebugString("AmdK9: No argument");
		return;
	}
	g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceHandler);

	if (g_StatusHandle == NULL)
	{
		OutputDebugString("AmdK9: RegisterServiceCtrlHandler failed");
		return;
	}

	ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;
	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		OutputDebugString(("AmdK9: SetServiceStatus returned error"));

	g_ServiceStopEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL)
	{
		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString("AmdK9: SetServiceStatus returned error");
		}
		return;
	}

	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		OutputDebugString("AmdK9: SetServiceStatus returned error");

	hThread = CreateThread(NULL, 0, ServiceThread, argv[argc - 1], 0, NULL);
	Threads.push_back(hThread);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(g_ServiceStopEvent);

	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		OutputDebugString("AmdK9: SetServiceStatus returned error");
	
	return;
}


VOID WINAPI ServiceHandler(DWORD CtrlCode)
{
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:
		
		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString("AmdK9: SetServiceStatus returned error");
		}
		SetEvent(g_ServiceStopEvent);
		CloseHandle(g_ServiceStopEvent);
		for (auto Thread : Threads)
		{
			TerminateThread(Thread, CtrlCode);
			CloseHandle(Thread);
		}
		
		break;
	default:
		break;
	}

}

DWORD WINAPI ServiceThread(LPVOID lpParam)
{
	
	ManualMap::WDLL* WDll = 0;
	Process::CProcess Service(std::string("BEService.exe"));
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		Service.Wait(10);
		Service.Open(PROCESS_ALL_ACCESS);
		WDll = new ManualMap::WDLL;
		if (!ManualMap::LoadFileA(Service.GetHandle(), reinterpret_cast<LPCSTR>(lpParam), 0, WDll))
		{
			OutputDebugString("AmdK9 : LoadFileA failed.");
			if (WDll)
				delete WDll;
			Service.Kill();
			Service.Close();
			continue;
		}
		if(WDll)
			delete WDll;
		WaitForSingleObject(Service.GetHandle(), INFINITE);
		Service.Close();
	}
	
	return ERROR_SUCCESS;
}

