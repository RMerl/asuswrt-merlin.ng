#ifndef __FAP_HW_63268_H_INCLUDED__
#define __FAP_HW_63268_H_INCLUDED__

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
 * File Name  : fap_hw_63268.h
 *
 * Description: This file contains 63268 specific hardware changes for FAP.
 *
 *******************************************************************************
 */

#include <bcm_intr.h>
#include "bcmtypes.h"
#include <63268_map_part.h>

#define NUM_FAPS 2

/* Testing against INVALID_FAP_IDX is not sufficient in many cases.  Using
   this is better */
#define isValidFapIdx(fapIdx)   ((uint32)fapIdx < NUM_FAPS)

// Comment out and rebuild all FAP files to get a log of every register access
//#define FAP_DEBUG_MODE

#if defined(FAP_DEBUG_MODE)
#if defined(FAP_4KE)
// Use these macros to access registers from code running on the 4ke
// Note that printing register access from 4ke is currently not possible
// since the prints would use the mailbox, which require register accesses...
#define FAP_4KE_REG_RD(_reg)                    FAP_ADDR_RD(&(_reg))
#define FAP_4KE_REG_WR(_reg, _val32)            FAP_ADDR_WR(&(_reg), _val32)
#else /* FAP_4KE */
// Use these macros to access registers from code running on the host
#define FAP_HOST_REG_RD(_reg)                   fapHostAddrRead((&(_reg)), #_reg)
#define FAP_HOST_REG_WR(_reg, _val32)           fapHostAddrWrite(&(_reg), (_val32), #_reg)
#endif /* else FAP_4KE */
#else /* FAP_DEBUG_MODE */
#if defined(FAP_4KE)
// Use these macros to access registers from code running on the 4ke
#define FAP_4KE_REG_RD(_reg)                    FAP_ADDR_RD(&(_reg))
#define FAP_4KE_REG_WR(_reg, _val32)            FAP_ADDR_WR(&(_reg), _val32)
#else /* FAP_4KE */
// Use these macros to access registers from code running on the host
#define FAP_HOST_REG_RD(_reg)                   FAP_ADDR_RD(&(_reg))
#define FAP_HOST_REG_WR(_reg, _val32)           FAP_ADDR_WR(&(_reg), _val32)
#endif /* else FAP_4KE */
#endif /* else FAP_DEBUG_MODE */

#define hwPrint(fmt, arg...)    printk(fmt, ##arg)
// Use these macros to print registers from code running in the Host
#define FAP_PRINT_REG(grp, reg) hwPrint("  %-20s  0x%08x\n", #reg, (unsigned int)grp->reg)
#define FAP_PRINT_REG_ELEM(grp, reg) hwPrint("  %-20s  0x%08x\n", #reg, (unsigned int)grp.reg)

// Internal use only
#define FAP_ADDR_RD(_addr) *((volatile uint32 *)(_addr))
#define FAP_ADDR_WR(_addr, _val32) *((volatile uint32 *)(_addr)) = (_val32)

// FAP Address Offsets
#define CTRL_REG_BLOCK_OFFSET 0x0000
#define OG_MSG_OFFSET         0x0100
#define IN_MSG_OFFSET         0x0200
#define DMA_OFFSET            0x0300
#define TKN_INTF_OFFSET       0x0400
#define PERF_MEAS_OFFSET      0x0600
#define MSG_ID_OFFSET         0x0700
#define DQM_OFFSET            0x0800
#define DQMQCNTRL_OFFSET      0x0a00
#define DQMQDATA_OFFSET       0x0c00
#define DQMQMIB_OFFSET        0x1000
#define QSM_OFFSET            0x3000
#define DPE_BASIC_OFFSET      0x3F000

#define mPhysToNonCacheVirtAddr(x)   ((x&0x1fffffff)|0xa0000000)
#define mPhysToCacheVirtAddr(x)      ((x&0x1fffffff)|0x80000000)
#define mVirtToPhysAddr(x)           (x&0x1fffffff)
#define mCacheToNonCacheVirtAddr(x)  ( ((uint32)(x) & 0x1fffffff) | 0xa0000000 )

