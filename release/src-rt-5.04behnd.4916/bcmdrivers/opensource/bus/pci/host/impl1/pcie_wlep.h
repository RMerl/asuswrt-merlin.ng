/*
    Copyright (c) 2021 Broadcom
    All Rights Reserved

    <:label-BRCM:2021:DUAL/GPL:standard
    
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
