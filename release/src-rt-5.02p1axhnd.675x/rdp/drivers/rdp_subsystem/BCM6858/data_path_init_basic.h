/*
   Copyright (c) 2014 Broadcom
   All Rights Reserved

    <:label-BRCM:2014:DUAL/GPL:standard
    
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

#ifndef _DATA_PATH_INIT_BASIC_
#define _DATA_PATH_INIT_BASIC_

#ifdef __cplusplus 
extern "C" {
#endif

#ifndef XRDP
#define XRDP
#endif

#define BBH_TX_COMMON_CONFIGURATION_DDRMBASEL_BASE_ADDRESS  0x82d0002c
#define BBH_TX_COMMON_CONFIGURATION_DDRMBASEL_BASE_ADDRESS1 0x82d0402c
#define BBH_TX_COMMON_CONFIGURATION_DDRMBASEL_BASE_ADDRESS2 0x82d0802c
#define BBH_TX_COMMON_CONFIGURATION_DDRMBASEL_BASE_ADDRESS3 0x82d0c02c
#define BBH_TX_COMMON_CONFIGURATION_DDRMBASEL_BASE_ADDRESS4 0x82d1002c
#define BBH_TX_COMMON_CONFIGURATION_DDRMBASEL_BASE_ADDRESS5 0x82d1402c
#define BBH_TX_COMMON_CONFIGURATION_DDRMBASEL_BASE_ADDRESS6 0x82d1802c
#define BBH_TX_COMMON_CONFIGURATION_DDRMBASEL_BASE_ADDRESS7 0x82d1c02c

/* includes */
#include "bdmf_data_types.h"
#include "rdp_common.h"

/* data structures */
typedef enum
{
    wan_pon_no_xfi_profile_id,
    wan_pon_xfi0_profile_id,
    wan_pon_xfi4_profile_id,
    profiles_num,
} profile_id_t;

/* functions */
int data_path_init(dpi_params_t *dpi_params);
int data_path_init_basic(dpi_params_t *dpi_params);
void data_path_init_basic_update_fpm(dpi_params_t *dpi_params);

#ifdef __cplusplus 
}
#endif
#endif

