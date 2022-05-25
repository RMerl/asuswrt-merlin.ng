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

#ifndef _BCM4912_XPORT_XLMAC_CORE_AG_H_
#define _BCM4912_XPORT_XLMAC_CORE_AG_H_

#include "access_macros.h"
#include "bcmtypes.h"

/**************************************************************************************************/
/* extended_hig2_en:  - Extended Higig 2 header is also known as sirius header. Setting this bit  */
/*                   to 0 will disable parsing for the extended header bit(5th bit of 8th header  */
/*                   byte) in HG2 header and hence all the Higig 2 packets will be treated as nor */
/*                   mal Higig2 packets irrespective of extended header bit value. Default value  */
/*                   of this field is 1 which will enable parsing extended header bit in every Hi */
/*                   gig 2 header.                                                                */
/* link_status_select:  - This configuration chooses between link status indication from software */
/*                      (SW_LINK_STATUS) or the hardware link status (hw_link_status)indication f */
/*                     rom the TSC. If reset, it selects the software link status                 */
/* sw_link_status:  - Link status indication from Software. If set, indicates that link is active */
/*                 .                                                                              */
/* xgmii_ipg_check_disable:  - If set, this will override the one column idle/sequence ordered se */
/*                          t check before SOP in XGMII mode - effectively supporting  reception  */
/*                          of packets with 1 byte IPG in XGMII mode                              */
/* rs_soft_reset:  - Resets the RS layer functionality - Fault detection and related responses ar */
/*                e disabled and IDLEs are sent on line                                           */
/* rsvd_5:  - Reserved                                                                            */
/* local_lpbk_leak_enb:  - If set, during the local loopback mode, the transmit packets are also  */
/*                      sent to the transmit line interface, apart from the loopback operation    */
/* rsvd_4:  - Reserved                                                                            */
/* soft_reset:  - If set, disables the corresponding port logic and status registers only. Packet */
/*              data and flow control logic is disabled. Fault handling is active and the MAC wil */
/*             l continue to respond to credits from TSC. When the soft reset is cleared MAC will */
/*              issue a fresh set of credits to EP in transmit side.                              */
/* lag_failover_en:  - If set, enable LAG Failover. This bit has priority over LOCAL_LPBK. The la */
/*                  g failover kicks in when the link status selected by LINK_STATUS_SELECT trans */
/*                  itions from 1 to 0. TSC clock and TSC credits must be active for lag failover */
/*                  .                                                                             */
/* remove_failover_lpbk:  - If set, XLMAC will move from lag failover state to normal operation.  */
/*                       This bit should be set after link is up.                                 */
/* rsvd_1:  - Reserved                                                                            */
/* local_lpbk:  - If set, enables local loopback from TX to RX. This loopback is on the line side */
/*              after clock domain crossing - from the last TX pipeline stage to the first RX pip */
/*             eline stage. Hence, TSC clock and TSC credits must be active for loopback. LAG_FAI */
/*             LOVER_EN should be disabled for this bit to work.                                  */
/* rx_en:  - If set, enables MAC receive datapath and flowcontrol logic.                          */
/* tx_en:  - If set, enables MAC transmit datapath and flowcontrol logic. When disabled, MAC will */
/*         respond to TSC credits with IDLE codewords.                                            */
/**************************************************************************************************/
typedef struct
{
    uint8_t extended_hig2_en;
    uint8_t link_status_select;
    uint8_t sw_link_status;
    uint8_t xgmii_ipg_check_disable;
    uint8_t rs_soft_reset;
    uint8_t rsvd_5;
    uint8_t local_lpbk_leak_enb;
    uint8_t rsvd_4;
    uint8_t soft_reset;
    uint8_t lag_failover_en;
    uint8_t remove_failover_lpbk;
    uint8_t rsvd_1;
    uint8_t local_lpbk;
    uint8_t rx_en;
    uint8_t tx_en;
} xport_xlmac_core_ctrl;


/**************************************************************************************************/
/* tx_threshold:  - Indicates the number of 16-byte cells that are buffered per packet in the Tx  */
/*               CDC FIFO, before starting transmission of the packet on the line side.
This sett */
/*               ing is useful to prevent underflow issues if the EP logic pumps in data at port  */
/*               rate, rather than bursting at full rate.
This mode will increase the overall lat */
/*               ency.
In quad port mode, this field should be set >= 1 and <= 4 for each port.
I */
/*               n single port mode, this field should be set >= 1 and <= 16 for the four lane po */
/*               rt (port0).
In dual port mode, this field should be set >= 1 and <= 8 for each t */
/*               wo lane port (port0 and port2).
In tri1/tri2, this field should be set >= 1 and  */
/*               <= 4 for each single lane port, and >= 1 and <= 8 for the two lane port.         */
/* ep_discard:  - If set, MAC accepts packets from the EP but does not write to the CDC FIFO and  */
/*             discards them on the core side without updating the statistics.                    */
/* tx_preamble_length:  - Number of preamble bytes for transmit IEEE packets, this value should i */
/*                     nclude the K.SOP & SFD character as well                                   */
/* throt_denom:  - Number of bytes to transmit before adding THROT_NUM bytes to the IPG.  This co */
/*              nfiguration is used for WAN IPG throttling. Refer MAC specs for more details.     */
/* throt_num:  - Number of bytes of extra IPG added whenever THROT_DENOM bytes have been transmit */
/*            ted. This configuration is used for WAN IPG throttling. Refer MAC specs for more de */
/*            tails.                                                                              */
/* average_ipg:  - Average interpacket gap. Must be programmed >= 8. Per packet IPG will vary bas */
/*              ed on DIC for 10G+ speeds.                                                        */
/* pad_threshold:  - If padding is enabled, packets smaller than PAD_THRESHOLD are padded to this */
/*                 size. This must be set to a value >= 17 (decimal)                              */
/* pad_en:  - If set, enable XLMAC to pad packets smaller than PAD_THRESHOLD on the Tx            */
/* tx_any_start:  - If reset, MAC forces the first byte of a packet to be /S/ character (0xFB) ir */
/*               respective of incoming EP data at SOP location in HIGIG modes                    */
/* discard:  - If set, MAC accepts packets from the EP and discards them on the line side.  The s */
/*          tatistics are updated.                                                                */
/* crc_mode:  - CRC mode for Transmit Side                                                        */
/**************************************************************************************************/
typedef struct
{
    uint8_t tx_threshold;
    uint8_t ep_discard;
    uint8_t tx_preamble_length;
    uint8_t throt_denom;
    uint8_t throt_num;
    uint8_t average_ipg;
    uint8_t pad_threshold;
    uint8_t pad_en;
    uint8_t tx_any_start;
    uint8_t discard;
    uint8_t crc_mode;
} xport_xlmac_core_tx_ctrl;


