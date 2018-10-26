/*  *********************************************************************
    *
    <:copyright-BRCM:2017:proprietary:standard
    
       Copyright (c) 2017 Broadcom 
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
    ********************************************************************* */

#include "rom_main.h"

#if (INC_PMC_DRIVER==1)
#include "pmc_drv.h"
#include "clk_rst.h"
#include "BPCM.h"
#endif
#include "bcm_otp.h"

#if defined(_BCM963138_)
#define CFE_CLOCKS_PER_USEC 200
#elif defined(_BCM963148_)
#define CFE_CLOCKS_PER_USEC 750
#elif defined(_BCM94908_)
#define CFE_CLOCKS_PER_USEC (400+1400*fast_cpu_clock)
#elif defined(_BCM963158_)
#define CFE_CLOCKS_PER_USEC (400+1275*fast_cpu_clock)
#elif defined(_BCM96858_) || defined(_BCM96856_)
#define CFE_CLOCKS_PER_USEC cpu_clock
#elif defined(_BCM96846_)
#define CFE_CLOCKS_PER_USEC 1000
#else
// use maximum frequency value for clock ticks
#define CFE_CLOCKS_PER_USEC 1000
#endif

#if defined(_BCM94908_) || defined (_BCM963158_)
int fast_cpu_clock = 0;
#endif
#if defined(_BCM96858_) || defined(_BCM96846_) || defined(_BCM96856_)
int cpu_clock = 0;
extern int bcm_otp_get_cpu_clk(uint32* val);
#endif
#if defined(_BCM96848_)
extern uint32_t clks_per_usec;
extern uint32_t otp_get_max_clk_sel(void);
#endif

int pll_init(void)
{
#if defined(_BCM96838_) && (INC_PMC_DRIVER==1)
    unsigned int retval = 0;
    unsigned int ddr_freq_straps[] = {0, 333, 333, 400, 400, 533, 400, 533};
    unsigned int ddr_jedec_straps[] = {0, 3, 10, 21, 6, 8, 10, 13};
    unsigned int ddr_speed_straps[] = {0, 1, 2, 4, 2, 7, 2, 7}; // {0,333,333,400,400,533,400,533}
    unsigned int ddr_max_freq[] = {1000, 666, 533, 400, 333};
    unsigned int straps = ((*(volatile unsigned int*)(GPIO_BASE+GPIO_DATA_MID)) & (0x7 << (GPIO_STRAP_PIN_STRT-32))) >> (GPIO_STRAP_PIN_STRT-32) ;

    unsigned int ddr_freq = ddr_freq_straps[straps];
    unsigned int ddr_jedec = ddr_jedec_straps[straps];
    unsigned int ddr_speed = ddr_speed_straps[straps]; 

    volatile unsigned long otp_shadow_reg;
    unsigned long viper_freq;
    unsigned long rdp_freq;

    board_setleds(0x504c4c49); // PLLI

    otp_shadow_reg = *((volatile unsigned long*)(OTP_BASE+OTP_SHADOW_BRCM_BITS_0_31));

    // enforce max DDR frequency from OTP - if greater then max program minimum
    if(ddr_freq > ddr_max_freq[(otp_shadow_reg & OTP_BRCM_DDR_MAX_FREQ_MASK) >> OTP_BRCM_DDR_MAX_FREQ_SHIFT])
    {
        if((straps == 4) || (straps == 5))
            straps = 1; // DDR2
        else
            straps = 2; // DDR3

        ddr_freq = ddr_freq_straps[straps];
        ddr_jedec = ddr_jedec_straps[straps];
        ddr_speed = ddr_speed_straps[straps];
    }

    // for DDR3 1600MHz set phy_4x_mode
    if(straps == 3)
    {
        GPIO->memc_phy_control |= 1;
    }

    {
        volatile unsigned long *p = (unsigned long *)0xb4e00458;
        *p = 0x20000000;
    }
    
    WaitPmc(kPMCRunStateAVSCompleteWaitingForImage);
    board_setleds(0x504d4342); // PMCB

    switch( (otp_shadow_reg & OTP_BRCM_VIPER_FREQ_MASK) >> OTP_BRCM_VIPER_FREQ_SHIFT )
    {
        case 0:
            viper_freq = 600;
            break;

        case 1:
            viper_freq = 400;
            break;

        case 2:
            viper_freq = 240;
            break;

        default:
            viper_freq = 0;
            break;
    }
    if ( viper_freq )
        viper_freq_set(viper_freq);

#ifdef TEST_MODE
       rdp_freq = -1;
#else
    switch( (otp_shadow_reg & OTP_BRCM_RDP_FREQ_MASK) >> OTP_BRCM_RDP_FREQ_SHIFT )
    {
        case 0:
            rdp_freq = 800;
            break;

        case 1:
            rdp_freq = 400;
            break;

        default:
            rdp_freq = 0;
            break;
    }
#endif
    if ( rdp_freq )
        rdp_freq_set(rdp_freq);

    ddr_freq_set(ddr_freq);

    if(straps)
    {
        if(ddr_freq == 333)
            retval = 0x80000000;
        retval |= (ddr_jedec << PHY_CONTROL_REGS_STRAP_STATUS_JEDEC_TYPE_STRT);
        retval |= ((ddr_speed & 0x3) << PHY_CONTROL_REGS_STRAP_STATUS_SPEED_LO_STRT);
        retval |= (((ddr_speed >> 2) & 0x1) << PHY_CONTROL_REGS_STRAP_STATUS_SPEED_HI_STRT);
        retval |= PHY_CONTROL_REGS_STRAP_STATUS_STRAPS_VLD_MASK;
    }
    
    return retval;
#endif
    return 0;
}

