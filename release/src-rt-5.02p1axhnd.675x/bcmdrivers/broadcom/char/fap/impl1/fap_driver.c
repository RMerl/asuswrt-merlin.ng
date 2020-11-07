/*
<:copyright-BRCM:2009:proprietary:standard

   Copyright (c) 2009 Broadcom 
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

/*
 *******************************************************************************
 * File Name  : fapDriver.c
 *
 * Description: This file contains Linux character device driver entry points
 *              for the BCM63268 FAP Driver.
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/bcm_log.h>
#include <linux/sysrq.h>
#include <linux/kthread.h>
#include <linux/bcm_realtime.h>
#include "fap.h"
#include "fap_hw.h"
#include "fap_task.h"
#include "fap_dqm.h"
#include "fap_dqmHost.h"
#include "fap_local.h"
#include "fap4ke_local.h"
#include "fap4ke_init.h"
#include "bcmPktDma.h"
#include "bcmPktDmaHooks.h"
#include "fap4ke_memory.h"
#include "fap4ke_msg.h"
#include "fap_packet.h"
#include "fap_protocol.h"
#include "fap_tm.h"
#include "fap_swq.h"
#include <board.h>
#include "robosw_reg.h"

#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
#include <linux/gbpm.h>
#include <bpm.h>
#endif

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#include <linux/iqos.h>
#include <ingqos.h>
#endif

#include <linux/sysrq.h>

#include "fap4ke_dynmem_shared.h"
#include "fap_dynmem_host.h"

#include <linux/sysrq.h>

#define DODEBUG(val6)  do { \
        pHostFapSdram(fapIdx)->dbgVals[5] = __LINE__; \
        pHostFapSdram(fapIdx)->dbgVals[6] = val6; \
    } while (0)

#define SETDEBUG(idx,val)  do { \
        pHostFapSdram(fapIdx)->dbgVals[7] = __LINE__; \
        pHostFapSdram(fapIdx)->dbgVals[idx] = (uint32)val; \
    } while (0)


#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
static DEFINE_SPINLOCK(fapHostIf_lock_g);
#else
static spinlock_t fapHostIf_lock_g = SPIN_LOCK_UNLOCKED;
#endif

#define FAP_HOSTIF_LOCK(flags)   spin_lock_irqsave(&fapHostIf_lock_g, flags)
#define FAP_HOSTIF_UNLOCK(flags) spin_unlock_irqrestore(&fapHostIf_lock_g, flags)
#else
#define FAP_HOSTIF_LOCK(flags)   local_irq_save(flags)
#define FAP_HOSTIF_UNLOCK(flags) local_irq_restore(flags)
#endif

#undef FAP_DECL
#define FAP_DECL(x) #x,

#if defined(CC_CONFIG_FCACHE_DEFER)
extern int fcache_defer;
#endif

#if defined(CC_FAP4KE_PKT_GSO)
#define pHostGso(_fapIdx) ( &pHostQsmGbl(_fapIdx)->gso )
#endif

static int timer_pid = -1;
extern struct task_struct *fapDrvTask;
void (*wfd_dump_fn)(void) = 0;
EXPORT_SYMBOL(wfd_dump_fn);
SWQInfo_t wfdF2HQInfo[WFD_NUM_QUEUE_SUPPORTED];
EXPORT_SYMBOL(wfdF2HQInfo);

static const char *fapIoctlName[] =
{
    FAP_DECL(FAP_IOC_HW)
    FAP_DECL(FAP_IOC_STATUS)
    FAP_DECL(FAP_IOC_INIT)
    FAP_DECL(FAP_IOC_ENABLE)
    FAP_DECL(FAP_IOC_DISABLE)
    FAP_DECL(FAP_IOC_DEBUG)
    FAP_DECL(FAP_IOC_PRINT)
    FAP_DECL(FAP_IOC_CPU)
    FAP_DECL(FAP_IOC_SWQ)
    FAP_DECL(FAP_IOC_DMA_DEBUG)
    FAP_DECL(FAP_IOC_MEM_DEBUG)
    FAP_DECL(FAP_IOC_MTU)
    FAP_DECL(FAP_IOC_TM)
    FAP_DECL(FAP_IOC_DM_DEBUG)
    FAP_DECL(FAP_IOC_FLOODING_MASK)
    FAP_DECL(FAP_IOC_ARL_FLUSH)
    FAP_DECL(FAP_IOC_ARL_SHOW)
    FAP_DECL(FAP_IOC_DO_4KE_TEST)
#if defined(CONFIG_BCM_GMAC)
    FAP_DECL(FAP_IOC_INIT_IUDMA)
    FAP_DECL(FAP_IOC_UNINIT_IUDMA)
#endif
    FAP_DECL(FAP_IOC_MAX)
};

static DECLARE_COMPLETION(wdtimer_done);
void fapAlive_watchdog( void );

static DECLARE_COMPLETION(fapDelayedAccess_done);

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
extern iqos_fap_ethRxDqmQueue_hook_t iqos_fap_ethRxDqmQueue_hook_g;
extern iqos_fap_xtmRxDqmQueue_hook_t iqos_fap_xtmRxDqmQueue_hook_g;
#endif

#if defined(CC_FAP4KE_PERF) && (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
void __init fapPerf_construct(void);
void __exit fapPerf_destruct(void);
#endif

typedef struct {
    uint32 fap2HostHostIf;
    uint32 host2FapHostIf;
} fapDrv_dqmStats_t;

extern fapPkt_flowAlloc_t flowAlloc_g[NUM_FAPS][FAP4KE_PKT_MAX_FLOWS];

#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
#define FAP_MAX_BPM_EVT 32

/* Hook is instantiated in net/core/dev.c, and not part of gbpm_g */
extern gbpm_fap_evt_hook_t gbpm_fap_evt_hook_g;

/* BPM Events pendig queue */
typedef struct
{
    uint32 headIdx;
    uint32 tailIdx;
    DQMQueueDataReg_S msg[FAP_MAX_BPM_EVT];
} bpmPendEvtQ_t;

bpmPendEvtQ_t bpmPendEvtQ_g;
bpmPendEvtQ_t *bpmPendEvtQ_gp = &bpmPendEvtQ_g;

static fapDrv_dqmStats_t fapDrvDqmStats[NUM_FAPS];

typedef struct {
    uint32 rdDqm;
    uint32 wrDqm;
    uint32 rdEvt;
    uint32 wrEvt;
} fapDrv_bpmStats_t;

static fapDrv_bpmStats_t fapDrvBpmStats[NUM_FAPS];

void fapBpm_eventQ_init( void );
int fapBpm_eventQ_put( DQMQueueDataReg_S msg );

/* various functions provided by FAP for the hooks exported by BPM */
void fapBpm_dumpStatus( void );
void fapBpm_dumpTxQThresh( void );
void fapBpm_dumpEthTxQThresh( void );
void fapBpm_updBufLvl( int lvl );

void fapBpm_allocBufResp_Dqm( uint8 drv, uint8 channel, uint8 seqId,
            uint16 numBufs);
void fapBpm_freeBufResp_Dqm( uint8 fapIdx, uint8 seqId, uint16 numBufs );

/* processes an event from BPM event pending queue */
void fapBpm_schedEvt(void);
#endif

#if defined(CC_FAP4KE_PMON)
static const char *pmonIdName[] = FAP4KE_PMON_ID_NAME;
#endif

#if defined(CC_FAP4KE_TRACE)
static char *fap4keTraceTypeName[] = FAP4KE_TRACE_TYPE_NAME;
#endif

#define FAP_DELAYED_MSG_MAX 8

typedef struct{
    uint32 msgId;
    uint32 fapIdx;
    fapMsgGroups_t msgType;
    xmit2FapMsg_t msg;
    uint32 hz;
    uint32 isInUse;
} fap_delayedMsgEntry_t;

typedef struct{
    int write;
    fap_delayedMsgEntry_t entry[FAP_DELAYED_MSG_MAX];
} fap_delayedMsg_t;

static fap_delayedMsg_t delayedMsg_g;

#if defined(CONFIG_BCM_FAP_GSO)
#define FAP_MAX_GSODESC 1024 

static DEFINE_SPINLOCK(fapGsoDescPool_lock);
fapGsoDesc_t *fapGsoDescPool=NULL;

void free_fapGsoDesc(fapGsoDesc_t *gsoDesc_p)
{
    if(gsoDesc_p)
    {
        spin_lock_bh(&fapGsoDescPool_lock);
        gsoDesc_p->isAllocated = 0;
        spin_unlock_bh(&fapGsoDescPool_lock);
    }
}
EXPORT_SYMBOL(free_fapGsoDesc);

fapGsoDesc_t *alloc_fapGsoDesc(void)
{
    static int allocIndex = 0;
    int i=0;
    fapGsoDesc_t *gsoDesc_p;

    spin_lock_bh(&fapGsoDescPool_lock);
    while(i < FAP_MAX_GSODESC)
    {
        if(allocIndex == FAP_MAX_GSODESC)
            allocIndex = 0;

        gsoDesc_p = &fapGsoDescPool[allocIndex];
        if(!gsoDesc_p->isAllocated)
        {
            //memset(gsoDesc_p, 0, sizeof(fapGsoDesc_t));

            gsoDesc_p->nr_frags = 0;
            gsoDesc_p->recycle_key = 0;
            gsoDesc_p->flags = 0;
            gsoDesc_p->mss = 0;

            gsoDesc_p->isAllocated = 1;
            allocIndex++;
            spin_unlock_bh(&fapGsoDescPool_lock);
            return gsoDesc_p;
        }
        allocIndex++;
        i++;
    }
    spin_unlock_bh(&fapGsoDescPool_lock);
    return NULL;
}
EXPORT_SYMBOL(alloc_fapGsoDesc);


#if defined(CONFIG_BCM_FAP_GSO_LOOPBACK)

extern void fapGsoLoopBk_uninit(void);
extern int fapGsoLoopBk_init(void);
extern void fapGsoLoopBk_dumpSWQs(uint32 fapId);
#endif

#endif

void fap_wfd_dumpSWQs(uint32 fapId)
{
    int qidx;

    printk("SWQ: FAP2HOST_WFD fapIdx %d\n", (int)fapId);
    for (qidx = 0; qidx < WFD_NUM_QUEUE_SUPPORTED / NUM_FAPS; qidx++)
    {
        swqDumpHost(&pHostPsmGbl(fapId)->wfdF2HSwq[qidx]);
    }
    if (wfd_dump_fn)
    {
        wfd_dump_fn();
    }
}

/*
 *------------------------------------------------------------------------------
 * Function Name: __fapDelayedAccess
 * Description  : Kernel thread that provides delayed messaging to a FAP.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
static int __fapDelayedAccess(void *delayedMsgEntry_p)
{
    static uint32 msgId = 0;
    fap_delayedMsgEntry_t *entry_p = (fap_delayedMsgEntry_t *)delayedMsgEntry_p;

    /*
     * This thread doesn't need any user-level access,
     * so get rid of all our resources
     */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
    daemonize("__fapDelayedAccess");
#endif

    set_current_state(TASK_INTERRUPTIBLE);

    /*  Sleep for 0.5 sec */
    schedule_timeout(entry_p->hz);

    /* Do work */

#if 0
    if(msgId != entry_p->msgId)
    {
        printk("FAP delayed message overrun: %lu, %lu\n", msgId, entry_p->msgId);

        msgId = entry_p->msgId;
    }
#endif

    msgId++;

    fapDrv_Xmit2Fap(entry_p->fapIdx, entry_p->msgType, &entry_p->msg);
    entry_p->isInUse = FALSE;

    complete_and_exit(&fapDelayedAccess_done, 0);
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fapDrv_Xmit2FapDelayed
 * Description  : Provides NON-GUARANTEED, delayed messaging to a FAP.
 *                IT IS POSSIBLE THAT A DELAYED MESSAGE WILL BE LOST.
 * Returns      : 0 on success.
 *------------------------------------------------------------------------------
 */
int fapDrv_Xmit2FapDelayed( uint32 fapIdx, fapMsgGroups_t msgType,
                             xmit2FapMsg_t *pMsg, uint32 hz )
{
    static int firstTime = 1;
    static uint32 msgId = 0;
    fap_delayedMsgEntry_t *entry_p;
    unsigned long flags;
    int rt;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
    struct task_struct * task;
#endif

    if(firstTime)
    {
        firstTime = 0;
        memset(&delayedMsg_g, 0, sizeof(fap_delayedMsg_t));
    }

    FAP_HOSTIF_LOCK(flags);

    entry_p = &delayedMsg_g.entry[delayedMsg_g.write];

    if (entry_p->isInUse)
    {
        FAP_HOSTIF_UNLOCK(flags);
        return -1;
    }
    entry_p->isInUse = TRUE;

    if(delayedMsg_g.write == FAP_DELAYED_MSG_MAX-1)
    {
        delayedMsg_g.write = 0;
    }
    else
    {
        delayedMsg_g.write++;
    }

    entry_p->msgId = msgId++;
    entry_p->fapIdx = fapIdx;
    entry_p->msgType = msgType;
    memcpy(&entry_p->msg, pMsg, sizeof(xmit2FapMsg_t));
    entry_p->hz = hz;

    FAP_HOSTIF_UNLOCK(flags);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
    task = kthread_run(__fapDelayedAccess, (void *) entry_p, "__fapDelayedAccess" );
    rt = ( IS_ERR(task) ? -1 : task->pid );
#else
    rt = kernel_thread(__fapDelayedAccess, entry_p, CLONE_KERNEL);
#endif
    return rt;
    
}


