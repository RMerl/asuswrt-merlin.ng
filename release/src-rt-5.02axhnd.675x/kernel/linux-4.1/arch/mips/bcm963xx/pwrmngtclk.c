#if defined(CONFIG_BCM_KF_MIPS_BCM963XX) && defined(CONFIG_MIPS_BCM963XX)
/***********************************************************
 *
 * Copyright (c) 2009 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2009:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license 
 * agreement governing use of this software, this software is licensed 
 * to you under the terms of the GNU General Public License version 2 
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php, 
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give 
 *    you permission to link this software with independent modules, and 
 *    to copy and distribute the resulting executable under terms of your 
 *    choice, provided that you also meet, for each linked independent 
 *    module, the terms and conditions of the license of that module. 
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications 
 *    of the software.  
 * 
 * Not withstanding the above, under no circumstances may you combine 
 * this software in any way with any other Broadcom software provided 
 * under a license other than the GPL, without Broadcom's express prior 
 * written consent. 
 * 
 * :>
 *
 ************************************************************/
#include <linux/module.h>
#include <asm/time.h>
#include <bcm_map_part.h>
#include "board.h"

#if defined(CONFIG_BCM_KF_POWER_SAVE) && defined(CONFIG_BCM_HOSTMIPS_PWRSAVE)
#define CLK_ALIGNMENT_REG   0xff410040
#define KEEPME_MASK         0x00007F00 // bit[14:8]

#define RATIO_ONE_SYNC      0x0 /* 0b000 */
#define RATIO_ONE_ASYNC     0x1 /* 0b001 */
#define RATIO_ONE_HALF      0x3 /* 0b011 */
#define RATIO_ONE_QUARTER   0x5 /* 0b101 */
#define RATIO_ONE_EIGHTH    0x7 /* 0b111 */

#define MASK_ASCR_BITS 0x7
#define MASK_ASCR_SHFT 28
#define MASK_ASCR (MASK_ASCR_BITS << MASK_ASCR_SHFT)

unsigned int originalMipsAscr = 0; // To keep track whether MIPS was in Async mode to start with at boot time
unsigned int originalMipsAscrChecked = 0;
unsigned int keepme;
#endif

#if defined(CONFIG_BCM_PWRMNGT_MODULE)
#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
unsigned int self_refresh_enabled = 0; // Wait for the module to control if it is enabled or not
#endif
#if defined(CONFIG_BCM_KF_POWER_SAVE) && defined(CONFIG_BCM_HOSTMIPS_PWRSAVE)
unsigned int clock_divide_enabled = 0; // Wait for the module to control if it is enabled or not
#endif
#else
#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
unsigned int self_refresh_enabled = 1;
#endif
#if defined(CONFIG_BCM_KF_POWER_SAVE) && defined(CONFIG_BCM_HOSTMIPS_PWRSAVE)
unsigned int clock_divide_enabled = 1;
#endif
#endif

unsigned int clock_divide_low_power0 = 0;
unsigned int clock_divide_active0 = 0;
unsigned int wait_count0 = 0;
#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS)
unsigned int TimerC0Snapshot0 = 0;
unsigned int prevTimerCnt0, newTimerCnt0, TimerAdjust0;
#endif

#if defined(CONFIG_SMP)
unsigned int clock_divide_low_power1 = 0;
unsigned int clock_divide_active1 = 0;
unsigned int wait_count1 = 0;
#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS)
unsigned int TimerC0Snapshot1 = 0;
unsigned int prevTimerCnt1, newTimerCnt1, TimerAdjust1;
#endif
#endif

#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS)
unsigned int C0divider, C0multiplier, C0ratio, C0adder;
#endif
extern volatile int isVoiceIdle;
 
DEFINE_SPINLOCK(pwrmgnt_clk_irqlock);
 
