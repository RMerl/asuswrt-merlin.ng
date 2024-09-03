/*
<:copyright-BRCM:2016:DUAL/GPL:standard

   Copyright (c) 2016 Broadcom 
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


typedef void (*f_activation)(int bus); 

struct _TRX_DESCRIPTOR
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
};

#endif /* TRX_DESCR_H_INCLUDED */
