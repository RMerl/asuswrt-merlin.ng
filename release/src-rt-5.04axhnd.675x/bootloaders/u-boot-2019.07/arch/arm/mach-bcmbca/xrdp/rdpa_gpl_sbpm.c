// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Broadcom
 */
/*
	
*/

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#include "rdpa_gpl_sbpm.h"
#include "rdp_common.h"
#include "rdp_subsystem_common.h"
#include "XRDP_AG.h"
#if defined(CONFIG_BCM63146)
#if defined(CONFIG_BCMBCA_XRDP_GPL)
#include "xrdp_drv_psram.h"
#include "xrdp_drv_sbpm.h"
#else
#include "xrdp_drv_psram_ag.h"
#include "xrdp_drv_sbpm_ag.h"
#endif
#else
#include "xrdp_drv_psram_mem_ag.h"
#endif

#define SBPM_MAX_NUM_OF_ITERS 1000

#ifndef MIN
#define MIN(a, b) ((a) >= (b) ? (b) : (a))
#endif

static int drv_sbpm_connect_single(uint16_t bn, uint16_t next_bn)
{
	int rc, num_of_iters;
	bdmf_boolean connect_ack, busy, rdy;

	rc = ag_drv_sbpm_regs_bn_connect_set(bn, 1, 0, next_bn);
	if (rc)
		return rc;
	for (num_of_iters = 0; num_of_iters < SBPM_MAX_NUM_OF_ITERS;
			num_of_iters++) {
		rc = ag_drv_sbpm_regs_bn_connect_rply_get(&connect_ack, &busy,
							  &rdy);
		if (rc)
			return rc;
		if (rdy && connect_ack)
			break;
	}
	if (num_of_iters == SBPM_MAX_NUM_OF_ITERS)
		return BDMF_ERR_INTERNAL;
	return 0;
}

static bdmf_error_t drv_sbpm_copy_buf_to_bn(uint16_t bn, uint32_t headroom,
					    uint8_t *data, uint32_t size)
{
#ifdef CONFIG_BCM_PON_XRDP
	psram_mem_memory_data _data = {};

	memcpy((uint8_t *)(&_data) + headroom, data,
	       MIN(size, SBPM_BUF_SIZE - headroom));
	return ag_drv_psram_mem_memory_data_set(bn, &_data);
#else
	psram_memory_data _data = {};

	memcpy((uint8_t *)(&_data) + headroom, data,
	       MIN(size, SBPM_BUF_SIZE - headroom));
	return ag_drv_psram_memory_data_set(bn, &_data);
#endif
}

static bdmf_error_t drv_sbpm_alloc_single(uint32_t size, uint32_t headroom,
					  uint8_t *data, uint16_t *bn)
{
	int rc, num_of_iters;
	sbpm_regs_bn_alloc_rply reply;

	rc = ag_drv_sbpm_regs_bn_alloc_set(30);
	if (rc)
		goto error;
	for (num_of_iters = 0; num_of_iters < SBPM_MAX_NUM_OF_ITERS;
			num_of_iters++) {
		rc = ag_drv_sbpm_regs_bn_alloc_rply_get(&reply);
		if (rc)
			goto error;
		if (reply.rdy && reply.alloc_bn_valid)
			break;
	}
	if (num_of_iters == SBPM_MAX_NUM_OF_ITERS) {
		printf("%s: alloc single 5 (%d)\n", __FUNCTION__, rc);
		rc = BDMF_ERR_INTERNAL;
		goto error;
	}

	drv_sbpm_copy_buf_to_bn(reply.alloc_bn, headroom, data, size);
	*bn = reply.alloc_bn;
	return 0;

error:
	*bn = SBPM_INVALID_BUFFER_NUMBER;
	return rc;
}

bdmf_error_t drv_sbpm_alloc_list(uint32_t size, uint32_t headroom, uint8_t *data,
				 uint16_t *bn0, uint16_t *bn1, uint8_t *bns_num)
{

	uint16_t head_bn = SBPM_INVALID_BUFFER_NUMBER, new_bn, curr_bn;
	int rc = 0, _size, sbpm_buf_size_wo_headroom;
	uint8_t _bns_num = 1;

	_size = (int)size;
	sbpm_buf_size_wo_headroom = SBPM_BUF_SIZE - headroom;

	/* Allocate first; then in loop, copy and allocate next as long
	 * as needed. */
	rc = drv_sbpm_alloc_single(_size, headroom, data, &head_bn);
	if (rc)
		goto error;
	if (_size <= sbpm_buf_size_wo_headroom) {
		/* We are done, short packet */
		new_bn = head_bn; /* Last BN = head BN */
		goto exit;
	}

	for (_size -= sbpm_buf_size_wo_headroom,
			data += sbpm_buf_size_wo_headroom, curr_bn = head_bn;
	     _size > 0;
	     _size -= SBPM_BUF_SIZE, data += SBPM_BUF_SIZE, curr_bn = new_bn) {
		rc = drv_sbpm_alloc_single(_size, 0, data, &new_bn);
		if (rc)
			goto error;
		rc = drv_sbpm_connect_single(curr_bn, new_bn);
		if (rc) {
			drv_sbpm_free_list(new_bn);
			goto error;
		}
		_bns_num++;
	}

exit:

	if (bn0)
		*bn0 = head_bn;
	if (bn1)
		*bn1 = new_bn;
	if (bns_num)
		*bns_num = _bns_num;
	return 0;

error:
	if (head_bn != SBPM_INVALID_BUFFER_NUMBER)
		drv_sbpm_free_list(head_bn);

	if (bn0)
		*bn0 = SBPM_INVALID_BUFFER_NUMBER;
	if (bn1)
		*bn1 = SBPM_INVALID_BUFFER_NUMBER;
	if (bns_num)
		*bns_num = 0;
	return rc;
}

