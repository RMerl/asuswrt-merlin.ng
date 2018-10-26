/*
   Copyright (c) 2015 Broadcom
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

#ifndef _XRDP_DRV_UBUS_SLV_AG_H_
#define _XRDP_DRV_UBUS_SLV_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* rgmii_mode_en: RGMII_MODE_EN - When set this bit enables RGMII interface.This bit acts as a re */
/*                set for RGMII block abd therefore it can be used to reset RGMII block when need */
/*                ed.                                                                             */
/* id_mode_dis: ID_MODE_DIS - RGMII Internal Delay (ID) mode disable.When set RGMII transmit cloc */
/*              k edges are aligned with the data.When cleared RGMII transmit clock edges are cen */
/*              tered in the middle of (transmit) data valid window.                              */
/* port_mode: PORT_MODE - Port Mode encoded as:000 : Internal EPHY (MII).001 : Internal GPHY (GMI */
/*            I/MII).010 : External EPHY (MII).011 : External GPHY (RGMII).100 : External RvMII.N */
/*            ot all combinations are applicable to all chips.                                    */
/* rvmii_ref_sel: RVMII_REF_SEL - Selects clock in RvMII mode.0 : RvMII reference clock is 50MHz. */
/*                1 : RvMII reference clock is 25MHz.                                             */
/* rx_pause_en: RX_PAUSE_EN - Rx Pause as negotiated by the attached PHY. Obtained by SW via MDIO */
/*              .                                                                                 */
/* tx_pause_en: TX_PAUSE_EN - Tx Pause as negotiated by the attached PHY. Obtained by SW via MDIO */
/*              .                                                                                 */
/* tx_clk_stop_en: TX_CLK_STOP_EN - hen set enables stopping TX_CLK after LPI is asserted. This b */
/*                 it should be set only when the connected EEE PHY supports it.                  */
/* lpi_count: LPI_COUNT - Specifies number of cycles after which TX_CLK will be stopped (after LP */
/*            I is asserted), if the clock stopping is enabled.                                   */
/* rx_err_mask: RX_ERR_MASK - When this bit is set to 1b1, RX_ERR signal toward the MAC is 1b0 (i */
/*              .e. no error). Applicable to MII/rvMII interfaces and used in case where link par */
/*              tner does not support RX_ERR.                                                     */
/* col_crs_mask: COL_CRS_MASK - COL/CRS mask. Encoded as:0 : COL/CRS signals toward the MAC are g */
/*               enerated internally to RGMII block based RX/TX activity, as per IEEE 802.3.1 : C */
/*               OL/CRS signals toward the MAC are both 1b0.Applicable to MII/rvMII interfaces an */
/*               d used in case where link partner or the chip does not support COL/CRS.This bit  */
/*               is only valid when PSEUDO_HD_MODE_EN is set to 1b1.Note that as per IEEE 802.3 M */
/*               ACs ignore COL/CRS in full-duplex mode and therefore it is not necessary require */
/*               d to set this bit.                                                               */
/* pseudo_hd_mode_en: PSEUDO_HD_MODE_EN - Pseudo half duplex mode enable. Encoded as-0:COL/CRS si */
/*                    gnals toward the MAC are coming from chip input pads1:COL/CRS signals towar */
/*                    d the MAC are generated based on COL_CRS_MASK programming                   */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean rgmii_mode_en;
    bdmf_boolean id_mode_dis;
    uint8_t port_mode;
    bdmf_boolean rvmii_ref_sel;
    bdmf_boolean rx_pause_en;
    bdmf_boolean tx_pause_en;
    bdmf_boolean tx_clk_stop_en;
    uint8_t lpi_count;
    bdmf_boolean rx_err_mask;
    bdmf_boolean col_crs_mask;
    bdmf_boolean pseudo_hd_mode_en;
} ubus_slv__cntrl;


