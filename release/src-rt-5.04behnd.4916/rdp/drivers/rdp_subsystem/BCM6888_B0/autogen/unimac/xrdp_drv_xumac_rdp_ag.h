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


#ifndef _XRDP_DRV_XUMAC_RDP_AG_H_
#define _XRDP_DRV_XUMAC_RDP_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    bdmf_boolean tx_ena;
    bdmf_boolean rx_ena;
    uint8_t eth_speed;
    bdmf_boolean promis_en;
    bdmf_boolean pad_en;
    bdmf_boolean crc_fwd;
    bdmf_boolean pause_fwd;
    bdmf_boolean pause_ignore;
    bdmf_boolean tx_addr_ins;
    bdmf_boolean hd_ena;
    bdmf_boolean rx_low_latency_en;
    bdmf_boolean overflow_en;
    bdmf_boolean sw_reset;
    bdmf_boolean fcs_corrupt_urun_en;
    bdmf_boolean loop_ena;
    bdmf_boolean mac_loop_con;
    bdmf_boolean sw_override_tx;
    bdmf_boolean sw_override_rx;
    bdmf_boolean oob_efc_mode;
    bdmf_boolean bypass_oob_efc_synchronizer;
    bdmf_boolean en_internal_tx_crs;
    bdmf_boolean ena_ext_config;
    bdmf_boolean cntl_frm_ena;
    bdmf_boolean no_lgth_check;
    bdmf_boolean line_loopback;
    bdmf_boolean fd_tx_urun_fix_en;
    bdmf_boolean ignore_tx_pause;
    bdmf_boolean oob_efc_disab;
    bdmf_boolean runt_filter_dis;
    bdmf_boolean eth_speed_bit2;
} xumac_rdp_command_config;

typedef struct
{
    uint8_t mac_speed;
    bdmf_boolean mac_duplex;
    bdmf_boolean mac_rx_pause;
    bdmf_boolean mac_tx_pause;
    bdmf_boolean link_status;
    bdmf_boolean mac_speed_bit2;
} xumac_rdp_mac_mode;

typedef struct
{
    bdmf_boolean eee_en;
    bdmf_boolean rx_fifo_check;
    bdmf_boolean eee_txclk_dis;
    bdmf_boolean dis_eee_10m;
    bdmf_boolean lp_idle_prediction_mode;
    bdmf_boolean lpi_clock_gating_en;
} xumac_rdp_umac_eee_ctrl;

typedef struct
{
    bdmf_boolean pfc_tx_enbl;
    bdmf_boolean pfc_rx_enbl;
    bdmf_boolean force_pfc_xon;
    bdmf_boolean rx_pass_pfc_frm;
    bdmf_boolean pfc_stats_en;
} xumac_rdp_mac_pfc_ctrl;

typedef struct
{
    bdmf_boolean xib_rx_en;
    bdmf_boolean xib_tx_en;
    bdmf_boolean rx_flush_en;
    bdmf_boolean tx_flush_en;
    bdmf_boolean link_down_rst_en;
    bdmf_boolean standard_mux_en;
    bdmf_boolean xgmii_sel;
    bdmf_boolean dic_dis;
    uint16_t rx_start_threshold;
    bdmf_boolean gmii_rx_clk_gate_en;
    bdmf_boolean strict_preamble_dis;
    uint8_t tx_ipg;
    uint8_t min_rx_ipg;
    bdmf_boolean xgmii_sel_ovrd;
    bdmf_boolean gmii_tx_clk_gate_en;
    bdmf_boolean autoconfig_en;
} xumac_rdp_control_1;

typedef struct
{
    bdmf_boolean rx_fifo_overrun;
    bdmf_boolean rx_fifo_underrun;
    bdmf_boolean tx_fifo_underrun;
    bdmf_boolean tx_fifo_overrun;
    uint8_t rx_fault_status;
} xumac_rdp_status;


