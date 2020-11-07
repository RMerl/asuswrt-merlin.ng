#ifndef __FAP4KE_LOCAL_H_INCLUDED__
#define __FAP4KE_LOCAL_H_INCLUDED__

/*

 Copyright (c) 2007 Broadcom Corporation
 All Rights Reserved

<:label-BRCM:2011:DUAL/GPL:standard

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
 * File Name  : fap4ke_local.h
 *
 * Description: This file contains global definitions for the BCM63268 FAP MIPS.
 *
 *******************************************************************************
 */

#include "fap4keLib_types.h"
#include "fap4keLib_string.h"
#include "bcmtypes.h"
#include "fap4ke_mailBox.h"
#include "fap4ke_printer.h"
#include "fap4ke_memory.h"

#if defined(CONFIG_BCM963268)
#define FAP4KE_MIPS_CLK_PERIOD_NSEC  5
#define FAP4KE_MIPS_CLK_HZ           200000000
#else
#error "Unsupported FAP based Chip"
#endif

#define CC_FAP4KE_PM_CPU
#define CC_FAP4KE_PM_CPU_PRECISION   5

#ifndef uint64
#define uint64 unsigned long long
#endif

#ifndef int64
#define int64 signed long long
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define HOST_DEBUG_REG(_x) HalDebugPrint(ZONE_INIT, "%s : 0x%08lX = 0x%08lX\n", #_x, (uint32)(&_x), _x)

#define fap4ke_readCp0StatusReg()                                       \
__extension__                                                           \
({                                                                      \
    uint32 __cp0StatusReg;                                              \
                                                                        \
    __asm__ volatile ("mfc0 %0, $12, 0" : "=d" (__cp0StatusReg));       \
                                                                        \
    /* This causes the value in __cp0StatusReg to be returned from   */ \
    /* the macro as if it was the return value from a function.      */ \
    /*return*/ __cp0StatusReg;                                          \
})

#define fap4ke_readCp0CauseReg()                                        \
__extension__                                                           \
({                                                                      \
    uint32 __cp0CauseReg;                                               \
                                                                        \
    __asm__ volatile ("mfc0 %0, $13, 0" : "=d" (__cp0CauseReg));        \
                                                                        \
    /*return*/ __cp0CauseReg;                                           \
})

#define fap4ke_readCp0EPCReg()                                          \
__extension__                                                           \
({                                                                      \
    uint32 __cp0EPCReg;                                                 \
                                                                        \
    __asm__ volatile ("mfc0 %0, $14, 0" : "=d" (__cp0EPCReg));          \
                                                                        \
    /*return*/ __cp0EPCReg;                                             \
})

#define fap4ke_writeCp0EPCReg(__cp0EPCReg)                              \
__extension__                                                           \
({                                                                      \
    __asm__ volatile ("mtc0 %0, $14, 0" : : "d" (__cp0EPCReg));         \
})

#define fap4ke_readCp0CountReg()                                        \
__extension__                                                           \
({                                                                      \
    uint32 __cp0CountReg;                                               \
                                                                        \
    __asm__ volatile ("mfc0 %0, $9, 0" : "=d" (__cp0CountReg));         \
                                                                        \
    /*return*/ __cp0CountReg;                                           \
})

#define fap4ke_readMipsReg(_regNbr)                                     \
__extension__                                                           \
({                                                                      \
    uint32 __reg;                                                       \
                                                                        \
    __asm__ volatile ("or %0, " #_regNbr ", $0" : "=d" (__reg));        \
                                                                        \
    /* This causes the value in __reg to be returned from the macro  */ \
    /* as if it was the return value from a function.                */ \
    /*return*/ __reg;                                                   \
})

#define fap4ke_readMipsRegSp() fap4ke_readMipsReg($29)
#define fap4ke_readMipsRegRa() fap4ke_readMipsReg($31)

#if 1
#define FAP4KE_SEND_DEBUG_WORD()                                \
    do {                                                        \
        fapMailBox_4keSendWord(fap4ke_readCp0StatusReg());      \
        fapMailBox_4keSendWord(fap4ke_readMipsRegSp());         \
        fapMailBox_4keSendWord(fap4ke_readMipsRegRa());         \
        fapMailBox_4keSendWord(0xffffffff);                     \
    } while(0)
#else
#define FAP4KE_SEND_DEBUG_WORD()
#endif

#define FAP4KE_MIPS_CP0_SR_IE_BIT 0x00000001
#define FAP4KE_MIPS_CP0_SR_IM_BIT 0x0000FC00

#if 1
#define FAP4KE_LOCK(_flags) ( (_flags) = __fap4ke_lock() )
#define FAP4KE_UNLOCK(_flags) ( __fap4ke_unlock(_flags) )
#else
#define FAP4KE_LOCK(_flags)
#define FAP4KE_UNLOCK(_flags)
#endif

uint32 __fap4ke_lock(void);
void __fap4ke_unlock(uint32 flags);

/* CP0 Counter register is incremented at every other
   clock cycle: 4KE Frequency = 200MHz */

#if defined(CC_FAP4KE_PMON)
#if 0 /* re-entrant */
#define FAP4KE_PMON_DECLARE()

#define FAP4KE_PMON_BEGIN(_pmonId)                    \
    do {                                              \
        p4kePmon->icachehit[(_pmonId)] =       \
            _4kePerfMeasReg->ICacheHit;               \
        p4kePmon->icachemiss[(_pmonId)] =      \
            _4kePerfMeasReg->ICacheMiss;              \
        p4kePmon->instncomplete[(_pmonId)] =   \
            _4kePerfMeasReg->InstnComplete;           \
        p4kePmon->halfCycles[(_pmonId)] =      \
            fap4ke_readCp0CountReg();                 \
    } while(0)

#define FAP4KE_PMON_END(_pmonId)                        \
    do {                                                \
        p4kePmon->halfCycles[(_pmonId)] =        \
            fap4ke_readCp0CountReg() -                  \
            p4kePmon->halfCycles[(_pmonId)];     \
        p4kePmon->instncomplete[(_pmonId)] =     \
            _4kePerfMeasReg->InstnComplete -            \
            p4kePmon->instncomplete[(_pmonId)];  \
        p4kePmon->icachemiss[(_pmonId)] =        \
            _4kePerfMeasReg->ICacheMiss -               \
            p4kePmon->icachemiss[(_pmonId)];     \
        p4kePmon->icachehit[(_pmonId)] =         \
            _4kePerfMeasReg->ICacheHit -                \
            p4kePmon->icachehit[(_pmonId)];      \
    } while(0)
#else /* better accuracy and performance */
#define FAP4KE_PMON_DECLARE() uint32 _pmonCycles, _pmonInst, _pmonIcacheHit, _pmonIcacheMiss, _pmonIrq

#define FAP4KE_PMON_BEGIN(_pmonId)                      \
    do {                                                \
        _pmonIcacheHit = _4kePerfMeasReg->ICacheHit;    \
        _pmonIcacheMiss = _4kePerfMeasReg->ICacheMiss;  \
        _pmonIrq = p4kePmon->globalIrqs;         \
        _pmonInst = _4kePerfMeasReg->InstnComplete;     \
        _pmonCycles = fap4ke_readCp0CountReg();         \
    } while(0)

#define FAP4KE_PMON_END(_pmonId)                                \
    do {                                                        \
        p4kePmon->halfCycles[(_pmonId)] =                \
            fap4ke_readCp0CountReg() - _pmonCycles;             \
        p4kePmon->instncomplete[(_pmonId)] =             \
            _4kePerfMeasReg->InstnComplete - _pmonInst;         \
        p4kePmon->interrupts[(_pmonId)] =                \
            p4kePmon->globalIrqs - _pmonIrq;             \
        p4kePmon->icachemiss[(_pmonId)] =                \
            _4kePerfMeasReg->ICacheMiss - _pmonIcacheMiss;      \
        p4kePmon->icachehit[(_pmonId)] =                 \
            _4kePerfMeasReg->ICacheHit - _pmonIcacheHit;        \
    } while(0)
#endif
#else
#define FAP4KE_PMON_DECLARE()
#define FAP4KE_PMON_BEGIN(_text)
#define FAP4KE_PMON_END(_text)
#endif

#define fap4ke_nop50()                                                  \
__extension__                                                           \
({                                                                      \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
    __asm__ volatile ("nop");                                           \
})

#ifdef FAP_4KE
/* The following cannot be used by host */
static inline void debugList(PDll_t list)
{
    PDll_t entry = list->next_p;

    fap4kePrt_Debug("*** list<0x%08lX>: next<0x%08lX>, prev<0x%08lX> ***\n",
                    (uint32)list,
                    (uint32)list->next_p, (uint32)list->prev_p);

    while(entry != list)
    {
        fap4kePrt_Debug("-> entry<0x%08lX>: next<0x%08lX>, prev<0x%08lX>\n",
                        (uint32)entry,
                        (uint32)entry->next_p, (uint32)entry->prev_p);

        entry = entry->next_p;
    }

    fap4kePrt_Debug("*** List End ***\n");
}
#endif

void dumpPacket(uint8 *packet_p, uint32 length);
void dumpHeader(uint8 *packet_p);
uint32 fap4ke_rand(void);

#if defined(CC_FAP4KE_TRACE)
void fap4keTrace_record(fap4keTrace_id_t id, uint32_t arg, fap4keTrace_type_t type);
void fap4keTrace_init(void);
void fap4keTrace_dump(void);
#else
#define fap4keTrace_record(id, arg, type)
#define fap4keTrace_init()
#define fap4keTrace_dump()
#endif

#endif  /* defined(__FAP4KE_LOCAL_H_INCLUDED__) */