/**************************************************************************************************/
/* ctri: CTRI - Charge pump current control. Contact BRCM for more information                    */
/* drng: DRNG - VCDL control. Contact BRCM for more information                                   */
/* iddq: IDDQ - When set puts 2ns delay line in IDDQ mode. Requires HW reset (see bit 8 of this r */
/*       egister) to bring 2ns delay line from power down.                                        */
/* bypass: BYPASS - When set it puts 2ns delay line in bypass mode (default). This bit should be  */
/*         cleared only in non-ID mode.                                                           */
/* dly_sel: DLY_SEL - When set delay line delay is ~2ns and when cleared delay line is > 2.2ns. V */
/*          alid only when DLY_OVERRIDE bit is set.                                               */
/* dly_override: DLY_OVERRIDE - Overrides HW selected delay.                                      */
/* reset: RESET - When set it resets 2ns delay line.                                              */
/**************************************************************************************************/
typedef struct
{
    uint8_t ctri;
    uint8_t drng;
    bdmf_boolean iddq;
    bdmf_boolean bypass;
    bdmf_boolean dly_sel;
    bdmf_boolean dly_override;
    bdmf_boolean reset;
} ubus_slv__rx_clock_delay_cntrl;


/**************************************************************************************************/
/* expected_data_0: EXPECTED_DATA_0 - Data expected on the even rising edge of the RXC clock on t */
/*                  he RGMII Rx interface. Bits[3:0] of this register are used only in MII modes  */
/*                  and they represent RXD[3:0]. Bit 8 corresponds RX_ER.Not used in Packet Gener */
/*                  ation mode.                                                                   */
/* expected_data_1: EXPECTED_DATA_1 - Data expected on the odd rising edge of the RXC clock on th */
/*                  e RGMII Rx interface. Bits[12:9] of this register are used only in MII modes  */
/*                  and they represent RXD[3:0]. Bit 17 corresponds RX_ER.Not used in Packet Gene */
/*                  ration mode.                                                                  */
/* good_count: GOOD_COUNT - Count that specifies how many consecutive {EXPECTED_DATA_0, EXPECTED_ */
/*             DATA_1, EXPECTED_DATA_2, EXPECTED_DATA_3 } patterns should be received before RX_O */
/*             K signal is asserted.In packet generation mode it specifies number of expected pac */
/*             kets.                                                                              */
/* pkt_count_rst: PKT_COUNT_RST - When set resets received packets counter. Used only in packet g */
/*                eneration mode (PKT_GEN_MODE bit is set).                                       */
/* ate_en: ATE_EN - When set enables ATE testing                                                  */
/**************************************************************************************************/
typedef struct
{
    uint16_t expected_data_0;
    uint16_t expected_data_1;
    uint8_t good_count;
    bdmf_boolean pkt_count_rst;
    bdmf_boolean ate_en;
} ubus_slv__ate_rx_cntrl_exp_data;


/**************************************************************************************************/
/* start_stop_ovrd: START_STOP_OVRD - START_STOP override. When this bit is set, transmit state m */
/*                  achine will be controlled by START_STOP bit of this register instead of the c */
/*                  hip pin.                                                                      */
/* start_stop: START_STOP - start_stop. When set transmit state matchin starts outputing programm */
/*             ed pattern over RGMII TX interface. When cleared transmit state machine stops outp */
/*             utting data.                                                                       */
/* pkt_gen_en: PKT_GEN_EN - When this bit is set ATE test logic operates in the packet generation */
/*              mode.                                                                             */
/* pkt_cnt: PKT_CNT - Number of packets generated when START_STOP bit is set.  When program to 0  */
/*          it means infinite number of packets will be transmit (i.e. until START_STOP is cleare */
/*          d).                                                                                   */
/* payload_length: PAYLOAD_LENGTH - Generated packet payload in bytes. Must be between 46B and 15 */
/*                 00B.                                                                           */
/* pkt_ipg: PKT_IPG - Inter-packet gap in packet generation mode.                                 */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean start_stop_ovrd;
    bdmf_boolean start_stop;
    bdmf_boolean pkt_gen_en;
    uint8_t pkt_cnt;
    uint16_t payload_length;
    uint8_t pkt_ipg;
} ubus_slv__ate_tx_cntrl;


