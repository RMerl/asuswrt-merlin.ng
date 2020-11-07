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

#ifndef _BCM6858_GPON_AG_H_
#define _BCM6858_GPON_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif

/**************************************************************************************************/
/* cr_xgwan_top_wan_misc_gpon_gearbox_ptg_status2_sel:  -                                         */
/* cr_xgwan_top_wan_misc_gpon_gearbox_ptg_status1_sel:  -                                         */
/* cr_xgwan_top_wan_misc_gpon_gearbox_fifo_status_sel:  -                                         */
/* cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_txlbe_bit_order:  -                              */
/* cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_rx_16bit_order:  - This field changes the bit or */
/*                                                               der of the 16-bit Rx data exitin */
/*                                                               g theRx FIFO to GPON MAC.0: No c */
/*                                                               hanges1: Rx data is reversed fro */
/*                                                               m [15:0] to [0:15]               */
/* cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_8bit_order:  - This field changes the bit ord */
/*                                                              er of the 8-bit Tx data enteringt */
/*                                                              he Tx FIFO.0: No changes1: Tx dat */
/*                                                              a is reversed from [7:0] to [0:7] */
/* cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_16bit_order:  - This field changes the bit or */
/*                                                               der of the 16-bit Tx data exitin */
/*                                                               gthe Tx FIFO to ONU2G PMD.0: Bit */
/*                                                                 0 is transmitted first1: Bit 1 */
/*                                                               9 is transmitted first           */
/* cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_pointer_distance_min:  - Minimum distance all */
/*                                                                 %s%-38sowed between the Tx FIF */
/*                                                                 %s%-38sO write and readpointer */
/*                                                                 %s%-38ss.  The TXFIFO_DRIFTED  */
/*                                                                 %s%-38sstatus bit is asserted  */
/*                                                                 %s%-38sifTX_POINTER_DISTANCE g */
/*                                                                 %s%-38soes below this minimum  */
/*                                                                 %s%-29svalue.                  */
/* cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_pointer_distance_max:  - Maximum distance all */
/*                                                                 %s%-29sowed between the Tx FIF */
/*                                                                 %s%-29sO write and readpointer */
/*                                                                 %s%-29ss.  The TXFIFO_DRIFTED  */
/*                                                                 %s%-29sstatus bit is asserted  */
/*                                                                 %s%-29sifTX_POINTER_DISTANCE g */
/*                                                                 %s%-29soes above this maximum  */
/*                                                                 %s%-29svalue.                  */
/* cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_bit_inv:  - This bit enables logically invers */
/*                                                           ion of every Tx bit.                 */
/* cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_wr_ptr_dly:  - Delay Tx FIFO write pointer by */
/*                                                               1 location (8 Tx bits).  The poi */
/*                                                              nteris adjusted on every 0 to 1 t */
/*                                                              ransition in this register field. */
/* cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_wr_ptr_adv:  - Advance Tx FIFO write pointer  */
/*                                                              by 1 location (8 Tx bits).  Thepo */
/*                                                              inter is adjusted on every 0 to 1 */
/*                                                               transition in this registerfield */
/*                                                              .                                 */
/* cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_clear_txfifo_collision:  - if 1, the TXFIFO_COLL */
/*                                                                 %s%-39ISION status bit resets  */
/*                                                                 %s%-30sto 0.                   */
/* cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_loopback_rx:  - If 1, the output of Rx FIFO is l */
/*                                                            ooped back to the input of Tx FIFO. */
/*                                                            In this case, the SATA PHY Tx data  */
/*                                                            rate is the same as the Rx datarate */
/*                                                             regardless of whether Gen2 or Gen3 */
/*                                                             is selected.                       */
/* cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_clear_txfifo_drifted:  - If 1, the TXFIFO_DRIFTE */
/*                                                                 %s%-32sD statsu bit resets to  */
/* cr_xgwan_top_wan_misc_gpon_gearbox_sw_reset_txfifo_reset:  - If 1, the Tx FIFO goes into reset */
/*                                                           .                                    */
/* cr_xgwan_top_wan_misc_gpon_gearbox_sw_reset_txpg_reset:  - If 1, The Tx Pattern Generator goes */
/*                                                          into reset.                           */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean cr_xgwan_top_wan_misc_gpon_gearbox_ptg_status2_sel;
    bdmf_boolean cr_xgwan_top_wan_misc_gpon_gearbox_ptg_status1_sel;
    bdmf_boolean cr_xgwan_top_wan_misc_gpon_gearbox_fifo_status_sel;
    bdmf_boolean cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_txlbe_bit_order;
    bdmf_boolean cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_rx_16bit_order;
    bdmf_boolean cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_8bit_order;
    bdmf_boolean cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_16bit_order;
    uint8_t cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_pointer_distance_min;
    uint8_t cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_pointer_distance_max;
    bdmf_boolean cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_bit_inv;
    bdmf_boolean cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_wr_ptr_dly;
    bdmf_boolean cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_tx_wr_ptr_adv;
    bdmf_boolean cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_clear_txfifo_collision;
    bdmf_boolean cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_loopback_rx;
    bdmf_boolean cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_0_clear_txfifo_drifted;
    bdmf_boolean cr_xgwan_top_wan_misc_gpon_gearbox_sw_reset_txfifo_reset;
    bdmf_boolean cr_xgwan_top_wan_misc_gpon_gearbox_sw_reset_txpg_reset;
} gpon_gearbox_0;

bdmf_error_t ag_drv_gpon_gearbox_0_set(const gpon_gearbox_0 *gearbox_0);
bdmf_error_t ag_drv_gpon_gearbox_0_get(gpon_gearbox_0 *gearbox_0);
bdmf_error_t ag_drv_gpon_pattern_cfg1_set(uint8_t cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_filler, uint8_t cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_payload, uint8_t cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_header, uint8_t cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_pg_mode);
bdmf_error_t ag_drv_gpon_pattern_cfg1_get(uint8_t *cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_filler, uint8_t *cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_payload, uint8_t *cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_header, uint8_t *cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg1_pg_mode);
bdmf_error_t ag_drv_gpon_pattern_cfg2_set(uint8_t cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg2_gap_size, uint8_t cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg2_burst_size);
bdmf_error_t ag_drv_gpon_pattern_cfg2_get(uint8_t *cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg2_gap_size, uint8_t *cr_xgwan_top_wan_misc_gpon_gearbox_pattern_cfg2_burst_size);
bdmf_error_t ag_drv_gpon_gearbox_2_set(uint8_t cr_xgwan_top_wan_misc_gpon_gearbox_config_burst_delay_cyc, uint8_t cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_1_tx_wr_pointer, uint8_t cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_1_tx_rd_pointer);
bdmf_error_t ag_drv_gpon_gearbox_2_get(uint8_t *cr_xgwan_top_wan_misc_gpon_gearbox_config_burst_delay_cyc, uint8_t *cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_1_tx_wr_pointer, uint8_t *cr_xgwan_top_wan_misc_gpon_gearbox_fifo_cfg_1_tx_rd_pointer);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_gpon_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