/* SMI Bus Control Register */
#define USE_SMISBUS

#define MIPS_SMISB_CTRL         0xFF400030
#define FAP_SMISB_CTRL_VAL      0x10C00001
#define mips_smisb_ctrl         ((volatile uint32 *)(MIPS_SMISB_CTRL))

#ifdef USE_SMISBUS
#define PHYS_FAP_BASE_0           0x10c01000 /* 63268 using SMISBUS */
#define PHYS_FAP_BASE_1           0x10e01000 /* 63268 using SMISBUS */
#else
#define PHYS_FAP_BASE_0           FAP0_CNTRL_BASE /* 63268 using UBUS    */
#define PHYS_FAP_BASE_1           FAP1_CNTRL_BASE /* 63268 using UBUS    */
#endif /* USE_SMISBUS */

static const uint32 PHYS_FAP_BASES[]           = {PHYS_FAP_BASE_0, PHYS_FAP_BASE_0};
static const uint32 SOFT_RST_FAPS[]            = {SOFT_RST_FAP0, SOFT_RST_FAP1};
static const uint32 FAP_CLK_ENS[]              = {FAP0_CLK_EN, FAP1_CLK_EN};
static const uint32 FAP_PLL_CLKENS[]           = {FAP1_PLL_CLKEN, FAP2_PLL_CLKEN};
static const uint32 FAP_INTERRUPT_ENS[]        = {INTERRUPT_ID_FAP_0, INTERRUPT_ID_FAP_1};
/* FAP_TBD: magic numbers??? */
static const uint32 FAP_INTERRUPT_IRQS[]       = { 32, 33 };



#define NON_CACHE_VIRT_FAP_BASE_0   mPhysToNonCacheVirtAddr(PHYS_FAP_BASE_0)
#define NON_CACHE_VIRT_FAP_BASE_1   mPhysToNonCacheVirtAddr(PHYS_FAP_BASE_1)
static const uint32 NON_CACHE_VIRT_FAP_BASES[] = { NON_CACHE_VIRT_FAP_BASE_0, NON_CACHE_VIRT_FAP_BASE_1 };

#if !defined(FAP_4KE)
/* Registers for Host Access */
#define hostRegCntrl(fapIdx)         ((volatile CoprocCtlRegs_S *)(NON_CACHE_VIRT_FAP_BASES[fapIdx] + CTRL_REG_BLOCK_OFFSET))
#define hostOgMsgReg(fapIdx)         ((volatile OGMsgFifoRegs_S *)(NON_CACHE_VIRT_FAP_BASES[fapIdx] + OG_MSG_OFFSET))
#define hostInMsgReg(fapIdx)         ((volatile INMsgFifoRegs_S *)(NON_CACHE_VIRT_FAP_BASES[fapIdx] + IN_MSG_OFFSET))
#define hostDmaReg(fapIdx)           ((volatile DMARegs_S *)(NON_CACHE_VIRT_FAP_BASES[fapIdx] + DMA_OFFSET))
#define hostTknIntfReg(fapIdx)       ((volatile TknIntfRegs_S *)(NON_CACHE_VIRT_FAP_BASES[fapIdx] + TKN_INTF_OFFSET))
#define hostPerfMeasReg(fapIdx)      ((volatile PMRegs_S *)(NON_CACHE_VIRT_FAP_BASES[fapIdx] + PERF_MEAS_OFFSET))
#define hostMsgIdReg(fapIdx)         ((volatile MsgIdRegs_S *)(NON_CACHE_VIRT_FAP_BASES[fapIdx] + MSG_ID_OFFSET))
#define hostDqmReg(fapIdx)           ((volatile DQMCtlRegs_S *)(NON_CACHE_VIRT_FAP_BASES[fapIdx] + DQM_OFFSET))
#define hostDqmQCntrlReg(fapIdx)     ((volatile DQMQCntrlRegs_S *)(NON_CACHE_VIRT_FAP_BASES[fapIdx] + DQMQCNTRL_OFFSET))
#define hostDqmQDataReg(fapIdx)      ((volatile DQMQDataRegs_S *)(NON_CACHE_VIRT_FAP_BASES[fapIdx] + DQMQDATA_OFFSET))
#define hostDqmQMibReg(fapIdx)       ((volatile DQMQMibRegs_S *)(NON_CACHE_VIRT_FAP_BASES[fapIdx] + DQMQMIB_OFFSET))
//#define hostDpeBasicReg(fapIdx)      ((volatile DpeBasicRegs_S *)(NON_CACHE_VIRT_FAP_BASES[fapIdx] + DPE_BASIC_OFFSET))
#endif /* !FAP_4KE */


