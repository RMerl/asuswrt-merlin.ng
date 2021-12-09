/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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

#ifndef DETECT_H_INCLUDED
#define DETECT_H_INCLUDED

#include "wan_types.h"


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

#define EPON2G (1 << 31)
#define RDPA_WAN_MASK 0xF


#define OPTICALDET_IOCTL_DETECT 1
#define OPTICALDET_IOCTL_SD 2

#define OPTICALDET_SUCCESS      0
#define OPTICALDET_INVPARM      1
#define OPTICALDET_NOBUS        2
#define OPTICALDET_NOSFP        3
#define OPTICALDET_INVSFP       4


extern int trx_get_supported_wan_type_bm(int bus, SUPPORTED_WAN_TYPES_BITMAP *wan_type_bm);
extern int trx_get_lbe_polarity(int bus, TRX_SIG_ACTIVE_POLARITY *lbe_polarity_p);
extern int trx_get_tx_sd_polarity(int bus, TRX_SIG_ACTIVE_POLARITY *tx_sd_polarity_p);
extern int trx_get_vendor_name_part_num(int bus, char *vendor_name_p, int vendor_name_len,
                                 char *part_num_p, int part_num_len);
extern int trx_get_tx_sd_supported(int bus, TRX_SIG_PRESENCE *signal_supported_p);
extern int trx_get_type(int bus, TRX_TYPE *trx_type);
#ifdef CONFIG_BP_PHYS_INTF
extern int opticaldet_get_i2c_bus_num(unsigned short intf_type, int intf_idx, int* bus);
#endif
extern int opticaldet_get_xpon_i2c_bus_num(int* bus);
extern int opticaldet_get_sgmii_i2c_bus_num(int* bus);

#endif /* DETECT_H_INCLUDED */
