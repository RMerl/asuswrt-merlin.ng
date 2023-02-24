/*
 * win32_io.c - A stdio-like disk I/O implementation for low-level disk access
 *		on Win32.  Can access an NTFS volume while it is mounted.
 *		Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2003-2004 Lode Leroy
 * Copyright (c) 2003-2006 Anton Altaparmakov
 * Copyright (c) 2004-2005 Yuval Fledel
 * Copyright (c) 2012-2014 Jean-Pierre Andre
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the NTFS-3G
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#ifdef HAVE_WINDOWS_H
#define BOOL WINBOOL /* avoid conflicting definitions of BOOL */
#include <windows.h>
#undef BOOL
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/*
 *		Definitions needed for <winioctl.h>
 */
#ifndef _ANONYMOUS_UNION
#define _ANONYMOUS_UNION
#define _ANONYMOUS_STRUCT
typedef unsigned long long DWORD64;
#endif

typedef struct {
        DWORD data1;     /* The first eight hexadecimal digits of the GUID. */
        WORD data2;     /* The first group of four hexadecimal digits. */
        WORD data3;     /* The second group of four hexadecimal digits. */ 
        char data4[8];    /* The first two bytes are the third group of four
                           hexadecimal digits. The remaining six bytes are the
                           final 12 hexadecimal digits. */
} GUID;

#include <winioctl.h>

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#define stat stat64
#define st_blocks  st_rdev /* emulate st_blocks, missing in Windows */
#endif

/* Prevent volume.h from being be loaded, as it conflicts with winnt.h. */
#define _NTFS_VOLUME_H
struct ntfs_volume;
typedef struct ntfs_volume ntfs_volume;

#include "debug.h"
#include "types.h"
#include "device.h"
#include "misc.h"

#define cpu_to_le16(x) (x)
#define const_cpu_to_le16(x) (x)

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

#ifndef NTFS_BLOCK_SIZE
#define NTFS_BLOCK_SIZE		512
#define NTFS_BLOCK_SIZE_BITS	9
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

#ifndef IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS
#define IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS 5636096
#endif

#ifndef IOCTL_DISK_GET_DRIVE_GEOMETRY
#define IOCTL_DISK_GET_DRIVE_GEOMETRY 0x70000
#endif

#ifndef IOCTL_GET_DISK_LENGTH_INFO
#define IOCTL_GET_DISK_LENGTH_INFO 0x7405c
#endif

#ifndef FSCTL_ALLOW_EXTENDED_DASD_IO
#define FSCTL_ALLOW_EXTENDED_DASD_IO 0x90083
#endif

/* Windows 2k+ imports. */
typedef HANDLE (WINAPI *LPFN_FINDFIRSTVOLUME)(LPTSTR, DWORD);
typedef BOOL (WINAPI *LPFN_FINDNEXTVOLUME)(HANDLE, LPTSTR, DWORD);
typedef BOOL (WINAPI *LPFN_FINDVOLUMECLOSE)(HANDLE);
typedef BOOL (WINAPI *LPFN_SETFILEPOINTEREX)(HANDLE, LARGE_INTEGER,
		PLARGE_INTEGER, DWORD);

static LPFN_FINDFIRSTVOLUME fnFindFirstVolume = NULL;
static LPFN_FINDNEXTVOLUME fnFindNextVolume = NULL;
static LPFN_FINDVOLUMECLOSE fnFindVolumeClose = NULL;
static LPFN_SETFILEPOINTEREX fnSetFilePointerEx = NULL;

#ifdef UNICODE
#define FNPOSTFIX "W"
#else
#define FNPOSTFIX "A"
#endif

enum { /* see http://msdn.microsoft.com/en-us/library/cc704588(v=prot.10).aspx */
   STATUS_UNKNOWN = -1,
   STATUS_SUCCESS =              0x00000000,
   STATUS_BUFFER_OVERFLOW =      0x80000005,
   STATUS_INVALID_HANDLE =       0xC0000008,
   STATUS_INVALID_PARAMETER =    0xC000000D,
   STATUS_INVALID_DEVICE_REQUEST = 0xC0000010,
   STATUS_END_OF_FILE =          0xC0000011,
   STATUS_CONFLICTING_ADDRESSES = 0xC0000018,
   STATUS_NO_MATCH =             0xC000001E,
   STATUS_ACCESS_DENIED =        0xC0000022,
   STATUS_BUFFER_TOO_SMALL =     0xC0000023,
   STATUS_OBJECT_TYPE_MISMATCH = 0xC0000024,
   STATUS_FILE_NOT_FOUND =       0xC0000028,
   STATUS_OBJECT_NAME_INVALID =  0xC0000033,
   STATUS_OBJECT_NAME_NOT_FOUND = 0xC0000034,
   STATUS_SHARING_VIOLATION =    0xC0000043,
   STATUS_INVALID_PARAMETER_1 =  0xC00000EF,
   STATUS_IO_DEVICE_ERROR =      0xC0000185,
   STATUS_GUARD_PAGE_VIOLATION = 0x80000001
 } ;

typedef u32 NTSTATUS; /* do not let the compiler choose the size */
#ifdef __x86_64__
typedef unsigned long long ULONG_PTR; /* an integer the same size as a pointer */
#else
typedef unsigned long ULONG_PTR; /* an integer the same size as a pointer */
#endif

HANDLE get_osfhandle(int); /* from msvcrt.dll */

/*
 *		A few needed definitions not included in <windows.h>
 */

typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID    Pointer;
	};
  	ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
#ifdef __x86_64__
	u32    padding;
#endif
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
	ULONG           Length;
#ifdef __x86_64__
	u32		padding1;
	HANDLE          RootDirectory;
	PUNICODE_STRING ObjectName;
	ULONG           Attributes;
	u32		padding2;
#else
	HANDLE          RootDirectory;
	PUNICODE_STRING ObjectName;
	ULONG           Attributes;
#endif
	PVOID           SecurityDescriptor;
	PVOID           SecurityQualityOfService;
}  OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

#define FILE_OPEN 1
#define FILE_CREATE 2
#define FILE_OVERWRITE 4
#define FILE_SYNCHRONOUS_IO_ALERT    0x10
#define FILE_SYNCHRONOUS_IO_NONALERT 0x20
#define OBJ_CASE_INSENSITIVE 0x40

typedef void (WINAPI *PIO_APC_ROUTINE)(void*, PIO_STATUS_BLOCK, ULONG);

extern WINAPI NTSTATUS NtOpenFile(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG ShareAccess,
	ULONG OpenOptions
);

extern WINAPI NTSTATUS NtReadFile(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID Buffer,
	ULONG Length,
	PLARGE_INTEGER ByteOffset,
	PULONG Key
);

extern WINAPI NTSTATUS NtWriteFile(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	LPCVOID Buffer,
	ULONG Length,
	PLARGE_INTEGER ByteOffset,
	PULONG Key
);

extern NTSTATUS WINAPI NtClose(
	HANDLE Handle
);

extern NTSTATUS WINAPI NtDeviceIoControlFile(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG IoControlCode,
	PVOID InputBuffer,
	ULONG InputBufferLength,
	PVOID OutputBuffer,
	ULONG OutputBufferLength
);

extern NTSTATUS	WINAPI NtFsControlFile(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG FsControlCode,
	PVOID InputBuffer,
	ULONG InputBufferLength,
	PVOID OutputBuffer,
	ULONG OutputBufferLength
);

/**
 * struct win32_fd -
 */
typedef struct {
	HANDLE handle;
	s64 pos;		/* Logical current position on the volume. */
	s64 part_start;
	s64 part_length;
	int part_hidden_sectors;
	s64 geo_size, geo_cylinders;
	s32 geo_sector_size;
	s64 volume_size;
	DWORD geo_sectors, geo_heads;
	HANDLE vol_handle;
	BOOL ntdll;
} win32_fd;

/**
 * ntfs_w32error_to_errno - convert a win32 error code to the unix one
 * @w32error:	the win32 error code
 *
 * Limited to a relatively small but useful number of codes.
 */
static int ntfs_w32error_to_errno(unsigned int w32error)
{
	ntfs_log_trace("Converting w32error 0x%x.\n",w32error);
	switch (w32error) {
		case ERROR_INVALID_FUNCTION:
			return EBADRQC;
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
		case ERROR_INVALID_NAME:
			return ENOENT;
		case ERROR_TOO_MANY_OPEN_FILES:
			return EMFILE;
		case ERROR_ACCESS_DENIED:
			return EACCES;
		case ERROR_INVALID_HANDLE:
			return EBADF;
		case ERROR_NOT_ENOUGH_MEMORY:
			return ENOMEM;
		case ERROR_OUTOFMEMORY:
			return ENOSPC;
		case ERROR_INVALID_DRIVE:
		case ERROR_BAD_UNIT:
			return ENODEV;
		case ERROR_WRITE_PROTECT:
			return EROFS;
		case ERROR_NOT_READY:
		case ERROR_SHARING_VIOLATION:
			return EBUSY;
		case ERROR_BAD_COMMAND:
			return EINVAL;
		case ERROR_SEEK:
		case ERROR_NEGATIVE_SEEK:
			return ESPIPE;
		case ERROR_NOT_SUPPORTED:
			return EOPNOTSUPP;
		case ERROR_BAD_NETPATH:
			return ENOSHARE;
		default:
			/* generic message */
			return ENOMSG;
	}
}