/**************************************************************************************************/
/* rx_pass_pfc:  - This configuration is used to pass or drop pfc packetw when pfc_ether_type is  */
/*              not equal to 0x8808.              
If set, PFC frames are passed to system side.  */
/*              Otherwise, PFC frames are dropped in XLMAC.
This configuration is used in Rx CDC  */
/*              mode only.                                                                        */
/* rx_pass_pause:  - If set, PAUSE frames are passed to sytem side. Otherwise, PAUSE frames are d */
/*                ropped in XLMAC 
This configuration is used in Rx CDC mode only.                */
/* rx_pass_ctrl:  - This configuration is used to drop or pass all control frames (with ether typ */
/*               e 0x8808) except pause packets.
If set, all control frames are passed to system  */
/*               side. 
Otherwise, control frames (including pfc frames wih ether type 0x8808) ar */
/*               e dropped in XLMAC.
This configuration is used in Rx CDC mode only.              */
/* rsvd_3:  - Reserved                                                                            */
/* rsvd_2:  - Reserved                                                                            */
/* runt_threshold:  - The runt threshold, below which the packets are dropped (CDC mode) or marke */
/*                 d as runt (Low latency mode). Should be programmed < 96 bytes (decimal)        */
/* strict_preamble:  - If set, MAC checks for IEEE Ethernet preamble - K.SOP +  6 "0x55" preamble */
/*                   bytes + "0xD5" SFD character - if this sequence is missing it is treated as  */
/*                  an errored packet                                                             */
/* strip_crc:  - If set, CRC is stripped from the received packet                                 */
/* rx_any_start:  - If set, MAC allows any undefined control character to start a packet          */
/* rsvd_1:  - Reserved                                                                            */
/**************************************************************************************************/
typedef struct
{
    uint8_t rx_pass_pfc;
    uint8_t rx_pass_pause;
    uint8_t rx_pass_ctrl;
    uint8_t rsvd_3;
    uint8_t rsvd_2;
    uint8_t runt_threshold;
    uint8_t strict_preamble;
    uint8_t strip_crc;
    uint8_t rx_any_start;
    uint8_t rsvd_1;
} xport_xlmac_core_rx_ctrl;


/**************************************************************************************************/
/* reset_flow_control_timers_on_link_down:  - If set, the Receive Pause, PFC & LLFC timers are re */
/*                                         set whenever the link status is down, or local or remo */
/*                                         te faults are received.                                */
/* drop_tx_data_on_link_interrupt:  - This bit determines the way MAC handles data during link in */
/*                                 terruption state, if LINK_INTERRUPTION_DISABLE is reset.
If se */
/*                                 t, during link interruption state, MAC drops transmit-data (st */
/*                                 atistics are updated) and sends IDLEs on the wire.
If reset, t */
/*                                 ransmit data is stalled in the internal FIFO under link interr */
/*                                 uption state.                                                  */
/* drop_tx_data_on_remote_fault:  - This bit determines the way MAC handles data during remote fa */
/*                               ult state, if REMOTE_FAULT_DISABLE is reset.
If set, during remo */
/*                               te fault state, MAC drops transmit-data (statistics are updated) */
/*                                and sends IDLEs on the wire.
If reset, transmit data is stalled */
/*                                in the internal FIFO under remote fault state.                  */
/* drop_tx_data_on_local_fault:  - This bit determines the way MAC handles data during local faul */
/*                              t state, if LOCAL_FAULT_DISABLE is reset.
If set, during local fa */
/*                              ult state, MAC drops transmit-data (statistics are updated) and s */
/*                              ends remote faults on the wire.
If reset, transmit data is stalle */
/*                              d in the internal FIFO under local fault state.                   */
/* link_interruption_disable:  - This bit determines the transmit response during link interrupti */
/*                            on state. The LINK_INTERRUPTION_STATUS bit is always updated irresp */
/*                            ective of this configuration.
If set, MAC will continue to transmit */
/*                             data irrespective of LINK_INTERRUPTION_STATUS.
If reset, MAC trans */
/*                            mit behavior is governed by DROP_TX_DATA_ON_LINK_INTERRUPT configur */
/*                            ation.                                                              */
/* use_external_faults_for_tx:  - If set, the transmit fault responses are determined from input  */
/*                             pins rather than internal receive status. 
In this mode, input fau */
/*                             lt from pins (from a peer MAC) is directly relayed on the transmit */
/*                              side of this MAC.
See specification document for more details.    */
/* remote_fault_disable:  - This bit determines the transmit response during remote fault state.  */
/*                       The REMOTE_FAULT_STATUS bit is always updated irrespective of this confi */
/*                       guration.
If set, MAC will continue to transmit data irrespective of REM */
/*                       OTE_FAULT_STATUS.
If reset, MAC transmit behavior is governed by DROP_TX */
/*                       _DATA_ON_REMOTE_FAULT configuration.                                     */
/* local_fault_disable:  - This bit determines the transmit response during local fault state. Th */
/*                      e LOCAL_FAULT_STATUS bit is always updated irrespective of this configura */
/*                      tion.
If set, MAC will continue to transmit data irrespective of LOCAL_FA */
/*                      ULT_STATUS.
If reset, MAC transmit behavior is governed by DROP_TX_DATA_O */
/*                      N_LOCAL_FAULT configuration.                                              */
/**************************************************************************************************/
typedef struct
{
    uint8_t reset_flow_control_timers_on_link_down;
    uint8_t drop_tx_data_on_link_interrupt;
    uint8_t drop_tx_data_on_remote_fault;
    uint8_t drop_tx_data_on_local_fault;
    uint8_t link_interruption_disable;
    uint8_t use_external_faults_for_tx;
    uint8_t remote_fault_disable;
    uint8_t local_fault_disable;
} xport_xlmac_core_rx_lss_ctrl;


