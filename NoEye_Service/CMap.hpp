#pragma once
#include "Includes.hpp"

namespace ManualMap
{

	struct WDLL
	{
		HANDLE hProcess;
		DWORD_PTR ImageSize;
		HMODULE Module;
		PVOID Image, Entrypoint;
		PIMAGE_DOS_HEADER DosHeader;
		PIMAGE_NT_HEADERS NtHeader;
		PIMAGE_BASE_RELOCATION BaseRelocation;
		PIMAGE_IMPORT_DESCRIPTOR ImportDes;
		PIMAGE_TLS_DIRECTORY Tls;
		DWORD Flags;
		LPVOID Parameter;

		WDLL() : hProcess(0), ImageSize(0), Module(0), Image(0), Entrypoint(0), DosHeader(0), NtHeader(0), BaseRelocation(0), ImportDes(0), Tls(0),
			Flags(0), Parameter(0)
		{
		}
		~WDLL()
		{
			if (Image)
				VirtualFree(Image, ImageSize, MEM_RELEASE);
		}
	};


	enum ManualMap_FLAGS
	{
		CALL_EXPORT = 1,
		HIDE_PE = 2,
		CALL_NO_ENTRYPOINT = 0x100,
		NO_FLAGS = 0
	};

	BOOL LoadFileA(HANDLE hProcess, LPCSTR Path, DWORD Flags, WDLL*& wFile);
	BOOL LoadFileW(HANDLE hProcess, LPCWSTR Path, DWORD Flags, WDLL*& wFile);
	BOOL LoadFileFromResourcesA(HANDLE hProcess, LPCSTR Name, LPCSTR ResourceType, DWORD Flags, WDLL*& wFile);
	BOOL LoadFileFromResourcesW(HANDLE hProcess, LPCWSTR lpName, LPCWSTR ResourceType, DWORD Flags, WDLL*& wFile);
	BOOL LoadFileFromMemory(HANDLE hProcess, LPVOID lpDll, DWORD Flags, WDLL*& wFile);
	FARPROC GetProcAddressA(WDLL* wDll, LPCSTR lpName);
	FARPROC GetProcAddressW(WDLL* wDll, LPCWSTR lpName);
	BOOL UnloadFile(WDLL* wDll);
}