int clk_init(void)
{
#if defined(_BCM96848_)
    unsigned int mips_otp = (otp_get_max_clk_sel() & MISC_STRAP_CLOCK_SEL_400) ? 400 : 250;
    unsigned int mips_index = (((MISC->miscStrapBus & MISC_STRAP_CLOCK_SEL_MASK) >> MISC_STRAP_CLOCK_SEL_SHIFT) & MISC_STRAP_CLOCK_SEL_400) ? 400 : 250;
    clks_per_usec = (mips_index <= mips_otp) ? mips_index : mips_otp;
#endif

#if defined(_BCM94908_) || defined(_BCM963158_)
    fast_cpu_clock = ((MISC->miscStrapBus)&MISC_STRAP_BUS_CPU_SLOW_FREQ) == 0 ? 1 : 0;
#elif (defined(_BCM96858_) || defined(_BCM96846_) || defined(_BCM96856_)) && !defined(CONFIG_BRCM_IKOS) && (BOOT_PRE_CFE==0)
#if defined(_BCM96846_)
    {
        unsigned int clk_index;
        bcm_otp_get_cpu_clk(&clk_index);
        switch (clk_index) {
        case 0:
        case 1:
            cpu_clock = 1000;
            break;
        case 2:
            cpu_clock = 750;
            break;
        default:
            cpu_clock = 0;
        }
    }
#else
    {
        unsigned int clk_index;
#if defined(_BCM96858_)
        unsigned int chipId = 0;
        bcm_otp_get_chipid(&chipId);

        if (chipId == 0x5 || chipId == 0x2)
        {
            cpu_clock = 1000;
            if (pll_ch_freq_set(PMB_ADDR_BIU_PLL, 0, 3))
                xprintf("Error: failed to set CPU clock\n");
        }
        else
#endif
        if ( !bcm_otp_get_cpu_clk(&clk_index) )
            cpu_clock = 500 + 500*(2-clk_index);
        else
            cpu_clock = 0;
    }
#endif
#if defined(_BCM96846_) || defined(_BCM96856_)
    //configure ubus clock
    UBUS4CLK->ClockCtrl = 0x04;
#endif

#if defined(_BCM96858_)
    /* configure AXI clock */
    if (pll_ch_freq_set(PMB_ADDR_BIU_PLL, 2, 4))
        xprintf("Error: failed to set AXI clock\n");
    
    /* Change cpu to fast clock */
    if ((MISC->miscStrapBus)&MISC_STRAP_BUS_CPU_SLOW_FREQ)
#endif
    {
        static int stat;
        PLL_CTRL_REG ctrl_reg;
#if defined(_BCM96858_)
        stat = ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
        ctrl_reg.Bits.byp_wait = 0;
        stat |= WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);
#elif defined(_BCM96846_) || defined(_BCM96856_)
        stat = ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLCLASSICBPCMRegOffset(resets), &ctrl_reg.Reg32);
        ctrl_reg.Bits.byp_wait = 0;
        stat |= WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLCLASSICBPCMRegOffset(resets), ctrl_reg.Reg32);
#endif
        if (stat)
            xprintf("Error: failed to set cpu fast mode\n");
    }
#endif

    return 0;
}

/*  *********************************************************************
    *  cfe_usleep(usec)
    *  
    *  Sleep for approximately the specified number of microseconds.
    *  
    *  Input parameters: 
    *      usec - number of microseconds to wait
    *      
    *  Return value:
    *      nothing
    ********************************************************************* */
