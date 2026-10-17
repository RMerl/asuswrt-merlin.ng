// SPDX-License-Identifier: GPL-2.0+
/*
 * Emulation of enough SCSI commands to find and read from a unit
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 *
 * implementation of SCSI functions required so that CONFIG_SCSI can be enabled
 * for sandbox.
 */

#define LOG_CATEGORY UCLASS_SCSI

#include <common.h>
#include <dm.h>
#include <log.h>
#include <scsi.h>
#include <scsi_emul.h>

int sb_scsi_emul_command(struct scsi_emul_info *info,
			 const struct scsi_cmd *req, int len)
{
	int ret = 0;

	info->buff_used = 0;
	log_debug("emul %x\n", *req->cmd);
	switch (*req->cmd) {
	case SCSI_INQUIRY: {
		struct scsi_inquiry_resp *resp = (void *)info->buff;

		info->alloc_len = req->cmd[4];
		memset(resp, '\0', sizeof(*resp));
		resp->data_format = 1;
		resp->additional_len = 0x1f;
		strncpy(resp->vendor, info->vendor, sizeof(resp->vendor));
		strncpy(resp->product, info->product, sizeof(resp->product));
		strncpy(resp->revision, "1.0", sizeof(resp->revision));
		info->buff_used = sizeof(*resp);
		break;
	}
	case SCSI_TST_U_RDY:
		break;
	case SCSI_RD_CAPAC: {
		struct scsi_read_capacity_resp *resp = (void *)info->buff;
		uint blocks;

		if (info->file_size)
			blocks = info->file_size / info->block_size - 1;
		else
			blocks = 0;
		resp->last_block_addr = cpu_to_be32(blocks);
		resp->block_len = cpu_to_be32(info->block_size);
		info->buff_used = sizeof(*resp);
		break;
	}
	case SCSI_READ10: {
		const struct scsi_read10_req *read_req = (void *)req;

		info->seek_block = be32_to_cpu(read_req->lba);
		info->read_len = be16_to_cpu(read_req->xfer_len);
		info->buff_used = info->read_len * info->block_size;
		ret = SCSI_EMUL_DO_READ;
		break;
	}
	case SCSI_WRITE10: {
		const struct scsi_write10_req *write_req = (void *)req;

		info->seek_block = be32_to_cpu(write_req->lba);
		info->write_len = be16_to_cpu(write_req->xfer_len);
		info->buff_used = info->write_len * info->block_size;
		ret = SCSI_EMUL_DO_WRITE;
		break;
	}
	default:
		debug("Command not supported: %x\n", req->cmd[0]);
		ret = -EPROTONOSUPPORT;
	}
	if (ret >= 0)
		info->phase = info->transfer_len ? SCSIPH_DATA : SCSIPH_STATUS;
	log_debug("   - done %x: ret=%d\n", *req->cmd, ret);

	return ret;
}
