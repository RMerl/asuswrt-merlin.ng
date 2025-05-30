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
#include "rdp_drv_cnpl.h"
#include "rdp_drv_bufmng.h"
#include "rdp_common.h"
#include "rdd_defs.h"
#include "XRDP_AG.h"


#ifdef USE_BDMF_SHELL


static bdmfmon_handle_t bufmng_dir;

#if defined(CONFIG_CPU_RX_FROM_XPM)
static int drv_cnpl_cli_bufmng_disable_cnt_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int32_t cntr_mask = parm[0].value.number;

    if (cntr_mask == -1)
    {
        RDD_BYTES_4_BITS_READ_G(cntr_mask, RDD_BUFMNG_HOST_CNT_DISABLE_TABLE_ADDRESS_ARR, 0);
        bdmf_session_print(session, "Disable mask: %d\n\r", cntr_mask);
    }
    else
    {
        RDD_BYTES_4_BITS_WRITE_G(cntr_mask, RDD_BUFMNG_HOST_CNT_DISABLE_TABLE_ADDRESS_ARR, 0);
    }

    return BDMF_ERR_OK;
}
#endif

void drv_bufmng_cli_init(bdmfmon_handle_t driver_dir)
{
    bufmng_dir = ag_drv_bufmng_cli_init(driver_dir);

    BDMFMON_MAKE_CMD_NOPARM(bufmng_dir, "cfg_get", "cnpl configuration", (bdmfmon_cmd_cb_t)drv_cnpl_cli_config_get);
    BDMFMON_MAKE_CMD_NOPARM(bufmng_dir, "bufmng_cfg_get", "BUFMNG configuration", (bdmfmon_cmd_cb_t)drv_cnpl_cli_bufmng_config_get);
#if defined(CONFIG_CPU_RX_FROM_XPM)
    BDMFMON_MAKE_CMD(bufmng_dir, "host_disable", "BUFMNG disable configured counters for host", drv_cnpl_cli_bufmng_disable_cnt_set,
        BDMFMON_MAKE_PARM_DEFVAL("cntr_mask", "counter mask", BDMFMON_PARM_NUMBER, 0, -1));
#endif
}

void drv_bufmng_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (bufmng_dir)
    {
        bdmfmon_token_destroy(bufmng_dir);
        bufmng_dir = NULL;
    }
}

/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/
/*

*/
#endif /* USE_BDMF_SHELL */