static __inline void *
convertFapToSdram(uint32 fapIdx, void *fapAddr)
{
    fapAddr = (void *)((uint32)fapAddr & ~0x80000000);
    if ( ((uint32)fapAddr & ~(uint32)gFap[fapIdx].blockMask[0]) == (uint32)gFap[fapIdx].blockFapAddr[0]) { 
        /* address translator 0: */ 
        return (void *)(((uint32)fapAddr & gFap[fapIdx].blockMask[0]) + gFap[fapIdx].blockHostAddr[0]);
    } else if ( ((uint32)fapAddr & ~(uint32)gFap[fapIdx].blockMask[1]) == (uint32)gFap[fapIdx].blockFapAddr[1]) { 
        /* address translator 1: */ 
        return (void *)(((uint32)fapAddr & gFap[fapIdx].blockMask[1]) + gFap[fapIdx].blockHostAddr[1]); 
    }
    return fapAddr;
}


/* FIXME: This function is broken! */
static void fapPrint4keTrace(uint32 fapIdx)
{
#if defined(CC_FAP4KE_TRACE)
    fap4keTrace_record_t *record_p;
    uint32_t read;
    uint32_t count;
    uint32_t currCycles=0, prevCycles, startCycles;
    uint32_t elapsed, delta;
    int i;

    pHostTrace(fapIdx)->enable = 0;

    read = pHostTrace(fapIdx)->history.write;
    if(read == FAP4KE_TRACE_HISTORY_SIZE-1)
    {
        read = 0;
    }
    else
    {
        read++;
    }

    count = pHostTrace(fapIdx)->history.count;
    if(count)
    {
        count--;
    }

    startCycles = currCycles = pHostTrace(fapIdx)->history.record[read].cycles;

    for(i=0; i<count; ++i)
    {
        record_p = &pHostTrace(fapIdx)->history.record[read];

        prevCycles = currCycles;

        currCycles = record_p->cycles;

        elapsed = (currCycles-startCycles) * 2 * FAP4KE_MIPS_CLK_PERIOD_NSEC;
        delta = (currCycles-prevCycles) * 2 * FAP4KE_MIPS_CLK_PERIOD_NSEC;

        printk("[%03u] %s\t: ", read, fap4keTraceTypeName[record_p->id]);
        switch(record_p->type)
        {
            case FAP4KE_TRACE_TYPE_DEC:
                printk("%d, \t\t\t", record_p->arg);
                break;
            case FAP4KE_TRACE_TYPE_HEX:
                printk("0x%X, \t\t\t", record_p->arg);
                break;
            case FAP4KE_TRACE_TYPE_STR:
                {
                    char *str = (char *)convertFapToSdram(fapIdx, (void *)record_p->arg);
                    printk("%s, ", str);
                   
                }
                break;
            default:
                printk("ERROR, ");
                break;
        }
        printk("Elapsed %u ns, Delta %u ns (%u cycles)\n",
               elapsed, delta, delta/FAP4KE_MIPS_CLK_PERIOD_NSEC);

        if(read == FAP4KE_TRACE_HISTORY_SIZE-1)
        {
            read = 0;
        }
        else
        {
            read++;
        }
    }

    pHostTrace(fapIdx)->enable = 1;
#endif
}

static void fapPrint4keCpu(uint32 fapIdx)
{
#if defined(CC_FAP4KE_PM_CPU)
    uint32 index = pHostCpuHistory(fapIdx)->index;
    uint32 cpu;
    int i;

    printk(FAP_IDX_FMT "::: CPU Utilization ::: ", fapIdx);

    if(index >= FAP4KE_PM_CPU_HISTORY_MAX)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid Index <%d>", index);

        return;
    }

    for(i=0; i<FAP4KE_PM_CPU_HISTORY_MAX; ++i)
    {
        if(index == 0)
        {
            /* wrap-around */
            index = FAP4KE_PM_CPU_HISTORY_MAX - 1;
        }
        else
        {
            index--;
        }

        /* CP0 Count increments at every other clock */
        cpu = (pHostCpuHistory(fapIdx)->busy[index] * 2) / CC_FAP4KE_PM_CPU_PRECISION;

//                printk("cpu<%lu>", cpu);

        cpu *= 100;

        cpu /= (FAP4KE_MIPS_CLK_HZ / CC_FAP4KE_PM_CPU_PRECISION);

        printk("%u%% ", cpu);
    }

    printk("\n");
#endif
}

#if defined(CC_FAP4KE_PMON)
#define pHostPmon(_fapIdx) ( &pHostQsmGbl(_fapIdx)->pmon )
#endif

static void fapPrint4kePmon(uint32 fapIdx)
{
#if defined(CC_FAP4KE_PMON)
    int i;
    uint32 cycles;
    uint32 nsec;
    uint32 nsecAccum = 0;
    uint32 cyclesAccum = 0;
    uint32 instAccum = 0;
    uint32 iCacheHitsAccum = 0;
    uint32 iCacheMissesAccum = 0;
    uint32 irqAccum = 0;
    uint32 pps = 0;

    printk("\n::: PMON :::\n");

    for(i=0; i<FAP4KE_PMON_ID_MAX; ++i)
    {
        cycles = 2 * pHostPmon(fapIdx)->halfCycles[i];
        nsec = cycles * FAP4KE_MIPS_CLK_PERIOD_NSEC;
        printk("%35s : <%04ld> nsec, Cycles <%04ld>, Instr <%04ld>, "
               "I$ Hit <%04ld>, I$ Miss <%04ld>, IRQ <%04ld>\n",
               pmonIdName[i], nsec, cycles, pHostPmon(fapIdx)->instncomplete[i],
               pHostPmon(fapIdx)->icachehit[i], pHostPmon(fapIdx)->icachemiss[i],
               pHostPmon(fapIdx)->interrupts[i]);
        nsecAccum += nsec;
        cyclesAccum += cycles;
        instAccum += pHostPmon(fapIdx)->instncomplete[i];
        iCacheHitsAccum += pHostPmon(fapIdx)->icachehit[i];
        iCacheMissesAccum += pHostPmon(fapIdx)->icachemiss[i];
        irqAccum += pHostPmon(fapIdx)->interrupts[i];
    }

    pps = 1000000000 / (cyclesAccum * FAP4KE_MIPS_CLK_PERIOD_NSEC);

    printk("\n%35s : <%d> nsec, cycles <%d> / <%d> pps, instructions <%d>\n"
           "%37s icache hits <%d>, icache misses <%d>, irqs <%d>\n\n",
           "Totals", nsecAccum, cyclesAccum, pps, instAccum,
           " ", iCacheHitsAccum, iCacheMissesAccum, irqAccum);
#endif
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fapIoctl
 * Description  : Main entry point to handle user applications IOCTL requests.
 * Returns      : 0 - success or error
 *------------------------------------------------------------------------------
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
static long fapIoctl(struct file *filep, unsigned int command, unsigned long arg)
#else
int fapIoctl(struct inode *inode, struct file *filep, unsigned int command, unsigned long arg)
#endif
{
    fapIoctl_t cmd;
    int ret = FAP_SUCCESS;
    uint32 fapIdx;

    if (command >= FAP_IOC_MAX)
        cmd = FAP_IOC_MAX;
    else
        cmd = (fapIoctl_t)command;

    BCM_LOG_INFO(BCM_LOG_ID_FAP, "cmd<%d> %s arg<0x%08lX>",
                 command, fapIoctlName[command - FAP_IOC_HW], arg);

    switch( cmd )
    {
        case FAP_IOC_HW:
        {
           fapRegGroups_t hwGrp = (fapRegGroups_t) arg;
           for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
           {
                printk("FAP%u:\n", fapIdx);
                fapHostHw_PrintRegs(fapIdx, hwGrp);
           }
           break;
        }

        case FAP_IOC_STATUS:
            fapStatus();
            break;

        case FAP_IOC_INIT:
            break;

        case FAP_IOC_ENABLE:
            fapEnable();
            break;

        case FAP_IOC_DISABLE:
            fapDisable();
            break;

        case FAP_IOC_DEBUG:
            ret = fapDebug((int)arg);
            break;

        case FAP_IOC_PRINT:
            fapPrint((int16)(arg >> 16), (int16)(arg & 0xFFFF));
            break;

        case FAP_IOC_MTU:
            {
                uint32 fapIdx;
                int fapIdxParam = (arg & 0xF0000000) >> 28;
                int flowIdParam = (arg & 0x0FFF0000) >> 16;
                int mtuParam = (arg & 0x0000FFFF);
                xmit2FapMsg_t fapMsg;
                fap4kePkt_flowInfo_t *flowInfo_p;
                int flowId;

                if (flowIdParam == 0x0FFF)
                    flowIdParam = -1;
                if (fapIdxParam == 0xF)
                    fapIdxParam = -1;
                
                printk("Overriding FAP MTUs.  fapIdx=%d%s, flowId=%d%s, mtu=%d\n", 
                    fapIdxParam, fapIdxParam==-1?"(ALL)":"",
                    flowIdParam, flowIdParam==-1?"(ALL)":"",
                    mtuParam);


                if (fapIdxParam < -1 || fapIdxParam >= NUM_FAPS)
                {
                    printk("fapIdx out of range\n");
                    ret = FAP_ERROR;
                    break;                    
                }

                if (mtuParam != 0 && (mtuParam < MIN_FAP_MTU || mtuParam >= MAX_FAP_MTU))
                {
                    printk("mtu out of range\n");
                    ret = FAP_ERROR;
                    break;                    
                }
                
                for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
                {
                    if (fapIdxParam == -1 || fapIdx == fapIdxParam)
                    {      
                        memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));
                        fapMsg.mtu.word[0] = fapIdxParam;
                        fapMsg.mtu.word[1] = flowIdParam;
                        fapMsg.mtu.word[2] = mtuParam;
                        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_SET_MTU, &fapMsg);
                        
                        /* updating host copy of flow as well */

                        if (flowIdParam != -1)
                        {
                            flowInfo_p = &pHostFlowInfoPool(fapIdx)[flowIdParam];
                            flowInfo_p->fapMtu = mtuParam;             
                        }
                        else
                        {                            
                            pHostPsmGbl(fapIdx)->mtuOverride = mtuParam;
                            for (flowId = 0; flowId < FAP4KE_PKT_MAX_FLOWS; flowId++)
                            {
                                flowInfo_p = &pHostFlowInfoPool(fapIdx)[flowId];
                                flowInfo_p->fapMtu = mtuParam;             
                            }
                        }
                    }
                }
                
            }
            break;

#if defined(CC_FAP4KE_TM)
        case FAP_IOC_TM:
        {
            ret = fapTm_ioctl(arg);

            break;
        }
#else
        case FAP_IOC_TM:
        {
            printk("FAP Traffic Management is not available\n");

            break;
        }
#endif
        case FAP_IOC_DM_DEBUG:
            {
                int fapIdxParam = (arg & 0xF0000000) >> 28;
                int typeParam = (arg & 0x00FF0000) >> 16;
                int valParam = (arg & 0x0000FFFF);
                
                if (fapIdxParam < -1 || fapIdxParam >= NUM_FAPS)
                {
                    printk("fapIdx out of range\n");
                    ret = FAP_ERROR;
                    break;                    
                }
                fapDm_debug(fapIdxParam,(fapDm_DebugType)typeParam, valParam);
                break;

            }
        case FAP_IOC_DO_4KE_TEST:
        {
            xmit2FapMsg_t fapMsg;
            int i;
            int fapIdx;
   
            for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++) {
                for (i = 0; i < FAP4KE_PKT_MAX_FLOWS; i++) {
                    if ( flowAlloc_g[fapIdx][i].isAlloc || flowAlloc_g[fapIdx][i].isHostValid )
                    {
                        printk("[fap%d.flow%d]: %s:%s \n", 
                            fapIdx, i, flowAlloc_g[fapIdx][i].isAlloc?"ALLOC":"", flowAlloc_g[fapIdx][i].isHostValid?"HOSTVALID":"");
                    }
                }
            }
            printk("\n");
            
            memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));    
            for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++) {
                fapDrv_Xmit2FapDelayed(fapIdx, FAP_MSG_DO_4KE_TEST, &fapMsg, (HZ*(fapIdx+1))/8);
            }
            break;
        }

#if defined(CONFIG_BCM_FAP_LAYER2)
        case FAP_IOC_FLOODING_MASK:
        {
            int drop = (arg >> 16) & 0xFF;
            uint8 channel = (arg >> 8) & 0xFF;
            uint8 mask = arg & 0xFF;

            fapPkt_setFloodingMask(channel, mask, drop);

            break;
        }

        case FAP_IOC_ARL_FLUSH:
        {
            fapPkt_arlFlush(arg);

            break;
        }

        case FAP_IOC_ARL_SHOW:
        {
            fapPkt_arlPrint();

            break;
        }