/**************************************************************************************************/
/* pause_xoff_timer:  - Pause time value sent in the timer field for XOFF state (unit is 512 bit- */
/*                   times)                                                                       */
/* rsvd_2:  - Reserved                                                                            */
/* rsvd_1:  - Reserved                                                                            */
/* rx_pause_en:  - When set, enables detection of pause frames in the receive direction and pause */
/*              /resume the transmit data path                                                    */
/* tx_pause_en:  - When set, enables the transmission of pause frames whenever there is a transit */
/*              ion on txbkp input to MAC from MMU                                                */
/* pause_refresh_en:  - When set, enables the periodic re-generation of XOFF pause frames based o */
/*                   n the interval specified in PAUSE_REFRESH_TIMER                              */
/* pause_refresh_timer:  - This field specifies the interval at which pause frames are re-generat */
/*                      ed during XOFF state, provided PAUSE_REFRESH_EN is set (unit is 512 bit-t */
/*                      imes)                                                                     */
/**************************************************************************************************/
typedef struct
{
    uint16_t pause_xoff_timer;
    uint8_t rsvd_2;
    uint8_t rsvd_1;
    uint8_t rx_pause_en;
    uint8_t tx_pause_en;
    uint8_t pause_refresh_en;
    uint16_t pause_refresh_timer;
} xport_xlmac_core_pause_ctrl;


/**************************************************************************************************/
/* tx_pfc_en:  - When set, enables the transmission of PFC frames                                 */
/* rx_pfc_en:  - When set, enables detection of PFC frames in the receive direction and generatio */
/*            n of COSMAPs to MMU based on incoming timer values                                  */
/* pfc_stats_en:  - When set, enables the generation of receive and transmit PFC events into the  */
/*               corresponding statistics vectors (RSV and TSV)                                   */
/* rsvd:  - Reserved                                                                              */
/* force_pfc_xon:  - When set, forces the MAC to generate an XON indication to the MMU for all cl */
/*                asses of service in the receive direction                                       */
/* pfc_refresh_en:  - When set, enables the periodic re-generation of PFC frames based on the int */
/*                 erval specified in PFC_REFRESH_TIMER                                           */
/* pfc_xoff_timer:  - Pause time value sent in the timer field for classes in XOFF state (unit is */
/*                  512 bit-times)                                                                */
/* pfc_refresh_timer:  - This field specifies the interval at which PFC frames are re-generated f */
/*                    or a class of service in XOFF state, provided PFC_REFRESH_EN is set (unit i */
/*                    s 512 bit-times)                                                            */
/**************************************************************************************************/
typedef struct
{
    uint8_t tx_pfc_en;
    uint8_t rx_pfc_en;
    uint8_t pfc_stats_en;
    uint8_t rsvd;
    uint8_t force_pfc_xon;
    uint8_t pfc_refresh_en;
    uint16_t pfc_xoff_timer;
    uint16_t pfc_refresh_timer;
} xport_xlmac_core_pfc_ctrl;


/**************************************************************************************************/
/* llfc_img:  - This field indicates the minimum Inter Message Gap that is enforced by the MAC be */
/*           tween 2 LLFC messages in the transmit direction (unit is 1 credit)                   */
/* no_som_for_crc_llfc:  - When set, LLFC CRC computation does not include the SOM character      */
/* llfc_crc_ignore:  - When set, disables the CRC check for LLFC messages in the receive directio */
/*                  n                                                                             */
/* llfc_cut_through_mode:  - When LLFC_IN_IPG_ONLY is reset, the mode of transmission of LLFC mes */
/*                        sages is controlled by this bit depending upon whether the LLFC message */
/*                         is XON or XOFF
When LLFC_CUT_THROUGH_MODE is reset, all LLFC messages  */
/*                        are transmitted pre-emptively (within a packet)
When LLFC_CUT_THROUGH_M */
/*                        ODE is set, only XOFF LLFC messages are transmitted pre-emptively, XON  */
/*                        LLFC messages are transmitted during IPG                                */
/* llfc_in_ipg_only:  - When set, all LLFC messages are transmitted during IPG
When reset, the mo */
/*                   de of insertion of LLFC messages is controlled by LLFC_CUT_THROUGH_MODE      */
/* rx_llfc_en:  - When set, enables processing of LLFC frames in the receive direction and genera */
/*             tion of COSMAPs to MMU                                                             */
/* tx_llfc_en:  - When set, enables the generation and transmission of LLFC frames in the transmi */
/*             t direction                                                                        */
/**************************************************************************************************/
typedef struct
{
    uint8_t llfc_img;
    uint8_t no_som_for_crc_llfc;
    uint8_t llfc_crc_ignore;
    uint8_t llfc_cut_through_mode;
    uint8_t llfc_in_ipg_only;
    uint8_t rx_llfc_en;
    uint8_t tx_llfc_en;
} xport_xlmac_core_llfc_ctrl;


