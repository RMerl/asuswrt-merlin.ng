/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
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
#ifndef __RDP_UNIMAC_H_
#define __RDP_UNIMAC_H_

#include "rdp_map.h"

/********************** UNIMAC Block *************************/
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_HD_BKP_CNTL (UNIMAC_CONF_RDP + 0x4) /* UniMAC Half Duplex Backpressure Control Register */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD         (UNIMAC_CONF_RDP + 0x8) /* UniMAC Command Register */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_MAC0        (UNIMAC_CONF_RDP + 0xc) /*Unimac MAC address first 4 bytes*/
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_MAC1        (UNIMAC_CONF_RDP + 0x10) /*Unimac MAC address 2 last bytes*/
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_FRM_LEN     (UNIMAC_CONF_RDP + 0x14) /* UniMAC Frame Length */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_QUNAT (UNIMAC_CONF_RDP + 0x18) /* UniMAC Pause Quanta */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_MODE        (UNIMAC_CONF_RDP + 0x44) /* UniMAC Mode */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_TX_IPG_LEN  (UNIMAC_CONF_RDP + 0x5c) /* UniMAC Inter Packet Gap */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_EEE_CTRL    (UNIMAC_CONF_RDP + 0x64) /* UniMAC Energy Efficient Ethernet Control */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_EEE_REF_COUNT   (UNIMAC_CONF_RDP + 0x70) /* UniMAC Energy Efficient Ethernet Ref Clock Speed */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_EEE_REF_COUNT_VAL 350               /* Ref clock is half speed of Runner clock */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_CNTRL (UNIMAC_CONF_RDP + 0x30) /* UniMAC Repetitive Pause Control in TX direction */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_RX_MAX_PKT_SIZE (UNIMAC_CONF_RDP + 0x608) /* UniMAC RX MAX packet Size Register */

#define UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG      (UNIMAC_MISC_RDP + 0x0) /* configuration register */
#define UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1 (UNIMAC_MISC_RDP + 0x4)
#define UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2 (UNIMAC_MISC_RDP + 0x8)

#define UNIMAC_CONF_EMAC_OFFSET                     0x1000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_HD_BKP_CNTL_ipg_config_rx_MASK 0x0000007c
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_HD_BKP_CNTL_ipg_config_rx_SHIFT 2
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_HD_BKP_CNTL_hd_fc_ena_MASK 0x00000001
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_HD_BKP_CNTL_hd_fc_ena_SHIFT 0
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_txrx_en_config_MASK    0x20000000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_txrx_en_config_SHIFT   29
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_tx_pause_ignore_MASK   0x10000000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_tx_pause_ignore_SHIFT  28
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_rmt_loop_ena_MASK      0x02000000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_rmt_loop_ena_SHIFT     25
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_cntl_frm_ena_MASK      0x00800000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_cntl_frm_ena_SHIFT     23
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_lcl_loop_ena_MASK      0x00008000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_lcl_loop_ena_SHIFT     15
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_sw_reset_MASK          0x00002000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_sw_reset_SHIFT         13
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_hd_ena_MASK            0x00000400
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_hd_ena_SHIFT           10
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_rx_pause_ignore_MASK   0x00000100
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_rx_pause_ignore_SHIFT  8
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_pause_fwd_MASK         0x00000080
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_pause_fwd_SHIFT        7
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_crc_fwd_MASK           0x00000040
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_crc_fwd_SHIFT          6
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_pad_en_MASK            0x00000020
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_pad_en_SHIFT           5
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_eth_speed_MASK         0x0000000c
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_eth_speed_SHIFT        2
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_rx_ena_MASK            0x00000002
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_rx_ena_SHIFT           1
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_tx_ena_MASK            0x00000001
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_tx_ena_SHIFT           0
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_FRM_LEN_frame_length_MASK  0x00003fff
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_FRM_LEN_frame_length_SHIFT 0
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_TX_IPG_LEN_tx_ipg_len_MASK 0x0000007f
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_TX_IPG_LEN_tx_ipg_len_SHIFT 0
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_TX_IPG_LEN_tx_min_pkt_size_MASK  0x00007F00
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_TX_IPG_LEN_tx_min_pkt_size_SHIFT 8
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_RX_MAX_PKT_SIZE_max_pkt_size_MASK 0x00003fff
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_RX_MAX_PKT_SIZE_max_pkt_size_SHIFT 0
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_CNTRL_pause_control_en_MASK 0x00020000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_CNTRL_pause_control_en_SHIFT 17
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_CNTRL_pause_timer_MASK 0x0001ffff
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_CNTRL_pause_timer_SHIFT 0
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_QUNAT_pause_quant_MASK 0x0000ffff
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_QUNAT_pause_quant_SHIFT 0

#define UNIMAC_MISC_EMAC_OFFSET 0x400
#define UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT 1 << 16


