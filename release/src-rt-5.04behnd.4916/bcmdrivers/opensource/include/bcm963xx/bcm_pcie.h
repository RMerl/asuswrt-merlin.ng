/*
   Copyright (c) 2022 Broadcom
   All Rights Reserved

   <:label-BRCM:2022:DUAL/GPL:standard
   
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
#ifndef __BCM_PCIE_H__
#define __BCM_PCIE_H__

#if !defined(CONFIG_PHYS_ADDR_T_64BIT)
#define BCM_PCIE_MAP_ADDR_INVALID       (-1U)
#else
#define BCM_PCIE_MAP_ADDR_INVALID       (-1ULL)
#endif

/*
 * Configure incoming address window to an available bar
 *
 * param
 *   pdev    : pci device
 *   bar     : 0  search available bar
 *             >0 use the specific bar
 *   addr    : incoming address base (UBUS mapped)
 *   size    : size of incoming address window
 *             0: unconfig the existing bar
 *
 * return: Result of configuration
 *  0: Success
 * -ve: Failure
 */
extern int bcm_pcie_config_bar_addr(struct pci_dev *pdev, int bar,
	phys_addr_t addr, u32 size);

/*
 * Map PCIe incoming address to UBUS decodable address
 *
 * param
 *   pdev    : pci device
 *   addr    : address to be decoded
 *   size    : size of decode address window
 *             0: use default size
 *
 * return  : UBUS mapped PCIe incoming window address
 */
extern phys_addr_t bcm_pcie_map_bar_addr(struct pci_dev *pdev, phys_addr_t addr,
	u32 size);
#endif /* __BCM_PCIE_H__ */
