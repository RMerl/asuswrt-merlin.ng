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


#ifndef TRX_DESCR_GEN_H_INCLUDED
#define TRX_DESCR_GEN_H_INCLUDED

#include <linux/types.h>
#include "opticaldet.h"
#include "wan_types.h"

#define TRX_EEPROM_OFFSET_TYPE  0
#define TRX_EEPROM_OFFSET_CODE  3
#define TRX_EEPROM_OFFSET_CODE2 36
#define TRX_EEPROM_OFFSET_NAME 20
#define TRX_EEPROM_OFFSET_OUI  37
#define TRX_EEPROM_OFFSET_REV  56
#define TRX_EEPROM_OFFSET_PN   40
#define TRX_EEPROM_OFFSET_SN   68
#define TRX_EEPROM_OFFSET_TX_WAVLEN   60
#define TRX_EEPROM_LEN_NAME    16
#define TRX_EEPROM_LEN_CODE     8
#define TRX_EEPROM_LEN_OUI      3
#define TRX_EEPROM_LEN_REV      4
#define TRX_EEPROM_LEN_PN      16
#define TRX_EEPROM_LEN_SN      16
#define TRX_EEPROM_LEN_TX_WAVLEN   2
#define SOURCEPHOTONICS_FIXUP_PN "SPPS2748FN2CDFA"
#define TRX_EEPROM_CC_10GETH_MASK 0xf0
#define TRX_EEPROM_CC_ETH_MASK    0xff

#define TRX_XFP_EEPROM_PAGE_SELECT   127
#define TRX_XFP_EEPROM_PAGE_1        128
#define TRX_XFP_EEPROM_OFFSET_NAME   148
#define TRX_XFP_EEPROM_OFFSET_PN     168
#define TRX_XFP_EEPROM_LEN_REV       2
#define TRX_XFP_EEPROM_CC_10GETH_MASK 0xff
#define TRX_XFP_EEPROM_CC_ETH_MASK    0xc0

typedef void (*f_activation) (int bus); 
void ltw2601_activation(int bus);

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


/*
 * List of xPON and AE transcievers used by BRCM.
 * To extend/override this list put the entries to trx_usr[] array in ./trx_descr_usr.h
 */

