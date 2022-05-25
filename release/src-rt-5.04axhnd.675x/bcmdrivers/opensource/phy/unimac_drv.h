/*
   Copyright (c) 2013 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard

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
/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the definition for the Unimac block 			          */
/*                                                                            */
/******************************************************************************/
#ifndef __UNIMAC_DRV_H
#define __UNIMAC_DRV_H

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#include "os_dep.h"
#include "hwapi_mac.h"
#if defined(CONFIG_BCM947622)
#define WRITE_32(a, r)			( *(volatile uint32_t*)(a) = *(uint32_t*)&(r) )
#define READ_32(a, r)			( *(volatile uint32_t*)&(r) = *(volatile uint32_t*) (a) )
#define VAL32(_a)               ( *(volatile uint32_t*)(_a))
#else
#include "access_macros.h"
#endif

#if !defined(CONFIG_BCM963138) && !defined(CONFIG_BCM963148) && !defined(CONFIG_BCM963148) && !defined(CONFIG_BCM94908)
#define swap4bytes(x)    (x)
#endif

#if defined(CONFIG_BCM94908)
// remove dependency to soc_base_address
#undef DEVICE_ADDRESS
#define DEVICE_ADDRESS(_a) ( (volatile uint8_t * const) ((uint64_t)(_a) ) )
#endif

extern uintptr_t unimac_conf_base;
extern uintptr_t unimac_mib_base;
extern uintptr_t unimac_top_base;
extern uint32_t conf_offset;
extern uint32_t mib_offset;
extern uint32_t top_offset;
extern uint32_t mib_step;

#define UNIMAC_CFG_BASE                     unimac_conf_base
#define UNIMAC_MIB_BASE                     unimac_mib_base
#define UNIMAC_TOP_BASE                     unimac_top_base

#define MAX_NUM_OF_EMACS                    8
#define UNIMAC_CONF_INSTANCE_OFFSET(e)      ((e) * conf_offset)
#define UNIMAC_MIB_INSTANCE_OFFSET(e)       ((e) * mib_offset)
#define UNIMAC_MISC_INSTANCE_OFFSET(e)      ((e) * top_offset)
#define MIB_STEP                            mib_step

/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/

#define UNIMAC_CONFIGURATION_UMAC_0_UMAC_DUMMY              UNIMAC_CFG_BASE + 0x0000    /* UniMAC Dummy Register */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_HD_BKP_CNTL         UNIMAC_CFG_BASE + 0x0004    /* UniMAC Half Duplex Backpressure Control Register */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD                 UNIMAC_CFG_BASE + 0x0008    /* UniMAC Command Register */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_MAC0                UNIMAC_CFG_BASE + 0x000c    /* UniMAC MAC address first 4 bytes*/
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_MAC1                UNIMAC_CFG_BASE + 0x0010    /* UniMAC MAC address 2 last bytes*/
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_FRM_LEN             UNIMAC_CFG_BASE + 0x0014    /* UniMAC Frame Length */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_QUNAT         UNIMAC_CFG_BASE + 0x0018    /* UniMAC Pause Quanta */
#define UNIMAC_CONFIGURATION_UMAC_0_SFD_OFFSET              UNIMAC_CFG_BASE + 0x0040    /* UniMAC EFM Preamble Length */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_MODE                UNIMAC_CFG_BASE + 0x0044    /* UniMAC Mode */
#define UNIMAC_CONFIGURATION_UMAC_0_FRM_TAG0                UNIMAC_CFG_BASE + 0x0048    /* UniMAC Preamble Outer TAG 0 */
#define UNIMAC_CONFIGURATION_UMAC_0_FRM_TAG1                UNIMAC_CFG_BASE + 0x004c    /* UniMAC Preamble Outer TAG 1 */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_TX_IPG_LEN          UNIMAC_CFG_BASE + 0x005c    /* UniMAC Inter Packet Gap */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_EEE_CTRL            UNIMAC_CFG_BASE + 0x0064    /* UniMAC Energy Efficient Ethernet Control */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_EEE_LPI_TIMER       UNIMAC_CFG_BASE + 0x0068    /* EEE LPI timer */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_EEE_WAKE_TIMER      UNIMAC_CFG_BASE + 0x006c    /* EEE wakeup timer */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_EEE_REF_COUNT       UNIMAC_CFG_BASE + 0x0070    /* UniMAC Energy Efficient Ethernet Ref Clock Speed */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_EEE_MII_LPI_TIMER   UNIMAC_CFG_BASE + 0x0068    /* MII EEE LPI timer */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_EEE_GMII_LPI_TIMER  UNIMAC_CFG_BASE + 0x006c    /* GMII EEE LPI timer */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_EEE_MII_WAKE_TIMER  UNIMAC_CFG_BASE + 0x0080    /* MII EEE wakeup timer */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_EEE_GMII_WAKE_TIMER UNIMAC_CFG_BASE + 0x0084    /* GMII EEE wakeup timer */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_CNTRL         UNIMAC_CFG_BASE + 0x0330    /* UniMAC Repetitive Pause Control in TX direction */
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_RX_MAX_PKT_SIZE     UNIMAC_CFG_BASE + 0x0608    /* UniMAC RX MAX packet Size Register */

#ifndef BRCM_64B_UMAC_MIB