static const uint32 FAP_HOST_PSM_BASES[] = {FAP0_PSM_BASE, FAP1_PSM_BASE};

/* Place Enet Rx Bds at upper end of 16KB QSM. 400 BDs * 2W per BD * 2 ch * 4bytes per word */
#ifdef USE_SMISBUS
#define FAP0_HOST_QSM_BASE FAP0_QSM_SMI_BASE
#define FAP1_HOST_QSM_BASE FAP1_QSM_SMI_BASE
#else /* USE_SMISBUS */
#define FAP0_HOST_QSM_BASE FAP0_QSM_UBUS_BASE
#define FAP1_HOST_QSM_BASE FAP1_QSM_UBUS_BASE
#endif /* USE_SMISBUS */
static const uint32 FAP_HOST_QSM_BASES[] = {FAP0_HOST_QSM_BASE, FAP1_HOST_QSM_BASE};

// 4ke FAP Base Address
#define _4KE_BASE    0xe0001000

// Registers for 4ke Access
#define _4keRegCntrl          ((volatile CoprocCtlRegs_S *)(_4KE_BASE + CTRL_REG_BLOCK_OFFSET))
#define _4keOgMsgReg          ((volatile OGMsgFifoRegs_S *)(_4KE_BASE + OG_MSG_OFFSET))
#define _4keInMsgReg          ((volatile INMsgFifoRegs_S *)(_4KE_BASE + IN_MSG_OFFSET))
#define _4keDmaReg            ((volatile DMARegs_S *)(_4KE_BASE + DMA_OFFSET))
#define _4keTknIntfReg        ((volatile TknIntfRegs_S *)(_4KE_BASE + TKN_INTF_OFFSET))
#define _4kePerfMeasReg       ((volatile PMRegs_S *)(_4KE_BASE + PERF_MEAS_OFFSET))
#define _4keMsgIdReg          ((volatile MsgIdRegs_S *)(_4KE_BASE + MSG_ID_OFFSET))
#define _4keDqmReg            ((volatile DQMCtlRegs_S *)(_4KE_BASE + DQM_OFFSET))
#define _4keDqmQCntrlReg      ((volatile DQMQCntrlRegs_S *)(_4KE_BASE + DQMQCNTRL_OFFSET))
#define _4keDqmQDataReg       ((volatile DQMQDataRegs_S *)(_4KE_BASE + DQMQDATA_OFFSET))
#define _4keDqmQMibReg        ((volatile DQMQMibRegs_S *)(_4KE_BASE + DQMQMIB_OFFSET))
#define _4keSharedMemPtr      ((volatile uint32 *)(_4KE_BASE + SHARED_MEM_OFFSET))
#define _4keDpeBasicReg       ((volatile DpeBasicRegs_S *)(_4KE_BASE + DPE_BASIC_OFFSET))
#define _4keMiscReg           ((volatile Misc *)(MISC_BASE))
#define _4kePerfControl  ((PerfControl * )(PERF_BASE))

