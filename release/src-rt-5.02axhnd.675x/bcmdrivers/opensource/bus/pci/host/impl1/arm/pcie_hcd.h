/*
    Copyright (c) 2019 Broadcom
    All Rights Reserved

    <:label-BRCM:2019:DUAL/GPL:standard
    
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
#ifndef __PCIE_HCD_H
#define __PCIE_HCD_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 *
 *  Defines
 *
 */

/*
 *
 *  Macros
 *
 */

/*
 *  Structures
 *
 */

/* Forward declerations */
struct bcm963xx_pcie_hcd;

/*
 *
 *  Function declerations
 *
 */
int pcie_hcd_procfs_init(struct bcm963xx_pcie_hcd *pdrv);
void pcie_hcd_procfs_deinit(struct bcm963xx_pcie_hcd *pdrv);
#ifdef __cplusplus
}
#endif

#endif /* __PCIE_HCD_H */
