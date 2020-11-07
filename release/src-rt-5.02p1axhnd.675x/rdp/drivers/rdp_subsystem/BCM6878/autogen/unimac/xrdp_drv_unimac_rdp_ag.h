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
/* runt_filter_dis:  - When set, disable runt filtering.                                          */
/* oob_efc_en:  - If set then out-of-band egress flow control is enabled. When this bit is set an */
/*             d input pin ext_tx_flow_control is enabled then data frame trasmission is stopped, */
/*              whereas Pause & PFC frames are transmitted normally. This operation is similar to */
/*              halting the transmit datapath due to the reception of a Pause Frame with non-zero */
/*              timer value, and is used in applications where the flow control information is ex */
/*             changed out of band. Enabling or disabling this bit has no effect on regular Rx_pa */
/*             use pkt based egress flow control.                                                 */
/* ignore_tx_pause:  - Ignores the back pressure signaling from the system and hence no Tx pause  */
/*                  generation, when set.                                                         */
/* fd_tx_urun_fix_en:  - Tx Underflow detection can be improved by accounting for residue bytes i */
/*                    n 128b to 8b convertor. The fix is valid only for full duplex mode and can  */
/*                    be enabled by setting this bit.                                             */
/* line_loopback:  - Enable Line Loopback i.e. MAC FIFO side loopback (RX to TX) when set to '1', */
/*                 normal operation when set to '0' (Reset value).                                */
/* no_lgth_check:  - Payload Length Check Disable. When set to '0', the Core checks the frame's p */
/*                ayload length with the Frame
Length/Type field, when set to '1'(Reset value), t */
/*                he payload length check is disabled.                                            */
/* cntl_frm_ena:  - MAC Control Frame Enable. When set to '1', MAC Control frames with any 
Opcod */
/*               e other than 0x0001 are accepted and forward to the Client interface. 
When set  */
/*               to '0' (Reset value), MAC Control frames with any Opcode other 
than 0x0001 are  */
/*               silently discarded.                                                              */
/* ena_ext_config:  - Enable Configuration with External Pins. When set to '0' (Reset value) 
the */
/*                  Core speed and Mode is programmed with the register bits ETH_SPEED(1:0) 
and  */
/*                 HD_ENA. When set to '1', the Core is configured with the pins 
set_speed(1:0)  */
/*                 and set_duplex.                                                                */
/* en_internal_tx_crs:  - If enabled, then CRS input to Unimac is ORed with tds[8] (tx data valid */
/*                      output). This is helpful when TX CRS is disabled inside PHY.              */
/* sw_override_rx:  - If set, enables the SW programmed Rx pause capability config bits to overwr */
/*                 ite the auto negotiated Rx pause capabilities when ena_ext_config (autoconfig) */
/*                  is set.
If cleared, and when ena_ext_config (autoconfig) is set, then SW prog */
/*                 rammed Rx pause capability config bits has no effect over auto negotiated capa */
/*                 bilities.                                                                      */
/* sw_override_tx:  - If set, enables the SW programmed Tx pause capability config bits to overwr */
/*                 ite the auto negotiated Tx pause capabilities when ena_ext_config (autoconfig) */
/*                  is set.
If cleared, and when ena_ext_config (autoconfig) is set, then SW prog */
/*                 rammed Tx pause capability config bits has no effect over auto negotiated capa */
/*                 bilities.                                                                      */
/* mac_loop_con:  - Transmit packets to PHY while in MAC local loopback, when set to '1', otherwi */
/*               se transmit to PHY is disabled (normal operation),
when set to '0' (Reset value) */
/*               .                                                                                */
/* loop_ena:  - Enable GMII/MII loopback (TX to RX) when set to '1', normal operation when set to */
/*            '0' (Reset value).                                                                  */
/* fcs_corrupt_urun_en:  - Corrupt Tx FCS, on underrun, when set to '1', No FCS corruption when s */
/*                      et to '0' (Reset value).                                                  */
/* sw_reset:  - Software Reset Command. When asserted, the TX and RX are 
disabled. Config regist */
/*           ers are not affected by sw reset. Write a 0 to de-assert the sw reset.               */
/* overflow_en:  - If set, enables Rx FIFO overflow logic. In this case, the RXFIFO_STAT[1] regis */
/*              ter bit is not operational (always set to 0).
If cleared, disables RX FIFO overfl */
/*              ow logic. In this case, the RXFIFO_STAT[1] register bit is operational (Sticky se */
/*              t when overrun occurs, clearable only by SW_Reset).                               */
/* rx_low_latency_en:  - This works only when runt filter is disabled. It reduces the receive lat */
/*                    ency by 48 MAC clock time.                                                  */
/* hd_ena:  - Half duplex enable. When set to '1', enables half duplex mode, when set 
to '0', th */
/*         e MAC operates in full duplex mode.
Ignored at ethernet speeds 1G/2.5G or when the reg */
/*         ister ENA_EXT_CONFIG is set to '1'.                                                    */
/* tx_addr_ins:  - Set MAC address on transmit. If enabled (Set to '1') the MAC overwrites 
the s */
/*              ource MAC address with the programmed MAC address in registers MAC_0 
and MAC_1.  */
/*              If disabled (Set to reset value '0'), the source MAC address 
received from the t */
/*              ransmit application transmitted is not modified by the MAC.                       */
/* pause_ignore:  - Ignore Pause Frame Quanta. If enabled (Set to '1') received pause frames 
are */
/*                ignored by the MAC. When disabled (Set to reset value '0') the transmit 
proces */
/*               s is stopped for the amount of time specified in the pause quanta 
received with */
/*               in the pause frame.                                                              */
/* pause_fwd:  - Terminate/Forward Pause Frames. If enabled (Set to '1') pause frames are 
forwar */
/*            ded to the user application.  If disabled (Set to reset value '0'), 
pause frames a */
/*            re terminated and discarded in the MAC.                                             */
/* crc_fwd:  - Terminate/Forward Received CRC. If enabled (1) the CRC field of received 
frames a */
/*          re transmitted to the user application.
If disabled (Set to reset value '0') the CRC  */
/*          field is stripped from the frame.
Note: If padding function (bit PAD_EN set to '1') i */
/*          s enabled. CRC_FWD is 
ignored and the CRC field is checked and always terminated and */
/*           removed.                                                                             */
/* pad_en:  - Enable/Disable Frame Padding. If enabled (Set to '1'), then padding is removed from */
/*          the received frame before it is transmitted to the user
application. If disabled (set */
/*          to reset value '0'), then no padding is removed on receive by the MAC. 
This bit has  */
/*         no effect on Tx padding and hence Transmit always pad runts to guarantee a minimum fra */
/*         me size of 64 octets.                                                                  */
/* promis_en:  - Enable/Disable MAC promiscuous operation. When asserted (Set to '1'), 
all frame */
/*            s are received without Unicast address filtering.                                   */
/* eth_speed:  - Set MAC speed. Ignored when the register bit ENA_EXT_CONFIG is set to '1'.  When */
/*             the Register bit ENA_EXT_CONFIG is set to '0', used to set the core mode of operat */
/*            ion: 00: Enable 10Mbps Ethernet mode 01: Enable 100Mbps Ethernet mode 10: Enable Gi */
/*            gabit Ethernet mode 11: Enable 2.5Gigabit Ethernet mode                             */
/* rx_ena:  - Enable/Disable MAC receive path. When set to '0' (Reset value), the MAC 
receive fu */
/*         nction is disable.  When set to '1', the MAC receive function is enabled.              */
/* tx_ena:  - Enable/Disable MAC transmit path for data packets & pause/pfc packets sent in the n */
/*         ormal data path.
Pause/pfc packets generated internally are allowed if ignore_tx_pause */
/*          is not set. When set to '0' (Reset value), the MAC 
transmit function is disable.  Wh */
/*         en set to '1', the MAC transmit function is enabled.                                   */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean runt_filter_dis;
    bdmf_boolean oob_efc_en;
    bdmf_boolean ignore_tx_pause;
    bdmf_boolean fd_tx_urun_fix_en;
    bdmf_boolean line_loopback;
    bdmf_boolean no_lgth_check;
    bdmf_boolean cntl_frm_ena;
    bdmf_boolean ena_ext_config;
    bdmf_boolean en_internal_tx_crs;
    bdmf_boolean sw_override_rx;
    bdmf_boolean sw_override_tx;
    bdmf_boolean mac_loop_con;
    bdmf_boolean loop_ena;
    bdmf_boolean fcs_corrupt_urun_en;
    bdmf_boolean sw_reset;
    bdmf_boolean overflow_en;
    bdmf_boolean rx_low_latency_en;
    bdmf_boolean hd_ena;
    bdmf_boolean tx_addr_ins;
    bdmf_boolean pause_ignore;
    bdmf_boolean pause_fwd;
    bdmf_boolean crc_fwd;
    bdmf_boolean pad_en;
    bdmf_boolean promis_en;
    uint8_t eth_speed;
    bdmf_boolean rx_ena;
    bdmf_boolean tx_ena;
} unimac_rdp_command_config;


