/*
    Copyright (c) 2021 Broadcom
    All Rights Reserved

    <:label-BRCM:2021:DUAL/GPL:standard
    
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
#ifndef __PCIE_WLEP_H
#define __PCIE_WLEP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <bcmtypes.h>

/*
 * +-----------------------------------------------------
 *  Defines
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 *  Macros
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 *  Structures
 * +-----------------------------------------------------
 */
struct wlep_dev;


/*
 * +-----------------------------------------------------
 *  Function declerations
 * +-----------------------------------------------------
 */
/* To HC */
extern int wlep_mdio_write(struct wlep_dev *dev, u16 phyad, u16 regad,
	u16 wrdata);
extern u16 wlep_mdio_read(struct wlep_dev *dev, u16 phyad, u16 regad);

extern int wlep_set_power_state(struct wlep_dev *dev, pci_power_t state);
extern int wlep_get_mlw(struct wlep_dev *dev);

extern int wlep_config_dev(struct wlep_dev *dev);
extern struct wlep_dev* wlep_probe_dev(struct pci_bus *root_bus, u32 owin,
	int *prc);
extern void wlep_free_dev(struct wlep_dev *dev);


/*
 * +-----------------------------------------------------
 *  Variable declerations
 * +-----------------------------------------------------
 */

#ifdef __cplusplus
}
#endif

#endif /* __PCIE_WLEP_H */
