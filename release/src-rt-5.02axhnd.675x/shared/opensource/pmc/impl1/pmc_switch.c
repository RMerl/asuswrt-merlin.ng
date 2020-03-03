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
#include "lib_types.h"
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

#if defined(CONFIG_BCM963158) && (CONFIG_BRCM_CHIP_REV==0x63158A0)
#define SWITCH_PLL_NORMAL   (0xC) /* switch PLL is from 3GMHz, divider of 12 (0xD) will bring it to 250 MHz */
#define SWITCH_PLL_LOW_PWR  (0xF0) /* switch PLL is from 3GMHz, divider of 240 (0xF0) will bring it to 12.5 MHz */
#elif defined(CONFIG_BCM963158)
/* From VLSI:
   PLL VCO frequency increased by 1.5x from A0 to B0;
   In normal mode we increased Switch frequency from A0 to B0 and that's why the value is less than 1.5x.
   Low power mode can stay at 1.5x which provides the same PLL output freq between A0 and B0.
*/
#define SWITCH_PLL_NORMAL   (0x11)
#define SWITCH_PLL_LOW_PWR  (0xFF)
#elif defined(CONFIG_BCM963178)
#define SWITCH_PLL_NORMAL   (0x06)
#define SWITCH_PLL_LOW_PWR  (0xFF)
#endif

void pmc_switch_clock_lowpower_mode (int low_power)
{
#if defined(CONFIG_BCM963158) && !defined(_CFE_)
	PLL_CHCFG_REG pll_ch23_cfg;

	if (low_power == 1)
	{
		/* lower the clock rate to the switch, by adjusting the SyncE PLL post divider */
		/* channel 3 is the dedicated 250MHz clock to the switch on BCM63158 */
		ReadBPCMRegister(PMB_ADDR_SYNC_PLL, PLLBPCMRegOffset(ch23_cfg), &pll_ch23_cfg.Reg32);
		pll_ch23_cfg.Bits.mdiv1 = SWITCH_PLL_LOW_PWR;  
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
		pll_ch23_cfg.Bits.mdiv1 = SWITCH_PLL_NORMAL;  
		WriteBPCMRegister(PMB_ADDR_SYNC_PLL, PLLBPCMRegOffset(ch23_cfg), pll_ch23_cfg.Reg32);
		mdelay(1);
		pll_ch23_cfg.Bits.mdiv_override1 = 1;
		WriteBPCMRegister(PMB_ADDR_SYNC_PLL, PLLBPCMRegOffset(ch23_cfg), pll_ch23_cfg.Reg32);
		mdelay(1);    
	}
#elif defined(CONFIG_BCM963178) && !defined(_CFE_)
	PLL_CHCFG_REG pll_ch45_cfg;

	if (low_power == 1)
	{
		/* lower the clock rate to the switch, by adjusting the i_clk_vary post_divider */
		ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ch45_cfg), &pll_ch45_cfg.Reg32);
		pll_ch45_cfg.Bits.mdiv0 = SWITCH_PLL_LOW_PWR;
		WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ch45_cfg), pll_ch45_cfg.Reg32);
		mdelay(1);
		pll_ch45_cfg.Bits.mdiv_override0 = 1;
		WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ch45_cfg), pll_ch45_cfg.Reg32);
		mdelay(1);
	}
	else
	{
		/* change the switch core clock to 12.5 MHz */
		ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ch45_cfg), &pll_ch45_cfg.Reg32);
		pll_ch45_cfg.Bits.mdiv0 = SWITCH_PLL_NORMAL;
		WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ch45_cfg), pll_ch45_cfg.Reg32);
		mdelay(1);
		pll_ch45_cfg.Bits.mdiv_override0 = 1;
		WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ch45_cfg), pll_ch45_cfg.Reg32);
		mdelay(1);
	}
#endif
	(void)low_power;
}

#if defined(CONFIG_BCM963158) || defined (CONFIG_BCM963178)
void pmc_sysport_reset_system_port (int port)
{
	int status;
	uint32_t reg;
	(void) port;

	// for both BCM63158 and BCM63178, system port is tide to Zone 3 of the Switch BPCM
	status = ReadBPCMRegister(PMB_ADDR_SWITCH, BPCMRegOffset(sr_control), &reg);
	if (status == kPMC_NO_ERROR)
	{
		// system port is on zone 3 of the switch BPCM
		reg |= 0x8;
		status = WriteBPCMRegister(PMB_ADDR_SWITCH, BPCMRegOffset(sr_control), reg);

		if (status == kPMC_NO_ERROR)
		{
			reg &= ~0x8;
			status = WriteBPCMRegister(PMB_ADDR_SWITCH, BPCMRegOffset(sr_control), reg);
		}
	}
}
#endif

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

