/*
<:copyright-BRCM:2013:proprietary:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/
#ifndef __BCM63XX_ETH_H
#define __BCM63XX_ETH_H

#include "bcm_hwdefs.h"
#include "bcm_map.h"
#include "boardparms.h"
#include "cfe_timer.h"

// from linux if_ether.h
#define ETH_ALEN                6       /* Octets in one ethernet addr	 */
#define ETH_HLEN                14      /* Total octets in header.	 */
#define ETH_ZLEN                60      /* Min. octets in frame sans FCS */
#define ETH_DATA_LEN            1500    /* Max. octets in payload	 */
#define ETH_CRC_LEN	            4       /* CRC length */
// end if_ether.h

/*---------------------------------------------------------------------*/
/* specify number of BDs and buffers to use                            */
/*---------------------------------------------------------------------*/
#define NR_TX_BDS               40
#define NR_RX_BDS               40
#define ENET_MAX_MTU_SIZE       1522    /* Body(1500) + EH_SIZE(14) + FCS(4) + VLAN(4) */
#define DMA_MAX_BURST_LENGTH    8       /* in 64 bit words */
#define ENET_BUF_SIZE           ((ENET_MAX_MTU_SIZE + 63) & ~63)
#define DMA_FC_THRESH_LO        5
#define DMA_FC_THRESH_HI        10
#define EMAC_TX_WATERMARK       32

#define MAKE4(x)   ((x[3] & 0xFF) | ((x[2] & 0xFF) << 8) | ((x[1] & 0xFF) << 16) | ((x[0] & 0xFF) << 24))
#define MAKE2(x)   ((x[1] & 0xFF) | ((x[0] & 0xFF) << 8))


#define ERROR(x)        xsprintf x
#ifndef ASSERT
#define ASSERT(x)       if (x); else ERROR(("assert: "__FILE__" line %d\n", __LINE__)); 
#endif

//#define DUMP_TRACE

#if defined(DUMP_TRACE)
#define TRACE (x)         xprintf x
#else
#define TRACE(x)
#endif

typedef struct PM_Addr {
    uint16              valid;          /* 1 indicates the corresponding address is valid */
    unsigned char       dAddr[ETH_ALEN];/* perfect match register's destination address */
    unsigned int        ref;            /* reference count */
} PM_Addr;					 
#define MAX_PMADDR      4               /* # of perfect match address */

#define NUM_PORTS                   1

typedef struct gpio_reg_addrs_t {
    volatile uint16 *gpio_direction_reg;/* GPIO direction register */
    volatile uint16 *gpio_value_reg;    /* GPIO value register */
} gpio_reg_addrs_t;

typedef struct ethsw_info_t {
    gpio_reg_addrs_t sbh;
    uint32 ssl, clk, mosi, miso;        /* GPIO mapping */
    int cid, page;                      /* Current chip ID and page */
} ethsw_info_t;

typedef struct bcmenet_softc {

    volatile DmaRegs *dmaCtrl;
    
    /* transmit variables */    
    volatile DmaChannelCfg *txDma;      /* location of transmit DMA register set */
    volatile DmaDesc *txBds;            /* Memory location of tx Dma BD ring */
    volatile DmaDesc *txFirstBdPtr;     /* ptr to first allocated Tx BD */
    volatile DmaDesc *txNextBdPtr;      /* ptr to next Tx BD to transmit with */
    volatile DmaDesc *txLastBdPtr;      /* ptr to last allocated Tx BD */

    /* receive variables */
    volatile DmaChannelCfg *rxDma;      /* location of receive DMA register set */
    volatile DmaDesc *rxBds;            /* Memory location of rx bd ring */
    volatile DmaDesc *rxFirstBdPtr;     /* ptr to first allocated rx bd */
    volatile DmaDesc *rxBdReadPtr;      /* ptr to next rx bd to be processed */
    volatile DmaDesc *rxLastBdPtr;      /* ptr to last allocated rx bd */

    uint8_t* rxBuffers;
    uint8_t* txBuffers;

    uint16          chipId;             /* chip's id */
    uint16          chipRev;            /* step */
    uint8_t         hwaddr[ETH_ALEN];
    ethsw_info_t    ethSwitch;          /* external switch */
    ETHERNET_MAC_INFO EnetInfo;
    uint32_t dmaPort;
    uint32_t linkCheck;
} bcmenet_softc;



#define IncRxBdPtr(x, s) if (x == ((bcmenet_softc *)s)->rxLastBdPtr) \
                             x = ((bcmenet_softc *)s)->rxBds; \
                      else x++
#define InctxBdPtr(x, s) if (x == ((bcmenet_softc *)s)->txLastBdPtr) \
                             x = ((bcmenet_softc *)s)->txBds; \
                      else x++

// extern and function prototype

#ifdef DUMP_DATA
static void hexdump( unsigned char * src, int srclen, int rowlen, int rows );
#endif

#endif // __BCM63XX_ETH_H