/**************************************************************************************************/
/* link_status:  - This bit indicates the link status used by XLMAC EEE and lag-failover state ma */
/*              chines. This reflects the live status of the link as seen by the MAC. If set, ind */
/*              icates that link is active.                                                       */
/* rx_pkt_overflow:  - If set, indicates RX packet fifo overflow                                  */
/* tx_ts_fifo_overflow:  - If set, indicates overflow occurred in TX two-step Time Stamp FIFO     */
/* tx_llfc_msg_overflow:  - If set, indicates TX LLFC message fifo overflow                       */
/* rsvd_2:  - Reserved                                                                            */
/* tx_pkt_overflow:  - If set, indicates tx packet fifo overflow                                  */
/* tx_pkt_underflow:  - If set, indicates tx packet fifo underflow                                */
/* rx_msg_overflow:  - If set, indicates rx message fifo overflow                                 */
/* rsvd_1:  - Reserved                                                                            */
/**************************************************************************************************/
typedef struct
{
    uint8_t link_status;
    uint8_t rx_pkt_overflow;
    uint8_t tx_ts_fifo_overflow;
    uint8_t tx_llfc_msg_overflow;
    uint8_t rsvd_2;
    uint8_t tx_pkt_overflow;
    uint8_t tx_pkt_underflow;
    uint8_t rx_msg_overflow;
    uint8_t rsvd_1;
} xport_xlmac_core_fifo_status;


/**************************************************************************************************/
/* clear_rx_pkt_overflow:  - A rising edge on this register bit (0->1), clears the sticky RX_PKT_ */
/*                        OVERFLOW status bit.                                                    */
/* clear_tx_ts_fifo_overflow:  - A rising edge on this register bit (0->1), clears the sticky TX_ */
/*                            TS_FIFO_OVERFLOW status bit.                                        */
/* clear_tx_llfc_msg_overflow:  - A rising edge on this register bit (0->1), clears the sticky TX */
/*                             _LLFC_MSG_OVERFLOW status bit.                                     */
/* rsvd_2:  - Reserved                                                                            */
/* clear_tx_pkt_overflow:  - A rising edge on this register bit (0->1), clears the sticky TX_PKT_ */
/*                        OVERFLOW status bit.                                                    */
/* clear_tx_pkt_underflow:  - A rising edge on this register bit (0->1), clears the sticky TX_PKT */
/*                         _UNDERFLOW status bit                                                  */
/* clear_rx_msg_overflow:  - A rising edge on this register bit (0->1), clears the sticky RX_MSG_ */
/*                        OVERFLOW status bit                                                     */
/* rsvd_1:  - Reserved                                                                            */
/**************************************************************************************************/
typedef struct
{
    uint8_t clear_rx_pkt_overflow;
    uint8_t clear_tx_ts_fifo_overflow;
    uint8_t clear_tx_llfc_msg_overflow;
    uint8_t rsvd_2;
    uint8_t clear_tx_pkt_overflow;
    uint8_t clear_tx_pkt_underflow;
    uint8_t clear_rx_msg_overflow;
    uint8_t rsvd_1;
} xport_xlmac_core_clear_fifo_status;


/**************************************************************************************************/
/* e2efc_dual_modid_en:  - When set, dual modid is enabled for E2EFC (Only 32 ports IBP is sent o */
/*                      ut). When reset, single modid is enabled for E2EFC (64 ports IBP is sent) */
/* e2ecc_legacy_imp_en:  - When set, legacy E2ECC stage2 loading enabled (single stage2 buffer fo */
/*                      r all ports). When reset, new E2ECC stage2 loading enabled (per port stag */
/*                      e2 buffer)                                                                */
/* e2ecc_dual_modid_en:  - When set, dual modid is enabled for E2ECC. When reset, single modid is */
/*                       enabled for E2ECC                                                        */
/* honor_pause_for_e2e:  - When set, E2ECC/FC frames are not transmitted during pause state. When */
/*                       reset, E2ECC/FC frames are transmitted even during pause state similar t */
/*                      o other flow control frames.                                              */
/* e2e_enable:  - When set, MAC enables E2EFC/E2ECC frame generation and transmission.            */
/**************************************************************************************************/
typedef struct
{
    uint8_t e2efc_dual_modid_en;
    uint8_t e2ecc_legacy_imp_en;
    uint8_t e2ecc_dual_modid_en;
    uint8_t honor_pause_for_e2e;
    uint8_t e2e_enable;
} xport_xlmac_core_e2e_ctrl;


