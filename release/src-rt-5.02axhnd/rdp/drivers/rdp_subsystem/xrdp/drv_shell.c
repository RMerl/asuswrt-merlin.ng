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

#include "drv_shell.h"
#include "rdp_common.h"
#include "rdp_drv_bbh_rx.h"
#include "rdp_drv_bbh_tx.h"
#include "rdp_drv_dma.h"
#include "rdp_drv_sbpm.h"
#include "xrdp_drv_psram_ag.h"
#if !defined(BCM63158)
#include "xrdp_drv_psram_mem_ag.h"
#endif
#include "rdp_drv_fpm.h"
#include "rdp_drv_qm.h"
#include "rdp_drv_dqm.h"
#include "rdp_drv_rnr.h"
#include "rdp_drv_cnpl.h"
#include "rdp_drv_tcam.h"
#include "rdp_drv_natc.h"
#include "rdp_drv_dis_reor.h"
#include "rdp_drv_hash.h"
#include "rdp_drv_system.h"

void drv_cli_init(bdmfmon_handle_t driver_dir)
{
    ru_cli_init(driver_dir);
    drv_bbh_rx_cli_init(driver_dir);
    drv_bbh_tx_cli_init(driver_dir);
    drv_dma_cli_init(driver_dir);
    drv_sbpm_cli_init(driver_dir);
    drv_cnpl_cli_init(driver_dir);
    ag_drv_psram_cli_init(driver_dir);
#if !defined(BCM63158)
    ag_drv_psram_mem_cli_init(driver_dir);
#endif
    drv_fpm_cli_init(driver_dir);
    drv_qm_cli_init(driver_dir);
    drv_dqm_cli_init(driver_dir);
    drv_rnr_cli_init(driver_dir);
    drv_tcam_cli_init(driver_dir);
    drv_dis_reor_cli_init(driver_dir);
    drv_natc_cli_init(driver_dir);
    drv_system_cli_init(driver_dir);
    drv_hash_cli_init(driver_dir);
}

void drv_cli_exit(bdmfmon_handle_t driver_dir)
{
    ru_cli_exit(driver_dir);
    drv_bbh_rx_cli_exit(driver_dir);
    drv_bbh_tx_cli_exit(driver_dir);
    drv_dma_cli_exit(driver_dir);
    drv_sbpm_cli_exit(driver_dir);
    drv_cnpl_cli_exit(driver_dir);
    drv_qm_cli_exit(driver_dir);
    drv_dqm_cli_exit(driver_dir);
    drv_rnr_cli_exit(driver_dir);
    drv_tcam_cli_exit(driver_dir);
    drv_dis_reor_cli_exit(driver_dir);
    drv_fpm_cli_exit(driver_dir);
    drv_system_cli_exit();
    drv_hash_cli_exit(driver_dir);
}

#endif

