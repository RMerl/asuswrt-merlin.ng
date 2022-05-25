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
#if defined(WLAN1_PHYS_BASE) || defined(WLAN0_PHYS_BASE)
#define NUM_VCORE                           1
#else
#define NUM_VCORE                           0
#endif

/*
 * Maximum virtual devices per domain
 *
 * When two devices are present, default pcie enumeration is
 *   <domain>:<bus>:<slot0>
 *   <domain>:<bus>:<slot1>
 *
 * This can be changed to below by changing the NUM_VCORE to 2
 *   <domain0>:<bus>:<slot0>
 *   <domain1>:<bus>:<slot0>
 */
#define MAX_NUM_VDEV                        2

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