/**************************************************************************************************/
/* sum_ts_entry_valid:  - Active high qualifier for the TimeStamp & SEQUENCE_ID fields.           */
/* sum_link_interruption_status:  - True when link interruption state is detected as per RS layer */
/*                                state machine. Sticky bit is cleared by CLEAR_LINK_INTERRUPTION */
/*                               _STATUS.                                                         */
/* sum_remote_fault_status:  - True when remote fault state is detected as per RS layer state mac */
/*                          hine. Sticky bit is cleared by CLEAR_REMOTE_FAULT_STATUS.             */
/* sum_local_fault_status:  - True when local fault state is detected as per RS layer state machi */
/*                         ne. Sticky bit is cleared by CLEAR_LOCAL_FAULT_STATUS                  */
/* sum_rx_cdc_double_bit_err:  - This status bit indicates a double bit error occurred in the Rx  */
/*                            CDC memory                                                          */
/* sum_rx_cdc_single_bit_err:  - This status bit indicates a single bit error occurred in the Rx  */
/*                            CDC memory                                                          */
/* sum_tx_cdc_double_bit_err:  - This status bit indicates a double bit error occurred in the Tx  */
/*                            CDC memory                                                          */
/* sum_tx_cdc_single_bit_err:  - This status bit indicates a single bit error occurred in the Tx  */
/*                            CDC memory                                                          */
/* sum_rx_msg_overflow:  - If set, indicates rx message fifo overflow                             */
/* sum_rx_pkt_overflow:  - If set, indicates RX packet fifo overflow                              */
/* sum_tx_ts_fifo_overflow:  - If set, indicates overflow occurred in TX two-step Time Stamp FIFO */
/* sum_tx_llfc_msg_overflow:  - If set, indicates TX LLFC message fifo overflow                   */
/* sum_tx_pkt_overflow:  - If set, indicates tx packet fifo overflow                              */
/* sum_tx_pkt_underflow:  - If set, indicates tx packet fifo underflow                            */
/**************************************************************************************************/
typedef struct
{
    uint8_t sum_ts_entry_valid;
    uint8_t sum_link_interruption_status;
    uint8_t sum_remote_fault_status;
    uint8_t sum_local_fault_status;
    uint8_t sum_rx_cdc_double_bit_err;
    uint8_t sum_rx_cdc_single_bit_err;
    uint8_t sum_tx_cdc_double_bit_err;
    uint8_t sum_tx_cdc_single_bit_err;
    uint8_t sum_rx_msg_overflow;
    uint8_t sum_rx_pkt_overflow;
    uint8_t sum_tx_ts_fifo_overflow;
    uint8_t sum_tx_llfc_msg_overflow;
    uint8_t sum_tx_pkt_overflow;
    uint8_t sum_tx_pkt_underflow;
} xport_xlmac_core_intr_status;


/**************************************************************************************************/
/* en_ts_entry_valid:  - If set, SUM_TS_ENTRY_VALID can set mac interrupt.                        */
/* en_link_interruption_status:  - If set, SUM_LINK_INTERRUPTION_STATUS can set mac interrupt.    */
/* en_remote_fault_status:  - If set, SUM_REMOTE_FAULT_STATUS can set mac interrupt.              */
/* en_local_fault_status:  - If set, SUM_LOCAL_FAULT_STATUS can set mac interrupt.                */
/* en_rx_cdc_double_bit_err:  - If set, SUM_RX_CDC_DOUBLE_BIT_ERR can set mac interrupt.          */
/* en_rx_cdc_single_bit_err:  - If set, SUM_RX_CDC_SINGLE_BIT_ERR can set mac interrupt.          */
/* en_tx_cdc_double_bit_err:  - If set, SUM_TX_CDC_DOUBLE_BIT_ERR can set mac interrupt.          */
/* en_tx_cdc_single_bit_err:  - If set, SUM_TX_CDC_SINGLE_BIT_ERR can set mac interrupt.          */
/* en_rx_msg_overflow:  - If set, SUM_RX_MSG_OVERFLOW can set mac interrupt.                      */
/* en_rx_pkt_overflow:  - If set, SUM_RX_PKT_OVERFLOW can set mac interrupt.                      */
/* en_tx_ts_fifo_overflow:  - If set, SUM_TX_TS_FIFO_OVERFLOW can set mac interrupt.              */
/* en_tx_llfc_msg_overflow:  - If set, SUM_TX_LLFC_MSG_OVERFLOW can set mac interrupt.            */
/* en_tx_pkt_overflow:  - If set, SUM_TX_PKT_OVERFLOW can set mac interrupt.                      */
/* en_tx_pkt_underflow:  - If set, SUM_TX_PKT_UNDERFLOW can set mac interrupt.                    */
/**************************************************************************************************/
typedef struct
{
    uint8_t en_ts_entry_valid;
    uint8_t en_link_interruption_status;
    uint8_t en_remote_fault_status;
    uint8_t en_local_fault_status;
    uint8_t en_rx_cdc_double_bit_err;
    uint8_t en_rx_cdc_single_bit_err;
    uint8_t en_tx_cdc_double_bit_err;
    uint8_t en_tx_cdc_single_bit_err;
    uint8_t en_rx_msg_overflow;
    uint8_t en_rx_pkt_overflow;
    uint8_t en_tx_ts_fifo_overflow;
    uint8_t en_tx_llfc_msg_overflow;
    uint8_t en_tx_pkt_overflow;
    uint8_t en_tx_pkt_underflow;
} xport_xlmac_core_intr_enable;

