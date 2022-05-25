/*
   Copyright (c) 2019 Broadcom Corporation
   All Rights Reserved

<:label-BRCM:2019:DUAL/GPL:standard

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

#include <linux/module.h>
#include <board.h>

#if !defined(CONFIG_BCM_PMC)
int pmc_init(void)
{
    return 0;   	
}
EXPORT_SYMBOL(pmc_init);

#endif

int pmc_usb_power_up(unsigned int usb_block)
{
    return 0;
}
EXPORT_SYMBOL(pmc_usb_power_up);

int pmc_usb_power_down(unsigned int usb_block)
{
    return 0;
}
EXPORT_SYMBOL(pmc_usb_power_down);

int pmc_xport_power_on(int xport_id)
{
    return 0;
}
EXPORT_SYMBOL(pmc_xport_power_on);

void pmc_mpm_en(void)
{
    return;
}
EXPORT_SYMBOL(pmc_mpm_en);

unsigned long get_rdp_freq(unsigned int* rdp_freq)
{
    *rdp_freq = 1000;
    
    return 0; 	
}
EXPORT_SYMBOL(get_rdp_freq);

int pmc_sdhci_set_base_clk( uint64_t freq )
{
    return 0;
}

void bcm_set_vreg_sync(void)
{
    return;
}

void ubus_master_port_init(void)
{
    return;
}
EXPORT_SYMBOL(ubus_master_port_init);

void bcm_ubus_config(void)
{
    return;
}
EXPORT_SYMBOL(bcm_ubus_config);

void ubus_cong_threshold_wr(int port_id, unsigned int val)
{
    return;
}
EXPORT_SYMBOL(ubus_cong_threshold_wr);

void wan_prbs_status(int *valid, uint32_t *errors)
{
   return;
}
EXPORT_SYMBOL(wan_prbs_status);

#include "wan_drv.h"
void wan_prbs_gen(uint32_t enable, int enable_host_tracking, int mode, serdes_wan_type_t wan_type, int *valid)
{
   return;
}
EXPORT_SYMBOL(wan_prbs_gen);

int kerSysGetMacAddress( unsigned char *pucaMacAddr, unsigned long ulId )
{
   pucaMacAddr[0] = 10;
   pucaMacAddr[1] = 2;
   pucaMacAddr[2] = 3;
   pucaMacAddr[3] = 4;
   pucaMacAddr[4] = 55;
   pucaMacAddr[5] = 66;
   return 0;
}
EXPORT_SYMBOL(kerSysGetMacAddress);

int kerSysScratchPadGet(char *tokName, char *tokBuf, int tokLen)
{
    return 0;
}
EXPORT_SYMBOL(kerSysScratchPadGet);

int kerSysScratchPadSet(char *tokName, char *tokBuf, int tokLen)
{
    return 0;
}
EXPORT_SYMBOL(kerSysScratchPadSet);

unsigned long kerSysGetMacAddressType( unsigned char *ifName )
{
    return 0;
}
EXPORT_SYMBOL(kerSysGetMacAddressType);

int kerSysReleaseMacAddress( unsigned char *pucaAddr )
{
   return 0;
}
EXPORT_SYMBOL(kerSysReleaseMacAddress);

void kerSysDeregisterDyingGaspHandler(char *devname)
{
   return;
}
EXPORT_SYMBOL(kerSysDeregisterDyingGaspHandler);


void kerSysRegisterDyingGaspHandler(char *devname, void *cbfn, void *context)
{
   return;
}EXPORT_SYMBOL(kerSysRegisterDyingGaspHandler);

void kerSysRegisterDyingGaspHandlerV2(char *devname, void *cbfn, void *context)
{
  return;  
}EXPORT_SYMBOL(kerSysRegisterDyingGaspHandlerV2);

unsigned long kerSysGetSdramSize(void)
{
   return 256*1024*1024;
}
EXPORT_SYMBOL(kerSysGetSdramSize);

int kerSysGetDslPhyEnable(void)
{
    return 1;
}
EXPORT_SYMBOL(kerSysGetDslPhyEnable);

void kerSysLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState)
{
}
EXPORT_SYMBOL(kerSysLedCtrl);

void kerSysSendtoMonitorTask(int msgType, char *msgData, int msgDataLen)
{
}
EXPORT_SYMBOL(kerSysSendtoMonitorTask);

int kerSysGetGpioValue(unsigned short bpGpio)
{
    return 0;
}
EXPORT_SYMBOL(kerSysGetGpioValue);

int kerSysSetGpioDirInput(unsigned short bpGpio)
{
    return 0;
}
EXPORT_SYMBOL(kerSysSetGpioDirInput);

void kerSysSetGpioDir(unsigned short bpGpio)
{
}
EXPORT_SYMBOL(kerSysSetGpioDir);
void kerSysSetGpioState(unsigned short bpGpio, GPIO_STATE_t state)
{
}
EXPORT_SYMBOL(kerSysSetGpioState);
int ext_irq_connect(int irq, void* param, void *isr)
{
    return 0;
}
EXPORT_SYMBOL(ext_irq_connect);

void BcmHalSetIrqAffinity(unsigned int irq, struct cpumask *mask)
{
	return;
}
EXPORT_SYMBOL(BcmHalSetIrqAffinity);

int sgmii_bpcm_init(void)
{
	return 0;
}
EXPORT_SYMBOL(sgmii_bpcm_init);

void pmc_rgmii_clk_en(void)
{
	return;
}
EXPORT_SYMBOL(pmc_rgmii_clk_en);
