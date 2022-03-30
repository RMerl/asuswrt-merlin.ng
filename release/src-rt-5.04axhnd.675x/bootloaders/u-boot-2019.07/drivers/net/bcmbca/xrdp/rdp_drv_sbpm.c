// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <asm/io.h>
#include "rdd_data_structures.h"

typedef char bdmf_boolean;

#define SBPM_INIT_OFFSET                    (SBPM_MAX_BUFFER_NUMBER)
#define SBPM_MAX_NUM_OF_BNS                 (SBPM_MAX_BUFFER_NUMBER + 1)

#define MIN(a, b) ((a) >= (b) ? (b) : (a))

#define SBPM_BUF_SIZE 128
#define SBPM_MAX_NUM_OF_ITERS 1000
#define HEADROOM_SIZE 18

#define SBPM_REGS_BN_ALLOC_RPLY_REG_OFFSET 0x00000008
#define SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG_OFFSET 0x00000038
#define SBPM_REGS_BN_CONNECT_RPLY_REG_OFFSET 0x00000020
#define SBPM_REGS_BN_ALLOC_REG_OFFSET 0x00000004
#define SBPM_REGS_BN_CONNECT_REG_OFFSET 0x0000001c
#define SBPM_INVALID_BUFFER_NUMBER 16383

static void _ag_drv_sbpm_regs_get_next_set(uint16_t bn)
{
	writel(bn << 0, SBPM_ADDRS+0x00000024);
}

static void _ag_drv_sbpm_regs_bn_alloc_set(uint8_t sa)
{
	writel(sa << 14, SBPM_ADDRS+SBPM_REGS_BN_ALLOC_REG_OFFSET);
}

static void _ag_drv_sbpm_regs_bn_connect_set(uint16_t bn, bdmf_boolean ack_req,
	bdmf_boolean wr_req, uint16_t pointed_bn)
{
	writel(bn << 0 | ack_req << 14 | wr_req << 15 | pointed_bn << 16,
		SBPM_ADDRS+SBPM_REGS_BN_CONNECT_REG_OFFSET);
}

static void _ag_drv_sbpm_regs_bn_free_without_contxt_set(uint16_t head_bn,
	uint8_t sa, bdmf_boolean ack_req)
{
	writel(head_bn << 0 | sa << 14 | ack_req << 31,
		SBPM_ADDRS+SBPM_REGS_BN_FREE_WITHOUT_CONTXT_REG_OFFSET);
}

static void _ag_drv_sbpm_regs_bn_free_without_contxt_rply_get(
	bdmf_boolean *free_ack, bdmf_boolean *rdy)
{
	uint32_t reg_regs_bn_free_without_contxt_rply =
		readl(SBPM_ADDRS+0x0000003c);

	*free_ack =
		(((reg_regs_bn_free_without_contxt_rply) & 0x00000001) >> 0);
	*rdy = (((reg_regs_bn_free_without_contxt_rply) & 0x80000000) >> 31);
}

static void _ag_drv_sbpm_regs_bn_alloc_rply_get(bdmf_boolean *alloc_bn_valid,
	uint16_t *alloc_bn, bdmf_boolean *rdy)
{
	uint32_t reg_regs_bn_alloc_rply =
		readl(SBPM_ADDRS+SBPM_REGS_BN_ALLOC_RPLY_REG_OFFSET);

	*alloc_bn_valid = (((reg_regs_bn_alloc_rply) & 0x00000001) >> 0);
	*alloc_bn = (((reg_regs_bn_alloc_rply) & 0x00007ffe) >> 1);
	*rdy = (((reg_regs_bn_alloc_rply) & 0x80000000) >> 31);
}

static void _ag_drv_sbpm_regs_bn_connect_rply_get(bdmf_boolean *connect_ack,
	bdmf_boolean *busy, bdmf_boolean *rdy)
{
	uint32_t reg_regs_bn_connect_rply =
		readl(SBPM_ADDRS+SBPM_REGS_BN_CONNECT_RPLY_REG_OFFSET);

	*connect_ack = (((reg_regs_bn_connect_rply) & 0x00000001) >> 0);
	*busy = (((reg_regs_bn_connect_rply) & 0x40000000) >> 30);
	*rdy = (((reg_regs_bn_connect_rply) & 0x80000000) >> 31);
}

struct sbpm_regs_get_next_rply
{
	bdmf_boolean bn_valid;
	uint16_t next_bn;
	bdmf_boolean bn_null;
	uint8_t mcnt_val;
	bdmf_boolean busy;
	bdmf_boolean rdy;
};

static void _ag_drv_sbpm_regs_get_next_rply_get(
	struct sbpm_regs_get_next_rply *regs_get_next_rply)
{
	uint32_t get_next_rply = readl(SBPM_ADDRS+0x00000028);

	regs_get_next_rply->bn_valid = (((get_next_rply) & 0x00000001) >> 0);
	regs_get_next_rply->bn_null = (((get_next_rply) & 0x00008000) >> 15);
	regs_get_next_rply->next_bn = (((get_next_rply) & 0x00007ffe) >> 1);
	regs_get_next_rply->mcnt_val = (((get_next_rply) & 0x00ff0000) >> 16);
	regs_get_next_rply->busy = (((get_next_rply) & 0x40000000) >> 30);
}

