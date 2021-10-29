/*
 <:copyright-BRCM:2014:DUAL/GPL:standard
 
    Copyright (c) 2014 Broadcom 
    All Rights Reserved
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License, version 2, as published by
 the Free Software Foundation (the "GPL").
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 
 A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.
 
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
