/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:> 
*/
#ifndef _CFE_
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#endif

#include "pmc_drv.h"
#include "pmc_apm.h"
#include "BPCM.h"
#include "bcm_ubus4.h"

void pmc_apm_power_up(void)
{
   BPCM_PWR_ZONE_N_CONTROL bpcmPwrZoneControl;
   BPCM_SR_CONTROL sr_control;
   int clkrst_zone0 = 0;
     
   /* Enable power only if it is currently down */
   ReadZoneRegister(PMB_ADDR_APM, APM_Zone_Main, BPCMZoneRegOffset(control), &bpcmPwrZoneControl.Reg32);
   if (!bpcmPwrZoneControl.Bits.pwr_on_state) {
      /* Enable 200Mhz Clock via BPCM. All FCW calculations are based on this 200Mhz Clock */
      /* XXX 63138/63148 don't have this register */
      WriteZoneRegister(PMB_ADDR_CHIP_CLKRST, clkrst_zone0, BPCMZoneRegOffset(config2), 0);
   
      msleep(1);

      /* RESET APM via BPCM ( Block Power Control Module ) */
      ResetDevice( PMB_ADDR_APM);
      ResetZone(PMB_ADDR_APM, APM_Zone_Main );
      ResetZone(PMB_ADDR_APM, APM_Zone_Audio);
      ResetZone(PMB_ADDR_APM, APM_Zone_HVG  );
      ResetZone(PMB_ADDR_APM, APM_Zone_BMU  );

#if defined (CONFIG_BCM96858) || defined(_BCM96858_)
      ubus_register_port(UCB_NODE_ID_SLV_APM);
      ubus_register_port(UCB_NODE_ID_MST_APM);
#endif
  
      /* Assert APM SoftResets via BPCM */
      ReadBPCMRegister(PMB_ADDR_APM, BPCMRegOffset(sr_control), &sr_control.Reg32);
      sr_control.Bits.sr |= ( BPCM_APM_SRESET_AUDIO_N | BPCM_APM_SRESET_200_N  |
                       BPCM_APM_SRESET_HVGA_N  | BPCM_APM_SRESET_HVGB_N | 
                       BPCM_APM_SRESET_BMU_N  ) ;    
      WriteBPCMRegister(PMB_ADDR_APM, BPCMRegOffset(sr_control), sr_control.Reg32);

      /* Sleep to ensure full soft reset */
      msleep(1);
   
      /* De-Assert APM SoftResets via BPCM */
      ReadBPCMRegister(PMB_ADDR_APM, BPCMRegOffset(sr_control), &sr_control.Reg32);
      sr_control.Bits.sr &= ~( BPCM_APM_SRESET_AUDIO_N | BPCM_APM_SRESET_200_N  |
                        BPCM_APM_SRESET_HVGA_N  | BPCM_APM_SRESET_HVGB_N | 
                        BPCM_APM_SRESET_BMU_N  ) ;    
      WriteBPCMRegister(PMB_ADDR_APM, BPCMRegOffset(sr_control), sr_control.Reg32);

      /* Sleep to ensure full soft reset */
      msleep(1);
   }
}

void pmc_apm_power_down(void)
{
#if defined (CONFIG_BCM96858) || defined(_BCM96858_)
    ubus_deregister_port(UCB_NODE_ID_SLV_APM);
    ubus_deregister_port(UCB_NODE_ID_MST_APM);
#endif

   /* Power off zones via BPCM ( Block Power Control Module ) */
   PowerOffZone(PMB_ADDR_APM, APM_Zone_BMU  );
   PowerOffZone(PMB_ADDR_APM, APM_Zone_HVG  );
   PowerOffZone(PMB_ADDR_APM, APM_Zone_Audio);
   PowerOffZone(PMB_ADDR_APM, APM_Zone_Main );
}

void pmc_pcm_power_up(void)
{
   BPCM_SR_CONTROL sr_control;

   /* RESET PCM via BPCM ( Block Power Control Module ) */
   ResetZone( PMB_ADDR_APM, APM_Zone_PCM );

   /* Assert PCM SoftResets via BPCM */
   ReadBPCMRegister(PMB_ADDR_APM, BPCMRegOffset(sr_control), &sr_control.Reg32);
   sr_control.Bits.sr |= ( BPCM_APM_SRESET_PCM_N );
   WriteBPCMRegister(PMB_ADDR_APM, BPCMRegOffset(sr_control), sr_control.Reg32);

   /* Sleep to ensure full soft reset */
   msleep(10);

   /* De-assert PCM SoftResets via BPCM */
   ReadBPCMRegister(PMB_ADDR_APM, BPCMRegOffset(sr_control), &sr_control.Reg32);
   sr_control.Bits.sr &= ~( BPCM_APM_SRESET_PCM_N );
   WriteBPCMRegister(PMB_ADDR_APM, BPCMRegOffset(sr_control), sr_control.Reg32);

    /* Sleep to ensure full soft reset */
   msleep(10);
}

void pmc_pcm_power_down(void)
{
   BPCM_SR_CONTROL sr_control;

   /* Assert PCM SoftResets via BPCM */
   ReadBPCMRegister(PMB_ADDR_APM, BPCMRegOffset(sr_control), &sr_control.Reg32);
   sr_control.Bits.sr |= ( BPCM_APM_SRESET_PCM_N );
   WriteBPCMRegister(PMB_ADDR_APM, BPCMRegOffset(sr_control), sr_control.Reg32);

   /* Sleep to ensure full soft reset */
   msleep(10);

   /* Power off PCM zone */
   PowerOffZone(PMB_ADDR_APM, APM_Zone_PCM  );
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_apm_power_up);
EXPORT_SYMBOL(pmc_apm_power_down);
EXPORT_SYMBOL(pmc_pcm_power_up);
EXPORT_SYMBOL(pmc_pcm_power_down);
#endif

