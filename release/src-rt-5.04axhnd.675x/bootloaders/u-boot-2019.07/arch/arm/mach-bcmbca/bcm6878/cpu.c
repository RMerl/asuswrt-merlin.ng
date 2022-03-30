/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr.h>
#include <spl.h>
#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_drv.h"
#include "asm/arch/BPCM.h"
#include "clk_rst.h"
#endif

#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_TPL_BUILD)
#define CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV (2)
#define CLOCK_RESET_XTAL_CONTROL_BIT_PWRON  (27)
#define CLOCK_RESET_XTAL_CONTROL2_BIT_PD    (0)

static void disable_xtal_clk(void)
{
    uint32_t data;
    int ret;

    ret = ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST,
                   CLKRSTBPCMRegOffset(pmd_xtal_cntl), &data);

    data |= (0x1 << CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV);
    data &= ~(0x1 << CLOCK_RESET_XTAL_CONTROL_BIT_PWRON);

    ret |= WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST,
                CLKRSTBPCMRegOffset(pmd_xtal_cntl), data);

    ret |= ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST,
                 CLKRSTBPCMRegOffset(pmd_xtal_cntl2), &data);

    data |= (0x1 << CLOCK_RESET_XTAL_CONTROL2_BIT_PD);

    ret |= WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST,
                CLKRSTBPCMRegOffset(pmd_xtal_cntl2), data);

    if (ret)
        printf("Failed to disable xtal clk\n");
}
#endif

int arch_cpu_init(void)
{
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
    u32 frq = COUNTER_FREQUENCY;

    // set arch timer frequency
    asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (frq));

    // enable system timer
    BIUCFG->TSO_CNTCR |= 1;

    /* enable unalgined access */    
    set_cr(get_cr() & ~CR_A);
#endif

    return 0;
}

void boost_cpu_clock(void)
{
    unsigned int clk_index, cpu_clock;
    int stat;
    PLL_CTRL_REG ctrl_reg;

    if ( !bcm_otp_get_cpu_clk(&clk_index) )
    {
        switch (clk_index) {
            case 0:
            case 1:
                cpu_clock = 1200;
                break;
            case 2:
                cpu_clock = 800;
                break;
            default:
                cpu_clock = 0;
            }
    }
    else
        printf("ERROR: failed to read cpu clock\n");

    stat = ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
    ctrl_reg.Bits.byp_wait = 0;
    stat |= WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);

    if (stat)
        printf("ERROR: failed to set cpu fast clock\n");
    else
        printf("CPU Clock: %dMHz\n", cpu_clock);

    disable_xtal_clk();
}

#if !defined(CONFIG_SPL_BUILD)
void print_chipinfo(void)
{
	char *mktname = NULL;
	unsigned int cpu_speed, rdp_speed;
	unsigned int chipId = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
	unsigned int revId = PERF->RevID & REV_ID_MASK;

	switch (chipId) {
	case(0x6878A):
		mktname = "68782G";
		break;
	case(0x68789):
		mktname = "68781H";
		break;
	case(0x6878C):
		mktname = "68789";
		break;
	case(0x6878E):
		mktname = "68782N";
		break;
	case(0x6878D):
		mktname = "68781G";
		break;
	default:
		mktname = NULL;
	}

	if (mktname == NULL)
		printf("Chip ID: BCM%X_%X\n",chipId,revId);
	else
		printf("Chip ID: BCM%s_%X\n",mktname,revId);

	pll_ch_freq_get(PMB_ADDR_BIU_PLL, 0, &cpu_speed);
	get_rdp_freq(&rdp_speed);

	printf("ARM Cortex A7 Dual Core: %dMHz\n",cpu_speed);
	printf("RDP: %dMHz\n",rdp_speed);
}
#endif

int bcmbca_get_boot_device(void)
{
	if ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_NAND_MASK) == MISC_STRAP_BUS_BOOT_NAND)
		return BOOT_DEVICE_NAND;

	if ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_MASK) == MISC_STRAP_BUS_BOOT_SPI_NAND)
		return BOOT_DEVICE_SPI;

	printf("Error: boot_sel straps are not set correctly\n");

	return BOOT_DEVICE_NONE;
}

#if !defined(CONFIG_TPL_ATF)
void boot_secondary_cpu(unsigned long vector)
{
    uint32_t cpu = 1;
    uint32_t nr_cpus = 2;
    ARM_CONTROL_REG ctrl_reg;
    uint64_t rvbar = vector;
    int stat;

    printf("boot secondary cpu from 0x%lx\n", vector);

    *(volatile uint32_t*)(BOOTLUT_BASE+0x20) = vector;

    BIUCFG->cluster[0].rvbar_addr[cpu] = rvbar;

    stat = PowerOnDevice(PMB_ADDR_ORION_CPU0 + cpu);
        if (stat != kPMC_NO_ERROR)
            printf("failed to power on secondary cpu %d - sts %d\n", cpu, stat);

    while (cpu < nr_cpus) 
    {
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

