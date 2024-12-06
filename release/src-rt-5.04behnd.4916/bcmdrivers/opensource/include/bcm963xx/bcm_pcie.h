/*
   Copyright (c) 2022 Broadcom
   All Rights Reserved

   <:label-BRCM:2022:DUAL/GPL:standard

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
