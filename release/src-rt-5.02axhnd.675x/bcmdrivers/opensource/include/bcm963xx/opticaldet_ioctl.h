/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
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

#ifndef OPTDETECT_IOCTL_H_INCLUDED
#define OPTDETECT_IOCTL_H_INCLUDED

#include "wan_types.h"


#define OPTDETECT_IOC_DEV "/dev/opticaldetect"

#define OPTDETECT_IOC_MAGIC     'd'

/* ioctl cmd */
#define OPTDETECT_IOCTL_GET_TRX_INFO  _IOR   (OPTDETECT_IOC_MAGIC, 0, TRX_INFOMATION)

typedef enum {
     TRX_SFF = 0x2,
     TRX_SFP = 0x3,
     TRX_XFP = 0x6,
     TRX_PMD = 0xff,
} TRX_FORM_FACTOR; 

typedef enum {
     TRX_TYPE_XPON, 
     TRX_TYPE_ETHERNET,
     TRX_TYPE_UNKNOWN,
} TRX_TYPE; 

typedef enum {
    TRX_ACTIVE_LOW,
    TRX_ACTIVE_HIGH
} TRX_SIG_ACTIVE_POLARITY;

typedef enum {
    TRX_SIGNAL_NOT_SUPPORTED,
    TRX_SIGNAL_SUPPORTED
} TRX_SIG_PRESENCE;

typedef enum {
    TRX_PB_PRX10,
    TRX_PB_PR10,
    TRX_PB_PRX20,
    TRX_PB_PR20,
    TRX_PB_PRX30,
    TRX_PB_PR30
} TRX_POWER_BUDGETS;  /* IEEE 802.3av */


typedef struct
{
    TRX_FORM_FACTOR form_factor;
    TRX_TYPE type;
    uint8_t  vendor_name[16+1];
    uint8_t  vendor_pn[16+1];
    uint8_t  vendor_sn[16];
    SUPPORTED_WAN_TYPES_BITMAP wan_types_bitmap;
    TRX_POWER_BUDGETS power_budget;
    uint16_t tx_wavlen;
    uint16_t rx_wavlen;
}  TRX_INFOMATION;

#endif /* OPTDETECT_IOCTL_H_INCLUDED */