#endif
        case FAP_IOC_CPU:
        {
            printk("IOC_CPU\n");

            for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
            {
                int delay = 0;

                fapPrint4keTrace(fapIdx);

                fapEvent_print();

                fapPrint4keCpu(fapIdx);

                fapPrint4kePmon(fapIdx);

                fapEnetStats_dump();

#if defined(CONFIG_BCM_FAP_GSO)
                printk(FAP_IDX_FMT "::: GSO ::: packets <%u>, bytes <%u>, "
                       "txDropped <%u>, outOfMem <%u>\n", fapIdx,
                       pHostGso(fapIdx)->shared.stats.packets,
                       pHostGso(fapIdx)->shared.stats.bytes,
                       pHostGso(fapIdx)->shared.stats.txDropped,
                       pHostGso(fapIdx)->shared.stats.outOfMem);
#endif
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
                printk(FAP_IDX_FMT "fap2HostHostIf: Host <%u>, FAP <%u>\n",
                       fapIdx, fapDrvDqmStats[fapIdx].fap2HostHostIf,
                       pHostQsmGbl(fapIdx)->dqmStats.fap2HostHostIf);
                printk(FAP_IDX_FMT "host2FapHostIf: Host <%u>, FAP <%u>\n",
                       fapIdx, fapDrvDqmStats[fapIdx].host2FapHostIf,
                       pHostQsmGbl(fapIdx)->dqmStats.host2FapHostIf);
#endif
#if 0
                printk("rxHighWm <%lu>, txCount <%lu>\n",
                       pHostPsmGbl(fapIdx)->stats.rxHighWm, pHostPsmGbl(fapIdx)->stats.txCount);
#endif
                printk(FAP_IDX_FMT "::: Stats :::\n", fapIdx);

                printk(FAP_IDX_FMT "ENET rxDropped: dqmLow <%u>, dqmHigh <%u>, iq <%u>\n",
                       fapIdx,
                       pHostQsmGbl(fapIdx)->enetStats.rxDroppedDqmLow,
                       pHostQsmGbl(fapIdx)->enetStats.rxDroppedDqmHigh,
                       pHostQsmGbl(fapIdx)->enetStats.rxDroppedIq);
                printk(FAP_IDX_FMT "ENET  rxSent: dqmLow <%u>, dqmHigh <%u> \n", 
                       fapIdx,
                       pHostQsmGbl(fapIdx)->enetStats.rxSentDqmLow,
                       pHostQsmGbl(fapIdx)->enetStats.rxSentDqmHigh);

                printk(FAP_IDX_FMT "XTM  rxDropped: dqmLow <%u>, dqmHigh <%u>, iq <%u>\n",
                       fapIdx,
                       pHostQsmGbl(fapIdx)->xtmStats.rxDroppedDqmLow,
                       pHostQsmGbl(fapIdx)->xtmStats.rxDroppedDqmHigh,
                       pHostQsmGbl(fapIdx)->xtmStats.rxDroppedIq);
                printk(FAP_IDX_FMT "XTM  rxSent: dqmLow <%u>, dqmHigh <%u> \n",
                       fapIdx,
                       pHostQsmGbl(fapIdx)->xtmStats.rxSentDqmLow,
                       pHostQsmGbl(fapIdx)->xtmStats.rxSentDqmHigh);
                /* print 4ke IRQ stats */
                {
                    xmit2FapMsg_t fapMsg;

                    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

                    delay += HZ/4;
                    fapDrv_Xmit2FapDelayed(fapIdx, FAP_MSG_DBG_IRQ_STATS, &fapMsg, delay);
                    delay += HZ/4;
                    fapDrv_Xmit2FapDelayed(fapIdx, FAP_MSG_DBG_STACK, &fapMsg, delay);
                }

                printk("\n");
            }

            break;
        }
        case FAP_IOC_SWQ:
        {
#if defined(CONFIG_BCM_FAP_GSO_LOOPBACK)
            printk("\n");

            for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
            {
                printk("Dumping FAP%u SWQueue's Info\n", fapIdx);
                fapGsoLoopBk_dumpSWQs(fapIdx); 
                fap_wfd_dumpSWQs(fapIdx);
                printk("\n");
            }
#endif
            break;
        }

        case FAP_IOC_DMA_DEBUG:
        {
            xmit2FapMsg_t fapMsg;
            const ETHERNET_MAC_INFO *EnetInfo; 

            if ( (EnetInfo = BpGetEthernetMacInfoArrayPtr()) == NULL)
            {
                printk("board id not set\n");
            }
            else
            {
                int port;

                for (port=0; port<MAX_SWITCH_PORTS; ++port)
                {
                    if (EnetInfo[0].sw.port_map & (1 << port)
                        )
                    {
                        fapIdx = getFapIdxFromEthRxPort(port);

#if NUM_FAPS > 1
                        printk("eth port %d -> iudma %d -> fap %d (%s)\n",
                               port, getEthRxIudmaFromPort(port),
                               fapIdx, (fapIdx == PKTDMA_US_FAP_INDEX) ? "US" : "DS");
#else
                        printk("eth port %d -> iudma %d -> %s\n",
                               port, getEthRxIudmaFromPort(port),
                               (fapIdx == FAP_INVALID_IDX) ? "host" : "fap 0");
#endif
                    }
                }
                printk("\n");
            }

            memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));
            fapDrv_Xmit2Fap(0, FAP_MSG_DBG_DUMP_IUDMA, &fapMsg);
            if (NUM_FAPS > 1)
                fapDrv_Xmit2FapDelayed(1, FAP_MSG_DBG_DUMP_IUDMA, &fapMsg, HZ/10);
            break;
        }

        case FAP_IOC_MEM_DEBUG:
        {
            xmit2FapMsg_t fapMsg;

            printk("FAP Memory:");
            printk("  fap4kePkt_flowSizeIpv4=%d\n", fap4kePkt_flowSizeIpv4);
            printk("  fap4kePkt_flowSizeIpv6=%d\n", fap4kePkt_flowSizeIpv6);
            printk("  fap4kePkt_flowSizeL2=%d\n\n", fap4kePkt_flowSizeL2);
            
            memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));
            fapDrv_Xmit2Fap(0, FAP_MSG_DBG_DUMP_MEM, &fapMsg);
            if (NUM_FAPS > 1)
                fapDrv_Xmit2FapDelayed(1, FAP_MSG_DBG_DUMP_MEM, &fapMsg, HZ/2);
            break;
        }

        default:
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid Command [%u]", command);
            ret = FAP_ERROR;
        }
    }

    return ret;

} /* fapIoctl */

/*
 *------------------------------------------------------------------------------
 * Function Name: fapOpen
 * Description  : Called when an user application opens this device.
 * Returns      : 0 - success
 *------------------------------------------------------------------------------
 */
static int fapOpen(struct inode *inode, struct file *filp)
{
    BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "Access FAP Char Device");
    return FAP_SUCCESS;
}

/* Global file ops */
static struct file_operations fapFops =
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
    .unlocked_ioctl  = fapIoctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = fapIoctl,
#endif
#else
    .ioctl  = fapIoctl,
#endif
    .open   = fapOpen,
};
/*
 *------------------------------------------------------------------------------
 * Function Name: fap_Updatejiffies
 * Description  : Callback to update the jiffies of the last keep alive packet
 *                received
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
void fap_Updatejiffies(uint32 fapIdx)
{
    gFap[fapIdx].lastRxAlivePktJiffies = jiffies;
 return;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: keepAliveWd_timer
 * Description  : 1.5 sec timer to monitor missed keepAlive message from the 4ke.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
static void keepAliveWd_timer(unsigned long arg)
{
    unsigned int elapsed_msecs;
    uint32 fapIdx;

    /* This thread doesn't need any user-level access,
     * so get rid of all our resources
     */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
    daemonize("kpAliveWatchdog_timer");
#endif

    while(1)
    {
        /* Delay first to give FAP a chance to start and transmit a keepalive message - June 2010 */
        set_current_state(TASK_INTERRUPTIBLE);
        /*  Sleep for 1.5 sec */
        schedule_timeout((HZ + (HZ/2)));

        for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
        {
                elapsed_msecs = jiffies_to_msecs(jiffies - gFap[fapIdx].lastRxAlivePktJiffies);

                if (elapsed_msecs > 1500)
                {
                    /* Reboot removed Feb 12, 2010. Log error of delayed keepAlive message. */
                BCM_LOG_NOTICE(BCM_LOG_ID_FAP,"FAP%u Keep-Alive Timer expired: elapsed_msecs %d last rx alive %u \n",
                                  fapIdx, elapsed_msecs, gFap[fapIdx].lastRxAlivePktJiffies);
                }

                if ( pHostPsmGbl(fapIdx)->scribble0 != FAP4KE_PSM_SCRIBBLE_0 ||
                     pHostPsmGbl(fapIdx)->scribble1 != FAP4KE_PSM_SCRIBBLE_1 ||
                     pHostPsmGbl(fapIdx)->scribble2 != FAP4KE_PSM_SCRIBBLE_2 ||
                     pHostPsmGbl(fapIdx)->scribble3 != FAP4KE_PSM_SCRIBBLE_3 ||
                     pHostPsmGbl(fapIdx)->scribble4 != FAP4KE_PSM_SCRIBBLE_4 ||
                     pHostQsmGbl(fapIdx)->scribble0 != FAP4KE_QSM_SCRIBBLE_0 ||
                     pHostQsmGbl(fapIdx)->scribble1 != FAP4KE_QSM_SCRIBBLE_1 )
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_FAP, "keepAliveWd_timer: Detected scribble error for FAP%d\n", fapIdx);
                    BCM_LOG_ERROR(BCM_LOG_ID_FAP, "PSM scribble 0 (%p)=0x%08x / 0x%08x\n",& pHostPsmGbl(fapIdx)->scribble0, pHostPsmGbl(fapIdx)->scribble0, FAP4KE_PSM_SCRIBBLE_0);
                    BCM_LOG_ERROR(BCM_LOG_ID_FAP, "PSM scribble 1 (%p)=0x%08x / 0x%08x\n",& pHostPsmGbl(fapIdx)->scribble1, pHostPsmGbl(fapIdx)->scribble1, FAP4KE_PSM_SCRIBBLE_1);
                    BCM_LOG_ERROR(BCM_LOG_ID_FAP, "PSM scribble 2 (%p)=0x%08x / 0x%08x\n",& pHostPsmGbl(fapIdx)->scribble2, pHostPsmGbl(fapIdx)->scribble2, FAP4KE_PSM_SCRIBBLE_2);
                    BCM_LOG_ERROR(BCM_LOG_ID_FAP, "PSM scribble 3 (%p)=0x%08x / 0x%08x\n",& pHostPsmGbl(fapIdx)->scribble3, pHostPsmGbl(fapIdx)->scribble3, FAP4KE_PSM_SCRIBBLE_3);
                    BCM_LOG_ERROR(BCM_LOG_ID_FAP, "PSM scribble 4 (%p)=0x%08x / 0x%08x\n",& pHostPsmGbl(fapIdx)->scribble4, pHostPsmGbl(fapIdx)->scribble4, FAP4KE_PSM_SCRIBBLE_4);
                    BCM_LOG_ERROR(BCM_LOG_ID_FAP, "QSM scribble 0 (%p)=0x%08x / 0x%08x\n",& pHostQsmGbl(fapIdx)->scribble0, pHostPsmGbl(fapIdx)->scribble0, FAP4KE_QSM_SCRIBBLE_0);
                    BCM_LOG_ERROR(BCM_LOG_ID_FAP, "QSM scribble 1 (%p)=0x%08x / 0x%08x\n",& pHostQsmGbl(fapIdx)->scribble1, pHostPsmGbl(fapIdx)->scribble1, FAP4KE_QSM_SCRIBBLE_1);

                    pHostPsmGbl(fapIdx)->scribble0 = FAP4KE_PSM_SCRIBBLE_0;
                    pHostPsmGbl(fapIdx)->scribble1 = FAP4KE_PSM_SCRIBBLE_1;
                    pHostPsmGbl(fapIdx)->scribble2 = FAP4KE_PSM_SCRIBBLE_2;
                    pHostPsmGbl(fapIdx)->scribble3 = FAP4KE_PSM_SCRIBBLE_3;
                    pHostPsmGbl(fapIdx)->scribble4 = FAP4KE_PSM_SCRIBBLE_4;
                    pHostQsmGbl(fapIdx)->scribble0 = FAP4KE_QSM_SCRIBBLE_0;
                    pHostQsmGbl(fapIdx)->scribble1 = FAP4KE_QSM_SCRIBBLE_1;
                }


        }
    }

    complete_and_exit(&wdtimer_done, 0);

    BCM_LOG_NOTICE(BCM_LOG_ID_FAP,"keepAliveWd_timer: thread exits!");
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fapAlive_watchdog
 * Description  : Function to start the watchdog kernel thread to monitor
 *                missed keepAlive message from the 4ke.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
void fapAlive_watchdog( void )
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
    struct task_struct * task;
    task = kthread_run( (int(*)(void *))keepAliveWd_timer, (void *) 0, "keepAliveWd_timer" );
    timer_pid = IS_ERR(task) ? -1 : task->pid;
#else
    timer_pid = kernel_thread((int(*)(void *))keepAliveWd_timer, 0, CLONE_KERNEL);
#endif

    return;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fapDrv_Xmit2Fap
 * Description  : Sends a control message of 'len' uint32's to FAP (4ke MIPS)
 * Returns      : FAP_SUCCESS or FAP_ERROR.
 *------------------------------------------------------------------------------
 */