static int ntfs_ntstatus_to_errno(NTSTATUS status)
{
	ntfs_log_trace("Converting w32error 0x%x.\n",w32error);
	switch (status) {
		case STATUS_INVALID_HANDLE :
		case STATUS_INVALID_PARAMETER :
		case STATUS_OBJECT_NAME_INVALID :
		case STATUS_INVALID_DEVICE_REQUEST :
			return (EINVAL);
		case STATUS_ACCESS_DENIED :
			return (EACCES);
		case STATUS_IO_DEVICE_ERROR :
		case STATUS_END_OF_FILE :
			return (EIO);
		case STATUS_SHARING_VIOLATION :
			return (EBUSY);
		default:
			/* generic message */
			return ENOMSG;
	}
}

/**
 * libntfs_SetFilePointerEx - emulation for SetFilePointerEx()
 *
 * We use this to emulate SetFilePointerEx() when it is not present.  This can
 * happen since SetFilePointerEx() only exists in Win2k+.
 */
static BOOL WINAPI libntfs_SetFilePointerEx(HANDLE hFile,
		LARGE_INTEGER liDistanceToMove,
		PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod)
{
	liDistanceToMove.u.LowPart = SetFilePointer(hFile,
			liDistanceToMove.u.LowPart,
			&liDistanceToMove.u.HighPart, dwMoveMethod);
	SetLastError(NO_ERROR);
	if (liDistanceToMove.u.LowPart == INVALID_SET_FILE_POINTER &&
			GetLastError() != NO_ERROR) {
		if (lpNewFilePointer)
			lpNewFilePointer->QuadPart = -1;
		return FALSE;
	}
	if (lpNewFilePointer)
		lpNewFilePointer->QuadPart = liDistanceToMove.QuadPart;
	return TRUE;
}

/**
 * ntfs_device_win32_init_imports - initialize the function pointers
 *
 * The Find*Volume and SetFilePointerEx functions exist only on win2k+, as such
 * we cannot just staticly import them.
 *
 * This function initializes the imports if the functions do exist and in the
 * SetFilePointerEx case, we emulate the function ourselves if it is not
 * present.
 *
 * Note: The values are cached, do be afraid to run it more than once.
 */
static void ntfs_device_win32_init_imports(void)
{
	HMODULE kernel32 = GetModuleHandle("kernel32");
	if (!kernel32) {
		errno = ntfs_w32error_to_errno(GetLastError());
		ntfs_log_trace("kernel32.dll could not be imported.\n");
	}
	if (!fnSetFilePointerEx) {
		if (kernel32)
			fnSetFilePointerEx = (LPFN_SETFILEPOINTEREX)
					GetProcAddress(kernel32,
					"SetFilePointerEx");
		/*
		 * If we did not get kernel32.dll or it is not Win2k+, emulate
		 * SetFilePointerEx().
		 */
		if (!fnSetFilePointerEx) {
			ntfs_log_debug("SetFilePointerEx() not found in "
					"kernel32.dll: Enabling emulation.\n");
			fnSetFilePointerEx = libntfs_SetFilePointerEx;
		}
	}
	/* Cannot do lookups if we could not get kernel32.dll... */
	if (!kernel32)
		return;
	if (!fnFindFirstVolume)
		fnFindFirstVolume = (LPFN_FINDFIRSTVOLUME)
				GetProcAddress(kernel32, "FindFirstVolume"
				FNPOSTFIX);
	if (!fnFindNextVolume)
		fnFindNextVolume = (LPFN_FINDNEXTVOLUME)
				GetProcAddress(kernel32, "FindNextVolume"
				FNPOSTFIX);
	if (!fnFindVolumeClose)
		fnFindVolumeClose = (LPFN_FINDVOLUMECLOSE)
				GetProcAddress(kernel32, "FindVolumeClose");
}

/**
 * ntfs_device_unix_status_flags_to_win32 - convert unix->win32 open flags
 * @flags:	unix open status flags
 *
 * Supported flags are O_RDONLY, O_WRONLY and O_RDWR.
 */
static __inline__ int ntfs_device_unix_status_flags_to_win32(int flags)
{
	int win_mode;

	switch (flags & O_ACCMODE) {
	case O_RDONLY:
		win_mode = GENERIC_READ;
		break;
	case O_WRONLY:
		win_mode = GENERIC_WRITE;
		break;
	case O_RDWR:
		win_mode = GENERIC_READ | GENERIC_WRITE;
		break;
	default:
		/* error */
		ntfs_log_trace("Unknown status flags.\n");
		win_mode = 0;
	}
	return win_mode;
}


/**
 * ntfs_device_win32_simple_open_file - just open a file via win32 API
 * @filename:	name of the file to open
 * @handle:	pointer the a HANDLE in which to put the result
 * @flags:	unix open status flags
 * @locking:	will the function gain an exclusive lock on the file?
 *
 * Supported flags are O_RDONLY, O_WRONLY and O_RDWR.
 *
 * Return 0 if o.k.
 *	 -1 if not, and errno set.  In this case handle is trashed.
 */
static int ntfs_device_win32_simple_open_file(const char *filename,
		HANDLE *handle, int flags, BOOL locking)
{
	*handle = CreateFile(filename,
			ntfs_device_unix_status_flags_to_win32(flags),
			locking ? 0 : (FILE_SHARE_WRITE | FILE_SHARE_READ),
			NULL, (flags & O_CREAT ? OPEN_ALWAYS : OPEN_EXISTING),
			0, NULL);
	if (*handle == INVALID_HANDLE_VALUE) {
		errno = ntfs_w32error_to_errno(GetLastError());
		ntfs_log_trace("CreateFile(%s) failed.\n", filename);
		return -1;
	}
	return 0;
}

/**
 * ntfs_device_win32_lock - lock the volume
 * @handle:	a win32 HANDLE for a volume to lock
 *
 * Locking a volume means no one can access its contents.
 * Exiting the process automatically unlocks the volume, except in old NT4s.
 *
 * Return 0 if o.k.
 *	 -1 if not, and errno set.
 */
static int ntfs_device_win32_lock(HANDLE handle)
{
	DWORD i;

	if (!DeviceIoControl(handle, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &i,
			NULL)) {
		errno = ntfs_w32error_to_errno(GetLastError());
		ntfs_log_trace("Couldn't lock volume.\n");
		return -1;
	}
	ntfs_log_debug("Volume locked.\n");
	return 0;
}

/**
 * ntfs_device_win32_unlock - unlock the volume
 * @handle:	the win32 HANDLE which the volume was locked with
 *
 * Return 0 if o.k.
 *	 -1 if not, and errno set.
 */
static int ntfs_device_win32_unlock(HANDLE handle)
{
	DWORD i;

	if (!DeviceIoControl(handle, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &i,
			NULL)) {
		errno = ntfs_w32error_to_errno(GetLastError());
		ntfs_log_trace("Couldn't unlock volume.\n");
		return -1;
	}
	ntfs_log_debug("Volume unlocked.\n");
	return 0;
}

static int ntfs_device_win32_setlock(HANDLE handle, ULONG code)
{
	IO_STATUS_BLOCK io_status;
	NTSTATUS res;

	io_status.Status = STATUS_SUCCESS;
	io_status.Information = 0;
	res = NtFsControlFile(handle,(HANDLE)NULL,
				(PIO_APC_ROUTINE)NULL,(void*)NULL,
				&io_status, code,
				(char*)NULL,0,(char*)NULL,0);
	if (res != STATUS_SUCCESS)
		errno = ntfs_ntstatus_to_errno(res);
	return (res == STATUS_SUCCESS ? 0 : -1);
}

/**
 * ntfs_device_win32_dismount - dismount a volume
 * @handle:	a win32 HANDLE for a volume to dismount
 *
 * Dismounting means the system will refresh the volume in the first change it
 * gets.  Usefull after altering the file structures.
 * The volume must be locked by the current process while dismounting.
 * A side effect is that the volume is also unlocked, but you must not rely om
 * this.
 *
 * Return 0 if o.k.
 *	 -1 if not, and errno set.
 */
