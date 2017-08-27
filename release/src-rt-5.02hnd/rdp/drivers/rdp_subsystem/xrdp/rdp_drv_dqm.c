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

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#include "rdp_subsystem_common.h"
#include "rdp_common.h"
#include "rdp_drv_dqm.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "ru.h"

/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* Macros definitions                                                         */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* Global variables definitions                                               */
/*                                                                            */
/******************************************************************************/

#ifdef USE_BDMF_SHELL

/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/

static bdmfmon_handle_t dqm_dir;

void drv_dqm_cli_init(bdmfmon_handle_t driver_dir)
{
    ag_drv_dqm_cli_init(driver_dir);

    if (!(dqm_dir = bdmfmon_dir_find(driver_dir, "dqm")))
        return;

    /* Add commands to call drv_dqm_queue_enable_set/drv_dqm_queue_enable_get */
    /* ToDo: */
}

void drv_dqm_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (dqm_dir)
    {
        bdmfmon_token_destroy(dqm_dir);
        dqm_dir = NULL;
    }
}

#endif /* USE_BDMF_SHELL */

