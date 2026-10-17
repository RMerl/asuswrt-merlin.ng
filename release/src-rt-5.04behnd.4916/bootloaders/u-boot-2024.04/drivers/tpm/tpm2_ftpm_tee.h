/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Microsoft Corporation
 */

#ifndef __TPM2_FTPM_TEE_H__
#define __TPM2_FTPM_TEE_H__

/* This UUID is generated with uuidgen */
#define TA_FTPM_UUID { 0xBC50D971, 0xD4C9, 0x42C4, \
	{0x82, 0xCB, 0x34, 0x3F, 0xB7, 0xF3, 0x78, 0x96} }

/* The TAFs ID implemented in this TA */
#define FTPM_OPTEE_TA_SUBMIT_COMMAND  (0)
#define FTPM_OPTEE_TA_EMULATE_PPI     (1)

/* max. buffer size supported by fTPM  */
#define MAX_COMMAND_SIZE       4096
#define MAX_RESPONSE_SIZE      4096

/**
 * struct ftpm_tee_private - fTPM's private context
 * @tee_dev:  struct udevice for TEE.
 * @session:  fTPM TA session identifier.
 * @is_open:  Indicates whether the driver is already opened by client or not.
 * @shm:      Memory pool shared with fTPM TA in TEE.
 */
struct ftpm_tee_private {
	struct udevice *tee_dev;
	u32 session;
	int is_open;
	struct tee_shm *shm;
};

#endif /* __TPM2_FTPM_TEE_H__ */
