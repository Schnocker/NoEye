#pragma once

#include "Includes.hpp"

namespace ServiceInstaller
{
	NTSTATUS NTAPI InstallService(LPCSTR lpServiceName, LPCSTR lpDisplayName, LPCSTR lpPath);
	NTSTATUS NTAPI UninstallService(LPCSTR lpServiceName);
	NTSTATUS NTAPI GetServiceStatus(LPCSTR lpServiceName);
}

extern "C"
{

	__declspec(dllexport) NTSTATUS ServiceInstaller_InstallService(LPCSTR lpServiceName, LPCSTR lpDisplayName, LPCSTR lpPath);
	__declspec(dllexport) NTSTATUS ServiceInstaller_UninstallService(LPCSTR lpServiceName);
	__declspec(dllexport) NTSTATUS ServiceInstaller_GetServiceStatus(LPCSTR lpServiceName);
}