static int __xmit2Fap( uint32 fapIdx, fapMsgGroups_t msgType, xmit2FapMsg_t *pMsg, int block )
{
    DQMQueueDataReg_S msg;
  
    /* set the message type */
    msg.word0 = msgType;

    switch(msgType)
    {
        case FAP_MSG_FLW_ACTIVATE:
        case FAP_MSG_FLW_DEACTIVATE:
        case FAP_MSG_FLW_UPDATE:
        case FAP_MSG_FLW_RESET_STATS:
        case FAP_MSG_FLW_MCAST_ADD_CLIENT:
        case FAP_MSG_FLW_MCAST_UPDATE_CLIENT:
        case FAP_MSG_FLW_MCAST_DEL_CLIENT:
            msg.word1 = pMsg->flowCfg.flowId;
            break;

        case FAP_MSG_MCAST_SET_MISS_BEHAVIOR:
            msg.word1 = pMsg->generic.word[0];
            break;

#if defined(CONFIG_BCM_FAP_LAYER2)
        case FAP_MSG_SET_FLOODING_MASK:
            msg.word1 = pMsg->floodingMask.u32;
            break;

        case FAP_MSG_ARL_PRINT:
            break;

        case FAP_MSG_ARL_ADD:
        case FAP_MSG_ARL_REMOVE:
        case FAP_MSG_ARL_FLUSH:
            msg.word1 = pMsg->arlEntry.key.macAddrHigh;
            msg.word2 = pMsg->arlEntry.key.macAddrLowVlanId;
            msg.word3 = pMsg->arlEntry.info.u32;
            break;
#endif

        case FAP_MSG_DRV_CTL:
             /* Pass msg to FAP 4ke MIPs using DQM queue */
            msg.word1 = pMsg->drvCtl.cmd;
            msg.word2 = ((pMsg->drvCtl.drv & 0xFFFF) << 16) | (pMsg->drvCtl.channel & 0xFFFF);

            /* Params added for xtmrt dmaStatus field generation for xtm flows - Apr 2010 */
            msg.word3 = pMsg->drvCtl.params;

            break;

        case FAP_MSG_DRV_ENET_INIT:
            /* Pass msg to FAP 4ke MIPs using DQM queue */
            msg.word1 = (pMsg->drvInit.cmd | ((pMsg->drvInit.channel & 0xFF) << 8) | ((pMsg->drvInit.numBds & 0xFFFF) << 16));
            msg.word2 = pMsg->drvInit.Bds;
            msg.word3 = pMsg->drvInit.Dma;
#if defined (CONFIG_BCM_PORTS_ON_INT_EXT_SW)
            if (pMsg->extSwInit.cmd == FAPMSG_CMD_INIT_EXTSW)
            {
                msg.word1 = pMsg->extSwInit.cmd;
                msg.word2 = pMsg->extSwInit.extSwConnPort;
            }
#endif
            break;

#if defined(CONFIG_BCM_GMAC)
        case FAP_MSG_DRV_ENET_UNINIT:
            /* Pass msg to FAP 4ke MIPs using DQM queue */
            msg.word1 = (pMsg->drvInit.cmd | ((pMsg->drvInit.channel & 0xFF) << 8) | (0 << 16));
            msg.word2 = 0;
            msg.word3 = 0;
#if defined (CONFIG_BCM_PORTS_ON_INT_EXT_SW)
            if (pMsg->extSwInit.cmd == FAPMSG_CMD_INIT_EXTSW)
            {
                msg.word1 = 0;
                msg.word2 = 0;
            }
#endif
            break;
#endif /* defined(CONFIG_BCM_GMAC) */

        case FAP_MSG_DRV_XTM_INIT:
		case FAP_MSG_DRV_XTM_UNINIT:
            /* Pass msg to FAP 4ke MIPs using DQM queue */
            msg.word1 = (pMsg->drvInit.cmd | ((pMsg->drvInit.channel & 0xFF) << 8) | ((pMsg->drvInit.numBds & 0xFFFF) << 16));
            msg.word2 = pMsg->drvInit.Bds;
            msg.word3 = pMsg->drvInit.Dma;

            break;

        case FAP_MSG_DRV_XTM_INIT_STATE:
            /* Pass msg to FAP 4ke MIPs using DQM queue */
            msg.word1 = (pMsg->drvInit.cmd | ((pMsg->drvInit.channel & 0xFF) << 8)) ;
            msg.word2 = pMsg->drvInit.DmaStateRam;

            break;

        case FAP_MSG_DRV_XTM_CREATE_DEVICE:
            msg.word1 = pMsg->xtmCreateDevice.devId;
            msg.word2 = pMsg->xtmCreateDevice.encapType;
            msg.word3 = ((pMsg->xtmCreateDevice.headerLen << 16) |
                         (pMsg->xtmCreateDevice.trailerLen & 0xFFFF));
            break;

        case FAP_MSG_DRV_XTM_LINK_UP:
            msg.word1 = pMsg->xtmLinkUp.devId;
            msg.word2 = pMsg->xtmLinkUp.matchId;

            break;

        case FAP_MSG_XTM_QUEUE_DROPALG_CONFIG:
            msg.word1 = pMsg->xtmQueueDropAlg.word[0];
            msg.word2 = pMsg->xtmQueueDropAlg.word[1];
            msg.word3 = pMsg->xtmQueueDropAlg.word[2];
            break;

        case FAP_MSG_DBG_PRINT_FLOW:
            msg.word1 = pMsg->flowCfg.flowId;
            break;

        case FAP_MSG_DBG_DUMP_IUDMA:
        case FAP_MSG_DBG_DUMP_MEM:
        case FAP_MSG_DBG_IRQ_STATS:
        case FAP_MSG_DBG_STACK:
        case FAP_MSG_DO_4KE_TEST:
            /* no arguments to pass */
            break;

        case FAP_MSG_SET_MTU:
            msg.word1 = pMsg->mtu.word[0];
            msg.word2 = pMsg->mtu.word[1];
            msg.word3 = pMsg->mtu.word[2];
            break;

#if defined(CC_FAP4KE_TM)
        case FAP_MSG_TM_ENABLE:
        case FAP_MSG_TM_PORT_CONFIG:
        case FAP_MSG_TM_QUEUE_CONFIG:
        case FAP_MSG_TM_QUEUE_PROFILE_CONFIG:
        case FAP_MSG_TM_QUEUE_DROPALG_CONFIG:
        case FAP_MSG_TM_ARBITER_CONFIG:
        case FAP_MSG_TM_STATS:
        case FAP_MSG_TM_MAP_PORT_TO_SCHED:
        case FAP_MSG_TM_MAP_TMQUEUE_TO_SWQUEUE:
        case FAP_MSG_TM_MAP_DUMP:
        case FAP_MSG_TM_PAUSE_EN:
            msg.word1 = pMsg->tm.word[0];
            msg.word2 = pMsg->tm.word[1];
            msg.word3 = pMsg->tm.word[2];
            break;
#endif

#if defined(CC_FAP4KE_PERF) && (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
        case FAP_MSG_PERF_ENABLE:
        case FAP_MSG_PERF_DISABLE:
        case FAP_MSG_PERF_SET_GENERATOR:
        case FAP_MSG_PERF_SET_ANALYZER:
            msg.word1 = pMsg->tm.word[0];
            msg.word2 = pMsg->tm.word[1];
            msg.word3 = pMsg->tm.word[2];
            break;
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        case FAP_MSG_BPM:
            msg.word1 = pMsg->allocBuf.word[0];
            msg.word2 = pMsg->allocBuf.word[1];
            msg.word3 = pMsg->allocBuf.word[2];
            break;
#endif
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
        case FAP_MSG_IQ:
            msg.word1 = pMsg->status.word[0];
            msg.word2 = pMsg->status.word[1];
            msg.word3 = pMsg->status.word[2];
            break;
#endif
        case FAP_MSG_STATS:
            msg.word1 = pMsg->stats.word[0];
            msg.word2 = pMsg->stats.word[1];
            msg.word3 = pMsg->stats.word[2];
            break;

        case FAP_MSG_CONFIG_TCP_ACK_MFLOWS:
            msg.word1 = pMsg->generic.word[0];
            break;

        default:
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Invalid HostIf msgType [%u]", msgType);
            BCM_ASSERT(0);
    }

    {
        unsigned long flags;

        FAP_HOSTIF_LOCK(flags);

#if 1
        {
            /* DEBUG CODE: do error if can't send message for to long */
            unsigned long timeout = jiffies + HZ*2;

            /*
             * Blocking mode: wait until there is room to send the message to FAP
             * Unblocking mode: return right away if no room to send message to FAP
             */
            while(1)
            {
                if(!dqmXmitAvailableHost(fapIdx, DQM_HOST2FAP_HOSTIF_Q))
                {
                    if(!block)
                    {
                        FAP_HOSTIF_UNLOCK(flags);

                        return FAP_ERROR;
                    }
                }
                else
                {
                    break;
                }

                if (time_after(jiffies, timeout))
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_FAP, "FAP MESSAGE TIMED OUT! [%u] "
                                  "(0x%08x/0x%08x(%d)/0x%08x/0x%08x/0x%08x/0x%08x)", 
                        msgType, 
                        pHostFapSdram(fapIdx)->dbgVals[0],
                        pHostFapSdram(fapIdx)->dbgVals[1],pHostFapSdram(fapIdx)->dbgVals[1],
                        pHostFapSdram(fapIdx)->dbgVals[2],
                        pHostFapSdram(fapIdx)->dbgVals[3],
                        pHostFapSdram(fapIdx)->dbgVals[4],
                        pHostFapSdram(fapIdx)->dbgVals[5]);
                    timeout = jiffies + HZ*10;                    
                    ASSERT(0);
                    FAP_HOSTIF_UNLOCK(flags);
                    return FAP_ERROR;
                }
            }
        }
#else
        {
            /*
             * Blocking mode: wait until there is room to send the message to FAP
             * Unblocking mode: return right away if no room to send message to FAP
             */
            while(1)
            {
                if(!dqmXmitAvailableHost(fapIdx, DQM_HOST2FAP_HOSTIF_Q))
                {
                    if(!block)
                    {
                        FAP_HOSTIF_UNLOCK(flags);

                        return FAP_ERROR;
                    }
                }
                else
                {
                    break;
                }
            }
        }
#endif

        dqmXmitMsgHost(fapIdx, DQM_HOST2FAP_HOSTIF_Q, DQM_HOST2FAP_HOSTIF_Q_SIZE, &msg);

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        fapDrvDqmStats[fapIdx].host2FapHostIf++;
#endif

        FAP_HOSTIF_UNLOCK(flags);
    }

    return FAP_SUCCESS;
}

void fapDrv_Xmit2Fap( uint32 fapIdx, fapMsgGroups_t msgType, xmit2FapMsg_t *pMsg )
{
    __xmit2Fap( fapIdx, msgType, pMsg, 1 );
}

/*
 *------------------------------------------------------------------------------
 * Function Name: bindToBcmPktDma
 * Description  : Binds FAP driver APIs to the PKT DMA Library.
 * Returns      : FAP_SUCCESS or FAP_ERROR.
 *------------------------------------------------------------------------------
 */
static int isDqmRecvAvailableHost(uint32 fapIdx, uint32 queue)
{
    return dqmRecvAvailableHost(fapIdx, queue);
}

static int isDqmXmitAvailableHost(uint32 fapIdx, uint32 queue)
{
    return dqmXmitAvailableHost(fapIdx, queue);
}

static fapRet bindToBcmPktDma(void)
{
    fapRet ret;
    bcmPktDma_hostHooks_t hostHooks;

    hostHooks.xmit2Fap = fapDrv_Xmit2Fap;
    hostHooks.psmAlloc = fapDrv_psmAlloc;

    hostHooks.dqmXmitMsgHost = dqmXmitMsgHost;
    hostHooks.dqmRecvMsgHost = dqmRecvMsgHost;
    hostHooks.isDqmXmitAvailableHost = isDqmXmitAvailableHost;
    hostHooks.isDqmRecvAvailableHost = isDqmRecvAvailableHost;
    hostHooks.dqmEnableHost = _dqmHandlerEnableHost;
    hostHooks.dqmEnableNotEmptyIrq = dqmEnableNotEmptyIrqMskHost;
    hostHooks.dqmHandlerRegisterHost = dqmHandlerRegisterHost;
    hostHooks.swqRecvAvailableHost = swqRecvAvailableHost;
    hostHooks.swqRecvMsgHost = swqRecvMsgHost;

#if defined(CC_FAP4KE_TM)
    hostHooks.tmMasterConfig = fapTm_masterConfig;
    hostHooks.tmPortConfig = fapTm_portConfig;
    hostHooks.tmSetPortMode = fapTm_setPortMode;
    hostHooks.tmGetPortMode = fapTm_getPortMode;
    hostHooks.tmPortType = fapTm_portType;
    hostHooks.tmPortEnable = fapTm_portEnable;
    hostHooks.tmPauseEnable = fapTm_pauseEnable;
    hostHooks.tmApply = fapTm_apply;
    hostHooks.tmCheckSetHighPrio = fapTm_checkSetHighPrio;
    hostHooks.tmXtmCheckHighPrio = fapTm_xtmCheckHighPrio;
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    hostHooks.tmXtmQueueDropAlgConfig = fapTm_XtmQueueDropAlgConfigExt;
#endif
#endif

    ret = bcmPktDma_bind(&hostHooks);
    if(ret == FAP_ERROR)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Could not bind to BCM Packet DMA Library");
    }

    return ret;
}

#if defined(CC_FAP_EVENTS)
#define FAP_EVENT_HISTORY_SIZE 200

static char *fapEventTypeName[] = FAP_EVENT_TYPE_NAME;

typedef struct {
    fapEvent_type_t type;
    uint32_t time;
    uint32_t arg;
} fapEvent_record_t;

typedef struct {
    uint32_t write;
    fapEvent_record_t record[FAP_EVENT_HISTORY_SIZE];
} fapEvent_history_t;

static fapEvent_history_t fapEvent_history_g;

uint32_t fapEnet_txQueueUsage(uint32 fapIdx)
{
    return (DQM_FAP2HOST_PACKET_Q_DEPTH - /* Tx Queue depth */
            FAP_HOST_REG_RD(hostDqmQCntrlReg(fapIdx)->q[DQM_FAP2HOST_PACKET_Q].sts));
}

