/* FILE-CSTYLED */

//
// FILE FUNCTION: BTCEAMP11 Driver Definitions
//
// REVISION HISTORY:
//
// DATE     NAME            REASON
// -------  --------------  ----------------
// 17Sep03  Ilya Faenson    Create original
//
// Copyright (c) 2003 WidComm, Inc.
//
#ifndef _BTCEAMP11_
#define _BTCEAMP11_

#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include <winioctl.h>
#include <pkfuncs.h>
#include <ndis.h>

// Interface with the BTKRNL
#include "btkrnl_dev_ioctl_ce.h"

// Our own external definitions
#include "btceamp11_ext.h"

// H4 Packet types
#define H4_HCI_COMMAND    0x01
#define H4_HCI_ACL        0x02
#define H4_HCI_SCO        0x03
#define H4_HCI_EVENT      0x04

extern void LogIntoFile(const char *fmt_str, ...);
extern void LogViaStack(const char *fmt_str, ...);
extern void LogViaStackRB(const char *fmt_str, ...);

#ifdef DEBUG
#undef KdPrint
#define KdPrint LogIntoFile
#else
#undef KdPrint
#define KdPrint
#endif

#define BTWUSB_MAXIMUM_TRANSFER_SIZE 1500

//
// A structure representing the instance information associated with
// this particular device.
//
typedef struct _DEVICE_EXTENSION
    {
    NDIS_HANDLE NdisDeviceHandle;

    // BTKRNL has been closed
    BOOLEAN KrnlDown;

    // Device is being removed
    BOOLEAN alreadyRemoved;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#ifndef UINT16_TO_STREAM
#define UINT16_TO_STREAM(p, u16) {*(p)++ = (UINT8)(u16); *(p)++ = (UINT8)((u16) >> 8);}
#endif
#ifndef UINT8_TO_STREAM
#define UINT8_TO_STREAM(p, u8)   {*(p)++ = (UINT8)(u8);}
#endif

// Shared variables
extern BOOLEAN BtKernFound;  // BTKERN is loaded flag
extern NDIS_HANDLE wl_wrapper_handle;  // miniport wrapper handle


// Global functions
int FindBtKern (void);
int SendDeviceIoControl(ULONG IoctlCode, PVOID InputBuffer,
    ULONG SizeOfInputBuffer, PVOID OutputBuffer, ULONG SizeOfOutputBuffer,
    PULONG pBytesReturned);
void CloseBtKern (void);
VOID BtKernStart (void *Context);
VOID BtKernStop (void *Context);
VOID BtKernForwardEvent (void *Context, PVOID buffer, ULONG len);
VOID BtKernForwardData (void *Context, PVOID buffer, ULONG len);

// NDIS Bluetooth start/stop callbacks
NTSTATUS BtInit (void *Context);
void BtHalt (void *Context);

#endif // _BTCEAMP11_
