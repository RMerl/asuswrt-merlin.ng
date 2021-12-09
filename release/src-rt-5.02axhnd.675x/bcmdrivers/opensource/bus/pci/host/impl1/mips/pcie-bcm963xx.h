#if defined(CONFIG_BCM_KF_PCI_FIXUP)
/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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