/**************************************************************************************************/
/* txd0_del_sel: TXD0_DEL_SEL - txd0 CKTAP delay control. Refer to the CKTAP datasheet for progra */
/*               mming                                                                            */
/* txd0_del_ovrd_en: TXD0_DEL_OVRD_EN - txd0 CKTAP delay override enable. When set enables CKTAP  */
/*                   delay to be controlled from this register.                                   */
/* txd1_del_sel: TXD1_DEL_SEL - txd1 CKTAP delay control. Refer to the CKTAP datasheet for progra */
/*               mming                                                                            */
/* txd1_del_ovrd_en: TXD1_DEL_OVRD_EN - txd1 CKTAP delay override enable. When set enables CKTAP  */
/*                   delay to be controlled from this register.                                   */
/* txd2_del_sel: TXD2_DEL_SEL - txd2 CKTAP delay control. Refer to the CKTAP datasheet for progra */
/*               mming                                                                            */
/* txd2_del_ovrd_en: TXD2_DEL_OVRD_EN - txd2 CKTAP delay override enable. When set enables CKTAP  */
/*                   delay to be controlled from this register.                                   */
/* txd3_del_sel: TXD3_DEL_SEL - txd3 CKTAP delay control. Refer to the CKTAP datasheet for progra */
/*               mming                                                                            */
/* txd3_del_ovrd_en: TXD3_DEL_OVRD_EN - txd3 CKTAP delay override enable. When set enables CKTAP  */
/*                   delay to be controlled from this register.                                   */
/**************************************************************************************************/
typedef struct
{
    uint8_t txd0_del_sel;
    bdmf_boolean txd0_del_ovrd_en;
    uint8_t txd1_del_sel;
    bdmf_boolean txd1_del_ovrd_en;
    uint8_t txd2_del_sel;
    bdmf_boolean txd2_del_ovrd_en;
    uint8_t txd3_del_sel;
    bdmf_boolean txd3_del_ovrd_en;
} ubus_slv__tx_delay_cntrl_0;


/**************************************************************************************************/
/* txctl_del_sel: TXCTL_DEL_SEL - txctl CKTAP delay control. Refer to the CKTAP datasheet for pro */
/*                gramming.                                                                       */
/* txctl_del_ovrd_en: TXCTL_DEL_OVRD_EN - txctl CKTAP delay override enable. When set enables CKT */
/*                    AP delay to be controlled from this register.                               */
/* txclk_del_sel: TXCLK_DEL_SEL - txclk NON-ID mode CKTAP delay control. Refer to the CKTAP datas */
/*                heet for programming.                                                           */
/* txclk_del_ovrd_en: TXCLK_DEL_OVRD_EN - txclk NON_ID mode CKTAP delay override enable. When set */
/*                     enables CKTAP delay to be controlled from this register.                   */
/* txclk_id_del_sel: TXCLK_ID_DEL_SEL - txclk ID mode CKTAP delay control. Refer to the CKTAP dat */
/*                   asheet for programming.                                                      */
/* txclk_id_del_ovrd_en: TXCLK_ID_DEL_OVRD_EN - txclk ID mode CKTAP delay override enable. When s */
/*                       et enables CKTAP delay to be controlled from this register.              */
/**************************************************************************************************/
typedef struct
{
    uint8_t txctl_del_sel;
    bdmf_boolean txctl_del_ovrd_en;
    uint8_t txclk_del_sel;
    bdmf_boolean txclk_del_ovrd_en;
    uint8_t txclk_id_del_sel;
    bdmf_boolean txclk_id_del_ovrd_en;
} ubus_slv__tx_delay_cntrl_1;