#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR64      UNIMAC_MIB_BASE + 0x0000*MIB_STEP   /* Receive 64B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR127     UNIMAC_MIB_BASE + 0x0004*MIB_STEP   /* Receive 65B to 127B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR255     UNIMAC_MIB_BASE + 0x0008*MIB_STEP   /* Receive 128B to 255B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR511     UNIMAC_MIB_BASE + 0x000c*MIB_STEP   /* Receive 256B to 511B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR1023    UNIMAC_MIB_BASE + 0x0010*MIB_STEP   /* Receive 512B to 1023B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR1518    UNIMAC_MIB_BASE + 0x0014*MIB_STEP   /* Receive 1024B to 1518B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRMGV     UNIMAC_MIB_BASE + 0x0018*MIB_STEP   /* Receive 1519B to 1522B Good VLAN Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR2047    UNIMAC_MIB_BASE + 0x001c*MIB_STEP   /* Receive 1519B to 2047B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR4095    UNIMAC_MIB_BASE + 0x0020*MIB_STEP   /* Receive 2048B to 4095B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR9216    UNIMAC_MIB_BASE + 0x0024*MIB_STEP   /* Receive 4096B to 9216B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRPKT     UNIMAC_MIB_BASE + 0x0028*MIB_STEP   /* Receive Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRBYT     UNIMAC_MIB_BASE + 0x002c*MIB_STEP   /* Receive Byte Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRMCA     UNIMAC_MIB_BASE + 0x0030*MIB_STEP   /* Receive Multicast Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRBCA     UNIMAC_MIB_BASE + 0x0034*MIB_STEP   /* Receive Broadcast Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRFCS     UNIMAC_MIB_BASE + 0x0038*MIB_STEP   /* Receive FCS Error Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRXCF     UNIMAC_MIB_BASE + 0x003c*MIB_STEP   /* Receive Control Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRXPF     UNIMAC_MIB_BASE + 0x0040*MIB_STEP   /* Receive Pause Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRXUO     UNIMAC_MIB_BASE + 0x0044*MIB_STEP   /* Receive Unknown OP Code Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRALN     UNIMAC_MIB_BASE + 0x0048*MIB_STEP   /* Receive Alignmenet Error Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRFLR     UNIMAC_MIB_BASE + 0x004c*MIB_STEP   /* Receive Frame Length Out Of Range Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRCDE     UNIMAC_MIB_BASE + 0x0050*MIB_STEP   /* Receive Code Error Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRFCR     UNIMAC_MIB_BASE + 0x0054*MIB_STEP   /* Receive Carrier Sense Error Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GROVR     UNIMAC_MIB_BASE + 0x0058*MIB_STEP   /* Receive Oversize Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRJBR     UNIMAC_MIB_BASE + 0x005c*MIB_STEP   /* Receive Jabber Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRMTUE    UNIMAC_MIB_BASE + 0x0060*MIB_STEP   /* Receive MTU Error Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRPOK     UNIMAC_MIB_BASE + 0x0064*MIB_STEP   /* Receive Good Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRUC      UNIMAC_MIB_BASE + 0x0068*MIB_STEP   /* Receive Unicast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRPPP     UNIMAC_MIB_BASE + 0x006c*MIB_STEP   /* Receive PPP Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRCRC     UNIMAC_MIB_BASE + 0x0070*MIB_STEP   /* Receive CRC Match Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR64      UNIMAC_MIB_BASE + 0x0080*MIB_STEP   /* Transmit 64B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR127     UNIMAC_MIB_BASE + 0x0084*MIB_STEP   /* Transmit 65B to 127B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR255     UNIMAC_MIB_BASE + 0x0088*MIB_STEP   /* Transmit 128B to 255B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR511     UNIMAC_MIB_BASE + 0x008c*MIB_STEP   /* Transmit 256B to 511B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR1023    UNIMAC_MIB_BASE + 0x0090*MIB_STEP   /* Transmit 512B to 1023B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR1518    UNIMAC_MIB_BASE + 0x0094*MIB_STEP   /* Transmit 1024B to 1518B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TRMGV     UNIMAC_MIB_BASE + 0x0098*MIB_STEP   /* Transmit 1519B to 1522B Good VLAN Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR2047    UNIMAC_MIB_BASE + 0x009c*MIB_STEP   /* Transmit 1519B to 2047B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR4095    UNIMAC_MIB_BASE + 0x00a0*MIB_STEP   /* Transmit 2048B to 4095B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR9216    UNIMAC_MIB_BASE + 0x00a4*MIB_STEP   /* Transmit 4096B to 9216B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTPKT     UNIMAC_MIB_BASE + 0x00a8*MIB_STEP   /* Transmit Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTMCA     UNIMAC_MIB_BASE + 0x00ac*MIB_STEP   /* Transmit Multicast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTBCA     UNIMAC_MIB_BASE + 0x00b0*MIB_STEP   /* Transmit Broadcast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTXPF     UNIMAC_MIB_BASE + 0x00b4*MIB_STEP   /* Transmit Pause Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTXCF     UNIMAC_MIB_BASE + 0x00b8*MIB_STEP   /* Transmit Control Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTFCS     UNIMAC_MIB_BASE + 0x00bc*MIB_STEP   /* Transmit FCS Error Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTOVR     UNIMAC_MIB_BASE + 0x00c0*MIB_STEP   /* Transmit Oversize Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTDRF     UNIMAC_MIB_BASE + 0x00c4*MIB_STEP   /* Transmit Deferral Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTEDF     UNIMAC_MIB_BASE + 0x00c8*MIB_STEP   /* Transmit Excessive Deferral Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTSCL     UNIMAC_MIB_BASE + 0x00cc*MIB_STEP   /* Transmit Single Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTMCL     UNIMAC_MIB_BASE + 0x00d0*MIB_STEP   /* Transmit Multiple Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTLCL     UNIMAC_MIB_BASE + 0x00d4*MIB_STEP   /* Transmit Late Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTXCL     UNIMAC_MIB_BASE + 0x00d8*MIB_STEP   /* Transmit Excessive Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTFRG     UNIMAC_MIB_BASE + 0x00dc*MIB_STEP   /* Transmit Fragments Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTNCL     UNIMAC_MIB_BASE + 0x00e0*MIB_STEP   /* Transmit Total Collision Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTJBR     UNIMAC_MIB_BASE + 0x00e4*MIB_STEP   /* Transmit Jabber Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTBYT     UNIMAC_MIB_BASE + 0x00e8*MIB_STEP   /* Transmit Byte Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTPOK     UNIMAC_MIB_BASE + 0x00ec*MIB_STEP   /* Transmit Good Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTUC      UNIMAC_MIB_BASE + 0x00f0*MIB_STEP   /* Transmit Unicast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRPKT     UNIMAC_MIB_BASE + 0x0100*MIB_STEP   /* Receive RUNT Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRUND     UNIMAC_MIB_BASE + 0x0104*MIB_STEP   /* Receive RUNT Packet And Contain A Valid FCS */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRFRG     UNIMAC_MIB_BASE + 0x0108*MIB_STEP   /* Receive RUNT Packet And Contain Invalid FCS or Alignment Error */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRBYT     UNIMAC_MIB_BASE + 0x010c*MIB_STEP   /* Receive RUNT Packet Byte Counter */
#if defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_MIB_CNTRL UNIMAC_MIB_BASE + 0x0140*MIB_STEP   /* MIB Control Register */
#else
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_MIB_CNTRL UNIMAC_MIB_BASE + 0x0180*MIB_STEP   /* MIB Control Register */
#endif

#else /* BRCM_64B_UMAC_MIB */