/**************************************************************************************************/
/* link_status:  - Link Status Indication. Set to '0', when link_status input is low.
Set to '1', */
/*               when link_status input is High.                                                  */
/* mac_tx_pause:  - MAC Pause Enabled in Transmit. 
0: MAC Pause Disabled in Transmit
1: MAC Paus */
/*               e Enabled in Transmit                                                            */
/* mac_rx_pause:  - MAC Pause Enabled in Receive. 
0: MAC Pause Disabled in Receive
1: MAC Pause  */
/*               Enabled in Receive                                                               */
/* mac_duplex:  - MAC Duplex. 
0: Full Duplex Mode enabled
1: Half Duplex Mode enabled            */
/* mac_speed:  - MAC Speed. 
00: 10Mbps Ethernet Mode enabled
01: 100Mbps Ethernet Mode enabled
1 */
/*            0: Gigabit Ethernet Mode enabled
11: 2.5Gigabit Ethernet Mode enabled               */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean link_status;
    bdmf_boolean mac_tx_pause;
    bdmf_boolean mac_rx_pause;
    bdmf_boolean mac_duplex;
    uint8_t mac_speed;
} unimac_rdp_mac_mode;


/**************************************************************************************************/
/* lp_idle_prediction_mode:  - When set to 1, enables LP_IDLE Prediction. When set to 0, disables */
/*                           LP_IDLE Prediction.                                                  */
/* dis_eee_10m:  - When this bit is set and link is established at 10Mbps, LPI is not supported ( */
/*              saving is achieved by reduced PHY's output swing). UNIMAC ignores EEE feature on  */
/*              both Tx & Rx in 10Mbps.
When cleared, Unimac doesn't differentiate between speeds */
/*               for EEE feature.                                                                 */
/* eee_txclk_dis:  - If enabled, UNIMAC will shut down TXCLK to PHY, when in LPI state.           */
/* rx_fifo_check:  - If enabled, lpi_rx_detect is set whenever the LPI_IDLES are being received o */
/*                n the RX line and Unimac Rx FIFO is empty.
By default, lpi_rx_detect is set onl */
/*                y when whenever the LPI_IDLES are being received on the RX line.                */
/* eee_en:  - If set, the TX LPI policy control engine is enabled and the MAC inserts LPI_idle co */
/*         des if the link is idle. The rx_lpi_detect assertion is independent of this configurat */
/*         ion. Reset default depends on EEE_en_strap input, which if tied to 1, defaults to enab */
/*         led, otherwise if tied to 0, defaults to disabled.                                     */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean lp_idle_prediction_mode;
    bdmf_boolean dis_eee_10m;
    bdmf_boolean eee_txclk_dis;
    bdmf_boolean rx_fifo_check;
    bdmf_boolean eee_en;
} unimac_rdp_umac_eee_ctrl;


