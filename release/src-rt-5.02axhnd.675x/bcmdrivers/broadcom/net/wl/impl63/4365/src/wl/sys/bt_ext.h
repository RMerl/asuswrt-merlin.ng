/* FILE-CSTYLED */

//
// FILE FUNCTION: BTW11P External Header
//
// REVISION HISTORY:
//
// DATE     NAME            REASON
// -------  ------------    ------------
// 17Mar08  Ilya Faenson    Create original
//
// Copyright (c) 2008 Broadcom Corp.
//

#ifndef __bt_ext_h__
#define __bt_ext_h__

//
// BTWUSB Stats
//
typedef struct BTWUSBStats
{
    // Number of Read IRPs submitted to the USB software stack
    unsigned long reads_submitted;

    // Number of Read IRP completions from the USB software stack
    unsigned long reads_completed;

    // Number of Read IRP completions in error
    unsigned long reads_completed_error;

    // Read frames discarded due to no buffers
    unsigned long reads_disc_nobuf;

    // Bytes received
    unsigned long reads_bytes;

    // Read buffers filled
    unsigned long reads_filled;

    // Number of Event IRPs submitted to the USB software stack
    unsigned long events_submitted;

    // Number of Event IRP completions from the USB software stack
    unsigned long events_completed;

    // Number of Event IRP completions in error
    unsigned long events_completed_error;

    // Event frames discarded due to no buffers
    unsigned long events_disc_nobuf;

    // Bytes received
    unsigned long events_bytes;

    // Event buffers filled
    unsigned long events_filled;

    // Number of Write IRPs submitted
    unsigned long writes_submitted;

    // Number of Write IRPs completed
    unsigned long writes_completed;

    // Number of Write IRPs completed in error
    unsigned long writes_completed_error;

    // Number of Write time-outs
    unsigned long writes_timeout;

    // Number of Writes not submitted due to no room on the tx queue
    unsigned long writes_disc_nobuf;

    // Number of Writes not submitted due to too long data
    unsigned long writes_disc_toolong;

    // Number of Command IRPs submitted
    unsigned long commands_submitted;

    // Number of Command IRPs completed
    unsigned long commands_completed;

    // Number of Command IRPs completed in error
    unsigned long commands_completed_error;

    // Number of Command time-outs
    unsigned long commands_timeout;

    // Number of Commands not submitted due to no room on the tx queue
    unsigned long commands_disc_nobuf;

    // Number of Commands not submitted due to too long data
    unsigned long commands_disc_toolong;

    // Number of configuration request time-outs
    unsigned long configs_timeout;

    // Number of voice IRPs submitted to the USB software stack
    unsigned long voicerx_submitted;

    // Number of voice IRP completions from the USB software stack
    unsigned long voicerx_completed;

    // Number of Voice IRP completions in error
    unsigned long voicerx_completed_error;

    // Voice frames discarded due to no buffers
    unsigned long voicerx_disc_nobuf;

    // Voice frames discarded due to no headers in data
    unsigned long voicerx_disc_nohdr;

    // Voice frames discarded due to no headers in the body of data
    // (data is in sync, i.e. the whole chunk starts from the right header)
    unsigned long voicerx_disc_nobodyhdr;

    // Voice frames discarded due to no headers in the body of data
    // (data is not in sync, i.e. the whole chunk does not start from the right header)
    unsigned long voicerx_disc_nobodyhdr2;

    // Voice frames discarded due to no headers in the tail of data
    // being saved after every aligned has been forwarded (data was in sync)
    unsigned long voicerx_disc_notailhdr;

    // Voice frames discarded due to no headers in the tail of data
    // being saved after every aligned has been forwarded (data was not in sync)
    unsigned long voicerx_disc_notailhdr2;

    // Voice rx completions running in parallel
    unsigned long voicerx_parallel_dpcs;

    // Bytes received
    unsigned long voicerx_bytes;

    // Event buffers filled
    unsigned long voicerx_filled;

    // Number of Voice Tx IRPs submitted
    unsigned long voicetx_submitted;

    // Number of Voice Tx IRPs completed
    unsigned long voicetx_completed;

    // Number of Voice Tx IRPs completed in error
    unsigned long voicetx_completed_error;

    // Number of Voice Tx time-outs
    unsigned long voicetx_timeout;

    // Number of Voice Tx not submitted due to no room on the tx queue
    unsigned long voicetx_disc_nobuf;

    // Number of Writes not submitted due to too long data
    unsigned long voicetx_disc_toolong;

    // Last error reported by IRP
    unsigned long last_irp_error;

    // Last error reported by URB
    unsigned long last_urb_error;

    // Idle timeouts triggered
    unsigned long idle_timeout;

    // Idle callbacks called by the USBHUB
    unsigned long idle_callback;

    // Idle IRP completions
    unsigned long idle_completion;

    // Idle IRP cancellations
    unsigned long idle_cancellation;

    // Number of Read Diag IRPs submitted to the USB software stack
    unsigned long diags_submitted;

    // Number of Read Diag IRP completions from the USB software stack
    unsigned long diags_completed;

    // Number of Read Diag IRP completions in error
    unsigned long diags_completed_error;

    // Read diag frames discarded due to no buffers
    unsigned long diags_disc_nobuf;

    // Diag Bytes received
    unsigned long diags_bytes;

    // Read Diag buffers filled
    unsigned long diags_filled;

    // Number of Write Diag IRPs submitted
    unsigned long diag_writes_submitted;

    // Number of Write Diag IRPs completed
    unsigned long diag_writes_completed;

    // Number of Write Diag IRPs completed in error
    unsigned long diag_writes_completed_error;

    // Number of Write Diag time-outs
    unsigned long diag_writes_timeout;

    // Number of Diag Writes not submitted due to no room on the tx queue
    unsigned long diag_writes_disc_nobuf;

    // Number of Diag Writes not submitted due to too long data
    unsigned long diag_writes_disc_toolong;

} BTWUSBStats, *pBTWUSBStats;

