/* 
    Copyright 2000-2017 Broadcom Corporation

    <:label-BRCM:2017:DUAL/GPL:standard
    
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
#ifndef __BCM63XX_SEC_H
#define __BCM63XX_SEC_H

#if defined (BOARD_SEC_ARCH)

#include "bcm_sec.h"
#include "btrm_if.h"
cfe_sec_err_t cfe_sec_get_boot_status(boot_status_t *info);
Booter1Args* cfe_sec_get_bootrom_args(void);
cfe_sec_err_t cfe_sec_reset_keys(void);
cfe_sec_err_t cfe_sec_init(void);
#ifdef CFG_RAMAPP
void cfe_sec_set_console_privLvl(void);
int cfe_sec_is_console_nonsec(void);
void cfe_sec_set_sotp_access_permissions(void);
#endif
#define CH4_TO_U32(CH4) be32_to_cpu(*((uint32_t*)((char*)#CH4)))
#define PRINT4CH(CH4) do { board_setleds(CH4_TO_U32(CH4)); }while(0);
#define PRINTU32(ui) do { board_setleds(ui); }while(0);
 
#endif

#endif
