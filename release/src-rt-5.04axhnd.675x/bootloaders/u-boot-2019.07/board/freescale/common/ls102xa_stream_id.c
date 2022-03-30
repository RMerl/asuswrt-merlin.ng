// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/ls102xa_stream_id.h>

void ls102xa_config_smmu_stream_id(struct smmu_stream_id *id, uint32_t num)
{
	void *scfg = (void *)CONFIG_SYS_FSL_SCFG_ADDR;
	int i;
	u32 icid;

	for (i = 0; i < num; i++) {
		icid = (id[i].stream_id & 0xff) << 24;
		out_be32((u32 *)(scfg + id[i].offset), icid);
	}
}

void ls1021x_config_caam_stream_id(struct liodn_id_table *tbl, int size)
{
	int i;
	u32 liodn;

	for (i = 0; i < size; i++) {
		if (tbl[i].num_ids == 2)
			liodn = (tbl[i].id[0] << 16) | tbl[i].id[1];
		else
			liodn = tbl[i].id[0];

		out_le32((u32 *)(tbl[i].reg_offset), liodn);
	}
}
