/* FILE-CSTYLED */

/*****************************************************************************/
/*                                                                           */
/*  Name:          btkrnl_dev_ioctl.h                                        */
/*                                                                           */
/*  Description:   Device driver to BTKRNL interface definitions             */
/*                                                                           */
/*  Date        Modification                                                 */
/*  ------------------------                                                 */
/* 05/14/00     Satyajit Create                                              */
/*                                                                           */
/*  Copyright (c) 2000, Widcomm Inc., All Rights Reserved.                   */
/*  Widcomm Bluetooth Software. Proprietary and confidential.                */
/*****************************************************************************/

#ifndef __btkrnl_dev_ioctl__h_
#define __btkrnl_dev_ioctl__h_


#ifndef DD_TYPE_BTKRNL
#define DD_TYPE_BTKRNL  FILE_DEVICE_BUS_EXTENDER
#define DD_TYPE_BTKRNL_OLD  60500                       // old proprietary value, need to support from < 3.1 apps
#endif

// Define IOCTL codes
#define IOCTL_BTKRNL_BASE          0x100

// IOCTLs passed from the lower layer drivers to BTKRNL
// This request registers a newly started Bluetooth device instance
// with the protocol stack.  Input buffer holds a BTDEVICE_PARS
// structure, there is no output buffer for conventional Bluetooth
// devices.  AMP output buffer contains the BTKRNL context pointer.
// Returns DEVIOCTL_NOERROR if succeeds.
#define BTKRNL_IOCTL_DEVICE_START        CTL_CODE(DD_TYPE_BTKRNL,     IOCTL_BTKRNL_BASE+0x30, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define BTKRNL_IOCTL_DEVICE_START_OLD    CTL_CODE(DD_TYPE_BTKRNL_OLD, IOCTL_BTKRNL_BASE+0x30, METHOD_BUFFERED, FILE_ANY_ACCESS)

// This request de-registers a previously started Bluetooth device
// instance from the protocol stack.  Input buffer holds a device handle,
// there is no output buffer.  Returns DEVIOCTL_NOERROR if succeeds.
// Called when the device is stopped or removed from the system.
#define BTKRNL_IOCTL_DEVICE_STOP         CTL_CODE(DD_TYPE_BTKRNL,     IOCTL_BTKRNL_BASE+0x34, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define BTKRNL_IOCTL_DEVICE_STOP_OLD     CTL_CODE(DD_TYPE_BTKRNL_OLD, IOCTL_BTKRNL_BASE+0x34, METHOD_BUFFERED, FILE_ANY_ACCESS)

// This request passes Bluetooth data newly received from the dongle
// to the protocol stack.  Input buffer holds device handle followed
// by data itself, there is no output buffer.  Input buffer size is
// an aggregate size of both handle and data.  Returns DEVIOCTL_NOERROR
// if succeeds.  May be called at high execution priority levels.
#define BTKRNL_IOCTL_DEVICE_DATA         CTL_CODE(DD_TYPE_BTKRNL,     IOCTL_BTKRNL_BASE+0x31, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define BTKRNL_IOCTL_DEVICE_DATA_OLD     CTL_CODE(DD_TYPE_BTKRNL_OLD, IOCTL_BTKRNL_BASE+0x31, METHOD_BUFFERED, FILE_ANY_ACCESS)

// This request passes Bluetooth event data newly received from the dongle
// to the protocol stack.  Input buffer holds device handle followed
// by event data itself, there is no output buffer.  Input buffer size is
// an aggregate size of both handle and event data.  Returns DEVIOCTL_NOERROR
// if succeeds.  May be called at high execution priority levels.
#define BTKRNL_IOCTL_DEVICE_EVENT        CTL_CODE(DD_TYPE_BTKRNL,     IOCTL_BTKRNL_BASE+0x32, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define BTKRNL_IOCTL_DEVICE_EVENT_OLD    CTL_CODE(DD_TYPE_BTKRNL_OLD, IOCTL_BTKRNL_BASE+0x32, METHOD_BUFFERED, FILE_ANY_ACCESS)