static TRX_DESCRIPTOR trx_lst[] = {
  {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "SOURCEPHOTONICS",
    .vendor_pn             = "SPPS2748FN2CDFA",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = true,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_XGPON
  },
  {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "SOURCEPHOTONICS",
    .vendor_pn             = "SPPS27XER3CDFD",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = true,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_XGSPON
  },
  {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "SOURCEPHOTONICS",
    .vendor_pn             = "SPPS2710SN2CDFA",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = true,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_XGSPON
  },
  {
    .form_factor           = TRX_XFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Hisense",
    .vendor_pn             = "LTW2601C-BC+",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) ltw2601_activation,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_NGPON2_10_10
  },
  {
    .form_factor           = TRX_XFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Hisense",
    .vendor_pn             = "LTW2601-BC",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) ltw2601_activation,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_NGPON2_10_10
  },
  {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Ligent Photonics",
    .vendor_pn             = "LTF7219-BC",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_HIGH,
    .tx_pwr_down_cfg_req   = true,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_EPON_10_10 | SUPPORTED_WAN_TYPES_BIT_XGSPON
  },
  {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Ligent",
    .vendor_pn             = "LTF7219-BC",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_HIGH,
    .tx_pwr_down_cfg_req   = true,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_EPON_10_10 | SUPPORTED_WAN_TYPES_BIT_XGSPON,
    .rx_wavlen             = 1577,
    .power_budget          = TRX_PB_PR30
  },
  {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Hisense",
    .vendor_pn             = "LTF7219-BH+",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_EPON_10_10 | SUPPORTED_WAN_TYPES_BIT_EPON_10_1 | SUPPORTED_WAN_TYPES_BIT_XGSPON,
    .rx_wavlen             = 1577,
    .power_budget          = TRX_PB_PR30
  },  
  {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "NEOPHOTONICS",
    .vendor_pn             = "PTNEN3-41CP-ST+",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_EPON_10_1,
    .rx_wavlen             = 1577,
    .power_budget          = TRX_PB_PRX30
  },
  {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "NEOPHOTONICS",
    .vendor_pn             = "PTNEN3-44CP-ST+",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_EPON_10_10,
    .rx_wavlen             = 1577,
    .power_budget          = TRX_PB_PR30
  },
  {
    .form_factor           = TRX_SFF,
    .type                  = TRX_TYPE_XPON,        
    .vendor_name           = "DELTA",
    .vendor_pn             = "OPGP-34-A4B3SN",
    .lbe_polarity          = TRX_ACTIVE_HIGH,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_NOT_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_GPON | SUPPORTED_WAN_TYPES_BIT_EPON_1_1 | SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1
  },
  {
    .form_factor           = TRX_SFF,
    .type                  = TRX_TYPE_XPON,        
    .vendor_name           = "Hisense",
    .vendor_pn             = "LTB3467-BCG+",
    .lbe_polarity          = TRX_ACTIVE_HIGH,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_NOT_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_GPON | SUPPORTED_WAN_TYPES_BIT_EPON_1_1 | SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1,
    .rx_wavlen             = 1490,
    .power_budget          = TRX_PB_PR20
  },
  {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_XPON,        
    .vendor_name           = "Hisense",
    .vendor_pn             = "LTF7225-BC+",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_HIGH,
    .tx_pwr_down_cfg_req   = true,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_XGSPON
  },
  {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_XPON,        
    .vendor_name           = "Hisense",
    .vendor_pn             = "LTF7225-BCA+",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_HIGH,
    .tx_pwr_down_cfg_req   = true,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_XGSPON
  },
  {
    .form_factor           = TRX_XFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Ligent",
    .vendor_pn             = "LTW2601C-BC",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) ltw2601_activation,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_NGPON2_10_10
  },
  {
    .form_factor           = TRX_XFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Hisense",
    .vendor_pn             = "LTW2601D-BC+",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) ltw2601_activation,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_NGPON2_10_10
  },
  {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Ligent Photonics",
    .vendor_pn             = "LTF7221-BH",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_XGPON
  },
  {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Hisense",
    .vendor_pn             = "LTF7221-BH+",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_XGPON,
    .rx_wavlen             = 1577
  },  
  {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Ligent Photonics",
    .vendor_pn             = "LTF7221-BC",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_XGPON
  },
  {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Ligent Photonics",
    .vendor_pn             = "LTF7225-BC",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_HIGH,
    .tx_pwr_down_cfg_req   = true,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_XGSPON
  },
  {
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Ligent Photonics",
    .vendor_pn             = "LTF7215-BC",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_HIGH,
    .tx_pwr_down_cfg_req   = true,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_EPON_10_1,
    .rx_wavlen             = 1577,
    .power_budget          = TRX_PB_PRX30
  },
  {
    .form_factor           = TRX_SFP,      
    .type                  = TRX_TYPE_XPON,        
    .vendor_name           = "ZKTEL",
    .vendor_pn             = "ZP5342034-KCST",
    .lbe_polarity          = TRX_ACTIVE_HIGH,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_GPON
  },
  {
    .form_factor           = TRX_SFP,      
    .type                  = TRX_TYPE_ETHERNET, 
    .vendor_name           = "FiberStore",
    .vendor_pn             = "SFP-10GSR-85",
    .tx_pwr_down_cfg_req   = false, /*  RS1 Rate Select 1 | 4. Internally pulled down per SFF-8431 Rev 2.0 */
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_AE_10_10
  },
  {
    .form_factor           = TRX_SFP,      
    .type                  = TRX_TYPE_ETHERNET, 
    .vendor_name           = "JDSU",
    .vendor_pn             = "PLRXPLSCS4322N",
    .tx_pwr_down_cfg_req   = false, /* TX Rate Select (LVTTL) This pin has an internal 30 k?? pulldown to ground. A signal on this pin will not affect module performance. */
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_AE_10_10
  },
  {
    .form_factor           = TRX_SFP,      
    .type                  = TRX_TYPE_ETHERNET, 
    .vendor_name           = "FINISAR CORP.",
    .vendor_pn             = "FCLF-8521-3",
    .tx_pwr_down_cfg_req   = false, /* Receiver ground (common with transmitter ground) */
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_AE_1_1
  },
  {
    .form_factor           = TRX_SFP,      
    .type                  = TRX_TYPE_ETHERNET, 
    .vendor_name           = "SOURCEPHOTONICS",
    .vendor_pn             = "SPP10ESRCDFF",
    .tx_pwr_down_cfg_req   = false, /* Rate Select 1, NOT implement | The pins are pulled low to VCCT with a >30k?? resistor in the module. */
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_AE_10_10
  },
  {
    .form_factor           = TRX_SFP,      
    .type                  = TRX_TYPE_XPON,        
    .vendor_name           = "ZKTEL",
    .vendor_pn             = "ZP5342033-HCSY",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_NOT_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_EPON_1_1 | SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1,
    .rx_wavlen             = 1490,
    .power_budget          = TRX_PB_PR20
  },
  {
    .form_factor           = TRX_SFF,
    .type                  = TRX_TYPE_XPON,        
    .vendor_name           = "DELTA",
    .vendor_pn             = "OPGP-34-A4B3SV",
    .lbe_polarity          = TRX_ACTIVE_HIGH,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_NOT_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_GPON | SUPPORTED_WAN_TYPES_BIT_EPON_1_1 | SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1,
    .rx_wavlen             = 1490,
    .power_budget          = TRX_PB_PR20
  },
  {
    .form_factor           = TRX_SFP,      
    .type                  = TRX_TYPE_XPON,        
    .vendor_name           = "Hisense",
    .vendor_pn             = "LTE3468-BHG1",
    .lbe_polarity          = TRX_ACTIVE_HIGH,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_GPON
  },
  {
    .form_factor           = TRX_SFP,      
    .type                  = TRX_TYPE_XPON,        
    .vendor_name           = "Hisense",
    .vendor_pn             = "LTE3468L-BHG2",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_GPON
  },
  {
    .form_factor           = TRX_SFP,      
    .type                  = TRX_TYPE_XPON,        
    .vendor_name           = "MENTECHOPTO",
    .vendor_pn             = "SOG22-LD6I-AC",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .tx_sd_supported       = TRX_SIGNAL_SUPPORTED,
    .activation_func       = (f_activation) NULL,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_GPON
  },
} ;

#endif /* TRX_DESCR_GEN_H_INCLUDED */
