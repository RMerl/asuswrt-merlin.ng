/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

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
#ifndef _CFE_
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#else
#include "lib_printf.h"
#endif

#ifndef _CFE_
#define PRINTK	printk
#else
#define PRINTK	xprintf
#endif

#include "pmc_drv.h"
#include "pmc_rdp.h"
#include "BPCM.h"

static int pmc_rdp_start_pll_with_clk(uint32_t clk)
{
	uint32_t bpcmResReg;
	uint32_t error;

	if (WriteBPCMRegister(PMB_ADDR_RDPPLL, PLLBPCMRegOffset(resets), 0))
		PRINTK("failed to configure PMB RDPPLL at word offset of 0x%02x\n", (unsigned int)PLLBPCMRegOffset(resets));

#if 0	// FIXME! after we know how to pass the RDP clk info, also clean the hardcode value?
	if (clk == 550MHz) {
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0x7, 0x00000084))
			PRINTK("failed to configure PMB RDPPLL 0x7\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0x8, 0x80000002))
			PRINTK("failed to configure PMB RDPPLL 0x8\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0xb, 0x800c8006))
			PRINTK("failed to configure PMB RDPPLL 0xb\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0xc, 0x81008021))
			PRINTK("failed to configure PMB RDPPLL 0xc\n");
		PRINTK("%s:setting up RDP PLL to run at reduced speed of 550/275 MHz\n", __func__);	
	} else if (clk == 400MHz) {
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0x7, 0x00000080))
			PRINTK("failed to configure PMB RDPPLL 0x7\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0x8, 0x80000002))
			PRINTK("failed to configure PMB RDPPLL 0x8\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0xb, 0x80108008))
			PRINTK("failed to configure PMB RDPPLL 0xb\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0xc, 0x81008020))
			PRINTK("failed to configure PMB RDPPLL 0xc\n");
		PRINTK("%s:setting up RDP PLL to run at reduced speed of 400/200 MHz\n", __func__);	
	} else if (clk == 200MHz) {
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0x7, 0x00000080))
			PRINTK("failed to configure PMB RDPPLL 0x7\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0x8, 0x80000002))
			PRINTK("failed to configure PMB RDPPLL 0x8\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0xb, 0x80208010))
			PRINTK("failed to configure PMB RDPPLL 0xb\n");
		if (WriteBPCMRegister(PMB_ADDR_RDPPLL, 0xc, 0x81008020))
			PRINTK("failed to configure PMB RDPPLL 0xc\n");
		PRINTK("%s:setting up RDP PLL to run at reduced speed of 200/100 MHz\n", __func__);	
	}
#endif

	error = ReadBPCMRegister(PMB_ADDR_RDPPLL, PLLBPCMRegOffset(resets),
			(uint32*)&bpcmResReg);
	if (error)
		PRINTK("Failed to ReadBPCMRegister RDPPLL block at word offset "
				"of 0x%02x\n", (unsigned int)PLLBPCMRegOffset(resets));

	if (WriteBPCMRegister(PMB_ADDR_RDPPLL, PLLBPCMRegOffset(resets),
				(bpcmResReg | (1 << 0))))
		PRINTK("failed to configure PMB RDPPLL of 0x%02x\n",
				(unsigned int)PLLBPCMRegOffset(resets));

	do {
		error = ReadBPCMRegister(PMB_ADDR_RDPPLL,
				PLLBPCMRegOffset(stat), (uint32*)&bpcmResReg);
		if (error)
			PRINTK("Failed to ReadBPCMRegister RDPPLL block "
					"0x%02x\n", (unsigned int)PLLBPCMRegOffset(stat));
	} while (!(bpcmResReg & 0x80000000));

	error = ReadBPCMRegister(PMB_ADDR_RDPPLL, PLLBPCMRegOffset(resets),
			(uint32*)&bpcmResReg);
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
	return PowerOnDevice(PMB_ADDR_RDP);
#if 0
	int i, ret;
	BPCM_PWR_ZONE_N_CONTROL pwr_zone_ctrl;

	for (i = 0; i< PMB_ZONES_RDP;i++) {
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
	return PowerOffDevice(PMB_ADDR_RDP, 0);
#if 0
	int i, ret;
	BPCM_PWR_ZONE_N_CONTROL pwr_zone_ctrl;

	for (i = 0; i< PMB_ZONES_RDP;i++) {
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

	/* put all RDP modules in reset state */
	ret = WriteBPCMRegister(PMB_ADDR_RDP, BPCMRegOffset(sr_control), 0);
	if (ret)
		PRINTK("%s:%d:failed to configure PMB RDP at word offset of "
				"0x%02x\n", __func__, __LINE__,
				(unsigned int)BPCMRegOffset(sr_control));

	ret = pmc_rdp_power_down();
	if (ret)
		PRINTK("%s:%d:initialization fails! ret = %d\n", __func__,
				__LINE__, ret);

	ret = pmc_rdp_power_up();
	if (ret)
		PRINTK("%s:%d:initialization fails! ret = %d\n", __func__,
				__LINE__, ret);

	/* we will just put all the modules off reset */
	ret = WriteBPCMRegister(PMB_ADDR_RDP, BPCMRegOffset(sr_control),
			0xffffffff);
	if (ret)
		PRINTK("%s:%d:failed to configure PMB RDP at word offset of "
				"0x%02x\n", __func__, __LINE__,
				(unsigned int)BPCMRegOffset(sr_control));

	return ret;
}

int pmc_rdp_shut_down(void)
{
	int ret;

	/* put all RDP modules in reset state */
	ret = WriteBPCMRegister(PMB_ADDR_RDP, BPCMRegOffset(sr_control), 0);
	if (ret)
		PRINTK("%s:%d:failed to configure PMB RDP at word offset of "
				"0x%02x\n", __func__, __LINE__,
				(unsigned int)BPCMRegOffset(sr_control));
	return ret;
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_rdp_power_up);
EXPORT_SYMBOL(pmc_rdp_power_down);
EXPORT_SYMBOL(pmc_rdp_init);
EXPORT_SYMBOL(pmc_rdp_shut_down);
#if !defined(CONFIG_BCM94908)
postcore_initcall(pmc_rdp_init);
#endif
#endif

