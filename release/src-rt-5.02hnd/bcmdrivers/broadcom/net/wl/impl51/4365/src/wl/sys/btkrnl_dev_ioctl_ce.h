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

#include "btkrnl_ioctl_codes.h"


// Filter buffer types:
#define BTKRNL_FILTER_BUFFER_TYPE_COMMAND  0x01
#define BTKRNL_FILTER_BUFFER_TYPE_DATAOUT  0x02
#define BTKRNL_FILTER_BUFFER_TYPE_EVENT    0x03
#define BTKRNL_FILTER_BUFFER_TYPE_DATAIN   0x04

// BTKRNL decisions on what to do with the data (stored into an output buffer)
#define BTKRNL_FILTER_FORWARD_DATA         0x00
#define BTKRNL_FILTER_DONOT_FORWARD_DATA   0x01


#define BTDEVICE_SUCCESS         0
#define BTDEVICE_ERROR_RESOURCES 1

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

    unsigned long control_type;

    union {

#define VOICE_ALT_SETTING_16BIT_ENCODING   0
#define VOICE_ALT_SETTING_8BIT_ENCODING    1

    unsigned long voice_setting;

#define VOICE_HEADER_SIZE                      3
    unsigned char voice_header[VOICE_HEADER_SIZE];
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

// OR this with something above if it's a filter for this media type
#define BTDEVICE_TYPE_FILTER  0x10
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

    // TBD - chances we better pass a UNICODE description string to
    // assure WinNT/2000 compatibility?
#define BTDEVICE_MAX_DESCRIPTION_SIZE 80
    unsigned char DeviceDescription[BTDEVICE_MAX_DESCRIPTION_SIZE];

} BTDEVICE_PARS, *PBTDEVICE_PARS;

extern BTDEVICE_PARS device_pars;


#define BTDEVICE_TYPE_NONE    0             // no device
#define BTDEVICE_TYPE_AMP    10

//
// The newer version of this structure
// (most members described above)
//
typedef struct _BTDEVICE_PARS_EX
{
    unsigned long   ParsVersion;  // version of this type (currently 1)
    void            *DeviceHandle;
    unsigned long   DeviceType;
    unsigned long   DeviceNumber;
    BTDEVICE_SEND_DATA_FUNC BtDeviceSendDataFunc;
    BTDEVICE_SEND_CMD_FUNC  BtDeviceSendCmdFunc;
    BTDEVICE_SEND_VOICE_FUNC BtDeviceSendVoiceFunc;
    BTDEVICE_SEND_CTL_FUNC BtDeviceSendCtlFunc;
    BTDEVICE_SEND_DATAUP_FUNC BtDeviceSendUpDataFunc;
    unsigned char   DeviceDescription[BTDEVICE_MAX_DESCRIPTION_SIZE];
} BTDEVICE_PARS_EX, *PBTDEVICE_PARS_EX;


#endif
