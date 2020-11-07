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

#ifndef _BCM6858_MISC_AG_H_
#define _BCM6858_MISC_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif

/**************************************************************************************************/
/* cr_xgwan_top_wan_misc_pmd_lane_mode:  -                                                        */
/* cr_xgwan_top_wan_misc_epon_tx_fifo_off:  -                                                     */
/* cr_xgwan_top_wan_misc_onu2g_phya:  -                                                           */
/* cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld:  -                                                  */
/* cr_xgwan_top_wan_misc_mdio_fast_mode:  -                                                       */
/* cr_xgwan_top_wan_misc_mdio_mode:  -                                                            */
/* cr_xgwan_top_wan_misc_refout_en:  -                                                            */
/* cr_xgwan_top_wan_misc_refin_en:  -                                                             */
/* cr_xgwan_top_wan_misc_onu2g_pmd_status_sel:  -                                                 */
/**************************************************************************************************/
typedef struct
{
    uint16_t cr_xgwan_top_wan_misc_pmd_lane_mode;
    uint8_t cr_xgwan_top_wan_misc_epon_tx_fifo_off;
    uint8_t cr_xgwan_top_wan_misc_onu2g_phya;
    bdmf_boolean cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld;
    bdmf_boolean cr_xgwan_top_wan_misc_mdio_fast_mode;
    bdmf_boolean cr_xgwan_top_wan_misc_mdio_mode;
    bdmf_boolean cr_xgwan_top_wan_misc_refout_en;
    bdmf_boolean cr_xgwan_top_wan_misc_refin_en;
    bdmf_boolean cr_xgwan_top_wan_misc_onu2g_pmd_status_sel;
} misc_misc_0;


/**************************************************************************************************/
/* cfg_apm_mux_sel_1:  - 0: Select MUX 0 output for wan_rbc_for_apm. 1: ntr_sync_pulse forwan_rbc */
/*                    _for_apm.                                                                   */
/* cfg_apm_mux_sel_0:  - 0: Select ncoProgClk for MUX 0 output.  1: Select ncoClk8KHz for MUX0 ou */
/*                    tput.                                                                       */
/* cfgngponrxclk:  - 0: Selects divide-by-2 clock divider for mac_rx_clk.  1: Selectsdivide-by-4  */
/*                clock divider for mac_rx_clk. 2: Selects divide-by-1clock divider. 3: Unused.   */
/* cfgngpontxclk:  - 0: Selects divide-by-2 clock divider for mac_tx_clk.  1: Selectsdivide-by-4  */
/*                clock divider for mac_tx_clk. 2: Selects divide-by-1clock divider. 3: Unused.   */
/* cfgactiveethernet2p5:  - 0: Selects divide-by-2 clock divider for clkRbc125. 1: Selectsdivide- */
/*                       by-1 clock divider for clkRbc125.                                        */
/* cr_xgwan_top_wan_misc_pmd_rx_osr_mode:  -                                                      */
/* cr_xgwan_top_wan_misc_pmd_tx_mode:  -                                                          */
/* cr_xgwan_top_wan_misc_pmd_tx_osr_mode:  -                                                      */
/* cr_xgwan_top_wan_misc_pmd_tx_disable:  -                                                       */
/* cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn:  -                                                    */
/* cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn:  -                                                    */
/* cr_xgwan_top_wan_misc_pmd_ext_los:  -                                                          */
/* cr_xgwan_top_wan_misc_pmd_por_h_rstb:  -                                                       */
/* cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb:  -                                                 */
/* cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb:  -                                                 */
/* cr_xgwan_top_wan_misc_pmd_ln_h_rstb:  -                                                        */
/* cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb:  -                                                     */
/* cr_xgwan_top_wan_misc_pmd_rx_mode:  -                                                          */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean cfg_apm_mux_sel_1;
    bdmf_boolean cfg_apm_mux_sel_0;
    uint8_t cfgngponrxclk;
    uint8_t cfgngpontxclk;
    bdmf_boolean cfgactiveethernet2p5;
    uint8_t cr_xgwan_top_wan_misc_pmd_rx_osr_mode;
    uint8_t cr_xgwan_top_wan_misc_pmd_tx_mode;
    uint8_t cr_xgwan_top_wan_misc_pmd_tx_osr_mode;
    bdmf_boolean cr_xgwan_top_wan_misc_pmd_tx_disable;
    bdmf_boolean cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn;
    bdmf_boolean cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn;
    bdmf_boolean cr_xgwan_top_wan_misc_pmd_ext_los;
    bdmf_boolean cr_xgwan_top_wan_misc_pmd_por_h_rstb;
    bdmf_boolean cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb;
    bdmf_boolean cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb;
    bdmf_boolean cr_xgwan_top_wan_misc_pmd_ln_h_rstb;
    bdmf_boolean cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb;
    bdmf_boolean cr_xgwan_top_wan_misc_pmd_rx_mode;
} misc_misc_2;


/**************************************************************************************************/
/* cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel:  -                                               */
/* cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel:  -                                                */
/* cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel:  -                                          */
/* cr_xgwan_top_wan_misc_wan_cfg_laser_oe:  -                                                     */
/* cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select:  - The lower bit selects the speed, and th */
/*                                                     e upper bit selects thetechnology. 0: EPON */
/*                                                     . 1: 10G EPON. 2: GPON. 3: NGPON.          */
/* cr_xgwan_top_wan_misc_wan_cfg_laser_mode:  - Bit 0 selects the speed, and bit 1 selects the te */
/*                                           chnology.  0: EPON.1: 10G EPON. 2: GPON. 3: NGPON. 4 */
/*                                            (or higher): Disable laser.                         */
/* cr_xgwan_top_wan_misc_wan_cfg_laser_invert:  -                                                 */
/* cr_xgwan_top_wan_misc_wan_cfg_mem_reb:  -                                                      */
/**************************************************************************************************/
typedef struct
{
    uint8_t cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel;
    uint8_t cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel;
    uint8_t cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel;
    bdmf_boolean cr_xgwan_top_wan_misc_wan_cfg_laser_oe;
    uint8_t cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select;
    uint8_t cr_xgwan_top_wan_misc_wan_cfg_laser_mode;
    bdmf_boolean cr_xgwan_top_wan_misc_wan_cfg_laser_invert;
    bdmf_boolean cr_xgwan_top_wan_misc_wan_cfg_mem_reb;
} misc_misc_3;

bdmf_error_t ag_drv_misc_misc_0_set(const misc_misc_0 *misc_0);
bdmf_error_t ag_drv_misc_misc_0_get(misc_misc_0 *misc_0);
bdmf_error_t ag_drv_misc_misc_1_set(uint16_t cr_xgwan_top_wan_misc_pmd_core_1_mode, uint16_t cr_xgwan_top_wan_misc_pmd_core_0_mode);
bdmf_error_t ag_drv_misc_misc_1_get(uint16_t *cr_xgwan_top_wan_misc_pmd_core_1_mode, uint16_t *cr_xgwan_top_wan_misc_pmd_core_0_mode);
bdmf_error_t ag_drv_misc_misc_2_set(const misc_misc_2 *misc_2);
bdmf_error_t ag_drv_misc_misc_2_get(misc_misc_2 *misc_2);
bdmf_error_t ag_drv_misc_misc_3_set(const misc_misc_3 *misc_3);
bdmf_error_t ag_drv_misc_misc_3_get(misc_misc_3 *misc_3);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_misc_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