// This request passes Bluetooth voice packet newly received from the dongle
// to the protocol stack.  Input buffer holds device handle followed
// by data itself, there is no output buffer.  Input buffer size is
// an aggregate size of both handle and data.  Returns DEVIOCTL_NOERROR
// if succeeds.  May be called at high execution priority levels.
#define BTKRNL_IOCTL_DEVICE_VOICE        CTL_CODE(DD_TYPE_BTKRNL,     IOCTL_BTKRNL_BASE+0x35, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define BTKRNL_IOCTL_DEVICE_VOICE_OLD    CTL_CODE(DD_TYPE_BTKRNL_OLD, IOCTL_BTKRNL_BASE+0x35, METHOD_BUFFERED, FILE_ANY_ACCESS)

// This is an indication from the device that it has completed its suspend
// requested by the BTDEVICE_CONTROL_DEVICE_SUSPEND control command
// (WinXP or later USB device only at the moment)
// DWORD input buffer holds the device handle followed by the completion status code (0 if no errors)
#define BTKRNL_IOCTL_DEVICE_SUSPEND_COMPLETED     CTL_CODE(DD_TYPE_BTKRNL,     IOCTL_BTKRNL_BASE+0x36, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define BTKRNL_IOCTL_DEVICE_SUSPEND_COMPLETED_OLD CTL_CODE(DD_TYPE_BTKRNL_OLD, IOCTL_BTKRNL_BASE+0x36, METHOD_BUFFERED, FILE_ANY_ACCESS)

// This is an indication from the device that it has completed its resume
// requested by the BTDEVICE_CONTROL_DEVICE_RESUME control command
// (WinXP or later USB device only at the moment)
// DWORD input buffer holds the device handle followed by the completion status code (0 if no errors)
#define BTKRNL_IOCTL_DEVICE_RESUME_COMPLETED     CTL_CODE(DD_TYPE_BTKRNL,     IOCTL_BTKRNL_BASE+0x37, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define BTKRNL_IOCTL_DEVICE_RESUME_COMPLETED_OLD CTL_CODE(DD_TYPE_BTKRNL_OLD, IOCTL_BTKRNL_BASE+0x37, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// This request passes Bluetooth data newly received either from the
// dongle or from the functional Bluetooth driver to the protocol stack.
// Input buffer holds device handle DWORD followed by a BYTE of one of
// the buffer types below followed by data itself.  Output buffer is
// a DWORD of status.  Possible values are listed below.
// May be called at high execution priority levels.
//
// NOTE: Filters employ this request as opposed to BTKRNL_IOCTL_DEVICE_DATA
// or BTKRNL_IOCTL_DEVICE_EVENT
//
#define BTKRNL_IOCTL_DEVICE_FILTER_DATA     CTL_CODE(DD_TYPE_BTKRNL, IOCTL_BTKRNL_BASE+0x38, METHOD_BUFFERED, FILE_ANY_ACCESS)
// Filter buffer types:
#define BTKRNL_FILTER_BUFFER_TYPE_COMMAND  0x01
#define BTKRNL_FILTER_BUFFER_TYPE_DATAOUT  0x02
#define BTKRNL_FILTER_BUFFER_TYPE_EVENT    0x03
#define BTKRNL_FILTER_BUFFER_TYPE_DATAIN   0x04
// BTKRNL decisions on what to do with the data (stored into an output buffer)
#define BTKRNL_FILTER_FORWARD_DATA         0x00
#define BTKRNL_FILTER_DONOT_FORWARD_DATA   0x01

