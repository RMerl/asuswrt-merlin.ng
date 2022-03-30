/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom Ltd.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ubus4.h>
#include "bcm_otp.h"
#include <spl.h>
#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_drv.h"
#include "asm/arch/BPCM.h"
#include "clk_rst.h"
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
    swrw(0,3,0x5372);
    swrw(0,6,0xb000);
    swrw(0,7,0x0029);
    swrw(1,3,0x5370);
    swrw(1,7,0x0029);
    swrw(2,3,0x5370);
    swrw(2,7,0x0029);
    swrw(3,3,0x5370);
    swrw(3,7,0x0029);
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
    int stat;
    PLL_CTRL_REG ctrl_reg;

    //configure ubus clock
    UBUS4CLK->ClockCtrl = 0x04;

    if ( !bcm_otp_get_cpu_clk(&clk_index) )
        cpu_clock = 500 + 500*(2-clk_index);
        else
        cpu_clock = 0;

    stat = ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLCLASSICBPCMRegOffset(resets), &ctrl_reg.Reg32);
        ctrl_reg.Bits.byp_wait = 0;
        stat |= WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLCLASSICBPCMRegOffset(resets), ctrl_reg.Reg32);

    if (stat)
        printf("Error: failed to set cpu fast mode\n");

    printf("CPU Clock: %dMHz\n", cpu_clock);

    disable_xtal_clk();
}

int arch_cpu_init(void)
{
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	spl_ddrinit_prepare();
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
	unsigned int cpu_speed, rdp_speed, otp_cores;
	unsigned int chipId = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
	unsigned int revId = PERF->RevID & REV_ID_MASK;
	unsigned int chipvar;

	if ( !bcm_otp_get_chipid(&chipvar) )
	{
		if ((chipId==0x68560) && (chipvar==3))
			chipId=0x68560B;
	}
	printf("Chip ID: BCM%X_%X\n",chipId,revId);

	if ( !bcm_otp_get_nr_cpus(&otp_cores) )
	{
		if (otp_cores == 0)
			nr_cores = "Dual";
		else if (otp_cores == 1)
			nr_cores = "Single";
	}

	pll_ch_freq_get(PMB_ADDR_BIU_PLL, 0, &cpu_speed);
	get_rdp_freq(&rdp_speed);

	printf("Broadcom B53 %s Core: %dMHz\n", nr_cores, cpu_speed);
	printf("RDP: %dMHz\n",rdp_speed);
}
#endif

int bcmbca_get_boot_device(void)
{
	if ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_NAND_MASK) == MISC_STRAP_BUS_BOOT_NAND)
		return BOOT_DEVICE_NAND;

	if ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_MASK) == MISC_STRAP_BUS_BOOT_SPI_NAND)
		return BOOT_DEVICE_SPI;

	if ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_MASK) == MISC_STRAP_BUS_BOOT_EMMC)
		return BOOT_DEVICE_MMC1;

	printf("Error: boot_sel straps are not set correctly\n");

	return BOOT_DEVICE_NONE;
}

#if !defined(CONFIG_TPL_ATF)
void boot_secondary_cpu(unsigned long vector)
{
	uint32_t cpu = 1; 
	uint32_t nr_cpus = 2;
	ARM_CONTROL_REG ctrl_reg;

	printf("boot secondary cpu from 0x%lx\n", vector);

	while (cpu < nr_cpus) {
		int stat;

		BIUCFG->cluster[0].rvbar_addr[cpu] = vector;
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
