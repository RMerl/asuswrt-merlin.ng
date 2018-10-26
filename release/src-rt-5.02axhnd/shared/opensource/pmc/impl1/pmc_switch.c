/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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
#include <linux/delay.h>
#else
#include "lib_printf.h"
#endif

#ifndef _CFE_
#define PRINTK	printk
#else
#define PRINTK	xprintf
#endif

#include "pmc_drv.h"
#include "pmc_switch.h"
#include "BPCM.h"
#include "boardparms.h"

int pmc_switch_power_up(void)
{
	int ret;

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158) ||\
	defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM94908_) || defined(_BCM963158_) 
#if defined(CONFIG_BCM963158) || defined(_BCM963158_)
        BPCM_GLOBAL_CNTL_1 global_cntl_1; 
        BPCM_GLOBAL_CNTL_2 global_cntl_2; 
#else
	BPCM_GLOBAL_CNTL global_cntl;
#endif
#if defined(CONFIG_BCM963148) || defined(_BCM963148_)
	BPCM_SGPHY_CNTL sgphy_cntl;
#endif
	const ETHERNET_MAC_INFO* Enet;
	int i, cbport;
	int z1_clk_enable = 0, z2_clk_enable = 0;

	/* determine if RGMII port is defined in bp and then turn on rgmii clock respectively.
	   P5 and P7 is controlled by zone 1 and P11 and P12 by zone 2 */
	Enet = BpGetEthernetMacInfoArrayPtr();
#if defined(CONFIG_BCM963158) || defined(_BCM963158_)
	if( Enet[1].sw.port_map&0x8 && IsRGMII(Enet[1].sw.phy_id[3]) )
		z1_clk_enable = 1;
		
#elif !defined(CONFIG_BCM94908) && !defined(_BCM94908_)
	/* check P5 and P7, they are on the second switch - SF2 */
	if( Enet[1].sw.port_map&0xa0 )
		z1_clk_enable = 1;
#endif
	/* check P11 and P12. they are on the crossbar so it can be either first and second switch */
	for (i = 0 ; i < BP_MAX_ENET_MACS ; i++) {
		cbport = BP_PHY_PORT_TO_CROSSBAR_PORT(11);
		if (Enet[i].sw.crossbar[cbport].switch_port != BP_CROSSBAR_NOT_DEFINED)  {
			z2_clk_enable = 1;
		}
#if !defined(CONFIG_BCM94908) && !defined(_BCM94908_)
		cbport = BP_PHY_PORT_TO_CROSSBAR_PORT(12);
		if (Enet[i].sw.crossbar[cbport].switch_port != BP_CROSSBAR_NOT_DEFINED)  {
			z2_clk_enable = 1;
		}
#endif
	}

#if defined(CONFIG_BCM94908) || defined(_BCM94908_)
	if( z2_clk_enable == 1 )
		z1_clk_enable = 1;
#endif

#if defined(CONFIG_BCM963148) || defined(_BCM963148_)
	sgphy_cntl.Reg32 = 0x33;
	ret = WriteBPCMRegister(PMB_ADDR_SWITCH,
			BPCMRegOffset(sgphy_cntl), sgphy_cntl.Reg32);
#endif

#if defined(CONFIG_BCM963158) || defined(_BCM963158_)
	ret = ReadBPCMRegister(PMB_ADDR_SWITCH,
			BPCMRegOffset(global_control_1), &global_cntl_1.Reg32);
	ret = ReadBPCMRegister(PMB_ADDR_SWITCH,
			BPCMRegOffset(global_control_2), &global_cntl_2.Reg32);

	global_cntl_1.Bits.z1_ck250_clk_en = 0;
	global_cntl_2.Bits.z2_ck250_clk_en = 0;
	ret = WriteBPCMRegister(PMB_ADDR_SWITCH,
			BPCMRegOffset(global_control_1), global_cntl_1.Reg32);
	ret = WriteBPCMRegister(PMB_ADDR_SWITCH,
			BPCMRegOffset(global_control_2), global_cntl_2.Reg32);

	ret = ReadBPCMRegister(PMB_ADDR_SWITCH,
			BPCMRegOffset(global_control_1), &global_cntl_1.Reg32);

	ret = ReadBPCMRegister(PMB_ADDR_SWITCH,
			BPCMRegOffset(global_control_2), &global_cntl_2.Reg32);
   	if( z1_clk_enable )
		global_cntl_1.Bits.z1_ck250_clk_en = 1;
	if( z2_clk_enable )
		global_cntl_2.Bits.z2_ck250_clk_en = 1;
	ret = WriteBPCMRegister(PMB_ADDR_SWITCH,
		BPCMRegOffset(global_control_1), global_cntl_1.Reg32);
	ret = WriteBPCMRegister(PMB_ADDR_SWITCH,
		BPCMRegOffset(global_control_2), global_cntl_2.Reg32);