int drv_sbpm_free_list(uint16_t head_bn)
{
	int num_of_iters;
	bdmf_boolean free_ack;
	bdmf_boolean rdy;

	_ag_drv_sbpm_regs_bn_free_without_contxt_set(head_bn, HEADROOM_SIZE, 1);
	for (num_of_iters = 0; num_of_iters < SBPM_MAX_NUM_OF_ITERS;
		num_of_iters++) {
		_ag_drv_sbpm_regs_bn_free_without_contxt_rply_get(&free_ack,
			&rdy);
		if (rdy)
			break;
	}

	if (free_ack == 0) {
		printf("Failed to release bn =%d free_ack=0\n", head_bn);
		return EFAULT;
	}

	if (num_of_iters == SBPM_MAX_NUM_OF_ITERS) {
		printf("Failed to release bn=%d max_iter\n", head_bn);
		return EFAULT;
	}

	return 0;
}

static void drv_sbpm_copy_buf_to_bn(uint16_t bn, uint32_t headroom,
	uint8_t *data, uint32_t size)
{
	void *foo = (void *)(unsigned long)(PSRAM_MEM_ADDRS + bn*SBPM_BUF_SIZE);

	memcpy(foo + headroom, data, MIN(size, SBPM_BUF_SIZE - headroom));
}

static int drv_sbpm_alloc_single(uint32_t size, uint32_t headroom,
	uint8_t *data, uint16_t *bn)
{
	int rc, num_of_iters;
	bdmf_boolean alloc_bn_valid;
	uint16_t alloc_bn;
	bdmf_boolean rdy;

	_ag_drv_sbpm_regs_bn_alloc_set(30);
	for (num_of_iters = 0; num_of_iters < SBPM_MAX_NUM_OF_ITERS;
		num_of_iters++) {
		_ag_drv_sbpm_regs_bn_alloc_rply_get(&alloc_bn_valid, &alloc_bn,
			&rdy);
		if (rdy && alloc_bn_valid)
			break;
	}

	if (num_of_iters == SBPM_MAX_NUM_OF_ITERS) {
		printf("%s: alloc single\n", __FUNCTION__);
		rc = -EFAULT;
		goto error;
	}

	drv_sbpm_copy_buf_to_bn(alloc_bn, headroom, data, size);
	*bn = alloc_bn;
	return 0;

error:
	*bn = SBPM_INVALID_BUFFER_NUMBER;
	return rc;
}

static int drv_sbpm_connect_single(uint16_t bn, uint16_t next_bn)
{
	int num_of_iters;
	bdmf_boolean connect_ack, busy, rdy;

	_ag_drv_sbpm_regs_bn_connect_set(bn, 1, 0, next_bn);
	for (num_of_iters = 0; num_of_iters < SBPM_MAX_NUM_OF_ITERS;
			num_of_iters++) {
	    _ag_drv_sbpm_regs_bn_connect_rply_get(&connect_ack, &busy, &rdy);
	    if (rdy && connect_ack)
		    break;
    }

    if (num_of_iters == SBPM_MAX_NUM_OF_ITERS)
	    return -EFAULT;

    return 0;
}

int drv_sbpm_alloc_list(uint32_t size, uint32_t headroom, uint8_t *data,
	uint16_t *bn0, uint16_t *bn1, uint8_t *bns_num)
{
	uint16_t head_bn = SBPM_INVALID_BUFFER_NUMBER, new_bn, curr_bn;
	int rc, _size, sbpm_buf_size_wo_headroom;
	uint8_t _bns_num = 1;

	_size = (int)size;
	sbpm_buf_size_wo_headroom = SBPM_BUF_SIZE - headroom;

	/* Allocate first; then in loop, copy and allocate next as long as
	 * needed. */
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
		_size -= SBPM_BUF_SIZE, data += SBPM_BUF_SIZE,
		curr_bn = new_bn) {
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

uint16_t drv_sbpm_get_next_bn(int16_t bn)
{
	int rc = 0;
	struct sbpm_regs_get_next_rply next_rply = {};
	uint32_t next_bn = SBPM_INVALID_BUFFER_NUMBER;

	_ag_drv_sbpm_regs_get_next_set(bn);
	while (1) {
		_ag_drv_sbpm_regs_get_next_rply_get(&next_rply);
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
		printf(" ### BN_NULL (0x%x)\n", next_rply.next_bn);

	if (next_rply.mcnt_val != 0)
		printf("bn: %d, mcast value: %d\n", bn, next_rply.mcnt_val);

	return next_bn;
}

static void _drv_sbpm_copy_single(int bn, uint8_t **data, int skip)
{
	void *foo = (void *)(unsigned long)(PSRAM_MEM_ADDRS + bn*SBPM_BUF_SIZE);

	memcpy(*data, foo + skip, SBPM_BUF_SIZE - skip);
	*data += SBPM_BUF_SIZE - skip;
}

/* this funnction copy list of sbpm to buffer */
int drv_sbpm_copy_list(uint16_t bn, uint8_t *dest_buffer)
{
	uint16_t next_bn;
	int i;

	_drv_sbpm_copy_single(bn, &dest_buffer, HEADROOM_SIZE);

	for (i = 0; i < SBPM_MAX_NUM_OF_BNS; i++) {
		next_bn = drv_sbpm_get_next_bn(bn);
		if (next_bn == SBPM_INVALID_BUFFER_NUMBER)
			break;

		bn = next_bn;
		_drv_sbpm_copy_single(bn, &dest_buffer, 0);
	}

	if (i == SBPM_MAX_NUM_OF_BNS) {
		printf("===== BAD LIST ALLOCATED, STOP SCANNING...., original "\
			"bn is %d\n", bn);
		return -EFAULT;
	}

	return 0;
}