static int ntfs_device_win32_dismount(HANDLE handle)
{
	DWORD i;

	if (!DeviceIoControl(handle, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0,
			&i, NULL)) {
		errno = ntfs_w32error_to_errno(GetLastError());
		ntfs_log_trace("Couldn't dismount volume.\n");
		return -1;
	}
	ntfs_log_debug("Volume dismounted.\n");
	return 0;
}

/**
 * ntfs_device_win32_getsize - get file size via win32 API
 * @handle:	pointer the file HANDLE obtained via open
 *
 * Only works on ordinary files.
 *
 * Return The file size if o.k.
 *	 -1 if not, and errno set.
 */
static s64 ntfs_device_win32_getsize(HANDLE handle)
{
	LONG loword, hiword;

	SetLastError(NO_ERROR);
	hiword = 0;
	loword = SetFilePointer(handle, 0, &hiword, 2);
	if ((loword == INVALID_SET_FILE_POINTER)
	    && (GetLastError() != NO_ERROR)) {
		errno = ntfs_w32error_to_errno(GetLastError());
		ntfs_log_trace("Couldn't get file size.\n");
		return -1;
	}
	return ((s64)hiword << 32) + (ULONG)loword;
}

/**
 * ntfs_device_win32_getdisklength - get disk size via win32 API
 * @handle:	pointer the file HANDLE obtained via open
 * @argp:	pointer to result buffer
 *
 * Only works on PhysicalDriveX type handles.
 *
 * Return The disk size if o.k.
 *	 -1 if not, and errno set.
 */
static s64 ntfs_device_win32_getdisklength(HANDLE handle)
{
	GET_LENGTH_INFORMATION buf;
	DWORD i;

	if (!DeviceIoControl(handle, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &buf,
			sizeof(buf), &i, NULL)) {
		errno = ntfs_w32error_to_errno(GetLastError());
		ntfs_log_trace("Couldn't get disk length.\n");
		return -1;
	}
	ntfs_log_debug("Disk length: %lld.\n", buf.Length.QuadPart);
	return buf.Length.QuadPart;
}

/**
 * ntfs_device_win32_getntfssize - get NTFS volume size via win32 API
 * @handle:	pointer the file HANDLE obtained via open
 * @argp:	pointer to result buffer
 *
 * Only works on NTFS volume handles.
 * An annoying bug in windows is that an NTFS volume does not occupy the entire
 * partition, namely not the last sector (which holds the backup boot sector,
 * and normally not interesting).
 * Use this function to get the length of the accessible space through a given
 * volume handle.
 *
 * Return The volume size if o.k.
 *	 -1 if not, and errno set.
 */
static s64 ntfs_device_win32_getntfssize(HANDLE handle)
{
	s64 rvl;
#ifdef FSCTL_GET_NTFS_VOLUME_DATA
	DWORD i;
	NTFS_VOLUME_DATA_BUFFER buf;

	if (!DeviceIoControl(handle, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0, &buf,
			sizeof(buf), &i, NULL)) {
		errno = ntfs_w32error_to_errno(GetLastError());
		ntfs_log_trace("Couldn't get NTFS volume length.\n");
		return -1;
	}
	rvl = buf.NumberSectors.QuadPart * buf.BytesPerSector;
	ntfs_log_debug("NTFS volume length: 0x%llx.\n", (long long)rvl);
#else
	errno = EINVAL;
	rvl = -1;
#endif
	return rvl;
}

/**
 * ntfs_device_win32_getgeo - get CHS information of a drive
 * @handle:	an open handle to the PhysicalDevice
 * @fd:		a win_fd structure that will be filled
 *
 * Return 0 if o.k.
 *	 -1 if not, and errno  set.
 *
 * In Windows NT+: fills size, sectors, and cylinders and sets heads to -1.
 * In Windows XP+: fills size, sectors, cylinders, and heads.
 *
 * Note: In pre XP, this requires write permission, even though nothing is
 * actually written.
 *
 * If fails, sets sectors, cylinders, heads, and size to -1.
 */
static int ntfs_device_win32_getgeo(HANDLE handle, win32_fd *fd)
{
	DWORD i;
	BOOL rvl;
	BYTE b[sizeof(DISK_GEOMETRY) + sizeof(DISK_PARTITION_INFO) +
			sizeof(DISK_DETECTION_INFO) + 512];

	rvl = DeviceIoControl(handle, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL,
			0, &b, sizeof(b), &i, NULL);
	if (rvl) {
		ntfs_log_debug("GET_DRIVE_GEOMETRY_EX detected.\n");
		DISK_DETECTION_INFO *ddi = (PDISK_DETECTION_INFO)
				(((PBYTE)(&((PDISK_GEOMETRY_EX)b)->Data)) +
				(((PDISK_PARTITION_INFO)
				(&((PDISK_GEOMETRY_EX)b)->Data))->
				SizeOfPartitionInfo));
		fd->geo_cylinders = ((DISK_GEOMETRY*)&b)->Cylinders.QuadPart;
		fd->geo_sectors = ((DISK_GEOMETRY*)&b)->SectorsPerTrack;
		fd->geo_size = ((DISK_GEOMETRY_EX*)&b)->DiskSize.QuadPart;
		fd->geo_sector_size = NTFS_BLOCK_SIZE;
		switch (ddi->DetectionType) {
		case DetectInt13:
			fd->geo_cylinders = ddi->Int13.MaxCylinders;
			fd->geo_sectors = ddi->Int13.SectorsPerTrack;
			fd->geo_heads = ddi->Int13.MaxHeads;
			return 0;
		case DetectExInt13:
			fd->geo_cylinders = ddi->ExInt13.ExCylinders;
			fd->geo_sectors = ddi->ExInt13.ExSectorsPerTrack;
			fd->geo_heads = ddi->ExInt13.ExHeads;
			return 0;
		case DetectNone:
		default:
			break;
		}
	} else
		fd->geo_heads = -1;
	rvl = DeviceIoControl(handle, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0,
			&b, sizeof(b), &i, NULL);
	if (rvl) {
		ntfs_log_debug("GET_DRIVE_GEOMETRY detected.\n");
		fd->geo_cylinders = ((DISK_GEOMETRY*)&b)->Cylinders.QuadPart;
		fd->geo_sectors = ((DISK_GEOMETRY*)&b)->SectorsPerTrack;
		fd->geo_size = fd->geo_cylinders * fd->geo_sectors *
				((DISK_GEOMETRY*)&b)->TracksPerCylinder *
				((DISK_GEOMETRY*)&b)->BytesPerSector;
		fd->geo_sector_size = ((DISK_GEOMETRY*)&b)->BytesPerSector;
		return 0;
	}
	errno = ntfs_w32error_to_errno(GetLastError());
	ntfs_log_trace("Couldn't retrieve disk geometry.\n");
	fd->geo_cylinders = -1;
	fd->geo_sectors = -1;
	fd->geo_size = -1;
	fd->geo_sector_size = NTFS_BLOCK_SIZE;
	return -1;
}

static int ntfs_device_win32_getntgeo(HANDLE handle, win32_fd *fd)
{
	DISK_GEOMETRY geo;
	NTSTATUS st;
	IO_STATUS_BLOCK status;
	u64 bytes;
	int res;

	res = -1;
	fd->geo_cylinders = 0;
	fd->geo_sectors = 0;
	fd->geo_size = 1073741824;
	fd->geo_sectors = fd->geo_size >> 9;
	fd->geo_sector_size = NTFS_BLOCK_SIZE;

	st = NtDeviceIoControlFile(handle, (HANDLE)NULL,
			(PIO_APC_ROUTINE)NULL, (void*)NULL,
			&status, IOCTL_DISK_GET_DRIVE_GEOMETRY, (void*)NULL, 0,
			(void*)&geo, sizeof(geo));
	if (st == STATUS_SUCCESS) {
		/* over-estimate the (rounded) number of cylinders */
		fd->geo_cylinders = geo.Cylinders.QuadPart + 1;
		fd->geo_sectors = fd->geo_cylinders
				*geo.TracksPerCylinder*geo.SectorsPerTrack;
		fd->geo_size = fd->geo_sectors*geo.BytesPerSector;
		fd->geo_sector_size = geo.BytesPerSector;
		res = 0;
			/* try to get the exact sector count */
		st = NtDeviceIoControlFile(handle, (HANDLE)NULL,
				(PIO_APC_ROUTINE)NULL, (void*)NULL,
				&status, IOCTL_GET_DISK_LENGTH_INFO,
				(void*)NULL, 0,
				(void*)&bytes, sizeof(bytes));
		if (st == STATUS_SUCCESS) {
			fd->geo_size = bytes;
			fd->geo_sectors = bytes/geo.BytesPerSector;
		}
	}
	return (res);
}

/**
 * ntfs_device_win32_open_file - open a file via win32 API
 * @filename:	name of the file to open
 * @fd:		pointer to win32 file device in which to put the result
 * @flags:	unix open status flags
 *
 * Return 0 if o.k.
 *	 -1 if not, and errno set.
 */
