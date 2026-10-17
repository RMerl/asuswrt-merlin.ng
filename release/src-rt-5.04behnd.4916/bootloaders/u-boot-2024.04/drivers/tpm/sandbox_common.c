// SPDX-License-Identifier: GPL-2.0+
/*
 * Common features for sandbox TPM1 and TPM2 implementations
 *
 * Copyright 2021 Google LLC
 */

#define LOG_CATEGORY	UCLASS_TPM

#include <common.h>
#include <tpm-v1.h>
#include <tpm-v2.h>
#include <asm/unaligned.h>
#include "sandbox_common.h"

#define TPM_ERR_CODE_OFS	(2 + 4)		/* after tag and size */

int sb_tpm_index_to_seq(u32 index)
{
	index &= ~HR_NV_INDEX;
	switch (index) {
	case FIRMWARE_NV_INDEX:
		return NV_SEQ_FIRMWARE;
	case KERNEL_NV_INDEX:
		return NV_SEQ_KERNEL;
	case BACKUP_NV_INDEX:
		return NV_SEQ_BACKUP;
	case FWMP_NV_INDEX:
		return NV_SEQ_FWMP;
	case MRC_REC_HASH_NV_INDEX:
		return NV_SEQ_REC_HASH;
	case 0:
		return NV_SEQ_GLOBAL_LOCK;
	case TPM_NV_INDEX_LOCK:
		return NV_SEQ_ENABLE_LOCKING;
	}

	printf("Invalid nv index %#x\n", index);
	return -1;
}

void sb_tpm_read_data(const struct nvdata_state nvdata[NV_SEQ_COUNT],
		      enum sandbox_nv_space seq, u8 *buf, int data_ofs,
		      int length)
{
	const struct nvdata_state *nvd = &nvdata[seq];

	if (!nvd->present)
		put_unaligned_be32(TPM_BADINDEX, buf + TPM_ERR_CODE_OFS);
	else if (length > nvd->length)
		put_unaligned_be32(TPM_BAD_DATASIZE, buf + TPM_ERR_CODE_OFS);
	else
		memcpy(buf + data_ofs, &nvd->data, length);
}

void sb_tpm_write_data(struct nvdata_state nvdata[NV_SEQ_COUNT],
		       enum sandbox_nv_space seq, const u8 *buf, int data_ofs,
		       int length)
{
	struct nvdata_state *nvd = &nvdata[seq];

	if (length > nvd->length)
		log_err("Invalid length %x (max %x)\n", length, nvd->length);
	else
		memcpy(&nvdata[seq].data, buf + data_ofs, length);
}

void sb_tpm_define_data(struct nvdata_state nvdata[NV_SEQ_COUNT],
			enum sandbox_nv_space seq, int length)
{
	struct nvdata_state *nvd = &nvdata[seq];

	if (length > NV_DATA_SIZE)
		log_err("Invalid length %x (max %x)\n", length, NV_DATA_SIZE);
	nvd->length = length;
	nvd->present = true;
}