#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR64            UNIMAC_MIB_BASE + 0x0000    /* Receive 64B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR64_UPPER      UNIMAC_MIB_BASE + 0x0004    /* Receive 64B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR127           UNIMAC_MIB_BASE + 0x0008    /* Receive 65B to 127B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR127_UPPER     UNIMAC_MIB_BASE + 0x000c    /* Receive 65B to 127B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR255           UNIMAC_MIB_BASE + 0x0010    /* Receive 128B to 255B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR255_UPPER     UNIMAC_MIB_BASE + 0x0014    /* Receive 128B to 255B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR511           UNIMAC_MIB_BASE + 0x0018    /* Receive 256B to 511B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR511_UPPER     UNIMAC_MIB_BASE + 0x001c    /* Receive 256B to 511B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR1023          UNIMAC_MIB_BASE + 0x0020    /* Receive 512B to 1023B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR1023_UPPER    UNIMAC_MIB_BASE + 0x0024    /* Receive 512B to 1023B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR1518          UNIMAC_MIB_BASE + 0x0028    /* Receive 1024B to 1518B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR1518_UPPER    UNIMAC_MIB_BASE + 0x002c    /* Receive 1024B to 1518B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRMGV           UNIMAC_MIB_BASE + 0x0030    /* Receive 1519B to 1522B Good VLAN Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRMGV_UPPER     UNIMAC_MIB_BASE + 0x0034    /* Receive 1519B to 1522B Good VLAN Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR2047          UNIMAC_MIB_BASE + 0x0038    /* Receive 1519B to 2047B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR2047_UPPER    UNIMAC_MIB_BASE + 0x003c    /* Receive 1519B to 2047B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR4095          UNIMAC_MIB_BASE + 0x0040    /* Receive 2048B to 4095B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR4095_UPPER    UNIMAC_MIB_BASE + 0x0044    /* Receive 2048B to 4095B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR9216          UNIMAC_MIB_BASE + 0x0048    /* Receive 4096B to 9216B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GR9216_UPPER    UNIMAC_MIB_BASE + 0x004c    /* Receive 4096B to 9216B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRPKT           UNIMAC_MIB_BASE + 0x0050    /* Receive Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRPKT_UPPER     UNIMAC_MIB_BASE + 0x0054    /* Receive Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRBYT           UNIMAC_MIB_BASE + 0x0058    /* Receive Byte Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRBYT_UPPER     UNIMAC_MIB_BASE + 0x005c    /* Receive Byte Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRMCA           UNIMAC_MIB_BASE + 0x0060    /* Receive Multicast Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRMCA_UPPER     UNIMAC_MIB_BASE + 0x0064    /* Receive Multicast Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRBCA           UNIMAC_MIB_BASE + 0x0068    /* Receive Broadcast Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRBCA_UPPER     UNIMAC_MIB_BASE + 0x006c    /* Receive Broadcast Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRFCS           UNIMAC_MIB_BASE + 0x0070    /* Receive FCS Error Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRFCS_UPPER     UNIMAC_MIB_BASE + 0x0074    /* Receive FCS Error Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRXCF           UNIMAC_MIB_BASE + 0x0078    /* Receive Control Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRXCF_UPPER     UNIMAC_MIB_BASE + 0x007c    /* Receive Control Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRXPF           UNIMAC_MIB_BASE + 0x0080    /* Receive Pause Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRXPF_UPPER     UNIMAC_MIB_BASE + 0x0084    /* Receive Pause Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRXUO           UNIMAC_MIB_BASE + 0x0088    /* Receive Unknown OP Code Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRXUO_UPPER     UNIMAC_MIB_BASE + 0x008c    /* Receive Unknown OP Code Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRALN           UNIMAC_MIB_BASE + 0x0090    /* Receive Alignmenet Error Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRALN_UPPER     UNIMAC_MIB_BASE + 0x0094    /* Receive Alignmenet Error Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRFLR           UNIMAC_MIB_BASE + 0x0098    /* Receive Frame Length Out Of Range Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRFLR_UPPER     UNIMAC_MIB_BASE + 0x009c    /* Receive Frame Length Out Of Range Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRCDE           UNIMAC_MIB_BASE + 0x00a0    /* Receive Code Error Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRCDE_UPPER     UNIMAC_MIB_BASE + 0x00a4    /* Receive Code Error Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRFCR           UNIMAC_MIB_BASE + 0x00a8    /* Receive Carrier Sense Error Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRFCR_UPPER     UNIMAC_MIB_BASE + 0x00ac    /* Receive Carrier Sense Error Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GROVR           UNIMAC_MIB_BASE + 0x00b0    /* Receive Oversize Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GROVR_UPPER     UNIMAC_MIB_BASE + 0x00b4    /* Receive Oversize Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRJBR           UNIMAC_MIB_BASE + 0x00b8    /* Receive Jabber Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRJBR_UPPER     UNIMAC_MIB_BASE + 0x00bc    /* Receive Jabber Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRMTUE          UNIMAC_MIB_BASE + 0x00c0    /* Receive MTU Error Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRMTUE_UPPER    UNIMAC_MIB_BASE + 0x00c4    /* Receive MTU Error Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRPOK           UNIMAC_MIB_BASE + 0x00c8    /* Receive Good Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRPOK_UPPER     UNIMAC_MIB_BASE + 0x00cc    /* Receive Good Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRUC            UNIMAC_MIB_BASE + 0x00d0    /* Receive Unicast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRUC_UPPER      UNIMAC_MIB_BASE + 0x00d4    /* Receive Unicast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRPPP           UNIMAC_MIB_BASE + 0x00d8    /* Receive PPP Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRPPP_UPPER     UNIMAC_MIB_BASE + 0x00dc    /* Receive PPP Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRCRC           UNIMAC_MIB_BASE + 0x00e0    /* Receive CRC Match Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GRCRC_UPPER     UNIMAC_MIB_BASE + 0x00e4    /* Receive CRC Match Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR64            UNIMAC_MIB_BASE + 0x0100    /* Transmit 64B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR64_UPPER      UNIMAC_MIB_BASE + 0x0104    /* Transmit 64B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR127           UNIMAC_MIB_BASE + 0x0108    /* Transmit 65B to 127B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR127_UPPER     UNIMAC_MIB_BASE + 0x010c    /* Transmit 65B to 127B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR255           UNIMAC_MIB_BASE + 0x0110    /* Transmit 128B to 255B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR255_UPPER     UNIMAC_MIB_BASE + 0x0114    /* Transmit 128B to 255B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR511           UNIMAC_MIB_BASE + 0x0118    /* Transmit 256B to 511B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR511_UPPER     UNIMAC_MIB_BASE + 0x011c    /* Transmit 256B to 511B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR1023          UNIMAC_MIB_BASE + 0x0120    /* Transmit 512B to 1023B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR1023_UPPER    UNIMAC_MIB_BASE + 0x0124    /* Transmit 512B to 1023B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR1518          UNIMAC_MIB_BASE + 0x0128    /* Transmit 1024B to 1518B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR1518_UPPER    UNIMAC_MIB_BASE + 0x012c    /* Transmit 1024B to 1518B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TRMGV           UNIMAC_MIB_BASE + 0x0130    /* Transmit 1519B to 1522B Good VLAN Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TRMGV_UPPER     UNIMAC_MIB_BASE + 0x0134    /* Transmit 1519B to 1522B Good VLAN Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR2047          UNIMAC_MIB_BASE + 0x0138    /* Transmit 1519B to 2047B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR2047_UPPER    UNIMAC_MIB_BASE + 0x013c    /* Transmit 1519B to 2047B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR4095          UNIMAC_MIB_BASE + 0x0140    /* Transmit 2048B to 4095B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR4095_UPPER    UNIMAC_MIB_BASE + 0x0144    /* Transmit 2048B to 4095B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR9216          UNIMAC_MIB_BASE + 0x0148    /* Transmit 4096B to 9216B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_TR9216_UPPER    UNIMAC_MIB_BASE + 0x014c    /* Transmit 4096B to 9216B Frame Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTPKT           UNIMAC_MIB_BASE + 0x0150    /* Transmit Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTPKT_UPPER     UNIMAC_MIB_BASE + 0x0154    /* Transmit Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTMCA           UNIMAC_MIB_BASE + 0x0158    /* Transmit Multicast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTMCA_UPPER     UNIMAC_MIB_BASE + 0x015c    /* Transmit Multicast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTBCA           UNIMAC_MIB_BASE + 0x0160    /* Transmit Broadcast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTBCA_UPPER     UNIMAC_MIB_BASE + 0x0164    /* Transmit Broadcast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTXPF           UNIMAC_MIB_BASE + 0x0168    /* Transmit Pause Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTXPF_UPPER     UNIMAC_MIB_BASE + 0x016c    /* Transmit Pause Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTXCF           UNIMAC_MIB_BASE + 0x0170    /* Transmit Control Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTXCF_UPPER     UNIMAC_MIB_BASE + 0x0174    /* Transmit Control Frame Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTFCS           UNIMAC_MIB_BASE + 0x0178    /* Transmit FCS Error Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTFCS_UPPER     UNIMAC_MIB_BASE + 0x017c    /* Transmit FCS Error Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTOVR           UNIMAC_MIB_BASE + 0x0180    /* Transmit Oversize Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTOVR_UPPER     UNIMAC_MIB_BASE + 0x0184    /* Transmit Oversize Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTDRF           UNIMAC_MIB_BASE + 0x0188    /* Transmit Deferral Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTDRF_UPPER     UNIMAC_MIB_BASE + 0x018c    /* Transmit Deferral Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTEDF           UNIMAC_MIB_BASE + 0x0190    /* Transmit Excessive Deferral Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTEDF_UPPER     UNIMAC_MIB_BASE + 0x0194    /* Transmit Excessive Deferral Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTSCL           UNIMAC_MIB_BASE + 0x0198    /* Transmit Single Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTSCL_UPPER     UNIMAC_MIB_BASE + 0x019c    /* Transmit Single Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTMCL           UNIMAC_MIB_BASE + 0x01a0    /* Transmit Multiple Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTMCL_UPPER     UNIMAC_MIB_BASE + 0x01a4    /* Transmit Multiple Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTLCL           UNIMAC_MIB_BASE + 0x01a8    /* Transmit Late Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTLCL_UPPER     UNIMAC_MIB_BASE + 0x01ac    /* Transmit Late Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTXCL           UNIMAC_MIB_BASE + 0x01b0    /* Transmit Excessive Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTXCL_UPPER     UNIMAC_MIB_BASE + 0x01b4    /* Transmit Excessive Collision Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTFRG           UNIMAC_MIB_BASE + 0x01b8    /* Transmit Fragments Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTFRG_UPPER     UNIMAC_MIB_BASE + 0x01bc    /* Transmit Fragments Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTNCL           UNIMAC_MIB_BASE + 0x01c0    /* Transmit Total Collision Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTNCL_UPPER     UNIMAC_MIB_BASE + 0x01c4    /* Transmit Total Collision Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTJBR           UNIMAC_MIB_BASE + 0x01c8    /* Transmit Jabber Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTJBR_UPPER     UNIMAC_MIB_BASE + 0x01cc    /* Transmit Jabber Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTBYT           UNIMAC_MIB_BASE + 0x01d0    /* Transmit Byte Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTBYT_UPPER     UNIMAC_MIB_BASE + 0x01d4    /* Transmit Byte Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTPOK           UNIMAC_MIB_BASE + 0x01d8    /* Transmit Good Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTPOK_UPPER     UNIMAC_MIB_BASE + 0x01dc    /* Transmit Good Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTUC            UNIMAC_MIB_BASE + 0x01e0    /* Transmit Unicast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_GTUC_UPPER      UNIMAC_MIB_BASE + 0x01e4    /* Transmit Unicast Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRPKT           UNIMAC_MIB_BASE + 0x0200    /* Receive RUNT Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRPKT_UPPER     UNIMAC_MIB_BASE + 0x0204    /* Receive RUNT Packet Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRUND           UNIMAC_MIB_BASE + 0x0208    /* Receive RUNT Packet And Contain A Valid FCS */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRUND_UPPER     UNIMAC_MIB_BASE + 0x020c    /* Receive RUNT Packet And Contain A Valid FCS */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRFRG           UNIMAC_MIB_BASE + 0x0210    /* Receive RUNT Packet And Contain Invalid FCS or Alignment Error */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRFRG_UPPER     UNIMAC_MIB_BASE + 0x0214    /* Receive RUNT Packet And Contain Invalid FCS or Alignment Error */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRBYT           UNIMAC_MIB_BASE + 0x0218    /* Receive RUNT Packet Byte Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_RRBYT_UPPER     UNIMAC_MIB_BASE + 0x021c    /* Receive RUNT Packet Byte Counter */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_MIB_CNTRL       UNIMAC_MIB_BASE + 0x0280    /* MIB Control Register */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_MIB_READ        UNIMAC_MIB_BASE + 0x0284    /* MIB read data */
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_MIB_WRITE      UNIMAC_MIB_BASE + 0x0288    /* MIB read data */