// This request passes Bluetooth data newly received from the AMP dongle
// to the protocol stack.  Input buffer holds BTKRNL context followed
// by data itself, there is no output buffer.  Input buffer size is
// an aggregate size of both context and data.  Returns DEVIOCTL_NOERROR
// if succeeds.  May be called at high execution priority levels.
#define BTKRNL_IOCTL_DEVICE_DATA_AMP     CTL_CODE(DD_TYPE_BTKRNL,     IOCTL_BTKRNL_BASE+0x39, METHOD_BUFFERED, FILE_ANY_ACCESS)

// This request passes Bluetooth event data newly received from the AMP dongle
// to the protocol stack.  Input buffer holds BTKRNL context followed
// by event data itself, there is no output buffer.  Input buffer size is
// an aggregate size of both context and event data.  Returns DEVIOCTL_NOERROR
// if succeeds.  May be called at high execution priority levels.
#define BTKRNL_IOCTL_DEVICE_EVENT_AMP   CTL_CODE(DD_TYPE_BTKRNL,     IOCTL_BTKRNL_BASE+0x3a, METHOD_BUFFERED, FILE_ANY_ACCESS)

// This request de-registers a previously started AMP device
// instance from the protocol stack.  Input buffer holds a BTKRNL context,
// there is no output buffer.  Returns DEVIOCTL_NOERROR if succeeds.
// Called when the device is stopped or removed from the system.
#define BTKRNL_IOCTL_DEVICE_STOP_AMP    CTL_CODE(DD_TYPE_BTKRNL,     IOCTL_BTKRNL_BASE+0x3b, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define BTDEVICE_SUCCESS         0
#define BTDEVICE_ERROR_RESOURCES 1

#include <pshpack1.h>

//
// Pointer to this structure is expected in the
// BTDEVICE_SEND_CTL_FUNC Control function argument
//
typedef struct _BTDEVICE_CONTROL
{
// voice related requests
#define BTDEVICE_CONTROL_VOICE_ENCODING        1
#define BTDEVICE_CONTROL_VOICE_ADD_CHANNEL     2
#define BTDEVICE_CONTROL_VOICE_REMOVE_CHANNEL  3

// WinXP or later USB selective suspend requests
#define BTDEVICE_CONTROL_DEVICE_SUSPEND        4
#define BTDEVICE_CONTROL_DEVICE_RESUME         5

// Tell USB driver if remote wakeup is required during the S3
#define BTDEVICE_CONTROL_ARM_REMOTE_WAKEUP     6
#define BTDEVICE_CONTROL_DISARM_REMOTE_WAKEUP  7

// notify USB driver that this device is now active
#define BTDEVICE_CONTROL_SET_ACTIVE            8
#define BTDEVICE_CONTROL_SET_NOT_ACTIVE        9

    unsigned long control_type;

    union {

#define VOICE_ALT_SETTING_16BIT_ENCODING   0
#define VOICE_ALT_SETTING_8BIT_ENCODING    1

    unsigned long voice_setting;

#define VOICE_HEADER_SIZE                      3
    unsigned char voice_header[VOICE_HEADER_SIZE];

#define USB_KEEP_READS                     0
#define USB_CANCEL_READS                   1

    unsigned long read_setting;
    } u;
} BTDEVICE_CONTROL, *PBTDEVICE_CONTROL;


typedef int (__cdecl *BTDEVICE_SEND_DATA_FUNC)(void *DeviceHandle,
                                       void *SendBuffer,
                                       unsigned long SendBufferSize);

typedef int (__cdecl *BTDEVICE_SEND_VOICE_FUNC)(void *DeviceHandle,
                                       void *SendBuffer,
                                       unsigned long SendBufferSize);

typedef int (__cdecl *BTDEVICE_SEND_CMD_FUNC)(void *DeviceHandle,
                                      void *SendBuffer,
                                      unsigned long SendBufferSize);

typedef int (__cdecl *BTDEVICE_SEND_CTL_FUNC)(void *DeviceHandle,
                                      void *Control,
                                      unsigned long ControlSize);

