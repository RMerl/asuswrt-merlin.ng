/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

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
/*
 * xport_stats.h
 *
 */

#ifndef SHARED_OPENSOURCE_DRV_XPORT_XPORT_STATS_H_
#define SHARED_OPENSOURCE_DRV_XPORT_XPORT_STATS_H_

#include <linux/types.h>



typedef enum
{
    STAT_GRX64,   /*< Receive 64 Byte Frame Counter */
    STAT_GRX127,  /*< Receive 65 to 127 Byte Frame Counter*/
    STAT_GRX255,   /*< Receive 128 to 255 Byte Frame Counter*/
    STAT_GRX511,   /*< Receive 256 to 511 Byte Frame Counter*/
    STAT_GRX1023,  /*< Receive 512 to 1023 Byte Frame Counter*/
    STAT_GRX1518,  /*< Receive 1024 to 1518 Byte Frame Counter*/
    STAT_GRX1522,  /*< Receive 1519 to 1522 Byte Good VLAN Frame Counter*/
    STAT_GRX2047,  /*< Receive 1519 to 2047 Byte Frame Counter*/
    STAT_GRX4095,  /*< Receive 2048 to 4095 Byte Frame Counter*/
    STAT_GRX9216,  /*< Receive 4096 to 9216 Byte Frame Counter*/
    STAT_GRX16383, /*< Receive 9217 to 16838 Byte Frame Counter*/
    STAT_GRXPKT,   /*< Receive frame/packet Counter*/
    STAT_GRXUCA,   /*< Receive Unicast Frame Counter*/
    STAT_GRXMCA,   /*< Receive Multicast Frame Counter*/
    STAT_GRXBCA,   /*< Receive Broadcast Frame Counter*/
    STAT_GRXFCS,   /*< Receive FCS Error Frame Counter*/
    STAT_GRXCF,    /*< Receive Control Frame Counter*/
    STAT_GRXPF,    /*< Receive PAUSE Frame Counter*/
    STAT_GRXPP,    /*< Receive PFC (Per-Priority Pause) Frame Counter*/
    STAT_GRXUO,    /*< Receive Unsupported Opcode Frame Counter*/
    STAT_GRXUDA,   /*< Receive Unsupported DA for PAUSE/PFC Frame Counter*/
    STAT_GRXWSA,   /*< Receive Wrong SA Frame Counter*/
    STAT_GRXALN,   /*< Receive Alignment Error Frame Counter*/
    STAT_GRXFLR,   /*< Receive Length Out of Range Frame Counter*/
    STAT_GRXFRERR, /*< Receive Code Error Frame Counter*/
    STAT_GRXFCR,   /*< Receive False Carrier Counter*/
    STAT_GRXOVR,   /*< Receive Oversized Frame Counter*/
    STAT_GRXJBR,   /*< Receive Jabber Frame Counter*/
    STAT_GRXMTUE,  /*< Receive MTU Check Error Frame Counter*/
    STAT_GRXMCRC,  /*< Matched CRC Frame Counter*/
    STAT_GRXPRM,   /*< Receive Promiscuous Frame Counter*/
    STAT_GRXVLN,   /*< Receive VLAN Tag Frame Counter*/
    STAT_GRXDVLN,  /*< Receive Double VLAN Tag Frame Counter*/
    STAT_GRXTRFU,  /*< Receive Truncated Frame Counter (due to RX FIFO full)*/
    STAT_GRXPOK,   /*< Receive Good Packet Counter*/
    STAT_GRXSCHCRC,    /*< Receive SCH CRC Error*/
    STAT_GRXBYT,   /*< Receive Byte Counter*/
    STAT_GRXRPKT,  /*< Receive RUNT Frame Counter*/
    STAT_GRXUND,   /*< Receive Undersize Frame Counter*/
    STAT_GRXFRG,   /*< Receive Fragment Counter*/
    STAT_GRXRBYT,  /*< Receive Runt Byte Counter*/
    STAT_GRXLPI,   /*< RX EEE LPI Event Counter*/
    STAT_GRXDLPI,  /*< RX EEE LPI Duration Counte*/
    STAT_GRXPTLLFC,    /*< Receive Physical Type LLFC message counter*/
    STAT_GRXLTLLFC,   /*< Receive Logical Type LLFC message Counter*/
    STAT_GRXLLFCFCS   /*< Receive Type LLFC message with CRC error Counter*/
}XPORT_STATS_RX_TYPE;