#endif /* !BRCM_64B_UMAC_MIB */

#if defined(CONFIG_BCM947622)
#define UNIMAC_RXERR_MASK(e)                               (UNIMAC_TOP_BASE + 0x0024 + UNIMAC_CONF_INSTANCE_OFFSET(e))
#define UNIMAC_TOPCTRL_MIB_MAX_PKT_SIZE                     UNIMAC_TOP_BASE + 0x0028    /* Value by MIB to differentiate regular and oversized packets */

#elif defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
#define UNIMAC_RXERR_MASK(e)                               (UNIMAC_CFG_BASE + 0x0784 + UNIMAC_CONF_INSTANCE_OFFSET(e))
#define UNIMAC_XIB_CTRL                                     UNIMAC_CFG_BASE + 0x0740    /* XIB Control Register */
#define     XIB_CTRL_STRICT_PREAMBLE_DIS                    (1<<18)
#define UNIMAC_TOPCTRL_MIB_MAX_PKT_SIZE                     UNIMAC_CFG_BASE + 0x0788    /* Value by MIB to differentiate regular and oversized packets */
#define UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2              UNIMAC_TOP_BASE + 0x0008    /* UNIMAC_EXT_CFG2 Register */

#else
#define UNIMAC_RXERR_MASK(e)                               (UNIMAC_TOP_BASE + 0x001c + UNIMAC_MISC_INSTANCE_OFFSET(e))
#define UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG                   UNIMAC_TOP_BASE + 0x0000    /* UNIMAC_CFG Register */
#define UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1              UNIMAC_TOP_BASE + 0x0004    /* UNIMAC_EXT_CFG1 Register */
#define UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2              UNIMAC_TOP_BASE + 0x0008    /* UNIMAC_EXT_CFG2 Register */
#endif