static __inline__ int ntfs_device_win32_open_file(char *filename, win32_fd *fd,
		int flags)
{
	HANDLE handle;
	int mode;

	if (ntfs_device_win32_simple_open_file(filename, &handle, flags,
			FALSE)) {
		/* open error */
		return -1;
	}
	mode = flags & O_ACCMODE;
	if ((mode == O_RDWR) || (mode == O_WRONLY)) {
		DWORD bytes;

		/* try making sparse (but ignore errors) */
		DeviceIoControl(handle, FSCTL_SET_SPARSE,
				(void*)NULL, 0, (void*)NULL, 0,
				&bytes, (LPOVERLAPPED)NULL);
	}
	/* fill fd */
	fd->handle = handle;
	fd->part_start = 0;
	fd->part_length = ntfs_device_win32_getsize(handle);
	fd->pos = 0;
	fd->part_hidden_sectors = -1;
	fd->geo_size = -1;	/* used as a marker that this is a file */
	fd->vol_handle = INVALID_HANDLE_VALUE;
	fd->geo_sector_size = 512;  /* will be adjusted from the boot sector */
	fd->ntdll = FALSE;
	return 0;
}

/**
 * ntfs_device_win32_open_drive - open a drive via win32 API
 * @drive_id:	drive to open
 * @fd:		pointer to win32 file device in which to put the result
 * @flags:	unix open status flags
 *
 * return 0 if o.k.
 *        -1 if not, and errno set.
 */
static __inline__ int ntfs_device_win32_open_drive(int drive_id, win32_fd *fd,
		int flags)
{
	HANDLE handle;
	int err;
	char filename[MAX_PATH];

	sprintf(filename, "\\\\.\\PhysicalDrive%d", drive_id);
	if ((err = ntfs_device_win32_simple_open_file(filename, &handle, flags,
			TRUE))) {
		/* open error */
		return err;
	}
	/* store the drive geometry */
	ntfs_device_win32_getgeo(handle, fd);
	/* Just to be sure */
	if (fd->geo_size == -1)
		fd->geo_size = ntfs_device_win32_getdisklength(handle);
	/* fill fd */
	fd->ntdll = FALSE;
	fd->handle = handle;
	fd->part_start = 0;
	fd->part_length = fd->geo_size;
	fd->pos = 0;
	fd->part_hidden_sectors = -1;
	fd->vol_handle = INVALID_HANDLE_VALUE;
	return 0;
}

/**
 * ntfs_device_win32_open_lowlevel - open a drive via low level win32 API
 * @drive_id:	drive to open
 * @fd:		pointer to win32 file device in which to put the result
 * @flags:	unix open status flags
 *
 * return 0 if o.k.
 *        -1 if not, and errno set.
 */
static __inline__ int ntfs_device_win32_open_lowlevel(int drive_id,
		win32_fd *fd, int flags)
{
	HANDLE handle;
	NTSTATUS st;
	ACCESS_MASK access;
	ULONG share;
	OBJECT_ATTRIBUTES attr;
	IO_STATUS_BLOCK io_status;
	UNICODE_STRING unicode_name;
	ntfschar unicode_buffer[7];
	int mode;
	static const ntfschar unicode_init[] = {
		const_cpu_to_le16('\\'), const_cpu_to_le16('?'),
		const_cpu_to_le16('?'), const_cpu_to_le16('\\'),
		const_cpu_to_le16(' '), const_cpu_to_le16(':'),
		const_cpu_to_le16(0)
	};

	memcpy(unicode_buffer, unicode_init, sizeof(unicode_buffer));
	unicode_buffer[4] = cpu_to_le16(drive_id + 'A');
	unicode_name.Buffer = unicode_buffer;
	unicode_name.Length = 6*sizeof(ntfschar);
	unicode_name.MaximumLength = 6*sizeof(ntfschar);

	attr.Length = sizeof(OBJECT_ATTRIBUTES);
	attr.RootDirectory = (HANDLE*)NULL;
	attr.ObjectName = &unicode_name;
	attr.Attributes = OBJ_CASE_INSENSITIVE;
	attr.SecurityDescriptor = (void*)NULL;
	attr.SecurityQualityOfService = (void*)NULL;

	io_status.Status = 0;
	io_status.Information = 0;
	mode = flags & O_ACCMODE;
	share = (mode == O_RDWR ?
			0 : FILE_SHARE_READ | FILE_SHARE_WRITE);
	access = (mode == O_RDWR ?
			 FILE_READ_DATA | FILE_WRITE_DATA | SYNCHRONIZE
			: FILE_READ_DATA | SYNCHRONIZE);

	st = NtOpenFile(&handle, access,
			&attr, &io_status,
			share,
			FILE_SYNCHRONOUS_IO_ALERT);
	if (st != STATUS_SUCCESS) {
		errno = ntfs_ntstatus_to_errno(st);
		return (-1);
	}
	ntfs_device_win32_setlock(handle,FSCTL_LOCK_VOLUME);
	/* store the drive geometry */
	ntfs_device_win32_getntgeo(handle, fd);
	fd->ntdll = TRUE;
	/* allow accessing the full partition */
	st = NtFsControlFile(handle, (HANDLE)NULL,
			(PIO_APC_ROUTINE)NULL,
			(PVOID)NULL, &io_status,
			FSCTL_ALLOW_EXTENDED_DASD_IO,
			NULL, 0, NULL, 0);
	if (st != STATUS_SUCCESS) {
		errno = ntfs_ntstatus_to_errno(st);
		NtClose(handle);
		return (-1);
	}
	/* fill fd */
	fd->handle = handle;
	fd->part_start = 0;
	fd->part_length = fd->geo_size;
	fd->pos = 0;
	fd->part_hidden_sectors = -1;
	fd->vol_handle = INVALID_HANDLE_VALUE;
	return 0;
}

/**
 * ntfs_device_win32_open_volume_for_partition - find and open a volume
 *
 * Windows NT/2k/XP handles volumes instead of partitions.
 * This function gets the partition details and return an open volume handle.
 * That volume is the one whose only physical location on disk is the described
 * partition.
 *
 * The function required Windows 2k/XP, otherwise it fails (gracefully).
 *
 * Return success: a valid open volume handle.
 *        fail   : INVALID_HANDLE_VALUE
 */
static HANDLE ntfs_device_win32_open_volume_for_partition(unsigned int drive_id,
		s64 part_offset, s64 part_length, int flags)
{
	HANDLE vol_find_handle;
	TCHAR vol_name[MAX_PATH];

	/* Make sure all the required imports exist. */
	if (!fnFindFirstVolume || !fnFindNextVolume || !fnFindVolumeClose) {
		ntfs_log_trace("Required dll imports not found.\n");
		return INVALID_HANDLE_VALUE;
	}
	/* Start iterating through volumes. */
	ntfs_log_trace("Entering with drive_id=%d, part_offset=%lld, "
			"path_length=%lld, flags=%d.\n", drive_id,
			(unsigned long long)part_offset,
			(unsigned long long)part_length, flags);
	vol_find_handle = fnFindFirstVolume(vol_name, MAX_PATH);
	/* If a valid handle could not be aquired, reply with "don't know". */
	if (vol_find_handle == INVALID_HANDLE_VALUE) {
		ntfs_log_trace("FindFirstVolume failed.\n");
		return INVALID_HANDLE_VALUE;
	}
	do {
		int vol_name_length;
		HANDLE handle;

		/* remove trailing '/' from vol_name */
#ifdef UNICODE
		vol_name_length = wcslen(vol_name);
#else
		vol_name_length = strlen(vol_name);
#endif
		if (vol_name_length>0)
			vol_name[vol_name_length-1]=0;

		ntfs_log_debug("Processing %s.\n", vol_name);
		/* open the file */
		handle = CreateFile(vol_name,
				ntfs_device_unix_status_flags_to_win32(flags),
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
				OPEN_EXISTING, 0, NULL);
		if (handle != INVALID_HANDLE_VALUE) {
			DWORD bytesReturned;
#define EXTENTS_SIZE sizeof(VOLUME_DISK_EXTENTS) + 9 * sizeof(DISK_EXTENT)
			char extents[EXTENTS_SIZE];

			/* Check physical locations. */
			if (DeviceIoControl(handle,
					IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
					NULL, 0, extents, EXTENTS_SIZE,
					&bytesReturned, NULL)) {
				if (((VOLUME_DISK_EXTENTS *)extents)->
						NumberOfDiskExtents == 1) {
					DISK_EXTENT *extent = &((
							VOLUME_DISK_EXTENTS *)
							extents)->Extents[0];
					if ((extent->DiskNumber==drive_id) &&
							(extent->StartingOffset.
							QuadPart==part_offset)
							&& (extent->
							ExtentLength.QuadPart
							== part_length)) {
						/*
						 * Eureka! (Archimedes, 287 BC,
						 * "I have found it!")
						 */
						fnFindVolumeClose(
							vol_find_handle);
						return handle;
					}
				}
			}
		} else
			ntfs_log_trace("getExtents() Failed.\n");
	} while (fnFindNextVolume(vol_find_handle, vol_name, MAX_PATH));
	/* End of iteration through volumes. */
	ntfs_log_trace("Closing, volume was not found.\n");
	fnFindVolumeClose(vol_find_handle);
	return INVALID_HANDLE_VALUE;
}

