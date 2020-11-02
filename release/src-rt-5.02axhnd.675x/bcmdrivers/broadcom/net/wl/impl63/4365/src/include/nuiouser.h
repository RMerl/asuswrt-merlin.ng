/*++

Copyright (c) 2000  Microsoft Corporation

Module Name:

    nuiouser.h

Abstract:

    Constants and types to access the NDISUIO driver.
    Users must also include ntddndis.h

Environment:

    User/Kernel mode.

Revision History:

    arvindm     4/12/2000    Created

--*/

/*FILE-CSTYLED*/

#ifndef __NUIOUSER__H
#define __NUIOUSER__H

#define FSCTL_NDISUIO_BASE      FILE_DEVICE_NETWORK

#define _NDISUIO_CTL_CODE(_Function, _Method, _Access)  \
            CTL_CODE(FSCTL_NDISUIO_BASE, _Function, _Method, _Access)

#define IOCTL_NDISUIO_OPEN_DEVICE   \
            _NDISUIO_CTL_CODE(0x200, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_NDISUIO_QUERY_OID_VALUE   \
            _NDISUIO_CTL_CODE(0x201, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_NDISUIO_SET_OID_VALUE   \
            _NDISUIO_CTL_CODE(0x205, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_NDISUIO_SET_ETHER_TYPE   \
            _NDISUIO_CTL_CODE(0x202, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_NDISUIO_QUERY_BINDING   \
            _NDISUIO_CTL_CODE(0x203, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_NDISUIO_BIND_WAIT   \
            _NDISUIO_CTL_CODE(0x204, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
//  Structure to go with IOCTL_NDISUIO_QUERY_OID_VALUE.
//  The Data part is of variable length, determined by
//  the input buffer length passed to DeviceIoControl.
//
typedef struct _NDISUIO_QUERY_OID
{
    NDIS_OID        Oid;
    UCHAR           Data[sizeof(ULONG)];

} NDISUIO_QUERY_OID, *PNDISUIO_QUERY_OID;

//
//  Structure to go with IOCTL_NDISUIO_SET_OID_VALUE.
//  The Data part is of variable length, determined
//  by the input buffer length passed to DeviceIoControl.
//
typedef struct _NDISUIO_SET_OID
{
    NDIS_OID        Oid;
    UCHAR           Data[sizeof(ULONG)];

} NDISUIO_SET_OID, *PNDISUIO_SET_OID;

//
//  Structure to go with IOCTL_NDISUIO_QUERY_BINDING.
//  The input parameter is BindingIndex, which is the
//  index into the list of bindings active at the driver.
//  On successful completion, we get back a device name
//  and a device descriptor (friendly name).
//
typedef struct _NDISUIO_QUERY_BINDING
{
	ULONG			BindingIndex;		// 0-based binding number
	ULONG			DeviceNameOffset;	// from start of this struct
	ULONG			DeviceNameLength;	// in bytes
	ULONG			DeviceDescrOffset;	// from start of this struct
	ULONG			DeviceDescrLength;	// in bytes

} NDISUIO_QUERY_BINDING, *PNDISUIO_QUERY_BINDING;

#endif // __NUIOUSER__H
