/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

#ifndef _BCM6858_LPORT_SRDS_AG_H_
#define _BCM6858_LPORT_SRDS_AG_H_

#include "access_macros.h"
#include "bcmtypes.h"
typedef struct
{
    uint8_t serdes_test_en;
    uint8_t serdes_ln_offset;
    uint8_t serdes_prtad;
    uint8_t serdes_reset;
    uint8_t refclk_reset;
    uint8_t iddq;
} lport_srds_dual_serdes_0_cntrl;

typedef struct
{
    uint8_t mod_def0;
    uint8_t ext_sig_det;
    uint8_t pll_lock;
    uint8_t link_status;
    uint8_t cdr_lock;
    uint8_t rx_sigdet;
} lport_srds_dual_serdes_0_status;

typedef struct
{
    uint8_t serdes_test_en;
    uint8_t serdes_ln_offset;
    uint8_t serdes_prtad;
    uint8_t serdes_reset;
    uint8_t refclk_reset;
    uint8_t iddq;
} lport_srds_dual_serdes_1_cntrl;

typedef struct
{
    uint8_t mod_def0;
    uint8_t ext_sig_det;
    uint8_t pll_lock;
    uint8_t link_status;
    uint8_t cdr_lock;
    uint8_t rx_sigdet;
} lport_srds_dual_serdes_1_status;

int ag_drv_lport_srds_merlin_rev_get(uint16_t *serdes_rev);
int ag_drv_lport_srds_serdes_0_indir_acc_cntrl_0_set(uint8_t err, uint8_t start_busy, uint8_t r_w, uint16_t reg_data);
int ag_drv_lport_srds_serdes_0_indir_acc_cntrl_0_get(uint8_t *err, uint8_t *start_busy, uint8_t *r_w, uint16_t *reg_data);
int ag_drv_lport_srds_serdes_0_indir_acc_addr_0_set(uint32_t reg_addr);
int ag_drv_lport_srds_serdes_0_indir_acc_addr_0_get(uint32_t *reg_addr);
int ag_drv_lport_srds_serdes_0_indir_acc_mask_0_set(uint16_t reg_mask);
int ag_drv_lport_srds_serdes_0_indir_acc_mask_0_get(uint16_t *reg_mask);
int ag_drv_lport_srds_serdes_0_indir_acc_cntrl_1_set(uint8_t err, uint8_t start_busy, uint8_t r_w, uint16_t reg_data);
int ag_drv_lport_srds_serdes_0_indir_acc_cntrl_1_get(uint8_t *err, uint8_t *start_busy, uint8_t *r_w, uint16_t *reg_data);
int ag_drv_lport_srds_serdes_0_indir_acc_addr_1_set(uint32_t reg_addr);
int ag_drv_lport_srds_serdes_0_indir_acc_addr_1_get(uint32_t *reg_addr);
int ag_drv_lport_srds_serdes_0_indir_acc_mask_1_set(uint16_t reg_mask);
int ag_drv_lport_srds_serdes_0_indir_acc_mask_1_get(uint16_t *reg_mask);
int ag_drv_lport_srds_serdes_1_indir_acc_cntrl_0_set(uint8_t err, uint8_t start_busy, uint8_t r_w, uint16_t reg_data);
int ag_drv_lport_srds_serdes_1_indir_acc_cntrl_0_get(uint8_t *err, uint8_t *start_busy, uint8_t *r_w, uint16_t *reg_data);
int ag_drv_lport_srds_serdes_1_indir_acc_addr_0_set(uint32_t reg_addr);
int ag_drv_lport_srds_serdes_1_indir_acc_addr_0_get(uint32_t *reg_addr);
int ag_drv_lport_srds_serdes_1_indir_acc_mask_0_set(uint16_t reg_mask);
int ag_drv_lport_srds_serdes_1_indir_acc_mask_0_get(uint16_t *reg_mask);
int ag_drv_lport_srds_serdes_1_indir_acc_cntrl_1_set(uint8_t err, uint8_t start_busy, uint8_t r_w, uint16_t reg_data);
int ag_drv_lport_srds_serdes_1_indir_acc_cntrl_1_get(uint8_t *err, uint8_t *start_busy, uint8_t *r_w, uint16_t *reg_data);
int ag_drv_lport_srds_serdes_1_indir_acc_addr_1_set(uint32_t reg_addr);
int ag_drv_lport_srds_serdes_1_indir_acc_addr_1_get(uint32_t *reg_addr);
int ag_drv_lport_srds_serdes_1_indir_acc_mask_1_set(uint16_t reg_mask);
int ag_drv_lport_srds_serdes_1_indir_acc_mask_1_get(uint16_t *reg_mask);
int ag_drv_lport_srds_dual_serdes_0_cntrl_set(const lport_srds_dual_serdes_0_cntrl *dual_serdes_0_cntrl);
int ag_drv_lport_srds_dual_serdes_0_cntrl_get(lport_srds_dual_serdes_0_cntrl *dual_serdes_0_cntrl);
int ag_drv_lport_srds_dual_serdes_0_status_get(lport_srds_dual_serdes_0_status *dual_serdes_0_status);
int ag_drv_lport_srds_dual_serdes_1_cntrl_set(const lport_srds_dual_serdes_1_cntrl *dual_serdes_1_cntrl);
int ag_drv_lport_srds_dual_serdes_1_cntrl_get(lport_srds_dual_serdes_1_cntrl *dual_serdes_1_cntrl);
int ag_drv_lport_srds_dual_serdes_1_status_get(lport_srds_dual_serdes_1_status *dual_serdes_1_status);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_lport_srds_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

