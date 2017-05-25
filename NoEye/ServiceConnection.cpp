#include "ServiceConnection.hpp"
#include "Options.hpp"

namespace BE
{
	namespace Kernelmode
	{
		XDriver* XDriver::Instance;
		std::vector<HANDLE> XDriver::fIOs;
		XDriver::p_NtCreateFile XDriver::o_NtCreateFile;
		XDriver::p_ZwReadFile XDriver::o_ZwReadFile;
		XDriver::p_ZwWriteFile XDriver::o_ZwWriteFile;
		DWORD XDriver::dwProcessId;

		XDriver* XDriver::GetInstance()
		{
			if (!Instance)
				Instance = new XDriver;
			return Instance;
		}
		XDriver::XDriver()
		{
			o_NtCreateFile = 0;
			o_ZwReadFile = 0;
			o_ZwWriteFile = 0;
			dwProcessId = 0;
			fIOs.clear();
		}
		XDriver::~XDriver()
		{
			if (Instance)
				delete Instance;
		}
		bool XDriver::Init()
		{
			return detour_DriverConnection(true);
		}
		bool XDriver::Uninit()
		{
			return detour_DriverConnection(false);
		}

		bool XDriver::detour_DriverConnection(bool Status)
		{

			BOOL Result = 1;
			o_NtCreateFile = o_NtCreateFile ? o_NtCreateFile : reinterpret_cast<p_NtCreateFile>(reinterpret_cast<DWORD_PTR>(GetProcAddress(GetModuleHandle("ntdll.dll"), "NtCreateFile")));
			if (!o_NtCreateFile)
				Result = 0;
			o_ZwWriteFile = o_ZwWriteFile ? o_ZwWriteFile : reinterpret_cast<p_ZwWriteFile>(reinterpret_cast<DWORD_PTR>(GetProcAddress(GetModuleHandle("ntdll.dll"), "ZwWriteFile")));
			if (!o_ZwWriteFile)
				Result = 0;
			o_ZwReadFile = o_ZwReadFile ? o_ZwReadFile : reinterpret_cast<p_ZwReadFile>(reinterpret_cast<DWORD_PTR>(GetProcAddress(GetModuleHandle("ntdll.dll"), "ZwReadFile")));
			if (!o_ZwReadFile)
				Result = 0;
			if (DetourTransactionBegin() != NO_ERROR ||
				DetourUpdateThread(GetCurrentThread()) != NO_ERROR ||
				(Status ? DetourAttach : DetourDetach)(&(PVOID&)o_NtCreateFile, NtCreateFile_Hook) != NO_ERROR ||
				(Status ? DetourAttach : DetourDetach)(&(PVOID&)o_ZwReadFile, ZwReadFile_Hook) != NO_ERROR ||
				(Status ? DetourAttach : DetourDetach)(&(PVOID&)o_ZwWriteFile, ZwWriteFile_Hook) != NO_ERROR ||
				DetourTransactionCommit() != NO_ERROR)
				Result = 0;

			return Result;
		}


		NTSTATUS NTAPI XDriver::NtCreateFile_Hook(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize, ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength)
		{

			DWORD dwWritten = 0;
			NTSTATUS Status = -1;
			if (ObjectAttributes &&
				ObjectAttributes->ObjectName &&
				ObjectAttributes->ObjectName->Buffer &&
				wcsstr(ObjectAttributes->ObjectName->Buffer, SERVICE_NAME))
			{
				Status = o_NtCreateFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
				if (NT_SUCCESS(Status))
				{
					if (FileHandle)
						fIOs.push_back(*FileHandle);
				}
				return Status;
			}

			return o_NtCreateFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
		};

		NTSTATUS NTAPI XDriver::ZwReadFile_Hook(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE  ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, ULONG Length, PLARGE_INTEGER   ByteOffset, PULONG Key)
		{

			typedef struct BATTLEYE_DATA
			{
				BYTE ID;
				BYTE Data[256];
			}BATTLEYE_DATA, *PBATTLEYE_DATA;
			PBATTLEYE_DATA Packet = reinterpret_cast<PBATTLEYE_DATA>(Buffer);
			NTSTATUS Status = o_ZwReadFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, Buffer, Length, ByteOffset, Key);
			if (NT_SUCCESS(Status))
			{
				if (Packet->ID == 3)
				{
					if (!dwProcessId)
					{
						dwProcessId = *reinterpret_cast<DWORD*>(reinterpret_cast<DWORD_PTR>(Buffer) + 1);
					}
				}
				return Status;
			}

			return Status;
		}

		NTSTATUS NTAPI XDriver::ZwWriteFile_Hook(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE  ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, ULONG Length, PLARGE_INTEGER   ByteOffset, PULONG Key)
		{

			typedef struct BATTLEYE_DATA
			{
				BYTE ID;
				BYTE Data[256];
			}BATTLEYE_DATA, *PBATTLEYE_DATA;
			_MEMORY_BASIC_INFORMATION mbi = { 0,0,0,0,0,0,0 };
			DWORD Old = 0;
			PBATTLEYE_DATA Packet = reinterpret_cast<PBATTLEYE_DATA>(Buffer);
			for (auto Handle : fIOs)
			{
				if (Handle == FileHandle &&
					Length == 4)
				{
					*reinterpret_cast<DWORD*>(Buffer) = GetCurrentProcessId();
				}
			}
			if (Length && Buffer && Packet->ID == 2)
			{
				if (*reinterpret_cast<DWORD*>(reinterpret_cast<DWORD_PTR>(Buffer) + 1) == 0)
				{
					*reinterpret_cast<DWORD*>(reinterpret_cast<DWORD_PTR>(Buffer) + 1) = dwProcessId;
				}
			}

			return o_ZwWriteFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, Buffer, Length, ByteOffset, Key);
		}
	}
}