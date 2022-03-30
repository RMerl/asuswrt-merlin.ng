// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
 
*/
#include "pmc_drv.h"
#include "pmc_switch.h"
#include "asm/arch/BPCM.h"

int pmc_switch_enable_rgmii_zone_clk(int z1_clk_enable, int z2_clk_enable)
{
	int ret = 0;
#if defined(CONFIG_BCM63138) || defined(CONFIG_BCM63148) || defined(CONFIG_BCM4908) || defined(CONFIG_BCM63158)
#if defined(CONFIG_BCM63158)
	BPCM_GLOBAL_CNTL_1 global_cntl_1;
	BPCM_GLOBAL_CNTL_2 global_cntl_2;
#else
	BPCM_GLOBAL_CNTL global_cntl;
#endif

#if defined(CONFIG_BCM63148)
	BPCM_SGPHY_CNTL sgphy_cntl;

	sgphy_cntl.Reg32 = 0x33;
	ret = WriteBPCMRegister(PMB_ADDR_SWITCH,
				BPCMRegOffset(sgphy_cntl), sgphy_cntl.Reg32);
#endif

#if defined(CONFIG_BCM63158)
	ret = ReadBPCMRegister(PMB_ADDR_SWITCH,
			       BPCMRegOffset(global_control_1),
			       &global_cntl_1.Reg32);
	ret =
	    ReadBPCMRegister(PMB_ADDR_SWITCH, BPCMRegOffset(global_control_2),
			     &global_cntl_2.Reg32);

	global_cntl_1.Bits.z1_ck250_clk_en = 0;
	global_cntl_2.Bits.z2_ck250_clk_en = 0;
	ret = WriteBPCMRegister(PMB_ADDR_SWITCH,
				BPCMRegOffset(global_control_1),
				global_cntl_1.Reg32);
	ret =
	    WriteBPCMRegister(PMB_ADDR_SWITCH, BPCMRegOffset(global_control_2),
			      global_cntl_2.Reg32);

	ret = ReadBPCMRegister(PMB_ADDR_SWITCH,
			       BPCMRegOffset(global_control_1),
			       &global_cntl_1.Reg32);

	ret = ReadBPCMRegister(PMB_ADDR_SWITCH,
			       BPCMRegOffset(global_control_2),
			       &global_cntl_2.Reg32);
	if (z1_clk_enable)
		global_cntl_1.Bits.z1_ck250_clk_en = 1;
	if (z2_clk_enable)
		global_cntl_2.Bits.z2_ck250_clk_en = 1;
	ret = WriteBPCMRegister(PMB_ADDR_SWITCH,
				BPCMRegOffset(global_control_1),
				global_cntl_1.Reg32);
	ret =
	    WriteBPCMRegister(PMB_ADDR_SWITCH, BPCMRegOffset(global_control_2),
			      global_cntl_2.Reg32);

#else
	ret = ReadBPCMRegister(PMB_ADDR_SWITCH,
			       BPCMRegOffset(global_control),
			       &global_cntl.Reg32);

	global_cntl.Bits.z1_ck250_clk_en = 0;
	global_cntl.Bits.z2_ck250_clk_en = 0;
	ret = WriteBPCMRegister(PMB_ADDR_SWITCH,
				BPCMRegOffset(global_control),
				global_cntl.Reg32);

	ret = ReadBPCMRegister(PMB_ADDR_SWITCH,
			       BPCMRegOffset(global_control),
			       &global_cntl.Reg32);
	if (z1_clk_enable)
		global_cntl.Bits.z1_ck250_clk_en = 1;
	if (z2_clk_enable)
		global_cntl.Bits.z2_ck250_clk_en = 1;
	ret = WriteBPCMRegister(PMB_ADDR_SWITCH,
				BPCMRegOffset(global_control),
				global_cntl.Reg32);
#endif

	printf("%s: Rgmii Tx clock zone1 enable %d zone2 enable %d. \n",
	       __FUNCTION__, z1_clk_enable, z2_clk_enable);
#endif
	
	return ret;
}

int pmc_switch_power_up(void)
{
	return PowerOnDevice(PMB_ADDR_SWITCH);
}

int pmc_switch_power_down(void)
{
	return PowerOffDevice(PMB_ADDR_SWITCH, 0);
}

#if defined(CONFIG_BCM63158) && (CONFIG_BRCM_CHIP_REV==0x63158A0)
#define SWITCH_PLL_NORMAL   (0xC)	/* switch PLL is from 3GMHz, divider of 12 (0xD) will bring it to 250 MHz */
#define SWITCH_PLL_LOW_PWR  (0xF0)	/* switch PLL is from 3GMHz, divider of 240 (0xF0) will bring it to 12.5 MHz */
#elif defined(CONFIG_BCM63158)
/* From VLSI:
   PLL VCO frequency increased by 1.5x from A0 to B0;
   In normal mode we increased Switch frequency from A0 to B0 and that's why the value is less than 1.5x.
   Low power mode can stay at 1.5x which provides the same PLL output freq between A0 and B0.
*/
#define SWITCH_PLL_NORMAL   (0x11)
#define SWITCH_PLL_LOW_PWR  (0xFF)
#elif defined(CONFIG_BCM63178)
#define SWITCH_PLL_NORMAL   (0x06)
#define SWITCH_PLL_LOW_PWR  (0xFF)
#endif

#if defined(CONFIG_BCM63158) || defined (CONFIG_BCM63178) || defined (CONFIG_BCM6756)
void pmc_sysport_reset_system_port(int port)
{
	int status;
	uint32_t reg;
	(void)port;

	// for both BCM63158 and BCM63178, system port is tide to Zone 3 of the Switch BPCM
	status =
	    ReadBPCMRegister(PMB_ADDR_SWITCH, BPCMRegOffset(sr_control), &reg);
	if (status == kPMC_NO_ERROR) {
		// system port is on zone 3 of the switch BPCM
		reg |= 0x8;
		status =
		    WriteBPCMRegister(PMB_ADDR_SWITCH,
				      BPCMRegOffset(sr_control), reg);

		if (status == kPMC_NO_ERROR) {
			reg &= ~0x8;
			status =
			    WriteBPCMRegister(PMB_ADDR_SWITCH,
					      BPCMRegOffset(sr_control), reg);
		}
	}
}
#endif