#define UNIMAC_CONFIGURATION_UMAC_0_RDP_HD_BKP_CNTL_ipg_config_rx_MASK          0x0000007c
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_HD_BKP_CNTL_ipg_config_rx_SHIFT         2
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_HD_BKP_CNTL_hd_fc_ena_MASK              0x00000001
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_HD_BKP_CNTL_hd_fc_ena_SHIFT             0
#if defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_eth_speed_bit2_MASK                 0x80000000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_eth_speed_bit2_SHIFT                31
#endif
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_txrx_en_config_MASK                 0x20000000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_txrx_en_config_SHIFT                29
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_tx_pause_ignore_MASK                0x10000000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_tx_pause_ignore_SHIFT               28
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_rmt_loop_ena_MASK                   0x02000000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_rmt_loop_ena_SHIFT                  25
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_cntl_frm_ena_MASK                   0x00800000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_cntl_frm_ena_SHIFT                  23
#if defined(CONFIG_BCM947622) || defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_sw_override_rx_MASK                 0x00040000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_sw_override_rx_SHIFT                18
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_sw_override_tx_MASK                 0x00020000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_sw_override_tx_SHIFT                17
#endif
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_lcl_loop_ena_MASK                   0x00008000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_lcl_loop_ena_SHIFT                  15
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_sw_reset_MASK                       0x00002000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_sw_reset_SHIFT                      13
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_hd_ena_MASK                         0x00000400
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_hd_ena_SHIFT                        10
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_rx_pause_ignore_MASK                0x00000100
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_rx_pause_ignore_SHIFT               8
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_pause_fwd_MASK                      0x00000080
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_pause_fwd_SHIFT                     7
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_crc_fwd_MASK                        0x00000040
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_crc_fwd_SHIFT                       6
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_pad_en_MASK                         0x00000020
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_pad_en_SHIFT                        5
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_eth_speed_MASK                      0x0000000c
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_eth_speed_SHIFT                     2
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_rx_ena_MASK                         0x00000002
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_rx_ena_SHIFT                        1
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_tx_ena_MASK                         0x00000001
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_CMD_tx_ena_SHIFT                        0
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_FRM_LEN_frame_length_MASK               0x00003fff
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_FRM_LEN_frame_length_SHIFT              0
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_TX_IPG_LEN_tx_ipg_len_MASK              0x0000007f
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_TX_IPG_LEN_tx_ipg_len_SHIFT             0
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_TX_IPG_LEN_tx_min_pkt_size_MASK         0x00007F00
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_TX_IPG_LEN_tx_min_pkt_size_SHIFT        8
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_RX_MAX_PKT_SIZE_max_pkt_size_MASK       0x00003fff
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_RX_MAX_PKT_SIZE_max_pkt_size_SHIFT      0
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_CNTRL_pause_control_en_MASK       0x00020000
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_CNTRL_pause_control_en_SHIFT      17
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_CNTRL_pause_timer_MASK            0x0001ffff
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_CNTRL_pause_timer_SHIFT           0
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_QUNAT_pause_quant_MASK            0x0000ffff
#define UNIMAC_CONFIGURATION_UMAC_0_RDP_PAUSE_QUNAT_pause_quant_SHIFT           0

#define UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2_BACKPRESSURE_ENABLE_INT          1 << 16

#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_MIB_CNTRL_rx_cnt_st_MASK      0x00000001
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_MIB_CNTRL_rx_cnt_st_SHIFT     0
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_MIB_CNTRL_tx_cnt_rst_MASK     0x00000004
#define UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_MIB_CNTRL_tx_cnt_rst_SHIFT    2

/* Macros for reading and writing UNIMAC_CONFIGURATION registers*/

#define UNIMAC_READ32_REG(e,r,o) READ_32( UNIMAC_CONFIGURATION_UMAC_0_RDP_##r + UNIMAC_CONF_INSTANCE_OFFSET(e) , (o) )

#define UNIMAC_WRITE32_REG(e,r,i) WRITE_32( UNIMAC_CONFIGURATION_UMAC_0_RDP_##r  + UNIMAC_CONF_INSTANCE_OFFSET(e) , (i) )

