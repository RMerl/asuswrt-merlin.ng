/****************************************************************************
 *
 * <:copyright-BRCM:2015:DUAL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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
