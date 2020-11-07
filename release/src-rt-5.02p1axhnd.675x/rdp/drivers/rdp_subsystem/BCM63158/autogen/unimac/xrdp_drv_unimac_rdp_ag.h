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

#ifndef _XRDP_DRV_UNIMAC_RDP_AG_H_
#define _XRDP_DRV_UNIMAC_RDP_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* runt_filter_dis:  - 1: Disable RX side RUNT filtering.0: Enable  RUNT filtering.               */
/* txrx_en_config:  - This mode only works in auto-config mode:0: After auto-config, TX_ENA and R */
/*                 X_ENA bits are set to 1.1: After auto-config, TX_ENA and RX_ENA bits are set t */
/*                 o 0,meaning SW will have to come in and enable TX and RX.                      */
/* tx_pause_ignore:  - Ignore TX PAUSE frame transmit request.                                    */
/* prbl_ena:  - 1: Enable extract/insert of EFM headers.                                          */
/* rx_err_disc:  - This bit currently not used.                                                   */
/* rmt_loop_ena:  - 1: Enable remote loopback at the fifo system side.0: Normal operation.        */
/* no_lgth_check:  - Payload length check.0: Check payload length with Length/Type field.1: Check */
/*                 disabled.                                                                      */
/* cntl_frm_ena:  - MAC control frame enable.1: MAC control frames with opcode other than 0x0001  */
/*               are accepted and forwarded to the client interface.0: MAC control frames with op */
/*               code other than 0x0000 and 0x0001 are silently discarded.                        */
/* ena_ext_config:  - Enable/Disable auto-configuration.1: Enable 0: Disable                      */
/* lcl_loop_ena:  - Enable GMII/MII loopback1: Loopback enabled.0: Normal operation.              */
/* sw_reset:  - 1: RX and RX engines are put in reset.0: come out of SW reset.                    */
/* hd_ena:  - Ignored when RTH_SPEED[1]==1, gigabit mode.1: half duplex0: full duplex             */
/* tx_addr_ins:  - 1: The MAC overwrites the source MAC address with a programmed MAC address in  */
/*              register MAC_0 and MAC_1.0: Not modify the source address received from the trans */
/*              mit application client.                                                           */
/* rx_pause_ignore:  - 1: Receive PAUSE frames are ignored by the MAC.0: The tramsmit process is  */
/*                  stiooed for the amount of time specified in the pause wuanta received within  */
/*                  the PAUSE frame.                                                              */
/* pause_fwd:  - 1: PAUSE frames are forwarded to the user application.0: The PAUSE frames are te */
/*            rminated and discarded in the MAC.                                                  */
/* crc_fwd:  - 1: The CRC field of received frames is transmitted to the user application.0: The  */
/*          CRC field is stripped from the frame.                                                 */
/* pad_en:  - 1: Padding is removed along with crc field before the frame is sent to the user app */
/*         lication.0: No padding is removed by the MAC.                                          */
/* promis_en:  - 1: All frames are received without Unicast address filtering.0:                  */
/* eth_speed:  - 00: 10Mbps, 01: 100Mbps, 10: 1000Mbps, 11: 2500Mbps                              */
/* rx_ena:  - 1: The MAC receive function is enabled.0: The MAC receive function is disabled.The  */
/*         enable works on packet boundary meaning that only on the assertion on the bit during e */
/*         very 0->1 transition of rx_dv.                                                         */
/* tx_ena:  - 1: The MAC transmit function is enabled.0: The MAC transmit function is disabled.Th */
/*         e enable works on packet boundary meaning that only on the assertion of the bit during */
/*          every SOP.                                                                            */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean runt_filter_dis;
    bdmf_boolean txrx_en_config;
    bdmf_boolean tx_pause_ignore;
    bdmf_boolean prbl_ena;
    bdmf_boolean rx_err_disc;
    bdmf_boolean rmt_loop_ena;
    bdmf_boolean no_lgth_check;
    bdmf_boolean cntl_frm_ena;
    bdmf_boolean ena_ext_config;
    bdmf_boolean lcl_loop_ena;
    bdmf_boolean sw_reset;
    bdmf_boolean hd_ena;
    bdmf_boolean tx_addr_ins;
    bdmf_boolean rx_pause_ignore;
    bdmf_boolean pause_fwd;
    bdmf_boolean crc_fwd;
    bdmf_boolean pad_en;
    bdmf_boolean promis_en;
    uint8_t eth_speed;
    bdmf_boolean rx_ena;
    bdmf_boolean tx_ena;
} unimac_rdp_cmd;