#else
	ret = ReadBPCMRegister(PMB_ADDR_SWITCH,
			BPCMRegOffset(global_control), &global_cntl.Reg32);


	global_cntl.Bits.z1_ck250_clk_en = 0;
	global_cntl.Bits.z2_ck250_clk_en = 0;
	ret = WriteBPCMRegister(PMB_ADDR_SWITCH,
			BPCMRegOffset(global_control), global_cntl.Reg32);

	ret = ReadBPCMRegister(PMB_ADDR_SWITCH,
			BPCMRegOffset(global_control), &global_cntl.Reg32);
   	if( z1_clk_enable )
		global_cntl.Bits.z1_ck250_clk_en = 1;
	if( z2_clk_enable )
		global_cntl.Bits.z2_ck250_clk_en = 1;
	ret = WriteBPCMRegister(PMB_ADDR_SWITCH,
		BPCMRegOffset(global_control), global_cntl.Reg32);
#endif

	PRINTK("%s: Rgmii Tx clock zone1 enable %d zone2 enable %d. \n", __FUNCTION__, z1_clk_enable, z2_clk_enable);
#endif
	ret = PowerOnDevice(PMB_ADDR_SWITCH);
	return ret;
}

int pmc_switch_power_down(void)
{
	return PowerOffDevice(PMB_ADDR_SWITCH, 0);
}


void pmc_switch_clock_lowpower_mode (int low_power)
{
#if defined(CONFIG_BCM963158) && !defined(_CFE_)
	PLL_CHCFG_REG pll_ch23_cfg;

	if (low_power == 1)
	{
		/* lower the clock rate to the switch, by adjusting the SyncE PLL post divider */
		/* channel 3 is the dedicated 250MHz clock to the switch on BCM63158 */
		ReadBPCMRegister(PMB_ADDR_SYNC_PLL, PLLBPCMRegOffset(ch23_cfg), &pll_ch23_cfg.Reg32);
		pll_ch23_cfg.Bits.mdiv1 = 240;  /* switch PLL is from 3GMHz, divider of 240 will bring it to 12.5 MHz */
		WriteBPCMRegister(PMB_ADDR_SYNC_PLL, PLLBPCMRegOffset(ch23_cfg), pll_ch23_cfg.Reg32);
		mdelay(1);
		pll_ch23_cfg.Bits.mdiv_override1 = 1;
		WriteBPCMRegister(PMB_ADDR_SYNC_PLL, PLLBPCMRegOffset(ch23_cfg), pll_ch23_cfg.Reg32);
		mdelay(1);
	}
	else
	{
		/* change the switch core clock to 12.5 MHz */
		ReadBPCMRegister(PMB_ADDR_SYNC_PLL, PLLBPCMRegOffset(ch23_cfg), &pll_ch23_cfg.Reg32);
		pll_ch23_cfg.Bits.mdiv1 = 12;  /* switch PLL is from 3GMHz, divider of 12 will bring it to 250 MHz */
		WriteBPCMRegister(PMB_ADDR_SYNC_PLL, PLLBPCMRegOffset(ch23_cfg), pll_ch23_cfg.Reg32);
		mdelay(1);
		pll_ch23_cfg.Bits.mdiv_override1 = 1;
		WriteBPCMRegister(PMB_ADDR_SYNC_PLL, PLLBPCMRegOffset(ch23_cfg), pll_ch23_cfg.Reg32);
		mdelay(1);    
	}
#endif
    (void)low_power;
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_switch_power_up);
EXPORT_SYMBOL(pmc_switch_power_down);
EXPORT_SYMBOL(pmc_switch_clock_lowpower_mode);
int pmc_switch_init(void)
{
	int ret;

	ret = pmc_switch_power_up();
	if (ret != 0)
		PRINTK("%s:%d:initialization fails! ret = %d\n", __func__, __LINE__, ret);

	return ret;
}
#endif