/**
 * ntfs_device_win32_find_partition - locates partition details by id.
 * @handle:		HANDLE to the PhysicalDrive
 * @partition_id:	the partition number to locate
 * @part_offset:	pointer to where to put the offset to the partition
 * @part_length:	pointer to where to put the length of the partition
 * @hidden_sectors:	pointer to where to put the hidden sectors
 *
 * This function requires an open PhysicalDrive handle and a partition_id.
 * If a partition with the required id is found on the supplied device,
 * the partition attributes are returned back.
 *
 * Returns: TRUE  if found, and sets the output parameters.
 *          FALSE if not and errno is set to the error code.
 */
static BOOL ntfs_device_win32_find_partition(HANDLE handle, DWORD partition_id,
		s64 *part_offset, s64 *part_length, int *hidden_sectors)
{
	DRIVE_LAYOUT_INFORMATION *drive_layout;
	unsigned int err, buf_size, part_count;
	DWORD i;

	/*
	 * There is no way to know the required buffer, so if the ioctl fails,
	 * try doubling the buffer size each time until the ioctl succeeds.
	 */
	part_count = 8;
	do {
		buf_size = sizeof(DRIVE_LAYOUT_INFORMATION) +
				part_count * sizeof(PARTITION_INFORMATION);
		drive_layout = (DRIVE_LAYOUT_INFORMATION*)ntfs_malloc(buf_size);
		if (!drive_layout) {
			errno = ENOMEM;
			return FALSE;
		}
		if (DeviceIoControl(handle, IOCTL_DISK_GET_DRIVE_LAYOUT, NULL,
				0, (BYTE*)drive_layout, buf_size, &i, NULL))
			break;
		err = GetLastError();
		free(drive_layout);
		if (err != ERROR_INSUFFICIENT_BUFFER) {
			ntfs_log_trace("GetDriveLayout failed.\n");
			errno = ntfs_w32error_to_errno(err);
			return FALSE;
		}
		ntfs_log_debug("More than %u partitions.\n", part_count);
		part_count <<= 1;
		if (part_count > 512) {
			ntfs_log_trace("GetDriveLayout failed: More than 512 "
					"partitions?\n");
			errno = ENOBUFS;
			return FALSE;
		}
	} while (1);
	for (i = 0; i < drive_layout->PartitionCount; i++) {
		if (drive_layout->PartitionEntry[i].PartitionNumber ==
				partition_id) {
			*part_offset = drive_layout->PartitionEntry[i].
					StartingOffset.QuadPart;
			*part_length = drive_layout->PartitionEntry[i].
					PartitionLength.QuadPart;
			*hidden_sectors = drive_layout->PartitionEntry[i].
					HiddenSectors;
			free(drive_layout);
			return TRUE;
		}
	}
	free(drive_layout);
	errno = ENOENT;
	return FALSE;
}

/**
 * ntfs_device_win32_open_partition - open a partition via win32 API
 * @drive_id:		drive to open
 * @partition_id:	partition to open
 * @fd:			win32 file device to return
 * @flags:		unix open status flags
 *
 * Return  0 if o.k.
 *        -1 if not, and errno set.
 *
 * When fails, fd contents may have not been preserved.
 */
static int ntfs_device_win32_open_partition(int drive_id,
		unsigned int partition_id, win32_fd *fd, int flags)
{
	s64 part_start, part_length;
	HANDLE handle;
	int err, hidden_sectors;
	char drive_name[MAX_PATH];

	sprintf(drive_name, "\\\\.\\PhysicalDrive%d", drive_id);
	/* Open the entire device without locking, ask questions later */
	if ((err = ntfs_device_win32_simple_open_file(drive_name, &handle,
			flags, FALSE))) {
		/* error */
		return err;
	}
	if (ntfs_device_win32_find_partition(handle, partition_id, &part_start,
			&part_length, &hidden_sectors)) {
		s64 tmp;
		HANDLE vol_handle = ntfs_device_win32_open_volume_for_partition(
			drive_id, part_start, part_length, flags);
		/* Store the drive geometry. */
		ntfs_device_win32_getgeo(handle, fd);
		fd->handle = handle;
		fd->pos = 0;
		fd->part_start = part_start;
		fd->part_length = part_length;
		fd->part_hidden_sectors = hidden_sectors;
		fd->geo_sector_size = 512;
		fd->ntdll = FALSE;
		tmp = ntfs_device_win32_getntfssize(vol_handle);
		if (tmp > 0)
			fd->geo_size = tmp;
		else
			fd->geo_size = fd->part_length;
		if (vol_handle != INVALID_HANDLE_VALUE) {
			if (((flags & O_RDWR) == O_RDWR) &&
					ntfs_device_win32_lock(vol_handle)) {
				CloseHandle(vol_handle);
				CloseHandle(handle);
				return -1;
			}
			fd->vol_handle = vol_handle;
		} else {
			if ((flags & O_RDWR) == O_RDWR) {
				/* Access if read-write, no volume found. */
				ntfs_log_trace("Partitions containing Spanned/"
						"Mirrored volumes are not "
						"supported in R/W status "
						"yet.\n");
				CloseHandle(handle);
				errno = EOPNOTSUPP;
				return -1;
			}
			fd->vol_handle = INVALID_HANDLE_VALUE;
		}
		return 0;
	} else {
		ntfs_log_debug("Partition %u not found on drive %d.\n",
				partition_id, drive_id);
		CloseHandle(handle);
		errno = ENODEV;
		return -1;
	}
}

/**
 * ntfs_device_win32_open - open a device
 * @dev:	a pointer to the NTFS_DEVICE to open
 * @flags:	unix open status flags
 *
 * @dev->d_name must hold the device name, the rest is ignored.
 * Supported flags are O_RDONLY, O_WRONLY and O_RDWR.
 *
 * If name is in format "(hd[0-9],[0-9])" then open a partition.
 * If name is in format "(hd[0-9])" then open a volume.
 * Otherwise open a file.
 */
static int ntfs_device_win32_open(struct ntfs_device *dev, int flags)
{
	int drive_id = 0, numparams;
	unsigned int part = 0;
	char drive_char;
	win32_fd fd;
	int err;

	if (NDevOpen(dev)) {
		errno = EBUSY;
		return -1;
	}
	ntfs_device_win32_init_imports();
	numparams = sscanf(dev->d_name, "/dev/hd%c%u", &drive_char, &part);
	if (!numparams
	    && (dev->d_name[1] == ':')
	    && (dev->d_name[2] == '\0')) {
		drive_char = dev->d_name[0];
		numparams = 3;
		drive_id = toupper(drive_char) - 'A';
	}
	switch (numparams) {
	case 0:
		ntfs_log_debug("win32_open(%s) -> file.\n", dev->d_name);
		err = ntfs_device_win32_open_file(dev->d_name, &fd, flags);
		break;
	case 1:
		ntfs_log_debug("win32_open(%s) -> drive %d.\n", dev->d_name,
				drive_id);
		err = ntfs_device_win32_open_drive(drive_id, &fd, flags);
		break;
	case 2:
		ntfs_log_debug("win32_open(%s) -> drive %d, part %u.\n",
				dev->d_name, drive_id, part);
		err = ntfs_device_win32_open_partition(drive_id, part, &fd,
				flags);
		break;
	case 3:
		ntfs_log_debug("win32_open(%s) -> drive %c:\n",
				dev->d_name, drive_char);
		err = ntfs_device_win32_open_lowlevel(drive_id, &fd,
				flags);
		break;
	default:
		ntfs_log_debug("win32_open(%s) -> unknwon file format.\n",
				dev->d_name);
		err = -1;
	}
	if (err)
		return err;
	ntfs_log_debug("win32_open(%s) -> %p, offset 0x%llx.\n", dev->d_name,
			dev, fd.part_start);
	/* Setup our read-only flag. */
	if ((flags & O_RDWR) != O_RDWR)
		NDevSetReadOnly(dev);
	dev->d_private = (win32_fd*)ntfs_malloc(sizeof(win32_fd));
	memcpy(dev->d_private, &fd, sizeof(win32_fd));
	NDevSetOpen(dev);
	NDevClearDirty(dev);
	return 0;
}

