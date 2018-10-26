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
extern void msleep (unsigned int);
#endif

#include "pmc_drv.h"
#include "bcm_map_part.h"
#include "pmc_pcm.h"
#include "BPCM.h"

void pmc_pcm_power_up(void)
{
#if !(defined(CONFIG_BCM96856) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846))
   BPCM_PWR_ZONE_N_CONTROL bpcmPwrZoneControl;
   BPCM_SR_CONTROL sr_control;
#endif

#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) 
   printk("Configuring PCM bus mux\n"); 
   /* This is needed to swap the endianness on the BUS */
   *(unsigned int*)PCM_BUS_BASE = 0x5;
   msleep(5);
#endif

#if !(defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856))
   /* Enable power only if it is currently down */
   ReadZoneRegister(PMB_ADDR_PCM, PCM_Zone_Main, BPCMZoneRegOffset(control), &bpcmPwrZoneControl.Reg32);

#if defined(CONFIG_BCM96858)
   {
#else
   if (!bpcmPwrZoneControl.Bits.pwr_on_state) {
#endif
      /* zone0 is not powered so the whole device is powered off */
      /* just power on the device */
      PowerOnDevice(PMB_ADDR_PCM);

      /* Assert PCM SoftResets via BPCM */
      ReadBPCMRegister(PMB_ADDR_PCM, BPCMRegOffset(sr_control), &sr_control.Reg32);
      sr_control.Bits.sr |= ( BPCM_PCM_SRESET_PCM_N );
      WriteBPCMRegister(PMB_ADDR_PCM, BPCMRegOffset(sr_control), sr_control.Reg32);

      /* Sleep to ensure full soft reset */
      msleep(10);

      /* De-assert PCM SoftResets via BPCM */
      ReadBPCMRegister(PMB_ADDR_PCM, BPCMRegOffset(sr_control), &sr_control.Reg32);
      sr_control.Bits.sr &= ~( BPCM_PCM_SRESET_PCM_N );
      WriteBPCMRegister(PMB_ADDR_PCM, BPCMRegOffset(sr_control), sr_control.Reg32);

       /* Sleep to ensure full soft reset */
      msleep(10);
   }
#endif 
}

void pmc_pcm_power_down(void)
{
#if !(defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856))
   BPCM_SR_CONTROL sr_control;

   /* Assert PCM SoftResets via BPCM */
   ReadBPCMRegister(PMB_ADDR_PCM, BPCMRegOffset(sr_control), &sr_control.Reg32);
   sr_control.Bits.sr |= ( BPCM_PCM_SRESET_PCM_N );
   WriteBPCMRegister(PMB_ADDR_PCM, BPCMRegOffset(sr_control), sr_control.Reg32);

   /* Sleep to ensure full soft reset */
   msleep(10);

   /* Power off PCM zone */
   PowerOffZone(PMB_ADDR_PCM, PCM_Zone_PCM  );
#endif
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_pcm_power_up);
EXPORT_SYMBOL(pmc_pcm_power_down);
#endif