/**************************************************************************************************/
/* mac_link_stat:  - Link status indication.                                                      */
/* mac_tx_pause:  - 1: MAC Tx pause enabled.0: MAC Tx pause disabled.                             */
/* mac_rx_pause:  - 1: MAC Rx pause enabled.0: MAC Rx pause disabled.                             */
/* mac_duplex:  - 1: Half duplex.0: Full duplex.                                                  */
/* mac_speed:  - 00: 10Mbps, 01: 100Mbps, 10: 1Gbps, 11: 2.5Gbps                                  */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean mac_link_stat;
    bdmf_boolean mac_tx_pause;
    bdmf_boolean mac_rx_pause;
    bdmf_boolean mac_duplex;
    uint8_t mac_speed;
} unimac_rdp_mode;


/**************************************************************************************************/
/* lp_idle_prediction_mode:  - 1: Enable LP_IDLE prediction. 
0: Disable LP_IDLE prediction       */
/* dis_eee_10m:  - 1: When this bit is set and link is established at 10Mbp, LPI is not supported */
/*              (saving is achieved by reduced PHY's output swing). UNIMAC ignores EEE feature on */
/*               both Tx & Rx in 10Mbps. 
0: When cleared, Unimac doesn't differentiate between s */
/*              peeds for EEE features.                                                           */
/* eee_txclk_dis:  - If enabled, UNIMAC will shut down TXCLK to PHY, when in LPI state.           */
/* rx_fifo_check:  - If enabled, lpi_rx_detect is set whenever the LPI_IDELS are being received o */
/*                n the RX line and Unimac Rx FIFO is empty.
By default, lpi_rx_detect is set onl */
/*                y whenever the LPI_IDLES are being received on the RX line.                     */
/* eee_en:  - If set, the TX LPI policy control engine is enabled and the MAC inserts LPI_idle co */
/*         des if the link is idle. The rx_lpi_detect assertion is independent of this configurat */
/*         ion. 
Reset default depends EEE_en_strap_input, which if tied to 1, defaults to enable */
/*         d, otherwise if tied to 0, defaults to disabled.                                       */
/* en_lpi_tx_pause:  - Enable LPI Tx Pause.                                                       */
/* en_lpi_tx_pfc:  - Enable LPI Tx PFC.                                                           */
/* en_lpi_rx_pause:  - Enable LPI Rx Pause.                                                       */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean lp_idle_prediction_mode;
    bdmf_boolean dis_eee_10m;
    bdmf_boolean eee_txclk_dis;
    bdmf_boolean rx_fifo_check;
    bdmf_boolean eee_en;
    bdmf_boolean en_lpi_tx_pause;
    bdmf_boolean en_lpi_tx_pfc;
    bdmf_boolean en_lpi_rx_pause;
} unimac_rdp_eee_ctrl;


/**************************************************************************************************/
/* pfc_stats_en:  - When clear, none of PFC related counters should increment. Otherwise, PFC cou */
/*               nters is in full function. 
Note: it is programming requirement to set this bit  */
/*               when PFC function is enable.                                                     */
/* rx_pass_pfc_frm:  - When set, MAC pass PFC frame to the system. Otherwise, PFC frame is discar */
/*                  ded.                                                                          */
/* force_ppp_xon:  - Instructs the MAC to send xon for all classes of service.                    */
/* ppp_en_rx:  - Enable MAC PPP_RX function.                                                      */
/* ppp_en_tx:  - Enable MAC PPP_TX function.                                                      */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean pfc_stats_en;
    bdmf_boolean rx_pass_pfc_frm;
    bdmf_boolean force_ppp_xon;
    bdmf_boolean ppp_en_rx;
    bdmf_boolean ppp_en_tx;
} unimac_rdp_ppp_cntrl;


/**************************************************************************************************/
/* mdio_busy:  - CPU writes this bit to 1 in order to initiate MCIO transaction. When transaction */
/*             completes hardware will clear this bit.                                            */
/* fail:  - This bit is set when PHY does not reply to READ command (PHY does not drive 0 on bus  */
/*       turnaround).                                                                             */
/* op_code:  - MDIO command that is OP[1:0]: 00 - Address for clause 45 01 - Write 10 - Read incr */
/*          ement for clause 45 11 - Read for clause 45 10 - Read for clause 22                   */
/* phy_prt_addr:  - PHY address[4:0] for clause 22, Port address[4:0] for Clause 45.              */
/* reg_dec_addr:  - Register address[4:0] for clause 22, Device address[4:0] for Clause 45.       */
/* data_addr:  - MDIO Read/Write data[15:0], clause 22 and 45 or MDIO address[15:0] for clause 45 */
/*            ".                                                                                  */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean mdio_busy;
    bdmf_boolean fail;
    uint8_t op_code;
    uint8_t phy_prt_addr;
    uint8_t reg_dec_addr;
    uint16_t data_addr;
} unimac_rdp_mdio_cmd;

