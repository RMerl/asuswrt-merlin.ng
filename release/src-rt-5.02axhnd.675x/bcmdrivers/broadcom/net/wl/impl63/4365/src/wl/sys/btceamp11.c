/* FILE-CSTYLED */

//
// FILE FUNCTION: BTCEAMP11 WiFi Main stub
//
// REVISION HISTORY:
//
// DATE     NAME            REASON
// -------  ------------    ------------
// 11Sep08  Ilya Faenson    Create original
//
// Copyright (c) 2008 Broadcom
//

//
// header files to make compiler happy...
//

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <oidencap.h>
#include <wlioctl.h>
#include <siutils.h>
#include <bcmendian.h>
#include <sbconfig.h>
#include <nicpci.h>
#ifdef DHD_SPROM
#include <bcmsrom.h>
#endif /* DHD_SPROM */
#include <proto/802.11.h>
#include <proto/802.1d.h>
#include <proto/bcmip.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_ndis.h>

// All the relevant includes
#include "btceamp11.h"

// BTKRNL handle and flag
HANDLE hStack = INVALID_HANDLE_VALUE;
BOOLEAN BtKernFound = FALSE;

static DWORD amp_inx = 0;

//
// Trace into the file only
//
void LogIntoFile (const char *fmt_str, ...)
{
    char        buf[256];
    TCHAR       tbuf[256];
    va_list     marker;
    HANDLE      hFile;
    int         len;
    DWORD       bytesWritten;

    // Add a time stamp in milliseconds
    len = sprintf (buf, "WLAN [%08d]: ", GetTickCount ());

    // Format the message
    va_start (marker, fmt_str);
    len += vsnprintf (buf + len, sizeof(buf) - len, fmt_str, marker);
    va_end  (marker);

    // Place \0 at last position -2 as a safety to prevent overun for array buf writing \r\n
    buf[253] = '\0';
    strcat (buf, "\r\n");

    MultiByteToWideChar (CP_ACP, 0, buf, -1, tbuf, 256);

    hFile = CreateFile(TEXT("usblog.txt"), GENERIC_WRITE, 0,
                        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    SetFilePointer(hFile, 0, NULL, FILE_END);       /* append! */
    WriteFile(hFile, tbuf, wcslen(tbuf) * 2, &bytesWritten, NULL);
    CloseHandle(hFile);
}

//
// Trace through the stack
//
void LogViaStack (const char *fmt_str, ...)
{
    char        buf[256];
    va_list     marker;
    int         len;
    DWORD       bytesWritten;
    int		ret;

    // Add a time stamp in milliseconds
    len = sprintf (buf, "WLAN [%08d]: ", GetTickCount());

    // Format the message
    va_start (marker, fmt_str);
    len += vsnprintf (buf + len, sizeof(buf) - len, fmt_str, marker);
    va_end  (marker);

    ret = SendDeviceIoControl (BTKRNL_IOCTL_DEVICE_LOG, buf, len + 1, NULL, 0, &bytesWritten);
    if (ret != 0)
    {
        KdPrint(buf);
    }
}

//
// Trace through the stack (into a Ring Buffer)
//
void LogViaStackRB (const char *fmt_str, ...)
{
    char        buf[256];
    va_list     marker;
    int         len;
    DWORD       bytesWritten;
    int		ret;

    // Format the message
    va_start (marker, fmt_str);
    len = vsnprintf (buf, sizeof(buf), fmt_str, marker);
    va_end  (marker);

    ret = SendDeviceIoControl (BTKRNL_IOCTL_DEVICE_LOG_RB, buf, len + 1, NULL, 0, &bytesWritten);
    if (ret != 0)
    {
        KdPrint(buf);
    }
}

//
// Bluetooth init
//
// Will really be called with a WiFi context as a parameter:
//    wl_info_t *wl = (wl_info_t *) Context;
//    PDEVICE_EXTENSION pdx = &wl->bt_ext;
//
NTSTATUS BtInit (void *Context)
{
    wl_info_t *wl = (wl_info_t *) Context;
    PDEVICE_EXTENSION pdx = &wl->bt_ext;

    pdx->alreadyRemoved = FALSE;
    pdx->KrnlDown = TRUE;

    BtKernStart(Context);
    return STATUS_SUCCESS;
}
//
// Bluetooth Halt
//
// Will really be called with a WiFi context as a parameter:
//    wl_info_t *wl = (wl_info_t *) Context;
//    PDEVICE_EXTENSION pdx = &wl->bt_ext;
//
// The "wl" parameter will be stored as global...
//
void BtHalt (void *Context)
{
    wl_info_t *wl = (wl_info_t *) Context;
    PDEVICE_EXTENSION pdx = &wl->bt_ext;

    BtKernStop(Context);

    pdx->alreadyRemoved = TRUE;
    pdx->KrnlDown = TRUE;

    return;
}

//
// Returns TRUE if BTKRNL is open and
// FALSE otherwise.
//
int FindBtKern (void)
{
    // Open already?
    if (hStack != INVALID_HANDLE_VALUE)
        return TRUE;

    hStack = CreateFile (_T("BTS1:"), GENERIC_READ | GENERIC_WRITE,  0, NULL, OPEN_EXISTING, 0, NULL);

    if (hStack == INVALID_HANDLE_VALUE)
    {
        KdPrint("BT stack open failed with status %d!", GetLastError());
        return FALSE;
    }

    KdPrint("BT stack opened OK, handle: 0x%08x", hStack);

    // Found it!
    return TRUE;
}

//
// Dereference BTKRNL
//
void CloseBtKern (void)
{
    // Nothing to do if closed already...
    if (hStack == INVALID_HANDLE_VALUE)
        return;

    // Close the driver...
    CloseHandle (hStack);
    hStack = INVALID_HANDLE_VALUE;
    return;
}

//
// SendDeviceIoControl
//
// Returns 0 if successful
//
int SendDeviceIoControl(ULONG IoctlCode, PVOID Buffer, ULONG SizeOfInputBuffer,
                        PVOID OutBuffer, ULONG SizeOfOutputBuffer, PULONG pBytesReturned)
{
    DWORD   BytesRet;
    int     retVal = 0;

    if (!pBytesReturned)
        pBytesReturned = &BytesRet;

    // Stack has got to be open already...
    if (hStack == INVALID_HANDLE_VALUE)
    {
        if (!FindBtKern())
            return 12;
    }

    if (!DeviceIoControl (hStack, IoctlCode, Buffer, SizeOfInputBuffer,
        OutBuffer, SizeOfOutputBuffer, pBytesReturned, NULL))
    {
//        KdPrint("The stack DeviceIoControl failed with the status of %d!", GetLastError());
        return 11;
    }

    return 0;
}

//
// BTKRNL Data Write Callback
//
int __cdecl BTCEAMP11_SubmitWrite (void *Context, void *buffer, unsigned long length)
{
    wl_info_t *wl = (wl_info_t *) Context;
    PDEVICE_EXTENSION pdx = &wl->bt_ext;

    // Don't submit anything if we're removing the device
    if (!pdx || pdx->alreadyRemoved)
        {
        KdPrint ("BTCEAMP11_SubmitWrite discarded, no device");
        return FALSE;
        }

    // To be added at the WiFi integration time...
    if (wl->wlc->bta)
        {
//        KdPrint ("BTCEAMP11_SubmitWrite with %u bytes", length);
        wlc_bta_tx_hcidata(wl->wlc->bta, buffer, (uint)length);
        return TRUE;
        }

    return FALSE;
}

//
// BTKRNL Command Write Callback
//
int __cdecl BTCEAMP11_SubmitCmd (void *Context, void *buffer, unsigned long length)
{
    wl_info_t *wl = (wl_info_t *) Context;
    PDEVICE_EXTENSION pdx = &wl->bt_ext;

    // Don't submit anything if we're removing the device
    if (!pdx || pdx->alreadyRemoved)
        {
        KdPrint ("BTCEAMP11_SubmitCmd discarded, no device");
        return FALSE;
        }

    // To be added at the WiFi integration time...
    if (wl->wlc->bta)
        {
//        KdPrint ("BTCEAMP11_SubmitCmd with %u bytes", length);
        wlc_bta_docmd(wl->wlc->bta, buffer, (uint)length);
        return TRUE;
        }

    return FALSE;
}

//
// BTKRNL Start Processing
//
VOID BtKernStart (void *Context)
{
    wl_info_t *wl = (wl_info_t *) Context;
    PDEVICE_EXTENSION pdx = &wl->bt_ext;
    BTDEVICE_PARS_EX pars;
    ULONG bytes_ret = 0;

    // If BTKRNL is not around, try finding it again...
    if (!BtKernFound)
        {
        BtKernFound = (FindBtKern ()) ? TRUE : FALSE;
        }
    if (!BtKernFound)
        {
        KdPrint("BtKernStart: BTKRNL does not seem to be loaded...");
        return;
        }
    if (!pdx->KrnlDown)
        {
        KdPrint("BtKernStart: BTKRNL is open already...");
        return;
        }

    // Fill in control structure for the START IoControl
    memset (&pars, 0, sizeof (pars));

    pars.DeviceHandle         = Context;
    pars.DeviceType           = 10; // BTDEVICE_TYPE_AMP;
    pars.DeviceNumber         = 0;
    pars.BtDeviceSendDataFunc = BTCEAMP11_SubmitWrite;
    pars.BtDeviceSendCmdFunc  = BTCEAMP11_SubmitCmd;

    strcpy (pars.DeviceDescription, "BTCEAMP11");

    // Call BTKRNL
    if (SendDeviceIoControl(BTKRNL_IOCTL_DEVICE_START, &pars, sizeof (pars),
                            &amp_inx, sizeof (amp_inx), &bytes_ret))
    {
        KdPrint("BtKernStart: Failure calling BTKRNL to start AMP...");
    }
    else
    {
        pdx->KrnlDown = FALSE;

        // About it...
        KdPrint("BtKernStart for 0x%x device completed amp_inx %u", Context, amp_inx);
    }
    return;
}

//
// Stop Processing...
//
VOID BtKernStop (void *Context)
{
    wl_info_t *wl = (wl_info_t *) Context;
    PDEVICE_EXTENSION pdx = &wl->bt_ext;
    ULONG bytes_ret = 0;

    if (!BtKernFound)
        {
        KdPrint("BtKernStop: BTKRNL does not seem to be loaded...");
        return;
        }
    if (pdx->KrnlDown)
        {
        KdPrint("BtKernStop: BTKRNL is stopped already...");
        return;
        }
    pdx->KrnlDown = TRUE;

    // Call BTKRNL
    if (SendDeviceIoControl(BTKRNL_IOCTL_DEVICE_WM_AMP_STOP,
                            &amp_inx, sizeof (amp_inx), NULL, 0, &bytes_ret))
    {
        KdPrint("BtKernStop: Failure calling BTKRNL to stop AMP...");
    }

    CloseBtKern ();
    BtKernFound = FALSE;

    // About it...
    KdPrint("BtKernStop for 0x%x device completed amp_inx %u", Context, amp_inx);
    return;
}

//
// BTKRNL Data Forwarding Processing
//
// NOTE: CE version does NOT assumes there's room for 4 bytes
// in front of data!
//
VOID BtKernForwardData (void *Context, PVOID buffer, ULONG len)
{
    ULONG bytes_ret = 0;

    if (!BtKernFound)
        {
        KdPrint("BTKRNL does not seem to be loaded...");
        return;
        }

//    logf(" WLAN->BT: data len %u", len);
    // Call BTKRNL
    if (SendDeviceIoControl(BTKRNL_IOCTL_DEVICE_WM_AMP_DATA, buffer, len, NULL, 0, &bytes_ret))
        {
        KdPrint("Failure calling BTKRNL_IOCTL_DEVICE_WM_AMP_DATA");
        }

    // About it...
    return;
}

//
// BTKRNL Event Forwarding Processing
//
// NOTE: CE version does NOT assumes there's room for 4 bytes
// in front of data!
//
VOID BtKernForwardEvent (void *Context, PVOID buffer, ULONG len)
{
    ULONG bytes_ret = 0;

    if (!BtKernFound)
        {
        KdPrint("BTKRNL does not seem to be loaded...");
        return;
        }

    // Call BTKRNL
    if (SendDeviceIoControl(BTKRNL_IOCTL_DEVICE_WM_AMP_EVENT, buffer, len, NULL, 0, &bytes_ret))
        {
        KdPrint("Failure calling BTKRNL_IOCTL_DEVICE_WM_AMP_EVENT");
        }

    // About it...
    return;
}
