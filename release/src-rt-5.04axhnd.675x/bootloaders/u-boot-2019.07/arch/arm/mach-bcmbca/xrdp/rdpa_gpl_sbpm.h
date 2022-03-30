// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Broadcom
 */
/*
	
*/

#ifndef _RDP_DRV_SBPM_H_
#define _RDP_DRV_SBPM_H_

#include <linux/types.h>
#include <bdmf_errno.h>

#define SBPM_BUF_SIZE 128

bdmf_error_t drv_sbpm_copy_list(uint16_t bn, uint8_t *dest_buffer);
bdmf_error_t drv_sbpm_alloc_list(uint32_t size, uint32_t headroom,
				 uint8_t *data, uint16_t *bn0, uint16_t *bn1,
				 uint8_t *bns_num);
bdmf_error_t drv_sbpm_free_list(uint16_t head_bn);

#endif