int ag_drv_xport_xlmac_core_ctrl_set(uint8_t port_id, const xport_xlmac_core_ctrl *ctrl);
int ag_drv_xport_xlmac_core_ctrl_get(uint8_t port_id, xport_xlmac_core_ctrl *ctrl);
int ag_drv_xport_xlmac_core_mode_set(uint8_t port_id, uint8_t speed_mode, uint8_t no_sop_for_crc_hg, uint8_t hdr_mode);
int ag_drv_xport_xlmac_core_mode_get(uint8_t port_id, uint8_t *speed_mode, uint8_t *no_sop_for_crc_hg, uint8_t *hdr_mode);
int ag_drv_xport_xlmac_core_spare0_set(uint8_t port_id, uint32_t rsvd);
int ag_drv_xport_xlmac_core_spare0_get(uint8_t port_id, uint32_t *rsvd);
int ag_drv_xport_xlmac_core_spare1_set(uint8_t port_id, uint8_t rsvd);
int ag_drv_xport_xlmac_core_spare1_get(uint8_t port_id, uint8_t *rsvd);
int ag_drv_xport_xlmac_core_tx_ctrl_set(uint8_t port_id, const xport_xlmac_core_tx_ctrl *tx_ctrl);
int ag_drv_xport_xlmac_core_tx_ctrl_get(uint8_t port_id, xport_xlmac_core_tx_ctrl *tx_ctrl);
int ag_drv_xport_xlmac_core_tx_mac_sa_set(uint8_t port_id, uint64_t ctrl_sa);
int ag_drv_xport_xlmac_core_tx_mac_sa_get(uint8_t port_id, uint64_t *ctrl_sa);
int ag_drv_xport_xlmac_core_rx_ctrl_set(uint8_t port_id, const xport_xlmac_core_rx_ctrl *rx_ctrl);
int ag_drv_xport_xlmac_core_rx_ctrl_get(uint8_t port_id, xport_xlmac_core_rx_ctrl *rx_ctrl);
int ag_drv_xport_xlmac_core_rx_mac_sa_set(uint8_t port_id, uint64_t rx_sa);
int ag_drv_xport_xlmac_core_rx_mac_sa_get(uint8_t port_id, uint64_t *rx_sa);
int ag_drv_xport_xlmac_core_rx_max_size_set(uint8_t port_id, uint16_t rx_max_size);
int ag_drv_xport_xlmac_core_rx_max_size_get(uint8_t port_id, uint16_t *rx_max_size);
int ag_drv_xport_xlmac_core_rx_vlan_tag_set(uint8_t port_id, uint8_t outer_vlan_tag_enable, uint8_t inner_vlan_tag_enable, uint16_t outer_vlan_tag, uint16_t inner_vlan_tag);
int ag_drv_xport_xlmac_core_rx_vlan_tag_get(uint8_t port_id, uint8_t *outer_vlan_tag_enable, uint8_t *inner_vlan_tag_enable, uint16_t *outer_vlan_tag, uint16_t *inner_vlan_tag);
int ag_drv_xport_xlmac_core_rx_lss_ctrl_set(uint8_t port_id, const xport_xlmac_core_rx_lss_ctrl *rx_lss_ctrl);
int ag_drv_xport_xlmac_core_rx_lss_ctrl_get(uint8_t port_id, xport_xlmac_core_rx_lss_ctrl *rx_lss_ctrl);
int ag_drv_xport_xlmac_core_rx_lss_status_get(uint8_t port_id, uint8_t *link_interruption_status, uint8_t *remote_fault_status, uint8_t *local_fault_status);
int ag_drv_xport_xlmac_core_clear_rx_lss_status_set(uint8_t port_id, uint8_t clear_link_interruption_status, uint8_t clear_remote_fault_status, uint8_t clear_local_fault_status);
int ag_drv_xport_xlmac_core_clear_rx_lss_status_get(uint8_t port_id, uint8_t *clear_link_interruption_status, uint8_t *clear_remote_fault_status, uint8_t *clear_local_fault_status);
int ag_drv_xport_xlmac_core_pause_ctrl_set(uint8_t port_id, const xport_xlmac_core_pause_ctrl *pause_ctrl);
int ag_drv_xport_xlmac_core_pause_ctrl_get(uint8_t port_id, xport_xlmac_core_pause_ctrl *pause_ctrl);
int ag_drv_xport_xlmac_core_pfc_ctrl_set(uint8_t port_id, const xport_xlmac_core_pfc_ctrl *pfc_ctrl);
int ag_drv_xport_xlmac_core_pfc_ctrl_get(uint8_t port_id, xport_xlmac_core_pfc_ctrl *pfc_ctrl);
int ag_drv_xport_xlmac_core_pfc_type_set(uint8_t port_id, uint16_t pfc_eth_type);
int ag_drv_xport_xlmac_core_pfc_type_get(uint8_t port_id, uint16_t *pfc_eth_type);
int ag_drv_xport_xlmac_core_pfc_opcode_set(uint8_t port_id, uint16_t pfc_opcode);
int ag_drv_xport_xlmac_core_pfc_opcode_get(uint8_t port_id, uint16_t *pfc_opcode);
int ag_drv_xport_xlmac_core_pfc_da_set(uint8_t port_id, uint64_t pfc_macda);
int ag_drv_xport_xlmac_core_pfc_da_get(uint8_t port_id, uint64_t *pfc_macda);
int ag_drv_xport_xlmac_core_llfc_ctrl_set(uint8_t port_id, const xport_xlmac_core_llfc_ctrl *llfc_ctrl);
int ag_drv_xport_xlmac_core_llfc_ctrl_get(uint8_t port_id, xport_xlmac_core_llfc_ctrl *llfc_ctrl);
int ag_drv_xport_xlmac_core_tx_llfc_msg_fields_set(uint8_t port_id, uint16_t llfc_xoff_time, uint8_t tx_llfc_fc_obj_logical, uint8_t tx_llfc_msg_type_logical);
int ag_drv_xport_xlmac_core_tx_llfc_msg_fields_get(uint8_t port_id, uint16_t *llfc_xoff_time, uint8_t *tx_llfc_fc_obj_logical, uint8_t *tx_llfc_msg_type_logical);
int ag_drv_xport_xlmac_core_rx_llfc_msg_fields_set(uint8_t port_id, uint8_t rx_llfc_fc_obj_physical, uint8_t rx_llfc_msg_type_physical, uint8_t rx_llfc_fc_obj_logical, uint8_t rx_llfc_msg_type_logical);
int ag_drv_xport_xlmac_core_rx_llfc_msg_fields_get(uint8_t port_id, uint8_t *rx_llfc_fc_obj_physical, uint8_t *rx_llfc_msg_type_physical, uint8_t *rx_llfc_fc_obj_logical, uint8_t *rx_llfc_msg_type_logical);
int ag_drv_xport_xlmac_core_tx_timestamp_fifo_data_get(uint8_t port_id, uint8_t *ts_entry_valid, uint16_t *sequence_id, uint32_t *time_stamp);
int ag_drv_xport_xlmac_core_tx_timestamp_fifo_status_get(uint8_t port_id, uint8_t *entry_count);
int ag_drv_xport_xlmac_core_fifo_status_get(uint8_t port_id, xport_xlmac_core_fifo_status *fifo_status);
int ag_drv_xport_xlmac_core_clear_fifo_status_set(uint8_t port_id, const xport_xlmac_core_clear_fifo_status *clear_fifo_status);
int ag_drv_xport_xlmac_core_clear_fifo_status_get(uint8_t port_id, xport_xlmac_core_clear_fifo_status *clear_fifo_status);
int ag_drv_xport_xlmac_core_lag_failover_status_get(uint8_t port_id, uint8_t *rsvd, uint8_t *lag_failover_loopback);
int ag_drv_xport_xlmac_core_eee_ctrl_set(uint8_t port_id, uint8_t rsvd, uint8_t eee_en);
int ag_drv_xport_xlmac_core_eee_ctrl_get(uint8_t port_id, uint8_t *rsvd, uint8_t *eee_en);
int ag_drv_xport_xlmac_core_eee_timers_set(uint8_t port_id, uint16_t eee_ref_count, uint16_t eee_wake_timer, uint32_t eee_delay_entry_timer);
int ag_drv_xport_xlmac_core_eee_timers_get(uint8_t port_id, uint16_t *eee_ref_count, uint16_t *eee_wake_timer, uint32_t *eee_delay_entry_timer);
int ag_drv_xport_xlmac_core_eee_1_sec_link_status_timer_set(uint8_t port_id, uint32_t one_second_timer);
int ag_drv_xport_xlmac_core_eee_1_sec_link_status_timer_get(uint8_t port_id, uint32_t *one_second_timer);
int ag_drv_xport_xlmac_core_higig_hdr_0_set(uint8_t port_id, uint64_t higig_hdr_0);
int ag_drv_xport_xlmac_core_higig_hdr_0_get(uint8_t port_id, uint64_t *higig_hdr_0);
int ag_drv_xport_xlmac_core_higig_hdr_1_set(uint8_t port_id, uint64_t higig_hdr_1);
int ag_drv_xport_xlmac_core_higig_hdr_1_get(uint8_t port_id, uint64_t *higig_hdr_1);
int ag_drv_xport_xlmac_core_gmii_eee_ctrl_set(uint8_t port_id, uint8_t gmii_lpi_predict_mode_en, uint16_t gmii_lpi_predict_threshold);
int ag_drv_xport_xlmac_core_gmii_eee_ctrl_get(uint8_t port_id, uint8_t *gmii_lpi_predict_mode_en, uint16_t *gmii_lpi_predict_threshold);
int ag_drv_xport_xlmac_core_timestamp_adjust_set(uint8_t port_id, uint8_t ts_use_cs_offset, uint8_t ts_tsts_adjust, uint16_t ts_osts_adjust);
int ag_drv_xport_xlmac_core_timestamp_adjust_get(uint8_t port_id, uint8_t *ts_use_cs_offset, uint8_t *ts_tsts_adjust, uint16_t *ts_osts_adjust);
int ag_drv_xport_xlmac_core_timestamp_byte_adjust_set(uint8_t port_id, uint8_t rx_timer_byte_adjust_en, uint16_t rx_timer_byte_adjust, uint8_t tx_timer_byte_adjust_en, uint16_t tx_timer_byte_adjust);
int ag_drv_xport_xlmac_core_timestamp_byte_adjust_get(uint8_t port_id, uint8_t *rx_timer_byte_adjust_en, uint16_t *rx_timer_byte_adjust, uint8_t *tx_timer_byte_adjust_en, uint16_t *tx_timer_byte_adjust);
int ag_drv_xport_xlmac_core_tx_crc_corrupt_ctrl_set(uint8_t port_id, uint32_t prog_tx_crc, uint8_t tx_crc_corruption_mode, uint8_t tx_crc_corrupt_en, uint8_t tx_err_corrupts_crc);
int ag_drv_xport_xlmac_core_tx_crc_corrupt_ctrl_get(uint8_t port_id, uint32_t *prog_tx_crc, uint8_t *tx_crc_corruption_mode, uint8_t *tx_crc_corrupt_en, uint8_t *tx_err_corrupts_crc);
int ag_drv_xport_xlmac_core_e2e_ctrl_set(uint8_t port_id, const xport_xlmac_core_e2e_ctrl *e2e_ctrl);
int ag_drv_xport_xlmac_core_e2e_ctrl_get(uint8_t port_id, xport_xlmac_core_e2e_ctrl *e2e_ctrl);
int ag_drv_xport_xlmac_core_e2ecc_module_hdr_0_set(uint8_t port_id, uint64_t e2ecc_module_hdr_0);
int ag_drv_xport_xlmac_core_e2ecc_module_hdr_0_get(uint8_t port_id, uint64_t *e2ecc_module_hdr_0);
int ag_drv_xport_xlmac_core_e2ecc_module_hdr_1_set(uint8_t port_id, uint64_t e2ecc_module_hdr_1);
int ag_drv_xport_xlmac_core_e2ecc_module_hdr_1_get(uint8_t port_id, uint64_t *e2ecc_module_hdr_1);
int ag_drv_xport_xlmac_core_e2ecc_data_hdr_0_set(uint8_t port_id, uint64_t e2ecc_data_hdr_0);
int ag_drv_xport_xlmac_core_e2ecc_data_hdr_0_get(uint8_t port_id, uint64_t *e2ecc_data_hdr_0);
int ag_drv_xport_xlmac_core_e2ecc_data_hdr_1_set(uint8_t port_id, uint64_t e2ecc_data_hdr_1);
int ag_drv_xport_xlmac_core_e2ecc_data_hdr_1_get(uint8_t port_id, uint64_t *e2ecc_data_hdr_1);
int ag_drv_xport_xlmac_core_e2efc_module_hdr_0_set(uint8_t port_id, uint64_t e2efc_module_hdr_0);
int ag_drv_xport_xlmac_core_e2efc_module_hdr_0_get(uint8_t port_id, uint64_t *e2efc_module_hdr_0);
int ag_drv_xport_xlmac_core_e2efc_module_hdr_1_set(uint8_t port_id, uint64_t e2efc_module_hdr_1);
int ag_drv_xport_xlmac_core_e2efc_module_hdr_1_get(uint8_t port_id, uint64_t *e2efc_module_hdr_1);
int ag_drv_xport_xlmac_core_e2efc_data_hdr_0_set(uint8_t port_id, uint64_t e2efc_data_hdr_0);
int ag_drv_xport_xlmac_core_e2efc_data_hdr_0_get(uint8_t port_id, uint64_t *e2efc_data_hdr_0);
int ag_drv_xport_xlmac_core_e2efc_data_hdr_1_set(uint8_t port_id, uint64_t e2efc_data_hdr_1);
int ag_drv_xport_xlmac_core_e2efc_data_hdr_1_get(uint8_t port_id, uint64_t *e2efc_data_hdr_1);
int ag_drv_xport_xlmac_core_txfifo_cell_cnt_get(uint8_t port_id, uint8_t *cell_cnt);
int ag_drv_xport_xlmac_core_txfifo_cell_req_cnt_get(uint8_t port_id, uint8_t *req_cnt);
int ag_drv_xport_xlmac_core_mem_ctrl_set(uint8_t port_id, uint16_t tx_cdc_mem_ctrl_tm, uint16_t rx_cdc_mem_ctrl_tm);
int ag_drv_xport_xlmac_core_mem_ctrl_get(uint8_t port_id, uint16_t *tx_cdc_mem_ctrl_tm, uint16_t *rx_cdc_mem_ctrl_tm);
int ag_drv_xport_xlmac_core_ecc_ctrl_set(uint8_t port_id, uint8_t tx_cdc_ecc_ctrl_en, uint8_t rx_cdc_ecc_ctrl_en);
int ag_drv_xport_xlmac_core_ecc_ctrl_get(uint8_t port_id, uint8_t *tx_cdc_ecc_ctrl_en, uint8_t *rx_cdc_ecc_ctrl_en);
int ag_drv_xport_xlmac_core_ecc_force_double_bit_err_set(uint8_t port_id, uint8_t tx_cdc_force_double_bit_err, uint8_t rx_cdc_force_double_bit_err);
int ag_drv_xport_xlmac_core_ecc_force_double_bit_err_get(uint8_t port_id, uint8_t *tx_cdc_force_double_bit_err, uint8_t *rx_cdc_force_double_bit_err);
int ag_drv_xport_xlmac_core_ecc_force_single_bit_err_set(uint8_t port_id, uint8_t tx_cdc_force_single_bit_err, uint8_t rx_cdc_force_single_bit_err);
int ag_drv_xport_xlmac_core_ecc_force_single_bit_err_get(uint8_t port_id, uint8_t *tx_cdc_force_single_bit_err, uint8_t *rx_cdc_force_single_bit_err);
int ag_drv_xport_xlmac_core_rx_cdc_ecc_status_get(uint8_t port_id, uint8_t *rx_cdc_double_bit_err, uint8_t *rx_cdc_single_bit_err);
int ag_drv_xport_xlmac_core_tx_cdc_ecc_status_get(uint8_t port_id, uint8_t *tx_cdc_double_bit_err, uint8_t *tx_cdc_single_bit_err);
int ag_drv_xport_xlmac_core_clear_ecc_status_set(uint8_t port_id, uint8_t clear_tx_cdc_double_bit_err, uint8_t clear_tx_cdc_single_bit_err, uint8_t clear_rx_cdc_double_bit_err, uint8_t clear_rx_cdc_single_bit_err);
int ag_drv_xport_xlmac_core_clear_ecc_status_get(uint8_t port_id, uint8_t *clear_tx_cdc_double_bit_err, uint8_t *clear_tx_cdc_single_bit_err, uint8_t *clear_rx_cdc_double_bit_err, uint8_t *clear_rx_cdc_single_bit_err);
int ag_drv_xport_xlmac_core_intr_status_get(uint8_t port_id, xport_xlmac_core_intr_status *intr_status);
int ag_drv_xport_xlmac_core_intr_enable_set(uint8_t port_id, const xport_xlmac_core_intr_enable *intr_enable);
int ag_drv_xport_xlmac_core_intr_enable_get(uint8_t port_id, xport_xlmac_core_intr_enable *intr_enable);
int ag_drv_xport_xlmac_core_version_id_get(uint8_t port_id, uint16_t *xlmac_version);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xport_xlmac_core_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