#define UNIMAC_READ_FIELD(e,r,f,o) \
	 ( *(volatile uint32_t*)&(o) = (swap4bytes(VAL32(UNIMAC_CONFIGURATION_UMAC_0_RDP_##r + UNIMAC_CONF_INSTANCE_OFFSET(e) ))   & \
		 UNIMAC_CONFIGURATION_UMAC_0_RDP_##r##_##f##_MASK ) >> UNIMAC_CONFIGURATION_UMAC_0_RDP_##r##_##f##_SHIFT  )

#define UNIMAC_WRITE_FIELD(e,r,f,i) \
	    ( ( VAL32(UNIMAC_CONFIGURATION_UMAC_0_RDP_##r + UNIMAC_CONF_INSTANCE_OFFSET(e) ) ) = \
		( VAL32(UNIMAC_CONFIGURATION_UMAC_0_RDP_##r +  UNIMAC_CONF_INSTANCE_OFFSET(e))  & swap4bytes(~UNIMAC_CONFIGURATION_UMAC_0_RDP_##r##_##f##_MASK) ) | \
		( swap4bytes(( (i) << UNIMAC_CONFIGURATION_UMAC_0_RDP_##r##_##f##_SHIFT ) & UNIMAC_CONFIGURATION_UMAC_0_RDP_##r##_##f##_MASK)) )

/*Macros for reading and Writing UNIMAC_MIB counters*/

#define UNIMAC_READ32_MIB(e,r,o) READ_32( UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_##r + UNIMAC_MIB_INSTANCE_OFFSET(e) , (o) )

#define UNIMAC_WRITE32_MIB(e,r,i) WRITE_32( UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_##r  + UNIMAC_MIB_INSTANCE_OFFSET(e) , (i) )

#define UNIMAC_READ_MIB_FIELD(e,r,f,o) \
	 ( *(volatile uint32_t*)&(o) = (* ((volatile uint32_t*) DEVICE_ADDRESS(UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_##r + UNIMAC_MIB_INSTANCE_OFFSET(e) ) )  & \
		 UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_##r##_##f##_MASK ) >> UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_##r##_##f##_SHIFT  )

#define UNIMAC_WRITE_MIB_FIELD(e,r,f,i) \
	    ( ( VAL32(UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_##r + UNIMAC_MIB_INSTANCE_OFFSET(e)) ) = \
		( VAL32(UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_##r +  UNIMAC_MIB_INSTANCE_OFFSET(e))  & swap4bytes(~UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_##r##_##f##_MASK) ) | \
		( swap4bytes(( (i) << UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_##r##_##f##_SHIFT ) & UNIMAC_CONFIGURATION_UMAC_MIB_0Module_RDP_##r##_##f##_MASK)) )


typedef enum
{
	UNIMAC_SPEED_10,
	UNIMAC_SPEED_100,
	UNIMAC_SPEED_1000,
	UNIMAC_SPEED_2500,
	UNIMAC_SPEED_10000,
	UNIMAC_SPEED_5000,
	UNIMAC_SPEED_MAX
}UNIMAC_SPEED;

typedef enum
{
	UNIMAC_DUPLEX_HALF,
	UNIMAC_DUPLEX_FULL,
	UNIMAC_DUPLEX_MAX
}UNIMAC_DUPLEX;


#ifndef _BYTE_ORDER_LITTLE_ENDIAN_
typedef struct 
{
	/*Reserved bit must be written with 0.  A read returns an unknown value.*/
	uint32_t	reserved3:1;
	/*Disable RX side RUNT filtering. 0: Enable  RUNT filtering.Reset value is 0x0. */
	uint32_t	runt_filter_dis:1;
	/*	This mode only works in auto-config mode: 
		0: After auto-config, TX_ENA and RX_ENA bits are set to 1. 1: After auto-config, TX_ENA and RX_ENA bits are set to 0,    
		meaning SW will have to come in and enable TX and RX.Reset value is 0x0. */
	uint32_t	txrx_en_config:1;
	/*Ignore TX PAUSE frame transmit request.Reset value is 0x0. */
	uint32_t	tx_pause_ignore:1;
	/*Enable extract/insert of EFM headers.Reset value is 0x0. */
	uint32_t	prbl_ena:1;
	/*This bit currently not used.Reset value is 0x0.*/
	uint32_t	rx_err_disc:1;  
	/*Enable remote loopback at the fifo system side. 0: Normal operation.Reset value is 0x0. */
	uint32_t	rmt_loop_ena:1;
	/*Payload length check. 0: Check payload length with Length/Type field. 1: Check disabled.Reset value is 0x1. */
	uint32_t	no_lgth_check:1;
	/*	MAC control frame enable. 1: MAC control frames with opcode other than 0x0001 are accepted and forwarded to the client interface.
		0: MAC control frames with opcode other than 0x0000 and 0x0001 are silently discarded.
		Reset value is 0x0.
	*/
	uint32_t	cntl_frm_ena:1; 
	/*Enable/Disable auto-configuration. 1: Enable  0: Disable Reset value is 0x0. */
	uint32_t	ena_ext_config:1;
	/*Reserved bits must be written with 0.  A read returns an unknown value. */
	uint32_t	reserved2:6;
	/*Enable GMII/MII loopback 1: Loopback enabled. 0: Normal operation.Reset value is 0x0. */
	uint32_t 	lcl_loop_ena:1;
	/*Reserved bit must be written with 0.  A read returns an unknown value. */
	uint32_t	reserved1:1;
	/*RX and RX engines are put in reset. 0: come out of SW reset.Reset value is 0x0. */
	uint32_t	sw_reset:1; 
	/* Reserved bits must be written with 0.  A read returns an unknown value. */
	uint32_t	reserved0:2;
	/* Ignored when RTH_SPEED[1]==1, gigabit mode. 1: half duplex 0: full duplex Reset value is 0x0.*/
	uint32_t	hd_ena:1;
	/*	The MAC overwrites the source MAC address with a programmed MAC address in register MAC_0 and MAC_1." 
		0: Not modify the source address received from the transmit application client.
		Reset value is 0x0.
	*/
	uint32_t	tx_addr_ins:1;
	/*	Receive PAUSE frames are ignored by the MAC. 
		0: The tramsmit process is stiooed for the amount of time specified in the pause wuanta received within the PAUSE frame.
		Reset value is 0x0. 
	*/
	uint32_t	rx_pause_ignore:1;
	/* 	PAUSE frames are forwarded to the user application. 
		0: The PAUSE frames are terminated and discarded in the MAC.Reset value is 0x1. 
	*/
	uint32_t	pause_fwd:1;
	/*	The CRC field of received frames is transmitted to the user application. 
		0: The CRC field is stripped from the frame.Reset value is 0x1. 
	*/
	uint32_t	crc_fwd:1;
	/*  Padding is removed along with crc field before the frame is sent to the user application. 
		0: No padding is removed by the MAC.
		Reset value is 0x0.
	*/
	uint32_t	pad_en:1; 
	/* All frames are received without Unicast address filtering. 
	   0: Reset value is 0x1. */
	uint32_t 	promis_en:1;
	/*00: 10Mbps, 01: 100Mbps, 10: 1000Mbps, 11: 2500Mbps Reset value is 0x2. */
	uint32_t	eth_speed:2;
	/* 	The MAC receive function is enabled. 0: The MAC receive function is disabled. 
		The enable works on packet boundary meaning that only on the assertion on the bit during every 0->1 transition of rx_dv.
		Reset value is 0x0. 
	*/	
	uint32_t	rx_ena:1;
	/*	The MAC transmit function is enabled. 0: The MAC transmit function is disabled.
		The enable works on packet boundary meaning that only on the assertion of the bit during every SOP.
		Reset value is 0x0. 
	*/
	uint32_t    tx_ena:1;

}S_UNIMAC_CMD_REG;

typedef struct
{

	/* Reserved bits must be written with 0.  A read returns an unknown value. */
	uint32_t	reserved:26;
	/*Link status indication.Reset value is 0x0. */
	uint32_t	mac_link_stat:1; 
	/* 1: MAC Tx pause enabled. 0: MAC Tx pause disabled.Reset value is 0x1. */
	uint32_t	mac_tx_pause:1;
	/*1: MAC Rx pause enabled. 0: MAC Rx pause disabled.Reset value is 0x1. */
	uint32_t	mac_rx_pause:1; 
	/*1: Half duplex. 0: Full duplex. Reset value is 0x0. */
	uint32_t	mac_duplex:1;
	/* 00: 10Mbps, 01: 100Mbps, 10: 1Gbps, 11: 2.5Gbps Reset value is 0x2. 	*/
	uint32_t	mac_speed:2;

}S_UNIMAC_STAT_REG;

typedef struct
{
	uint32_t	reserved:15;
	uint32_t	pp_gen:1; 
	uint32_t	pp_pse_en:8; 
	uint32_t	launch_en:1; 
	uint32_t	ext_tx_flow:1; 
	uint32_t	mac_crc_owrt:1; 
	uint32_t	mac_crc_fwd:1; 
	uint32_t	txcrcerr:1;
	uint32_t	ss_mode_mii:1; 
	uint32_t	gport_mode:1;
	uint32_t	gmii_direct:1;

}S_UNIMAC_CFG_REG;

typedef struct
{
    uint32_t    reserved:7;
    uint32_t    rxfifo_congestion_threshold:9;
    uint32_t    reserved1:2;
    uint32_t    max_pkt_size:14;

}S_UNIMAC_TOP_CFG1_REG;

typedef struct
{
	uint32_t	reserved:24;
	uint32_t	lp_idle_prediction_mode:1; 
	uint32_t	dis_eee_10m:1; 
	uint32_t	eee_txclk_dis:1; 
	uint32_t	rx_fifo_check:1; 
	uint32_t	eee_en:1;
	uint32_t	en_lpi_tx_pause:1; 
	uint32_t	en_lpi_tx_pfc:1;
	uint32_t	en_lpi_rx_pause:1;
}S_UNIMAC_EEE_CTRL_REG;

#else /* little endian */

typedef struct 
{
	/*	The MAC transmit function is enabled. 0: The MAC transmit function is disabled.
		The enable works on packet boundary meaning that only on the assertion of the bit during every SOP.
		Reset value is 0x0. 
	*/
	uint32_t    tx_ena:1;               //b00
	/* 	The MAC receive function is enabled. 0: The MAC receive function is disabled. 
		The enable works on packet boundary meaning that only on the assertion on the bit during every 0->1 transition of rx_dv.
		Reset value is 0x0. 
	*/	
	uint32_t	rx_ena:1;               //b01
	/*00: 10Mbps, 01: 100Mbps, 10: 1000Mbps, 11: 2500Mbps Reset value is 0x2. */
	uint32_t	eth_speed:2;            //b02
	/* All frames are received without Unicast address filtering. 
	   0: Reset value is 0x1. */
	uint32_t 	promis_en:1;            //b04
	/*  Padding is removed along with crc field before the frame is sent to the user application. 
		0: No padding is removed by the MAC.
		Reset value is 0x0.
	*/
	uint32_t	pad_en:1;               //b05
	/*	The CRC field of received frames is transmitted to the user application. 
		0: The CRC field is stripped from the frame.Reset value is 0x1. 
	*/
	uint32_t	crc_fwd:1;              //b06
	/* 	PAUSE frames are forwarded to the user application. 
		0: The PAUSE frames are terminated and discarded in the MAC.Reset value is 0x1. 
	*/
	uint32_t	pause_fwd:1;            //b07
	/*	Receive PAUSE frames are ignored by the MAC. 
		0: The tramsmit process is stiooed for the amount of time specified in the pause wuanta received within the PAUSE frame.
		Reset value is 0x0. 
	*/
	uint32_t	rx_pause_ignore:1;      //b08
	/*	The MAC overwrites the source MAC address with a programmed MAC address in register MAC_0 and MAC_1." 
		0: Not modify the source address received from the transmit application client.
		Reset value is 0x0.
	*/
	uint32_t	tx_addr_ins:1;          //b09
	/* Ignored when RTH_SPEED[1]==1, gigabit mode. 1: half duplex 0: full duplex Reset value is 0x0.*/
	uint32_t	hd_ena:1;               //b10
#if defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
    uint32_t    rx_low_latency_en:1;    //b11
    uint32_t    overflow_en:1;          //b12
#else
	/* Reserved bits must be written with 0.  A read returns an unknown value. */
	uint32_t	reserved0:2;
#endif
	/*RX and RX engines are put in reset. 0: come out of SW reset.Reset value is 0x0. */
	uint32_t	sw_reset:1;             //b13
#if defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
    uint32_t    fcs_corrupt_urun_en:1;  //b14
#else 
	/*Reserved bit must be written with 0.  A read returns an unknown value. */
	uint32_t	reserved1:1;
#endif
	/*Enable GMII/MII loopback 1: Loopback enabled. 0: Normal operation.Reset value is 0x0. */
	uint32_t 	lcl_loop_ena:1;         //b15
#if defined(CONFIG_BCM947622) || defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
    uint32_t    mac_loop_con:1;         //b16
    uint32_t    sw_override_tx:1;       //b17
    uint32_t    sw_override_rx:1;       //b18
#if defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
    uint32_t    oob_efc_mode:1;         //b19
    uint32_t    bypass_oob_efc_sync:1;  //b20
#else
    uint32_t    reserved2:2;
#endif
    uint32_t    en_internal_tx_crs:1;   //b21
#else
	/*Reserved bits must be written with 0.  A read returns an unknown value. */
	uint32_t	reserved2:6;
#endif
	/*Enable/Disable auto-configuration. 1: Enable  0: Disable Reset value is 0x0. */
	uint32_t	ena_ext_config:1;       //b22
	/*	MAC control frame enable. 1: MAC control frames with opcode other than 0x0001 are accepted and forwarded to the client interface.
		0: MAC control frames with opcode other than 0x0000 and 0x0001 are silently discarded.
		Reset value is 0x0.
	*/
	uint32_t	cntl_frm_ena:1;         //b23
	/*Payload length check. 0: Check payload length with Length/Type field. 1: Check disabled.Reset value is 0x1. */
	uint32_t	no_lgth_check:1;        //b24
	/*Enable remote loopback at the fifo system side. 0: Normal operation.Reset value is 0x0. */
	uint32_t	rmt_loop_ena:1;         //b25
#if defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
    uint32_t    reserved26:1;
    uint32_t    fd_tx_urun_fix_en:1;    //b27
#else
	/*This bit currently not used.Reset value is 0x0.*/
	uint32_t	rx_err_disc:1;          //b26
	/*Enable extract/insert of EFM headers.Reset value is 0x0. */
	uint32_t	prbl_ena:1;             //b27
#endif
	/*Ignore TX PAUSE frame transmit request.Reset value is 0x0. */
	uint32_t	tx_pause_ignore:1;      //b28
#if defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
    uint32_t    oob_efc_disab:1;        //b29
#else
	/*	This mode only works in auto-config mode: 
		0: After auto-config, TX_ENA and RX_ENA bits are set to 1. 1: After auto-config, TX_ENA and RX_ENA bits are set to 0,    
		meaning SW will have to come in and enable TX and RX.Reset value is 0x0. */
	uint32_t	txrx_en_config:1;       //b29
#endif
	/*Disable RX side RUNT filtering. 0: Enable  RUNT filtering.Reset value is 0x0. */
	uint32_t	runt_filter_dis:1;      //b30
#if defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
    uint32_t    eth_speed_bit2:1;       //b31
#else
	/*Reserved bit must be written with 0.  A read returns an unknown value.*/
	uint32_t	reserved3:1;
#endif
}S_UNIMAC_CMD_REG;

typedef struct
{

	/* 00: 10Mbps, 01: 100Mbps, 10: 1Gbps, 11: 2.5Gbps Reset value is 0x2. 	*/
	uint32_t	mac_speed:2;
	/*1: Half duplex. 0: Full duplex. Reset value is 0x0. */
	uint32_t	mac_duplex:1;
	/*1: MAC Rx pause enabled. 0: MAC Rx pause disabled.Reset value is 0x1. */
	uint32_t	mac_rx_pause:1; 
	/* 1: MAC Tx pause enabled. 0: MAC Tx pause disabled.Reset value is 0x1. */
	uint32_t	mac_tx_pause:1;
	/*Link status indication.Reset value is 0x0. */
	uint32_t	mac_link_stat:1; 
	/* Reserved bits must be written with 0.  A read returns an unknown value. */
	uint32_t	reserved:26;

}S_UNIMAC_STAT_REG;

#if defined(CONFIG_BCM947622) || defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
typedef struct
{
    uint32_t    max_pkt_size:14;
    uint32_t    reserved1:18;

}S_UNIMAC_TOPCTRL_MAX_PKT_SZ_REG;

#else   //!47622
typedef struct
{
	uint32_t	gmii_direct:1;
	uint32_t	gport_mode:1;
	uint32_t	ss_mode_mii:1; 
	uint32_t	txcrcerr:1;
	uint32_t	mac_crc_fwd:1; 
	uint32_t	mac_crc_owrt:1; 
	uint32_t	ext_tx_flow:1; 
	uint32_t	launch_en:1; 
	uint32_t	pp_pse_en:8; 
	uint32_t	pp_gen:1; 
	uint32_t	reserved:15;

}S_UNIMAC_CFG_REG;

typedef struct
{
    uint32_t    max_pkt_size:14;
    uint32_t    reserved1:2;
    uint32_t    rxfifo_congestion_threshold:9;
    uint32_t    reserved:7;

}S_UNIMAC_TOP_CFG1_REG;

typedef struct
{
    /* RX FIFO Threshold - This is the fifo located between the UNIMAC IP and BBH.
       Once the threshold is reached pause is sent. This configuration is in
       16-byte resolution the number of bytes in a FIFO line */
    uint32_t    rxfifo_pause_threshod:9;
    uint32_t    reserved1:7;
    /* Backpressure enable for internal unimac */
    uint32_t    backpressure_enable_int:1;
    /* Backpressure enable for external switch */
    uint32_t    backpressure_enable_ext:1;
    /* Enable the mechanism for always receiving data from UNIMAC IP and
       dropping overrun words in the unimac glue FIFO. */
    uint32_t    fifo_overrun_ctl_en:1;
    /* When setting this bit RX recovered clock will also input the
       clk25 pll ref clk in the UNIMAC.*/
    uint32_t    remote_loopback_en:1;
    uint32_t    reserved2:12;

}S_UNIMAC_TOP_CFG2_REG;
#endif //!47622

typedef struct
{
	uint32_t	en_lpi_rx_pause:1;
	uint32_t	en_lpi_tx_pfc:1;
	uint32_t	en_lpi_tx_pause:1; 
	uint32_t	eee_en:1;
	uint32_t	rx_fifo_check:1; 
	uint32_t	eee_txclk_dis:1; 
	uint32_t	dis_eee_10m:1; 
	uint32_t	lp_idle_prediction_mode:1; 
	uint32_t	reserved:24;
}S_UNIMAC_EEE_CTRL_REG;

#endif

#endif