/**
 * ntfs_device_win32_seek - change current logical file position
 * @dev:	ntfs device obtained via ->open
 * @offset:	required offset from the whence anchor
 * @whence:	whence anchor specifying what @offset is relative to
 *
 * Return the new position on the volume on success and -1 on error with errno
 * set to the error code.
 *
 * @whence may be one of the following:
 *	SEEK_SET - Offset is relative to file start.
 *	SEEK_CUR - Offset is relative to current position.
 *	SEEK_END - Offset is relative to end of file.
 */
static s64 ntfs_device_win32_seek(struct ntfs_device *dev, s64 offset,
		int whence)
{
	s64 abs_ofs;
	win32_fd *fd = (win32_fd *)dev->d_private;

	ntfs_log_trace("seek offset = 0x%llx, whence = %d.\n", offset, whence);
	switch (whence) {
	case SEEK_SET:
		abs_ofs = offset;
		break;
	case SEEK_CUR:
		abs_ofs = fd->pos + offset;
		break;
	case SEEK_END:
		/* End of partition != end of disk. */
		if (fd->part_length == -1) {
			ntfs_log_trace("Position relative to end of disk not "
					"implemented.\n");
			errno = EOPNOTSUPP;
			return -1;
		}
		abs_ofs = fd->part_length + offset;
		break;
	default:
		ntfs_log_trace("Wrong mode %d.\n", whence);
		errno = EINVAL;
		return -1;
	}
	if ((abs_ofs < 0)
	    || (fd->ntdll && (abs_ofs > fd->part_length))) {
		ntfs_log_trace("Seeking outsize seekable area.\n");
		errno = EINVAL;
		return -1;
	}
	fd->pos = abs_ofs;
	return abs_ofs;
}

/**
 * ntfs_device_win32_pio - positioned low level i/o
 * @fd:		win32 device descriptor obtained via ->open
 * @pos:	at which position to do i/o from/to
 * @count:	how many bytes should be transfered
 * @b:		source/destination buffer
 * @write:	TRUE if write transfer and FALSE if read transfer
 *
 * On success returns the number of bytes transfered (can be < @count) and on
 * error returns -1 and errno set.  Transfer starts from position @pos on @fd.
 *
 * Notes:
 *	- @pos, @buf, and @count must be aligned to geo_sector_size
 *	- When dealing with volumes, a single call must not span both volume
 *	  and disk extents.
 *	- Does not use/set @fd->pos.
 */
static s64 ntfs_device_win32_pio(win32_fd *fd, const s64 pos,
		const s64 count, void *rbuf, const void *wbuf)
{
	LARGE_INTEGER li;
	HANDLE handle;
	DWORD bt;
	BOOL res;
	s64 bytes;

	ntfs_log_trace("pos = 0x%llx, count = 0x%llx, direction = %s.\n",
			(long long)pos, (long long)count, write ? "write" :
			"read");
	li.QuadPart = pos;
	if (fd->vol_handle != INVALID_HANDLE_VALUE && pos < fd->geo_size) {
		ntfs_log_debug("Transfering via vol_handle.\n");
		handle = fd->vol_handle;
	} else {
		ntfs_log_debug("Transfering via handle.\n");
		handle = fd->handle;
		li.QuadPart += fd->part_start;
	}

	if (fd->ntdll) {
		IO_STATUS_BLOCK io_status;
		NTSTATUS res;
		LARGE_INTEGER offset;

		io_status.Status = STATUS_SUCCESS;
		io_status.Information = 0;
		offset.QuadPart = pos;
		if (wbuf) {
			res = NtWriteFile(fd->handle,(HANDLE)NULL,
					(PIO_APC_ROUTINE)NULL,(void*)NULL,
					&io_status, wbuf, count,
					&offset, (PULONG)NULL);
		} else {
			res = NtReadFile(fd->handle,(HANDLE)NULL,
					(PIO_APC_ROUTINE)NULL,(void*)NULL,
					&io_status, rbuf, count,
					&offset, (PULONG)NULL);
		}
		if (res == STATUS_SUCCESS) {
			bytes = io_status.Information;
		} else {
			bytes = -1;
			errno = ntfs_ntstatus_to_errno(res);
		}
	} else {
		if (!fnSetFilePointerEx(handle, li, NULL, FILE_BEGIN)) {
			errno = ntfs_w32error_to_errno(GetLastError());
			ntfs_log_trace("SetFilePointer failed.\n");
			return -1;
		}
		if (wbuf)
			res = WriteFile(handle, wbuf, count, &bt, NULL);
		else
			res = ReadFile(handle, rbuf, count, &bt, NULL);
		bytes = bt;
		if (!res) {
			errno = ntfs_w32error_to_errno(GetLastError());
			ntfs_log_trace("%sFile() failed.\n", write ?
							"Write" : "Read");
			return -1;
		}
		if (rbuf && !pos) {
			/* get the sector size from the boot sector */
			char *boot = (char*)rbuf;
			fd->geo_sector_size = (boot[11] & 255)
						+ ((boot[12] & 255) << 8);
		}
	}
	return bytes;
}

/**
 * ntfs_device_win32_pread_simple - positioned simple read
 * @fd:		win32 device descriptor obtained via ->open
 * @pos:	at which position to read from
 * @count:	how many bytes should be read
 * @b:		a pointer to where to put the contents
 *
 * On success returns the number of bytes read (can be < @count) and on error
 * returns -1 and errno set.  Read starts from position @pos.
 *
 * Notes:
 *	- @pos, @buf, and @count must be aligned to geo_sector_size.
 *	- When dealing with volumes, a single call must not span both volume
 *	  and disk extents.
 *	- Does not use/set @fd->pos.
 */
static inline s64 ntfs_device_win32_pread_simple(win32_fd *fd, const s64 pos,
		const s64 count, void *b)
{
	return ntfs_device_win32_pio(fd, pos, count, b, (void*)NULL);
}

/**
 * ntfs_device_win32_read - read bytes from an ntfs device
 * @dev:	ntfs device obtained via ->open
 * @b:		pointer to where to put the contents
 * @count:	how many bytes should be read
 *
 * On success returns the number of bytes actually read (can be < @count).
 * On error returns -1 with errno set.
 */
static s64 ntfs_device_win32_read(struct ntfs_device *dev, void *b, s64 count)
{
	s64 old_pos, to_read, i, br = 0;
	win32_fd *fd = (win32_fd *)dev->d_private;
	BYTE *alignedbuffer;
	int old_ofs, ofs;

	old_pos = fd->pos;
	old_ofs = ofs = old_pos & (fd->geo_sector_size - 1);
	to_read = (ofs + count + fd->geo_sector_size - 1) &
			~(s64)(fd->geo_sector_size - 1);
	/* Impose maximum of 2GB to be on the safe side. */
	if (to_read > 0x80000000) {
		int delta = to_read - count;
		to_read = 0x80000000;
		count = to_read - delta;
	}
	ntfs_log_trace("fd = %p, b = %p, count = 0x%llx, pos = 0x%llx, "
			"ofs = %i, to_read = 0x%llx.\n", fd, b,
			(long long)count, (long long)old_pos, ofs,
			(long long)to_read);
	if (!((unsigned long)b & (fd->geo_sector_size - 1)) && !old_ofs &&
			!(count & (fd->geo_sector_size - 1)))
		alignedbuffer = b;
	else {
		alignedbuffer = (BYTE *)VirtualAlloc(NULL, to_read, MEM_COMMIT,
				PAGE_READWRITE);
		if (!alignedbuffer) {
			errno = ntfs_w32error_to_errno(GetLastError());
			ntfs_log_trace("VirtualAlloc failed for read.\n");
			return -1;
		}
	}
	if (fd->vol_handle != INVALID_HANDLE_VALUE && old_pos < fd->geo_size) {
		s64 vol_to_read = fd->geo_size - old_pos;
		if (count > vol_to_read) {
			br = ntfs_device_win32_pread_simple(fd,
					old_pos & ~(s64)(fd->geo_sector_size - 1),
					ofs + vol_to_read, alignedbuffer);
			if (br == -1)
				goto read_error;
			to_read -= br;
			if (br < ofs) {
				br = 0;
				goto read_partial;
			}
			br -= ofs;
			fd->pos += br;
			ofs = fd->pos & (fd->geo_sector_size - 1);
			if (br != vol_to_read)
				goto read_partial;
		}
	}
	i = ntfs_device_win32_pread_simple(fd,
			fd->pos & ~(s64)(fd->geo_sector_size - 1), to_read,
			alignedbuffer + br);
	if (i == -1) {
		if (br)
			goto read_partial;
		goto read_error;
	}
	if (i < ofs)
		goto read_partial;
	i -= ofs;
	br += i;
	if (br > count)
		br = count;
	fd->pos = old_pos + br;
read_partial:
	if (alignedbuffer != b) {
		memcpy((void*)b, alignedbuffer + old_ofs, br);
		VirtualFree(alignedbuffer, 0, MEM_RELEASE);
	}
	return br;
read_error:
	if (alignedbuffer != b)
		VirtualFree(alignedbuffer, 0, MEM_RELEASE);
	return -1;
}