void fapEvent_record(fapEvent_type_t type, uint32_t arg)
{
    uint32_t time = __read_pfctr_0();
    fapEvent_record_t *record_p = &fapEvent_history_g.record[fapEvent_history_g.write];

//    BCM_ASSERT(fapEvent_history_g.write < FAP_EVENT_HISTORY_SIZE);

    record_p->type = type;
    record_p->time = time;
    record_p->arg = arg;

    if(++fapEvent_history_g.write == FAP_EVENT_HISTORY_SIZE)
    {
        fapEvent_history_g.write = 0;
    }
}

void fapEvent_print(void)
{
    fapEvent_record_t *record_p;
    uint32_t startTime;
    uint32_t read;
    uint32_t currTime=0, prevTime;
    int i;

    read = fapEvent_history_g.write;
    startTime = fapEvent_history_g.record[read].time;

    for(i=0; i<FAP_EVENT_HISTORY_SIZE; ++i)
    {
        if(read == FAP_EVENT_HISTORY_SIZE)
        {
            read = 0;
        }

        record_p = &fapEvent_history_g.record[read];
        prevTime = currTime;
        currTime = ((record_p->time - startTime) * 2) / 1000;

        printk("[%03u] %s\t: %u, %u us\n",
               read, fapEventTypeName[record_p->type],
               record_p->arg, prevTime-currTime);

        read++;
    }
}

static void fapEvent_init(void)
{
    memset(&fapEvent_history_g, 0, sizeof(fapEvent_history_t));
}

EXPORT_SYMBOL(fapEvent_record);
EXPORT_SYMBOL(fapEvent_print);
EXPORT_SYMBOL(fapEnet_txQueueUsage);
#endif

uint32_t fapIq_EthRxQueueUsage(uint32_t chnl)
{
    uint32 queUsage = 0;
#if 0
    int fapIdx = 0;


    fapIdx = getFapIdxFromEthRxIudma(chnl);

    if (isValidFapIdx(fapIdx))
    {
        if (chnl == 0)
        {
            queUsage = (DQM_FAP2HOST_ETH0_RX_DEPTH -
                FAP_HOST_REG_RD(hostDqmQCntrlReg(fapIdx)
                                    ->q[DQM_FAP2HOST_ETH0_RX_Q].sts) );
        }
        else
        {
            queUsage = (DQM_FAP2HOST_ETH0_RX_DEPTH -
                FAP_HOST_REG_RD(hostDqmQCntrlReg(fapIdx)
                                    ->q[DQM_FAP2HOST_ETH1_RX_Q].sts) );
        }
    }
    else
    {
        printk("WARNING: %s.%d: invalid FAP index %d\n", 
            __func__, __LINE__, fapIdx);
    }
#endif
    return queUsage;
}
EXPORT_SYMBOL(fapIq_EthRxQueueUsage);

uint32_t fapIq_XtmRxQueueUsage( uint32_t chnl )
{
    uint32 queUsage = 0;
#if 0
    int fapIdx = 0;

    fapIdx = getFapIdxFromXtmRxIudma(chnl);

    if (isValidFapIdx(fapIdx))
    {
        if (chnl == 0)
        {
            queUsage = (DQM_FAP2HOST_XTM0_RX_DEPTH -
                    FAP_HOST_REG_RD(hostDqmQCntrlReg(fapIdx)
                                        ->q[DQM_FAP2HOST_XTM0_RX_Q].sts) );
        }
        else
        {
            queUsage = (DQM_FAP2HOST_XTM1_RX_DEPTH -
                    FAP_HOST_REG_RD(hostDqmQCntrlReg(fapIdx)
                                        ->q[DQM_FAP2HOST_XTM1_RX_Q].sts) );
        }
    }
    else
    {
        printk("WARNING: %s.%d: invalid FAP index %d\n", 
            __func__, __LINE__, fapIdx);
    }
#endif
    return queUsage;
}
EXPORT_SYMBOL(fapIq_XtmRxQueueUsage);

/******************************************************************************
 * Layer 2 Flows
 *****************************************************************************/

#if defined(CONFIG_BCM_FAP_LAYER2)
static int fapNotifierHandler(struct notifier_block *unused, unsigned long event, void *ptr)
{
    struct net_device *dev = NETDEV_NOTIFIER_GET_DEV(ptr);
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    /* We run under the RTNL lock here */

    __arlKey((uint16 *)dev->dev_addr, 0, &fapMsg.arlEntry.key);

    switch(event)
    {
        case NETDEV_CHANGEADDR:
        {
#if 1
            {
                uint8 *u8_p = (uint8 *)dev->dev_addr;
                printk("%s : NETDEV_CHANGEADDR(%u) %s <%02X:%02X:%02X:%02X:%02X:%02X>\n",
                       __FUNCTION__, event, dev->name,
                       u8_p[0], u8_p[1], u8_p[2], u8_p[3], u8_p[4], u8_p[5]);
            }
#endif
            for(fapIdx=0; fapIdx<NUM_FAPS; fapIdx++)
            {
                fapMsg.arlEntry.info.phy = FAP4KE_PKT_PHY_HOST;
                fapMsg.arlEntry.info.channelMask = 0;
                fapMsg.arlEntry.info.flags = 0;

                fapDrv_Xmit2Fap(fapIdx, FAP_MSG_ARL_ADD, &fapMsg);
            }

            break;
        }

        case NETDEV_UNREGISTER:
        {
#if 1
            {
                uint8 *u8_p = (uint8 *)dev->dev_addr;
                printk("%s : NETDEV_UNREGISTER(%u) %s <%02X:%02X:%02X:%02X:%02X:%02X>\n",
                       __FUNCTION__, event, dev->name,
                       u8_p[0], u8_p[1], u8_p[2], u8_p[3], u8_p[4], u8_p[5]);
            }
#endif
            for(fapIdx=0; fapIdx<NUM_FAPS; fapIdx++)
            {
                fapDrv_Xmit2Fap(fapIdx, FAP_MSG_ARL_REMOVE, &fapMsg);
            }

            break;
        }
    }

    return NOTIFY_DONE;
}

static struct notifier_block fapNotifierBlock = {
    .notifier_call = fapNotifierHandler,
};

static void fapMacAddressHandler(unsigned char *pucaMacAddr, MAC_ADDRESS_OPERATION op)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

#if 0
    {
        uint8 *u8_p = (uint8 *)pucaMacAddr;

        printk("\n\tfapMacAddressHandler: op <%d>, <%02X:%02X:%02X:%02X:%02X:%02X>\n\n",
               op, u8_p[0], u8_p[1], u8_p[2], u8_p[3], u8_p[4], u8_p[5]);
    }
#endif

    __arlKey((uint16 *)pucaMacAddr, 0, &fapMsg.arlEntry.key);

    switch(op)
    {
        case MAC_ADDRESS_OP_GET:
            for(fapIdx=0; fapIdx<NUM_FAPS; fapIdx++)
            {
                fapMsg.arlEntry.info.phy = FAP4KE_PKT_PHY_HOST;
                fapMsg.arlEntry.info.channelMask = 0;
                fapMsg.arlEntry.info.flags = 0;

                fapDrv_Xmit2Fap(fapIdx, FAP_MSG_ARL_ADD, &fapMsg);
            }
            break;

        case MAC_ADDRESS_OP_RELEASE:
#if 0
            for(fapIdx=0; fapIdx<NUM_FAPS; fapIdx++)
            {
                fapDrv_Xmit2Fap(fapIdx, FAP_MSG_ARL_REMOVE, &fapMsg);
            }
#endif
            break;

        default:
            BCM_ASSERT(0);
    }
}

static void __dumpArlNotify(uint32 fapIdx, fapMsgGroups_t msgType, fapMsg_arlEntry_t *entry_p)
{
    uint8 *u8_p = (uint8 *)&entry_p->key.macAddrHigh;
    char *msgType_p = ((hostMsgGroups_t)msgType == HOST_MSG_ARL_ADD) ? "ADD" : "REM";

    printk("--> FAP%u ARL Notification\n", fapIdx);

    if(entry_p->info.revIvl)
    {
        printk("%s: Mac <%02X:%02X:%02X:%02X:%02X:%02X>, Vid <%u>, Phy <%u>, ChanMask <0x%02X>\n"
               "     RevIvl [Queue <%u>, Vid <%u>], destChannelMask <0x%02X>, nbrOfTags <%u>\n",
               msgType_p, u8_p[0], u8_p[1], u8_p[2], u8_p[3], u8_p[4], u8_p[5],
               entry_p->key.vlanId, entry_p->info.phy, entry_p->info.channelMask,
               entry_p->info.queue, entry_p->info.vlanId, entry_p->destChannelMask, entry_p->nbrOfTags);
    }
    else
    {
        printk("%s: Mac <%02X:%02X:%02X:%02X:%02X:%02X>, Vid <%u>, Phy <%u>, ChanMask <0x%02X,%u>\n"
               "     destChannelMask <0x%02X>, nbrOfTags <%u>\n",
               msgType_p, u8_p[0], u8_p[1], u8_p[2], u8_p[3], u8_p[4], u8_p[5], entry_p->key.vlanId,
               entry_p->info.phy, entry_p->info.channelMask, entry_p->info.multiChan,
               entry_p->destChannelMask, entry_p->nbrOfTags);
    }
}
#endif /*CONFIG_BCM_FAP_LAYER2 */


#define __dumpDqm(fapIdx, dqm)                                          \
    do {                                                                \
        printk("[FAP%u] %s (%u): avail %u\n",                                \
               fapIdx, #dqm, dqm, hostDqmQCntrlReg(fapIdx)->q[dqm].sts & AVAIL_TOKEN_SPACE_MASK); \
    } while(0)

static void fap_sysrq_dqm_dump(uint32 fapIdx)
{
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    __dumpDqm(fapIdx, DQM_FAP2HOST_XTM_RX_Q_LOW);
    __dumpDqm(fapIdx, DQM_FAP2HOST_XTM_RX_Q_HI);
    __dumpDqm(fapIdx, DQM_HOST2FAP_XTM_XMIT_Q_LOW);
    __dumpDqm(fapIdx, DQM_HOST2FAP_XTM_XMIT_Q_HI);
    __dumpDqm(fapIdx, DQM_HOST2FAP_XTM_FREE_RXBUF_Q);
    __dumpDqm(fapIdx, DQM_FAP2HOST_XTM_FREE_TXBUF_Q);
#endif /* CONFIG_BCM_XTMCFG */

    __dumpDqm(fapIdx, DQM_FAP2HOST_ETH_RX_Q_LOW);
    __dumpDqm(fapIdx, DQM_FAP2HOST_ETH_RX_Q_HI);
    __dumpDqm(fapIdx, DQM_HOST2FAP_ETH_XMIT_Q_LOW);
    __dumpDqm(fapIdx, DQM_HOST2FAP_ETH_XMIT_Q_HI);
    __dumpDqm(fapIdx, DQM_HOST2FAP_ETH_FREE_RXBUF_Q);
    __dumpDqm(fapIdx, DQM_FAP2HOST_ETH_FREE_TXBUF_Q);

    __dumpDqm(fapIdx, DQM_HOST2FAP_HOSTIF_Q);
    __dumpDqm(fapIdx, DQM_FAP2HOST_HOSTIF_Q);
}

void fapMailBox_sysRq(uint32 fapIdx);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static void fap_sysrq_handler(int key)
#else
static void fap_sysrq_handler(int key, struct tty_struct *tty)
#endif
{
    int i;
    uint32 fapIdx;

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        printk("\nFAP%d\n", fapIdx);
        
        if (!pHostFapSdram(fapIdx))
        {
            printk("host sdram not yet initialized\n");
            return;
        }

        for (i = 0; i < 10; i++)
        {
            printk("FAP4KE [%2d] 0x%08x (%d)\n",
                   i, pHostFapSdram(fapIdx)->dbgVals[i],
                   pHostFapSdram(fapIdx)->dbgVals[i]);
        }
        printk("\n");

        fap_sysrq_dqm_dump(fapIdx);

        printk("\n");
    }

    mdelay(200);

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        fapMailBox_sysRq(fapIdx);
    }
}

static struct sysrq_key_op fapdbg_sysrq_op = {
    .handler = fap_sysrq_handler,
    .help_msg = "dump-fap-debug(d)",
    .action_msg = "dump fap debugs",
};