#define UNIMAC_MIB_EMAC_OFFSET 0x400
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_MIB_CNTRL (UNIMAC_MIB_RDP + 0x180) /* MIB Control Register */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR64 (UNIMAC_MIB_RDP + 0x0) /* Receive 64B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR127 (UNIMAC_MIB_RDP + 0x4) /* Receive 65B to 127B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR255 (UNIMAC_MIB_RDP + 0x8) /* Receive 128B to 255B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR511 (UNIMAC_MIB_RDP + 0xc) /* Receive 256B to 511B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR1023 (UNIMAC_MIB_RDP + 0x10) /* Receive 512B to 1023B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR1518 (UNIMAC_MIB_RDP + 0x14) /* Receive 1024B to 1518B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRMGV (UNIMAC_MIB_RDP + 0x18) /* Receive 1519B to 1522B Good VLAN Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR2047 (UNIMAC_MIB_RDP + 0x1c) /* Receive 1519B to 2047B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR4095 (UNIMAC_MIB_RDP + 0x20) /* Receive 2048B to 4095B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR9216 (UNIMAC_MIB_RDP + 0x24) /* Receive 4096B to 9216B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRPKT (UNIMAC_MIB_RDP + 0x28) /* Receive Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRBYT (UNIMAC_MIB_RDP + 0x2c) /* Receive Byte Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRMCA (UNIMAC_MIB_RDP + 0x30) /* Receive Multicast Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRBCA (UNIMAC_MIB_RDP + 0x34) /* Receive Broadcast Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRFCS (UNIMAC_MIB_RDP + 0x38) /* Receive FCS Error Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRXCF (UNIMAC_MIB_RDP + 0x3c) /* Receive Control Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRXPF (UNIMAC_MIB_RDP + 0x40) /* Receive Pause Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRXUO (UNIMAC_MIB_RDP + 0x44) /* Receive Unknown OP Code Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRALN (UNIMAC_MIB_RDP + 0x48) /* Receive Alignmenet Error Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRFLR (UNIMAC_MIB_RDP + 0x4c) /* Receive Frame Length Out Of Range Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRCDE (UNIMAC_MIB_RDP + 0x50) /* Receive Code Error Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRFCR (UNIMAC_MIB_RDP + 0x54) /* Receive Carrier Sense Error Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GROVR (UNIMAC_MIB_RDP + 0x58) /* Receive Oversize Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRJBR (UNIMAC_MIB_RDP + 0x5c) /* Receive Jabber Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRMTUE (UNIMAC_MIB_RDP + 0x60) /* Receive MTU Error Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRPOK (UNIMAC_MIB_RDP + 0x64) /* Receive Good Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRUC (UNIMAC_MIB_RDP + 0x68) /* Receive Unicast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRPPP (UNIMAC_MIB_RDP + 0x6c) /* Receive PPP Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRCRC (UNIMAC_MIB_RDP + 0x70) /* Receive CRC Match Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRPKT (UNIMAC_MIB_RDP + 0x100) /* Receive RUNT Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRUND (UNIMAC_MIB_RDP + 0x104) /* Receive RUNT Packet And Contain A Valid FCS */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRFRG (UNIMAC_MIB_RDP + 0x108) /* Receive RUNT Packet And Contain Invalid FCS or Alignment Error */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRBYT (UNIMAC_MIB_RDP + 0x10c) /* Receive RUNT Packet Byte Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR64 (UNIMAC_MIB_RDP + 0x80) /* Transmit 64B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR127 (UNIMAC_MIB_RDP + 0x84) /* Transmit 65B to 127B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR255 (UNIMAC_MIB_RDP + 0x88) /* Transmit 128B to 255B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR511 (UNIMAC_MIB_RDP + 0x8c) /* Transmit 256B to 511B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR1023 (UNIMAC_MIB_RDP + 0x90) /* Transmit 512B to 1023B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR1518 (UNIMAC_MIB_RDP + 0x94) /* Transmit 1024B to 1518B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TRMGV (UNIMAC_MIB_RDP + 0x98) /* Transmit 1519B to 1522B Good VLAN Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR2047 (UNIMAC_MIB_RDP + 0x9c) /* Transmit 1519B to 2047B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR4095 (UNIMAC_MIB_RDP + 0xa0) /* Transmit 2048B to 4095B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR9216 (UNIMAC_MIB_RDP + 0xa4) /* Transmit 4096B to 9216B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTPKT (UNIMAC_MIB_RDP + 0xa8) /* Transmit Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTMCA (UNIMAC_MIB_RDP + 0xac) /* Transmit Multicast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTBCA (UNIMAC_MIB_RDP + 0xb0) /* Transmit Broadcast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTXPF (UNIMAC_MIB_RDP + 0xb4) /* Transmit Pause Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTXCF (UNIMAC_MIB_RDP + 0xb8) /* Transmit Control Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTFCS (UNIMAC_MIB_RDP + 0xbc) /* Transmit FCS Error Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTOVR (UNIMAC_MIB_RDP + 0xc0) /* Transmit Oversize Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTDRF (UNIMAC_MIB_RDP + 0xc4) /* Transmit Deferral Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTEDF (UNIMAC_MIB_RDP + 0xc8) /* Transmit Excessive Deferral Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTSCL (UNIMAC_MIB_RDP + 0xcc) /* Transmit Single Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTMCL (UNIMAC_MIB_RDP + 0xd0) /* Transmit Multiple Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTLCL (UNIMAC_MIB_RDP + 0xd4) /* Transmit Late Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTXCL (UNIMAC_MIB_RDP + 0xd8) /* Transmit Excessive Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTFRG (UNIMAC_MIB_RDP + 0xdc) /* Transmit Fragments Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTNCL (UNIMAC_MIB_RDP + 0xe0) /* Transmit Total Collision Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTJBR (UNIMAC_MIB_RDP + 0xe4) /* Transmit Jabber Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTBYT (UNIMAC_MIB_RDP + 0xe8) /* Transmit Byte Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTPOK (UNIMAC_MIB_RDP + 0xec) /* Transmit Good Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTUC (UNIMAC_MIB_RDP + 0xf0) /* Transmit Unicast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_MIB_CNTRL_rx_cnt_st_MASK 0x00000001
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_MIB_CNTRL_rx_cnt_st_SHIFT 0
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_MIB_CNTRL_tx_cnt_rst_MASK 0x00000004
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_MIB_CNTRL_tx_cnt_rst_SHIFT 2

#endif /* __RDP_UNIMAC_H_ */