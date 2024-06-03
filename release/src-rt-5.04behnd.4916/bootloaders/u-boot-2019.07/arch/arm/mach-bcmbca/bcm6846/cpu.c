/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr.h>
#include <asm/arch/ubus4.h>
#include <spl.h>
#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_drv.h"
#include "asm/arch/BPCM.h"
#include "clk_rst.h"
#endif
#include "spl_ddrinit.h"
#include "bcm_otp.h"
#include "bca_common.h"
#if defined(CONFIG_BCMBCA_UBUS4_DCM)
#include "bcm_ubus4.h"
#endif
#include "bcm_strap_drv.h"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_TPL_BUILD)
static void cci400_enable(void)
{
    CCI400->secr_acc |= SECURE_ACCESS_UNSECURE_ENABLE;
}

static void enable_ns_access(void)
{
    BIUCFG->aux.permission |= 0xff;
}
#endif

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
void bcm_setsw(void)
{
    swr_write(0,3,0x5171);
    swr_write(0,6,0xb000);
    swr_write(0,7,0x0029);
    /* 1.8 SWREG  set bit reg3[8] & reg3[4]  pll_en and pll_phase_en */
    swr_write(1,3,0x5170);
    swr_write(1,7,0x0029);
    /* 1.5 SWREG  set bit reg3[8] & reg3[4]  pll_en and pll_phase_en */
    swr_write(2,3,0x5170);
    swr_write(2,7,0x0029);
    /* 1.0 Analog SWREG  set bit reg3[8] & reg3[4]  pll_en and pll_phase_en */
    swr_write(3,3,0x5170);
    swr_write(3,7,0x0029);

    if (!(bcm_strap_parse_and_test(ofnode_null(), "enable_int_1p8v")))
        swr_write(1,0,0xc691);
}
#endif

#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_TPL_BUILD)
#define CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV (17)
static void disable_xtal_clk(void)
{
    uint32_t data;
    int ret;

    ret = ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST,
                   CLKRSTBPCMRegOffset(xtal_control), &data);

    data |= (0x1 << CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV);

    ret |= WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST,
                CLKRSTBPCMRegOffset(xtal_control), data);

    if (ret)
        printf("Failed to disable xtal clk\n");
}
#else
void disable_xtal_clk(void);
#endif

void boost_cpu_clock(void)
{
    unsigned int clk_index, cpu_clock=-1;
    int stat;
    PLL_CTRL_REG ctrl_reg;

#if defined(CONFIG_BCMBCA_UBUS4_DCM)
    //configure ubus clock
    bcm_ubus4_dcm_clk_bypass(1);
#endif

    if (!bcm_otp_get_cpu_clk(&clk_index))
    {
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
    else
        printf("Error: failed to read cpu clock\n");
   
    stat = ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLCLASSICBPCMRegOffset(resets), &ctrl_reg.Reg32);
    ctrl_reg.Bits.byp_wait = 0;
    stat |= WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLCLASSICBPCMRegOffset(resets), ctrl_reg.Reg32);

    if (stat)
        printf("Error: failed to set cpu fast mode\n");
    else
        printf("CPU Clock: %dMHz\n", cpu_clock);

    disable_xtal_clk();
}

int arch_cpu_init(void)
{
#if defined(CONFIG_BCMBCA_IKOS)
	icache_enable();
#endif  
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
    u32 frq = COUNTER_FREQUENCY;

    spl_ddrinit_prepare();

    // set arch timer frequency
    asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (frq));

    // enable spi slave
    *((volatile unsigned int*)CONFIG_BROM_REG_ADDR) |= 0x4;

    // enable system timer
    BIUCFG->TSO_CNTCR |= 1;

    /* enable unalgined access */    
    set_cr(get_cr() & ~CR_A);
#endif

#if defined(CONFIG_TPL_BUILD)
    enable_ns_access();
    cci400_enable();
#endif

#ifdef CONFIG_DISABLE_CONSOLE
        gd->flags |= GD_FLG_DISABLE_CONSOLE;
#endif
#ifdef CONFIG_SILENT_CONSOLE
	gd->flags |= GD_FLG_SILENT;
#endif

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)
const uint32_t cpu_speed_table[4] = {
	1000, 1000, 750, 0
};

void print_chipinfo(void)
{
	char *mktname = NULL;
	unsigned int cpu_speed, rdp_speed, clk_index;
	unsigned int chipId = bcmbca_get_chipid();
	unsigned int revId = bcmbca_get_chiprev();

	switch (chipId) {
	case(0x68463):
		mktname = "68460U";
		break;
	case(0x68464):
		mktname = "68461S";
	default:
		mktname = NULL;
	}

	if (mktname == NULL)
		printf("Chip ID: BCM%X_%X\n",chipId,revId);
	else
		printf("Chip ID: BCM%s_%X\n",mktname,revId);

	get_rdp_freq(&rdp_speed);
	if ( !bcm_otp_get_cpu_clk(&clk_index) )
		cpu_speed = cpu_speed_table[clk_index];
	else
		cpu_speed = 0;

	printf("ARM Cortex A7 Dual Core: %dMHz\n",cpu_speed);
	printf("RDP: %dMHz\n",rdp_speed);
}
#endif

#if !defined(CONFIG_TPL_ATF)
void boot_secondary_cpu(unsigned long vector)
{
	uint32_t cpu = 1; 
	uint32_t nr_cpus = 2;
	ARM_CONTROL_REG ctrl_reg;

	printf("boot secondary cpu from 0x%lx\n", vector);

	*(volatile uint32_t*)(BOOTLUT_BASE+0x20) = vector;
	
	while (cpu < nr_cpus) {
		int stat;

		stat = ReadBPCMRegister(PMB_ADDR_BIU_BPCM, ARMBPCMRegOffset(arm_control), &ctrl_reg.Reg32);
		ctrl_reg.Bits.cpu_reset_n &= ~(0x1 << cpu);
		stat |= WriteBPCMRegister(PMB_ADDR_BIU_BPCM, ARMBPCMRegOffset(arm_control), ctrl_reg.Reg32);
		if (stat != kPMC_NO_ERROR)
			printf("failed to boot secondary cpu %d - sts %d\n", cpu, stat);
		cpu++;
	}

	return;
}
#endif