/*
 *------------------------------------------------------------------------------
 * Function Name: fapDrv_change4keLogLevel
 * Description  : callback which is called when log level of 4ke changes.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
void fapDrv_change4keLogLevel(bcmLogId_t logId, bcmLogLevel_t level, void *ctx)
{
    uint32 fapIdx;
    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        pHostQsmGbl(fapIdx)->dbgEnabled[FAP_MAILBOX_MSGID_LOG_ERROR] = (level >= BCM_LOG_LEVEL_ERROR);
        pHostQsmGbl(fapIdx)->dbgEnabled[FAP_MAILBOX_MSGID_LOG_NOTICE] = (level >= BCM_LOG_LEVEL_NOTICE);
        pHostQsmGbl(fapIdx)->dbgEnabled[FAP_MAILBOX_MSGID_LOG_INFO] =  (level >= BCM_LOG_LEVEL_INFO);
        pHostQsmGbl(fapIdx)->dbgEnabled[FAP_MAILBOX_MSGID_LOG_DEBUG] = (level >= BCM_LOG_LEVEL_DEBUG);
    }    
}

int fap_wfd_init(void)
{
    int qidx;
    uint32 fapIdx=0;
    int bufSize = SWQ_FAP2HOST_WFD_Q_MEM_SIZE * 4;
    uint8 *swqMem_p;
    int dqm = 0;
    int dqmidx = 0;

    for (qidx = 0; qidx < WFD_NUM_QUEUE_SUPPORTED; qidx++)
    {
        dqmidx = qidx / NUM_FAPS; 
        dqm = DQM_FAP2HOST_WFD_BASE_Q + dqmidx;

        /*allocate and set the DDR memory for SWQueues. 4 corresponds to 4 bytes/1 word */
        swqMem_p = kmalloc(bufSize, GFP_KERNEL);

        if (swqMem_p == NULL)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, 
                          "Could not allocate(%d bytes) for qidx %d dqm %d", 
                          bufSize, qidx, dqm);
            return -1;
        }

        /* Invalidate memory from Host D$ */
        fap_cacheInvFlush((void *)(swqMem_p),
                          (void *)(swqMem_p + bufSize - 1),
                          0);

        pHostFapSdram(fapIdx)->initParams.wfdF2HSwqMem_p[dqmidx] =
           (uint32 *)KSEG1ADDR(swqMem_p);

        printk("Allocated FAP%d SWQ_FAP2HOST_WFD_Q mem=%p : %d bytes\n",
               fapIdx, pHostFapSdram(fapIdx)->initParams.wfdF2HSwqMem_p[dqmidx], bufSize);

        /* store the static information about the swq's in cached memory */
        wfdF2HQInfo[qidx].swq     = &pHostPsmGbl(fapIdx)->wfdF2HSwq[dqmidx];
        wfdF2HQInfo[qidx].qStart  = pHostFapSdram(fapIdx)->initParams.wfdF2HSwqMem_p[dqmidx];
        wfdF2HQInfo[qidx].qEnd    = wfdF2HQInfo[qidx].qStart + SWQ_FAP2HOST_WFD_Q_MEM_SIZE;
        wfdF2HQInfo[qidx].dqm     = dqm;
        wfdF2HQInfo[qidx].fapId   = fapIdx;
        wfdF2HQInfo[qidx].msgSize = SWQ_FAP2HOST_WFD_Q_MSG_SIZE;
#if NUM_FAPS == 2
        fapIdx = (fapIdx + 1) % NUM_FAPS;
#endif
    }

    return 0;
}

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
/*
 *------------------------------------------------------------------------------
 * Function Name: fapIq_setStatus
 * Description  : Sets the Ingress QoS enable/disable status level in 4KE
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
int fapIq_setStatus(void *iq_param)
{
    uint32_t status = ((iq_param_t *)iq_param)->status;
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

    fapMsg.status.cmd     = FAPMSG_CMD_SET_IQ_STATUS;
    fapMsg.status.drv     = FAPMSG_DRV_ENET;
    fapMsg.status.status  = status;

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_IQ, &fapMsg);
    }
    return 0;
}


/*
 *------------------------------------------------------------------------------
 * Function Name: fapIq_dumpStatus
 * Description  : Sends dump IQ Status message to FAP.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
int fapIq_dumpStatus(void *iq_param)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));

    fapMsg.status.cmd     = FAPMSG_CMD_DUMP_IQ_STATUS;
    fapMsg.status.drv     = FAPMSG_DRV_ENET;
    fapMsg.status.status  = 0;

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_IQ, &fapMsg);
    }

    return 0;
}

static void fapIq_add_L4port(uint8_t ipProto, uint16_t dport,
            uint8_t ent, uint8_t prio)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));
    fapMsg.iqinfo.cmd     = FAPMSG_CMD_IQ_ADD_PORT;
    fapMsg.iqinfo.drv     = FAPMSG_DRV_ENET;
    fapMsg.iqinfo.proto   = ipProto;
    fapMsg.iqinfo.dport   = dport;
    fapMsg.iqinfo.ent     = ent;
    fapMsg.iqinfo.prio    = prio;

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_IQ, &fapMsg);
    }
}

static void fapIq_rem_L4port(uint8_t ipProto, uint16_t dport, uint8_t ent)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));
    fapMsg.iqinfo.cmd     = FAPMSG_CMD_IQ_REM_PORT;
    fapMsg.iqinfo.drv     = FAPMSG_DRV_ENET;
    fapMsg.iqinfo.proto   = ipProto;
    fapMsg.iqinfo.dport   = dport;
    fapMsg.iqinfo.ent     = ent;
    fapMsg.iqinfo.prio    = 0;

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_IQ, &fapMsg);
    }
}

static void fapIq_set_proto_prio(uint8_t protoType, uint8_t protoval,
            uint8_t prio)
{

    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));
    fapMsg.iqinfo.cmd     = FAPMSG_CMD_IQ_ADD_PROTO_PRIO;
    fapMsg.iqinfo.drv     = FAPMSG_DRV_ENET;
    fapMsg.iqinfo.proto   = protoType;
    fapMsg.iqinfo.protoval= protoval;
    fapMsg.iqinfo.prio    = prio;

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_IQ, &fapMsg);
    }
}

static void fapIq_rem_proto_prio(uint8_t protoType, uint8_t protoval)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));
    fapMsg.iqinfo.cmd     = FAPMSG_CMD_IQ_REM_PROTO_PRIO;
    fapMsg.iqinfo.drv     = FAPMSG_DRV_ENET;
    fapMsg.iqinfo.proto   = protoType;
    fapMsg.iqinfo.protoval= protoval;

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_IQ, &fapMsg);
    }
}

int fapIq_dump_porttbl(void *iq_param)
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx = 0;

    memset(&fapMsg, 0, sizeof(xmit2FapMsg_t));
    fapMsg.iqinfo.cmd     = FAPMSG_CMD_IQ_DUMP_PORTTBL;
    fapMsg.iqinfo.drv     = FAPMSG_DRV_ENET;

    fapMsg.iqinfo.proto   = IPPROTO_UDP;
    /* Dump porttbl only from FAP0 to shorten the console output */
    fapDrv_Xmit2Fap(fapIdx, FAP_MSG_IQ, &fapMsg);

    fapMsg.iqinfo.proto   = IPPROTO_TCP;
    fapDrv_Xmit2Fap(fapIdx, FAP_MSG_IQ, &fapMsg);

    return 0;
}

int fapIq_add_entry(void *iq_param)
{
    uint32_t key_mask = ((iq_param_t *)iq_param)->key_mask;
    iq_key_data_t *key_data = &((iq_param_t *)iq_param)->key_data;
    iq_action_t *action = &((iq_param_t *)iq_param)->action;
    uint8_t ipProto = key_data->ip_proto;
    uint16_t dport  = key_data->l4_dst_port;
    uint8_t prio = action->value;

    if ((key_mask ^ (IQ_KEY_MASK_IP_PROTO | IQ_KEY_MASK_DST_PORT)) == 0)
    {
        fapIq_add_L4port(ipProto, dport, IQOS_ENT_STAT, prio);
    }

    /* TODO! fap only support IP Proto type, find a better
     * value to replace the next define */
#define FAP_IQ_PROTO_IP		0
    if ((key_mask ^ IQ_KEY_MASK_IP_PROTO) == 0)
    {
        fapIq_set_proto_prio(FAP_IQ_PROTO_IP, ipProto, prio);
    }

    return 0;
}

int fapIq_delete_entry(void *iq_param)
{
    uint32_t key_mask = ((iq_param_t *)iq_param)->key_mask;
    iq_key_data_t *key_data = &((iq_param_t *)iq_param)->key_data;
    uint8_t ipProto = key_data->ip_proto;
    uint16_t dport  = key_data->l4_dst_port;

    if ((key_mask ^ (IQ_KEY_MASK_IP_PROTO | IQ_KEY_MASK_DST_PORT)) == 0)
    {
        fapIq_rem_L4port(ipProto, dport, IQOS_ENT_STAT);
    }

    if ((key_mask ^ IQ_KEY_MASK_IP_PROTO) == 0)
    {
        fapIq_rem_proto_prio(FAP_IQ_PROTO_IP, ipProto);
    }

    return 0;
}

static const iq_hw_info_t fapIq_info_db = {
	.mask_capability = (IQ_KEY_MASK_IP_PROTO + IQ_KEY_MASK_DST_PORT),
	.add_entry = fapIq_add_entry,
	.delete_entry = fapIq_delete_entry,
	.set_status = fapIq_setStatus,
	.get_status = fapIq_dumpStatus,
	.dump_table = fapIq_dump_porttbl,
};
#endif /* (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)) */

#if defined(CONFIG_BCM_GMAC)
volatile int fapDrv_getEnetRxEnabledStatus( int channel )
{
    uint32 fapIdx = getFapIdxFromEthRxIudma(channel);
    return (pHostPsmGbl(fapIdx)->gmac.rxFlags[channel] & FAP_CHAN_ENABLED);
}
EXPORT_SYMBOL(fapDrv_getEnetRxEnabledStatus);
#endif


/*
 *------------------------------------------------------------------------------
 * Function Name: fapDrv_construct
 * Description  : Initial function that is called at system startup that
 *                registers this device. See fapConfig.c
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
int __init fapDrv_construct(void)
{
    int ret;
    uint32 chipId = PERF->RevID;
    uint32 fapIdx;

    printk("chipId 0x%08X\n", chipId);

    /* FIXME: Temporary test only! */
    //    bcmPktDma_registerArlNotifyHandler(arlNotifyHandlerFunc);

    ret = register_sysrq_key('d', &fapdbg_sysrq_op);
    if (ret != 0)
    {
        printk("WARNING -- could not register sysrq key for FAP!\n");
    }


#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)
    spin_lock_init(&fapHostIf_lock_g);
#endif
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    memset(&fapDrvDqmStats, 0, sizeof(fapDrv_dqmStats_t));
    memset(&fapDrvBpmStats, 0, sizeof(fapDrv_bpmStats_t));
#endif

#if defined(CONFIG_BCM_FAP_LAYER2)
    /* Register to receive MAC address events */
    ret = kerSysMacAddressNotifyBind(fapMacAddressHandler);
    if(ret)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Could not kerSysMacAddressNotifyBind <%d>", ret);

        return FAP_ERROR;
    }
#endif

    if(register_chrdev(FAPDRV_MAJOR, FAPDRV_NAME, &fapFops))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Unable to get major number <%d>", FAPDRV_MAJOR);

        return FAP_ERROR;
    }

    printk(FAP_MODNAME " Char Driver " FAP_VER_STR " Registered <%d>\n", FAPDRV_MAJOR);
    /* debugging only */
    bcmLog_setLogLevel(BCM_LOG_ID_FAP4KE, BCM_LOG_LEVEL_ERROR);
    bcmLog_setLogLevel(BCM_LOG_ID_FAP, BCM_LOG_LEVEL_ERROR);

#if defined(CC_FAP_EVENTS)
    fapEvent_init();
#endif


    /* allocate FAP memories, enable FAP clock, reset FAP block, and initialize 4ke registers */

    // FAP_TBD: review this to see if we can do one giant loop...

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        ret = fap_init4ke(fapIdx);
    }

    printk("FAP Debug values at 0x%p 0x%p\n", &(pHostFapSdram(0)->dbgVals[0]), &(pHostFapSdram(1)->dbgVals[0]));

    bcmLog_registerLevelChangeCallback(BCM_LOG_ID_FAP4KE, fapDrv_change4keLogLevel, NULL);


#if defined(CONFIG_BCM_FAP_GSO)

    fapGsoDescPool = kmalloc(sizeof(fapGsoDesc_t) * FAP_MAX_GSODESC, GFP_KERNEL);

    if(!fapGsoDescPool)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP,
                "Could not allocate FAP GSO Descriptors (%d bytes)",
                sizeof(fapGsoDesc_t) * FAP_MAX_GSODESC);

        unregister_chrdev(FAPDRV_MAJOR, FAPDRV_NAME);
        return FAP_ERROR;
    }

    memset(fapGsoDescPool, 0, sizeof(fapGsoDesc_t) * FAP_MAX_GSODESC);

    /* Invalidate memory from Host D$ */
    fap_cacheInvFlush((void *)(fapGsoDescPool),
            (void *)(fapGsoDescPool) + (sizeof(fapGsoDesc_t) * FAP_MAX_GSODESC),
            1);
    fapGsoDescPool = (fapGsoDesc_t *)KSEG1ADDR((void *)fapGsoDescPool);

#if defined(CONFIG_BCM_FAP_GSO_LOOPBACK)
    if(fapGsoLoopBk_init() == FAP_ERROR)
    {
        unregister_chrdev(FAPDRV_MAJOR, FAPDRV_NAME);
        return FAP_ERROR;
    }
#endif

#endif

    fap_wfd_init();

#if defined(CC_FAP4KE_TM)
    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        uint8 *pTmSdramAlloc = kmalloc(sizeof(fap4keTm_sdram_t), GFP_KERNEL);

        if(pTmSdramAlloc == NULL)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP,
                          "Could not allocate FAP Traffic Manager Buffers (%d bytes)",
                          sizeof(fap4keTm_sdram_t));

            unregister_chrdev(FAPDRV_MAJOR, FAPDRV_NAME);

            return FAP_ERROR;
        }

        /* Invalidate memory from Host D$ */
        fap_cacheInvFlush((void *)(pTmSdramAlloc),
                          (void *)(pTmSdramAlloc + sizeof(fap4keTm_sdram_t) - 1),
                          0);

        pHostFapSdram(fapIdx)->initParams.tmSdram_p =
            (fap4keTm_sdram_t *)KSEG1ADDR(pTmSdramAlloc);

        printk("Allocated FAP%d TM SDRAM Queue Storage (%p) : %u bytes @ %p\n",
               fapIdx, &pHostFapSdram(fapIdx)->initParams.tmSdram_p,
               sizeof(fap4keTm_sdram_t), pHostFapSdram(fapIdx)->initParams.tmSdram_p);
    }