#if defined(CONFIG_BCM_KF_POWER_SAVE) && defined(CONFIG_BCM_HOSTMIPS_PWRSAVE)
/* To put CPU in ASYNC mode and change CPU clock speed */
void __BcmPwrMngtSetASCR(unsigned int freq_div)
{
   register unsigned int temp;
   if (freq_div == RATIO_ONE_ASYNC) {
      // Gradually bring the processor speed back to 1:1
      // If it is done in one step, CP0 timer interrupts are missed.

      // E/ SYNC instruction   // Step E SYNC instruction  
      asm("sync" : : );

      // Step F1 change to 1/4
      asm("mfc0 %0,$22,5" : "=d"(temp) :);
      temp = ( temp & ~MASK_ASCR) | (RATIO_ONE_QUARTER << MASK_ASCR_SHFT);
      asm("mtc0 %0,$22,5" : : "d" (temp));

      // Step F2 change to 1/2
      temp = ( temp & ~MASK_ASCR) | (RATIO_ONE_HALF << MASK_ASCR_SHFT);
      asm("mtc0 %0,$22,5" : : "d" (temp));

      // Step F3 change to 1/1, high performance memory access
      temp = ( temp & ~MASK_ASCR);
      asm("mtc0 %0,$22,5" : : "d" (temp));

   } else {
      // E/ SYNC instruction   // Step E SYNC instruction  
      asm("sync" : : );

      // F/ change to 1/2, or 1/4, or 1/8 by setting cp0 sel 5 bits[30:28] (sel 4 bits[24:22] for single core mips)  to 011, 101, or 111 respectively
      // Step F change to 1/2, or 1/4, or 1/8 by setting cp0 bits[30:28]
      asm("mfc0 %0,$22,5" : "=d"(temp) :);
      temp = ( temp & ~MASK_ASCR) | (freq_div << MASK_ASCR_SHFT);
      asm("mtc0 %0,$22,5" : : "d" (temp));
   }

   return;
} /* BcmPwrMngtSetASCR */

void BcmPwrMngtSetASCR(unsigned int freq_div)
{
   unsigned long flags;

   if (!freq_div) {
      // Can't use this function to set to SYNC mode
      return;
   }

   spin_lock_irqsave(&pwrmgnt_clk_irqlock, flags);
   __BcmPwrMngtSetASCR(freq_div);
   spin_unlock_irqrestore(&pwrmgnt_clk_irqlock, flags);
   return;
} /* BcmPwrMngtSetASCR */
EXPORT_SYMBOL(BcmPwrMngtSetASCR);


/* To put CPU in SYNC mode and change CPU clock speed to 1:1 ratio */
/* No SYNC mode in newer MIPS core, use the __BcmPwrMngtSetASCR with ratio 1:1 instead */
void __BcmPwrMngtSetSCR(void)
{
   register unsigned int cp0_ascr_asc;

   // It is important to go back to divide by 1 async mode first, don't jump directly from divided clock back to SYNC mode.
   // A/ set cp0 reg 22 sel 5 bits[30:28]  (sel 4 bits[24:22] for single core mips)  to 001
   asm("mfc0 %0,$22,5" : "=d"(cp0_ascr_asc) :);
   if (!originalMipsAscrChecked) {
      originalMipsAscr = cp0_ascr_asc & MASK_ASCR;
      originalMipsAscrChecked = 1;
   }
   if (originalMipsAscr)
      return;
   cp0_ascr_asc = ( cp0_ascr_asc & ~MASK_ASCR) | (RATIO_ONE_ASYNC << MASK_ASCR_SHFT);
   asm("mtc0 %0,$22,5" : : "d" (cp0_ascr_asc));

   // B/ 16 nops // Was 32 nops (wait a while to make sure clk is back to full speed)
   asm("nop" : : ); asm("nop" : : );
   asm("nop" : : ); asm("nop" : : ); 
   asm("nop" : : ); asm("nop" : : );
   asm("nop" : : ); asm("nop" : : );
   asm("nop" : : ); asm("nop" : : );
   asm("nop" : : ); asm("nop" : : );
   asm("nop" : : ); asm("nop" : : );
   asm("nop" : : ); asm("nop" : : );

   // C/ SYNC instruction
   asm("sync" : : );


   // H/ set cp0 reg 22 sel 5 bits[30:28]  (sel 4 bits[24:22] for single core mips)  to 000
   asm("mfc0 %0,$22,5" : "=d"(cp0_ascr_asc) :);
   cp0_ascr_asc = ( cp0_ascr_asc & ~MASK_ASCR);
   asm("mtc0 %0,$22,5" : : "d" (cp0_ascr_asc));

   // I/ SYNC instruction 
   asm("sync" : : );

   return;
} /* BcmPwrMngtSetSCR */