/**************************************************************************************************/
/* rxd0_del_sel: RXD0_DEL_SEL - rxd0 CKTAP delay control. Refer to the CKTAP datasheet for progra */
/*               mming                                                                            */
/* rxd0_del_ovrd_en: RXD0_DEL_OVRD_EN - rxd0 CKTAP delay override enable. When set enables CKTAP  */
/*                   delay to be controlled from this register.                                   */
/* rxd1_del_sel: RXD1_DEL_SEL - rxd1 CKTAP delay control. Refer to the CKTAP datasheet for progra */
/*               mming                                                                            */
/* rxd1_del_ovrd_en: RXD1_DEL_OVRD_EN - rxd1 CKTAP delay override enable. When set enables CKTAP  */
/*                   delay to be controlled from this register.                                   */
/* rxd2_del_sel: RXD2_DEL_SEL - rxd2 CKTAP delay control. Refer to the CKTAP datasheet for progra */
/*               mming                                                                            */
/* rxd2_del_ovrd_en: RXD2_DEL_OVRD_EN - rxd2 CKTAP delay override enable. When set enables CKTAP  */
/*                   delay to be controlled from this register.                                   */
/* rxd3_del_sel: RXD3_DEL_SEL - rxd3 CKTAP delay control. Refer to the CKTAP datasheet for progra */
/*               mming                                                                            */
/* rxd3_del_ovrd_en: RXD3_DEL_OVRD_EN - rxd3 CKTAP delay override enable. When set enables CKTAP  */
/*                   delay to be controlled from this register.                                   */
/**************************************************************************************************/
typedef struct
{
    uint8_t rxd0_del_sel;
    bdmf_boolean rxd0_del_ovrd_en;
    uint8_t rxd1_del_sel;
    bdmf_boolean rxd1_del_ovrd_en;
    uint8_t rxd2_del_sel;
    bdmf_boolean rxd2_del_ovrd_en;
    uint8_t rxd3_del_sel;
    bdmf_boolean rxd3_del_ovrd_en;
} ubus_slv__rx_delay_cntrl_0;


/**************************************************************************************************/
/* rxd4_del_sel: RXD4_DEL_SEL - rxd4 CKTAP delay control. Refer to the CKTAP datasheet for progra */
/*               mming                                                                            */
/* rxd4_del_ovrd_en: RXD4_DEL_OVRD_EN - rxd4 CKTAP delay override enable. When set enables CKTAP  */
/*                   delay to be controlled from this register.                                   */
/* rxd5_del_sel: RXD5_DEL_SEL - rxd5 CKTAP delay control. Refer to the CKTAP datasheet for progra */
/*               mming                                                                            */
/* rxd5_del_ovrd_en: RXD5_DEL_OVRD_EN - rxd5 CKTAP delay override enable. When set enables CKTAP  */
/*                   delay to be controlled from this register.                                   */
/* rxd6_del_sel: RXD6_DEL_SEL - rxd6 CKTAP delay control. Refer to the CKTAP datasheet for progra */
/*               mming                                                                            */
/* rxd6_del_ovrd_en: RXD6_DEL_OVRD_EN - rxd6 CKTAP delay override enable. When set enables CKTAP  */
/*                   delay to be controlled from this register.                                   */
/* rxd7_del_sel: RXD7_DEL_SEL - rxd7 CKTAP delay control. Refer to the CKTAP datasheet for progra */
/*               mming                                                                            */
/* rxd7_del_ovrd_en: RXD7_DEL_OVRD_EN - rxd7 CKTAP delay override enable. When set enables CKTAP  */
/*                   delay to be controlled from this register.                                   */
/**************************************************************************************************/
typedef struct
{
    uint8_t rxd4_del_sel;
    bdmf_boolean rxd4_del_ovrd_en;
    uint8_t rxd5_del_sel;
    bdmf_boolean rxd5_del_ovrd_en;
    uint8_t rxd6_del_sel;
    bdmf_boolean rxd6_del_ovrd_en;
    uint8_t rxd7_del_sel;
    bdmf_boolean rxd7_del_ovrd_en;
} ubus_slv__rx_delay_cntrl_1;