bdmf_error_t ag_drv_unimac_rdp_umac_dummy_get(uint8_t umac_id, uint8_t *umac_dummy);
bdmf_error_t ag_drv_unimac_rdp_hd_bkp_cntl_set(uint8_t umac_id, uint8_t ipg_config_rx, bdmf_boolean hd_fc_bkoff_ok, bdmf_boolean hd_fc_ena);
bdmf_error_t ag_drv_unimac_rdp_hd_bkp_cntl_get(uint8_t umac_id, uint8_t *ipg_config_rx, bdmf_boolean *hd_fc_bkoff_ok, bdmf_boolean *hd_fc_ena);
bdmf_error_t ag_drv_unimac_rdp_cmd_set(uint8_t umac_id, const unimac_rdp_cmd *cmd);
bdmf_error_t ag_drv_unimac_rdp_cmd_get(uint8_t umac_id, unimac_rdp_cmd *cmd);
bdmf_error_t ag_drv_unimac_rdp_mac0_set(uint8_t umac_id, uint32_t mac_0);
bdmf_error_t ag_drv_unimac_rdp_mac0_get(uint8_t umac_id, uint32_t *mac_0);
bdmf_error_t ag_drv_unimac_rdp_mac1_set(uint8_t umac_id, uint16_t mac_1);
bdmf_error_t ag_drv_unimac_rdp_mac1_get(uint8_t umac_id, uint16_t *mac_1);
bdmf_error_t ag_drv_unimac_rdp_frm_len_set(uint8_t umac_id, uint16_t frame_length);
bdmf_error_t ag_drv_unimac_rdp_frm_len_get(uint8_t umac_id, uint16_t *frame_length);
bdmf_error_t ag_drv_unimac_rdp_pause_qunat_set(uint8_t umac_id, uint16_t pause_quant);
bdmf_error_t ag_drv_unimac_rdp_pause_qunat_get(uint8_t umac_id, uint16_t *pause_quant);
bdmf_error_t ag_drv_unimac_rdp_sfd_offset_get(uint8_t umac_id, uint32_t *temp);
bdmf_error_t ag_drv_unimac_rdp_mode_get(uint8_t umac_id, unimac_rdp_mode *mode);
bdmf_error_t ag_drv_unimac_rdp_frm_tag0_set(uint8_t umac_id, uint16_t outer_tag);
bdmf_error_t ag_drv_unimac_rdp_frm_tag0_get(uint8_t umac_id, uint16_t *outer_tag);
bdmf_error_t ag_drv_unimac_rdp_frm_tag1_set(uint8_t umac_id, uint16_t inner_tag);
bdmf_error_t ag_drv_unimac_rdp_frm_tag1_get(uint8_t umac_id, uint16_t *inner_tag);
bdmf_error_t ag_drv_unimac_rdp_tx_ipg_len_set(uint8_t umac_id, uint8_t tx_ipg_len);
bdmf_error_t ag_drv_unimac_rdp_tx_ipg_len_get(uint8_t umac_id, uint8_t *tx_ipg_len);
bdmf_error_t ag_drv_unimac_rdp_eee_ctrl_set(uint8_t umac_id, const unimac_rdp_eee_ctrl *eee_ctrl);
bdmf_error_t ag_drv_unimac_rdp_eee_ctrl_get(uint8_t umac_id, unimac_rdp_eee_ctrl *eee_ctrl);
bdmf_error_t ag_drv_unimac_rdp_eee_lpi_timer_set(uint8_t umac_id, uint32_t eee_lpi_timer);
bdmf_error_t ag_drv_unimac_rdp_eee_lpi_timer_get(uint8_t umac_id, uint32_t *eee_lpi_timer);
bdmf_error_t ag_drv_unimac_rdp_eee_wake_timer_set(uint8_t umac_id, uint16_t eee_wake_timer);
bdmf_error_t ag_drv_unimac_rdp_eee_wake_timer_get(uint8_t umac_id, uint16_t *eee_wake_timer);
bdmf_error_t ag_drv_unimac_rdp_eee_ref_count_set(uint8_t umac_id, uint16_t eee_reference_count);
bdmf_error_t ag_drv_unimac_rdp_eee_ref_count_get(uint8_t umac_id, uint16_t *eee_reference_count);
bdmf_error_t ag_drv_unimac_rdp_rx_pkt_drop_status_set(uint8_t umac_id, bdmf_boolean rx_ipg_invalid);
bdmf_error_t ag_drv_unimac_rdp_rx_pkt_drop_status_get(uint8_t umac_id, bdmf_boolean *rx_ipg_invalid);
bdmf_error_t ag_drv_unimac_rdp_symmetric_idle_threshold_set(uint8_t umac_id, uint16_t threshold_value);
bdmf_error_t ag_drv_unimac_rdp_symmetric_idle_threshold_get(uint8_t umac_id, uint16_t *threshold_value);
bdmf_error_t ag_drv_unimac_rdp_macsec_prog_tx_crc_set(uint8_t umac_id, uint32_t macsec_prog_tx_crc);
bdmf_error_t ag_drv_unimac_rdp_macsec_prog_tx_crc_get(uint8_t umac_id, uint32_t *macsec_prog_tx_crc);
bdmf_error_t ag_drv_unimac_rdp_macsec_cntrl_set(uint8_t umac_id, bdmf_boolean tx_crc_program, bdmf_boolean tx_crc_corrupt_en, bdmf_boolean tx_lanuch_en);
bdmf_error_t ag_drv_unimac_rdp_macsec_cntrl_get(uint8_t umac_id, bdmf_boolean *tx_crc_program, bdmf_boolean *tx_crc_corrupt_en, bdmf_boolean *tx_lanuch_en);
bdmf_error_t ag_drv_unimac_rdp_ts_status_cntrl_get(uint8_t umac_id, uint8_t *word_avail, bdmf_boolean *tx_ts_fifo_empty, bdmf_boolean *tx_ts_fifo_full);
bdmf_error_t ag_drv_unimac_rdp_tx_ts_data_get(uint8_t umac_id, uint32_t *tx_ts_data);
bdmf_error_t ag_drv_unimac_rdp_pause_cntrl_set(uint8_t umac_id, bdmf_boolean pause_control_en, uint32_t pause_timer);
bdmf_error_t ag_drv_unimac_rdp_pause_cntrl_get(uint8_t umac_id, bdmf_boolean *pause_control_en, uint32_t *pause_timer);
bdmf_error_t ag_drv_unimac_rdp_txfifo_flush_set(uint8_t umac_id, bdmf_boolean tx_flush);
bdmf_error_t ag_drv_unimac_rdp_txfifo_flush_get(uint8_t umac_id, bdmf_boolean *tx_flush);
bdmf_error_t ag_drv_unimac_rdp_rxfifo_stat_get(uint8_t umac_id, uint8_t *rxfifo_status);
bdmf_error_t ag_drv_unimac_rdp_txfifo_stat_get(uint8_t umac_id, bdmf_boolean *txfifo_overrun, bdmf_boolean *txfifo_underrun);
bdmf_error_t ag_drv_unimac_rdp_ppp_cntrl_set(uint8_t umac_id, const unimac_rdp_ppp_cntrl *ppp_cntrl);
bdmf_error_t ag_drv_unimac_rdp_ppp_cntrl_get(uint8_t umac_id, unimac_rdp_ppp_cntrl *ppp_cntrl);
bdmf_error_t ag_drv_unimac_rdp_ppp_refresh_cntrl_set(uint8_t umac_id, uint16_t ppp_refresh_timer, bdmf_boolean ppp_refresh_en);
bdmf_error_t ag_drv_unimac_rdp_ppp_refresh_cntrl_get(uint8_t umac_id, uint16_t *ppp_refresh_timer, bdmf_boolean *ppp_refresh_en);
bdmf_error_t ag_drv_unimac_rdp_tx_pause_prel0_get(uint8_t umac_id, uint32_t *tx_pause_prb0);
bdmf_error_t ag_drv_unimac_rdp_tx_pause_prel1_get(uint8_t umac_id, uint32_t *tx_pause_prb1);
bdmf_error_t ag_drv_unimac_rdp_tx_pause_prel2_get(uint8_t umac_id, uint32_t *tx_pause_prb2);
bdmf_error_t ag_drv_unimac_rdp_tx_pause_prel3_get(uint8_t umac_id, uint32_t *tx_pause_prb3);
bdmf_error_t ag_drv_unimac_rdp_rx_pause_prel0_get(uint8_t umac_id, uint32_t *rx_pause_prb0);
bdmf_error_t ag_drv_unimac_rdp_rx_pause_prel1_get(uint8_t umac_id, uint32_t *rx_pause_prb1);
bdmf_error_t ag_drv_unimac_rdp_rx_pause_prel2_get(uint8_t umac_id, uint32_t *rx_pause_prb2);
bdmf_error_t ag_drv_unimac_rdp_rx_pause_prel3_get(uint8_t umac_id, uint32_t *rx_pause_prb3);
bdmf_error_t ag_drv_unimac_rdp_rxerr_mask_set(uint8_t umac_id, uint32_t mac_rxerr_mask);
bdmf_error_t ag_drv_unimac_rdp_rxerr_mask_get(uint8_t umac_id, uint32_t *mac_rxerr_mask);
bdmf_error_t ag_drv_unimac_rdp_rx_max_pkt_size_set(uint8_t umac_id, uint16_t max_pkt_size);
bdmf_error_t ag_drv_unimac_rdp_rx_max_pkt_size_get(uint8_t umac_id, uint16_t *max_pkt_size);
bdmf_error_t ag_drv_unimac_rdp_mdio_cmd_set(uint8_t umac_id, const unimac_rdp_mdio_cmd *mdio_cmd);
bdmf_error_t ag_drv_unimac_rdp_mdio_cmd_get(uint8_t umac_id, unimac_rdp_mdio_cmd *mdio_cmd);
bdmf_error_t ag_drv_unimac_rdp_mdio_cfg_set(uint8_t umac_id, uint8_t mdio_clk_divider, bdmf_boolean mdio_clause);
bdmf_error_t ag_drv_unimac_rdp_mdio_cfg_get(uint8_t umac_id, uint8_t *mdio_clk_divider, bdmf_boolean *mdio_clause);
bdmf_error_t ag_drv_unimac_rdp_mdf_cnt_get(uint8_t umac_id, uint32_t *mdf_packet_counter);
bdmf_error_t ag_drv_unimac_rdp_diag_sel_set(uint8_t umac_id, uint8_t diag_hi_select, uint8_t diag_lo_select);
bdmf_error_t ag_drv_unimac_rdp_diag_sel_get(uint8_t umac_id, uint8_t *diag_hi_select, uint8_t *diag_lo_select);

