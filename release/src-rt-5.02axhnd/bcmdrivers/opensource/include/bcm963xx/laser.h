/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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

#ifndef _LASER_H_
#define _LASER_H_

//#include <linux/if.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include "bcmtypes.h"

#define LASER_DEV "/dev/laser_dev"
#define LASER_TOTAL_OPTICAL_PARAMS_LEN  (96)

#define LASER_IOC_MAGIC		'O'

#define LASER_IOCTL_GET_DRV_INFO	    _IOR    (LASER_IOC_MAGIC, 0, long)
#define LASER_IOCTL_SET_OPTICAL_PARAMS  _IOWR   (LASER_IOC_MAGIC, 1, LASER_OPTICAL_PARAMS)
#define LASER_IOCTL_GET_OPTICAL_PARAMS  _IOWR   (LASER_IOC_MAGIC, 2, LASER_OPTICAL_PARAMS)
#define LASER_IOCTL_INIT_TX_PWR	        _IO     (LASER_IOC_MAGIC, 3)
#define LASER_IOCTL_INIT_RX_PWR	        _IOW    (LASER_IOC_MAGIC, 4, long)
#define LASER_IOCTL_GET_RX_PWR	        _IOR    (LASER_IOC_MAGIC, 5, short)
#define LASER_IOCTL_GET_TX_PWR	        _IOR    (LASER_IOC_MAGIC, 6, short)
#define LASER_IOCTL_GET_INIT_PARAMS     _IOWR   (LASER_IOC_MAGIC, 7, LASER_INIT_PARAMS)
#define LASER_IOCTL_GET_TEMPTURE        _IOR    (LASER_IOC_MAGIC, 8,  short)
#define LASER_IOCTL_GET_VOTAGE	        _IOR    (LASER_IOC_MAGIC, 9,  short)
#define LASER_IOCTL_GET_BIAS_CURRENT    _IOR    (LASER_IOC_MAGIC, 10, short)


typedef struct LaserOpticalParams
{
    short opLength;
    BCM_IOC_PTR(unsigned char *, opRegisters);
} LASER_OPTICAL_PARAMS, *PLASER_OPTICAL_PARAMS;

typedef struct LaserInitParams
{
    unsigned short initRxReading;
    unsigned short initRxOffset;
    unsigned short initTxReading;
} LASER_INIT_PARAMS, *PLASER_INIT_PARAMS;

#endif /* ! _LASER_H_ */
