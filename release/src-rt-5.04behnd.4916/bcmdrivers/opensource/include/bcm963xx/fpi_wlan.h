
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
 * File Name : fpi_wlan.h
 *
 *******************************************************************************
 */

#ifndef __FPI_WLAN_H_INCLUDED__
#define __FPI_WLAN_H_INCLUDED__
#include <linux/netdevice.h>
#include <linux/blog.h>

typedef union fpi_wl_metadata {
	uint32_t     wl;     /* Generic 32bit access */
	BlogWfd_t    wfd;    /* Key for WiFi Forwarding Driver */
	BlogRnr_t    hwo;    /* Key for hardware offload */
} fpi_wl_metadata_t;

#define FPI_WL_KEY_VALID(metadata)     (((fpi_wl_metadata_t)(metadata)).hwo.is_tx_hw_acc_en)
typedef int (*fpi_get_wl_metadata_func_t)(struct net_device *net,
		uint8_t *sa, uint8_t *da, uint8_t prio, uint32_t *metadata);

int fpi_register_wl_get_metadata(fpi_get_wl_metadata_func_t func_p);
int fpi_register_dhd_get_metadata(fpi_get_wl_metadata_func_t func_p);

#endif /* __FPI_WLAN_H_INCLUDED__ */