/**************************************************************************************************/
/* rxctl_pos_del_sel: RXCTL_POS_DEL_SEL - rxctl_pos CKTAP delay control. Refer to the CKTAP datas */
/*                    heet for programming                                                        */
/* rxctl_pos_del_ovrd_en: RXCTL_POS_DEL_OVRD_EN - rxctl_pos CKTAP delay override enable. When set */
/*                         enables CKTAP delay to be controlled from this register.               */
/* rxctl_neg_del_sel: RXCTL_NEG_DEL_SEL - rxctl_neg CKTAP delay control. Refer to the CKTAP datas */
/*                    heet for programming.                                                       */
/* rxctl_neg_del_ovrd_en: RXCTL_NEG_DEL_OVRD_EN - rxctl_neg CKTAP delay override enable. When set */
/*                         enables CKTAP delay to be controlled from this register.               */
/* rxclk_del_sel: RXCLK_DEL_SEL - rxclk CKTAP delay control. Refer to the CKTAP datasheet for pro */
/*                gramming.                                                                       */
/* rxclk_del_ovrd_en: RXCLK_DEL_OVRD_EN - rxclk CKTAP delay override enable. When set enables CKT */
/*                    AP delay to be controlled from this register.                               */
/**************************************************************************************************/
typedef struct
{
    uint8_t rxctl_pos_del_sel;
    bdmf_boolean rxctl_pos_del_ovrd_en;
    uint8_t rxctl_neg_del_sel;
    bdmf_boolean rxctl_neg_del_ovrd_en;
    uint8_t rxclk_del_sel;
    bdmf_boolean rxclk_del_ovrd_en;
} ubus_slv__rx_delay_cntrl_2;