typedef int (__cdecl *BTDEVICE_SEND_DATAUP_FUNC)(void *DeviceHandle,
                                       unsigned long DataType,
                                       void *SendBuffer,
                                       unsigned long SendBufferSize);



typedef struct _BTDEVICE_PARS
{
    void *DeviceHandle;  // handle BTKRNL is to use when executing
                         // device's callback(s).  Opaque for BTKRNL,
                         // but will typically point to either device
                         // object or device extension.  Must be unique
                         // within the BT instances since BTKRNL will
                         // use it to identify the source of data

#define BTDEVICE_TYPE_USB     1
#define BTDEVICE_TYPE_PCMCIA  2
#define BTDEVICE_TYPE_SERIAL  3
#define BTDEVICE_TYPE_PCI     4
#define BTDEVICE_TYPE_SLBCSP  5
#define BTDEVICE_TYPE_AMP    10

// OR this with something above if it's a filter for this media type
#define BTDEVICE_TYPE_FILTER  0x10
// OR this if it's a preferred device
#define BTDEVICE_TYPE_PREFERRED 0x20
#define BTDEVICE_TYPE_DISABLED  0x40
    unsigned long DeviceType;  // device type

    unsigned long DeviceNumber;  // number of the device within the
                                 // range supported by this driver
                                 // starting from 0

    // Pointer to the device send data callback
    BTDEVICE_SEND_DATA_FUNC BtDeviceSendDataFunc;

    // Pointer to the device send commands callback
    BTDEVICE_SEND_CMD_FUNC  BtDeviceSendCmdFunc;

    // Pointer to the device send data callback
    BTDEVICE_SEND_VOICE_FUNC BtDeviceSendVoiceFunc;

    // Pointer to the device control callback
    BTDEVICE_SEND_CTL_FUNC BtDeviceSendCtlFunc;

    // Pointer to the send data or event up (to the client) callback
    // (filters only).  Specifies either BTKRNL_FILTER_BUFFER_TYPE_EVENT
    // or BTKRNL_FILTER_BUFFER_TYPE_DATAIN
    BTDEVICE_SEND_DATAUP_FUNC BtDeviceSendUpDataFunc;

    // TBD - chances we better pass a UNICODE description string to
    // assure WinNT/2000 compatibility?
#define BTDEVICE_MAX_DESCRIPTION_SIZE 80
    unsigned char DeviceDescription[BTDEVICE_MAX_DESCRIPTION_SIZE];

} BTDEVICE_PARS, *PBTDEVICE_PARS;

//
// The newer version of this structure
// (most members described above)
//
typedef struct _BTDEVICE_PARS_EX
{
    unsigned long ParsVersion;  // version of this type (currently 1)
    void *DeviceHandle;
    unsigned long DeviceType;
    unsigned long DeviceNumber;
    BTDEVICE_SEND_DATA_FUNC BtDeviceSendDataFunc;
    BTDEVICE_SEND_CMD_FUNC  BtDeviceSendCmdFunc;
    BTDEVICE_SEND_VOICE_FUNC BtDeviceSendVoiceFunc;
    BTDEVICE_SEND_CTL_FUNC BtDeviceSendCtlFunc;
    BTDEVICE_SEND_DATAUP_FUNC BtDeviceSendUpDataFunc;
    unsigned char DeviceDescription[BTDEVICE_MAX_DESCRIPTION_SIZE];
} BTDEVICE_PARS_EX, *PBTDEVICE_PARS_EX;

//
// Device Priority Values
//
#define DEVICE_PRIORITY_NORMAL    0
#define DEVICE_PRIORITY_DISABLED  1
#define DEVICE_PRIORITY_PREFERRED 2
// Masks for the same value
#define PRIORITY_DEVICE_ACTIVE          0x8000
#define PRIORITY_DEVICE_SMART_DONGLE    0x4000

extern BTDEVICE_PARS device_pars;

#include <poppack.h>

#endif
