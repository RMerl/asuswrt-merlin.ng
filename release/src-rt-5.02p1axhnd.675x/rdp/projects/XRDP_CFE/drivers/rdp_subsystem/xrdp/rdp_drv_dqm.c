/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
       All Rights Reserved
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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