void BcmPwrMngtSetSCR(void)
{
   unsigned long flags;

   spin_lock_irqsave(&pwrmgnt_clk_irqlock, flags);
   __BcmPwrMngtSetSCR();
   spin_unlock_irqrestore(&pwrmgnt_clk_irqlock, flags);

   return;
} /* BcmPwrMngtSetSCR */
EXPORT_SYMBOL(BcmPwrMngtSetSCR);


void BcmPwrMngtSetAutoClkDivide(unsigned int enable)
{
   printk("Host MIPS Clock divider pwrsaving is %s\n", enable?"enabled":"disabled");
   clock_divide_enabled = enable;
}
EXPORT_SYMBOL(BcmPwrMngtSetAutoClkDivide);


int BcmPwrMngtGetAutoClkDivide(void)
{
   return (clock_divide_enabled);
}
EXPORT_SYMBOL(BcmPwrMngtGetAutoClkDivide);
#endif

#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
void BcmPwrMngtSetDRAMSelfRefresh(unsigned int enable)
{
#if defined (CONFIG_BCM963381)
   if (0xA0 == ((PERF->RevID & REV_ID_MASK) & 0xF0)) {
      printk("DDR Self Refresh pwrsaving must not be enabled on 63381A0/A1\n");
      enable = 0;
   }
#endif

   printk("DDR Self Refresh pwrsaving is %s\n", enable?"enabled":"disabled");
   self_refresh_enabled = enable;

}
EXPORT_SYMBOL(BcmPwrMngtSetDRAMSelfRefresh);


int BcmPwrMngtGetDRAMSelfRefresh(void)
{
   return (self_refresh_enabled);
}
EXPORT_SYMBOL(BcmPwrMngtGetDRAMSelfRefresh);

#if defined(CONFIG_BCM_ADSL_MODULE) || defined(CONFIG_BCM_ADSL)
PWRMNGT_DDR_SR_CTRL *pDdrSrCtrl = NULL;
void BcmPwrMngtRegisterLmemAddr(PWRMNGT_DDR_SR_CTRL *pDdrSr)
{
    pDdrSrCtrl = pDdrSr;

    // Initialize tp0 to busy status and tp1 to idle
    // for cases where SMP is not compiled in.
    if(NULL != pDdrSrCtrl) {
        pDdrSrCtrl->word = 0;
        pDdrSrCtrl->tp0Busy = 1;
        pDdrSrCtrl->tp1Busy = 0;
    }
}
EXPORT_SYMBOL(BcmPwrMngtRegisterLmemAddr);
#else
PWRMNGT_DDR_SR_CTRL ddrSrCtl = {{.word=0}};
PWRMNGT_DDR_SR_CTRL *pDdrSrCtrl = &ddrSrCtl;
#endif
#endif