/**********************************************************************************************************************
 * hd_fc_ena: 
 *     When set, enables back-pressure in half-duplex mode.
 * hd_fc_bkoff_ok: 
 *     Register bit 1 refers to the application of backoff algorithm during HD backpressure.
 * ipg_config_rx: 
 *     The programmable Rx IPG below which the packets received are dropped graciously. The value is in Bytes for
 *     1/2.5G and Nibbles for 10/100M.
 * hd_random_jam: 
 *     Use 0x648532a6 as JAM when set to '1', use 0x55555555 as JAM when set to '0' (Reset value).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_ipg_hd_bkp_cntl_set(uint8_t xumac_id, bdmf_boolean hd_fc_ena, bdmf_boolean hd_fc_bkoff_ok, uint8_t ipg_config_rx, bdmf_boolean hd_random_jam);
bdmf_error_t ag_drv_xumac_rdp_ipg_hd_bkp_cntl_get(uint8_t xumac_id, bdmf_boolean *hd_fc_ena, bdmf_boolean *hd_fc_bkoff_ok, uint8_t *ipg_config_rx, bdmf_boolean *hd_random_jam);

/**********************************************************************************************************************
 * tx_ena: 
 *     Enable/Disable MAC transmit path for data packets & pause/pfc packets sent in the normal data path.
 *     Pause/pfc packets generated internally are allowed if ignore_tx_pause is not set. When set to '0' (Reset
 *     value), the MAC 
 *     transmit function is disable.  When set to '1', the MAC transmit function is enabled.
 * rx_ena: 
 *     Enable/Disable MAC receive path. When set to '0' (Reset value), the MAC 
 *     receive function is disable.  When set to '1', the MAC receive function is enabled.
 * eth_speed: 
 *     Set MAC speed. Bit 1:0 for ETH_SPEED[2:0]. Bit 2 is in ETH_SPEED_BIT2. Ignored when the register bit
 *     ENA_EXT_CONFIG is set to '1'.  When the Register bit ENA_EXT_CONFIG is set to '0', used to set the core mode of
 *     operation: 000: Enable 10Mbps Ethernet mode 001: Enable 100Mbps Ethernet mode 010: Enable Gigabit Ethernet mode
 *     011: Enable 2.5Gigabit Ethernet mode 101: Enable 5Gigabit Ethernet mode 100 Enable 10Gigabit Ethernet mode
 * promis_en: 
 *     Enable/Disable MAC promiscuous operation. When asserted (Set to '1'), 
 *     all frames are received without Unicast address filtering.
 * pad_en: 
 *     Enable/Disable Frame Padding. If enabled (Set to '1'), then padding is removed from the received frame before
 *     it is transmitted to the user
 *     application. If disabled (set to reset value '0'), then no padding is removed on receive by the MAC. 
 *     This bit has no effect on Tx padding and hence Transmit always pad runts to guarantee a minimum frame size of
 *     64 octets.
 * crc_fwd: 
 *     Terminate/Forward Received CRC. If enabled (1) the CRC field of received 
 *     frames are transmitted to the user application.
 *     If disabled (Set to reset value '0') the CRC field is stripped from the frame.
 *     Note: If padding function (bit PAD_EN set to '1') is enabled. CRC_FWD is 
 *     ignored and the CRC field is checked and always terminated and removed.
 * pause_fwd: 
 *     Terminate/Forward Pause Frames. If enabled (Set to '1') pause frames are 
 *     forwarded to the user application.  If disabled (Set to reset value '0'), 
 *     pause frames are terminated and discarded in the MAC.
 * pause_ignore: 
 *     Ignore Pause Frame Quanta. If enabled (Set to '1') received pause frames 
 *     are ignored by the MAC. When disabled (Set to reset value '0') the transmit 
 *     process is stopped for the amount of time specified in the pause quanta 
 *     received within the pause frame.
 * tx_addr_ins: 
 *     Set MAC address on transmit. If enabled (Set to '1') the MAC overwrites 
 *     the source MAC address with the programmed MAC address in registers MAC_0 
 *     and MAC_1. If disabled (Set to reset value '0'), the source MAC address 
 *     received from the transmit application transmitted is not modified by the MAC.
 * hd_ena: 
 *     Half duplex enable. When set to '1', enables half duplex mode, when set 
 *     to '0', the MAC operates in full duplex mode.
 *     Ignored at ethernet speeds 1G/2.5G or when the register ENA_EXT_CONFIG is set to '1'.
 * rx_low_latency_en: 
 *     This works only when runt filter is disabled. It reduces the receive latency by 48 MAC clock time.
 * overflow_en: 
 *     If set, enables Rx FIFO overflow logic. In this case, the RXFIFO_STAT[1] register bit is not operational
 *     (always set to 0).
 *     If cleared, disables RX FIFO overflow logic. In this case, the RXFIFO_STAT[1] register bit is operational
 *     (Sticky set when overrun occurs, clearable only by SW_Reset).
 * sw_reset: 
 *     Software Reset Command. When asserted, the TX and RX are 
 *     disabled. Config registers are not affected by sw reset. Write a 0 to de-assert the sw reset.
 * fcs_corrupt_urun_en: 
 *     Corrupt Tx FCS, on underrun, when set to '1', No FCS corruption when set to '0' (Reset value).
 * loop_ena: 
 *     Enable GMII/MII loopback (TX to RX) when set to '1', normal operation when set to '0' (Reset value).
 * mac_loop_con: 
 *     Transmit packets to PHY while in MAC local loopback, when set to '1', otherwise transmit to PHY is disabled
 *     (normal operation),
 *     when set to '0' (Reset value).
 * sw_override_tx: 
 *     If set, enables the SW programmed Tx pause capability config bits to overwrite the auto negotiated Tx pause
 *     capabilities when ena_ext_config (autoconfig) is set.
 *     If cleared, and when ena_ext_config (autoconfig) is set, then SW programmed Tx pause capability config bits has
 *     no effect over auto negotiated capabilities.
 * sw_override_rx: 
 *     If set, enables the SW programmed Rx pause capability config bits to overwrite the auto negotiated Rx pause
 *     capabilities when ena_ext_config (autoconfig) is set.
 *     If cleared, and when ena_ext_config (autoconfig) is set, then SW programmed Rx pause capability config bits has
 *     no effect over auto negotiated capabilities.
 * oob_efc_mode: 
 *     0=> strict/full OOB egress backpressure mode:
 *     - pause frames and PFC frames, as well as regular packets, are all affected by Unimac input
 *     ext_tx_flow_control, as long as OOB_EFC_DISAB is 0
 *     - in this mode, OOB backpressure will be active as long as ext_tx_flow_control is asserted and i_oob_efc_disab
 *     is 0, regardless of whether the MAC operates in half duplex mode or full duplex mode
 *     1=> legacy mode:
 *     - ext_tx_flow_control does not affect (does not prevent) transmission of Pause and PFC frames, i.e. in this
 *     mode OOB egress backpressure may only affect transmission of regular packets
 *     - OOB egress backpressure is fully disabled (ignored) when the MAC operates in half duplex mode.
 * bypass_oob_efc_synchronizer: 
 *     1=> bypass the OOB external flow control signal synchronizer, to e.g. reduce latency. In this case it is
 *     assumed/required that Unimac input ext_tx_flow_control is already in tx_clk clock domain (so there is no need
 *     to synchronize it)
 *     0=> locally synchronize the OOB egress flow control signal to tx_clk. In this case it is assumed/required that
 *     ext_tx_flow_control is glitchless (e.g. registered in its native clock domain).
 * en_internal_tx_crs: 
 *     If enabled, then CRS input to Unimac is ORed with tds[8] (tx data valid output). This is helpful when TX CRS is
 *     disabled inside PHY.
 * ena_ext_config: 
 *     Enable Configuration with External Pins. When set to '0' (Reset value) 
 *     the Core speed and Mode is programmed with the register bits ETH_SPEED(2:0) 
 *     and HD_ENA. When set to '1', the Core is configured with the pins 
 *     set_speed(1:0) and set_duplex.
 * cntl_frm_ena: 
 *     MAC Control Frame Enable. When set to '1', MAC Control frames with any 
 *     Opcode other than 0x0001 are accepted and forward to the Client interface. 
 *     When set to '0' (Reset value), MAC Control frames with any Opcode other 
 *     than 0x0001 are silently discarded.
 * no_lgth_check: 
 *     Payload Length Check Disable. When set to '0', the Core checks the frame's payload length with the Frame
 *     Length/Type field, when set to '1'(Reset value), the payload length check is disabled.
 * line_loopback: 
 *     Enable Line Loopback i.e. MAC FIFO side loopback (RX to TX) when set to '1', normal operation when set to '0'
 *     (Reset value).
 * fd_tx_urun_fix_en: 
 *     Tx Underflow detection can be improved by accounting for residue bytes in 128b to 8b convertor. The fix is
 *     valid only for full duplex mode and can be enabled by setting this bit.
 * ignore_tx_pause: 
 *     Ignores the back pressure signaling from the system and hence no Tx pause generation, when set.
 * oob_efc_disab: 
 *     When this bit is set, out-of-band egress flow control will be disabled. When this bit is 0 (out-of-band egress
 *     flow control enabled) and input pin ext_tx_flow_control is 1, frame transmissions may be stopped - see
 *     OOB_EFC_MODE for details.
 *     Out-of-band egress flow control operation is similar to halting the transmit datapath due to reception of a
 *     Pause Frame with a non-zero timer value. This bit however has no effect on regular Rx Pause Frame based egress
 *     flow control.
 * runt_filter_dis: 
 *     When set, disable runt filtering.
 * eth_speed_bit2: 
 *     This is bit 2 for ETH_SPEED. See ETH_SPEED below.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_command_config_set(uint8_t xumac_id, const xumac_rdp_command_config *command_config);
bdmf_error_t ag_drv_xumac_rdp_command_config_get(uint8_t xumac_id, xumac_rdp_command_config *command_config);

/**********************************************************************************************************************
 * mac_addr0: 
 *     Register bit 0 corresponds to bit 16 of the MAC address, register bit 1 corresponds to bit 17 of the MAC
 *     address, and so on.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_mac_0_set(uint8_t xumac_id, uint32_t mac_addr0);
bdmf_error_t ag_drv_xumac_rdp_mac_0_get(uint8_t xumac_id, uint32_t *mac_addr0);

/**********************************************************************************************************************
 * mac_addr1: 
 *     Register bit 0 corresponds to bit 0 of the MAC address, register bit 1 corresponds to bit 1 of the MAC address,
 *     and so on.
 *     Bits 16 to 31 are reserved.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_mac_1_set(uint8_t xumac_id, uint16_t mac_addr1);
bdmf_error_t ag_drv_xumac_rdp_mac_1_get(uint8_t xumac_id, uint16_t *mac_addr1);

/**********************************************************************************************************************
 * maxfr: 
 *     Defines a 14-bit maximum frame length used by the MAC receive logic to check frames.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_frm_length_set(uint8_t xumac_id, uint16_t maxfr);
bdmf_error_t ag_drv_xumac_rdp_frm_length_get(uint8_t xumac_id, uint16_t *maxfr);

/**********************************************************************************************************************
 * pause_quant: 
 *     16-bit value, sets, in increments of 512 Ethernet bit times, the pause quanta used in 
 *     each Pause Frame sent to the remote Ethernet device.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_pause_quant_set(uint8_t xumac_id, uint16_t pause_quant);
bdmf_error_t ag_drv_xumac_rdp_pause_quant_get(uint8_t xumac_id, uint16_t *pause_quant);

/**********************************************************************************************************************
 * tsts_seq_id: 
 *     Every read of this register will fetch out one seq_id from the transmit FIFO.(One seq_id per one read command
 *     on the sbus).
 *     Every 49 bit val_bit + seq_id + timestamp is read in two steps, i.e., one read from 0x10f (val_bit + seq_id)
 *     followed by another read from 0x1c7 (timestamp).
 *     Timestamp read without a preceding seq_id read will fetch stale timestamp value.
 * tsts_valid: 
 *     Indicates that a timestamp was captured and is valid. if the cpu reads an empty fifo the VALID bit will be 0.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tx_ts_seq_id_get(uint8_t xumac_id, uint16_t *tsts_seq_id, bdmf_boolean *tsts_valid);

/**********************************************************************************************************************
 * sfd_offset: 
 *     Defines the length of the EFM preamble between 5 and 15 Bytes. When set to 0, 1, 2, 3 or 4,
 *     the Preamble EFM length is set to 5 Bytes.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_sfd_offset_set(uint8_t xumac_id, uint8_t sfd_offset);
bdmf_error_t ag_drv_xumac_rdp_sfd_offset_get(uint8_t xumac_id, uint8_t *sfd_offset);

/**********************************************************************************************************************
 * mac_speed: 
 *     MAC Speed[2:0]. Bit 2 is in MAC_SPEED_BIT2.
 *     000: 10Mbps Ethernet Mode enabled
 *     001: 100Mbps Ethernet Mode enabled
 *     010: Gigabit Ethernet Mode enabled
 *     011: 2.5Gigabit Ethernet Mode enabled
 *     101: 5Gigabit Ethernet Mode enabled
 *     100: 10Gigabit Ethernet Mode enabled
 * mac_duplex: 
 *     MAC Duplex. 
 *     0: Full Duplex Mode enabled
 *     1: Half Duplex Mode enabled
 * mac_rx_pause: 
 *     MAC Pause Enabled in Receive. 
 *     0: MAC Pause Disabled in Receive
 *     1: MAC Pause Enabled in Receive
 * mac_tx_pause: 
 *     MAC Pause Enabled in Transmit. 
 *     0: MAC Pause Disabled in Transmit
 *     1: MAC Pause Enabled in Transmit
 * link_status: 
 *     Link Status Indication. Set to '0', when link_status input is low.
 *     Set to '1', when link_status input is High.
 * mac_speed_bit2: 
 *     Bit 2 of MAC_SPEED[2:0]
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_mac_mode_get(uint8_t xumac_id, xumac_rdp_mac_mode *mac_mode);

/**********************************************************************************************************************
 * frm_tag_0: 
 *     Outer tag of the programmable VLAN tag
 * config_outer_tpid_enable: 
 *     If cleared then disable outer TPID detection
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tag_0_set(uint8_t xumac_id, uint16_t frm_tag_0, bdmf_boolean config_outer_tpid_enable);
bdmf_error_t ag_drv_xumac_rdp_tag_0_get(uint8_t xumac_id, uint16_t *frm_tag_0, bdmf_boolean *config_outer_tpid_enable);

/**********************************************************************************************************************
 * frm_tag_1: 
 *     inner tag of the programmable VLAN tag
 * config_inner_tpid_enable: 
 *     If cleared then disable inner TPID detection
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tag_1_set(uint8_t xumac_id, uint16_t frm_tag_1, bdmf_boolean config_inner_tpid_enable);
bdmf_error_t ag_drv_xumac_rdp_tag_1_get(uint8_t xumac_id, uint16_t *frm_tag_1, bdmf_boolean *config_inner_tpid_enable);

/**********************************************************************************************************************
 * scale_value: 
 *     The pause timer is loaded with the value obtained after adding or subtracting the scale_value from the received
 *     pause quanta.
 * scale_control: 
 *     If clear, then subtract the scale_value from the received pause quanta. 
 *     If set, then add the scale_value from the received pause quanta.
 * scale_fix: 
 *     If set, then receive pause quanta is ignored and a fixed quanta value programmed in SCALE_VALUE is loaded into
 *     the pause timer.
 *     If set, then SCALE_CONTROL is ignored.
 *     If cleared, then SCALE_CONTROL takes into effect.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_rx_pause_quanta_scale_set(uint8_t xumac_id, uint16_t scale_value, bdmf_boolean scale_control, bdmf_boolean scale_fix);
bdmf_error_t ag_drv_xumac_rdp_rx_pause_quanta_scale_get(uint8_t xumac_id, uint16_t *scale_value, bdmf_boolean *scale_control, bdmf_boolean *scale_fix);

/**********************************************************************************************************************
 * tx_preamble: 
 *     Set the transmit preamble excluding SFD to be programmable from min of 2 bytes to the max allowable of 7 bytes,
 *     with granularity of 1 byte.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tx_preamble_set(uint8_t xumac_id, uint8_t tx_preamble);
bdmf_error_t ag_drv_xumac_rdp_tx_preamble_get(uint8_t xumac_id, uint8_t *tx_preamble);

/**********************************************************************************************************************
 * tx_ipg_length: 
 *     Set the Transmit minimum IPG from 8 to 64 Byte-times. If a value below 8 or above 64 is
 *     programmed, the minimum IPG is set to 12 byte-times.
 * tx_min_pkt_size: 
 *     Min. TX packet size without FCS, also without preamble+SFD.
 *     Padding will be appended if needed to ensure this size.
 *     Valid values are: 14..125
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tx_ipg_length_set(uint8_t xumac_id, uint8_t tx_ipg_length, uint8_t tx_min_pkt_size);
bdmf_error_t ag_drv_xumac_rdp_tx_ipg_length_get(uint8_t xumac_id, uint8_t *tx_ipg_length, uint8_t *tx_min_pkt_size);

/**********************************************************************************************************************
 * pfc_xoff_timer: 
 *     Time value sent in the Timer Field for classes in XOFF state (Unit is 512 bit-times).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_pfc_xoff_timer_set(uint8_t xumac_id, uint16_t pfc_xoff_timer);
bdmf_error_t ag_drv_xumac_rdp_pfc_xoff_timer_get(uint8_t xumac_id, uint16_t *pfc_xoff_timer);

/**********************************************************************************************************************
 * eee_en: 
 *     If set, the TX LPI policy control engine is enabled and the MAC inserts LPI_idle codes if the link is idle. The
 *     rx_lpi_detect assertion is independent of this configuration. Reset default depends on EEE_en_strap input,
 *     which if tied to 1, defaults to enabled, otherwise if tied to 0, defaults to disabled.
 * rx_fifo_check: 
 *     If enabled, lpi_rx_detect is set whenever the LPI_IDLES are being received on the RX line and Unimac Rx FIFO is
 *     empty.
 *     By default, lpi_rx_detect is set only when whenever the LPI_IDLES are being received on the RX line.
 * eee_txclk_dis: 
 *     If enabled, UNIMAC will shut down TXCLK to PHY, when in LPI state.
 * dis_eee_10m: 
 *     When this bit is set and link is established at 10Mbps, LPI is not supported (saving is achieved by reduced
 *     PHY's output swing). UNIMAC ignores EEE feature on both Tx & Rx in 10Mbps.
 *     When cleared, Unimac doesn't differentiate between speeds for EEE feature.
 * lp_idle_prediction_mode: 
 *     When set to 1, enables LP_IDLE Prediction. When set to 0, disables LP_IDLE Prediction.
 *     It is an experimental feature and not recommended to use for the production SW.
 * lpi_clock_gating_en: 
 *     When set to 1, RX and TX clocks to certain modules are shut down in LPI mode. When set to 0, RX and TX clocks
 *     are still on in LPI mode.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_umac_eee_ctrl_set(uint8_t xumac_id, const xumac_rdp_umac_eee_ctrl *umac_eee_ctrl);
bdmf_error_t ag_drv_xumac_rdp_umac_eee_ctrl_get(uint8_t xumac_id, xumac_rdp_umac_eee_ctrl *umac_eee_ctrl);

/**********************************************************************************************************************
 * mii_eee_lpi_timer: 
 *     This is the duration for which condition to move to LPI state must be satisfied, at the end of which MAC
 *     transitions to LPI State. The decrement unit is 1 micro-second.
 *     This register is meant for 10/100 Mbps speed.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_mii_eee_delay_entry_timer_set(uint8_t xumac_id, uint32_t mii_eee_lpi_timer);
bdmf_error_t ag_drv_xumac_rdp_mii_eee_delay_entry_timer_get(uint8_t xumac_id, uint32_t *mii_eee_lpi_timer);

/**********************************************************************************************************************
 * gmii_eee_lpi_timer: 
 *     This is the duration for which condition to move to LPI state must be satisfied, at the end of which MAC
 *     transitions to LPI State. The decrement unit is 1 micro-second.
 *     This register is meant for 1000 Mbps speed.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gmii_eee_delay_entry_timer_set(uint8_t xumac_id, uint32_t gmii_eee_lpi_timer);
bdmf_error_t ag_drv_xumac_rdp_gmii_eee_delay_entry_timer_get(uint8_t xumac_id, uint32_t *gmii_eee_lpi_timer);

/**********************************************************************************************************************
 * eee_ref_count: 
 *     This field controls clock divider used to generate ~1us reference pulses used by EEE timers. It specifies
 *     integer number of timer clock cycles contained within 1us.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_umac_eee_ref_count_set(uint8_t xumac_id, uint16_t eee_ref_count);
bdmf_error_t ag_drv_xumac_rdp_umac_eee_ref_count_get(uint8_t xumac_id, uint16_t *eee_ref_count);

/**********************************************************************************************************************
 * adjust: 
 *     Offset adjustment to outgoing TIMESTAMP to adjust for pipeline stalling and/or jitter asymmetry. The value is
 *     in 2's compliment format and is of 1ns granularity.
 * en_1588: 
 *     Enables 1588 one step timestamp feature.
 * auto_adjust: 
 *     Enables MAC Rx timestamp offset balancing at MAC TX timestamp.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_umac_timestamp_adjust_set(uint8_t xumac_id, uint16_t adjust, bdmf_boolean en_1588, bdmf_boolean auto_adjust);
bdmf_error_t ag_drv_xumac_rdp_umac_timestamp_adjust_get(uint8_t xumac_id, uint16_t *adjust, bdmf_boolean *en_1588, bdmf_boolean *auto_adjust);

/**********************************************************************************************************************
 * rx_ipg_inval: 
 *     Debug status, set if MAC receives an IPG less than programmed RX IPG or less than four bytes. Sticky bit.
 *     Clears when SW writes 0 into the field or by sw_reset.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_umac_rx_pkt_drop_status_set(uint8_t xumac_id, bdmf_boolean rx_ipg_inval);
bdmf_error_t ag_drv_xumac_rdp_umac_rx_pkt_drop_status_get(uint8_t xumac_id, bdmf_boolean *rx_ipg_inval);

/**********************************************************************************************************************
 * threshold_value: 
 *     If LPI_Prediction is enabled then this register defines the number of IDLEs to be received by the UniMAC before
 *     allowing LP_IDLE to be sent to Link Partner.
 *     It is an experimental feature and not recommended to use for the production SW.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_umac_symmetric_idle_threshold_set(uint8_t xumac_id, uint16_t threshold_value);
bdmf_error_t ag_drv_xumac_rdp_umac_symmetric_idle_threshold_get(uint8_t xumac_id, uint16_t *threshold_value);

/**********************************************************************************************************************
 * mii_eee_wake_timer: 
 *     This is the duration for which MAC must wait to go back to ACTIVE state from LPI state when it receives packet
 *     for transmission. The decrement unit is 1 micro-second.
 *     This register is meant for 100 Mbps speed.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_mii_eee_wake_timer_set(uint8_t xumac_id, uint16_t mii_eee_wake_timer);
bdmf_error_t ag_drv_xumac_rdp_mii_eee_wake_timer_get(uint8_t xumac_id, uint16_t *mii_eee_wake_timer);

/**********************************************************************************************************************
 * gmii_eee_wake_timer: 
 *     This is the duration for which MAC must wait to go back to ACTIVE state from LPI state when it receives packet
 *     for transmission. The decrement unit is 1 micro-second.
 *     This register is meant for 1000 Mbps speed.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gmii_eee_wake_timer_set(uint8_t xumac_id, uint16_t gmii_eee_wake_timer);
bdmf_error_t ag_drv_xumac_rdp_gmii_eee_wake_timer_get(uint8_t xumac_id, uint16_t *gmii_eee_wake_timer);

/**********************************************************************************************************************
 * patch: 
 *     Unimac revision patch number.
 * revision_id_minor: 
 *     Unimac version id field after decimal.
 * revision_id_major: 
 *     Unimac version id field before decimal.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_umac_rev_id_get(uint8_t xumac_id, uint8_t *patch, uint8_t *revision_id_minor, uint8_t *revision_id_major);

/**********************************************************************************************************************
 * gmii_2p5g_eee_lpi_timer: 
 *     This is the duration for which condition to move to LPI state must be satisfied, at the end of which MAC
 *     transitions to LPI State. The decrement unit is 1 micro-second.
 *     This register is meant for 2.5 Gbps speed.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gmii_2p5g_eee_delay_entry_timer_set(uint8_t xumac_id, uint32_t gmii_2p5g_eee_lpi_timer);
bdmf_error_t ag_drv_xumac_rdp_gmii_2p5g_eee_delay_entry_timer_get(uint8_t xumac_id, uint32_t *gmii_2p5g_eee_lpi_timer);

/**********************************************************************************************************************
 * gmii_5g_eee_lpi_timer: 
 *     This is the duration for which condition to move to LPI state must be satisfied, at the end of which MAC
 *     transitions to LPI State. The decrement unit is 1 micro-second.
 *     This register is meant for 5 Gbps speed.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gmii_5g_eee_delay_entry_timer_set(uint8_t xumac_id, uint32_t gmii_5g_eee_lpi_timer);
bdmf_error_t ag_drv_xumac_rdp_gmii_5g_eee_delay_entry_timer_get(uint8_t xumac_id, uint32_t *gmii_5g_eee_lpi_timer);

/**********************************************************************************************************************
 * gmii_10g_eee_lpi_timer: 
 *     This is the duration for which condition to move to LPI state must be satisfied, at the end of which MAC
 *     transitions to LPI State. The decrement unit is 1 micro-second.
 *     This register is meant for 10 Gbps speed.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gmii_10g_eee_delay_entry_timer_set(uint8_t xumac_id, uint32_t gmii_10g_eee_lpi_timer);
bdmf_error_t ag_drv_xumac_rdp_gmii_10g_eee_delay_entry_timer_get(uint8_t xumac_id, uint32_t *gmii_10g_eee_lpi_timer);

/**********************************************************************************************************************
 * gmii_2p5g_eee_wake_timer: 
 *     This is the duration for which MAC must wait to go back to ACTIVE state from LPI state when it receives packet
 *     for transmission. The decrement unit is 1 micro-second.
 *     This register is meant for 2.5 Gbps speed.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gmii_2p5g_eee_wake_timer_set(uint8_t xumac_id, uint16_t gmii_2p5g_eee_wake_timer);
bdmf_error_t ag_drv_xumac_rdp_gmii_2p5g_eee_wake_timer_get(uint8_t xumac_id, uint16_t *gmii_2p5g_eee_wake_timer);

/**********************************************************************************************************************
 * gmii_5g_eee_wake_timer: 
 *     This is the duration for which MAC must wait to go back to ACTIVE state from LPI state when it receives packet
 *     for transmission. The decrement unit is 1 micro-second.
 *     This register is meant for 5 Gbps speed.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gmii_5g_eee_wake_timer_set(uint8_t xumac_id, uint16_t gmii_5g_eee_wake_timer);
bdmf_error_t ag_drv_xumac_rdp_gmii_5g_eee_wake_timer_get(uint8_t xumac_id, uint16_t *gmii_5g_eee_wake_timer);

/**********************************************************************************************************************
 * gmii_10g_eee_wake_timer: 
 *     This is the duration for which MAC must wait to go back to ACTIVE state from LPI state when it receives packet
 *     for transmission. The decrement unit is 1 micro-second.
 *     This register is meant for 10 Gbps speed.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gmii_10g_eee_wake_timer_set(uint8_t xumac_id, uint16_t gmii_10g_eee_wake_timer);
bdmf_error_t ag_drv_xumac_rdp_gmii_10g_eee_wake_timer_get(uint8_t xumac_id, uint16_t *gmii_10g_eee_wake_timer);

/**********************************************************************************************************************
 * active_eee_lpi_timer: 
 *     Currently selected EEE LPI timer.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_active_eee_delay_entry_timer_get(uint8_t xumac_id, uint32_t *active_eee_lpi_timer);

/**********************************************************************************************************************
 * active_eee_wake_time: 
 *     Currently selected wake timer.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_active_eee_wake_timer_get(uint8_t xumac_id, uint16_t *active_eee_wake_time);

/**********************************************************************************************************************
 * pfc_eth_type: 
 *     Ethertype for PFC packets. The default value (0x8808) is the standard value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_mac_pfc_type_set(uint8_t xumac_id, uint16_t pfc_eth_type);
bdmf_error_t ag_drv_xumac_rdp_mac_pfc_type_get(uint8_t xumac_id, uint16_t *pfc_eth_type);

/**********************************************************************************************************************
 * pfc_opcode: 
 *     PFC opcode. The default value (0x0101) is the standard value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_mac_pfc_opcode_set(uint8_t xumac_id, uint16_t pfc_opcode);
bdmf_error_t ag_drv_xumac_rdp_mac_pfc_opcode_get(uint8_t xumac_id, uint16_t *pfc_opcode);

/**********************************************************************************************************************
 * pfc_macda_0: 
 *     Lower 32 bits of DA for PFC.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_mac_pfc_da_0_set(uint8_t xumac_id, uint32_t pfc_macda_0);
bdmf_error_t ag_drv_xumac_rdp_mac_pfc_da_0_get(uint8_t xumac_id, uint32_t *pfc_macda_0);

/**********************************************************************************************************************
 * pfc_macda_1: 
 *     Upper 16 bits of DA for PFC.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_mac_pfc_da_1_set(uint8_t xumac_id, uint16_t pfc_macda_1);
bdmf_error_t ag_drv_xumac_rdp_mac_pfc_da_1_get(uint8_t xumac_id, uint16_t *pfc_macda_1);

/**********************************************************************************************************************
 * macsec_prog_tx_crc: 
 *     The transmitted CRC can be corrupted by replacing the FCS of the transmitted frame by the FCS programmed in
 *     this register.
 *     This is enabled and controlled by MACSEC_CNTRL register.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_macsec_prog_tx_crc_set(uint8_t xumac_id, uint32_t macsec_prog_tx_crc);
bdmf_error_t ag_drv_xumac_rdp_macsec_prog_tx_crc_get(uint8_t xumac_id, uint32_t *macsec_prog_tx_crc);

/**********************************************************************************************************************
 * tx_launch_en: 
 *     Set the bit 0 (Tx_Launch_en) logic 0, if the tx_launch function is to be disabled. If set, then the
 *     launch_enable signal assertion/deassertion causes the packet transmit enabled/disabled. The launch_enable is
 *     per packet basis.
 * tx_crc_corupt_en: 
 *     Setting this field enables the CRC corruption on the transmitted packets. The options of how to corrupt,
 *     depends on
 *     the field 2 of this register (TX_CRC_PROGRAM). The CRC corruption happens only on the frames for which TXCRCER
 *     is asserted by the system.
 * tx_crc_program: 
 *     If CRC corruption feature in enabled (TX_CRC_CORUPT_EN set), then in case where this bit when set, replaces the
 *     transmitted FCS with the programmed FCS.
 *     When cleared, corrupts the CRC of the transmitted packet internally.
 * dis_pause_data_var_ipg: 
 *     When this bit is 1, IPG between pause and data frame is as per the original design, i.e., 13B or 12B, fixed. It
 *     should be noted, that as number of preamble bytes reduces from 7, the IPG also increases. 
 *     When this bit is 0, IPG between pause and data frame is variable and equals programmed IPG or programmed IPG +
 *     1.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_macsec_cntrl_set(uint8_t xumac_id, bdmf_boolean tx_launch_en, bdmf_boolean tx_crc_corupt_en, bdmf_boolean tx_crc_program, bdmf_boolean dis_pause_data_var_ipg);
bdmf_error_t ag_drv_xumac_rdp_macsec_cntrl_get(uint8_t xumac_id, bdmf_boolean *tx_launch_en, bdmf_boolean *tx_crc_corupt_en, bdmf_boolean *tx_crc_program, bdmf_boolean *dis_pause_data_var_ipg);

/**********************************************************************************************************************
 * tx_ts_fifo_full: 
 *     Read-only field assertion shows that the transmit timestamp FIFO is full.
 * tx_ts_fifo_empty: 
 *     Read-only field assertion shows that the transmit timestamp FIFO is empty.
 * word_avail: 
 *     Indicates number of cells filled in the TX timestamp FIFO.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_ts_status_get(uint8_t xumac_id, bdmf_boolean *tx_ts_fifo_full, bdmf_boolean *tx_ts_fifo_empty, uint8_t *word_avail);

/**********************************************************************************************************************
 * tx_ts_data: 
 *     Every read of this register will fetch out one timestamp value corresponding to the preceding seq_id read from
 *     the transmit FIFO.
 *     Every 49 bit, val_bit + seq_id + timestamp is read in two steps, i.e., one read from 0x10f (val_bit + seq_id)
 *     followed by another read from 0x1c7 (timestamp).
 *     Timestamp read without a preceding seq_id read will fetch stale timestamp value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tx_ts_data_get(uint8_t xumac_id, uint32_t *tx_ts_data);

/**********************************************************************************************************************
 * refresh_timer: 
 *     Timer expiry time, represented in 512 bit time units. Note that the actual expiry time depends on the port
 *     speed. Values of 0 and 1 are illegal.
 * enable: 
 *     Enable extra pause frames.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_pause_refresh_ctrl_set(uint8_t xumac_id, uint32_t refresh_timer, bdmf_boolean enable);
bdmf_error_t ag_drv_xumac_rdp_pause_refresh_ctrl_get(uint8_t xumac_id, uint32_t *refresh_timer, bdmf_boolean *enable);

/**********************************************************************************************************************
 * flush: 
 *     Flush enable bit to drop out all packets in Tx FIFO without egressing any packets when set.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_flush_control_set(uint8_t xumac_id, bdmf_boolean flush);
bdmf_error_t ag_drv_xumac_rdp_flush_control_get(uint8_t xumac_id, bdmf_boolean *flush);

/**********************************************************************************************************************
 * rxfifo_underrun: 
 *     RXFIFO Underrun occurred.
 * rxfifo_overrun: 
 *     RXFIFO Overrun occurred.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_rxfifo_stat_get(uint8_t xumac_id, bdmf_boolean *rxfifo_underrun, bdmf_boolean *rxfifo_overrun);

/**********************************************************************************************************************
 * txfifo_underrun: 
 *     TXFIFO Underrun occurred.
 * txfifo_overrun: 
 *     TXFIFO Overrun occurred.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_txfifo_stat_get(uint8_t xumac_id, bdmf_boolean *txfifo_underrun, bdmf_boolean *txfifo_overrun);

/**********************************************************************************************************************
 * pfc_tx_enbl: 
 *     Enables the PFC-Tx functionality.
 * pfc_rx_enbl: 
 *     Enables the PFC-Rx functionality.
 * force_pfc_xon: 
 *     Instructs MAC to send Xon message to all classes of service.
 * rx_pass_pfc_frm: 
 *     When set, MAC pass PFC frame to the system. Otherwise, PFC frame is discarded.
 * pfc_stats_en: 
 *     When clear, none of PFC related counters should increment. 
 *     Otherwise, PFC counters is in full function. 
 *     Note: it is programming requirement to set this bit when PFC function is enable.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_mac_pfc_ctrl_set(uint8_t xumac_id, const xumac_rdp_mac_pfc_ctrl *mac_pfc_ctrl);
bdmf_error_t ag_drv_xumac_rdp_mac_pfc_ctrl_get(uint8_t xumac_id, xumac_rdp_mac_pfc_ctrl *mac_pfc_ctrl);

/**********************************************************************************************************************
 * pfc_refresh_en: 
 *     Enables the PFC refresh functionality on the Tx side. When enabled, the MAC sends Xoff message on refresh
 *     counter becoming 0
 * pfc_refresh_timer: 
 *     PFC refresh counter value.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_mac_pfc_refresh_ctrl_set(uint8_t xumac_id, bdmf_boolean pfc_refresh_en, uint16_t pfc_refresh_timer);
bdmf_error_t ag_drv_xumac_rdp_mac_pfc_refresh_ctrl_get(uint8_t xumac_id, bdmf_boolean *pfc_refresh_en, uint16_t *pfc_refresh_timer);

/**********************************************************************************************************************
 * count: 
 *     Receive 64 Bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr64_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gr64_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr64_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gr64_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive 65 bytes to 127 bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr127_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gr127_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr127_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gr127_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive 128 bytes to 255 bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr255_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gr255_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr255_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gr255_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive 256 bytes to 511 bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr511_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gr511_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr511_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gr511_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive 512 bytes to 1023 bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr1023_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gr1023_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr1023_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gr1023_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive 1024 bytes to 1518 bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr1518_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gr1518_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr1518_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gr1518_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive 1519 bytes to 1522 bytes good VLAN frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grmgv_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grmgv_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grmgv_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grmgv_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive 1519 bytes to 2047 bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr2047_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gr2047_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr2047_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gr2047_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive 2048 bytes to 4096 bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr4095_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gr4095_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr4095_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gr4095_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive 4096 bytes to 9216 bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr9216_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gr9216_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gr9216_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gr9216_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grpkt_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grpkt_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grpkt_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grpkt_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive byte counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grbyt_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grbyt_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u16: 
 *     Upper 16 bits of 48-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grbyt_upper_set(uint8_t xumac_id, uint16_t count_u16);
bdmf_error_t ag_drv_xumac_rdp_grbyt_upper_get(uint8_t xumac_id, uint16_t *count_u16);

/**********************************************************************************************************************
 * count: 
 *     Receive multicast packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grmca_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grmca_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grmca_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grmca_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive broadcast packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grbca_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grbca_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grbca_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grbca_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive FCS error counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grfcs_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grfcs_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grfcs_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grfcs_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive control frame packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grxcf_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grxcf_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grxcf_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grxcf_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive pause frame packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grxpf_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grxpf_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grxpf_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grxpf_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive unknown op code packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grxuo_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grxuo_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grxuo_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grxuo_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive alignmenet error counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_graln_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_graln_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_graln_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_graln_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive frame length out of range counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grflr_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grflr_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grflr_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grflr_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive code error packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grcde_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grcde_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grcde_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grcde_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive carrier sense error packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grfcr_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grfcr_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grfcr_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grfcr_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive oversize packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grovr_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grovr_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grovr_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grovr_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive jabber counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grjbr_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grjbr_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grjbr_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grjbr_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive MTU error packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grmtue_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grmtue_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grmtue_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grmtue_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive good packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grpok_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grpok_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grpok_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grpok_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Received unicast packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gruc_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gruc_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gruc_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gruc_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive PPP packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grppp_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grppp_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grppp_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grppp_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Receive CRC match packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grcrc_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_grcrc_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_grcrc_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_grcrc_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit 64 bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr64_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_tr64_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr64_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_tr64_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit 65 bytes to 127 bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr127_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_tr127_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr127_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_tr127_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit 128 bytes to 255 bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr255_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_tr255_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr255_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_tr255_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit 256 bytes to 511 bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr511_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_tr511_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr511_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_tr511_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit 512 bytes to 1023 bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr1023_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_tr1023_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr1023_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_tr1023_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit 1024 bytes to 1518 bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr1518_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_tr1518_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr1518_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_tr1518_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit 1519 bytes to 1522 bytes good VLAN frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_trmgv_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_trmgv_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_trmgv_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_trmgv_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit 1519 bytes to 2047 bytes Frame Counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr2047_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_tr2047_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr2047_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_tr2047_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit 2048 bytes to 4095 bytes frame counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr4095_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_tr4095_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr4095_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_tr4095_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit 4096 bytes to 9216 bytes Frame Counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr9216_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_tr9216_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tr9216_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_tr9216_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtpkt_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtpkt_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtpkt_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtpkt_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit multicast packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtmca_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtmca_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtmca_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtmca_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit broadcast packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtbca_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtbca_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtbca_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtbca_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit pause frame packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtxpf_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtxpf_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtxpf_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtxpf_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit control frame packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtxcf_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtxcf_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtxcf_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtxcf_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit FCS error counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtfcs_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtfcs_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtfcs_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtfcs_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit oversize packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtovr_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtovr_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtovr_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtovr_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit deferral packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtdrf_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtdrf_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtdrf_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtdrf_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit excessive deferral packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtedf_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtedf_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtedf_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtedf_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit single collision packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtscl_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtscl_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtscl_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtscl_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit multiple collision packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtmcl_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtmcl_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtmcl_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtmcl_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit late collision packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtlcl_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtlcl_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtlcl_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtlcl_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit excessive collision packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtxcl_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtxcl_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtxcl_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtxcl_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit fragments packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtfrg_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtfrg_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtfrg_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtfrg_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit total collision counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtncl_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtncl_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtncl_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtncl_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit jabber counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtjbr_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtjbr_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtjbr_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtjbr_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmit byte counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtbyt_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtbyt_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u16: 
 *     Upper 16 bits of 48-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtbyt_upper_set(uint8_t xumac_id, uint16_t count_u16);
bdmf_error_t ag_drv_xumac_rdp_gtbyt_upper_get(uint8_t xumac_id, uint16_t *count_u16);

/**********************************************************************************************************************
 * count: 
 *     Transmitted good packets counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtpok_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtpok_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtpok_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtpok_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     Transmitted Unicast packets counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtuc_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_gtuc_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gtuc_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_gtuc_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     RX RUNT packet counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_rrpkt_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_rrpkt_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_rrpkt_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_rrpkt_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     RX RUNT packet with valid FCS counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_rrund_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_rrund_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_rrund_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_rrund_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     RX RUNT packet with invalid FCS or alignment error counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_rrfrg_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_rrfrg_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u8: 
 *     Upper 8 bits of 40-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_rrfrg_upper_set(uint8_t xumac_id, uint8_t count_u8);
bdmf_error_t ag_drv_xumac_rdp_rrfrg_upper_get(uint8_t xumac_id, uint8_t *count_u8);

/**********************************************************************************************************************
 * count: 
 *     RX RUNT packet byte counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_rrbyt_set(uint8_t xumac_id, uint32_t count);
bdmf_error_t ag_drv_xumac_rdp_rrbyt_get(uint8_t xumac_id, uint32_t *count);

/**********************************************************************************************************************
 * count_u16: 
 *     Upper 16 bits of 48-bit counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_rrbyt_upper_set(uint8_t xumac_id, uint16_t count_u16);
bdmf_error_t ag_drv_xumac_rdp_rrbyt_upper_get(uint8_t xumac_id, uint16_t *count_u16);

/**********************************************************************************************************************
 * rx_cnt_rst: 
 *     Active high. When this bit is set, RX statistics counters will be reseted.
 * runt_cnt_rst: 
 *     Active high. When this bit is set, Runt statistics counters will be reseted.
 * tx_cnt_rst: 
 *     Active high. When this bit is set, TX statistics counters will be reseted.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_mib_cntrl_set(uint8_t xumac_id, bdmf_boolean rx_cnt_rst, bdmf_boolean runt_cnt_rst, bdmf_boolean tx_cnt_rst);
bdmf_error_t ag_drv_xumac_rdp_mib_cntrl_get(uint8_t xumac_id, bdmf_boolean *rx_cnt_rst, bdmf_boolean *runt_cnt_rst, bdmf_boolean *tx_cnt_rst);

/**********************************************************************************************************************
 * data32: 
 *     32-bit data holder.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_mib_read_data_set(uint8_t xumac_id, uint32_t data32);
bdmf_error_t ag_drv_xumac_rdp_mib_read_data_get(uint8_t xumac_id, uint32_t *data32);

/**********************************************************************************************************************
 * data32: 
 *     32-bit data holder.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_mib_write_data_set(uint8_t xumac_id, uint32_t data32);
bdmf_error_t ag_drv_xumac_rdp_mib_write_data_get(uint8_t xumac_id, uint32_t *data32);

/**********************************************************************************************************************
 * mpd_en: 
 *     When this bit is set, Magic Packet detection is enabled.
 * mseq_len: 
 *     Number of repetitions of MAC Destination Address that must be detected in Magic Packet.
 * psw_en: 
 *     When set, enable Magic Sequence password check.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_control_set(uint8_t xumac_id, bdmf_boolean mpd_en, uint8_t mseq_len, bdmf_boolean psw_en);
bdmf_error_t ag_drv_xumac_rdp_control_get(uint8_t xumac_id, bdmf_boolean *mpd_en, uint8_t *mseq_len, bdmf_boolean *psw_en);

/**********************************************************************************************************************
 * psw_47_32: 
 *     Magic Packet Optional password bytes 0-1 (password bits [47:32]).Bits [47:40] correspond to password byte 0,
 *     which is the first password byte received from the wire.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_psw_ms_set(uint8_t xumac_id, uint16_t psw_47_32);
bdmf_error_t ag_drv_xumac_rdp_psw_ms_get(uint8_t xumac_id, uint16_t *psw_47_32);

/**********************************************************************************************************************
 * psw_31_0: 
 *     Magic Packet Optional Password bytes 2-5 (password bits [31:0]).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_psw_ls_set(uint8_t xumac_id, uint32_t psw_31_0);
bdmf_error_t ag_drv_xumac_rdp_psw_ls_get(uint8_t xumac_id, uint32_t *psw_31_0);

/**********************************************************************************************************************
 * xib_rx_en: 
 *     When set enables receive MAC. When cleared, all data received from PHY are ignored by RX MAC and no data are
 *     passed to the system.
 * xib_tx_en: 
 *     When set enables transmit MAC. When cleared all data received from the system are ignored and no data are
 *     transmitted to the wire (IDLEs are transmit instead).
 * rx_flush_en: 
 *     When set MAC receive pipe is flushed (control registers are not affected). Must be cleared by SW.
 * tx_flush_en: 
 *     When set MAC transmit pipe is flushed (control registers are not affected). Must be cleared by SW.
 * link_down_rst_en: 
 *     When set MAC flushes its RX and TX pipe when the loss of link is detected. It stays in reset until valid link
 *     status is indicated by PHY.
 * standard_mux_en: 
 *     When set glitch-less clock muxes behave as a regular clock muxes.
 *     This is debug only feature.
 * xgmii_sel: 
 *     When cleared XIB is by bypassed that is PHY's GMII interfaces is passed through. This bit is valid only when
 *     XGMII_SEL_OVRD = 1.
 * dic_dis: 
 *     When cleared GIB TX deploys DIC (Deficit Idle Counter) algorithm. This algorithms maintains 10Gbps data rate by
 *     inserting more or less idles than specified via TX_IPG.
 *     When set minimum of TX_IPG IDLEs are insert between packets leading to data rate that is lower than 10Gbps due
 *     to the SOP alignment rules.
 * rx_start_threshold: 
 *     Packet receive in GMII clock domain starts only when at least RX_START_THRESHOLD words are available in XIB RX
 *     FIFO.
 * gmii_rx_clk_gate_en: 
 *     When set XIB will gate GMII RX clock if RX FIFO becomes empty in the middle of the packet, in order to prevent
 *     packet corruption.
 * strict_preamble_dis: 
 *     When this bit is set XIB will support packets with shorter or longer than standard (i.e. less or more than 6B
 *     of 0x55) preamble. XIB will accept any number of preamble bytes (0x55) until SFD is detected. If SFD is not
 *     detected but instead SOP or EFD, current data are discarded and XIB restarts the parsing.
 *     When cleared packets with non-standard preamble are discarded.
 * tx_ipg: 
 *     An average number of XGMII IDLE characters that will be inserted between two packets on TX. The actual number
 *     of IDLEs between any two packets can be larger or smaller than this number, depending on the programmed IDLE
 *     insertion algorithm. Note that IEEE 802.3 specifies 5B as minimum IPG (TERMINATE + 4B of IDLE).
 * min_rx_ipg: 
 *     This value guaranties minimum IPG between any two of received packets. When set to 0 minimum RX IPG is not
 *     enforced that is it equals XGMII IPG (plus/minus IDLEs inserted/deleted in clock compensation purposes).
 * xgmii_sel_ovrd: 
 *     When set enables XGMII_SEL to select XIB PHY interface. When 0 interface is selected based on the HW pin.
 * gmii_tx_clk_gate_en: 
 *     When set XIB will gate GMII TX clock if the TX FIFO occupancy becomes greater or equal to its XOFF threshold.
 *     It will re-enable the clock when the TX FIFO occupancy is equal or below its XON threshold.
 *     Should not be enabled when TX_BACKPRESSURE_EN = 1.
 * autoconfig_en: 
 *     When set XIB will set N and M for clock swallower automatically based on the XGMII clock/data rate.
 *     Note: This is only applicable to case when the internal serdes is source of the clock used to generate XIB?s
 *     GMII clocks.
 *     Note: The default is the value to which i_autoconfig_en_strap is tied.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_control_1_set(uint8_t xumac_id, const xumac_rdp_control_1 *control_1);
bdmf_error_t ag_drv_xumac_rdp_control_1_get(uint8_t xumac_id, xumac_rdp_control_1 *control_1);

/**********************************************************************************************************************
 * tx_start_threshold: 
 *     Packet transmission in XGMII clock domain starts only when at least TX_START_THRESHOLD words are available in
 *     XIB TX FIFO. This threshold is applicable to cases where TX FIFO may become empty due to the link down or the
 *     other faults.
 * tx_xoff_threshold: 
 *     TX XOFF threshold. When TX FIFO depth is equal or larger than this threshold XIB asserts backpressure toward
 *     the switch port until FIFO occupancy falls below TX_XON_THERSHOLD.
 * tx_xon_threshold: 
 *     TX XON threshold. When FIFO occupancy drops below this threshold, XIB de-asserts backpressure toward switch
 *     port.
 * tx_backpressure_en: 
 *     When set enables asserting backpressure toward MAC (in between packets) when TX XOFF THRESHOLD is crossed.
 *     Should not be enabled when TX clock gating is enabled.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_extended_control_set(uint8_t xumac_id, uint16_t tx_start_threshold, uint16_t tx_xoff_threshold, uint16_t tx_xon_threshold, bdmf_boolean tx_backpressure_en);
bdmf_error_t ag_drv_xumac_rdp_extended_control_get(uint8_t xumac_id, uint16_t *tx_start_threshold, uint16_t *tx_xoff_threshold, uint16_t *tx_xon_threshold, bdmf_boolean *tx_backpressure_en);

/**********************************************************************************************************************
 * tx_idle_stuffing_ctrl: 
 *     This field controls the IDLE time. Idle time is inserted NX times when N equals to the packet size in bytes,
 *     including regular idle, preamble, SOP and EOP.
 *     4'b0000: No IDLE stuffing
 *     When the link speed is 10Gbps, available settings are:
 *     4'b0001: 1X. This provides 1/2 rate, i.e. 5Gbps
 *     4'b0010: 3X. This provides 1/4 rate, i.e. 2.5Gbps
 *     4'b0011: 9X. This provides 1/10 rate, i.e. 1Gbps
 *     4'b0100: 99X. This provides 1/100 rate, i.e. 100Mbps
 *     4'b0101: 999X. This provides 1/1000 rate, i.e. 10Mbps
 *     When the link speed is 5Gbps, available settings are:
 *     4'b0001: 1X. This provides 1/2 rate, i.e. 2.5Gbps
 *     4'b0110: 4X. This provides 1/5 rate, i.e. 1Gbps
 *     4'b0111: 49X. This provides 1/50 rate, i.e. 100Mbps
 *     4'b1000: 499X. This provides 1/500 rate, i.e. 10Mbps
 *     When the link speed is 2.5Gbps, available settings are:
 *     4'b1001: 1.5X. This provides 1/2.5 rate. i.e. 1Gbps
 *     4'b1010: 24X. This provides 1/25 rate, i.e. 100Mbps
 *     4'b1011: 249X. This provides 1/250 rate, i.e. 10Mbps
 *     4'b1100 - 4'b1111: Reserved
 * idle_stuffing_mode: 
 *     When cleared, TX_START_THRESHOLD is used from the register and idles are generated based on the XIB??s packet
 *     byte counter.
 *     When set and the idle stuffing is enabled (TX_IDLE_STUFFING_CTRL != 4'b0), idle stuffing logic uses packet size
 *     provided by the MAC on SOP (where supported) in order to calculate TX_START_THRESHOLD and as well to generate
 *     idles. This mode reduces packet transmission latency.
 * tx_start_threshold_offset: 
 *     When IDLE_STUFFING_MODE = 1, TX_START_THRESHOLD is calculated for each packet as TX_START_THRESHOLD =
 *     N*MAC_PKT_SIZE/8 + TX_START_THRESHOLD_OFFSET where MAC_PKT_SIZE is communicated by the MAC on SOP using the
 *     side-band interface and N=1/2 for 1X and 3/4 for 3X idle stuffing modes. Note that TX_START_THRESHOLD is
 *     calculated only for 1X and 3X modes and for all other modes TX_START_THRESHOLD = MAC_PKT_SIZE.
 * tx_start_on_eop: 
 *     When set XIB uses packet store and forward in idle stuffing mode.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tx_idle_stuffing_control_set(uint8_t xumac_id, uint8_t tx_idle_stuffing_ctrl, bdmf_boolean idle_stuffing_mode, uint8_t tx_start_threshold_offset, bdmf_boolean tx_start_on_eop);
bdmf_error_t ag_drv_xumac_rdp_tx_idle_stuffing_control_get(uint8_t xumac_id, uint8_t *tx_idle_stuffing_ctrl, bdmf_boolean *idle_stuffing_mode, uint8_t *tx_start_threshold_offset, bdmf_boolean *tx_start_on_eop);

/**********************************************************************************************************************
 * actual_data_rate: 
 *     Actual Data Rate. Accounts for IDLE stuffing. Encoded as:
 *     3'b000 - 10Mbps
 *     3'b001 - 100Mbps
 *     3'b010 - 1Gbps
 *     3'b011 - 2.5Gbps
 *     3'b100 - 10Gbps
 *     3'b101 - 5Gbps
 *     3'b110 - 3'b111 - Reserved
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_actual_data_rate_set(uint8_t xumac_id, uint8_t actual_data_rate);
bdmf_error_t ag_drv_xumac_rdp_actual_data_rate_get(uint8_t xumac_id, uint8_t *actual_data_rate);

/**********************************************************************************************************************
 * ndiv: 
 *     GMII clock swallower divisor. GMII clock swallower produces MDIV output clocks for every NDIV input clocks
 *     resulting in average MAC GMII RX and TX clock frequency of MDIV/NDIV*INPUT_CLOCK_FREQUENCY. Used only during
 *     XGMII to/from GMII conversion.
 * mdiv: 
 *     GMII clock swallower dividend. Must be <= NDIV. Used only during XGMII to/from GMII conversion.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_gmii_clock_swallower_control_set(uint8_t xumac_id, uint8_t ndiv, uint8_t mdiv);
bdmf_error_t ag_drv_xumac_rdp_gmii_clock_swallower_control_get(uint8_t xumac_id, uint8_t *ndiv, uint8_t *mdiv);

/**********************************************************************************************************************
 * xgmii_data_rate: 
 *     Indicates data rate over XGMII interface. Used to select GMII operating clocks.
 *     Encoded as:
 *     2'b00 - 2.5Gbps
 *     2'b01 - 5Gbps
 *     2'b10 - 10Gbps
 *     2'b11 - Reserved
 *     VAlid only wehn XGMII_SEL=1.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_xgmii_data_rate_status_get(uint8_t xumac_id, uint8_t *xgmii_data_rate);

/**********************************************************************************************************************
 * rx_fifo_overrun: 
 *     When set indicates that RX FIFO had overflow.
 *     Cleared on read.
 * rx_fifo_underrun: 
 *     When set indicates that RX FIFO become empty in the middle of the frame.
 *     Cleared on read.
 * tx_fifo_underrun: 
 *     When set indicates that TX FIFO become empty in the middle of the frame.
 *     Cleared on read.
 * tx_fifo_overrun: 
 *     When set indicates that TX FIFO had overflow.
 *     Cleared on read.
 * rx_fault_status: 
 *     Received faults status encoded as:
 *     2'b00 - No Fault
 *     2'b01 - Local Fault
 *     2'b10 - Remote Fault
 *     2'b11 - Link Interruption
 *     Cleared on read.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_status_get(uint8_t xumac_id, xumac_rdp_status *status);

/**********************************************************************************************************************
 * pkt_count: 
 *     This counter is a free running counter that counts received packets that are discarded by XIB due to framing or
 *     other irregularities.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_rx_discard_packet_counter_set(uint8_t xumac_id, uint32_t pkt_count);
bdmf_error_t ag_drv_xumac_rdp_rx_discard_packet_counter_get(uint8_t xumac_id, uint32_t *pkt_count);

/**********************************************************************************************************************
 * pkt_count: 
 *     This counter is a free running counter that counts transmitted packets that are discarded by XIB due to framing
 *     or other irregularities.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_tx_discard_packet_counter_set(uint8_t xumac_id, uint32_t pkt_count);
bdmf_error_t ag_drv_xumac_rdp_tx_discard_packet_counter_get(uint8_t xumac_id, uint32_t *pkt_count);

/**********************************************************************************************************************
 * sys_port_rev: 
 *     XUMAC revision code.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_rev_get(uint8_t xumac_id, uint16_t *sys_port_rev);

/**********************************************************************************************************************
 * mac_rxerr_mask: 
 *     Mask for RSV[33:16].
 *     The effective MAC_RXERR will be: |(RSV[33:16] & UMAC_RXERR_MASK)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_umac_rxerr_mask_set(uint8_t xumac_id, uint32_t mac_rxerr_mask);
bdmf_error_t ag_drv_xumac_rdp_umac_rxerr_mask_get(uint8_t xumac_id, uint32_t *mac_rxerr_mask);

/**********************************************************************************************************************
 * max_pkt_size: 
 *     This value is used by MIB counters to differentiate regular size packets from oversized packets (used for
 *     statistics counting only).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_xumac_rdp_mib_max_pkt_size_set(uint8_t xumac_id, uint16_t max_pkt_size);
bdmf_error_t ag_drv_xumac_rdp_mib_max_pkt_size_get(uint8_t xumac_id, uint16_t *max_pkt_size);

#ifdef USE_BDMF_SHELL
enum
{
    cli_xumac_rdp_ipg_hd_bkp_cntl,
    cli_xumac_rdp_command_config,
    cli_xumac_rdp_mac_0,
    cli_xumac_rdp_mac_1,
    cli_xumac_rdp_frm_length,
    cli_xumac_rdp_pause_quant,
    cli_xumac_rdp_tx_ts_seq_id,
    cli_xumac_rdp_sfd_offset,
    cli_xumac_rdp_mac_mode,
    cli_xumac_rdp_tag_0,
    cli_xumac_rdp_tag_1,
    cli_xumac_rdp_rx_pause_quanta_scale,
    cli_xumac_rdp_tx_preamble,
    cli_xumac_rdp_tx_ipg_length,
    cli_xumac_rdp_pfc_xoff_timer,
    cli_xumac_rdp_umac_eee_ctrl,
    cli_xumac_rdp_mii_eee_delay_entry_timer,
    cli_xumac_rdp_gmii_eee_delay_entry_timer,
    cli_xumac_rdp_umac_eee_ref_count,
    cli_xumac_rdp_umac_timestamp_adjust,
    cli_xumac_rdp_umac_rx_pkt_drop_status,
    cli_xumac_rdp_umac_symmetric_idle_threshold,
    cli_xumac_rdp_mii_eee_wake_timer,
    cli_xumac_rdp_gmii_eee_wake_timer,
    cli_xumac_rdp_umac_rev_id,
    cli_xumac_rdp_gmii_2p5g_eee_delay_entry_timer,
    cli_xumac_rdp_gmii_5g_eee_delay_entry_timer,
    cli_xumac_rdp_gmii_10g_eee_delay_entry_timer,
    cli_xumac_rdp_gmii_2p5g_eee_wake_timer,
    cli_xumac_rdp_gmii_5g_eee_wake_timer,
    cli_xumac_rdp_gmii_10g_eee_wake_timer,
    cli_xumac_rdp_active_eee_delay_entry_timer,
    cli_xumac_rdp_active_eee_wake_timer,
    cli_xumac_rdp_mac_pfc_type,
    cli_xumac_rdp_mac_pfc_opcode,
    cli_xumac_rdp_mac_pfc_da_0,
    cli_xumac_rdp_mac_pfc_da_1,
    cli_xumac_rdp_macsec_prog_tx_crc,
    cli_xumac_rdp_macsec_cntrl,
    cli_xumac_rdp_ts_status,
    cli_xumac_rdp_tx_ts_data,
    cli_xumac_rdp_pause_refresh_ctrl,
    cli_xumac_rdp_flush_control,
    cli_xumac_rdp_rxfifo_stat,
    cli_xumac_rdp_txfifo_stat,
    cli_xumac_rdp_mac_pfc_ctrl,
    cli_xumac_rdp_mac_pfc_refresh_ctrl,
    cli_xumac_rdp_gr64,
    cli_xumac_rdp_gr64_upper,
    cli_xumac_rdp_gr127,
    cli_xumac_rdp_gr127_upper,
    cli_xumac_rdp_gr255,
    cli_xumac_rdp_gr255_upper,
    cli_xumac_rdp_gr511,
    cli_xumac_rdp_gr511_upper,
    cli_xumac_rdp_gr1023,
    cli_xumac_rdp_gr1023_upper,
    cli_xumac_rdp_gr1518,
    cli_xumac_rdp_gr1518_upper,
    cli_xumac_rdp_grmgv,
    cli_xumac_rdp_grmgv_upper,
    cli_xumac_rdp_gr2047,
    cli_xumac_rdp_gr2047_upper,
    cli_xumac_rdp_gr4095,
    cli_xumac_rdp_gr4095_upper,
    cli_xumac_rdp_gr9216,
    cli_xumac_rdp_gr9216_upper,
    cli_xumac_rdp_grpkt,
    cli_xumac_rdp_grpkt_upper,
    cli_xumac_rdp_grbyt,
    cli_xumac_rdp_grbyt_upper,
    cli_xumac_rdp_grmca,
    cli_xumac_rdp_grmca_upper,
    cli_xumac_rdp_grbca,
    cli_xumac_rdp_grbca_upper,
    cli_xumac_rdp_grfcs,
    cli_xumac_rdp_grfcs_upper,
    cli_xumac_rdp_grxcf,
    cli_xumac_rdp_grxcf_upper,
    cli_xumac_rdp_grxpf,
    cli_xumac_rdp_grxpf_upper,
    cli_xumac_rdp_grxuo,
    cli_xumac_rdp_grxuo_upper,
    cli_xumac_rdp_graln,
    cli_xumac_rdp_graln_upper,
    cli_xumac_rdp_grflr,
    cli_xumac_rdp_grflr_upper,
    cli_xumac_rdp_grcde,
    cli_xumac_rdp_grcde_upper,
    cli_xumac_rdp_grfcr,
    cli_xumac_rdp_grfcr_upper,
    cli_xumac_rdp_grovr,
    cli_xumac_rdp_grovr_upper,
    cli_xumac_rdp_grjbr,
    cli_xumac_rdp_grjbr_upper,
    cli_xumac_rdp_grmtue,
    cli_xumac_rdp_grmtue_upper,
    cli_xumac_rdp_grpok,
    cli_xumac_rdp_grpok_upper,
    cli_xumac_rdp_gruc,
    cli_xumac_rdp_gruc_upper,
    cli_xumac_rdp_grppp,
    cli_xumac_rdp_grppp_upper,
    cli_xumac_rdp_grcrc,
    cli_xumac_rdp_grcrc_upper,
    cli_xumac_rdp_tr64,
    cli_xumac_rdp_tr64_upper,
    cli_xumac_rdp_tr127,
    cli_xumac_rdp_tr127_upper,
    cli_xumac_rdp_tr255,
    cli_xumac_rdp_tr255_upper,
    cli_xumac_rdp_tr511,
    cli_xumac_rdp_tr511_upper,
    cli_xumac_rdp_tr1023,
    cli_xumac_rdp_tr1023_upper,
    cli_xumac_rdp_tr1518,
    cli_xumac_rdp_tr1518_upper,
    cli_xumac_rdp_trmgv,
    cli_xumac_rdp_trmgv_upper,
    cli_xumac_rdp_tr2047,
    cli_xumac_rdp_tr2047_upper,
    cli_xumac_rdp_tr4095,
    cli_xumac_rdp_tr4095_upper,
    cli_xumac_rdp_tr9216,
    cli_xumac_rdp_tr9216_upper,
    cli_xumac_rdp_gtpkt,
    cli_xumac_rdp_gtpkt_upper,
    cli_xumac_rdp_gtmca,
    cli_xumac_rdp_gtmca_upper,
    cli_xumac_rdp_gtbca,
    cli_xumac_rdp_gtbca_upper,
    cli_xumac_rdp_gtxpf,
    cli_xumac_rdp_gtxpf_upper,
    cli_xumac_rdp_gtxcf,
    cli_xumac_rdp_gtxcf_upper,
    cli_xumac_rdp_gtfcs,
    cli_xumac_rdp_gtfcs_upper,
    cli_xumac_rdp_gtovr,
    cli_xumac_rdp_gtovr_upper,
    cli_xumac_rdp_gtdrf,
    cli_xumac_rdp_gtdrf_upper,
    cli_xumac_rdp_gtedf,
    cli_xumac_rdp_gtedf_upper,
    cli_xumac_rdp_gtscl,
    cli_xumac_rdp_gtscl_upper,
    cli_xumac_rdp_gtmcl,
    cli_xumac_rdp_gtmcl_upper,
    cli_xumac_rdp_gtlcl,
    cli_xumac_rdp_gtlcl_upper,
    cli_xumac_rdp_gtxcl,
    cli_xumac_rdp_gtxcl_upper,
    cli_xumac_rdp_gtfrg,
    cli_xumac_rdp_gtfrg_upper,
    cli_xumac_rdp_gtncl,
    cli_xumac_rdp_gtncl_upper,
    cli_xumac_rdp_gtjbr,
    cli_xumac_rdp_gtjbr_upper,
    cli_xumac_rdp_gtbyt,
    cli_xumac_rdp_gtbyt_upper,
    cli_xumac_rdp_gtpok,
    cli_xumac_rdp_gtpok_upper,
    cli_xumac_rdp_gtuc,
    cli_xumac_rdp_gtuc_upper,
    cli_xumac_rdp_rrpkt,
    cli_xumac_rdp_rrpkt_upper,
    cli_xumac_rdp_rrund,
    cli_xumac_rdp_rrund_upper,
    cli_xumac_rdp_rrfrg,
    cli_xumac_rdp_rrfrg_upper,
    cli_xumac_rdp_rrbyt,
    cli_xumac_rdp_rrbyt_upper,
    cli_xumac_rdp_mib_cntrl,
    cli_xumac_rdp_mib_read_data,
    cli_xumac_rdp_mib_write_data,
    cli_xumac_rdp_control,
    cli_xumac_rdp_psw_ms,
    cli_xumac_rdp_psw_ls,
    cli_xumac_rdp_control_1,
    cli_xumac_rdp_extended_control,
    cli_xumac_rdp_tx_idle_stuffing_control,
    cli_xumac_rdp_actual_data_rate,
    cli_xumac_rdp_gmii_clock_swallower_control,
    cli_xumac_rdp_xgmii_data_rate_status,
    cli_xumac_rdp_status,
    cli_xumac_rdp_rx_discard_packet_counter,
    cli_xumac_rdp_tx_discard_packet_counter,
    cli_xumac_rdp_rev,
    cli_xumac_rdp_umac_rxerr_mask,
    cli_xumac_rdp_mib_max_pkt_size,
};

int bcm_xumac_rdp_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_xumac_rdp_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