bdmf_error_t ag_drv_ubus_slv_vpb_base_set(uint32_t base);
bdmf_error_t ag_drv_ubus_slv_vpb_base_get(uint32_t *base);
bdmf_error_t ag_drv_ubus_slv_vpb_mask_set(uint32_t mask);
bdmf_error_t ag_drv_ubus_slv_vpb_mask_get(uint32_t *mask);
bdmf_error_t ag_drv_ubus_slv_apb_base_set(uint32_t base);
bdmf_error_t ag_drv_ubus_slv_apb_base_get(uint32_t *base);
bdmf_error_t ag_drv_ubus_slv_apb_mask_set(uint32_t mask);
bdmf_error_t ag_drv_ubus_slv_apb_mask_get(uint32_t *mask);
bdmf_error_t ag_drv_ubus_slv_dqm_base_set(uint32_t base);
bdmf_error_t ag_drv_ubus_slv_dqm_base_get(uint32_t *base);
bdmf_error_t ag_drv_ubus_slv_dqm_mask_set(uint32_t mask);
bdmf_error_t ag_drv_ubus_slv_dqm_mask_get(uint32_t *mask);
bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_isr_set(uint32_t ist);
bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_isr_get(uint32_t *ist);
bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_ism_get(uint32_t *ism);
bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_ier_set(uint32_t iem);
bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_ier_get(uint32_t *iem);
bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_itr_set(uint32_t ist);
bdmf_error_t ag_drv_ubus_slv_rnr_intr_ctrl_itr_get(uint32_t *ist);
bdmf_error_t ag_drv_ubus_slv_profiling_cfg_set(bdmf_boolean counter_enable, bdmf_boolean profiling_start, bdmf_boolean manual_stop_mode, bdmf_boolean do_manual_stop);
bdmf_error_t ag_drv_ubus_slv_profiling_cfg_get(bdmf_boolean *counter_enable, bdmf_boolean *profiling_start, bdmf_boolean *manual_stop_mode, bdmf_boolean *do_manual_stop);
bdmf_error_t ag_drv_ubus_slv_profiling_status_get(bdmf_boolean *profiling_on, uint32_t *cycles_counter);
bdmf_error_t ag_drv_ubus_slv_profiling_counter_get(uint32_t *val);
bdmf_error_t ag_drv_ubus_slv_profiling_start_value_get(uint32_t *val);
bdmf_error_t ag_drv_ubus_slv_profiling_stop_value_get(uint32_t *val);
bdmf_error_t ag_drv_ubus_slv_profiling_cycle_num_set(uint32_t profiling_cycles_num);
bdmf_error_t ag_drv_ubus_slv_profiling_cycle_num_get(uint32_t *profiling_cycles_num);
bdmf_error_t ag_drv_ubus_slv__cntrl_set(const ubus_slv__cntrl *_cntrl);
bdmf_error_t ag_drv_ubus_slv__cntrl_get(ubus_slv__cntrl *_cntrl);
bdmf_error_t ag_drv_ubus_slv__ib_status_set(uint8_t speed_decode, bdmf_boolean duplex_decode, bdmf_boolean link_decode, bdmf_boolean ib_status_ovrd);
bdmf_error_t ag_drv_ubus_slv__ib_status_get(uint8_t *speed_decode, bdmf_boolean *duplex_decode, bdmf_boolean *link_decode, bdmf_boolean *ib_status_ovrd);
bdmf_error_t ag_drv_ubus_slv__rx_clock_delay_cntrl_set(const ubus_slv__rx_clock_delay_cntrl *_rx_clock_delay_cntrl);
bdmf_error_t ag_drv_ubus_slv__rx_clock_delay_cntrl_get(ubus_slv__rx_clock_delay_cntrl *_rx_clock_delay_cntrl);
bdmf_error_t ag_drv_ubus_slv__ate_rx_cntrl_exp_data_set(const ubus_slv__ate_rx_cntrl_exp_data *_ate_rx_cntrl_exp_data);
bdmf_error_t ag_drv_ubus_slv__ate_rx_cntrl_exp_data_get(ubus_slv__ate_rx_cntrl_exp_data *_ate_rx_cntrl_exp_data);
bdmf_error_t ag_drv_ubus_slv__ate_rx_exp_data_1_set(uint16_t expected_data_2, uint16_t expected_data_3);
bdmf_error_t ag_drv_ubus_slv__ate_rx_exp_data_1_get(uint16_t *expected_data_2, uint16_t *expected_data_3);
bdmf_error_t ag_drv_ubus_slv__ate_rx_status_0_get(uint16_t *received_data_0, uint16_t *received_data_1, bdmf_boolean *rx_ok);
bdmf_error_t ag_drv_ubus_slv__ate_rx_status_1_get(uint16_t *received_data_2, uint16_t *received_data_3);
bdmf_error_t ag_drv_ubus_slv__ate_tx_cntrl_set(const ubus_slv__ate_tx_cntrl *_ate_tx_cntrl);
bdmf_error_t ag_drv_ubus_slv__ate_tx_cntrl_get(ubus_slv__ate_tx_cntrl *_ate_tx_cntrl);
bdmf_error_t ag_drv_ubus_slv__ate_tx_data_0_set(uint16_t tx_data_0, uint16_t tx_data_1);
bdmf_error_t ag_drv_ubus_slv__ate_tx_data_0_get(uint16_t *tx_data_0, uint16_t *tx_data_1);
bdmf_error_t ag_drv_ubus_slv__ate_tx_data_1_set(uint16_t tx_data_2, uint16_t tx_data_3);
bdmf_error_t ag_drv_ubus_slv__ate_tx_data_1_get(uint16_t *tx_data_2, uint16_t *tx_data_3);
bdmf_error_t ag_drv_ubus_slv__ate_tx_data_2_set(uint8_t tx_data_4, uint8_t tx_data_5, uint16_t ether_type);
bdmf_error_t ag_drv_ubus_slv__ate_tx_data_2_get(uint8_t *tx_data_4, uint8_t *tx_data_5, uint16_t *ether_type);
bdmf_error_t ag_drv_ubus_slv__tx_delay_cntrl_0_set(const ubus_slv__tx_delay_cntrl_0 *_tx_delay_cntrl_0);
bdmf_error_t ag_drv_ubus_slv__tx_delay_cntrl_0_get(ubus_slv__tx_delay_cntrl_0 *_tx_delay_cntrl_0);
bdmf_error_t ag_drv_ubus_slv__tx_delay_cntrl_1_set(const ubus_slv__tx_delay_cntrl_1 *_tx_delay_cntrl_1);
bdmf_error_t ag_drv_ubus_slv__tx_delay_cntrl_1_get(ubus_slv__tx_delay_cntrl_1 *_tx_delay_cntrl_1);
bdmf_error_t ag_drv_ubus_slv__rx_delay_cntrl_0_set(const ubus_slv__rx_delay_cntrl_0 *_rx_delay_cntrl_0);
bdmf_error_t ag_drv_ubus_slv__rx_delay_cntrl_0_get(ubus_slv__rx_delay_cntrl_0 *_rx_delay_cntrl_0);
bdmf_error_t ag_drv_ubus_slv__rx_delay_cntrl_1_set(const ubus_slv__rx_delay_cntrl_1 *_rx_delay_cntrl_1);
bdmf_error_t ag_drv_ubus_slv__rx_delay_cntrl_1_get(ubus_slv__rx_delay_cntrl_1 *_rx_delay_cntrl_1);
bdmf_error_t ag_drv_ubus_slv__rx_delay_cntrl_2_set(const ubus_slv__rx_delay_cntrl_2 *_rx_delay_cntrl_2);
bdmf_error_t ag_drv_ubus_slv__rx_delay_cntrl_2_get(ubus_slv__rx_delay_cntrl_2 *_rx_delay_cntrl_2);
bdmf_error_t ag_drv_ubus_slv__clk_rst_ctrl_set(bdmf_boolean swinit, bdmf_boolean clk250en);
bdmf_error_t ag_drv_ubus_slv__clk_rst_ctrl_get(bdmf_boolean *swinit, bdmf_boolean *clk250en);

