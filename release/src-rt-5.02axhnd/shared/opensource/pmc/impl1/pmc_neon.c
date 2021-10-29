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
#include "pmc_neon.h"
#include "BPCM.h"


#if defined(CONFIG_BCM963138)
static int pmc_is_neon_powered_up(void);
#endif /* CONFIG_BCM963138 */


void pmc_neon_power_up(void)
{
#if defined(CONFIG_BCM963138)
   ARM_CONTROL_REG arm_ctrl;
   ARM_CPUx_PWR_CTRL_REG arm_pwr_ctrl;

   if(pmc_is_neon_powered_up())
      return;

   ReadBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_control), &arm_ctrl.Reg32);
   ReadBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_neon_l2), &arm_pwr_ctrl.Reg32);

   /* 1) Power up Neon */
   arm_pwr_ctrl.Bits.pwr_on |= 1;
   WriteBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_neon_l2), arm_pwr_ctrl.Reg32);
   do {
      ReadBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_neon_l2), &arm_pwr_ctrl.Reg32);
   } while ((arm_pwr_ctrl.Bits.pwr_on_status & 0x1) == 0);

   arm_pwr_ctrl.Bits.pwr_ok |= 1;
   WriteBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_neon_l2), arm_pwr_ctrl.Reg32);
   do {
      ReadBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_neon_l2), &arm_pwr_ctrl.Reg32);
   } while ((arm_pwr_ctrl.Bits.pwr_ok_status & 0x1) == 0);

   arm_pwr_ctrl.Bits.clamp_on = 0;
   WriteBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_neon_l2), arm_pwr_ctrl.Reg32);

   /* 2) De-assert reset to Neon */
   arm_ctrl.Bits.neon_reset_n = 1;
   WriteBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_control), arm_ctrl.Reg32);
#endif /* CONFIG_BCM963138 */
}

void pmc_neon_power_down(void)
{
#if defined(CONFIG_BCM963138)
   ARM_CONTROL_REG arm_ctrl;
   ARM_CPUx_PWR_CTRL_REG arm_pwr_ctrl;

   if(!pmc_is_neon_powered_up())
      return;

   ReadBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_control), &arm_ctrl.Reg32);
   ReadBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_neon_l2), &arm_pwr_ctrl.Reg32);

   /* 1) assert reset to Neon */
   arm_ctrl.Bits.neon_reset_n = 0;
   WriteBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_control), arm_ctrl.Reg32);

   /* 2) Power down Neon */
   arm_pwr_ctrl.Bits.clamp_on = 1;
   WriteBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_neon_l2), arm_pwr_ctrl.Reg32);

   arm_pwr_ctrl.Bits.pwr_ok &= 0xe;
   WriteBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_neon_l2), arm_pwr_ctrl.Reg32);
   do {
      ReadBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_neon_l2), &arm_pwr_ctrl.Reg32);
   } while ((arm_pwr_ctrl.Bits.pwr_ok_status & 0x1) == 1);

   arm_pwr_ctrl.Bits.pwr_on &= 0xe;
   WriteBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_neon_l2), arm_pwr_ctrl.Reg32);
   do {
      ReadBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_neon_l2), &arm_pwr_ctrl.Reg32);
   } while ((arm_pwr_ctrl.Bits.pwr_on_status & 0x1) == 1);
#endif /* CONFIG_BCM963138 */
}

#if defined(CONFIG_BCM963138)
static int pmc_is_neon_powered_up(void)
{
   ARM_CONTROL_REG arm_ctrl;
   ARM_CPUx_PWR_CTRL_REG arm_pwr_ctrl;

   ReadBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_control), &arm_ctrl.Reg32);
   ReadBPCMRegister(PMB_ADDR_AIP, ARMBPCMRegOffset(arm_neon_l2), &arm_pwr_ctrl.Reg32);

   /* check if neon is on and running aready. */
   return ((arm_ctrl.Bits.neon_reset_n == 1) &&
         (arm_pwr_ctrl.Bits.clamp_on == 0) &&
         (arm_pwr_ctrl.Bits.pwr_ok & 0x1) &&
         (arm_pwr_ctrl.Bits.pwr_on & 0x1));
}
#endif /* CONFIG_BCM963138 */


#ifndef _CFE_
EXPORT_SYMBOL(pmc_neon_power_up);
EXPORT_SYMBOL(pmc_neon_power_down);
#endif
