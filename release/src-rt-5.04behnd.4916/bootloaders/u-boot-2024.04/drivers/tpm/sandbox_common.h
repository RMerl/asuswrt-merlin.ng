/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common features for sandbox TPM1 and TPM2 implementations
 *
 * Copyright 2021 Google LLC
 */

#ifndef __TPM_SANDBOX_COMMON_H
#define __TPM_SANDBOX_COMMON_H

/*
 * These numbers derive from adding the sizes of command fields as shown in
 * the TPM commands manual.
 */
#define TPM_HDR_LEN	10

/* These are the different non-volatile spaces that we emulate */
enum sandbox_nv_space {
	NV_SEQ_ENABLE_LOCKING,
	NV_SEQ_GLOBAL_LOCK,
	NV_SEQ_FIRMWARE,
	NV_SEQ_KERNEL,
	NV_SEQ_BACKUP,
	NV_SEQ_FWMP,
	NV_SEQ_REC_HASH,

	NV_SEQ_COUNT,
};

/* TPM NVRAM location indices */
#define FIRMWARE_NV_INDEX		0x1007
#define KERNEL_NV_INDEX			0x1008
#define BACKUP_NV_INDEX			0x1009
#define FWMP_NV_INDEX			0x100a
#define MRC_REC_HASH_NV_INDEX		0x100b

/* Size of each non-volatile space */
#define NV_DATA_SIZE		0x28

/**
 * struct nvdata_state - state of a single non-volatile-data 'space'
 *
 * @present: true if present
 * @length: length in bytes (max NV_DATA_SIZE)
 * @data: contents of non-volatile space
 */
struct nvdata_state {
	bool present;
	int length;
	u8 data[NV_DATA_SIZE];
};

/**
 * sb_tpm_index_to_seq() - convert an index into a space sequence number
 *
 * This converts the index as used by the vboot code into an internal sequence
 * number used by the sandbox emulation.
 *
 * @index: Index to use (FIRMWARE_NV_INDEX, etc.)
 * Return: associated space (enum sandbox_nv_space)
 */
int sb_tpm_index_to_seq(uint index);

/**
 * sb_tpm_read_data() - Read non-volatile data
 *
 * This handles a TPM read of nvdata. If the nvdata is not present, a
 * TPM_BADINDEX error is put in the buffer. If @length is too large,
 * TPM_BAD_DATASIZE is put in the buffer.
 *
 * @nvdata: Current nvdata state
 * @seq: Sequence number to read
 * @recvbuf: Buffer to update with the TPM response, assumed to contain zeroes
 * @data_ofs: Offset of the 'data' portion of @recvbuf
 * @length: Number of bytes to read
 */
void sb_tpm_read_data(const struct nvdata_state nvdata[NV_SEQ_COUNT],
		      enum sandbox_nv_space seq, u8 *recvbuf, int data_ofs,
		      int length);

/**
 * sb_tpm_write_data() - Write non-volatile data
 *
 * If @length is too large, an error is logged and nothing is written.
 *
 * @nvdata: Current nvdata state
 * @seq: Sequence number to read
 * @buf: Buffer containing the data to write
 * @data_ofs: Offset of the 'data' portion of @buf
 * @length: Number of bytes to write
 */
void sb_tpm_write_data(struct nvdata_state nvdata[NV_SEQ_COUNT],
		       enum sandbox_nv_space seq, const u8 *buf, int data_ofs,
		       int length);

/**
 * sb_tpm_define_data() - Set up non-volatile data
 *
 * If @length is too large, an error is logged and nothing is written.
 *
 * @nvdata: Current nvdata state
 * @seq: Sequence number to set up
 * @length: Length of space in bytes
 */
void sb_tpm_define_data(struct nvdata_state nvdata[NV_SEQ_COUNT],
			enum sandbox_nv_space seq, int length);

#endif