/**************************************************************************************************/
/* pfc_stats_en:  - When clear, none of PFC related counters should increment. 
Otherwise, PFC co */
/*               unters is in full function. 
Note: it is programming requirement to set this bit */
/*                when PFC function is enable.                                                    */
/* rx_pass_pfc_frm:  - When set, MAC pass PFC frame to the system. Otherwise, PFC frame is discar */
/*                  ded.                                                                          */
/* force_pfc_xon:  - Instructs MAC to send Xon message to all classes of service.                 */
/* pfc_rx_enbl:  - Enables the PFC-Rx functionality.                                              */
/* pfc_tx_enbl:  - Enables the PFC-Tx functionality.                                              */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean pfc_stats_en;
    bdmf_boolean rx_pass_pfc_frm;
    bdmf_boolean force_pfc_xon;
    bdmf_boolean pfc_rx_enbl;
    bdmf_boolean pfc_tx_enbl;
} unimac_rdp_mac_pfc_ctrl;

bdmf_error_t ag_drv_unimac_rdp_ipg_hd_bkp_cntl_set(uint8_t umac_id, uint8_t ipg_config_rx, bdmf_boolean hd_fc_bkoff_ok, bdmf_boolean hd_fc_ena);
bdmf_error_t ag_drv_unimac_rdp_ipg_hd_bkp_cntl_get(uint8_t umac_id, uint8_t *ipg_config_rx, bdmf_boolean *hd_fc_bkoff_ok, bdmf_boolean *hd_fc_ena);
bdmf_error_t ag_drv_unimac_rdp_command_config_set(uint8_t umac_id, const unimac_rdp_command_config *command_config);
bdmf_error_t ag_drv_unimac_rdp_command_config_get(uint8_t umac_id, unimac_rdp_command_config *command_config);
bdmf_error_t ag_drv_unimac_rdp_mac_0_set(uint8_t umac_id, uint32_t mac_addr0);
bdmf_error_t ag_drv_unimac_rdp_mac_0_get(uint8_t umac_id, uint32_t *mac_addr0);
bdmf_error_t ag_drv_unimac_rdp_mac_1_set(uint8_t umac_id, uint16_t mac_addr1);
bdmf_error_t ag_drv_unimac_rdp_mac_1_get(uint8_t umac_id, uint16_t *mac_addr1);
bdmf_error_t ag_drv_unimac_rdp_frm_length_set(uint8_t umac_id, uint16_t maxfr);
bdmf_error_t ag_drv_unimac_rdp_frm_length_get(uint8_t umac_id, uint16_t *maxfr);
bdmf_error_t ag_drv_unimac_rdp_pause_quant_set(uint8_t umac_id, uint16_t pause_quant);
bdmf_error_t ag_drv_unimac_rdp_pause_quant_get(uint8_t umac_id, uint16_t *pause_quant);
bdmf_error_t ag_drv_unimac_rdp_tx_ts_seq_id_get(uint8_t umac_id, bdmf_boolean *tsts_valid, uint16_t *tsts_seq_id);
bdmf_error_t ag_drv_unimac_rdp_sfd_offset_set(uint8_t umac_id, uint8_t sfd_offset);
bdmf_error_t ag_drv_unimac_rdp_sfd_offset_get(uint8_t umac_id, uint8_t *sfd_offset);
bdmf_error_t ag_drv_unimac_rdp_mac_mode_get(uint8_t umac_id, unimac_rdp_mac_mode *mac_mode);
bdmf_error_t ag_drv_unimac_rdp_tag_0_set(uint8_t umac_id, bdmf_boolean config_outer_tpid_enable, uint16_t frm_tag_0);
bdmf_error_t ag_drv_unimac_rdp_tag_0_get(uint8_t umac_id, bdmf_boolean *config_outer_tpid_enable, uint16_t *frm_tag_0);
bdmf_error_t ag_drv_unimac_rdp_tag_1_set(uint8_t umac_id, bdmf_boolean config_inner_tpid_enable, uint16_t frm_tag_1);
bdmf_error_t ag_drv_unimac_rdp_tag_1_get(uint8_t umac_id, bdmf_boolean *config_inner_tpid_enable, uint16_t *frm_tag_1);
bdmf_error_t ag_drv_unimac_rdp_rx_pause_quanta_scale_set(uint8_t umac_id, bdmf_boolean scale_fix, bdmf_boolean scale_control, uint16_t scale_value);
bdmf_error_t ag_drv_unimac_rdp_rx_pause_quanta_scale_get(uint8_t umac_id, bdmf_boolean *scale_fix, bdmf_boolean *scale_control, uint16_t *scale_value);
bdmf_error_t ag_drv_unimac_rdp_tx_preamble_set(uint8_t umac_id, uint8_t tx_preamble);
bdmf_error_t ag_drv_unimac_rdp_tx_preamble_get(uint8_t umac_id, uint8_t *tx_preamble);
bdmf_error_t ag_drv_unimac_rdp_tx_ipg_length_set(uint8_t umac_id, uint8_t tx_min_pkt_size, uint8_t tx_ipg_length);
bdmf_error_t ag_drv_unimac_rdp_tx_ipg_length_get(uint8_t umac_id, uint8_t *tx_min_pkt_size, uint8_t *tx_ipg_length);
bdmf_error_t ag_drv_unimac_rdp_pfc_xoff_timer_set(uint8_t umac_id, uint16_t pfc_xoff_timer);
bdmf_error_t ag_drv_unimac_rdp_pfc_xoff_timer_get(uint8_t umac_id, uint16_t *pfc_xoff_timer);
bdmf_error_t ag_drv_unimac_rdp_umac_eee_ctrl_set(uint8_t umac_id, const unimac_rdp_umac_eee_ctrl *umac_eee_ctrl);
bdmf_error_t ag_drv_unimac_rdp_umac_eee_ctrl_get(uint8_t umac_id, unimac_rdp_umac_eee_ctrl *umac_eee_ctrl);
bdmf_error_t ag_drv_unimac_rdp_mii_eee_delay_entry_timer_set(uint8_t umac_id, uint32_t mii_eee_lpi_timer);
bdmf_error_t ag_drv_unimac_rdp_mii_eee_delay_entry_timer_get(uint8_t umac_id, uint32_t *mii_eee_lpi_timer);
bdmf_error_t ag_drv_unimac_rdp_gmii_eee_delay_entry_timer_set(uint8_t umac_id, uint32_t gmii_eee_lpi_timer);
bdmf_error_t ag_drv_unimac_rdp_gmii_eee_delay_entry_timer_get(uint8_t umac_id, uint32_t *gmii_eee_lpi_timer);
bdmf_error_t ag_drv_unimac_rdp_umac_eee_ref_count_set(uint8_t umac_id, uint16_t eee_ref_count);
bdmf_error_t ag_drv_unimac_rdp_umac_eee_ref_count_get(uint8_t umac_id, uint16_t *eee_ref_count);
bdmf_error_t ag_drv_unimac_rdp_umac_timestamp_adjust_set(uint8_t umac_id, bdmf_boolean auto_adjust, bdmf_boolean en_1588, uint16_t adjust);
bdmf_error_t ag_drv_unimac_rdp_umac_timestamp_adjust_get(uint8_t umac_id, bdmf_boolean *auto_adjust, bdmf_boolean *en_1588, uint16_t *adjust);
bdmf_error_t ag_drv_unimac_rdp_umac_rx_pkt_drop_status_set(uint8_t umac_id, bdmf_boolean rx_ipg_inval);
bdmf_error_t ag_drv_unimac_rdp_umac_rx_pkt_drop_status_get(uint8_t umac_id, bdmf_boolean *rx_ipg_inval);
bdmf_error_t ag_drv_unimac_rdp_umac_symmetric_idle_threshold_set(uint8_t umac_id, uint16_t threshold_value);
bdmf_error_t ag_drv_unimac_rdp_umac_symmetric_idle_threshold_get(uint8_t umac_id, uint16_t *threshold_value);
bdmf_error_t ag_drv_unimac_rdp_mii_eee_wake_timer_set(uint8_t umac_id, uint16_t mii_eee_wake_timer);
bdmf_error_t ag_drv_unimac_rdp_mii_eee_wake_timer_get(uint8_t umac_id, uint16_t *mii_eee_wake_timer);
bdmf_error_t ag_drv_unimac_rdp_gmii_eee_wake_timer_set(uint8_t umac_id, uint16_t gmii_eee_wake_timer);
bdmf_error_t ag_drv_unimac_rdp_gmii_eee_wake_timer_get(uint8_t umac_id, uint16_t *gmii_eee_wake_timer);
bdmf_error_t ag_drv_unimac_rdp_umac_rev_id_get(uint8_t umac_id, uint8_t *revision_id_major, uint8_t *revision_id_minor, uint8_t *patch);
bdmf_error_t ag_drv_unimac_rdp_mac_pfc_type_set(uint8_t umac_id, uint16_t pfc_eth_type);
bdmf_error_t ag_drv_unimac_rdp_mac_pfc_type_get(uint8_t umac_id, uint16_t *pfc_eth_type);
bdmf_error_t ag_drv_unimac_rdp_mac_pfc_opcode_set(uint8_t umac_id, uint16_t pfc_opcode);
bdmf_error_t ag_drv_unimac_rdp_mac_pfc_opcode_get(uint8_t umac_id, uint16_t *pfc_opcode);
bdmf_error_t ag_drv_unimac_rdp_mac_pfc_da_0_set(uint8_t umac_id, uint32_t pfc_macda_0);
bdmf_error_t ag_drv_unimac_rdp_mac_pfc_da_0_get(uint8_t umac_id, uint32_t *pfc_macda_0);
bdmf_error_t ag_drv_unimac_rdp_mac_pfc_da_1_set(uint8_t umac_id, uint16_t pfc_macda_1);
bdmf_error_t ag_drv_unimac_rdp_mac_pfc_da_1_get(uint8_t umac_id, uint16_t *pfc_macda_1);
bdmf_error_t ag_drv_unimac_rdp_macsec_prog_tx_crc_set(uint8_t umac_id, uint32_t macsec_prog_tx_crc);
bdmf_error_t ag_drv_unimac_rdp_macsec_prog_tx_crc_get(uint8_t umac_id, uint32_t *macsec_prog_tx_crc);
bdmf_error_t ag_drv_unimac_rdp_macsec_cntrl_set(uint8_t umac_id, bdmf_boolean dis_pause_data_var_ipg, bdmf_boolean tx_crc_program, bdmf_boolean tx_crc_corupt_en, bdmf_boolean tx_launch_en);
bdmf_error_t ag_drv_unimac_rdp_macsec_cntrl_get(uint8_t umac_id, bdmf_boolean *dis_pause_data_var_ipg, bdmf_boolean *tx_crc_program, bdmf_boolean *tx_crc_corupt_en, bdmf_boolean *tx_launch_en);
bdmf_error_t ag_drv_unimac_rdp_ts_status_get(uint8_t umac_id, uint8_t *word_avail, bdmf_boolean *tx_ts_fifo_empty, bdmf_boolean *tx_ts_fifo_full);
bdmf_error_t ag_drv_unimac_rdp_tx_ts_data_get(uint8_t umac_id, uint32_t *tx_ts_data);
bdmf_error_t ag_drv_unimac_rdp_pause_refresh_ctrl_set(uint8_t umac_id, bdmf_boolean enable, uint32_t refresh_timer);
bdmf_error_t ag_drv_unimac_rdp_pause_refresh_ctrl_get(uint8_t umac_id, bdmf_boolean *enable, uint32_t *refresh_timer);
bdmf_error_t ag_drv_unimac_rdp_flush_control_set(uint8_t umac_id, bdmf_boolean flush);
bdmf_error_t ag_drv_unimac_rdp_flush_control_get(uint8_t umac_id, bdmf_boolean *flush);
bdmf_error_t ag_drv_unimac_rdp_rxfifo_stat_get(uint8_t umac_id, bdmf_boolean *rxfifo_overrun, bdmf_boolean *rxfifo_underrun);
bdmf_error_t ag_drv_unimac_rdp_txfifo_stat_get(uint8_t umac_id, bdmf_boolean *txfifo_overrun, bdmf_boolean *txfifo_underrun);
bdmf_error_t ag_drv_unimac_rdp_mac_pfc_ctrl_set(uint8_t umac_id, const unimac_rdp_mac_pfc_ctrl *mac_pfc_ctrl);
bdmf_error_t ag_drv_unimac_rdp_mac_pfc_ctrl_get(uint8_t umac_id, unimac_rdp_mac_pfc_ctrl *mac_pfc_ctrl);
bdmf_error_t ag_drv_unimac_rdp_mac_pfc_refresh_ctrl_set(uint8_t umac_id, uint16_t pfc_refresh_timer, bdmf_boolean pfc_refresh_en);
bdmf_error_t ag_drv_unimac_rdp_mac_pfc_refresh_ctrl_get(uint8_t umac_id, uint16_t *pfc_refresh_timer, bdmf_boolean *pfc_refresh_en);

