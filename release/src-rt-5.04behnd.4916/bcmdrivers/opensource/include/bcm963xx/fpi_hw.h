
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
 * File Name : fpi_hw.h
 *
 *******************************************************************************
 */

#ifndef __FPI_HW_H_INCLUDED__
#define __FPI_HW_H_INCLUDED__
#include <linux/blog.h>
#include "fpi_defs.h"

typedef struct {
	uint32_t registered : 1;
	uint32_t unused: 31;
	int (*set_mode)(fpi_mode_t mode);
	int (*get_mode)(fpi_mode_t *mode_p);
	int (*set_default_priority)(uint8_t prio);
	int (*get_default_priority)(uint8_t *prio_p);

	int (*add_flow)(Blog_t *blog_p, int *handle_p);
	int (*delete_flow)(int handle);	/* must */
	int (*delete_flow_by_blog)(Blog_t *blog_p);

	int (*get_stat)(int handle, uint32_t *pkt_cnt_p, uint64_t *byte_cnt_p);
	int (*add_ap_mac)(uint8_t *mac_addr_p);
	int (*delete_ap_mac)(uint8_t *mac_addr_p);
	int (*set_gre_mode)(fpi_gre_mode_t mode);
	int (*set_l2lkp_on_etype)(bool enable, uint16_t etype);
	int (*set_lkp_enable)(bool enable);
#if 0
	/* TODO! see if we need to support getting flow from HW */
	int (*get_flow(int handle, Blog_t *blog_p);
#endif
} fpi_hw_info_t;


int fpi_register_hw(const fpi_hw_info_t *hw_info);
int fpi_unregister_hw(fpi_hw_info_t *hw_info);

#endif /* __FPI_HW_H_INCLUDED__ */

