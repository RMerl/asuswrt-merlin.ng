// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Broadcom
 */
/*

*/

#include "pmc_drv.h"
#include "pmc_rdp.h"
#include "asm/arch/BPCM.h"

#define PRINTK	printf

static int pmc_rdp_start_pll_with_clk(uint32_t clk)
{
	uint32_t bpcmResReg;
	uint32_t error;

	if (WriteBPCMRegister(PMB_ADDR_RDPPLL, PLLBPCMRegOffset(resets), 0))
		PRINTK
		    ("failed to configure PMB RDPPLL at word offset of 0x%02x\n",
		     (unsigned int)PLLBPCMRegOffset(resets));

#if 0				// FIXME! after we know how to pass the RDP clk info, also clean the hardcode value?
	if (clk == 550 MHz) {
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0x7, 0x00000084))
			PRINTK("failed to configure PMB RDPPLL 0x7\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0x8, 0x80000002))
			PRINTK("failed to configure PMB RDPPLL 0x8\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0xb, 0x800c8006))
			PRINTK("failed to configure PMB RDPPLL 0xb\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0xc, 0x81008021))
			PRINTK("failed to configure PMB RDPPLL 0xc\n");
		PRINTK
		    ("%s:setting up RDP PLL to run at reduced speed of 550/275 MHz\n",
		     __func__);
	} else if (clk == 400 MHz) {
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0x7, 0x00000080))
			PRINTK("failed to configure PMB RDPPLL 0x7\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0x8, 0x80000002))
			PRINTK("failed to configure PMB RDPPLL 0x8\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0xb, 0x80108008))
			PRINTK("failed to configure PMB RDPPLL 0xb\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0xc, 0x81008020))
			PRINTK("failed to configure PMB RDPPLL 0xc\n");
		PRINTK
		    ("%s:setting up RDP PLL to run at reduced speed of 400/200 MHz\n",
		     __func__);
	} else if (clk == 200 MHz) {
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0x7, 0x00000080))
			PRINTK("failed to configure PMB RDPPLL 0x7\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0x8, 0x80000002))
			PRINTK("failed to configure PMB RDPPLL 0x8\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0xb, 0x80208010))
			PRINTK("failed to configure PMB RDPPLL 0xb\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0xc, 0x81008020))
			PRINTK("failed to configure PMB RDPPLL 0xc\n");
		PRINTK
		    ("%s:setting up RDP PLL to run at reduced speed of 200/100 MHz\n",
		     __func__);
	}
#endif

	error = ReadBPCMRegister(PMB_ADDR_RDPPLL, PLLBPCMRegOffset(resets),
				 (uint32_t *) & bpcmResReg);
	if (error)
		PRINTK("Failed to ReadBPCMRegister RDPPLL block at word offset "
		       "of 0x%02x\n", (unsigned int)PLLBPCMRegOffset(resets));

	if (WriteBPCMRegister(PMB_ADDR_RDPPLL, PLLBPCMRegOffset(resets),
			      (bpcmResReg | (1 << 0))))
		PRINTK("failed to configure PMB RDPPLL of 0x%02x\n",
		       (unsigned int)PLLBPCMRegOffset(resets));

	do {
		error = ReadBPCMRegister(PMB_ADDR_RDPPLL,
					 PLLBPCMRegOffset(stat),
					 (uint32_t *) & bpcmResReg);
		if (error)
			PRINTK("Failed to ReadBPCMRegister RDPPLL block "
			       "0x%02x\n",
			       (unsigned int)PLLBPCMRegOffset(stat));
	} while (!(bpcmResReg & 0x80000000));

	error = ReadBPCMRegister(PMB_ADDR_RDPPLL, PLLBPCMRegOffset(resets),
				 (uint32_t *) & bpcmResReg);
	if (error)
		PRINTK("Failed to ReadBPCMRegister RDPPLL block at word offset "
		       "of 0x%02x\n", (unsigned int)PLLBPCMRegOffset(resets));

	if (WriteBPCMRegister(PMB_ADDR_RDPPLL, PLLBPCMRegOffset(resets),
			      (bpcmResReg | (1 << 1))))
		PRINTK("failed to configure PMB RDPPLL of 0x%02x\n",
		       (unsigned int)PLLBPCMRegOffset(resets));
	return error;
}

int pmc_rdp_power_up(void)
{
#if defined(PMB_ADDR_RDP)
	return PowerOnDevice(PMB_ADDR_RDP);
#endif
#if 0
	int i, ret;
	BPCM_PWR_ZONE_N_CONTROL pwr_zone_ctrl;

	for (i = 0; i < PMB_ZONES_RDP; i++) {
		ret = ReadZoneRegister(PMB_ADDR_RDP, i, 0,
				       &pwr_zone_ctrl.Reg32);
		if (ret)
			return;
		pwr_zone_ctrl.Bits.pwr_dn_req = 0;
		pwr_zone_ctrl.Bits.dpg_ctl_en = 1;
		pwr_zone_ctrl.Bits.pwr_up_req = 1;
		pwr_zone_ctrl.Bits.mem_pwr_ctl_en = 1;
		pwr_zone_ctrl.Bits.blk_reset_assert = 1;

		ret = WriteZoneRegister(PMB_ADDR_RDP, i, 0,
					pwr_zone_ctrl.Reg32);
		if (ret)
			return;
	}
#endif
}

int pmc_rdp_power_down(void)
{
#if defined(PMB_ADDR_RDP)
	return PowerOffDevice(PMB_ADDR_RDP, 0);
#endif
#if 0
	int i, ret;
	BPCM_PWR_ZONE_N_CONTROL pwr_zone_ctrl;

	for (i = 0; i < PMB_ZONES_RDP; i++) {
		ret = ReadZoneRegister(PMB_ADDR_RDP, i, 0,
				       &pwr_zone_ctrl.Reg32);
		if (ret)
			return;
		pwr_zone_ctrl.Bits.pwr_dn_req = 1;
		pwr_zone_ctrl.Bits.dpg_ctl_en = 0;
		pwr_zone_ctrl.Bits.pwr_up_req = 0;
		pwr_zone_ctrl.Bits.mem_pwr_ctl_en = 0;
		pwr_zone_ctrl.Bits.blk_reset_assert = 0;

		ret = WriteZoneRegister(PMB_ADDR_RDP, i, 0,
					pwr_zone_ctrl.Reg32);
		if (ret)
			return;
	}
#endif
}

int pmc_rdp_init(void)
{
	int ret;

	pmc_rdp_start_pll_with_clk(0);

#if defined(PMB_ADDR_RDP)
	/* put all RDP modules in reset state */
	ret = WriteBPCMRegister(PMB_ADDR_RDP, BPCMRegOffset(sr_control), 0);
	if (ret)
		PRINTK("%s:%d:failed to configure PMB RDP at word offset of "
		       "0x%02x\n", __func__, __LINE__,
		       (unsigned int)BPCMRegOffset(sr_control));
#endif

	ret = pmc_rdp_power_down();
	if (ret)
		PRINTK("%s:%d:initialization fails! ret = %d\n", __func__,
		       __LINE__, ret);

	ret = pmc_rdp_power_up();
	if (ret)
		PRINTK("%s:%d:initialization fails! ret = %d\n", __func__,
		       __LINE__, ret);

#if defined(PMB_ADDR_RDP)
	/* we will just put all the modules off reset */
	ret = WriteBPCMRegister(PMB_ADDR_RDP, BPCMRegOffset(sr_control),
				0xffffffff);
	if (ret)
		PRINTK("%s:%d:failed to configure PMB RDP at word offset of "
		       "0x%02x\n", __func__, __LINE__,
		       (unsigned int)BPCMRegOffset(sr_control));
#endif

	return ret;
}

int pmc_rdp_shut_down(void)
{
#if defined(PMB_ADDR_RDP)
	int ret;

	/* put all RDP modules in reset state */
	ret = WriteBPCMRegister(PMB_ADDR_RDP, BPCMRegOffset(sr_control), 0);
	if (ret)
		PRINTK("%s:%d:failed to configure PMB RDP at word offset of "
		       "0x%02x\n", __func__, __LINE__,
		       (unsigned int)BPCMRegOffset(sr_control));
	return ret;
#else
	return -1;
#endif
}