#ifdef USE_BDMF_SHELL
enum
{
    cli_unimac_rdp_ipg_hd_bkp_cntl,
    cli_unimac_rdp_command_config,
    cli_unimac_rdp_mac_0,
    cli_unimac_rdp_mac_1,
    cli_unimac_rdp_frm_length,
    cli_unimac_rdp_pause_quant,
    cli_unimac_rdp_tx_ts_seq_id,
    cli_unimac_rdp_sfd_offset,
    cli_unimac_rdp_mac_mode,
    cli_unimac_rdp_tag_0,
    cli_unimac_rdp_tag_1,
    cli_unimac_rdp_rx_pause_quanta_scale,
    cli_unimac_rdp_tx_preamble,
    cli_unimac_rdp_tx_ipg_length,
    cli_unimac_rdp_pfc_xoff_timer,
    cli_unimac_rdp_umac_eee_ctrl,
    cli_unimac_rdp_mii_eee_delay_entry_timer,
    cli_unimac_rdp_gmii_eee_delay_entry_timer,
    cli_unimac_rdp_umac_eee_ref_count,
    cli_unimac_rdp_umac_timestamp_adjust,
    cli_unimac_rdp_umac_rx_pkt_drop_status,
    cli_unimac_rdp_umac_symmetric_idle_threshold,
    cli_unimac_rdp_mii_eee_wake_timer,
    cli_unimac_rdp_gmii_eee_wake_timer,
    cli_unimac_rdp_umac_rev_id,
    cli_unimac_rdp_mac_pfc_type,
    cli_unimac_rdp_mac_pfc_opcode,
    cli_unimac_rdp_mac_pfc_da_0,
    cli_unimac_rdp_mac_pfc_da_1,
    cli_unimac_rdp_macsec_prog_tx_crc,
    cli_unimac_rdp_macsec_cntrl,
    cli_unimac_rdp_ts_status,
    cli_unimac_rdp_tx_ts_data,
    cli_unimac_rdp_pause_refresh_ctrl,
    cli_unimac_rdp_flush_control,
    cli_unimac_rdp_rxfifo_stat,
    cli_unimac_rdp_txfifo_stat,
    cli_unimac_rdp_mac_pfc_ctrl,
    cli_unimac_rdp_mac_pfc_refresh_ctrl,
};

int bcm_unimac_rdp_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_unimac_rdp_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

