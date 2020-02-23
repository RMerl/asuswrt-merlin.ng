/* 
    Copyright 2000-2019 Broadcom Corporation

    <:label-BRCM:2019:DUAL/GPL:standard
    
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
#include "lib_types.h"
#include "cfe_timer.h"
#include "cfe.h"
#include "bcm_map.h"
#include "bcm_hwdefs.h"
#include "bcm63xx_ipc.h"
#include "bcm63xx_cmn_util.h"
#include "bcm63xx_common.h"



#if defined (CFG_RAMAPP) || defined (CONFIG_CFE_FAILSAFE_BOOT)
#if (INC_PMC_DRIVER==1)
#include "pmc_drv.h"
#include "BPCM.h"
#endif
static void _bcm63xx_fix_pll(void)
{
#if defined(_BCM94908_)
    PLL_CTRL_REG ctrl_reg;
    /* reset the pll manually to bypass mode if strap for slow clock */
    if (MISC->miscStrapBus&MISC_STRAP_BUS_CPU_SLOW_FREQ) {
        ReadBPCMRegister(PMB_ADDR_B53PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
        ctrl_reg.Bits.byp_wait = 1;
        WriteBPCMRegister(PMB_ADDR_B53PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);
    }
#endif
}
void bcm_cmn_wd_reset(void)
{
        _bcm63xx_fix_pll();
#if defined(_BCM963268_)
        PERF->pll_control |= SOFT_RESET;    // soft reset mips
#elif defined(_BCM96838_)
        PERF->TimerControl |= SOFT_RESET_0;
#elif defined(_BCM960333_)
        /*
         * After a soft-reset, one of the reserved bits of TIMER->SoftRst remains
         * enabled and the next soft-reset won't work unless TIMER->SoftRst is
         * set to 0.
         */
        TIMER->SoftRst = 0;
        TIMER->SoftRst |= SOFT_RESET;
#elif defined(_BCM947189_)
        /*
         * In theory, since 47189 has PMU enabled (MISC_1->capabilities &
         * CC_CAP_PMU), reset should be done through the PMU watchdog
         * (PMU->pmuwatchdog). But this hangs the BCM947189ACNRM_2 P235
         * evaluation board.
         * However, using the ChipCommon core watchdog works, so we're going
         * with that for now.
         */
        GPIO_WATCHDOG->watchdog = 1;
#elif defined (_BCM96858_) || defined (_BCM963158_) || defined (_BCM96846_) || defined (_BCM96856_)
        WDTIMER0->SoftRst |= SOFT_RESET;
#elif defined (_BCM963178_)  || defined(_BCM947622_) || defined(_BCM96878_)
        WDTIMER0->WDTimerCtl |= SOFT_RESET;
#else
        TIMER->SoftRst |= SOFT_RESET;
#endif
}

void bcm_cmn_wd_set(unsigned int delay, unsigned int en)
{
    if (en) {
        _bcm63xx_fix_pll();
        en = 0xFF0000FF; 
    } else {
        en = 0xEE0000EE;
    } 
#if defined (_BCM96838_)
        WDTIMER->WD0DefCount = delay*FPERIPH_WD;
        WDTIMER->WD0Ctl = (en>>16);
        WDTIMER->WD0Ctl = (en&0xffff);
#elif defined(_BCM947189_)
        /*
         * Watchdog reset - TODO: Get PLL freq to calculate the watchdog counter
         * value.
         */
#elif defined (_BCM96858_) || defined(_BCM963158_) || defined (_BCM96846_) || defined (_BCM96856_) || defined (_BCM963178_)  || defined(_BCM947622_) || defined(_BCM96878_)
        WDTIMER0->WatchDogDefCount = delay*FPERIPH_WD;
        WDTIMER0->WatchDogCtl = (en>>16);
        WDTIMER0->WatchDogCtl = (en&0xffff);
#else
        TIMER->WatchDogDefCount = delay*FPERIPH_WD;
        TIMER->WatchDogCtl = (en>>16);
        TIMER->WatchDogCtl = (en&0xffff);
#endif
}
#endif
