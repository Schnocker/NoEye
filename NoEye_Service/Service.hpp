#pragma once

#include "Includes.hpp"
#include "CProcess.hpp"
#include "CMap.hpp"

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;
std::vector<HANDLE>   Threads;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceHandler(DWORD);
DWORD WINAPI ServiceThread(LPVOID lpParam);

#define SERVICE_NAME  ("AmdK9")  