#endif

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        ret = fapIrq_init(fapIdx, TRUE);

        fapMailBox_hostInit(fapIdx);
    }

    fapDm_init();
    
    fapDqm_hostInit();

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    /* bind GBPM USER hooks */
    gbpm_fap_evt_hook_g    = fapBpm_schedEvt;
    gbpm_g.fap_status      = fapBpm_dumpStatus;
    gbpm_g.fap_thresh      = fapBpm_dumpTxQThresh;
    gbpm_g.fap_enet_thresh = fapBpm_dumpEthTxQThresh;
    gbpm_g.fap_upd_buf_lvl = fapBpm_updBufLvl;

    fapBpm_eventQ_init();
#endif

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        fap_enable4ke(fapIdx);
    }

    fapAlive_watchdog();

    fapPkt_construct();

    fapProtoConstruct();

    /* Initialize PSM ptr where memory allocation is made from in fapDrv_psmAlloc - Apr 2010 */
    /* round next ptr up to next 16 byte boundary */
    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        /* FAP_DM_P2: move this to dynamic memory */
        pHostPsmGbl(fapIdx)->pManagedMemory = (uint8 *)(((int)&pHostPsmGbl(fapIdx)->ManagedMemory[0] + 0xF) & ~0xF);
        if(pHostPsmGbl(fapIdx)->pManagedMemory != (&pHostPsmGbl(fapIdx)->ManagedMemory[0]))
            printk("fapDrv_construct: FAP%d: pManagedMemory=%x. wastage %d bytes\n",
                fapIdx, (int)pHostPsmGbl(fapIdx)->pManagedMemory,
                (int)pHostPsmGbl(fapIdx)->pManagedMemory - (int)&pHostPsmGbl(fapIdx)->ManagedMemory[0]);

        pHostPsmGbl(fapIdx)->mtuOverride = 0;
    }
    bindToBcmPktDma();

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    if (bcm_iq_register_hw(&fapIq_info_db))
        printk("\nFail to register FAP to Ingress QoS!");
    else
        printk("Complete registering FAP to Ingress QoS");

    /* bind Ingress QoS hooks */
    iqos_fap_ethRxDqmQueue_hook_g = fapIq_EthRxQueueUsage;
    iqos_fap_xtmRxDqmQueue_hook_g = fapIq_XtmRxQueueUsage;
#endif

//    fapHostHw_PrintRegs(FAP_CNTRL_REG);

#if defined(CONFIG_BCM_FAP_LAYER2)
    ret = register_netdevice_notifier(&fapNotifierBlock);
    if (ret != 0)
    {
        printk("WARNING -- could not register_netdevice_notifier for FAP!\n");
    }
#endif

#if defined(CC_FAP4KE_TM)
    fapTm_init();
#endif

#if defined(CC_FAP4KE_PERF) && (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
    fapPerf_construct();
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    return 0;
#else
    return FAPDRV_MAJOR;
#endif
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fapDrv_destruct
 * Description  : Final function that is called when the module is unloaded.
 *                See fapConfig.c
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
void __exit fapDrv_destruct(void)
{
#if defined(CONFIG_BCM_FAP_GSO)

#if defined(CONFIG_BCM_FAP_GSO_LOOPBACK)
    fapGsoLoopBk_uninit();
#endif

#endif

#if defined(CONFIG_BCM_FAP_LAYER2)
    unregister_netdevice_notifier(&fapNotifierBlock);
#endif

    fapProtoDestruct();

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    /* unbind BPM hooks */
    gbpm_fap_evt_hook_g    = (gbpm_fap_evt_hook_t)NULL;
    gbpm_g.fap_status      = (gbpm_fap_status_hook_t)NULL;
    gbpm_g.fap_thresh      = (gbpm_fap_thresh_hook_t)NULL;
    gbpm_g.fap_enet_thresh = (gbpm_fap_enet_thresh_hook_t)NULL;
    gbpm_g.fap_upd_buf_lvl = (gbpm_fap_upd_buf_lvl_hook_t)NULL;
#endif

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    bcm_iq_unregister_hw((iq_hw_info_t *)&fapIq_info_db);
    iqos_fap_ethRxDqmQueue_hook_g = NULL;
    iqos_fap_xtmRxDqmQueue_hook_g = NULL;
#endif

    unregister_chrdev(FAPDRV_MAJOR, FAPDRV_NAME);

    /* Unregister from receiving MAC address events */
    kerSysMacAddressNotifyBind(NULL);

#if defined(CC_FAP4KE_PERF) && (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
    fapPerf_destruct();
#endif

    BCM_LOG_NOTICE(BCM_LOG_ID_FAP, FAP_MODNAME " Char Driver " FAP_VER_STR
                   " Unregistered<%d>", FAPDRV_MAJOR );
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fapDrv_psmAlloc
 * Description  : Allocate size bytes from the ManagedMemory space in PSM
 * Returns      : HOST-based pointer to available space or FAP4KE_OUT_OF_PSM.
 * NOTE         : Executed from HOST MIPs only.
 *------------------------------------------------------------------------------
 */
uint8 * fapDrv_psmAlloc( uint32 fapIdx, int size )
{
    uint8 * p;

    BCM_ASSERT(isValidFapIdx(fapIdx));
    BCM_ASSERT(size != 0);
    BCM_ASSERT(!in_atomic());

    /* FAP_DM_P2: move this to managed memory */
    /* NOTE: make it so you can permenantly allocate memory from dynamic memory.  This would
       remove the block from the linked list (making walks faster).  It would unfortunately
       remove the truism that bh->next = bh + size... */
    p = pHostPsmGbl(fapIdx)->pManagedMemory;

    BCM_ASSERT(p != NULL);

    if((p + size) > (pHostPsmGbl(fapIdx)->ManagedMemory + FAP_PSM_MANAGED_MEMORY_SIZE))
    {
        printk("fapDrv_psmAlloc: out of memory: fapIdx=%d, size=%d, ManagedMemory=%p, totalSize=%d\n",
               fapIdx, size, pHostPsmGbl(fapIdx)->ManagedMemory, FAP_PSM_MANAGED_MEMORY_SIZE);
        return( FAP4KE_OUT_OF_PSM );
    }

    /* round next ptr up to next 16 byte boundary */
    pHostPsmGbl(fapIdx)->pManagedMemory = (uint8 *)(((unsigned int)(p + size) + 0xF) & ~0xF);

    if(pHostPsmGbl(fapIdx)->pManagedMemory != (p + size))
        printk("fapDrv_psmAlloc: wastage %d bytes\n", pHostPsmGbl(fapIdx)->pManagedMemory - (p + size));

    printk("fapDrv_psmAlloc: fapIdx=%d, size: %d, offset=%p bytes remaining %d\n",
        fapIdx, size, p,
        (int)(pHostPsmGbl(fapIdx)->ManagedMemory + FAP_PSM_MANAGED_MEMORY_SIZE) - (int)pHostPsmGbl(fapIdx)->pManagedMemory);

    return( p );
}



#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
/*
 *------------------------------------------------------------------------------
 * Function Name: fapBpm_updBufLvl
 * Description  : Updates the current BPM buffer level in all 4KEs
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
void fapBpm_updBufLvl( int lvl )
{
    uint32 fapIdx;
    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
            pHostPsmBpm(fapIdx)->buf_thresh_lvl = lvl;
    }
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fapBpm_dumpStatus
 * Description  : Sends dump BPM Status message to FAP.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
void fapBpm_dumpStatus( void )
{
    xmit2FapMsg_t   fapMsg;
    uint32          fapIdx;
#if (NUM_FAPS > 1)
    int delay = 0;
#endif

    fapMsg.status.cmd     = FAPMSG_CMD_DUMP_BPM_STATUS;
    fapMsg.status.drv     = FAPMSG_DRV_ENET;
    fapMsg.status.status  = 0;

    fapIdx = 0;
    fapDrv_Xmit2Fap(fapIdx, FAP_MSG_BPM, &fapMsg);

    printk(FAP_IDX_FMT "DQM: rdDqm<%u>, wrDqm<%u> "
           "EvtCnt: rdEvt<%u>, wrEvt<%u> EvtQ: head<%u> tail<%u>\n",
            fapIdx,
            fapDrvBpmStats[fapIdx].rdDqm, fapDrvBpmStats[fapIdx].wrDqm,
            fapDrvBpmStats[fapIdx].rdEvt, fapDrvBpmStats[fapIdx].wrEvt,
            bpmPendEvtQ_gp->headIdx, bpmPendEvtQ_gp->tailIdx);
 
#if (NUM_FAPS > 1)
    fapIdx = 1;
    printk(FAP_IDX_FMT "DQM: rdDqm<%u>, wrDqm<%u> "
           "EvtCnt: rdEvt<%u>, wrEvt<%u> EvtQ: head<%u> tail<%u>\n",
            fapIdx,
            fapDrvBpmStats[fapIdx].rdDqm, fapDrvBpmStats[fapIdx].wrDqm,
            fapDrvBpmStats[fapIdx].rdEvt, fapDrvBpmStats[fapIdx].wrEvt,
            bpmPendEvtQ_gp->headIdx, bpmPendEvtQ_gp->tailIdx);
    delay += HZ/4;
    fapDrv_Xmit2FapDelayed(fapIdx, FAP_MSG_BPM, &fapMsg, delay);
#endif
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fapBpm_dumpTxQThresh
 * Description  : Sends dump BPM TxQ Thresh message to FAP.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
void fapBpm_dumpTxQThresh( void )
{
    xmit2FapMsg_t   fapMsg;
    uint32          fapIdx;

    fapMsg.status.cmd     = FAPMSG_CMD_DUMP_TXQ_BPM_THRESH;
    fapMsg.status.drv     = FAPMSG_DRV_ENET;
    fapMsg.status.status  = 0;

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_BPM, &fapMsg);
    }
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fapBpm_dumpEthTxQThresh
 * Description  : Sends dump BPM Eth TxQ Thresh message to FAP.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
void fapBpm_dumpEthTxQThresh( void )
{
    xmit2FapMsg_t   fapMsg;
    uint32          fapIdx;

    fapMsg.status.cmd     = FAPMSG_CMD_DUMP_BPM_ETH_TXQ_THRESH;
    fapMsg.status.drv     = FAPMSG_DRV_ENET;
    fapMsg.status.status  = 0;

    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {
        fapDrv_Xmit2Fap(fapIdx, FAP_MSG_BPM, &fapMsg);
    }
}


/*
 *------------------------------------------------------------------------------
 * Function Name: fapBpm_allocBufResp_Dqm
 * Description  : Sends BPM buffer allocation response message to FAP.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
void fapBpm_allocBufResp_Dqm( uint8 drv, uint8 channel, uint8 seqId,
        uint16 numBufs )
{
    xmit2FapMsg_t fapMsg;
    uint32 fapIdx;

    BCM_ASSERT( (drv <= FAPMSG_DRV_FAP) && (channel < XTM_RX_CHANNELS_MAX) );
    BCM_ASSERT( (numBufs <= 512) );

    if (FAPMSG_DRV_FAP == drv)
        fapIdx = channel;
    else if (FAPMSG_DRV_XTM == drv)
        fapIdx = getFapIdxFromXtmRxIudma(channel);
    else
        fapIdx = getFapIdxFromEthRxIudma(channel);

    BCM_ASSERT(fapIdx < NUM_FAPS);

    fapMsg.allocBuf.cmd     = FAPMSG_CMD_ALLOC_BUF_RESP;
    fapMsg.allocBuf.drv     = drv;
    fapMsg.allocBuf.channel = channel;
    fapMsg.allocBuf.seqId   = seqId;
    fapMsg.allocBuf.numBufs = numBufs;

    fapDrv_Xmit2Fap(fapIdx, FAP_MSG_BPM, &fapMsg);
    fapDrvBpmStats[fapIdx].wrDqm++;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: fapBpm_freeBufResp_Dqm
 * Description  : Sends BPM buffer free response message to FAP.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
void fapBpm_freeBufResp_Dqm( uint8 fapIdx, uint8 seqId, uint16 numBufs)
{
    xmit2FapMsg_t fapMsg;

    BCM_ASSERT( (numBufs <= 512) );

    BCM_ASSERT( fapIdx < NUM_FAPS );

    fapMsg.freeBuf.cmd     = FAPMSG_CMD_FREE_BUF_RESP;
    fapMsg.freeBuf.fapIdx  = fapIdx;
    fapMsg.freeBuf.seqId   = seqId;
    fapMsg.freeBuf.numBufs = numBufs;

    fapDrv_Xmit2Fap(fapIdx, FAP_MSG_BPM, &fapMsg);
    fapDrvBpmStats[fapIdx].wrDqm++;
}


/*
 *------------------------------------------------------------------------------
 * Function Name: fapBpm_pendEvtHandle
 * Description  : Processes the BPM pending events like: buffer alloc, buffer
 *                free, etc. Finally these events send response to FAP.
 * Returns      : 1 on success; 0 otherwise.
 *------------------------------------------------------------------------------
 */
int fapBpm_pendEvtHandle( DQMQueueDataReg_S *msg_p )
{
    uint32 cmd  = msg_p->word1;
    uint8  drv = ((msg_p->word2 >> 28) & 0x0F);
    uint8  channel = ((msg_p->word2 >> 24) & 0x0F);
    uint8  seqId = ((msg_p->word2 >> 16) & 0xFF);
    uint16 numBufs = (msg_p->word2 & 0xFFFF);
    void ** mappedMemAddr;
    uint32 fapIdx;

    BCM_ASSERT( (drv <= FAPMSG_DRV_FAP) && (channel < XTM_RX_CHANNELS_MAX) );
    BCM_ASSERT( numBufs <= 512 );

    BCM_LOG_DEBUG( BCM_LOG_ID_FAP,
            "FAP BpmEvtQ cmd: %d drv: %d channel: %d "
            "seqId: %d numBufs %d", cmd, drv, channel, seqId, numBufs );

    switch (cmd)
    {
        case HOSTMSG_CMD_ALLOC_BUF_REQT:
        {
            if (drv == FAPMSG_DRV_FAP)
            {
                /*reused channel as fapIdx*/
                fapIdx = channel;
                fapDrvBpmStats[fapIdx].rdEvt++;
                mappedMemAddr =
                    (void **) &pHostFapSdram(fapIdx)->bpm.bufAddr[0];
            }
            else if (drv == FAPMSG_DRV_ENET)
            {
                fapIdx = getFapIdxFromEthRxIudma(channel);
                fapDrvBpmStats[fapIdx].rdEvt++;
                mappedMemAddr =
                    (void **) &pHostFapSdram(fapIdx)->bpm.enet[channel].bufAddr[0];
            }
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
           else if (drv == FAPMSG_DRV_XTM)
            {
                fapIdx = getFapIdxFromXtmRxIudma(channel);
                fapDrvBpmStats[fapIdx].rdEvt++;
                mappedMemAddr =
                    (void **) &pHostFapSdram(fapIdx)->bpm.xtm[channel].bufAddr[0];
            }
#endif
            else
            {
                BCM_LOG_ERROR( BCM_LOG_ID_FAP, "BPM Alloc Reqt Msg invalid drv=%d", drv);
                return 0;
            }

            if (gbpm_alloc_mult_buf(numBufs, mappedMemAddr)
                    == GBPM_ERROR )
            {
                BCM_LOG_NOTICE( BCM_LOG_ID_FAP, "FAP FapIF alloc buf reqt" );
                /* Send an error response by setting numBufs to 0 */
                fapBpm_allocBufResp_Dqm( drv, channel, seqId, 0);
                return GBPM_ERROR;
            }

#if defined(CC_BPM_DBG)
            {
                void ** memPtr;
                memPtr = mappedMemAddr;
                printk( "Bufs Alloc:");
                printk( "\t" );
                for(i=0; i < numBufs; i++, memPtr++)
                    printk( "0x%p ", (void *)*memPtr);
                printk( "\n" );
            }
#endif

            fapBpm_allocBufResp_Dqm( drv, channel, seqId, numBufs );
            break;
        }

        case HOSTMSG_CMD_FREE_BUF_REQT:
        {
            fapIdx = ((msg_p->word2 >> 24) & 0xFF);  /* Free Reqt only */
            BCM_ASSERT( fapIdx < NUM_FAPS );
            fapDrvBpmStats[fapIdx].rdEvt++;

            mappedMemAddr = (void **) &pHostFapSdram(fapIdx)->bpm.freeBufList[seqId][0];

            BCM_LOG_DEBUG( BCM_LOG_ID_FAP, "mapped memAddr: %p", 
                (void *) mappedMemAddr );

#if defined(CC_BPM_DBG)
            {
                uint32 i;
                void ** memPtr;
                memPtr = mappedMemAddr;
                printk( "Bufs Freed:\n");
                printk( "\t" );
                for(i=0; i < numBufs; i++, memPtr++)
                {
                    printk( "0x%p ", (void *) *memPtr);
                }
                printk( "\n" );
            }
#endif

            gbpm_free_mult_buf( numBufs, mappedMemAddr );
            fapBpm_freeBufResp_Dqm( fapIdx, seqId, numBufs );
            break;
        }

        default:
            BCM_LOG_ERROR( BCM_LOG_ID_FAP, "Invalid BPM Msg command %d", cmd );
            return 0;
    }

    return 1;
}


/*
 *------------------------------------------------------------------------------
 * Function Name: fapBpm_schedEvt
 * Description  : Schedules events from BPM event pending queue.
 * Returns      : None
 *------------------------------------------------------------------------------
 */
void fapBpm_schedEvt(void)
{
    DQMQueueDataReg_S *msg_p;

    while( bpmPendEvtQ_gp->headIdx != bpmPendEvtQ_gp->tailIdx )
    {
        BCM_LOG_DEBUG( BCM_LOG_ID_FAP, "headIdx=%d tailIdx=%d",
            (int) bpmPendEvtQ_gp->headIdx, (int) bpmPendEvtQ_gp->tailIdx);

        msg_p = &bpmPendEvtQ_gp->msg[bpmPendEvtQ_gp->headIdx];

        BCM_LOG_DEBUG( BCM_LOG_ID_FAP,
            "\tBpmEvtQ: msg: 0x%08x 0x%08x 0x%08x 0x%08x\n",
            (uint32_t) msg_p->word0, (uint32_t) msg_p->word1,
            (uint32_t) msg_p->word2, (uint32_t) msg_p->word3);

        fapBpm_pendEvtHandle( msg_p );

        bpmPendEvtQ_gp->headIdx++;
        bpmPendEvtQ_gp->headIdx %= FAP_MAX_BPM_EVT;
    }
}


/*
 *------------------------------------------------------------------------------
 * Function Name: fapBpm_eventQ_init
 * Description  : Initializes BPM event pending queue.
 * Returns      : None
 *------------------------------------------------------------------------------
 */
void fapBpm_eventQ_init( void )
{
    memset( bpmPendEvtQ_gp, 0, sizeof(bpmPendEvtQ_t) );
}


/*
 *------------------------------------------------------------------------------
 * Function Name: fapBpm_eventQ_put
 * Description  : Queues one event at the tail of BPM event pending queue.
 * Returns      : 1 on success; 0 otherwise.
 *------------------------------------------------------------------------------
 */
int fapBpm_eventQ_put( DQMQueueDataReg_S msg )
{
    uint32 tailIdx = bpmPendEvtQ_gp->tailIdx;
    DQMQueueDataReg_S *msg_p = &bpmPendEvtQ_gp->msg[tailIdx];

    BCM_LOG_DEBUG( BCM_LOG_ID_FAP, "\tBpmEvtQ: headIdx=%d tailIdx=%d",
        (int) bpmPendEvtQ_gp->headIdx, (int) bpmPendEvtQ_gp->tailIdx);

    BCM_LOG_DEBUG( BCM_LOG_ID_FAP,
        "\tBpmEvtQ: msg: 0x%08x 0x%08x 0x%08x 0x%08x\n",
        (uint32_t) msg.word0, (uint32_t) msg.word1,
        (uint32_t) msg.word2, (uint32_t) msg.word3);

    if ( ( (tailIdx + 1) % FAP_MAX_BPM_EVT) == bpmPendEvtQ_gp->headIdx )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_FAP, "\tBpmEvtQ: queue full\n");
        return 0;
    }
    else
    {
        msg_p->word0 = msg.word0;
        msg_p->word1 = msg.word1;
        msg_p->word2 = msg.word2;
        msg_p->word3 = msg.word3;

        tailIdx++;
        tailIdx %= FAP_MAX_BPM_EVT;
        bpmPendEvtQ_gp->tailIdx = tailIdx;
        wake_up_process(fapDrvTask);
    }
    return 1;
}


