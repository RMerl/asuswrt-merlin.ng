/*
 * FCBS utils impl (Common APIs)
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: fcbsutils.c 688113 2017-03-03 12:32:45Z $
 */

#ifdef FCBS_ROM_BUILD
#include <stdio.h>
#include <stdlib.h>
#else
#include <typedefs.h>
#include <osl.h>
#include <d11.h>
#include <siutils.h>
#include <bcmutils.h>
#endif /* This is not required inside DRV build environment */

#include <fcbsutils.h>

#ifdef FCBS_ROM_BUILD
#define PRINT_TUPLES 0
#else
#define PRINT_TUPLES 1
#endif // endif

#ifdef FCBS_ROM_BUILD
#define fcbs_malloc(osh, sz)		malloc(sz)
#define fcbs_mfree(osh, ptr, sz)	free(ptr)
#else
#define fcbs_malloc(osh, sz)		MALLOCZ((osl_t *)osh, sz)
#define fcbs_mfree(osh, ptr, sz)	MFREE((osl_t *)osh, ptr, sz)
#endif // endif

#define FCBS_ALLOC_ERR(ptr) \
	printf("%s: failed to allocate memory for " #ptr "\n", __FUNCTION__)

/* FCBS tuples for setting PSM and FCBS clock request to MAC ILP */
#define FCBS_DLY_TPL_PRE_HDR	0x0015
#define FCBS_DLY_TPL_PRE_CMD1	0xE800
#define FCBS_DLY_TPL_PRE_CMD2	0x0AE5
#define FCBS_DLY_TPL_PRE_DAT1	0x0347
#define FCBS_DLY_TPL_PRE_DAT2	0x0000

/* FCBS tuples for setting PSM and FCBS clock request to MAC HT */
#define FCBS_DLY_TPL_POST_HDR	0x0015
#define FCBS_DLY_TPL_POST_CMD1	0xE800
#define FCBS_DLY_TPL_POST_CMD2	0x0AE5
#define FCBS_DLY_TPL_POST_DAT1	0x03CF
#define FCBS_DLY_TPL_POST_DAT2	0x0000

static uint16
fcbs_header(int type, int len, int cdsize)
{
	return (((type & FCBS_TYPE_MASK) << FCBS_TYPE_SHIFT) |
		(((len - 1) & FCBS_LEN_MASK) << FCBS_LEN_SHIFT) |
		((cdsize & FCBS_CDSIZE_MASK) << FCBS_CDSIZE_SHIFT))
		& FCBS_TUPLE_MASK;
}

#if defined(PRINT_TUPLES) && PRINT_TUPLES
void
fcbs_print_tuples(fcbs_tuples_t *ft)
{
	uint i;

	if (!ft) {
		FCBS_DBG(("%s: ft is NULL\n", __FUNCTION__));
		goto ret;
	}

	FCBS_INFO(("------ FCBS tuples ------\n"));
	FCBS_INFO(("cmd_size:%d, dat_size:%d\n", ft->cmd_size, ft->dat_size));

	FCBS_INFO(("cmd_ptr:\n"));
	for (i = 0; i < ft->cmd_size; i++) {
		FCBS_INFO(("%02d: 0x%04x\n",
			i, ft->cmd_ptr[i] & FCBS_TUPLE_MASK));
	}

	FCBS_INFO(("dat_ptr:\n"));
	for (i = 0; i < ft->dat_size; i++) {
		FCBS_INFO(("%02d: 0x%04x\n",
			i, ft->dat_ptr[i] & FCBS_TUPLE_MASK));
	}
ret:
	return;
}
#else
void
fcbs_print_tuples(fcbs_tuples_t *ft) { }
#endif /* defined(PRINT_TUPLES) && PRINT_TUPLES */

static uint16
fcbs_delay_header(int delay_us)
{
	uint16 ft_delay = 0;

	if (delay_us >= FCBS_DELAY_MAX_US)
		delay_us = FCBS_DELAY_MAX_US - 1;

	ft_delay = (FCBS_DELAY) | ((delay_us << 3) << FCBS_DELAY_VAL_SHIFT);

	return FCBS_TPL(ft_delay);
}

static uint32
fcbs_delay_tpl_count(d11axiiv_t *iv)
{
	uint32 inx, count = 0;

	for (inx = 0; iv[inx].addr != INITVALS_END_MARKER; inx++) {
		if (iv[inx].addr == FCBS_DELAY_TPL) {
			count++;
		}
	}

	return count;
}

static void
fcbs_delay_tpl_populate(fcbs_tuples_t *ft, int *cmd_size, int *dat_size, int delay)
{
	/* FCBS tuples for setting PSM and FCBS clock request to MAC ILP */
	if (delay) {
		ft->cmd_ptr[(*cmd_size)++] = FCBS_DLY_TPL_PRE_HDR;
		ft->cmd_ptr[(*cmd_size)++] = FCBS_DLY_TPL_PRE_CMD1;
		ft->cmd_ptr[(*cmd_size)++] = FCBS_DLY_TPL_PRE_CMD2;

		ft->dat_ptr[(*dat_size)++] = FCBS_DLY_TPL_PRE_DAT1;
		ft->dat_ptr[(*dat_size)++] = FCBS_DLY_TPL_PRE_DAT2;
	}

	ft->cmd_ptr[(*cmd_size)++] = fcbs_delay_header(delay);

	/* FCBS tuples for setting PSM and FCBS clock request to MAC HT */
	if (delay) {
		ft->cmd_ptr[(*cmd_size)++] = FCBS_DLY_TPL_POST_HDR;
		ft->cmd_ptr[(*cmd_size)++] = FCBS_DLY_TPL_POST_CMD1;
		ft->cmd_ptr[(*cmd_size)++] = FCBS_DLY_TPL_POST_CMD2;

		ft->dat_ptr[(*dat_size)++] = FCBS_DLY_TPL_POST_DAT1;
		ft->dat_ptr[(*dat_size)++] = FCBS_DLY_TPL_POST_DAT2;
	}
}

static uint32
phy_tbl_tpl_width(uint32 width)
{
	uint32 tpl_width = 0;

	switch (width) {
	case 8:
		tpl_width = 0;
		break;
	case 16:
		tpl_width = 1;
		break;
	case 32:
		tpl_width = 2;
		break;
	case 48:
		tpl_width = 3;
		break;
	case 60:
	case 64:
		tpl_width = 4;
		break;
	}

	return tpl_width;
}

/* For PHY tables */
static fcbs_tuples_t *
fcbs_phy_tbl(void *osh, fcbs_input_data_t *input)
{
	int i, j, cmd_size = 0, dat_size = 0, data_bytes;
	uint8 *data;
	fcbs_input_phy_tbl_t *phy_tbl = (fcbs_input_phy_tbl_t *)(input->data);
	fcbs_tuples_t *ft;

	ft = (fcbs_tuples_t *) fcbs_malloc(osh, sizeof(*ft));
	if (!ft) {
		FCBS_ALLOC_ERR(ft);
		goto exit_ft;
	}

	/* Calculating the tuple buffer size
	 *	cmd: hdr: 1, ((tbl_id: 1, tbl_offset: 1, tbl_len: 1) * number of tables)
	 *	dat: (table_len * table_width / 8) of all the tables
	 */
	ft->cmd_buf_size =  (1 + input->data_size * 3) * FCBS_TUPLE_NUM_BYTES;
	ft->dat_buf_size = 0;
	for (i = 0; i < input->data_size; i++) {
		ft->dat_buf_size +=
			ROUNDUP((phy_tbl[i].len * (phy_tbl[i].width / 8)),
				FCBS_TUPLE_NUM_BYTES);
	}

	ft->cmd_ptr = (uint16 *) fcbs_malloc(osh, ft->cmd_buf_size);
	if (!ft->cmd_ptr) {
		FCBS_ALLOC_ERR(ft->cmd_ptr);
		goto exit_cmd_ptr;
	}

	ft->dat_ptr = (uint16 *) fcbs_malloc(osh, ft->dat_buf_size);
	if (!ft->dat_ptr) {
		FCBS_ALLOC_ERR(ft->dat_ptr);
		goto exit_dat_ptr;
	}

	/* Populating the header */
	ft->cmd_ptr[cmd_size++] = fcbs_header(input->type, input->data_size, 0);

	/* Populating cmd and data */
	for (i = 0; i < input->data_size; i++)
	{
		ft->cmd_ptr[cmd_size++] =
		(((phy_tbl[i].id & FCBS_PHY_TBL_ID_MASK) << FCBS_PHY_TBL_ID_SHIFT) |
		((phy_tbl_tpl_width(phy_tbl[i].width) & FCBS_PHY_TBL_WIDTH_MASK)
		 << FCBS_PHY_TBL_WIDTH_SHIFT)) & FCBS_TUPLE_MASK;
		ft->cmd_ptr[cmd_size++] = FCBS_TPL(phy_tbl[i].offset);
		ft->cmd_ptr[cmd_size++] = FCBS_TPL(phy_tbl[i].len - 1);

		data = (uint8 *) phy_tbl[i].data;
		data_bytes = phy_tbl[i].len * phy_tbl[i].width / 8;

		if (phy_tbl[i].width == 32) {
			for (j = 0; j < data_bytes/4; j++) {
				ft->dat_ptr[dat_size++] =
					FCBS_TPL((*((uint32 *) data)) >> 16);
				ft->dat_ptr[dat_size++] =
					FCBS_TPL(*((uint32 *) data));
				data += 4;
			}
		}
		else {
			for (j = 0; j < data_bytes/2; j++) {
				ft->dat_ptr[dat_size] = *(data++) & 0xFF;
				ft->dat_ptr[dat_size] |=
					((*(data++) & 0xFF) << 8);
				ft->dat_ptr[dat_size] &= FCBS_TUPLE_MASK;
				dat_size++;
			}

			if (ROUNDUP(data_bytes, 2) != data_bytes) {
				ft->dat_ptr[dat_size] = *(data) & 0xFF;
				ft->dat_ptr[dat_size] &= FCBS_TUPLE_MASK;
				dat_size++;
			}
		}
	}

	ft->cmd_size = cmd_size;
	ft->dat_size = dat_size;
	goto exit_ft;

exit_dat_ptr:
	fcbs_mfree(osh, ft->cmd_ptr, ft->cmd_buf_size);
exit_cmd_ptr:
	fcbs_mfree(osh, ft, sizeof(*ft));
	ft = NULL;
exit_ft:
	return ft;
}

/* For BP regs */
static fcbs_tuples_t *
fcbs_bp_reg_nocompress(void *osh, fcbs_input_data_t *input)
{
	int i, cmd_size = 0, dat_size = 0;
	fcbs_input_bp_t *bp_data = (fcbs_input_bp_t *)(input->data);
	fcbs_ad_t *data = (fcbs_ad_t *)bp_data->data;
	int data_size = input->data_size;
	fcbs_tuples_t *ft;

	ft = (fcbs_tuples_t *) fcbs_malloc(osh, sizeof(*ft));
	if (!ft) {
		FCBS_ALLOC_ERR(ft);
		goto exit_ft;
	}

	/* Calculating the tuple buffer size
	 *	cmd: hdr: 1, addr_hi: 1, ((addr_lo :1) * data_size)
	 *	dat: (data_lo: 1 + data_hi: 1) * data_size
	 */
	ft->cmd_buf_size = (2 + data_size) * FCBS_TUPLE_NUM_BYTES;
	ft->dat_buf_size = (data_size * FCBS_TUPLE_NUM_BYTES * 2);

	ft->cmd_ptr = (uint16 *) fcbs_malloc(osh, ft->cmd_buf_size);
	if (!ft->cmd_ptr) {
		FCBS_ALLOC_ERR(ft->cmd_ptr);
		goto exit_cmd_ptr;
	}

	ft->dat_ptr = (uint16 *) fcbs_malloc(osh, ft->dat_buf_size);
	if (!ft->dat_ptr) {
		FCBS_ALLOC_ERR(ft->dat_ptr);
		goto exit_dat_ptr;
	}

	/* FCBS header: len = 1 (for addr_hi) + data_size (for addr_lo) */
	ft->cmd_ptr[cmd_size++] =
		fcbs_header(input->type, 1 + data_size, 0) & FCBS_TUPLE_MASK;
	/* Populating addr_hi */
	ft->cmd_ptr[cmd_size++] = bp_data->addr_hi & FCBS_TUPLE_MASK;

	for (i = 0; i < data_size; i++)
	{
		ft->cmd_ptr[cmd_size++] = data[i].addr & FCBS_TUPLE_MASK;
		ft->dat_ptr[dat_size++] = data[i].data & FCBS_TUPLE_MASK;
		ft->dat_ptr[dat_size++] = (data[i].data >> FCBS_TUPLE_NUM_BITS)
							& FCBS_TUPLE_MASK;
	}

	ft->cmd_size = cmd_size;
	ft->dat_size = dat_size;
	goto exit_ft;

exit_dat_ptr:
	fcbs_mfree(osh, ft->cmd_ptr, ft->cmd_buf_size);
exit_cmd_ptr:
	fcbs_mfree(osh, ft, sizeof(*ft));
	ft = NULL;
exit_ft:
	return ft;
}

/* For initvals */
static fcbs_tuples_t *
fcbs_initvals_nocompress(void *osh, fcbs_input_data_t *input)
{
	int i, cmd_size = 0, dat_size = 0;
	d11axiiv_t *iv = (d11axiiv_t *)(input->data);
	int data_size = 0, tpl_len = 1, hdr_inx;
	uint16 addr_hi_old, addr_hi_new;
	fcbs_tuples_t *ft;

	ft = (fcbs_tuples_t *) fcbs_malloc(osh, sizeof(*ft));
	if (!ft) {
		FCBS_ALLOC_ERR(ft);
		goto exit_ft;
	}

	while (iv[data_size++].addr != INITVALS_END_MARKER);

	if (data_size <= 1)
		return ft;

	/* Calculating the tuple buffer size
	 *	cmd: (hdr: 1, addr_hi: 1, addr_lo :1 + data_size) + (7 * delay_tpl_cnt)
	 *	dat: (data_lo: 1 + data_hi: 1 + data_size) + (4 * delay_tpl_cnt)
	 */
	ft->cmd_buf_size = ((data_size + 2 * fcbs_delay_tpl_count(iv)) * FCBS_TUPLE_NUM_BYTES * 3);
	ft->dat_buf_size = ((data_size + 2 * fcbs_delay_tpl_count(iv)) * FCBS_TUPLE_NUM_BYTES * 2);

	ft->cmd_ptr = (uint16 *) fcbs_malloc(osh, ft->cmd_buf_size);
	if (!ft->cmd_ptr) {
		FCBS_ALLOC_ERR(ft->cmd_ptr);
		goto exit_cmd_ptr;
	}

	ft->dat_ptr = (uint16 *) fcbs_malloc(osh, ft->dat_buf_size);
	if (!ft->dat_ptr) {
		FCBS_ALLOC_ERR(ft->dat_ptr);
		goto exit_dat_ptr;
	}

	/* Header place holder */
	hdr_inx = cmd_size++;

	ft->cmd_ptr[cmd_size++] = addr_hi_old = FCBS_TPL(ADDR_HI(iv[0].addr));

	for (i = 0; i < data_size; i++)
	{
		addr_hi_new = ADDR_HI(iv[i].addr);

		if (addr_hi_new == addr_hi_old) {
			tpl_len++;
			if (iv[i].size == 2) {
				if ((ADDR_LO(iv[i].addr) & 0x3) == 0x2) {
					ft->cmd_ptr[cmd_size++] =
						FCBS_TPL(ADDR_LO(iv[i].addr));
					ft->dat_ptr[dat_size++] = 0x0;
					ft->dat_ptr[dat_size++] =
						FCBS_TPL(DATA_LO(iv[i].value));
				}
				else if ((ADDR_LO(iv[i].addr) & 0x3) == 0x0) {
					ft->cmd_ptr[cmd_size++] =
					FCBS_TPL((ADDR_LO(iv[i].addr) | 0x1));
					ft->dat_ptr[dat_size++] =
						FCBS_TPL(DATA_LO(iv[i].value));
					ft->dat_ptr[dat_size++] = 0x0;
				} else {
					FCBS_ERR(("Initvals input data error!\n[%d]: 0x%08x  %d  "
						"0x%08x", i, iv[i].addr, iv[i].size, iv[i].value));
				};
			} else {
				ft->cmd_ptr[cmd_size++] =
					FCBS_TPL(ADDR_LO(iv[i].addr));
				ft->dat_ptr[dat_size++] =
					FCBS_TPL(DATA_LO(iv[i].value));
				ft->dat_ptr[dat_size++] =
					FCBS_TPL(DATA_HI(iv[i].value));
			}
		}

		if ((addr_hi_new != addr_hi_old) || tpl_len == FCBS_MAX_LEN) {
			/* Filling the old header */
			ft->cmd_ptr[hdr_inx] =
				fcbs_header(input->type, tpl_len, 0);

			if (iv[i].addr == INITVALS_END_MARKER)
				break;

			if (iv[i].addr == FCBS_DELAY_TPL) {
				fcbs_delay_tpl_populate(ft, &cmd_size, &dat_size, iv[i].value);
				i++;
				addr_hi_new = ADDR_HI(iv[i].addr);
			}

			addr_hi_old = addr_hi_new;

			/* Place holder for next header */
			hdr_inx = cmd_size++;

			if (tpl_len == FCBS_MAX_LEN) {
				addr_hi_new = ADDR_HI(iv[i + 1].addr);
			}
			ft->cmd_ptr[cmd_size++] = FCBS_TPL(addr_hi_new);

			if (tpl_len != FCBS_MAX_LEN) {
				i--;
			}

			tpl_len = 1;
		}
	}

	ft->cmd_size = cmd_size;
	ft->dat_size = dat_size;
	goto exit_ft;

exit_dat_ptr:
	fcbs_mfree(osh, ft->cmd_ptr, ft->cmd_buf_size);
exit_cmd_ptr:
	fcbs_mfree(osh, ft, sizeof(*ft));
	ft = NULL;
exit_ft:
	return ft;
}

/* Generates delay tuple */
static fcbs_tuples_t *
fcbs_delay_tuple_ulp(void *osh, fcbs_input_data_t *input)
{
	int cmd_size = 0, dat_size = 0;
	uint16 *data = (uint16 *)(input->data);
	int data_size = input->data_size; /* 1 for FCBS_DELAY */
	fcbs_tuples_t *ft;

	ft = (fcbs_tuples_t *) fcbs_malloc(osh, sizeof(*ft));
	if (!ft) {
		FCBS_ALLOC_ERR(ft);
		goto exit_ft;
	}

	/* Calculating the tuple buffer size
	 *	cmd: (hdr: 1, addr_hi: 1, addr_lo :1 delat_tpl :1 hdr: 1, addr_hi: 1, addr_lo :1)
	 *	dat: (data_lo: 1 + data_hi: 1 data_lo: 1 data_hi: 1)
	 */
	ft->cmd_buf_size = ((data_size + 6) * FCBS_TUPLE_NUM_BYTES);
	ft->dat_buf_size = (4 * FCBS_TUPLE_NUM_BYTES);

	ft->cmd_ptr = (uint16 *) fcbs_malloc(osh, ft->cmd_buf_size);
	if (!ft->cmd_ptr) {
		FCBS_ALLOC_ERR(ft->cmd_ptr);
		goto exit_cmd_ptr;
	}

	ft->dat_ptr = (uint16 *) fcbs_malloc(osh, ft->dat_buf_size);
	if (!ft->dat_ptr) {
		FCBS_ALLOC_ERR(ft->dat_ptr);
		goto exit_dat_ptr;
	}

	/* Populating delay header */
	fcbs_delay_tpl_populate(ft, &cmd_size, &dat_size, *data);
	ft->cmd_size = cmd_size;
	ft->dat_size = dat_size;
	goto exit_ft;

exit_dat_ptr:
	fcbs_mfree(osh, ft->cmd_ptr, ft->cmd_buf_size);
exit_cmd_ptr:
	fcbs_mfree(osh, ft, sizeof(*ft));
exit_ft:
	return ft;
}

static fcbs_tuples_t *
fcbs_delay_tuple(void *osh, fcbs_input_data_t *input)
{
	int cmd_size = 0, dat_size = 0;
	uint16 *data = (uint16 *)(input->data);
	int data_size = input->data_size; /* 1 for FCBS_DELAY */
	fcbs_tuples_t *ft;

	ft = (fcbs_tuples_t *) fcbs_malloc(osh, sizeof(*ft));
	if (!ft) {
		FCBS_ALLOC_ERR(ft);
		goto exit_ft;
	}

	/* Calculating the tuple buffer size
	 *	cmd: (delay_tpl: 1)
	 */
	ft->cmd_buf_size = (data_size * FCBS_TUPLE_NUM_BYTES);
	ft->dat_buf_size = 0;

	ft->cmd_ptr = (uint16 *) fcbs_malloc(osh, ft->cmd_buf_size);
	if (!ft->cmd_ptr) {
		FCBS_ALLOC_ERR(ft->cmd_ptr);
		goto exit_cmd_ptr;
	}

	ft->dat_ptr = NULL;

	ft->cmd_ptr[cmd_size++] = fcbs_delay_header(*data);
	ft->cmd_size = cmd_size;
	ft->dat_size = dat_size;
	goto exit_ft;

exit_cmd_ptr:
	fcbs_mfree(osh, ft, sizeof(*ft));
exit_ft:
	return ft;
}

/* Copies raw data */
static fcbs_tuples_t *
fcbs_raw_data(void *osh, fcbs_input_data_t *input)
{
	fcbs_tuples_t *ft;

	ft = (fcbs_tuples_t *) fcbs_malloc(osh, sizeof(*ft));
	if (!ft) {
		FCBS_ALLOC_ERR(ft);
		goto exit_ft;
	}

	ft->cmd_size = 0;
	ft->cmd_buf_size = 0;
	ft->cmd_ptr = NULL;

	ft->dat_size = input->data_size;
	ft->dat_buf_size = ft->dat_size * FCBS_TUPLE_NUM_BYTES;
	ft->dat_ptr = (uint16 *) fcbs_malloc(osh, ft->dat_buf_size);
	if (!ft->dat_ptr) {
		FCBS_ALLOC_ERR(ft->dat_ptr);
		goto exit_dat_ptr;
	}

	memcpy(ft->dat_ptr, input->data, input->data_size * FCBS_TUPLE_NUM_BYTES);

	goto exit_ft;

exit_dat_ptr:
	fcbs_mfree(osh, ft, sizeof(*ft));
exit_ft:
	return ft;
}

/* Same function for PHY and Radio regs */
static fcbs_tuples_t *
fcbs_phy_rad_reg_nocompress(void *osh, fcbs_input_data_t *input)
{
	int i, j = 0, cmd_size = 0, dat_size = 0, len;
	void *data = (void *)(input->data);
	int data_size = input->data_size;
	fcbs_tuples_t *ft;

	ft = (fcbs_tuples_t *) fcbs_malloc(osh, sizeof(*ft));
	if (!ft) {
		FCBS_ALLOC_ERR(ft);
		goto exit_ft;
	}

	/* Calculating the tuple buffer size
	 *	cmd: (one header for each FCBS_MAX_LEN block) + ((addr_lo: 1) * data_size)
	 *	dat: (data_lo: 1) * data_size
	 */
	ft->cmd_buf_size =
		(((data_size + FCBS_MAX_LEN - 1)/ FCBS_MAX_LEN) + data_size)
		* FCBS_TUPLE_NUM_BYTES;
	ft->dat_buf_size = (data_size * FCBS_TUPLE_NUM_BYTES);

	ft->cmd_ptr = (uint16 *) fcbs_malloc(osh, ft->cmd_buf_size);
	if (!ft->cmd_ptr) {
		FCBS_ALLOC_ERR(ft->cmd_ptr);
		goto exit_cmd_ptr;
	}

	ft->dat_ptr = (uint16 *) fcbs_malloc(osh, ft->dat_buf_size);
	if (!ft->dat_ptr) {
		FCBS_ALLOC_ERR(ft->dat_ptr);
		goto exit_dat_ptr;
	}

	do {
		/* Using each header we could accomodate maximum
		 * FCBS_MAX_LEN (= 128) cmds.
		 */
		len = (data_size >= FCBS_MAX_LEN) ? FCBS_MAX_LEN : data_size;

		/* Populating header */
		ft->cmd_ptr[cmd_size++] = fcbs_header(input->type, len, 0);
		for (i = 0; i < len; i++, j++)
		{
			if (input->flags & FCBS_PHY_RADIO_DYNAMIC) {
				ft->cmd_ptr[cmd_size++] =
					FCBS_TPL(((fcbs_input_phy_rad_dyn_ad_t *)data + j)->addr);
				ft->dat_ptr[dat_size++] =
					FCBS_TPL(((fcbs_input_phy_rad_dyn_ad_t *)data + j)->data);
			} else {
				ft->cmd_ptr[cmd_size++] =
					FCBS_TPL(((fcbs_input_ad_t *)data + j)->addr);
				ft->dat_ptr[dat_size++] =
					FCBS_TPL(((fcbs_input_ad_t *)data + j)->data);
			}
		}

		data_size -= len;
	} while (data_size > 0);

	ft->cmd_size = cmd_size;
	ft->dat_size = dat_size;
	goto exit_ft;

exit_dat_ptr:
	fcbs_mfree(osh, ft->cmd_ptr, ft->cmd_buf_size);
exit_cmd_ptr:
	fcbs_mfree(osh, ft, sizeof(*ft));
	ft = NULL;
exit_ft:
	return ft;
}

/*
 * After calling this function, the caller should free the fcbs tuples,
 * using fcbs_free_tuples().
 */
fcbs_tuples_t *
fcbs_create_tuples(void *osh, fcbs_input_data_t *data)
{
	fcbs_tuples_t *ret = NULL;

	if (!data) {
		FCBS_ERR(("%s: data is NULL\n", __FUNCTION__));
		goto ret;
	}

	if (!data->data) {
		FCBS_ERR(("%s: data->data is NULL\n", __FUNCTION__));
		goto ret;
	}

	FCBS_INFO(("%s: type:%d, data_size:%d, flags:%d\n", __FUNCTION__,
			data->type, data->data_size, data->flags));

#ifndef FCBS_ROM_BUILD
	if (!osh) {
		FCBS_ERR(("%s: osh is NULL\n", __FUNCTION__));
		goto ret;
	}
#endif // endif

	switch (data->type) {
		case FCBS_DELAY:
			FCBS_INFO(("Delay tuple\n"));
			ret = fcbs_delay_tuple(osh, data);
			break;

		case FCBS_DELAY_ULP:
			FCBS_INFO(("Delay ULP tuple\n"));
			ret = fcbs_delay_tuple_ulp(osh, data);
			break;

		case FCBS_RADIO_REG:
			FCBS_INFO(("Radio Reg\n"));
			if (data->flags & FCBS_DATA_COMPRESS) {
				FCBS_ERR(("Radio Reg compression unsupported"
						"Fallback to no compress.\n"));
				ret = fcbs_phy_rad_reg_nocompress(osh, data);
			} else {
				ret = fcbs_phy_rad_reg_nocompress(osh, data);
			}
			break;

		case FCBS_PHY_REG:
			FCBS_INFO(("PHY Reg\n"));
			if (data->flags & FCBS_DATA_COMPRESS) {
				FCBS_ERR(("PHY Reg compression unsupported"
						"Fallback to no compress.\n"));
				ret = fcbs_phy_rad_reg_nocompress(osh, data);
			} else {
				ret = fcbs_phy_rad_reg_nocompress(osh, data);
			}
			break;

		case FCBS_PHY_TBL:
			FCBS_INFO(("Phy Table\n"));
			ret = fcbs_phy_tbl(osh, data);
			break;

		case FCBS_BP_ACCESS:
			FCBS_INFO(("BP Access: "));
			if (data->flags & FCBS_DATA_INITVALS) {
				FCBS_INFO(("Initvals\n"));
				ret = fcbs_initvals_nocompress(osh, data);
			} else if (data->flags & FCBS_DATA_COMPRESS) {
				FCBS_ERR(("BP Reg compression unsupported"
						"Fallback to no compress.\n"));
				ret = fcbs_bp_reg_nocompress(osh, data);
			} else {
				FCBS_INFO(("Normal\n"));
				ret = fcbs_bp_reg_nocompress(osh, data);
			}
			break;

		case FCBS_RAW_DATA:
			FCBS_INFO(("Raw data: \n"));
			ret = fcbs_raw_data(osh, data);

		default:
			FCBS_DBG(("Error: Unknow FCBS type!\n"));
			break;
	}

ret:
	if (ret == NULL) {
		FCBS_ERR(("%s: Pointer is NULL invalid data/pointer/malloc failed \n",
			__FUNCTION__));
#ifndef FCBS_ROM_BUILD
		/* FCBS_ROM_BUILD case will include only basic libraries and doesnot have assert
		*support
		*/
		ROMMABLE_ASSERT(0);
#endif // endif
	}
	return ret;
}

void
fcbs_free_tuples(void *osh, fcbs_tuples_t *ft)
{
	if (!osh || !ft)
		goto ret;

	if (ft->cmd_buf_size)
		fcbs_mfree(osh, ft->cmd_ptr, ft->cmd_buf_size);

	if (ft->dat_buf_size)
		fcbs_mfree(osh, ft->dat_ptr, ft->dat_buf_size);

	fcbs_mfree(osh, ft, sizeof(*ft));
ret:
	return;
}

void
fcbs_populate_input_data(fcbs_input_data_t *fid, uint8 inx, uint8 type,
	uint16 data_size, uint16 flags, void *data)
{
	if (fid != NULL) {
		fid[inx].type = type;
		fid[inx].data_size = data_size;
		fid[inx].flags = flags;
		fid[inx].data = data;
	}
}

void
fcbs_create_addr_data_pairs(fcbs_input_bp_t *p, int i, int addr, int data)
{
	if (p != NULL) {
		p->data[i].addr = addr;
		p->data[i].data = data;
	}
}

fcbs_tuples_t *
fcbs_populate_tuples(void *osh, void *cmd, void *dat, int cmd_sz, int dat_sz)
{
	fcbs_tuples_t *ft;

	ft = (fcbs_tuples_t *) fcbs_malloc(osh, sizeof(*ft));
	if (!ft) {
		FCBS_DBG(("%s: failed to allocate memory for ft\n", __FUNCTION__));
		goto ret;
	}

	ft->cmd_ptr = (uint16 *) cmd;
	ft->dat_ptr = (uint16 *) dat;
	ft->cmd_size = cmd_sz;
	ft->dat_size = dat_sz;
	ft->cmd_buf_size = 0;
	ft->dat_buf_size = 0;
ret:
	return ft;
}