// FAP QSM Registers
#define FAP_4KE_QSM_BASE                               0xE0004000  /* 4ke QSM base addr */
#define FAP_QSM_SIZE                                   0x10000  /* 64K */

// FAP PSM Registers
#define FAP_4KE_PSM_BASE_NONIOP                        0x80002000 /* Base addr of PSM port connected to the 4ke */
#define FAP_4KE_PSM_BASE_IOP                           0xE0020000 /* Base addr of PSM port connected to the IOP */
#define FAP_4KE_PSM_BASE                               FAP_4KE_PSM_BASE_NONIOP
#define FAP_PSM_SIZE                                   (0x6000) /* 24K */
#define FAP_PSM_SIZE_32                                (FAP_PSM_SIZE / 4) /* words */

#define FAP_4KE_PSM_NONIOP_TO_IOP(x)                   ( (void *)(((uint32)(x) - FAP_4KE_PSM_BASE_NONIOP) \
                                                                  + FAP_4KE_PSM_BASE_IOP) )
// FAP DPE Memory Base Addresses
#define FAP_DPE_NUG_MEM_BASE                           0xE0050000
#define FAP_DPE_PKT_MEM_BASE                           0xE0051000
#define FAP_DPE_INST_MEM_BASE                          0xE0052000

#define CONVERT_PSM_HOST2FAP(x)                        (((uint32)x & 0xFFFF) + FAP_4KE_PSM_BASE)

#define CONVERT_QSM_HOST2FAP(fapIdx, x)                (((uint32)x - (uint32)FAP_HOST_QSM_BASES[fapIdx]) | FAP_4KE_QSM_BASE)

#define FAP4KE_OUT_OF_PSM                              ((uint8 *)0)


/* The following is used for debugging: */
typedef enum {
  FAP_ALL_REG,
  FAP_CNTRL_REG,
  FAP_OGMSG_REG,
  FAP_INMSG_REG,
  FAP_DMA_REG,
  FAP_TKNINTF_REG,
  FAP_MSGID_REG,
  FAP_DQM_REG,
  FAP_DQMQCNTRL_REG,
  FAP_DQMQDATA_REG,
  FAP_DQMQMIB_REG,
  FAP_DPE_REG
} fapRegGroups_t;

void fapHostHw_PrintRegs(uint32 fapIdx, fapRegGroups_t regGroup);

#if !defined(FAP_4KE)
/* only available on host */
#if defined(BCM_ASSERT_SUPPORTED)
#define FAP_HOST_ASSERT(x)   BCM_ASSERT(x);
#else
#define FAP_HOST_ASSERT(x)
#endif /* BCM_ASSERT_SUPPORTED */

#ifndef HalDebugPrint
#define HalDebugPrint(zone, fmt, arg...)
#endif /* HalDebugPrint */

#if defined(BCM_LOG_SUPPORTED)
#define FAP_HOST_LOG(fmt, arg...) BCM_LOG_DEBUG(BCM_LOG_ID_FAP, fmt, ##arg)
#else
#define FAP_HOST_LOG(fmt, arg...) HalDebugPrint(ZONE_INIT, fmt "\n", ##arg)
#endif /* BCM_LOG_SUPPORTED */

#if defined(FAP_DEBUG_MODE)
// Use macros FAP_REG_RD and FAP_REG_WR instead of directly using these
static inline uint32 fapHostAddrRead(volatile uint32 *addr, char *addrName)
{
    uint32 val;

    FAP_HOST_ASSERT(addr != NULL);

    val = FAP_ADDR_RD(addr);

    FAP_HOST_LOG(" [FAP HOST RD @ 0x%08lX] %s = 0x%08lX", (uint32)(addr), addrName, val);

    return val;
}

static inline void fapHostAddrWrite(volatile uint32 *addr, uint32 val, char *addrName)
{
    FAP_HOST_ASSERT(addr != NULL);

    FAP_ADDR_WR(addr, val);

    FAP_HOST_LOG("[FAP HOST WR @ 0x%08lX] %s = 0x%08lX", (uint32)(addr), addrName, val);
}
#endif /* FAP_DEBUG_MODE */