// Determine if cpu is busy by checking the number of times we entered the wait
// state in the last milisecond. If we entered the wait state only once or
// twice, then the processor is very likely not busy and we can afford to slow
// it down while on wait state. Otherwise, we don't slow down the processor
// while on wait state in order to avoid affecting the time it takes to
// process interrupts
void BcmPwrMngtCheckWaitCount (void)
{
    int cpu = smp_processor_id();

    if (cpu == 0) {
#if defined(CONFIG_SMP) && defined(CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS)
        if (isVoiceIdle && TimerC0Snapshot1) {
#else
        if (isVoiceIdle) {
#endif
           if (wait_count0 > 0 && wait_count0 < 3) {
              clock_divide_low_power0 = 1;
           }
           else {
              clock_divide_low_power0 = 0;
           }
        }
        else {
           clock_divide_low_power0 = 0;
        }
        wait_count0 = 0;
    }
#if defined(CONFIG_SMP)
    else {
#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS)
        if (TimerC0Snapshot1) {
#else
        {
#endif
           if (wait_count1 > 0 && wait_count1 < 3) {
              clock_divide_low_power1 = 1;
           }
           else {
              clock_divide_low_power1 = 0;
           }
        }
        wait_count1 = 0;
    }
#endif
}

// When entering wait state, consider reducing the MIPS clock speed.
// Clock speed is reduced if it has been determined that the cpu was
// mostly idle in the previous milisecond. Clock speed is reduced only
// once per 1 milisecond interval.
void BcmPwrMngtReduceCpuSpeed (void)
{
    int cpu = smp_processor_id();
    unsigned long flags;

    spin_lock_irqsave(&pwrmgnt_clk_irqlock, flags);

    if (cpu == 0) {
        // Slow down the clock when entering wait instruction
        // only if the cpu is not busy
        if (clock_divide_low_power0) {
            if (wait_count0 < 2) {
                clock_divide_active0 = 1;
#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
                if (pDdrSrCtrl && self_refresh_enabled) {
                    // Communicate TP status to PHY MIPS
                    pDdrSrCtrl->tp0Busy = 0;
                }
#endif
            }
        }
        wait_count0++;
    }
#if defined(CONFIG_SMP)
    else {
        // Slow down the clock when entering wait instruction
        // only if the cpu is not busy
        if (clock_divide_low_power1) {
            if (wait_count1 < 2) {
                clock_divide_active1 = 1;
#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
                if (pDdrSrCtrl && self_refresh_enabled) {
                    // Communicate TP status to PHY MIPS
                    pDdrSrCtrl->tp1Busy = 0;
                }
#endif
            }
        }
        wait_count1++;
    }
#endif

#if defined(CONFIG_SMP)
    if (clock_divide_active0 && clock_divide_active1) {
#else
    if (clock_divide_active0) {
#endif
#if defined(CONFIG_BCM_KF_POWER_SAVE) && defined(CONFIG_BCM_HOSTMIPS_PWRSAVE)
        if (clock_divide_enabled) {
            __BcmPwrMngtSetASCR(RATIO_ONE_EIGHTH);
		}
#endif

#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
        // Place DDR in self-refresh mode if enabled and other processors are OK with it
        if (pDdrSrCtrl && !pDdrSrCtrl->word && self_refresh_enabled) {
            // Below defines are CHIP Specific - refer to xxxx_map_part.h
#if defined(DMODE_1_DRAMSLEEP)
            DDR->DMODE_1 |= DMODE_1_DRAMSLEEP;
#elif defined(MEMC_SELF_REFRESH)
            MEMC->Control |= MEMC_SELF_REFRESH;
#elif defined(CFG_DRAMSLEEP)
            MEMC->DRAM_CFG |= CFG_DRAMSLEEP;
#elif defined(SELF_REFRESH_CMD)
            MEMC->SDR_CFG.DRAM_CMD[SELF_REFRESH_CMD] = 0;
#else
            #error "DDR Self refresh definition missing in xxxx_map_part.h for this chip"
#endif
        }
#endif
    }
    spin_unlock_irqrestore(&pwrmgnt_clk_irqlock, flags);
}

// Full MIPS clock speed is resumed on the first interrupt following
// the wait instruction. If the clock speed was reduced, the MIPS
// C0 counter was also slowed down and its value needs to be readjusted.
// The adjustments are done based on a reliable timer from the peripheral
// block, timer2. The adjustments are such that C0 will never drift
// but will see minor jitter.
void BcmPwrMngtResumeFullSpeed (void)
{
#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS)
    unsigned int mult, rem, new;
#endif
    int cpu = smp_processor_id();
    unsigned long flags;

    spin_lock_irqsave(&pwrmgnt_clk_irqlock, flags);

#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
    if (pDdrSrCtrl) {
        // Communicate TP status to PHY MIPS
        // Here I don't check if Self-Refresh is enabled because when it is,
        // I want PHY MIPS to think the Host MIPS is always busy so it won't assert SR
        if (cpu == 0) {
            pDdrSrCtrl->tp0Busy = 1;
        } else {
            pDdrSrCtrl->tp1Busy = 1;
        }
    }
#endif


#if defined(CONFIG_BCM_KF_POWER_SAVE) && defined(CONFIG_BCM_HOSTMIPS_PWRSAVE)

#if defined(CONFIG_SMP)
    if (clock_divide_enabled && clock_divide_active0 && clock_divide_active1) {
#else
    if (clock_divide_enabled && clock_divide_active0) {
#endif
        // In newer MIPS core, there is no SYNC mode, simply use 1:1 async
        __BcmPwrMngtSetASCR(RATIO_ONE_ASYNC);
    }
#endif

#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS)
    if (cpu == 0) {
        // Check for TimerCnt2 rollover
        newTimerCnt0 = TIMER->TimerCnt2 & 0x3fffffff;
        if (newTimerCnt0 < prevTimerCnt0) {
           TimerAdjust0 += C0adder;
        }

        // fix the C0 counter because it slowed down while on wait state
        if (clock_divide_active0) {
           mult = newTimerCnt0/C0divider;
           rem  = newTimerCnt0%C0divider;
           new  = mult*C0multiplier + ((rem*C0ratio)>>10);
           write_c0_count(TimerAdjust0 + TimerC0Snapshot0 + new);
           clock_divide_active0 = 0;
        }
        prevTimerCnt0 = newTimerCnt0;
    }
#if defined(CONFIG_SMP)
    else {
        // Check for TimerCnt2 rollover
        newTimerCnt1 = TIMER->TimerCnt2 & 0x3fffffff;
        if (newTimerCnt1 < prevTimerCnt1) {
           TimerAdjust1 += C0adder;
        }

        // fix the C0 counter because it slowed down while on wait state
        if (clock_divide_active1) {
           mult = newTimerCnt1/C0divider;
           rem  = newTimerCnt1%C0divider;
           new  = mult*C0multiplier + ((rem*C0ratio)>>10);
           write_c0_count(TimerAdjust1 + TimerC0Snapshot1 + new);
           clock_divide_active1 = 0;
        }
        prevTimerCnt1 = newTimerCnt1;
    }
#endif
#else
    // On chips not requiring the PERIPH Timers workaround,
    // only need to clear the active flags, no need to adjust timers
    if (cpu == 0) {
       clock_divide_active0 = 0;
    }
#if defined(CONFIG_SMP)
    else {
       clock_divide_active1 = 0;
    }
#endif
#endif
    spin_unlock_irqrestore(&pwrmgnt_clk_irqlock, flags);
}


#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS)
// These numbers can be precomputed. The values are chosen such that the
// calculations will never overflow as long as the MIPS frequency never
// exceeds 850 MHz (hence mips_hpt_frequency must not exceed 425 MHz)
void BcmPwrMngtInitC0Speed (void)
{
    unsigned int mult, rem;
    if (mips_hpt_frequency > 425000000) {
       printk("\n\nWarning!!! CPU frequency exceeds limits to support" \
          " Clock Divider feature for Power Management\n");
    }
    C0divider = 50000000/128;
    C0multiplier = mips_hpt_frequency/128;
    C0ratio = ((mips_hpt_frequency/1000000)<<10)/50;
    mult = 0x40000000/C0divider;
    rem = 0x40000000%C0divider;
    // Value below may overflow from 32 bits but that's ok
    C0adder = mult*C0multiplier + ((rem*C0ratio)>>10);
    spin_lock_init(&pwrmgnt_clk_irqlock);
}
#endif //CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS

#endif //defined(CONFIG_BCM_KF_MIPS_BCM963XX)