typedef enum
{
    STAT_GTX64,    /*< Transmit 64 Byte Frame Counter*/
    STAT_GTX127,   /*< Transmit 65 to 127 Byte Frame Counter*/
    STAT_GTX255,   /*< Transmit 128 to 255 Byte Frame Counter*/
    STAT_GTX511,   /*< Transmit 256 to 511 Byte Frame Counter*/
    STAT_GTX1023,  /*< Transmit 512 to 1023 Byte Frame Counter*/
    STAT_GTX1518,  /*< Transmit 1024 to 1518 Byte Frame Counter*/
    STAT_GTX1522,  /*< Transmit 1519 to 1522 Byte Good VLAN Frame Counter*/
    STAT_GTX2047,  /*< Transmit 1519 to 2047 Byte Frame Counter*/
    STAT_GTX4095,  /*< Transmit 2048 to 4095 Byte Frame Counter*/
    STAT_GTX9216,  /*< Transmit 4096 to 9216 Byte Frame Counter*/
    STAT_GTX16383, /*< Transmit 9217 to 16383 Byte Frame Counter*/
    STAT_GTXPOK,   /*< Transmit Good Packet Counter*/
    STAT_GTXPKT,   /*< Transmit Packet/Frame Counter*/
    STAT_GTXUCA,   /*< Transmit Unicast Frame Counter*/
    STAT_GTXMCA,   /*< Transmit Multicast Frame Counter*/
    STAT_GTXBCA,   /*< Transmit Broadcast Frame Counter*/
    STAT_GTXPF,    /*< Transmit Pause Control Frame Counter*/
    STAT_GTXPFC,   /*< Transmit PFC/Per-Priority Pause Control Frame Counter*/
    STAT_GTXJBR,   /*< Transmit Jabber Counter*/
    STAT_GTXFCS,   /*< Transmit FCS Error Counter*/
    STAT_GTXCF,    /*< Transmit Control Frame Counter*/
    STAT_GTXOVR,   /*< Transmit Oversize Packet Counter*/
    STAT_GTXDFR,   /*< Transmit Single Deferral Frame Counter*/
    STAT_GTXEDF,   /*< Transmit Multiple Deferral Frame Counter*/
    STAT_GTXSCL,   /*< Transmit Single Collision Frame Counter*/
    STAT_GTXMCL,   /*< Transmit Multiple Collision Frame Counter*/
    STAT_GTXLCL,   /*< Transmit Late Collision Frame Counter*/
    STAT_GTXXCL,   /*< Transmit Excessive Collision Frame Counter*/
    STAT_GTXFRG,   /*< Transmit Fragment Counter*/
    STAT_GTXERR,   /*< Transmit Error (set by system) Counter*/
    STAT_GTXVLN,   /*< Transmit VLAN Tag Frame Counter*/
    STAT_GTXDVLN,  /*< Transmit Double VLAN Tag Frame Counter*/
    STAT_GTXRPKT,  /*< Transmit RUNT Frame Counter*/
    STAT_GTXUFL,   /*< Transmit FIFO Underrun Counter.*/
    STAT_GTXNCL,   /*< Transmit Total Collision Counter*/
    STAT_GTXBYT,   /*< Transmit Byte Counter*/
    STAT_GTXLPI,   /*< TX EEE LPI Event Counter*/
    STAT_GTXDLPI,  /*< TX EEE LPI Duration Counter*/
    STAT_GTXLTLLFC    /*< Transmit Logical Type LLFC message counter*/
}XPORT_STATS_TX_TYPE;