#ifdef USE_BDMF_SHELL
enum
{
    cli_unimac_rdp_umac_dummy,
    cli_unimac_rdp_hd_bkp_cntl,
    cli_unimac_rdp_cmd,
    cli_unimac_rdp_mac0,
    cli_unimac_rdp_mac1,
    cli_unimac_rdp_frm_len,
    cli_unimac_rdp_pause_qunat,
    cli_unimac_rdp_sfd_offset,
    cli_unimac_rdp_mode,
    cli_unimac_rdp_frm_tag0,
    cli_unimac_rdp_frm_tag1,
    cli_unimac_rdp_tx_ipg_len,
    cli_unimac_rdp_eee_ctrl,
    cli_unimac_rdp_eee_lpi_timer,
    cli_unimac_rdp_eee_wake_timer,
    cli_unimac_rdp_eee_ref_count,
    cli_unimac_rdp_rx_pkt_drop_status,
    cli_unimac_rdp_symmetric_idle_threshold,
    cli_unimac_rdp_macsec_prog_tx_crc,
    cli_unimac_rdp_macsec_cntrl,
    cli_unimac_rdp_ts_status_cntrl,
    cli_unimac_rdp_tx_ts_data,
    cli_unimac_rdp_pause_cntrl,
    cli_unimac_rdp_txfifo_flush,
    cli_unimac_rdp_rxfifo_stat,
    cli_unimac_rdp_txfifo_stat,
    cli_unimac_rdp_ppp_cntrl,
    cli_unimac_rdp_ppp_refresh_cntrl,
    cli_unimac_rdp_tx_pause_prel0,
    cli_unimac_rdp_tx_pause_prel1,
    cli_unimac_rdp_tx_pause_prel2,
    cli_unimac_rdp_tx_pause_prel3,
    cli_unimac_rdp_rx_pause_prel0,
    cli_unimac_rdp_rx_pause_prel1,
    cli_unimac_rdp_rx_pause_prel2,
    cli_unimac_rdp_rx_pause_prel3,
    cli_unimac_rdp_rxerr_mask,
    cli_unimac_rdp_rx_max_pkt_size,
    cli_unimac_rdp_mdio_cmd,
    cli_unimac_rdp_mdio_cfg,
    cli_unimac_rdp_mdf_cnt,
    cli_unimac_rdp_diag_sel,
};

int bcm_unimac_rdp_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_unimac_rdp_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

