/*
	<:copyright-BRCM:2019:DUAL/GPL:standard
	
	   Copyright (c) 2019 Broadcom 
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

#ifndef _RDP_DRV_SBPM_H_
#define _RDP_DRV_SBPM_H_

#define SBPM_BUF_SIZE 128

bdmf_error_t drv_sbpm_copy_list(uint16_t bn, uint8_t *dest_buffer);
bdmf_error_t drv_sbpm_alloc_list(uint32_t size, uint32_t headroom, uint8_t *data, uint16_t *bn0, uint16_t *bn1, uint8_t *bns_num);
bdmf_error_t drv_sbpm_free_list(uint16_t head_bn);

#endif
