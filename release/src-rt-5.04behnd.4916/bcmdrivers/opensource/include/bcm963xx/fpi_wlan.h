
/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom 
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

