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
#ifndef __PCIE_VCORE_H
#define __PCIE_VCORE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * +-----------------------------------------------------
 *  Defines
 * +-----------------------------------------------------
 */

/* Number of virtual cores */
#define NUM_VCORE                           MAX_NUM_VIRT_PCIE_CORES

/*
 * Maximum virtual devices per domain
 */
#define MAX_NUM_VDEV                        1

#define BCMVPCIE_HC_DEV_NAME                "pcie-vcore"
#define BCMVPCIE_HC_DRV_NAME                BCMVPCIE_HC_DEV_NAME

/*
 * +-----------------------------------------------------
 *  Structures
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 *  Macros
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 *  Function declerations
 * +-----------------------------------------------------
 */
#ifdef __cplusplus
}
#endif

#endif /* __PCIE_VCORE_H */
