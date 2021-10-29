#if defined(CONFIG_BCM_KF_PCI_FIXUP)
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
#ifndef __PCIE_BCM963XX_H
#define __PCIE_BCM963XX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pcie_common.h>

/**************************************
  *  Defines
  **************************************/
#if defined(PCIEH_1)
#define NUM_CORE                    2
#else
#define NUM_CORE                    1
#endif

/**************************************
  *  Macros
  **************************************/

/**************************************
  *  Structures
  **************************************/

/**
 * BCM963xx PCIe Host Controller Driver control block structure
 * for common code interface (>PCIE3_CORE)
 *
 * @core_id: pcie core id
 * @core_gen: PCIe core GEN (1,2,3)
 * @core_rev: PCIe core ip revision
 * @hc_cfg:  host controller configuration
 */
struct bcm963xx_pcie_hcd
{
	int core_id;
	int core_rev;
	uint8 core_gen;
	struct bcm963xx_pcie_hc_cfg hc_cfg;
};



/**************************************
  *  Function declerations
  **************************************/
uint16 bcm63xx_pcie_mdio_read (int port, uint16 phyad, uint16 regad);
int bcm63xx_pcie_mdio_write (int port, uint16 phyad, uint16 regad,
	uint16 wrdata);

#ifdef __cplusplus
}
#endif

#endif /* __PCIE_BCM963XX_H */
#endif