typedef struct
{
    uint64_t    GRX64;   /*< Receive 64 Byte Frame Counter */
    uint64_t    GRX127;  /*< Receive 65 to 127 Byte Frame Counter*/
    uint64_t    GRX255;   /*< Receive 128 to 255 Byte Frame Counter*/
    uint64_t    GRX511;   /*< Receive 256 to 511 Byte Frame Counter*/
    uint64_t    GRX1023;  /*< Receive 512 to 1023 Byte Frame Counter*/
    uint64_t    GRX1518;  /*< Receive 1024 to 1518 Byte Frame Counter*/
    uint64_t    GRX1522;  /*< Receive 1519 to 1522 Byte Good VLAN Frame Counter*/
    uint64_t    GRX2047;  /*< Receive 1519 to 2047 Byte Frame Counter*/
    uint64_t    GRX4095;  /*< Receive 2048 to 4095 Byte Frame Counter*/
    uint64_t    GRX9216;  /*< Receive 4096 to 9216 Byte Frame Counter*/
    uint64_t    GRX16383; /*< Receive 9217 to 16838 Byte Frame Counter*/
    uint64_t    GRXPKT;   /*< Receive frame/packet Counter*/
    uint64_t    GRXUCA;   /*< Receive Unicast Frame Counter*/
    uint64_t    GRXMCA;   /*< Receive Multicast Frame Counter*/
    uint64_t    GRXBCA;   /*< Receive Broadcast Frame Counter*/
    uint64_t    GRXFCS;   /*< Receive FCS Error Frame Counter*/
    uint64_t    GRXCF;    /*< Receive Control Frame Counter*/
    uint64_t    GRXPF;    /*< Receive PAUSE Frame Counter*/
    uint64_t    GRXPP;    /*< Receive PFC (Per-Priority Pause) Frame Counter*/
    uint64_t    GRXUO;    /*< Receive Unsupported Opcode Frame Counter*/
    uint64_t    GRXUDA;   /*< Receive Unsupported DA for PAUSE/PFC Frame Counter*/
    uint64_t    GRXWSA;   /*< Receive Wrong SA Frame Counter*/
    uint64_t    GRXALN;   /*< Receive Alignment Error Frame Counter*/
    uint64_t    GRXFLR;   /*< Receive Length Out of Range Frame Counter*/
    uint64_t    GRXFRERR; /*< Receive Code Error Frame Counter*/
    uint64_t    GRXFCR;   /*< Receive False Carrier Counter*/
    uint64_t    GRXOVR;   /*< Receive Oversized Frame Counter*/
    uint64_t    GRXJBR;   /*< Receive Jabber Frame Counter*/
    uint64_t    GRXMTUE;  /*< Receive MTU Check Error Frame Counter*/
    uint64_t    GRXMCRC;  /*< Matched CRC Frame Counter*/
    uint64_t    GRXPRM;   /*< Receive Promiscuous Frame Counter*/
    uint64_t    GRXVLN;   /*< Receive VLAN Tag Frame Counter*/
    uint64_t    GRXDVLN;  /*< Receive Double VLAN Tag Frame Counter*/
    uint64_t    GRXTRFU;  /*< Receive Truncated Frame Counter (due to RX FIFO full)*/
    uint64_t    GRXPOK;   /*< Receive Good Packet Counter*/
    uint64_t    GRXSCHCRC;    /*< Receive SCH CRC Error*/
    uint64_t    GRXBYT;   /*< Receive Byte Counter*/
    uint64_t    GRXRPKT;  /*< Receive RUNT Frame Counter*/
    uint64_t    GRXUND;   /*< Receive Undersize Frame Counter*/
    uint64_t    GRXFRG;   /*< Receive Fragment Counter*/
    uint64_t    GRXRBYT;  /*< Receive Runt Byte Counter*/
    uint32_t    GRXLPI;   /*< RX EEE LPI Event Counter*/
    uint32_t    GRXDLPI;  /*< RX EEE LPI Duration Counte*/
    uint64_t    GRXPTLLFC;    /*< Receive Physical Type LLFC message counter*/
    uint64_t    GRXLTLLFC;   /*< Receive Logical Type LLFC message Counter*/
    uint64_t    GRXLLFCFCS;   /*< Receive Type LLFC message with CRC error Counter*/
}xport_rx_stats_s;