// provide buffer zone such that we don't hang waiting for a corner case value such as max
#define BUFFER 8
void cfe_usleep(int usec)
{
    unsigned long newcount;
    unsigned long now;

    now = _getticks();
#if defined(_BCM96848_)
    newcount = now + ((unsigned long)usec) * ((unsigned long)clks_per_usec/2);
#else
    newcount = now + ((unsigned long)usec) * CFE_CLOCKS_PER_USEC;
#endif
    if (newcount > 0) // keep away from max
        newcount -= BUFFER;

    if (newcount < now)
        while (_getticks() > now)
            ;

    while (_getticks() < newcount)
            ;
}

#if defined(_BCM96858_) || defined(_BCM94908_) || defined(_BCM963158_) || \
    defined(_BCM96846_) || defined(_BCM96856_)
uint64_t cfe_get_utime(void)
{ 
    return (_getticks()/CFE_CLOCKS_PER_USEC);
}

uint32_t get_ms_timer_tick(void)
{
    return (uint32_t)(cfe_get_utime()/1000);
}

uint64_t get_us_timer_tick(void)
{
    return cfe_get_utime();
}

#endif

#if defined(_BCM963138_) || defined(_BCM963148_)

/* The 64 bit ARM Glob Timer is implemented here only for 63138 memc diagnotics code. The original gettick use 32 bit ARM performance 
monitor counter which runs at 200MHz initial CPU clock and it wraps around every 20s. The diagnotics code requires longer wait time 
period. Without introducing the interrupt, only way to get longer rollover timer is used to the 64 bit ARM timer */

/* ARM GTimer runs at CPU_CLK/2. At cfe rom, ARM runs 200MHz. Need to update this value if we
decide to speed up CPU to 1GHz in cfe rom */
#if defined(_BCM963138_)
#define GTIMER_CLOCK_MHZ          100

void enable_arm_gtimer(int enable)
{
    if( enable )
    {
        /* stop first */
        ARMGTIM->gtim_glob_ctrl &= ~ARM_GTIM_GLOB_CTRL_TIMER_EN;
        
        /* enable timer, no interrupt, no comparison, no prescale */
        ARMGTIM->gtim_glob_low = 0;
        ARMGTIM->gtim_glob_hi = 0;
        ARMGTIM->gtim_glob_ctrl = ARM_GTIM_GLOB_CTRL_TIMER_EN;
    }

    if( !enable )
        ARMGTIM->gtim_glob_ctrl &= ~ARM_GTIM_GLOB_CTRL_TIMER_EN;

    return;
}

uint64_t get_arm_gtimer_tick(void)
{
    uint32_t low, high;

    high = ARMGTIM->gtim_glob_hi;
    low = ARMGTIM->gtim_glob_low;

    /* read high again to ensure there is no rollover from low 32 bit */
    if( high != ARMGTIM->gtim_glob_hi ) 
    {
        //read again if high changed 
        high = ARMGTIM->gtim_glob_hi;
        low = ARMGTIM->gtim_glob_low;  
    }

    return ((uint64_t)low)|(((uint64_t)high)<<32);
}

#elif defined(_BCM963148_)  /* place holder for 63148. to be implemented. these function used by DDR library */
#define GTIMER_CLOCK_MHZ          50
void enable_arm_gtimer(int enable)
{
    uint32_t val0;
    if (enable) {
        val0 = GTIMER_CLOCK_MHZ * 1000000;
        asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (val0));
    } else {
        /* we will just let it run */
    }

    return;
}

uint64_t get_arm_gtimer_tick(void)
{
    uint32_t low, high;

    asm volatile("mrrc p15, 0, %0, %1, c14" : "=r"  (low), "=r" (high));
    return ((uint64_t)low) | (((uint64_t)high) << 32);
}
#endif

/* a rough function, return ticks in ms.It does not consider the case where ms tick exceeds the 32 bit range but
nobody really need 4x10^6 seconds */
uint32_t get_ms_timer_tick(void)
{
    return (uint32_t)(get_arm_gtimer_tick()/(GTIMER_CLOCK_MHZ*1000));
}

uint64_t get_us_timer_tick(void)
{
    return (get_arm_gtimer_tick()/(GTIMER_CLOCK_MHZ));
}
#endif

/* additonal DDR DIAG library required API */
#ifdef INC_DDR_DIAGS
#if defined(_BCM963158_)
uint32_t get_time_ms(void)
{
    return get_ms_timer_tick();
}

uint32_t get_time_ms_max_period_minus1(void)
{
    return (0xffffffff-1);
}

int delay_ms(uint32_t delay)
{
    int i;
    
    for( i = 0; i < delay; i++ )
        cfe_usleep(1000);

    return 0;
}
#endif
#endif
