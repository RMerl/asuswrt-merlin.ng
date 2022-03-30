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
    swrw(0,3,0x5171);
    swrw(0,6,0xb000);
    swrw(0,7,0x0029);
    /* 1.8 SWREG  set bit reg3[8] & reg3[4]  pll_en and pll_phase_en */
    swrw(1,3,0x5170);
    swrw(1,7,0x0029);
    /* 1.5 SWREG  set bit reg3[8] & reg3[4]  pll_en and pll_phase_en */
    swrw(2,3,0x5170);
    swrw(2,7,0x0029);
    /* 1.0 Analog SWREG  set bit reg3[8] & reg3[4]  pll_en and pll_phase_en */
    swrw(3,3,0x5170);
    swrw(3,7,0x0029);

    if (!(MISC->miscStrapBus & MISC_STRAP_ENABLE_INT_1p8V))
        swrw(1,0,0xc691);
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
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
    u32 frq = COUNTER_FREQUENCY;

    spl_ddrinit_prepare();

    // set arch timer frequency
    asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (frq));

    // enable spi slave
    *((volatile unsigned int*)CONFIG_BROM_REG_ADDR) |= 0x4;

    // enable system timer
    BIUCFG->TSO_CNTCR |= 1;

    bcm_setsw();
    
    /* enable unalgined access */    
    set_cr(get_cr() & ~CR_A);
#endif

#if defined(CONFIG_TPL_BUILD)
    enable_ns_access();
    cci400_enable();
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
	unsigned int chipId = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
	unsigned int revId = PERF->RevID & REV_ID_MASK;

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
