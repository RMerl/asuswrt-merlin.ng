/*
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
 :>
*/

/*******************************************************************
 * rdpa_module_init.c
 *
 * Runner Data Path API - init code
 * This file must be given to the linker first!
 *
 *******************************************************************/

#define DEBUG

#include <bdmf_dev.h>
#include "rdpa_api.h"
#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#include "drv_shell.h"
#ifdef XRDP
#include "rdp_drv_shell.h"
#endif
#endif

/*
 * Start type and aggregate type sections.
 */
unsigned long rdpa_type_section_start __attribute__((section("BDMF_init"))) = 0;
unsigned long rdpa_aggr_section_start __attribute__((section("BDMF_aggr_init"))) = 0;
extern unsigned long rdpa_type_section_end;
extern unsigned long rdpa_aggr_section_end;

#ifdef USE_BDMF_SHELL
static bdmfmon_handle_t driver_dir;
void misc_shell_init(void);
void misc_shell_uninit(void);

void register_shell_commands(void)
{
    driver_dir = bdmfmon_dir_add(NULL, "driver", "Device Drivers", BDMF_ACCESS_ADMIN, NULL);
    if (!driver_dir)
    {
        bdmf_session_print(NULL, "Can't create driver directory\n");
        return;
    }
    drv_cli_init(driver_dir);
#ifdef XRDP
    rdp_drv_shell_init(driver_dir);
#endif
    misc_shell_init();
}

void unregister_shell_commands(void)
{
    drv_cli_exit(driver_dir);
#ifdef XRDP
    rdp_drv_shell_exit(driver_dir);
#endif
    misc_shell_uninit();
    bdmfmon_token_destroy(driver_dir);
}
#endif

/** Initialize Broadcom Device Management Framefork
 *
 *  This function should be called once at init time.
 * \parm[in]    init_config     Initial configuration
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int rdpa_module_init(void)
{
    int rc;

    rc = bdmf_register_aggregate_types(&rdpa_aggr_section_start + 1,
        &rdpa_aggr_section_end - 1);
    rc = rc ? rc : bdmf_register_plugins(&rdpa_type_section_start + 1,
        &rdpa_type_section_end - 1);
#ifdef USE_BDMF_SHELL
    if (!rc)
        register_shell_commands();
#endif
    return rc;
}

void rdpa_module_exit(void)
{
    bdmf_unregister_plugins(&rdpa_type_section_start + 1,
        &rdpa_type_section_end - 1);
    bdmf_unregister_aggregate_types(&rdpa_aggr_section_start + 1,
        &rdpa_aggr_section_end - 1);
#ifdef USE_BDMF_SHELL
    unregister_shell_commands();
#endif
}

module_init(rdpa_module_init);
module_exit(rdpa_module_exit);
MODULE_LICENSE("Proprietary");

MODULE_DESCRIPTION("Runner Data Path API. (C) Broadcom");
