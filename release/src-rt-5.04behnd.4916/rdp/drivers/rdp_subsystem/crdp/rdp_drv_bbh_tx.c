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


#include "rdp_subsystem_common.h"
#include "rdp_common.h"
#include "rdp_drv_bbh_tx.h"
#include "rdd_data_structures_auto.h"


int drv_bbh_tx_cli_sanity_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
   return 0;
}

int drv_bbh_tx_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val, bdmf_boolean keep_alive, bdmf_boolean wan_enabled)
{
    return 0;
}

#ifdef USE_BDMF_SHELL


void drv_bbh_tx_cli_init(bdmfmon_handle_t driver_dir)
{
}

void drv_bbh_tx_cli_exit(bdmfmon_handle_t driver_dir)
{

}

/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/
/*
    cfg_get
    ag:
        pf_cfg_get (0_31 & group0_1)
        error_pm_counters
        pm_counters
*/
#endif /* USE_BDMF_SHELL */

