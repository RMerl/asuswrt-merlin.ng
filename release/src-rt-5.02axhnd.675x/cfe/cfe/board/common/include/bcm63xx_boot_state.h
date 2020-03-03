/*
    Copyright 2000-2019 Broadcom Corporation

    <:label-BRCM:2019:DUAL/GPL:standard
    
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

/**************************************************************************
 * File Name  : bcm6xx_boot_state.h
 *
 * Boot state maintainance across soft reset
 *
 *
 * Updates    : 04/11/2019  Created.
 ***************************************************************************/

#ifndef _BCM63XX_BOOT_STATE_H

#define _BCM63XX_BOOT_STATE_H

#include "bcm_mbox_map.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum _boot_state_info_type {
     CFE_BOOT_STATE_TYPE_NONE = 0,
     CFE_BOOT_STATE_TYPE_ST = 1,
     CFE_BOOT_STATE_TYPE_ERR = 2,
     CFE_BOOT_STATE_TYPE_INFO = 4,
} cfe_boot_state_info_type_t;

typedef enum cfe_boot_state_err {
      CFE_BOOT_STATE_ERR_OK = 0,
      CFE_BOOT_STATE_ERR_DIS = -1,
      CFE_BOOT_STATE_ERR_INVALID = -2,
} cfe_boot_state_err_t;



/* arm_wd - integer value -1 will turn off watchdog, 
   0 - will be ignored
   other values ms till reset by wd  */
cfe_boot_state_err_t cfe_boot_st_init(void);
cfe_boot_st_info_t cfe_boot_st_track(void);
void cfe_boot_st_complete(void);
unsigned int cfe_boot_st_error(void);
#ifdef __cplusplus
}
#endif

#endif /* _BCM63xx_BOOT_STATE_H */

