/*
 <:copyright-BRCM:2014:DUAL/GPL:standard
 
    Copyright (c) 2014 Broadcom 
    All Rights Reserved
 
 Unless you and Broadcom execute a separate written software license
 agreement governing use of this software, this software is licensed
 to you under the terms of the GNU General Public License version 2
 (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 with the following added to such license:
 
    As a special exception, the copyright holders of this software give
    you permission to link this software with independent modules, and
    to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent
    module, the terms and conditions of the license of that module.
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications
    of the software.
 
 Not withstanding the above, under no circumstances may you combine
 this software in any way with any other Broadcom software provided
 under a license other than the GPL, without Broadcom's express prior
 written consent.
 
:>
*/

/**
	@file
	@brief	This include file contains the IOCTL's declaration 
			for user kernel communication. 

	@defgroup SIMDeviceDrivers SIM Device Drivers
	@brief This group is the device driver interface to the SIM.
	@ingroup DeviceDriverGroup
*/

#ifndef _SIMIO_USER_KERNEL_H_
#define _SIMIO_USER_KERNEL_H_

#include <linux/ioctl.h>
 
typedef struct
{
    SIMIO_ID_t sim_id;
    union
    {
        struct
        {
            uint8_t data[SIM_CARD_MAX_BUFFER_SIZE];
            size_t len;
        } active; /* to use with SIMIO_ACTIVATE */
        int detection_status; /* to use with SIMIO_IS_ONLINE */
        struct
        {
            int F;
            int D;
        } baud_rate;  /* to use with SIMIO_SET_BAUDRATE */
        struct
        {
            int tx_len;
            int rx_len;
            uint8_t data[SIM_CARD_MAX_BUFFER_SIZE];
        } io; /* to use with SIMIO_READ and SIMIO_WRITE */
        PROTOCOL_t protocol; /* to use with SIMIO_SET_PROTOCOL */
        int control; /* to use with SIMIO_SET_CONTROL */
        struct
        {
            int reset;
            SIMIO_DIVISOR_t freq;
            SimVoltageLevel_t voltage;
        } reset; /* to use with SIMIO_RESET */
    } data;
    int ret; /* The action return value */
} simio_ioctl_arg_t;

#define SIMIO_ACTIVATE      _IOR('s', 1, simio_ioctl_arg_t *)
#define SIMIO_IS_ONLINE     _IOR('s', 2, simio_ioctl_arg_t *)
#define SIMIO_SET_BAUDRATE  _IOW('s', 3, simio_ioctl_arg_t *)
#define SIMIO_WRITE         _IOWR('s', 5, simio_ioctl_arg_t *)
#define SIMIO_READ          _IOWR('s', 6, simio_ioctl_arg_t *)
#define SIMIO_SET_PROTOCOL  _IOW('s', 11, simio_ioctl_arg_t *)
#define SIMIO_SET_CONTROL   _IOW('s', 12, simio_ioctl_arg_t *)
#define SIMIO_RESET         _IOW('s', 13, simio_ioctl_arg_t *)
 
#endif
