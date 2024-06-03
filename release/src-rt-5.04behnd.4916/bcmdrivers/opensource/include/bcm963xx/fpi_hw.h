
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

	int (*add_flow)(Blog_t *blog_p, int *handle_p);
	int (*delete_flow)(int handle);	/* must */
	int (*delete_flow_by_blog)(Blog_t *blog_p);

	int (*get_stat)(int handle, uint32_t *pkt_cnt_p, uint64_t *byte_cnt_p);
	int (*add_ap_mac)(uint8_t *mac_addr_p);
	int (*delete_ap_mac)(uint8_t *mac_addr_p);
#if 0
	/* TODO! see if we need to support getting flow from HW */
	int (*get_flow(int handle, Blog_t *blog_p);,
#endif
} fpi_hw_info_t;


int fpi_register_hw(const fpi_hw_info_t *hw_info);
int fpi_unregister_hw(fpi_hw_info_t *hw_info);

#endif /* __FPI_HW_H_INCLUDED__ */

