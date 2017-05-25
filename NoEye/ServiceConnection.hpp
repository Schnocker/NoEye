#pragma once
#include "Includes.hpp"

namespace  BE
{
	namespace Kernelmode
	{
		class XDriver
		{
		public:
			XDriver();
			~XDriver();
			static XDriver* GetInstance();
			bool Init(), Uninit();
		private:
			bool detour_DriverConnection(bool Status);
			static NTSTATUS NTAPI ZwWriteFile_Hook(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE  ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, ULONG Length, PLARGE_INTEGER ByteOffset, PULONG Key);
			static NTSTATUS NTAPI ZwReadFile_Hook(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE  ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, ULONG Length, PLARGE_INTEGER ByteOffset, PULONG Key);
			static NTSTATUS NTAPI NtCreateFile_Hook(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize, ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength);
		protected:
			typedef NTSTATUS(NTAPI*p_ZwWriteFile)(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE  ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, ULONG Length, PLARGE_INTEGER ByteOffset, PULONG Key);
			typedef NTSTATUS(NTAPI*p_ZwReadFile)(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, ULONG Length, PLARGE_INTEGER  ByteOffset, PULONG Key);
			typedef NTSTATUS(NTAPI*p_NtCreateFile)(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize, ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength);

			static p_NtCreateFile o_NtCreateFile;
			static p_ZwReadFile o_ZwReadFile;
			static p_ZwWriteFile o_ZwWriteFile;

			static std::vector<HANDLE> fIOs;
			static XDriver* Instance;
			static DWORD dwProcessId;
		};
	}
}