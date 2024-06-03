/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom
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

/*
 *******************************************************************************
 * File Name : fpi.h
 *
 *******************************************************************************
 */

#ifndef __FPI_H_INCLUDED__
#define __FPI_H_INCLUDED__
#include <linux/version.h>
#include "fpi_defs.h"

#define FPI_VERSION	"v1.0"
#define FPI_MODNAME	"Broadcom Flow Provisioning Interface Module"

#ifndef CONFIG_BCM_RUNNER_MAX_FLOWS
#define FPI_MAX_FLOWS	(32 * 1024)
#else
#define FPI_MAX_FLOWS	(CONFIG_BCM_RUNNER_MAX_FLOWS)
#endif

#define FPI_MAX_AP_MAC	8

/*
 * Setting system FPI mode
 * Return:
 * 	0 if succeeds. otherwise, fails */
int fpi_set_mode(fpi_mode_t mode);

/*
 * Getting system FPI mode
 * Return:
 * 	0 if succeeds. otherwise, fails */
int fpi_get_mode(fpi_mode_t *mode_p);

/*
 * Adding flow.
 * Input:
 * 	flow_p: pointer to flow data structure 
 * Output:
 * 	handle_p: memory to store returned handle
 * Return:
 * 	0 if succeeds. otherwise, fails
 */
int fpi_add_flow(fpi_flow_t *flow_p, uint32_t *handle_p);

/*
 * Deleting flow by providing the handle
 * Input:
 * 	handle: handle for the flow
 * Return:
 * 	0 if succeeds. otherwise, fails
 */
int fpi_delete_flow_by_handle(uint32_t handle);

/*
 * Deleting flow by providing the flow key info 
 * Input:
 * 	key_p: pointer to flow key data structure
 * Return:
 * 	0 if succeeds. otherwise, fails
 */
int fpi_delete_flow_by_key(fpi_key_t *key_p);

/*
 * Getting the statistics for flow
 * Input:
 * 	handle: handle for the flow
 * Output:
 * 	stat_p: pointer to statistics data structure (includes packet and
 * 		byte counters)
 * Return:
 * 	0 if succeeds. otherwise, fails
 */
int fpi_get_stat(uint32_t handle, fpi_stat_t *stat_p);

/*
 * Getting the flow info
 * Input:
 * 	handle: handle for the flow
 * Output:
 * 	flow_p: pointer to flow data structure
 * Return:
 * 	0 if succeeds. otherwise, fails
 */
int fpi_get_flow(uint32_t handle, fpi_flow_t *flow_p);

/*
 * Adding the AP MAC address
 * Input:
 * 	mac_addr_p: pointer to 6-byte array for MAC address
 * Output:
 * Return:
 * 	0 if succeeds. otherwise, fails
 */
int fpi_add_ap_mac(uint8_t *mac_addr_p);

/*
 * Deleting the AP MAC address
 * Input:
 * 	mac_addr_p: pointer to 6-byte array for MAC address
 * Output:
 * Return:
 * 	0 if succeeds. otherwise, fails
 */
int fpi_delete_ap_mac(uint8_t *mac_addr_p);

#endif /* __FPI_H_INCLUDED__ */