bdmf_error_t drv_sbpm_free_list(uint16_t head_bn)
{
	int rc, num_of_iters;
	sbpm_regs_bn_free_without_contxt_rply reply;

	rc = ag_drv_sbpm_regs_bn_free_without_contxt_set(head_bn, 18, 1);
	if (rc)
		return rc;
	for (num_of_iters = 0; num_of_iters < SBPM_MAX_NUM_OF_ITERS;
	     num_of_iters++) {
		rc = ag_drv_sbpm_regs_bn_free_without_contxt_rply_get(&reply);
		if (rc)
			return rc;
		if (reply.rdy)
			break;
	}
	if (reply.free_ack == 0) {
		bdmf_trace("coudlnt release bn =%d free_ack=0\n", head_bn);
		return BDMF_ERR_INTERNAL;
	}

	if (num_of_iters == SBPM_MAX_NUM_OF_ITERS) {
		bdmf_trace("coudlnt release bn=%d max_iter\n", head_bn);
		return BDMF_ERR_INTERNAL;
	}

	return 0;
}

uint16_t drv_sbpm_get_next_bn(int16_t bn)
{
	int rc;
	sbpm_regs_get_next_rply next_rply = {};
	uint32_t next_bn = SBPM_INVALID_BUFFER_NUMBER;

	rc = ag_drv_sbpm_regs_get_next_set(bn);
	while (!rc) {
		rc = ag_drv_sbpm_regs_get_next_rply_get(&next_rply);
		if (!next_rply.busy)
			break;
	}
	if (rc)
		return SBPM_INVALID_BUFFER_NUMBER;

	if (next_rply.bn_valid) {
		if (!next_rply.bn_null)
			next_bn = next_rply.next_bn;
	}
	else
		bdmf_trace(" ### BN_NULL (0x%x)\n", next_rply.next_bn);

	if (next_rply.mcnt_val != 0)
		bdmf_trace("bn: %d, mcast value: %d\n", bn, next_rply.mcnt_val);

	return next_bn;
}

/* this funnction copy single sbpm to buffer */
#ifdef CONFIG_BCM_PON_XRDP
void drv_sbpm_copy_single(psram_mem_memory_data *memory_data, int first,
			  uint8_t *dest_buffer, uint32_t *dest_index)
#else
void drv_sbpm_copy_single(psram_memory_data *memory_data, int first,
			  uint8_t *dest_buffer, uint32_t *dest_index)
#endif
{
	int i = 0;
	uint32_t swapped;

	if (first) {
		/* this is a case to skip headroom in first sbpm header */
		i = 5;
		swapped = ((memory_data->memory_data[4] >> 24) & 0xff) |
			  ((memory_data->memory_data[4] >> 8) & 0xff00) |
			  ((memory_data->memory_data[4] << 8) & 0xff0000) |
			  ((memory_data->memory_data[4] << 24) & 0xff000000);
		dest_buffer[*dest_index + 1] = swapped & 0xff;
		dest_buffer[*dest_index] = (swapped >> 8) & 0xff;
		*dest_index +=2;
	}


	for (; i < 32;i++) {
		swapped = ((memory_data->memory_data[i] >> 24) & 0xff) |
			  ((memory_data->memory_data[i] >> 8) & 0xff00) |
			  ((memory_data->memory_data[i] << 8) & 0xff0000) |
			  ((memory_data->memory_data[i] << 24) & 0xff000000);

		dest_buffer[*dest_index + 3] = swapped& 0xff;
		dest_buffer[*dest_index + 2] = (swapped >> 8) & 0xff;
		dest_buffer[*dest_index + 1] = (swapped >> 16) & 0xff;
		dest_buffer[*dest_index + 0] = (swapped >> 24) & 0xff;
		*dest_index +=4;
	}
}

/* this funnction copy list of sbpm to buffer */
bdmf_error_t drv_sbpm_copy_list(uint16_t bn, uint8_t *dest_buffer)
{
	uint16_t next_bn;
	int i;
	uint32_t dest_index = 0;
#ifdef CONFIG_BCM_PON_XRDP
	psram_mem_memory_data memory_data;
#else
	psram_memory_data memory_data;
#endif

#ifdef CONFIG_BCM_PON_XRDP
	ag_drv_psram_mem_memory_data_get(bn, &memory_data);
#else
	ag_drv_psram_memory_data_get(bn, &memory_data);
#endif
	drv_sbpm_copy_single(&memory_data, 1, &dest_buffer[0], &dest_index);


	for (i = 0; i < SBPM_MAX_NUM_OF_BNS; i++) {
		next_bn = drv_sbpm_get_next_bn(bn);
		if (next_bn == SBPM_INVALID_BUFFER_NUMBER)
			break;

		bn = next_bn;
#ifdef CONFIG_BCM_PON_XRDP
		ag_drv_psram_mem_memory_data_get(bn, &memory_data);
#else
		ag_drv_psram_memory_data_get(bn, &memory_data);
#endif
		drv_sbpm_copy_single(&memory_data, 0, &dest_buffer[0],
				     &dest_index);
	}
	if (i == SBPM_MAX_NUM_OF_BNS) {
		bdmf_trace("===== BAD LIST ALLOCATED, STOP SCANNING...., "
			   "original bn is %d\n", bn);
		return BDMF_ERR_INTERNAL;
	}
	return 0;
}

