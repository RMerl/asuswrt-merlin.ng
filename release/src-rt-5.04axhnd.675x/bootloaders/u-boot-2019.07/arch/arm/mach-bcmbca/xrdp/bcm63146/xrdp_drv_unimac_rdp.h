// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    
*/

#ifndef _XRDP_DRV_UNIMAC_RDP_H_
#define _XRDP_DRV_UNIMAC_RDP_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#include "rdp_common.h"


/**************************************************************************************************/
/* runt_filter_dis:  - When set, disable runt filtering.                                          */
/* oob_efc_disab:  - When this bit is set, out-of-band egress flow control will be disabled. When */
/*                 this bit is 0 (out-of-band egress flow control enabled) and input pin ext_tx_f */
/*                low_control is 1, frame transmissions may be stopped - see OOB_EFC_MODE for det */
/*                ails.
Out-of-band egress flow control operation is similar to halting the trans */
/*                mit datapath due to reception of a Pause Frame with a non-zero timer value. Thi */
/*                s bit however has no effect on regular Rx Pause Frame based egress flow control */
/*                .                                                                               */
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
/* bypass_oob_efc_synchronizer:  - 1=> bypass the OOB external flow control signal synchronizer,  */
/*                              to e.g. reduce latency. In this case it is assumed/required that  */
/*                              Unimac input ext_tx_flow_control is already in tx_clk clock domai */
/*                              n (so there is no need to synchronize it)
0=> locally synchronize */
/*                               the OOB egress flow control signal to tx_clk. In this case it is */
/*                               assumed/required that ext_tx_flow_control is glitchless (e.g. re */
/*                              gistered in its native clock domain).                             */
/* oob_efc_mode:  - 0=> strict/full OOB egress backpressure mode:
- pause frames and PFC frames,  */
/*               as well as regular packets, are all affected by Unimac input ext_tx_flow_control */
/*               , as long as OOB_EFC_DISAB is 0
- in this mode, OOB backpressure will be active  */
/*               as long as ext_tx_flow_control is asserted and i_oob_efc_disab is 0, regardless  */
/*               of whether the MAC operates in half duplex mode or full duplex mode
1=> legacy m */
/*               ode:
- ext_tx_flow_control does not affect (does not prevent) transmission of Pa */
/*               use and PFC frames, i.e. in this mode OOB egress backpressure may only affect tr */
/*               ansmission of regular packets
- OOB egress backpressure is fully disabled (ignor */
/*               ed) when the MAC operates in half duplex mode.                                   */
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
    bdmf_boolean oob_efc_disab;
    bdmf_boolean ignore_tx_pause;
    bdmf_boolean fd_tx_urun_fix_en;
    bdmf_boolean line_loopback;
    bdmf_boolean no_lgth_check;
    bdmf_boolean cntl_frm_ena;
    bdmf_boolean ena_ext_config;
    bdmf_boolean en_internal_tx_crs;
    bdmf_boolean bypass_oob_efc_synchronizer;
    bdmf_boolean oob_efc_mode;
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


bdmf_error_t ag_drv_unimac_rdp_command_config_set(uint8_t umac_id,
		const unimac_rdp_command_config *command_config);
bdmf_error_t ag_drv_unimac_rdp_command_config_get(uint8_t umac_id,
		unimac_rdp_command_config *command_config);

#endif