/**
 * ntfs_device_win32_close - close an open ntfs deivce
 * @dev:	ntfs device obtained via ->open
 *
 * Return 0 if o.k.
 *	 -1 if not, and errno set.  Note if error fd->vol_handle is trashed.
 */
static int ntfs_device_win32_close(struct ntfs_device *dev)
{
	win32_fd *fd = (win32_fd *)dev->d_private;
	BOOL rvl;

	ntfs_log_trace("Closing device %p.\n", dev);
	if (!NDevOpen(dev)) {
		errno = EBADF;
		return -1;
	}
	if (fd->vol_handle != INVALID_HANDLE_VALUE) {
		if (!NDevReadOnly(dev)) {
			ntfs_device_win32_dismount(fd->vol_handle);
			ntfs_device_win32_unlock(fd->vol_handle);
		}
		if (!CloseHandle(fd->vol_handle))
			ntfs_log_trace("CloseHandle() failed for volume.\n");
	}
	if (fd->ntdll) {
		ntfs_device_win32_setlock(fd->handle,FSCTL_UNLOCK_VOLUME);
		rvl = NtClose(fd->handle) == STATUS_SUCCESS;
	} else
		rvl = CloseHandle(fd->handle);
	NDevClearOpen(dev);
	free(fd);
	if (!rvl) {
		errno = ntfs_w32error_to_errno(GetLastError());
		if (fd->ntdll)
			ntfs_log_trace("NtClose() failed.\n");
		else
			ntfs_log_trace("CloseHandle() failed.\n");
		return -1;
	}
	return 0;
}

/**
 * ntfs_device_win32_sync - flush write buffers to disk
 * @dev:	ntfs device obtained via ->open
 *
 * Return 0 if o.k.
 *	 -1 if not, and errno set.
 *
 * Note: Volume syncing works differently in windows.
 *	 Disk cannot be synced in windows.
 */
static int ntfs_device_win32_sync(struct ntfs_device *dev)
{
	int err = 0;
	BOOL to_clear = TRUE;

	if (!NDevReadOnly(dev) && NDevDirty(dev)) {
		win32_fd *fd = (win32_fd *)dev->d_private;

		if ((fd->vol_handle != INVALID_HANDLE_VALUE) &&
				!FlushFileBuffers(fd->vol_handle)) {
			to_clear = FALSE;
			err = ntfs_w32error_to_errno(GetLastError());
		}
		if (!FlushFileBuffers(fd->handle)) {
			to_clear = FALSE;
			if (!err)
				err = ntfs_w32error_to_errno(GetLastError());
		}
		if (!to_clear) {
			ntfs_log_trace("Could not sync.\n");
			errno = err;
			return -1;
		}
		NDevClearDirty(dev);
	}
	return 0;
}

/**
 * ntfs_device_win32_pwrite_simple - positioned simple write
 * @fd:		win32 device descriptor obtained via ->open
 * @pos:	at which position to write to
 * @count:	how many bytes should be written
 * @b:		a pointer to the data to write
 *
 * On success returns the number of bytes written and on error returns -1 and
 * errno set.  Write starts from position @pos.
 *
 * Notes:
 *	- @pos, @buf, and @count must be aligned to geo_sector_size.
 *	- When dealing with volumes, a single call must not span both volume
 *	  and disk extents.
 *	- Does not use/set @fd->pos.
 */
static inline s64 ntfs_device_win32_pwrite_simple(win32_fd *fd, const s64 pos,
		const s64 count, const void *b)
{
	return ntfs_device_win32_pio(fd, pos, count, (void*)NULL, b);
}

/**
 * ntfs_device_win32_write - write bytes to an ntfs device
 * @dev:	ntfs device obtained via ->open
 * @b:		pointer to the data to write
 * @count:	how many bytes should be written
 *
 * On success returns the number of bytes actually written.
 * On error returns -1 with errno set.
 */
static s64 ntfs_device_win32_write(struct ntfs_device *dev, const void *b,
		s64 count)
{
	s64 old_pos, to_write, i, bw = 0;
	win32_fd *fd = (win32_fd *)dev->d_private;
	const BYTE *alignedbuffer;
	BYTE *readbuffer;
	int old_ofs, ofs;

	old_pos = fd->pos;
	old_ofs = ofs = old_pos & (fd->geo_sector_size - 1);
	to_write = (ofs + count + fd->geo_sector_size - 1) &
			~(s64)(fd->geo_sector_size - 1);
	/* Impose maximum of 2GB to be on the safe side. */
	if (to_write > 0x80000000) {
		int delta = to_write - count;
		to_write = 0x80000000;
		count = to_write - delta;
	}
	ntfs_log_trace("fd = %p, b = %p, count = 0x%llx, pos = 0x%llx, "
			"ofs = %i, to_write = 0x%llx.\n", fd, b,
			(long long)count, (long long)old_pos, ofs,
			(long long)to_write);
	if (NDevReadOnly(dev)) {
		ntfs_log_trace("Can't write on a R/O device.\n");
		errno = EROFS;
		return -1;
	}
	if (!count)
		return 0;
	NDevSetDirty(dev);
	readbuffer = (BYTE*)NULL;
	if (!((unsigned long)b & (fd->geo_sector_size - 1)) && !old_ofs &&
			!(count & (fd->geo_sector_size - 1)))
		alignedbuffer = (const BYTE *)b;
	else {
		s64 end;

		readbuffer = (BYTE *)VirtualAlloc(NULL, to_write,
				MEM_COMMIT, PAGE_READWRITE);
		if (!readbuffer) {
			errno = ntfs_w32error_to_errno(GetLastError());
			ntfs_log_trace("VirtualAlloc failed for write.\n");
			return -1;
		}
		/* Read first sector if start of write not sector aligned. */
		if (ofs) {
			i = ntfs_device_win32_pread_simple(fd,
					old_pos & ~(s64)(fd->geo_sector_size - 1),
					fd->geo_sector_size, readbuffer);
			if (i != fd->geo_sector_size) {
				if (i >= 0)
					errno = EIO;
				goto write_error;
			}
		}
		/*
		 * Read last sector if end of write not sector aligned and last
		 * sector is either not the same as the first sector or it is
		 * the same as the first sector but this has not been read in
		 * yet, i.e. the start of the write is sector aligned.
		 */
		end = old_pos + count;
		if ((end & (fd->geo_sector_size - 1)) &&
				((to_write > fd->geo_sector_size) || !ofs)) {
			i = ntfs_device_win32_pread_simple(fd,
					end & ~(s64)(fd->geo_sector_size - 1),
					fd->geo_sector_size, readbuffer +
					to_write - fd->geo_sector_size);
			if (i != fd->geo_sector_size) {
				if (i >= 0)
					errno = EIO;
				goto write_error;
			}
		}
		/* Copy the data to be written into @readbuffer. */
		memcpy(readbuffer + ofs, b, count);
		alignedbuffer = readbuffer;
	}
	if (fd->vol_handle != INVALID_HANDLE_VALUE && old_pos < fd->geo_size) {
		s64 vol_to_write = fd->geo_size - old_pos;
		if (count > vol_to_write) {
			bw = ntfs_device_win32_pwrite_simple(fd,
					old_pos & ~(s64)(fd->geo_sector_size - 1),
					ofs + vol_to_write, alignedbuffer);
			if (bw == -1)
				goto write_error;
			to_write -= bw;
			if (bw < ofs) {
				bw = 0;
				goto write_partial;
			}
			bw -= ofs;
			fd->pos += bw;
			ofs = fd->pos & (fd->geo_sector_size - 1);
			if (bw != vol_to_write)
				goto write_partial;
		}
	}
	i = ntfs_device_win32_pwrite_simple(fd,
			fd->pos & ~(s64)(fd->geo_sector_size - 1), to_write,
			alignedbuffer + bw);
	if (i == -1) {
		if (bw)
			goto write_partial;
		goto write_error;
	}
	if (i < ofs)
		goto write_partial;
	i -= ofs;
	bw += i;
	if (bw > count)
		bw = count;
	fd->pos = old_pos + bw;
write_partial:
	if (readbuffer)
		VirtualFree(readbuffer, 0, MEM_RELEASE);
	return bw;
write_error:
	bw = -1;
	goto write_partial;
}

/**
 * ntfs_device_win32_stat - get a unix-like stat structure for an ntfs device
 * @dev:	ntfs device obtained via ->open
 * @buf:	pointer to the stat structure to fill
 *
 * Note: Only st_mode, st_size, and st_blocks are filled.
 *
 * Return 0 if o.k.
 *	 -1 if not and errno set. in this case handle is trashed.
 */
