/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
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
* :>
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
