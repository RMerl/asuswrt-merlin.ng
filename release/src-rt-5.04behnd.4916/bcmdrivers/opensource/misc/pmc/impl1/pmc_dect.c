/****************************************************************************
 *
 * <:copyright-BRCM:2015:DUAL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 *
 ***************************************************************************/

#include "pmc_drv.h"
#include "BPCM.h"
#include <linux/delay.h>

/*****************************************************************************
*  FUNCTION:   dect_power_up
*
*  PURPOSE:    Handle powering up of the dect block
*
*  PARAMETERS: none
*
*  RETURNS:    0 on success, error otherwise
*
*****************************************************************************/
int dect_power_up(void)
{
   BPCM_PWR_ZONE_N_CONTROL pwr_zone_ctrl;

   /* RESET DECT via BPCM ( Block Power Control Module ) */
   ReadBPCMRegister(PMB_ADDR_DECT_UBUS, BPCMRegOffset(zones[0].control),
                    &pwr_zone_ctrl.Reg32);
   pwr_zone_ctrl.Bits.pwr_dn_req = 0;
   pwr_zone_ctrl.Bits.dpg_ctl_en = 1;
   pwr_zone_ctrl.Bits.pwr_up_req = 1;
   pwr_zone_ctrl.Bits.mem_pwr_ctl_en = 1;
   pwr_zone_ctrl.Bits.blk_reset_assert = 1;
   WriteBPCMRegister(PMB_ADDR_DECT_UBUS, BPCMRegOffset(zones[0].control),
                     pwr_zone_ctrl.Reg32);
   mdelay(1);

   return 0;
}

/*****************************************************************************
*  FUNCTION:   dect_power_down
*
*  PURPOSE:    Handle powering down of the dect block
*
*  PARAMETERS: none
*
*  RETURNS:    0 on success, error otherwise
*
*****************************************************************************/
int dect_power_down(void)
{
   BPCM_PWR_ZONE_N_CONTROL pwr_zone_ctrl;

   /* RESET DECT via BPCM ( Block Power Control Module ) */
   ReadBPCMRegister(PMB_ADDR_DECT_UBUS, BPCMRegOffset(zones[0].control),
                    &pwr_zone_ctrl.Reg32);
   pwr_zone_ctrl.Bits.pwr_dn_req = 1;
   pwr_zone_ctrl.Bits.dpg_ctl_en = 1;
   pwr_zone_ctrl.Bits.pwr_up_req = 0;
   WriteBPCMRegister(PMB_ADDR_DECT_UBUS, BPCMRegOffset(zones[0].control),
                     pwr_zone_ctrl.Reg32);

   return 0;
}

EXPORT_SYMBOL(dect_power_up);
EXPORT_SYMBOL(dect_power_down);
