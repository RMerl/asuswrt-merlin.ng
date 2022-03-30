// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    
*/

#ifndef _XRDP_DRV_SBPM_H_
#define _XRDP_DRV_SBPM_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#include "rdp_common.h"


/**************************************************************************************************/
/* alloc_bn_valid: alloc_bn_valid - alloc_bn_valid                                                */
/* alloc_bn: alloc_bn - alloc_bn                                                                  */
/* ack: ack - ack                                                                                 */
/* nack: nack - nack                                                                              */
/* excl_high: excl_high - Exclusive bit is indication of Exclusive_high status of client with rel */
/*            ated Alloc request                                                                  */
/* excl_low: excl_low - Exclusive bit is indication of Exclusive_low status of client with relate */
/*           d Alloc request                                                                      */
/* busy: busy - busy                                                                              */
/* rdy: rdy - rdy                                                                                 */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean alloc_bn_valid;
    uint16_t alloc_bn;
    bdmf_boolean ack;
    bdmf_boolean nack;
    bdmf_boolean excl_high;
    bdmf_boolean excl_low;
    bdmf_boolean busy;
    bdmf_boolean rdy;
} sbpm_regs_bn_alloc_rply;


/**************************************************************************************************/
/* bn_valid: bn_valid - Used for validation of Next BN reply                                      */
/* next_bn: next_bn - Next BN - reply of Get_next command                                         */
/* bn_null: bn_null - Next BN is null indication                                                  */
/* mcnt_val: mcnt_val - mcst cnt val                                                              */
/* busy: busy - Get Next command is busy                                                          */
/* rdy: rdy - Get Next command is ready                                                           */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean bn_valid;
    uint16_t next_bn;
    bdmf_boolean bn_null;
    uint8_t mcnt_val;
    bdmf_boolean busy;
    bdmf_boolean rdy;
} sbpm_regs_get_next_rply;


/**************************************************************************************************/
/* free_ack: free_ack - Acknowledge on Free command                                               */
/* ack_stat: ack_stat - ACK status of CPU                                                         */
/* nack_stat: nack_stat - NACK status of CPU                                                      */
/* excl_high_stat: excl_high_stat - Exclusive_high status of CPU                                  */
/* excl_low_stat: excl_low_stat - Exclusive_low status of CPU                                     */
/* bsy: bsy - Busy bit of command (command is currently in execution)                             */
/* rdy: rdy - Ready bit of command (ready for new command execution)                              */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean free_ack;
    bdmf_boolean ack_stat;
    bdmf_boolean nack_stat;
    bdmf_boolean excl_high_stat;
    bdmf_boolean excl_low_stat;
    bdmf_boolean bsy;
    bdmf_boolean rdy;
} sbpm_regs_bn_free_without_contxt_rply;


bdmf_error_t ag_drv_sbpm_regs_bn_alloc_set(uint8_t sa);
bdmf_error_t ag_drv_sbpm_regs_bn_alloc_rply_get(
		sbpm_regs_bn_alloc_rply *regs_bn_alloc_rply);
bdmf_error_t ag_drv_sbpm_regs_bn_connect_set(uint16_t bn, bdmf_boolean ack_req,
		bdmf_boolean wr_req, uint16_t pointed_bn);
bdmf_error_t ag_drv_sbpm_regs_bn_connect_rply_get(bdmf_boolean *connect_ack,
		bdmf_boolean *busy, bdmf_boolean *rdy);
bdmf_error_t ag_drv_sbpm_regs_get_next_set(uint16_t bn);
bdmf_error_t ag_drv_sbpm_regs_get_next_rply_get(
		sbpm_regs_get_next_rply *regs_get_next_rply);
bdmf_error_t ag_drv_sbpm_regs_bn_free_without_contxt_set(uint16_t head_bn,
		uint8_t sa, bdmf_boolean ack_req);
bdmf_error_t ag_drv_sbpm_regs_bn_free_without_contxt_rply_get(
		sbpm_regs_bn_free_without_contxt_rply *regs_bn_free_without_contxt_rply);

#endif