typedef struct
{
    uint64_t    GTX64;    /*< Transmit 64 Byte Frame Counter*/
    uint64_t    GTX127;   /*< Transmit 65 to 127 Byte Frame Counter*/
    uint64_t    GTX255;   /*< Transmit 128 to 255 Byte Frame Counter*/
    uint64_t    GTX511;   /*< Transmit 256 to 511 Byte Frame Counter*/
    uint64_t    GTX1023;  /*< Transmit 512 to 1023 Byte Frame Counter*/
    uint64_t    GTX1518;  /*< Transmit 1024 to 1518 Byte Frame Counter*/
    uint64_t    GTX1522;  /*< Transmit 1519 to 1522 Byte Good VLAN Frame Counter*/
    uint64_t    GTX2047;  /*< Transmit 1519 to 2047 Byte Frame Counter*/
    uint64_t    GTX4095;  /*< Transmit 2048 to 4095 Byte Frame Counter*/
    uint64_t    GTX9216;  /*< Transmit 4096 to 9216 Byte Frame Counter*/
    uint64_t    GTX16383; /*< Transmit 9217 to 16383 Byte Frame Counter*/
    uint64_t    GTXPOK;   /*< Transmit Good Packet Counter*/
    uint64_t    GTXPKT;   /*< Transmit Packet/Frame Counter*/
    uint64_t    GTXUCA;   /*< Transmit Unicast Frame Counter*/
    uint64_t    GTXMCA;   /*< Transmit Multicast Frame Counter*/
    uint64_t    GTXBCA;   /*< Transmit Broadcast Frame Counter*/
    uint64_t    GTXPF;    /*< Transmit Pause Control Frame Counter*/
    uint64_t    GTXPFC;   /*< Transmit PFC/Per-Priority Pause Control Frame Counter*/
    uint64_t    GTXJBR;   /*< Transmit Jabber Counter*/
    uint64_t    GTXFCS;   /*< Transmit FCS Error Counter*/
    uint64_t    GTXCF;    /*< Transmit Control Frame Counter*/
    uint64_t    GTXOVR;   /*< Transmit Oversize Packet Counter*/
    uint64_t    GTXDFR;   /*< Transmit Single Deferral Frame Counter*/
    uint64_t    GTXEDF;   /*< Transmit Multiple Deferral Frame Counter*/
    uint64_t    GTXSCL;   /*< Transmit Single Collision Frame Counter*/
    uint64_t    GTXMCL;   /*< Transmit Multiple Collision Frame Counter*/
    uint64_t    GTXLCL;   /*< Transmit Late Collision Frame Counter*/
    uint64_t    GTXXCL;   /*< Transmit Excessive Collision Frame Counter*/
    uint64_t    GTXFRG;   /*< Transmit Fragment Counter*/
    uint64_t    GTXERR;   /*< Transmit Error (set by system) Counter*/
    uint64_t    GTXVLN;   /*< Transmit VLAN Tag Frame Counter*/
    uint64_t    GTXDVLN;  /*< Transmit Double VLAN Tag Frame Counter*/
    uint64_t    GTXRPKT;  /*< Transmit RUNT Frame Counter*/
    uint64_t    GTXUFL;   /*< Transmit FIFO Underrun Counter.*/
    uint64_t    GTXNCL;   /*< Transmit Total Collision Counter*/
    uint64_t    GTXBYT;   /*< Transmit Byte Counter*/
    uint32_t    GTXLPI;   /*< TX EEE LPI Event Counter*/
    uint32_t    GTXDLPI;  /*< TX EEE LPI Duration Counter*/
    uint64_t    GTXLTLLFC;    /*< Transmit Logical Type LLFC message counter*/
}xport_tx_stats_s;

typedef struct
{
    uint8_t saturate_en; /*< Saturate counter on respective max value*/
    uint8_t cor_en; /*< Enable clear on read */
    uint8_t eee_sym_mode; /*< EEE Counter symetric mode */
}xport_stats_cfg_s;

int xport_stats_get_rx(uint32_t portid, xport_rx_stats_s *rx_stats);
int xport_stats_get_tx(uint32_t portid, xport_tx_stats_s *tx_stats);
int xport_stats_rst_rx_single(uint32_t portid, XPORT_STATS_RX_TYPE stat_type);
int xport_stats_rst_tx_single(uint32_t portid,  XPORT_STATS_TX_TYPE stat_type);
int xport_stats_cfg_get(uint32_t portid, xport_stats_cfg_s *stats_cfg);
int xport_stats_cfg_set(uint32_t portid, xport_stats_cfg_s *stats_cfg);
int xport_stats_reset(uint32_t portid);

#endif /* SHARED_OPENSOURCE_DRV_XPORT_XPORT_STATS_H_ */
