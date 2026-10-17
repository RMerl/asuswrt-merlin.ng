/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Emulation of enough SCSI commands to find and read from a unit
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 *
 * implementations of SCSI functions required so that CONFIG_SCSI can be enabled
 * for sandbox
 */

#ifndef __scsi_emul_h
#define __scsi_emul_h

/**
 * struct scsi_emul_info - information for emulating a SCSI device
 *
 * @vendor: Vendor name
 * @product: Product name
 * @block_size: Block size of device in bytes (normally 512)
 * @file_size: Size of the backing file for this emulator, in bytes
 * @seek_block: Seek position for file (block number)
 *
 * @phase: Current SCSI phase
 * @buff_used: Number of bytes ready to transfer back to host
 * @read_len: Number of bytes of data left in the current read command
 * @alloc_len: Allocation length from the last incoming command
 * @transfer_len: Transfer length from CBW header
 * @buff: Data buffer for outgoing data
 */
struct scsi_emul_info {
	/* provided by the caller: */
	void *buff;
	const char *vendor;
	const char *product;
	int block_size;
	loff_t file_size;
	int seek_block;

	/* state maintained by the emulator: */
	enum scsi_cmd_phase phase;
	int buff_used;
	int read_len;
	int write_len;
	uint seek_pos;
	int alloc_len;
	uint transfer_len;
};

/**
 * Return value from sb_scsi_emul_command() indicates that a read or write is
 * being started
 */
enum {
	SCSI_EMUL_DO_READ	= 1,
	SCSI_EMUL_DO_WRITE	= 2,
};

/**
 * sb_scsi_emul_command() - Process a SCSI command
 *
 * This sets up the response in info->buff and updates various other values
 * in info.
 *
 * If SCSI_EMUL_DO_READ is returned then the caller should set up so that the
 * backing file can be read, or return an error status if there is no file.
 *
 * @info: Emulation information
 * @req: Request to process
 * @len: Length of request in bytes
 * @return SCSI_EMUL_DO_READ if a read has started, SCSI_EMUL_DO_WRITE if a
 *	write has started, 0 if some other operation has started, -ve if there
 *	was an error
 */
int sb_scsi_emul_command(struct scsi_emul_info *info,
			 const struct scsi_cmd *req, int len);

#endif