//
// BTWUSB Status
//
// NOTE: All fields happen to be ULONGs to avoid the
// alignment issues between apps and the driver...
//
typedef struct BTWUSBStatus
{
    // Number of voice packets currently queued towards the USB stack
    unsigned long voicetxUsbPktsOutstanding;

    // Remote wakeup enabled / disabled flag
    unsigned long WakeupEnabled;

    // Idle suspend enabled flag
    unsigned long IdleSuspendEnabled;

    // We're selectively suspended...
    unsigned long deviceSelSuspended;

    // BTKRNL has been closed
    unsigned long KrnlDown;

    // Application has requested exclusive use of the driver
    // (No BTKRNL interface)
    unsigned long appExclusive;

    // Device is being reset
    unsigned long ResetInProgress;

    // Device is being removed
    unsigned long alreadyRemoved;

    // Number of voice channels opened
    unsigned long NumVoiceChannels;

    // Number of voice interface setting
    unsigned long VoiceSettingNum;

    // Max voice packet size
    unsigned long VoicePktSize;
} BTWUSBStatus, *pBTWUSBStatus;

//
//              IOCTL Definitions
// Note: this file depends on the file DEVIOCTL.H which contains
// the macro definition for "CTL_CODE" below.  Include that file
// before  you include this one in your source code.

// The base of the IOCTL control codes.
#define BTWUSB_IOCTL_INDEX  0x089a

#define IOCTL_BTWUSB_GET_PIPE_INFO     CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GET_DEVICE_DESCRIPTOR CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+1,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GET_CONFIGURATION_DESCRIPTOR CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+2,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_PUT CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+3,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_PUT_CMD CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+4,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GET CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+5,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GET_EVENT CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+6,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GET_STATS CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+7,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_CLEAR_STATS CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+8,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GET_VERSION CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+9,METHOD_BUFFERED, FILE_ANY_ACCESS)

// TMP!!
#define IOCTL_BTWUSB_CANCEL_READ CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0xA,METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BTWUSB_SUBMIT_READ CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0xB,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_RESET_PORT CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0xC,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GET_IDS CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0xD,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GO_DFU CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0xE,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_UNCOND_SUSPEND CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0xF,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_COND_SUSPEND CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x10,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GET_HARDWARE_ID CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x11,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GET_DEVICE_DESC CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x12,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_SET_VOICE CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x13,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_PUT_VOICE CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x14,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GET_VOICE CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x15,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_SET_EXCLUSIVE_ACCESS CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x16,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_RESET_EXCLUSIVE_ACCESS CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x17,METHOD_BUFFERED, FILE_ANY_ACCESS)

// #define IOCTL_BTWUSB_SET_VOICE_16BIT CTL_CODE(FILE_DEVICE_UNKNOWN,  \
//    BTWUSB_IOCTL_INDEX+0x18,METHOD_BUFFERED, FILE_ANY_ACCESS)
//
// #define IOCTL_BTWUSB_SET_VOICE_8BIT CTL_CODE(FILE_DEVICE_UNKNOWN,  \
//    BTWUSB_IOCTL_INDEX+0x19,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_ADD_VOICE_CHANNEL CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x1A,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_REMOVE_VOICE_CHANNEL CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x1B,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_ENTER_SEL_SUSPEND CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x1C,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_CANCEL_SEL_SUSPEND CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x1D,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GET_STATUS CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x1E,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_SET_REMOTE_WAKEUP_IN_SUSPEND CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x1F,METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// Broadcom diagnostic interface related requests
//
#define IOCTL_BTWUSB_IS_BRCM_DIAG_AVAILABLE CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x20,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_PUT_DIAG CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x21,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GET_DIAG CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x22,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_SET_PRIORITY CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x23,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GET_PRIORITY CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x24,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GET_SERIAL_NUMBER CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x25,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BTWUSB_GET_PDO_NAME CTL_CODE(FILE_DEVICE_UNKNOWN,  \
    BTWUSB_IOCTL_INDEX+0x26,METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif /* __bt_ext_h__ */
