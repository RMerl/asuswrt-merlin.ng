#ifndef __FAP4KE_GSO_H_INCLUDED__
#define __FAP4KE_GSO_H_INCLUDED__

/*
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
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

/*
 *******************************************************************************
 * File Name  : fap4ke_gso.h
 *
 * Description: This file contains ...
 *
 *******************************************************************************
 */

#ifdef CONFIG_BCM_FAP_GSO
#define CC_FAP4KE_PKT_GSO
#define CC_FAP4KE_PKT_GSO_FRAG

#ifdef CONFIG_BCM_FAP_GSO_LOOPBACK
#define CC_FAP4KE_PKT_GSO_LOOPBACK
#endif
#endif/* CONFIG_BCM_FAP_GSO */

//#define CC_FAP4KE_GSO_DEBUG
#define CC_FAP4KE_GSO_ERROR_CHECK
//#define CC_FAP4KE_GSO_PMON_ENABLE

#define FAP4KE_GSO_CMD_LIST_SIZE      48
#define FAP4KE_GSO_CSUM_LISTS         2
#define FAP4KE_GSO_CSUM_LIST_SIZE     28

#define FAP4KE_GSO_NUM_BUFFERS        256 /* MUST BE A INTEGER MULTIPLE OF 32 */

#define FAP4KE_GSO_BUFFER_MAP_SIZE_32 (FAP4KE_GSO_NUM_BUFFERS / 32)

#define FAP4KE_GSO_BUFFER_SIZE_SHIFT  12 /* 4096 */
#define FAP4KE_GSO_BUFFER_SIZE        (1 << FAP4KE_GSO_BUFFER_SIZE_SHIFT)

#define FAP4KE_GSO_BUFFER_HEADROOM  256

#if defined(CC_FAP4KE_GSO_ERROR_CHECK)
#define FAP4KE_GSO_ERROR_CHECK(condition, label, fmt, arg...) \
        if(condition)                      \
        {                                              \
            fap4kePrt_Error(fmt, ##arg);               \
            goto label;                                \
        }
#else
#define FAP4KE_GSO_ERROR_CHECK (condition, label , fmt, arg...) 
#endif

#if defined(CC_FAP4KE_GSO_DEBUG)
#define FAP4KE_GSO_ASSERT(condition) fap4kePrt_Assert(condition)
#define FAP4KE_GSO_DEBUG(fmt, arg...) fap4kePrt_Debug(fmt, ##arg)
#define FAP4KE_GSO_PRINT(fmt, arg...) fap4kePrt_Print(fmt, ##arg)
#define FAP4KE_GSO_ERROR(fmt, arg...) fap4kePrt_Error(fmt, ##arg)
#define FAP4KE_GSO_DUMP_PACKET(_packet_p, _length) dumpPacket(_packet_p, _length)
#define FAP4KE_GSO_DUMP_HEADER(_packet_p) dumpHeader(_packet_p)
#else
#define FAP4KE_GSO_ASSERT(condition)
#define FAP4KE_GSO_DEBUG(fmt, arg...)
#define FAP4KE_GSO_PRINT(fmt, arg...)
#define FAP4KE_GSO_ERROR(fmt, arg...)
#define FAP4KE_GSO_DUMP_PACKET(_packet_p, _length)
#define FAP4KE_GSO_DUMP_HEADER(_packet_p) 
#endif

#if defined(CC_FAP4KE_GSO_PMON_ENABLE)
#define FAP4KE_GSO_PMON_DECLARE() FAP4KE_PMON_DECLARE()
#define FAP4KE_GSO_PMON_BEGIN(_pmonId) FAP4KE_PMON_BEGIN(_pmonId)
#define FAP4KE_GSO_PMON_END(_pmonId) FAP4KE_PMON_END(_pmonId)
#else
#define FAP4KE_GSO_PMON_DECLARE()
#define FAP4KE_GSO_PMON_BEGIN(_pmonId)
#define FAP4KE_GSO_PMON_END(_pmonId)
#endif


/* Maximum header size:
 * 14 (ETH) + 8 (2 VLANs) +
 * 8 (PPPoE) + 20 (IPv4) + 60 (TCP/UDP) = 110 bytes
 */
#define FAP4KE_GSO_HEADER_SIZE      110

typedef struct {
    uint8 cmdList[FAP4KE_GSO_CMD_LIST_SIZE];
    uint8 csumList[FAP4KE_GSO_CSUM_LISTS][FAP4KE_GSO_CSUM_LIST_SIZE];
} __attribute__((aligned(4))) fap4keGso_iopDmaCmd_t;

typedef struct {
    uint32 packets;
    uint32 bytes;
    uint32 outOfMem;
    uint32 txDropped;
} fap4keGso_stats_t;

typedef struct {
    uint8 header[FAP4KE_GSO_HEADER_SIZE];
    fap4keGso_iopDmaCmd_t iopDmaCmd[IOPDMA_DMA_DEPTH];
    fapGsoDesc_t GsoDesc_p;
    fap4keGso_stats_t stats;
    unsigned int ipv6Id;
} fap4keGso_shared_t;

void fap4keGso_init(void);

int fap4keGso_start(fap4kePkt_gso_pkt *pGsoPkt);
int fap4keGso_fragXmit(fap4kePkt_gso_pkt *pGsoPkt);
int freeTxBuffersGso(fap4kePkt_phy_t phy, uint8 *txAddr, uint32 key, int source, int rxChannel);
void fap4keGso_freeGsoBuffer(uint8 *gsoBuffer_p);

#if defined(CONFIG_BCM963268) && defined(CONFIG_BCM_EXT_SWITCH)
int fap4keGso_checksum(uint8 *packet_p, int len, uint32 encapType, uint8 isExtSwitch);
#else
int fap4keGso_checksum(uint8 *packet_p, int len, uint32 encapType);
#endif

#endif  /* defined(__FAP4KE_GSO_H_INCLUDED__) */
