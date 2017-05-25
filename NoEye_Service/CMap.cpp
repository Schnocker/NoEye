#include "CMap.hpp"

namespace ManualMap
{
	DWORD WINAPI UnloadFile_Thread(WDLL* wDll);
	BOOL WINAPI BuildFile_Thread(WDLL* wDll);

	BOOL LoadFileA(HANDLE hProcess, LPCSTR Path, DWORD Flags, WDLL*& wFile)
	{
		
		LPWSTR wPath = 0;
		INT Size = 0;
		BOOL Result = 0;
		Size = MultiByteToWideChar(CP_UTF8, 0, Path, -1, NULL, 0);
		if (!Size)
		{
			SetLastError(1220);
			return FALSE;
		}
		wPath = new wchar_t[Size];
		if (!wPath)
		{
			SetLastError(1221);
			return FALSE;
		}
		if (!MultiByteToWideChar(CP_UTF8, 0, Path, -1, wPath, Size))
		{
			if (wPath) delete wPath;
			SetLastError(1222);
			return FALSE;
		}
		Result = LoadFileW(hProcess, wPath, Flags, wFile);
		if (wPath) delete wPath;
		
		return Result;
	}
	BOOL LoadFileW(HANDLE hProcess, LPCWSTR Path, DWORD Flags, WDLL*& wFile)
	{
		
		HANDLE hFile = 0;
		BOOL Result = 0;
		DWORD NumberOfBytesRead = 0, FileSize = 0;
		PIMAGE_SECTION_HEADER SectionHeaders = 0;
		LPVOID lpwDll = 0, lpwThread = 0;
		HANDLE hThread = 0, FileHandle = 0;
		if (!wFile)
		{
			SetLastError(1990);
			return 0;
		}
		wFile->hProcess = hProcess;
		if (!wFile->hProcess)
		{
			SetLastError(1991);
			return 0;
		}
		hFile = CreateFileW(Path, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			SetLastError(1992);
			return 0;
		}
		FileHandle = hFile;
		FileSize = GetFileSize(FileHandle, 0);
		if (!FileSize)
		{
			CloseHandle(FileHandle);
			SetLastError(1993);
			return 0;
		}
		wFile->Module = reinterpret_cast<HMODULE>(VirtualAlloc(0, FileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
		if (!wFile->Module)
		{
			CloseHandle(FileHandle);

			SetLastError(1994);
			return 0;
		}
		if (!ReadFile(hFile, wFile->Module, static_cast<DWORD>(FileSize), &NumberOfBytesRead, 0))
		{
			VirtualFree(wFile->Module, FileSize, MEM_DECOMMIT);
			CloseHandle(FileHandle);
			SetLastError(1995);
			return 0;
		}
		if (!LoadFileFromMemory(hProcess, wFile->Module, Flags, wFile))
		{
			CloseHandle(FileHandle);
			return 0;
		}
		if (!VirtualFree(wFile->Module, FileSize, MEM_DECOMMIT))
		{
			CloseHandle(FileHandle);
			SetLastError(1996);
			return 0;
		}
		SetLastError(0);
		CloseHandle(FileHandle);
		
		return 1;
	}
	BOOL LoadFileFromResourcesA(HANDLE hProcess, LPCSTR lpName, LPCSTR ResourceType, DWORD Flags, WDLL*& wFile)
	{
		
		LPWSTR wName = 0, wResType = 0;
		INT NameSize = 0, ResTypeSize = 0;
		BOOL Result = 0;
		NameSize = MultiByteToWideChar(CP_UTF8, 0, lpName, -1, NULL, 0);
		if (!NameSize)
		{
			SetLastError(2030);
			return FALSE;
		}
		wName = new wchar_t[NameSize];
		if (!wName)
		{
			SetLastError(2032);
			return FALSE;
		}
		ResTypeSize = MultiByteToWideChar(CP_UTF8, 0, ResourceType, -1, NULL, 0);
		if (!ResTypeSize)
		{
			SetLastError(2030);
			if (wName) delete wName;
			return FALSE;
		}
		wResType = new wchar_t[ResTypeSize];
		if (!wResType)
		{
			SetLastError(2032);
			if (wName) delete wName;
			return FALSE;
		}
		if (!MultiByteToWideChar(CP_UTF8, 0, lpName, -1, wName, NameSize) ||
			!MultiByteToWideChar(CP_UTF8, 0, ResourceType, -1, wResType, ResTypeSize))
		{
			if (wName) delete wName;
			if (wResType) delete wResType;
			SetLastError(2032);
			return FALSE;
		}
		Result = LoadFileFromResourcesW(hProcess, wName, wResType, Flags, wFile);
		if (wName) delete wName;
		if (wResType) delete wResType;
		
		return Result;
	}
	BOOL LoadFileFromResourcesW(HANDLE hProcess, LPCWSTR lpName, LPCWSTR ResourceType, DWORD Flags, WDLL*& wFile)
	{
		
		HRSRC hResource = 0;
		DWORD ResourceLen = 0, FileSize = 0;
		BOOL Status = 0;
		PIMAGE_SECTION_HEADER SectionHeaders = 0;
		if (!wFile)
		{
			SetLastError(1890);
			return 0;
		}
		hResource = FindResourceW(NULL, lpName, ResourceType);
		if (!hResource)
		{
			SetLastError(1891);
			return 0;
		}
		FileSize = SizeofResource(NULL, hResource);
		wFile->Module = reinterpret_cast<HMODULE>(LoadResource(0, hResource));

		if (!FileSize || !wFile->Module || !LockResource(wFile->Module))
		{
			SetLastError(1892);

			return 0;
		}
		wFile->hProcess = hProcess;
		wFile->Flags = Flags;
		Status = LoadFileFromMemory(wFile->hProcess, wFile->Module, wFile->Flags, wFile);
		if (wFile->Module)
			FreeResource(reinterpret_cast<HGLOBAL>(wFile->Module));
		wFile->Module = 0;
		
		return Status;
	}


	BOOL LoadFileFromMemory(HANDLE hProcess, LPVOID lpDll, DWORD Flags, WDLL*& wFile)
	{
		
		PIMAGE_SECTION_HEADER SectionHeaders = 0;
		LPVOID lpwDll = 0, lpwThread = 0;
		HANDLE hThread = 0;
		BOOL Status = TRUE;
		DWORD_PTR Functions[9];
		SIZE_T Written = 0;
		ZeroMemory(Functions, 9);

		typedef NTSTATUS(NTAPI*_RtlCreateUserThread)(HANDLE Process, PSECURITY_DESCRIPTOR ThreadSecurityDescriptor, BOOLEAN CreateSuspended, ULONG_PTR ZeroBits, SIZE_T MaximumStackSize, SIZE_T CommittedStackSize, PVOID StartAddress, PVOID Parameter, PHANDLE Thread, PVOID ClientId);
		static _RtlCreateUserThread RtlCreateUserThread = reinterpret_cast<_RtlCreateUserThread>(GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlCreateUserThread"));

		if (!wFile)
		{
			SetLastError(1890);
			return 0;
		}
		wFile->Module = reinterpret_cast<HMODULE>(lpDll);
		if (!wFile->Module)
		{
			SetLastError(1790);
			return 0;
		}
		wFile->DosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(wFile->Module);
		if (wFile->DosHeader &&
			wFile->DosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			SetLastError(1792);
			return 0;
		}
		wFile->NtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>((reinterpret_cast<DWORD_PTR>(wFile->Module) + wFile->DosHeader->e_lfanew));
		if (!wFile->NtHeader ||
			wFile->NtHeader->Signature != IMAGE_NT_SIGNATURE)
		{

			SetLastError(1793);
			return 0;
		}

		if (!(wFile->NtHeader->FileHeader.Characteristics & IMAGE_FILE_DLL) &&
			!(wFile->NtHeader->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE))
		{

			SetLastError(1794);
			return 0;
		}


#if defined _M_X64
		if (wFile->NtHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 &&
			wFile->NtHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_IA64)
		{

			SetLastError(1795);
			return 0;
		}
#elif defined _M_IX86
		if (wFile->NtHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
		{
			SetLastError(1795);
			return 0;
		}
#endif

		wFile->hProcess = hProcess;
		wFile->Flags = Flags;
		wFile->ImageSize = wFile->NtHeader->OptionalHeader.SizeOfImage;
		wFile->Image = (hProcess != GetCurrentProcess()) ? VirtualAllocEx(wFile->hProcess, 0, wFile->ImageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE) : VirtualAlloc(0, wFile->ImageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!wFile->Image)
		{

			SetLastError(1796);
			return 0;
		}
		if ((hProcess != GetCurrentProcess()) ? !WriteProcessMemory(hProcess, wFile->Image, reinterpret_cast<PVOID>(wFile->Module), wFile->NtHeader->OptionalHeader.SizeOfHeaders, 0) : !memcpy(wFile->Image, wFile->Module, wFile->NtHeader->OptionalHeader.SizeOfHeaders))
		{
			VirtualFreeEx(wFile->hProcess, wFile->Image, wFile->ImageSize, MEM_DECOMMIT);

			SetLastError(2003);
			return 0;
		}

		wFile->Parameter = (wFile->hProcess != GetCurrentProcess()) ? VirtualAllocEx(wFile->hProcess, 0, sizeof(DWORD_PTR) * _countof(Functions), MEM_COMMIT, PAGE_EXECUTE_READWRITE) : VirtualAlloc(0, sizeof(DWORD_PTR) * _countof(Functions), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!wFile->Parameter)
		{
			VirtualFreeEx(wFile->hProcess, wFile->Image, wFile->ImageSize, MEM_DECOMMIT);

			SetLastError(8220);
			return 0;
		}
		Functions[6] = reinterpret_cast<DWORD_PTR>((lpDll == GetModuleHandleA("ntdll.dll") ? GetModuleHandleA("ntdll.dll") : (lpDll == GetModuleHandleA("kernel32.dll") ? GetModuleHandleA("kernel32.dll") : 0)));

		SectionHeaders = reinterpret_cast<PIMAGE_SECTION_HEADER>(wFile->NtHeader + 1);
		for (int i = 0; i < wFile->NtHeader->FileHeader.NumberOfSections; i++)
		{
			if (hProcess == GetCurrentProcess())
			{
				memcpy(reinterpret_cast<PVOID>(reinterpret_cast<DWORD_PTR>(wFile->Image) + SectionHeaders[i].VirtualAddress), reinterpret_cast<PVOID>(reinterpret_cast<DWORD_PTR>(wFile->Module) + (Functions[6] ? SectionHeaders[i].VirtualAddress : SectionHeaders[i].PointerToRawData)), Functions[6] ? SectionHeaders[i].Misc.VirtualSize : SectionHeaders[i].SizeOfRawData);
				continue;
			}
			if (!WriteProcessMemory(hProcess, reinterpret_cast<PVOID>(reinterpret_cast<DWORD_PTR>(wFile->Image) + SectionHeaders[i].VirtualAddress), reinterpret_cast<PVOID>(reinterpret_cast<DWORD_PTR>(wFile->Module) + (Functions[6] ? SectionHeaders[i].VirtualAddress : SectionHeaders[i].PointerToRawData)), Functions[6] ? SectionHeaders[i].Misc.VirtualSize : SectionHeaders[i].SizeOfRawData, 0))
			{
				VirtualFreeEx(wFile->hProcess, wFile->Image, wFile->ImageSize, MEM_DECOMMIT);

				SetLastError(2003);
				return 0;
			}
		}


		wFile->Flags = Flags;
		Functions[0] = reinterpret_cast<DWORD_PTR>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "LdrLoadDll"));
		Functions[1] = reinterpret_cast<DWORD_PTR>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "LdrGetProcedureAddress"));
		Functions[2] = reinterpret_cast<DWORD_PTR>(GetModuleHandleA("ntdll.dll"));
		Functions[3] = reinterpret_cast<DWORD_PTR>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlInitUnicodeString"));
		Functions[4] = reinterpret_cast<DWORD_PTR>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlInitAnsiString"));
		Functions[5] = reinterpret_cast<DWORD_PTR>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlAnsiStringToUnicodeString"));
		Functions[7] = reinterpret_cast<DWORD_PTR>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "LdrGetDllHandle"));
		Functions[8] = reinterpret_cast<DWORD_PTR>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlFreeUnicodeString"));
		wFile->Entrypoint = reinterpret_cast<PVOID>((reinterpret_cast<DWORD_PTR>(wFile->Image) + wFile->NtHeader->OptionalHeader.AddressOfEntryPoint));
		wFile->BaseRelocation = reinterpret_cast<PIMAGE_BASE_RELOCATION>(reinterpret_cast<DWORD_PTR>(wFile->Image) + wFile->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		wFile->ImportDes = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(reinterpret_cast<DWORD_PTR>(wFile->Image) + wFile->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		wFile->Tls = reinterpret_cast<PIMAGE_TLS_DIRECTORY>((reinterpret_cast<DWORD_PTR>(wFile->Image) + wFile->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress));
		wFile->NtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>((reinterpret_cast<DWORD_PTR>(wFile->Image) + wFile->DosHeader->e_lfanew));
		if (!Functions[0] || !Functions[1] || !Functions[2] || !Functions[3] || !Functions[4] || !Functions[5] || !wFile->BaseRelocation || !wFile->ImportDes || !wFile->NtHeader)
		{
			VirtualFreeEx(wFile->hProcess, wFile->Image, wFile->ImageSize, MEM_DECOMMIT);
			VirtualFreeEx(wFile->hProcess, wFile->Parameter, sizeof(DWORD_PTR) * _countof(Functions), MEM_DECOMMIT);

			SetLastError(2010);
			return 0;
		}

		if ((wFile->hProcess != GetCurrentProcess()) ? !WriteProcessMemory(wFile->hProcess, wFile->Parameter, Functions, sizeof(DWORD_PTR) * _countof(Functions), 0) : !memcpy(wFile->Parameter, Functions, sizeof(DWORD_PTR) * _countof(Functions)))
		{
			VirtualFreeEx(wFile->hProcess, wFile->Image, wFile->ImageSize, MEM_DECOMMIT);
			VirtualFreeEx(wFile->hProcess, wFile->Parameter, sizeof(DWORD_PTR) * _countof(Functions), MEM_DECOMMIT);

			SetLastError(2010);
			return 0;
		}
		if (hProcess != GetCurrentProcess())
		{
			lpwDll = VirtualAllocEx(wFile->hProcess, 0, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			if (!lpwDll)
			{
				VirtualFreeEx(wFile->hProcess, wFile->Image, wFile->ImageSize, MEM_DECOMMIT);
				VirtualFreeEx(wFile->hProcess, wFile->Parameter, sizeof(DWORD_PTR) * _countof(Functions), MEM_DECOMMIT);

				SetLastError(2004);
				return 0;
			}

			if (!WriteProcessMemory(wFile->hProcess, lpwDll, wFile, sizeof(WDLL), &Written))
			{
				VirtualFreeEx(wFile->hProcess, wFile->Image, wFile->ImageSize, MEM_DECOMMIT);
				VirtualFreeEx(wFile->hProcess, wFile->Parameter, sizeof(DWORD_PTR) * _countof(Functions), MEM_DECOMMIT);
				VirtualFreeEx(wFile->hProcess, lpwDll, 0x1000, MEM_DECOMMIT);

				SetLastError(2005);
				return 0;
			}
			lpwThread = VirtualAllocEx(wFile->hProcess, 0, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			if (!lpwThread)
			{
				VirtualFreeEx(wFile->hProcess, wFile->Image, wFile->ImageSize, MEM_DECOMMIT);
				VirtualFreeEx(wFile->hProcess, wFile->Parameter, sizeof(DWORD_PTR) * _countof(Functions), MEM_DECOMMIT);
				VirtualFreeEx(wFile->hProcess, lpwDll, 0x1000, MEM_DECOMMIT);

				SetLastError(2006);
				return 0;
			}

			if (!WriteProcessMemory(wFile->hProcess, lpwThread, BuildFile_Thread, 0x1000, &Written))
			{
				VirtualFreeEx(wFile->hProcess, wFile->Image, wFile->ImageSize, MEM_DECOMMIT);
				VirtualFreeEx(wFile->hProcess, wFile->Parameter, sizeof(DWORD_PTR) * _countof(Functions), MEM_DECOMMIT);
				VirtualFreeEx(wFile->hProcess, lpwDll, 0x1000, MEM_DECOMMIT);
				VirtualFreeEx(wFile->hProcess, lpwThread, 0x1000, MEM_DECOMMIT);

				SetLastError(2007);
				return 0;
			}
			if ((RtlCreateUserThread(wFile->hProcess, 0, 0, 0, 0, 0, reinterpret_cast<PVOID>(lpwThread), lpwDll, &hThread, 0)))
			{
				VirtualFreeEx(wFile->hProcess, wFile->Image, wFile->ImageSize, MEM_DECOMMIT);
				VirtualFreeEx(wFile->hProcess, wFile->Parameter, sizeof(DWORD_PTR) * _countof(Functions), MEM_DECOMMIT);
				VirtualFreeEx(wFile->hProcess, lpwDll, 0x1000, MEM_DECOMMIT);
				VirtualFreeEx(wFile->hProcess, lpwThread, 0x1000, MEM_DECOMMIT);

				SetLastError(2008);
				return 0;
			}
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
			if (!VirtualFreeEx(wFile->hProcess, lpwDll, 0x1000, MEM_DECOMMIT) ||
				!VirtualFreeEx(wFile->hProcess, lpwThread, 0x1000, MEM_DECOMMIT))
			{

				SetLastError(2009);
				return 0;
			}
			return 1;
		}
		Status = BuildFile_Thread(wFile) ? true : false;
		
		return Status;
	}
	FARPROC GetProcAddressA(WDLL* wDll, LPCSTR lpName)
	{
		
		LPWSTR wName = 0;
		INT NameSize = 0;
		FARPROC Function = 0;
		NameSize = MultiByteToWideChar(CP_UTF8, 0, lpName, -1, NULL, 0);
		if (!NameSize)
		{
			SetLastError(4031);
			return FALSE;
		}
		wName = new wchar_t[NameSize];
		if (!wName)
		{
			SetLastError(4032);
			return FALSE;
		}
		if (!MultiByteToWideChar(CP_UTF8, 0, lpName, -1, wName, NameSize))
		{
			SetLastError(4033);
			if (wName) delete wName;
			return FALSE;
		}
		Function = GetProcAddressW(wDll, wName);
		if (wName) delete wName;
		
		return Function;
	}
	FARPROC GetProcAddressW(WDLL* wDll, LPCWSTR lpName)
	{
		
		DWORD Oridinal = reinterpret_cast<DWORD>(lpName), OrdSize = 0;
		size_t lpNameSize = 0;
		LPSTR lpOrdName = "";
		PIMAGE_EXPORT_DIRECTORY ImageDirectory = 0;
		PIMAGE_NT_HEADERS64 Nt64Header = 0;
		if (!wDll || !lpName)
		{
			SetLastError(4001);
			return FALSE;
		}
		if (!wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress ||
			!wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)
		{
			SetLastError(4002);
			return FALSE;
		}
		switch (wDll->NtHeader->OptionalHeader.Magic)
		{
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
			ImageDirectory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(reinterpret_cast<DWORD_PTR>(wDll->Image) + wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
			break;
		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
			ImageDirectory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(reinterpret_cast<DWORD_PTR>(wDll->Image) + wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
			break;
		default:
			SetLastError(3999);
			return FALSE;
			break;
		}

		if (!ImageDirectory)
		{
			SetLastError(4003);
			return FALSE;
		}

		if (Oridinal < 0x10000)
		{
			if (Oridinal < ImageDirectory->Base)
			{
				SetLastError(4004);
				return FALSE;
			}
			Oridinal -= ImageDirectory->Base;
		}

		if ((lpNameSize = wcslen(lpName)))
		{
			OrdSize = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, lpName, -1, 0, 0, 0, 0);
			if (!OrdSize)
			{
				SetLastError(4005);
				return FALSE;
			}
			lpOrdName = new char[OrdSize];
			if (!lpOrdName)
			{
				SetLastError(4006);
				return FALSE;
			}
			if (!WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, lpName, -1, lpOrdName, OrdSize, 0, 0))
			{
				SetLastError(4007);
				if (lpOrdName) delete lpOrdName;
				return FALSE;
			}
			for (int i = 0; i < ImageDirectory->NumberOfNames; ++i)
			{
				if (!_stricmp(reinterpret_cast<char*>(((reinterpret_cast<DWORD*>(ImageDirectory->AddressOfNames + reinterpret_cast<DWORD_PTR>(wDll->Image)))[i] + reinterpret_cast<DWORD_PTR>(wDll->Image))), lpOrdName))
				{
					Oridinal = reinterpret_cast<WORD*>((ImageDirectory->AddressOfNameOrdinals + reinterpret_cast<DWORD_PTR>(wDll->Image)))[i];
					break;
				}
			}
		}

		if (Oridinal >= ImageDirectory->NumberOfFunctions)
		{
			SetLastError(4000);
			if (lpOrdName) delete lpOrdName;
			return FALSE;
		}
		
		return reinterpret_cast<FARPROC>((reinterpret_cast<DWORD*>((ImageDirectory->AddressOfFunctions + reinterpret_cast<DWORD_PTR>(wDll->Image)))[Oridinal] + reinterpret_cast<DWORD_PTR>(wDll->Image)));
	}
	BOOL UnloadFile(WDLL* wDll)
	{
		

		typedef NTSTATUS(NTAPI*_RtlCreateUserThread)(HANDLE Process, PSECURITY_DESCRIPTOR ThreadSecurityDescriptor, BOOLEAN CreateSuspended, ULONG_PTR ZeroBits, SIZE_T MaximumStackSize, SIZE_T CommittedStackSize, PVOID StartAddress, PVOID Parameter, PHANDLE Thread, PVOID ClientId);
		static _RtlCreateUserThread RtlCreateUserThread = reinterpret_cast<_RtlCreateUserThread>(GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlCreateUserThread"));

		BOOL Result = TRUE;
		LPVOID lpUnloadThread = 0, lpwDll = 0;
		HANDLE hThread = 0;



		if (!(wDll->Flags & ManualMap_FLAGS::CALL_NO_ENTRYPOINT))
		{
			if (wDll->Entrypoint)
			{
				lpwDll = VirtualAllocEx(wDll->hProcess, 0, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
				if (!lpwDll)
				{
					SetLastError(3000);
					Result = FALSE;
				}
				if (!WriteProcessMemory(wDll->hProcess, lpwDll, wDll, 0x1000, 0))
				{
					SetLastError(3000);
					Result = FALSE;
				}
				lpUnloadThread = VirtualAllocEx(wDll->hProcess, 0, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
				if (!lpUnloadThread)
				{
					SetLastError(3000);
					Result = FALSE;
				}
				if (!WriteProcessMemory(wDll->hProcess, lpUnloadThread, UnloadFile_Thread, 0x1000, 0))
				{
					SetLastError(3000);
					Result = FALSE;
				}
				if (RtlCreateUserThread(wDll->hProcess, 0, 0, 0, 0, 0, reinterpret_cast<PVOID>(lpUnloadThread), lpwDll, &hThread, 0))
				{
					SetLastError(3000);
					Result = FALSE;
				}

				if (hThread)
					WaitForSingleObject(hThread, INFINITE);
				if (lpUnloadThread &&
					!VirtualFreeEx(wDll->hProcess, lpUnloadThread, 0x1000, MEM_DECOMMIT) ||
					lpwDll &&
					!VirtualFreeEx(wDll->hProcess, lpwDll, 0x1000, MEM_DECOMMIT))
				{
					SetLastError(3000);
					Result = FALSE;
				}
			}
		}
		if (wDll->Image)
		{
			if (!VirtualFreeEx(wDll->hProcess, wDll->Image, wDll->ImageSize, MEM_DECOMMIT))
			{
				SetLastError(3002);
				return FALSE;
			}
		}
		
		return Result;
	}
	BOOL WINAPI BuildFile_Thread(WDLL* wDll)
	{
		typedef struct _STRING {
			USHORT Length;
			USHORT MaximumLength;
			PCHAR  Buffer;
		} ANSI_STRING, *PANSI_STRING;
		typedef struct _LSA_UNICODE_STRING {
			USHORT Length;
			USHORT MaximumLength;
			PWSTR  Buffer;
		} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

		PIMAGE_DOS_HEADER DosHeader = 0;
		PIMAGE_NT_HEADERS NtHeader = 0;
		PIMAGE_BASE_RELOCATION BaseRelocationStart = 0, BaseRelocationEnd = 0, xBaseRelocationStart = 0, xBaseRelocationEnd = 0;
		PIMAGE_IMPORT_DESCRIPTOR ImportDes = 0;
		PIMAGE_IMPORT_BY_NAME ImportName = 0;
		PIMAGE_TLS_CALLBACK* TlsCallBack = 0;
		PIMAGE_EXPORT_DIRECTORY Exports = 0;
		PIMAGE_THUNK_DATA FirstThunk = 0, OriginalThunk = 0;
		DWORD_PTR Delta = 0,
			*PTR = 0, Count = 0, Counter = 0, Function = 0, *Functions = 0, EntryPoint = 0, Oridinal = 0;
		WORD* List = 0;

		HMODULE Module = 0, NtDll = 0;
		BOOL Status = TRUE;
		UNICODE_STRING DllName;
		ANSI_STRING FunctionName;

		if (!wDll)
			return FALSE;

		Delta = (reinterpret_cast<DWORD_PTR>(wDll->Image) - wDll->NtHeader->OptionalHeader.ImageBase);
		BaseRelocationStart = wDll->BaseRelocation;
		ImportDes = wDll->ImportDes;
		Functions = reinterpret_cast<DWORD_PTR*>(wDll->Parameter);

		if (!Delta || !BaseRelocationStart || !ImportDes || !Functions)
			return FALSE;

		if (Functions[6])
		{
			DosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(Functions[2]);
			if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE)
				return FALSE;
			NtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(Functions[2] + DosHeader->e_lfanew);
			if (NtHeader->Signature != IMAGE_NT_SIGNATURE)
				return FALSE;
		}
		while (true)
		{
			if (*reinterpret_cast<BYTE*>(Functions[2]) == 0x4D &&
				*reinterpret_cast<BYTE*>(Functions[2] + 1) == 0x5A &&
				*reinterpret_cast<BYTE*>(Functions[2] + 2) == 0x90)
				break;
		}

		if (wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress &&
			wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
		{
			BaseRelocationStart = reinterpret_cast<PIMAGE_BASE_RELOCATION>((reinterpret_cast<DWORD_PTR>(wDll->Image) + wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress));
			BaseRelocationEnd = reinterpret_cast<PIMAGE_BASE_RELOCATION>((reinterpret_cast<DWORD_PTR>(BaseRelocationStart) + wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size - sizeof(IMAGE_BASE_RELOCATION)));
			if (Functions[6])
			{
				xBaseRelocationStart = reinterpret_cast<PIMAGE_BASE_RELOCATION>((Functions[2] + NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress));
				xBaseRelocationEnd = reinterpret_cast<PIMAGE_BASE_RELOCATION>((reinterpret_cast<DWORD_PTR>(BaseRelocationStart) + NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size - sizeof(IMAGE_BASE_RELOCATION)));
			}
			for (; BaseRelocationStart->VirtualAddress; BaseRelocationStart = reinterpret_cast<PIMAGE_BASE_RELOCATION>((reinterpret_cast<LPBYTE>(BaseRelocationStart) + BaseRelocationStart->SizeOfBlock)))
			{
				if (BaseRelocationStart->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION))
				{
					Count = (BaseRelocationStart->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
					List = reinterpret_cast<PWORD>(BaseRelocationStart + 1);

					for (Counter = 0; Counter < Count; Counter++)
					{
						if ((List[Counter]))
							*reinterpret_cast<DWORD_PTR*>((reinterpret_cast<LPBYTE>(wDll->Image) + (BaseRelocationStart->VirtualAddress + (List[Counter] & 0xFFF)))) = Functions[6] ? *reinterpret_cast<DWORD_PTR*>((reinterpret_cast<LPBYTE>(Functions[2]) + (xBaseRelocationStart->VirtualAddress + (List[Counter] & 0xFFF)))) : *reinterpret_cast<DWORD_PTR*>((reinterpret_cast<LPBYTE>(wDll->Image) + (BaseRelocationStart->VirtualAddress + (List[Counter] & 0xFFF)))) + Delta;
					}
				}
				if (Functions[6])
					xBaseRelocationStart = reinterpret_cast<PIMAGE_BASE_RELOCATION>((reinterpret_cast<LPBYTE>(xBaseRelocationStart) + xBaseRelocationStart->SizeOfBlock));
			}
		}

		if (wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress &&
			wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
		{
			for (; ImportDes->Characteristics; ImportDes++)
			{
				OriginalThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(reinterpret_cast<DWORD_PTR>(wDll->Image) + ImportDes->OriginalFirstThunk);
				FirstThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(reinterpret_cast<DWORD_PTR>(wDll->Image) + ImportDes->FirstThunk);

				Module = 0;
				reinterpret_cast<VOID(NTAPI*)(PANSI_STRING, LPCSTR)>(Functions[3])(&FunctionName, reinterpret_cast<LPCSTR>(reinterpret_cast<DWORD_PTR>(wDll->Image) + ImportDes->Name));
				if (reinterpret_cast<NTSTATUS(NTAPI*)(PUNICODE_STRING, PANSI_STRING, BOOLEAN)>(Functions[5])(&DllName, &FunctionName, true) != 0)
					return FALSE;
				reinterpret_cast<NTSTATUS(NTAPI*)(PUNICODE_STRING, PWCHAR)>(Functions[3])(&DllName, DllName.Buffer);
				if (reinterpret_cast<NTSTATUS(NTAPI*)(PWCHAR, PULONG, PUNICODE_STRING, PVOID*)>(Functions[0])(0, 0, &DllName, reinterpret_cast<PVOID*>(&Module)) != 0)
					reinterpret_cast<NTSTATUS(NTAPI*)(PWCHAR, PULONG, PUNICODE_STRING, PVOID*)>(Functions[7])(0, 0, &DllName, reinterpret_cast<PVOID*>(&Module));
				if (!Module)
					return FALSE;

				while (OriginalThunk->u1.AddressOfData)
				{
					if (OriginalThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
					{
						if (reinterpret_cast<NTSTATUS(WINAPI*)(HMODULE, PANSI_STRING, WORD, DWORD_PTR*)>(Functions[1])(Module, 0, OriginalThunk->u1.Ordinal & 0xFFFF, &Function) != 0)
							return FALSE;
						FirstThunk->u1.Function = Function;
						OriginalThunk++;
						FirstThunk++;
						continue;
					}
					ImportName = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(((reinterpret_cast<DWORD_PTR>(wDll->Image) + OriginalThunk->u1.AddressOfData)));
					reinterpret_cast<VOID(NTAPI*)(PANSI_STRING, LPCSTR)>(Functions[3])(&FunctionName, reinterpret_cast<LPCSTR>(ImportName->Name));
					if (reinterpret_cast<NTSTATUS(WINAPI*)(HMODULE, PANSI_STRING, WORD, DWORD_PTR*)> (Functions[1])(Module, &FunctionName, 0, &Function) != 0)
						return FALSE;
					FirstThunk->u1.Function = Function;
					OriginalThunk++;
					FirstThunk++;
				}

				reinterpret_cast<VOID(NTAPI*)(PUNICODE_STRING)>(Functions[8])(&DllName);
			}

		}

		if (wDll->Tls &&
			wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
		{
			TlsCallBack = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(wDll->Tls->AddressOfCallBacks);
			if (TlsCallBack)
			{
				while (*TlsCallBack)
				{
					Status = reinterpret_cast<BOOL(WINAPI*)(DWORD_PTR, DWORD, LPVOID)>(*TlsCallBack)(reinterpret_cast<DWORD_PTR>(wDll->Image), DLL_PROCESS_ATTACH, NULL);
					TlsCallBack++;
				}
			}
		}

		if (wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress &&
			wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size &&
			(wDll->Flags & ManualMap_FLAGS::CALL_EXPORT) == ManualMap_FLAGS::CALL_EXPORT)
		{
			Exports = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(reinterpret_cast<DWORD_PTR>(wDll->Image) + wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
			Oridinal = 0;
			if (Oridinal + 1 > Exports->NumberOfFunctions)
				return FALSE;
			wDll->Entrypoint = reinterpret_cast<FARPROC>((reinterpret_cast<DWORD*>((Exports->AddressOfFunctions + reinterpret_cast<DWORD_PTR>(wDll->Image)))[Oridinal] + reinterpret_cast<DWORD_PTR>(wDll->Image)));
			if (wDll->Entrypoint)
			{
				Status = reinterpret_cast<BOOL(WINAPI*)(DWORD_PTR, DWORD, LPVOID)>(wDll->Entrypoint)(reinterpret_cast<DWORD_PTR>(wDll->Image), DLL_PROCESS_ATTACH, NULL);
			}
		}

		if (wDll->NtHeader->OptionalHeader.AddressOfEntryPoint &&
			!(wDll->Flags & ManualMap_FLAGS::CALL_NO_ENTRYPOINT))
		{
			if (wDll->Entrypoint)
			{
				if ((wDll->NtHeader->FileHeader.Characteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL)
					Status = reinterpret_cast<BOOL(WINAPI*)(DWORD_PTR, DWORD, LPVOID)>(wDll->Entrypoint)(reinterpret_cast<DWORD_PTR>(wDll->Image), DLL_PROCESS_ATTACH, NULL);
				//if ((wDll->NtHeader->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) == IMAGE_FILE_EXECUTABLE_IMAGE)
				//	Status = reinterpret_cast<INT(*)()>(wDll->Entrypoint)();
			}
		}

		if ((wDll->Flags & ManualMap_FLAGS::HIDE_PE) && NtDll)
		{
			for (int i = 0; i < 0x1000; i++)
				reinterpret_cast<BYTE*>(wDll->Image)[i] = reinterpret_cast<BYTE*>(NtDll)[i];
		}
		return Status;
	};
	DWORD WINAPI UnloadFile_Thread(WDLL* wDll)
	{
		PIMAGE_EXPORT_DIRECTORY Exports = 0;
		PIMAGE_TLS_CALLBACK* TlsCallBack;
		DWORD Oridinal = 0, Status = 0;
		if (wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress &&
			wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size &&
			(wDll->Flags & ManualMap_FLAGS::CALL_EXPORT))
		{
			Exports = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(reinterpret_cast<DWORD_PTR>(wDll->Image) + wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
			Oridinal = 0;
			if (Oridinal > Exports->NumberOfFunctions)
				return FALSE;
			wDll->Entrypoint = reinterpret_cast<FARPROC>((reinterpret_cast<DWORD*>((Exports->AddressOfFunctions + reinterpret_cast<DWORD_PTR>(wDll->Image)))[Oridinal] + reinterpret_cast<DWORD_PTR>(wDll->Image)));
			if (wDll->Entrypoint)
			{
				Status = reinterpret_cast<BOOL(WINAPI*)(DWORD_PTR, DWORD, LPVOID)>(wDll->Entrypoint)(reinterpret_cast<DWORD_PTR>(wDll->Image), DLL_PROCESS_DETACH, NULL);
			}
		}
		if (wDll->Tls &&
			wDll->NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
		{
			TlsCallBack = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(wDll->Tls->AddressOfCallBacks);
			if (TlsCallBack)
			{
				while (*TlsCallBack)
				{
					Status = reinterpret_cast<BOOL(WINAPI*)(DWORD_PTR, DWORD, LPVOID)>(*TlsCallBack)(reinterpret_cast<DWORD_PTR>(wDll->Image), DLL_PROCESS_DETACH, NULL);
					TlsCallBack++;
				}
			}
		}

		wDll->Entrypoint = reinterpret_cast<PVOID>((reinterpret_cast<DWORD_PTR>(wDll->Image) + wDll->NtHeader->OptionalHeader.AddressOfEntryPoint));
		if (wDll->NtHeader->OptionalHeader.AddressOfEntryPoint && !(wDll->Flags & ManualMap_FLAGS::CALL_EXPORT))
		{
			if ((wDll->NtHeader->FileHeader.Characteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL)
				Status = reinterpret_cast<BOOL(WINAPI*)(DWORD_PTR, DWORD, LPVOID)>(wDll->Entrypoint)(reinterpret_cast<DWORD_PTR>(wDll->Image), DLL_PROCESS_DETACH, NULL);
		}
		return Status;

	}
}