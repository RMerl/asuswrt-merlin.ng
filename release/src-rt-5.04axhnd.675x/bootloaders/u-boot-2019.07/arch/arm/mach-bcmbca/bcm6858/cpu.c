/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <spl_ddrinit.h>
#include <asm/arch/cpu.h>
#include <asm/arch/misc.h>
#include "bcm_otp.h"
#include <spl.h>
#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_drv.h"
#include "clk_rst.h"
#include "asm/arch/BPCM.h"
#endif

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
static void swrw(unsigned int ps, unsigned int reg, unsigned int val)
{
    unsigned int cmd = 0;
    unsigned int cmd1 = 0;
    unsigned int reg0 = 0;

    PROCMON->SSBMaster.control = SWR_EN;

    if (reg == 0) {
        /* no need read reg0 in case that we write to it , we know wal :) */
        reg0 = val;
    } else {
        /* read reg0 */
        cmd1 = SWR_READ_CMD_P | SET_ADDR(ps, 0);
        PROCMON->SSBMaster.control = cmd1;
        SR_TEST(1)
            reg0 = PROCMON->SSBMaster.rd_data;
    }
    /* write reg */
    PROCMON->SSBMaster.wr_data = val;
    cmd = SWR_WR_CMD_P | SET_ADDR(ps, reg);
    PROCMON->SSBMaster.control = cmd;
    SR_TEST(2);
    /*toggele bit 1 reg0 this load the new regs value */
    cmd1 = SWR_WR_CMD_P | SET_ADDR(ps, 0);
    PROCMON->SSBMaster.wr_data = reg0 & ~0x2;
    PROCMON->SSBMaster.control = cmd1;
    SR_TEST(3);
    PROCMON->SSBMaster.wr_data = reg0 | 0x2;
    PROCMON->SSBMaster.control = cmd1;
    SR_TEST(4);
    PROCMON->SSBMaster.wr_data = reg0 & ~0x2;
    PROCMON->SSBMaster.control = cmd1;
    SR_TEST(5);
}

static void bcm_setsw(void)
{
    swrw(1,3,0x5170);
    swrw(1,7,0x4829);
    swrw(2,3,0x5172);
    swrw(2,7,0x4829);
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
#endif

void boost_cpu_clock(void)
{
    unsigned int clk_index, cpu_clock;
    unsigned int chipId = 0;
    int stat;
    bcm_otp_get_chipid(&chipId);

    if (chipId == 0x5 || chipId == 0x2)
    {
        cpu_clock = 1000;
        if (pll_ch_freq_set(PMB_ADDR_BIU_PLL, 0, 3))
            printf("Error: failed to set CPU clock\n");
    }
    else if ( !bcm_otp_get_cpu_clk(&clk_index) )
            cpu_clock = 500 + 500*(2-clk_index);
    else
        cpu_clock = 0;
    
    /* configure AXI clock */
    if (pll_ch_freq_set(PMB_ADDR_BIU_PLL, 2, 4))
        printf("Error: failed to set AXI clock\n");

    /* Change cpu to fast clock */
    if ((MISC->miscStrapBus)&MISC_STRAP_BUS_CPU_SLOW_FREQ)
    {
        int stat;
        PLL_CTRL_REG ctrl_reg;
        stat = ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
        ctrl_reg.Bits.byp_wait = 0;
        stat |= WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);
        if (stat)
            printf("Error: failed to set cpu fast mode\n");
    }

    printf("CPU Clock: %dMHz\n", cpu_clock);

    stat = PowerOnDevice(PMB_ADDR_RDPPLL);

    stat = pll_ch_freq_set(PMB_ADDR_RDPPLL, 0, 2);
    stat |= pll_ch_freq_set(PMB_ADDR_RDPPLL, 1, 1);

    if (stat)
        printf("Error: Failed to set RDPPLL\n");

    disable_xtal_clk();
}

int arch_cpu_init(void)
{
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	spl_ddrinit_prepare();
	// enable system timer
	BIUCFG->TSO_CNTCR |= 1;

	bcm_setsw();
	/* enable unalgined access */	
	set_sctlr(get_sctlr() & ~CR_A);	
#endif

#if defined(CONFIG_TPL_BUILD)
    enable_ns_access();
    cci400_enable();
#endif

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)
void print_chipinfo(void)
{
	char *mktname = NULL;
	char *nr_cores = NULL;
	unsigned int cpu_speed, rdp_speed, chipId;
	unsigned int revId = PERF->RevID & REV_ID_MASK;
	
	if (bcm_otp_get_chipid(&chipId)) {
		chipId = 0;
	}

 
	switch (chipId) {
	case(0x0):
		nr_cores = "Quad";
	case(0x1):
		mktname = "68580X";
		break;
	case(0x2):
		nr_cores = "Dual";
		mktname = "55040";
		break;
	case(0x3):
		mktname = "68580H";
		break;
	case(0x4):
		mktname = "55040P";
		break;
	case(0x5):
		nr_cores = "Dual";
		mktname = "55045";
		break;
	case(0x6):
		mktname = "68580XV";
		break;
	case(0x7):
		mktname = "49508";
		break;
	case(0x8):
		mktname = "62119";
		break;
	case(0x9):
		mktname = "68580XP";
		break;
	case(0xA):
		mktname = "62119P";
		break;
	case(0xB):
		nr_cores = "Dual";
		mktname = "55040B";
		break;
	case(0xC):
		mktname = "55040M";
		break;
	case(0xD):
		mktname = "68580XF";
		break;
	default:
		mktname = NULL;
	}

	printf("Chip ID: BCM%s_%X\n",mktname,revId);

	pll_ch_freq_get(PMB_ADDR_BIU_PLL, 0, &cpu_speed);
	printf("Broadcom B53 %s Core: %dMHz\n", nr_cores, cpu_speed);

	get_rdp_freq(&rdp_speed);
	printf("RDP: %dMHz\n",rdp_speed);
}
#endif

int bcmbca_get_boot_device(void)
{
	unsigned int bootsel = ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL0_4_MASK) >> MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT) |
							((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL5_MASK) >> BOOT_SEL5_STRAP_ADJ_SHIFT);

	if ((bootsel & BOOT_SEL_STRAP_BOOT_SEL_NAND_MASK) == BOOT_SEL_STRAP_NAND)
		return BOOT_DEVICE_NAND;

	if ((bootsel & BOOT_SEL_STRAP_BOOT_SEL_MASK) == BOOT_SEL_STRAP_SPI_NAND)
		return BOOT_DEVICE_SPI;

	if ((bootsel & BOOT_SEL_STRAP_BOOT_SEL_MASK) == BOOT_SEL_STRAP_EMMC)
		return BOOT_DEVICE_MMC1;

	printf("Error: boot_sel straps are not set correctly\n");

	return BOOT_DEVICE_NONE;
}

#if !defined(CONFIG_TPL_ATF)
void boot_secondary_cpu(unsigned long vector)
{
	uint32_t cpu = 1; 
	uint32_t nr_cpus = 4;
	ARM_CONTROL_REG ctrl_reg;

	printf("boot secondary cpu from 0x%lx\n", vector);

	while (cpu < nr_cpus) {
		int stat;

		BIUCFG->cluster[0].rvbar_addr[cpu] = vector >> 8;
		stat = PowerOnDevice(PMB_ADDR_ORION_CPU0 + cpu);
		if (stat != kPMC_NO_ERROR)
			printf("failed to power on secondary cpu %d - sts %d\n", cpu, stat);

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
