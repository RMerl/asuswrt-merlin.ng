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

#ifdef USE_BDMF_SHELL
#include "rdp_drv_shell.h"
#include "rdp_drv_system.h"
#include "rdp_drv_rnr.h"
#include "rdp_drv_sbpm.h"
#include "rdp_drv_fpm.h"
#include "rdp_drv_qm.h"
#include "rdp_drv_bbh_tx.h"

bdmfmon_handle_t drv_system_dir;

static int _rdp_drv_system_sanity(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int rc;

    rc = drv_sbpm_cli_sanity_check(session, parm, n_parms);
    rc = rc ? rc : drv_rnr_cli_sanity_get(session, parm, n_parms);
    rc = rc ? rc : drv_qm_cli_sanity_check(session, parm, n_parms);
    rc = rc ? rc : drv_fpm_cli_sanity_check(session, parm, n_parms);
    rc = rc ? rc : drv_bbh_tx_cli_sanity_get(session, parm, n_parms);

    return rc;
}

static int _rdp_drv_system_profiling(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    return 0;
}

int drv_system_cli_init(bdmfmon_handle_t driver_dir)
{
    drv_system_dir = bdmfmon_dir_add(driver_dir, "system", "sytem profiling and sanity", BDMF_ACCESS_ADMIN, NULL);

    BDMFMON_MAKE_CMD_NOPARM(drv_system_dir, "sanity", "system sanity validatioin",
        (bdmfmon_cmd_cb_t)_rdp_drv_system_sanity);
    BDMFMON_MAKE_CMD_NOPARM(drv_system_dir, "profiling", "present system profiling info",
        (bdmfmon_cmd_cb_t )_rdp_drv_system_profiling);

    return 0;
}

void drv_system_cli_exit(void)
{
    if (drv_system_dir)
    {
        bdmfmon_token_destroy(drv_system_dir);
        drv_system_dir = NULL;
    }
}

#endif