#ifdef USE_BDMF_SHELL
enum
{
    cli_ubus_slv_vpb_base,
    cli_ubus_slv_vpb_mask,
    cli_ubus_slv_apb_base,
    cli_ubus_slv_apb_mask,
    cli_ubus_slv_dqm_base,
    cli_ubus_slv_dqm_mask,
    cli_ubus_slv_rnr_intr_ctrl_isr,
    cli_ubus_slv_rnr_intr_ctrl_ism,
    cli_ubus_slv_rnr_intr_ctrl_ier,
    cli_ubus_slv_rnr_intr_ctrl_itr,
    cli_ubus_slv_profiling_cfg,
    cli_ubus_slv_profiling_status,
    cli_ubus_slv_profiling_counter,
    cli_ubus_slv_profiling_start_value,
    cli_ubus_slv_profiling_stop_value,
    cli_ubus_slv_profiling_cycle_num,
    cli_ubus_slv__cntrl,
    cli_ubus_slv__ib_status,
    cli_ubus_slv__rx_clock_delay_cntrl,
    cli_ubus_slv__ate_rx_cntrl_exp_data,
    cli_ubus_slv__ate_rx_exp_data_1,
    cli_ubus_slv__ate_rx_status_0,
    cli_ubus_slv__ate_rx_status_1,
    cli_ubus_slv__ate_tx_cntrl,
    cli_ubus_slv__ate_tx_data_0,
    cli_ubus_slv__ate_tx_data_1,
    cli_ubus_slv__ate_tx_data_2,
    cli_ubus_slv__tx_delay_cntrl_0,
    cli_ubus_slv__tx_delay_cntrl_1,
    cli_ubus_slv__rx_delay_cntrl_0,
    cli_ubus_slv__rx_delay_cntrl_1,
    cli_ubus_slv__rx_delay_cntrl_2,
    cli_ubus_slv__clk_rst_ctrl,
};

int bcm_ubus_slv_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_ubus_slv_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