extern uint8 * fapDrv_psmAlloc( uint32 fapIdx, int size );

/* only usable by host... */
#define NUM_4KE_ADDRESS_TRANSLATORS 2
typedef struct _Fap_FapInfo
{
    uint32      blockHostAddr[NUM_4KE_ADDRESS_TRANSLATORS];         /* host address of allocated
                                                                   memory block */
    uint32      blockSize[NUM_4KE_ADDRESS_TRANSLATORS];         /* size of allocated memory
                                                                   block */
    uint32      blockMask[NUM_4KE_ADDRESS_TRANSLATORS];         /* mask of Block */
    uint32      blockFapAddr[NUM_4KE_ADDRESS_TRANSLATORS];      /* 4ke Addr of Block */

    uint32      pBase;          /* address at which 4KE registers are available */

    uint32      mainBlock;      /* address block in which entry function exists */
    uint32      mainOffset;     /* offset from begining of block at which to call main */

    char *      pPrintBuffer;   /* host address of print buffer */
    void *      pSdram;         /* host address of sdram */

    uint32      lastRxAlivePktJiffies;

    void *      pPsm;           /* host address of PSM */
    void *      pQsm;           /* host address of QSM */

    char        cacheName[16];  /* kernel name of cache */
} Fap_FapInfo;

/* defined in fap_hwinit.c */
extern Fap_FapInfo gFap[NUM_FAPS];

typedef struct _Fap_FapSharedInfo
{
    // need to define this even if it is not used
} Fap_FapSharedInfo;


extern Fap_FapSharedInfo gFaps;


#define pHostFapPrintBuffer(fapIdx)            ((char *)(gFap[fapIdx].pPrintBuffer))
#define pHostFapSdram(fapIdx)                  ((fap4keSdram_alloc_t * )(gFap[fapIdx].pSdram))
#define pHostPsm(fapIdx)                       ( (fap4kePsm_alloc_t *)(FAP_HOST_PSM_BASES[fapIdx]) )
#define pHostPsmGbl(fapIdx)                    ( &pHostPsm(fapIdx)->global )
#define pHostPsmBpm(fapIdx)                    ( &pHostPsmGbl(fapIdx)->bpm )
#define pHostTrace(fapIdx)                     ( &pHostQsmGbl(fapIdx)->trace )
#define pHostQsm(fapIdx)                       ( (fap4keQsm_alloc_t *)(FAP_HOST_QSM_BASES[fapIdx]) )
#define pHostQsmGbl(fapIdx)                    ( &pHostQsm(fapIdx)->global )
#define pHostFlowInfoPool(fapIdx)              ( pHostFapSdram(fapIdx)->packet.flowInfoPool )

static __inline uint32 getFapIdxFromFapIrq(int irq)
{
	if(FAP_INTERRUPT_IRQS[0] == irq)
		return( 0 );
	else if(FAP_INTERRUPT_IRQS[1] == irq)
		return( 1 );
	else
		return( 0x0DEADFAB /* FAP_INVALID_IDX */ );
}

#endif /* !FAP_4KE */

#ifdef FAP_4KE

#define fap4ke_readCp0StatusReg_priv()                                       \
__extension__                                                           \
({                                                                      \
    uint32 __cp0StatusReg;                                              \
                                                                        \
    __asm__ volatile ("mfc0 %0, $15, 1" : "=d" (__cp0StatusReg));       \
                                                                        \
    /* This causes the value in __cp0StatusReg to be returned from   */ \
    /* the macro as if it was the return value from a function.      */ \
    /*return*/ __cp0StatusReg;                                          \
})

static inline uint32 getFapIdx(void)
{
    return ((fap4ke_readCp0StatusReg_priv() & 0x3FF)==0x003?1:0);
}

#endif /* FAP_4KE */

#endif  /* defined(__FAP_HW_63268_H_INCLUDED__) */
