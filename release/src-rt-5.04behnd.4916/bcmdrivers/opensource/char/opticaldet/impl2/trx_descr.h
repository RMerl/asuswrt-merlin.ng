/*
<:copyright-BRCM:2016:DUAL/GPL:standard

   Copyright (c) 2016 Broadcom 
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


#ifndef TRX_DESCR_H_INCLUDED
#define TRX_DESCR_H_INCLUDED

#include "opticaldet.h"
#include "wan_types.h"
struct device;

#define TRX_EEPROM_LEN_NAME    16
#define TRX_EEPROM_LEN_CODE     8
#define TRX_EEPROM_LEN_OUI      3
#define TRX_EEPROM_LEN_REV      4
#define TRX_EEPROM_LEN_PN      16
#define TRX_EEPROM_LEN_SN      16


typedef void (*f_activation)(struct device *dev); 

typedef struct
{
    TRX_FORM_FACTOR form_factor;
    TRX_TYPE type;
    uint8_t  vendor_name[TRX_EEPROM_LEN_NAME+1];
    uint8_t  vendor_pn[TRX_EEPROM_LEN_PN+1];
    uint8_t  vendor_rev[TRX_EEPROM_LEN_REV+1];
    uint8_t  vendor_sn[TRX_EEPROM_LEN_SN];
    TRX_SIG_ACTIVE_POLARITY    lbe_polarity;
    TRX_SIG_ACTIVE_POLARITY    tx_sd_polarity;
    TRX_SIG_ACTIVE_POLARITY    tx_pwr_down_polarity;
    bool                       tx_pwr_down_cfg_req;
    TRX_SIG_PRESENCE           tx_sd_supported;
    f_activation               activation_func;
    SUPPORTED_WAN_TYPES_BITMAP wan_types_bitmap;
    TRX_POWER_BUDGETS power_budget;
    uint16_t tx_wavlen;
    uint16_t rx_wavlen;
}  TRX_DESCRIPTOR;

#endif /* TRX_DESCR_H_INCLUDED */
