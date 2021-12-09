/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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
