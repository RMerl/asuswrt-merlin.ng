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
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(_BCM94908_)
	BPCM_GLOBAL_CNTL global_cntl;
#if defined(CONFIG_BCM963148)
	BPCM_SGPHY_CNTL sgphy_cntl;
#endif

#if !defined(CONFIG_BCM94908) && !defined(_BCM94908_)
	const ETHERNET_MAC_INFO* Enet;
	int i, cbport;
#endif
	int z1_clk_enable = 0, z2_clk_enable = 0;

#if !defined(CONFIG_BCM94908) && !defined(_BCM94908_)
	/* determine if RGMII port is defined in bp and then turn on rgmii clock respectively.
	   P5 and P7 is controlled by zone 1 and P11 and P12 by zone 2 */
	Enet = BpGetEthernetMacInfoArrayPtr();
	/* check P5 and P7, they are on the second switch - SF2 */
	if( Enet[1].sw.port_map&0xa0 )
		z1_clk_enable = 1;
	/* check P11 and P12. they are on the crossbar so it can be either first and second switch */
	for (i = 0 ; i < BP_MAX_ENET_MACS ; i++) {
		cbport = BP_PHY_PORT_TO_CROSSBAR_PORT(11);
		if (Enet[i].sw.crossbar[cbport].switch_port != BP_CROSSBAR_NOT_DEFINED)  {
			z2_clk_enable = 1;
		}
		cbport = BP_PHY_PORT_TO_CROSSBAR_PORT(12);
		if (Enet[i].sw.crossbar[cbport].switch_port != BP_CROSSBAR_NOT_DEFINED)  {
			z2_clk_enable = 1;
		}
	}
#else
	/* determine if RGMII port is defined in bp and then turn on rgmii clock respectively.
	   4908 only one RGMII P11 port controlled  by zone 2. For now set both on per sim code */
	z1_clk_enable = 1;
	z2_clk_enable = 1;
#endif

#if defined(CONFIG_BCM963148)
	sgphy_cntl.Reg32 = 0x33;
	ret = WriteBPCMRegister(PMB_ADDR_SWITCH,
			BPCMRegOffset(sgphy_cntl), sgphy_cntl.Reg32);
#endif

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

	PRINTK("%s: Rgmii Tx clock zone1 enable %d zone2 enable %d. \n", __FUNCTION__, z1_clk_enable, z2_clk_enable);
#endif
	ret = PowerOnDevice(PMB_ADDR_SWITCH);

	return ret;
}

int pmc_switch_power_down(void)
{
	return PowerOffDevice(PMB_ADDR_SWITCH, 0);
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_switch_power_up);
EXPORT_SYMBOL(pmc_switch_power_down);
int pmc_switch_init(void)
{
	int ret;

	ret = pmc_switch_power_up();
	if (ret != 0)
		PRINTK("%s:%d:initialization fails! ret = %d\n", __func__, __LINE__, ret);

	return ret;
}
#endif