/* --------------------------------------------------------------------------
    Name: fapIf_bpmHandle
 Purpose: Receive BPM message
  Return: 1 on success; 0 otherwise
   Notes: This is executed on the Host to receive a message from FAP4KE.
-------------------------------------------------------------------------- */
int fapIf_bpmHandle( uint32 fapIdx, DQMQueueDataReg_S msg )
{
    uint32  cmd = msg.word1;
//    uint8  drv = ((msg.word2 >> 28) & 0x0F);
    uint8  channel = ((msg.word2 >> 24) & 0x0F);
//    uint8  seqId = ((msg.word2 >> 16) & 0xFF);
    uint16 numBufs = (msg.word2 & 0xFFFF);

//    BCM_ASSERT( (drv <= FAPMSG_DRV_XTM) && (channel < XTM_RX_CHANNELS_MAX) );
    BCM_ASSERT( (numBufs <= 512) );
    BCM_LOG_DEBUG( BCM_LOG_ID_FAP,
        "FAP FapIF cmd: %d channel: %d numBufs %d ", cmd, channel, numBufs );

    fapDrvBpmStats[fapIdx].rdDqm++;
    switch (cmd)
    {
        case HOSTMSG_CMD_ALLOC_BUF_REQT:
        case HOSTMSG_CMD_FREE_BUF_REQT:
        {
            if (fapBpm_eventQ_put( msg ))
                fapDrvBpmStats[fapIdx].wrEvt++;
            break;
        }

        default:
            BCM_LOG_ERROR( BCM_LOG_ID_FAP, "Invalid BPM Msg command %d", cmd );
            return 0;
    }

    return 1;
}
#endif

#if defined(CONFIG_BCM_FAP_LAYER2) && defined(CC_FAP_MANAGE_HW_ARL)
int fapPkt_hwArlUpdate(hostMsgGroups_t msgType, fapMsg_arlEntry_t *arlEntry_p);
#endif

/******************************************************************************
* Function: fapIf_recv_dqmhandler                                             *
*                                                                             *
* Description: Handles incoming pkts on the fap IF interfaces                 *
******************************************************************************/
void fapIf_recv_dqmhandler(uint32 fapIdx, unsigned long unused)
{
    DQMQueueDataReg_S msg;
    uint32            dqm = DQM_FAP2HOST_HOSTIF_Q;
    uint32            qbit = 1 << dqm;
    hostMsgGroups_t   msgType;

    /* clear the interrupt */
    dqmClearNotEmptyIrqStsHost(fapIdx, qbit);

    while(dqmRecvAvailableHost(fapIdx, dqm))
    {
        dqmRecvMsgHost(fapIdx, dqm, DQM_FAP2HOST_HOSTIF_Q_SIZE, &msg);

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        fapDrvDqmStats[fapIdx].fap2HostHostIf++;
#endif

        msgType = msg.word0 >> 24;

        switch(msgType)
        {

#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
            case HOST_MSG_BPM:
            {
                BCM_LOG_INFO( BCM_LOG_ID_FAP, "recvd HOST_MSG_BPM" );
                fapIf_bpmHandle( fapIdx, msg );
                break;
            }
#endif

            case HOST_MSG_FREE_DYN_MEM:
            {
                BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "recvd HOST_MSG_FREE_DYN_MEM");
                fapPkt_dynMemFreeIsr( fapIdx, msg );
                break;
            }

#if defined(CONFIG_BCM_FAP_LAYER2)
            case HOST_MSG_ARL_ADD:
            case HOST_MSG_ARL_REMOVE:
            {
#if (NUM_FAPS > 1)
                xmit2FapMsg_t fapMsg;
                uint32 otherFapIdx;
                fapMsgGroups_t fapMsgType = (msgType == HOST_MSG_ARL_ADD) ?
                    FAP_MSG_ARL_ADD : FAP_MSG_ARL_REMOVE;

                /* send notification to the other FAP */
                otherFapIdx = (fapIdx == 0) ? 1 : 0;

                fapMsg.arlEntry.word[0] = msg.word0;
                fapMsg.arlEntry.word[1] = msg.word1;
                fapMsg.arlEntry.word[2] = msg.word2;
                fapMsg.arlEntry.word[3] = msg.word3;

                if(bcmLog_logIsEnabled(BCM_LOG_ID_FAP, BCM_LOG_LEVEL_INFO))
                {
                    __dumpArlNotify(fapIdx, msgType, &fapMsg.arlEntry);
                }

                fapDrv_Xmit2Fap(otherFapIdx, fapMsgType, &fapMsg);

#if defined(CC_FAP_MANAGE_HW_ARL)
                if(fapPkt_hwArlUpdate(msgType, &fapMsg.arlEntry))
#endif
                {
                    bcmPktDma_arlNotify(msgType, &fapMsg.arlEntry);
                }
#endif
                break;
            }
#endif
            default:
                BCM_LOG_ERROR( BCM_LOG_ID_FAP,
                               "Unknown Fap IF msg type %u \n", msgType);
                break;
        }
    }

    /* enable the interrupt */
    dqmEnableNotEmptyIrqMskHost(fapIdx, qbit);
}


module_init(fapDrv_construct);
module_exit(fapDrv_destruct);

#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
EXPORT_SYMBOL(fap_bind_arl);
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
EXPORT_SYMBOL(fapBpm_schedEvt);
#endif
EXPORT_SYMBOL(fapDrv_Xmit2Fap);

EXPORT_SYMBOL(fapDrv_psmAlloc);

#if defined(CONFIG_BCM_FAP_LAYER2)
EXPORT_SYMBOL(fapPkt_setFloodingMask);
EXPORT_SYMBOL(fapPkt_arlFlush);
EXPORT_SYMBOL(fapL2flow_defaultVlanTagConfig);
#if defined(CC_FAP_MANAGE_HW_ARL)
EXPORT_SYMBOL(fapPkt_hwArlConfig);
#endif
#endif

EXPORT_SYMBOL(fapPkt_mcastSetMissBehavior);

MODULE_DESCRIPTION(FAP_MODNAME);
MODULE_VERSION(FAP_VERSION);
MODULE_LICENSE("Proprietary");
