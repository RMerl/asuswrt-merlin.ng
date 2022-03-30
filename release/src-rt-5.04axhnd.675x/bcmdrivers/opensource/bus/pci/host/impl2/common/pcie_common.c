/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/module.h>
#include <linux/module.h>
#include <linux/delay.h>


#include <bcm_map_part.h>

#include <pcie_common.h>
#include <pcie-bcm947xx.h>


/**************************************
  *
  *  Defines
  *
  **************************************/

/**************************************
 *
 *  Macros
 *
 **************************************/
#define HCD_HC_CORE_CFG(config, core)              \
	(((0xF << ((core)*4)) & (config)) >> ((core)*4))


/**************************************
 *
 *  Structures
 *
 **************************************/

/**************************************
 *
 *  Global variables
 *
 **************************************/
/*
 * config_ssc values
 *
 *     0 - disable
 *     1 - Enable
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16][15-12] [11-08] [07-04] [03-00]
 *                                                                        [core 3] [core 2] [core 1]
 */
u32 pcie_ssc_cfg = 0x0000;
module_param(pcie_ssc_cfg, int, S_IRUGO);

/*
 * config_speed values
 *
 *     0 - default (keep reset value)
 *     1 - 2.5Gbps
 *     2 - 5Gbps
 *     3 - 8 Gbps
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16][15-12] [11-08] [07-04] [03-00]
 *                                                                        [core 3] [core 2] [core 1]
 */
u32 pcie_speed_cfg = 0x0000;
module_param(pcie_speed_cfg, int, S_IRUGO);

/***********
  * HCD Logging
  ***********/
#ifdef HCD_DEBUG
int hcd_log_level = HCD_LOG_LVL_ERROR;
#endif

/**************************************
 *
 *  Global Function definitions
 *
 **************************************/

/*
  *
  * Function bcm947xx_pcie_hcd_init_hc_cfg (pdrv)
  *
  *
  *   Parameters:
  *    pdrv ... pointer to pcie core hcd data structure
  *
  *   Description:
  *     Initialize the port hc_cfg parameters from global storage area
  *
  *  Return: None
  */
void bcm947xx_pcie_hcd_init_hc_cfg(struct bcm947xx_pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	pdrv->hc_cfg.ssc = HCD_HC_CORE_CFG(pcie_ssc_cfg, pdrv->core_id) ? TRUE : FALSE;
	pdrv->hc_cfg.speed = HCD_HC_CORE_CFG(pcie_speed_cfg, pdrv->core_id);

	HCD_FN_EXT();
}