static int ntfs_device_win32_stat(struct ntfs_device *dev, struct stat *buf)
{
	win32_fd *fd = (win32_fd *)dev->d_private;
	mode_t st_mode;

	if ((dev->d_name[1] == ':') && (dev->d_name[2] == '\0'))
		st_mode = S_IFBLK;
	else
		switch (GetFileType(fd->handle)) {
		case FILE_TYPE_CHAR:
			st_mode = S_IFCHR;
			break;
		case FILE_TYPE_DISK:
			st_mode = S_IFREG;
			break;
		case FILE_TYPE_PIPE:
			st_mode = S_IFIFO;
			break;
		default:
			st_mode = 0;
		}
	memset(buf, 0, sizeof(struct stat));
	buf->st_mode = st_mode;
	buf->st_size = fd->part_length;
	if (buf->st_size != -1)
		buf->st_blocks = buf->st_size >> 9;
	else
		buf->st_size = 0;
	return 0;
}

#ifdef HDIO_GETGEO
/**
 * ntfs_win32_hdio_getgeo - get drive geometry
 * @dev:	ntfs device obtained via ->open
 * @argp:	pointer to where to put the output
 *
 * Note: Works on windows NT/2k/XP only.
 *
 * Return 0 if o.k.
 *	 -1 if not, and errno set.  Note if error fd->handle is trashed.
 */
static __inline__ int ntfs_win32_hdio_getgeo(struct ntfs_device *dev,
		struct hd_geometry *argp)
{
	win32_fd *fd = (win32_fd *)dev->d_private;

	argp->heads = fd->geo_heads;
	argp->sectors = fd->geo_sectors;
	argp->cylinders = fd->geo_cylinders;
	argp->start = fd->part_hidden_sectors;
	return 0;
}
#endif

/**
 * ntfs_win32_blksszget - get block device sector size
 * @dev:	ntfs device obtained via ->open
 * @argp:	pointer to where to put the output
 *
 * Note: Works on windows NT/2k/XP only.
 *
 * Return 0 if o.k.
 *	 -1 if not, and errno set.  Note if error fd->handle is trashed.
 */
static __inline__ int ntfs_win32_blksszget(struct ntfs_device *dev,int *argp)
{
	win32_fd *fd = (win32_fd *)dev->d_private;
	DWORD bytesReturned;
	DISK_GEOMETRY dg;

	if (DeviceIoControl(fd->handle, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0,
			&dg, sizeof(DISK_GEOMETRY), &bytesReturned, NULL)) {
		/* success */
		*argp = dg.BytesPerSector;
		return 0;
	}
	errno = ntfs_w32error_to_errno(GetLastError());
	ntfs_log_trace("GET_DRIVE_GEOMETRY failed.\n");
	return -1;
}

static int ntfs_device_win32_ioctl(struct ntfs_device *dev,
		unsigned long request, void *argp)
{
#if defined(BLKGETSIZE) | defined(BLKGETSIZE64)
	win32_fd *fd = (win32_fd *)dev->d_private;
#endif

	ntfs_log_trace("win32_ioctl(0x%lx) called.\n", request);
	switch (request) {
#if defined(BLKGETSIZE)
	case BLKGETSIZE:
		ntfs_log_debug("BLKGETSIZE detected.\n");
		if (fd->part_length >= 0) {
			*(int *)argp = (int)(fd->part_length / 512);
			return 0;
		}
		errno = EOPNOTSUPP;
		return -1;
#endif
#if defined(BLKGETSIZE64)
	case BLKGETSIZE64:
		ntfs_log_debug("BLKGETSIZE64 detected.\n");
		if (fd->part_length >= 0) {
			*(s64 *)argp = fd->part_length;
			return 0;
		}
		errno = EOPNOTSUPP;
		return -1;
#endif
#ifdef HDIO_GETGEO
	case HDIO_GETGEO:
		ntfs_log_debug("HDIO_GETGEO detected.\n");
		return ntfs_win32_hdio_getgeo(dev, (struct hd_geometry *)argp);
#endif
#ifdef BLKSSZGET
	case BLKSSZGET:
		ntfs_log_debug("BLKSSZGET detected.\n");
		if (fd && !fd->ntdll) {
			*(int*)argp = fd->geo_sector_size;
			return (0);
		} else
			return ntfs_win32_blksszget(dev, (int *)argp);
#endif
#ifdef BLKBSZSET
	case BLKBSZSET:
		ntfs_log_debug("BLKBSZSET detected.\n");
		/* Nothing to do on Windows. */
		return 0;
#endif
	default:
		ntfs_log_debug("unimplemented ioctl 0x%lx.\n", request);
		errno = EOPNOTSUPP;
		return -1;
	}
}

static s64 ntfs_device_win32_pread(struct ntfs_device *dev, void *b,
		s64 count, s64 offset)
{
	s64 got;
	win32_fd *fd;

		/* read the fast way if sector aligned */
	fd = (win32_fd*)dev->d_private;
	if (!((count | offset) & (fd->geo_sector_size - 1))) {
		got = ntfs_device_win32_pio(fd, offset, count, b, (void*)NULL);
	} else {
		if (ntfs_device_win32_seek(dev, offset, 0) == -1)
			got = 0;
		else
			got = ntfs_device_win32_read(dev, b, count);
	}

	return (got);
}

static s64 ntfs_device_win32_pwrite(struct ntfs_device *dev, const void *b,
		s64 count, s64 offset)
{
	s64 put;
	win32_fd *fd;

		/* write the fast way if sector aligned */
	fd = (win32_fd*)dev->d_private;
	if (!((count | offset) & (fd->geo_sector_size - 1))) {
		put = ntfs_device_win32_pio(fd, offset, count, (void*)NULL, b);
	} else {
		if (ntfs_device_win32_seek(dev, offset, 0) == -1)
			put = 0;
		else
			put = ntfs_device_win32_write(dev, b, count);
	}
	return (put);
}

struct ntfs_device_operations ntfs_device_win32_io_ops = {
	.open		= ntfs_device_win32_open,
	.close		= ntfs_device_win32_close,
	.seek		= ntfs_device_win32_seek,
	.read		= ntfs_device_win32_read,
	.write		= ntfs_device_win32_write,
	.pread		= ntfs_device_win32_pread,
	.pwrite		= ntfs_device_win32_pwrite,
	.sync		= ntfs_device_win32_sync,
	.stat		= ntfs_device_win32_stat,
	.ioctl		= ntfs_device_win32_ioctl
};

/*
 *			Mark an open file as sparse
 *
 *	This is only called by ntfsclone when cloning a volume to a file.
 *	The argument is the target file, not a volume.
 *
 *	Returns 0 if successful.
 */

int ntfs_win32_set_sparse(int fd)
{
	BOOL ok;
	HANDLE handle;
	DWORD bytes;   

	handle = get_osfhandle(fd);
	if (handle == INVALID_HANDLE_VALUE)
		ok = FALSE;
	else
		ok = DeviceIoControl(handle, FSCTL_SET_SPARSE,
				(void*)NULL, 0, (void*)NULL, 0,
				&bytes, (LPOVERLAPPED)NULL);
	return (!ok);
}

/*
 *			Resize an open file
 *
 *	This is only called by ntfsclone when cloning a volume to a file.
 *	The argument must designate a file, not a volume.
 *
 *	Returns 0 if successful.
 */

static int win32_ftruncate(HANDLE handle, s64 size)
{
	BOOL ok;
	LONG hsize, lsize;
	LONG ohsize, olsize;

	if (handle == INVALID_HANDLE_VALUE)
		ok = FALSE;
	else {
		SetLastError(NO_ERROR);
			/* save original position */
		ohsize = 0;
		olsize = SetFilePointer(handle, 0, &ohsize, 1);
		hsize = size >> 32;
		lsize = size & 0xffffffff;
		ok = (SetFilePointer(handle, lsize, &hsize, 0) == (DWORD)lsize)
		    && (GetLastError() == NO_ERROR)
		    && SetEndOfFile(handle);
			/* restore original position, even if above failed */
		SetFilePointer(handle, olsize, &ohsize, 0);
		if (GetLastError() != NO_ERROR)
			ok = FALSE;
	}
	if (!ok)
		errno = EINVAL;
	return (ok ? 0 : -1);
}

int ntfs_device_win32_ftruncate(struct ntfs_device *dev, s64 size)
{
	win32_fd *fd;
	int ret;

	ret = -1;
	fd = (win32_fd*)dev->d_private;
	if (fd && !fd->ntdll)
		ret = win32_ftruncate(fd->handle, size);
	return (ret);
}

int ntfs_win32_ftruncate(int fd, s64 size)
{
	int ret;
	HANDLE handle;

	handle = get_osfhandle(fd);
	ret = win32_ftruncate(handle, size);
	return (ret);
}
