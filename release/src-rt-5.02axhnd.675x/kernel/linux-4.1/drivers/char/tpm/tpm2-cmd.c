/*
 * Copyright (C) 2014 Intel Corporation
 *
 * Authors:
 * Jarkko Sakkinen <jarkko.sakkinen@linux.intel.com>
 *
 * Maintained by: <tpmdd-devel@lists.sourceforge.net>
 *
 * This file contains TPM2 protocol implementations of the commands
 * used by the kernel internally.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */

#include "tpm.h"

struct tpm2_startup_in {
	__be16	startup_type;
} __packed;

struct tpm2_self_test_in {
	u8	full_test;
} __packed;

struct tpm2_pcr_read_in {
	__be32	pcr_selects_cnt;
	__be16	hash_alg;
	u8	pcr_select_size;
	u8	pcr_select[TPM2_PCR_SELECT_MIN];
} __packed;

struct tpm2_pcr_read_out {
	__be32	update_cnt;
	__be32	pcr_selects_cnt;
	__be16	hash_alg;
	u8	pcr_select_size;
	u8	pcr_select[TPM2_PCR_SELECT_MIN];
	__be32	digests_cnt;
	__be16	digest_size;
	u8	digest[TPM_DIGEST_SIZE];
} __packed;

struct tpm2_null_auth_area {
	__be32			handle;
	__be16			nonce_size;
	u8			attributes;
	__be16			auth_size;
} __packed;

struct tpm2_pcr_extend_in {
	__be32				pcr_idx;
	__be32				auth_area_size;
	struct tpm2_null_auth_area	auth_area;
	__be32				digest_cnt;
	__be16				hash_alg;
	u8				digest[TPM_DIGEST_SIZE];
} __packed;

struct tpm2_get_tpm_pt_in {
	__be32	cap_id;
	__be32	property_id;
	__be32	property_cnt;
} __packed;

struct tpm2_get_tpm_pt_out {
	u8	more_data;
	__be32	subcap_id;
	__be32	property_cnt;
	__be32	property_id;
	__be32	value;
} __packed;

struct tpm2_get_random_in {
	__be16	size;
} __packed;

struct tpm2_get_random_out {
	__be16	size;
	u8	buffer[TPM_MAX_RNG_DATA];
} __packed;

union tpm2_cmd_params {
	struct	tpm2_startup_in		startup_in;
	struct	tpm2_self_test_in	selftest_in;
	struct	tpm2_pcr_read_in	pcrread_in;
	struct	tpm2_pcr_read_out	pcrread_out;
	struct	tpm2_pcr_extend_in	pcrextend_in;
	struct	tpm2_get_tpm_pt_in	get_tpm_pt_in;
	struct	tpm2_get_tpm_pt_out	get_tpm_pt_out;
	struct	tpm2_get_random_in	getrandom_in;
	struct	tpm2_get_random_out	getrandom_out;
};

struct tpm2_cmd {
	tpm_cmd_header		header;
	union tpm2_cmd_params	params;
} __packed;

/*
 * Array with one entry per ordinal defining the maximum amount
 * of time the chip could take to return the result. The values
 * of the SHORT, MEDIUM, and LONG durations are taken from the
 * PC Client Profile (PTP) specification.
 */
static const u8 tpm2_ordinal_duration[TPM2_CC_LAST - TPM2_CC_FIRST + 1] = {
	TPM_UNDEFINED,		/* 11F */
	TPM_UNDEFINED,		/* 120 */
	TPM_LONG,		/* 121 */
	TPM_UNDEFINED,		/* 122 */
	TPM_UNDEFINED,		/* 123 */
	TPM_UNDEFINED,		/* 124 */
	TPM_UNDEFINED,		/* 125 */
	TPM_UNDEFINED,		/* 126 */
	TPM_UNDEFINED,		/* 127 */
	TPM_UNDEFINED,		/* 128 */
	TPM_LONG,		/* 129 */
	TPM_UNDEFINED,		/* 12a */
	TPM_UNDEFINED,		/* 12b */
	TPM_UNDEFINED,		/* 12c */
	TPM_UNDEFINED,		/* 12d */
	TPM_UNDEFINED,		/* 12e */
	TPM_UNDEFINED,		/* 12f */
	TPM_UNDEFINED,		/* 130 */
	TPM_UNDEFINED,		/* 131 */
	TPM_UNDEFINED,		/* 132 */
	TPM_UNDEFINED,		/* 133 */
	TPM_UNDEFINED,		/* 134 */
	TPM_UNDEFINED,		/* 135 */
	TPM_UNDEFINED,		/* 136 */
	TPM_UNDEFINED,		/* 137 */
	TPM_UNDEFINED,		/* 138 */
	TPM_UNDEFINED,		/* 139 */
	TPM_UNDEFINED,		/* 13a */
	TPM_UNDEFINED,		/* 13b */
	TPM_UNDEFINED,		/* 13c */
	TPM_UNDEFINED,		/* 13d */
	TPM_MEDIUM,		/* 13e */
	TPM_UNDEFINED,		/* 13f */
	TPM_UNDEFINED,		/* 140 */
	TPM_UNDEFINED,		/* 141 */
	TPM_UNDEFINED,		/* 142 */
	TPM_LONG,		/* 143 */
	TPM_MEDIUM,		/* 144 */
	TPM_UNDEFINED,		/* 145 */
	TPM_UNDEFINED,		/* 146 */
	TPM_UNDEFINED,		/* 147 */
	TPM_UNDEFINED,		/* 148 */
	TPM_UNDEFINED,		/* 149 */
	TPM_UNDEFINED,		/* 14a */
	TPM_UNDEFINED,		/* 14b */
	TPM_UNDEFINED,		/* 14c */
	TPM_UNDEFINED,		/* 14d */
	TPM_LONG,		/* 14e */
	TPM_UNDEFINED,		/* 14f */
	TPM_UNDEFINED,		/* 150 */
	TPM_UNDEFINED,		/* 151 */
	TPM_UNDEFINED,		/* 152 */
	TPM_UNDEFINED,		/* 153 */
	TPM_UNDEFINED,		/* 154 */
	TPM_UNDEFINED,		/* 155 */
	TPM_UNDEFINED,		/* 156 */
	TPM_UNDEFINED,		/* 157 */
	TPM_UNDEFINED,		/* 158 */
	TPM_UNDEFINED,		/* 159 */
	TPM_UNDEFINED,		/* 15a */
	TPM_UNDEFINED,		/* 15b */
	TPM_MEDIUM,		/* 15c */
	TPM_UNDEFINED,		/* 15d */
	TPM_UNDEFINED,		/* 15e */
	TPM_UNDEFINED,		/* 15f */
	TPM_UNDEFINED,		/* 160 */
	TPM_UNDEFINED,		/* 161 */
	TPM_UNDEFINED,		/* 162 */
	TPM_UNDEFINED,		/* 163 */
	TPM_UNDEFINED,		/* 164 */
	TPM_UNDEFINED,		/* 165 */
	TPM_UNDEFINED,		/* 166 */
	TPM_UNDEFINED,		/* 167 */
	TPM_UNDEFINED,		/* 168 */
	TPM_UNDEFINED,		/* 169 */
	TPM_UNDEFINED,		/* 16a */
	TPM_UNDEFINED,		/* 16b */
	TPM_UNDEFINED,		/* 16c */
	TPM_UNDEFINED,		/* 16d */
	TPM_UNDEFINED,		/* 16e */
	TPM_UNDEFINED,		/* 16f */
	TPM_UNDEFINED,		/* 170 */
	TPM_UNDEFINED,		/* 171 */
	TPM_UNDEFINED,		/* 172 */
	TPM_UNDEFINED,		/* 173 */
	TPM_UNDEFINED,		/* 174 */
	TPM_UNDEFINED,		/* 175 */
	TPM_UNDEFINED,		/* 176 */
	TPM_LONG,		/* 177 */
	TPM_UNDEFINED,		/* 178 */
	TPM_UNDEFINED,		/* 179 */
	TPM_MEDIUM,		/* 17a */
	TPM_LONG,		/* 17b */
	TPM_UNDEFINED,		/* 17c */
	TPM_UNDEFINED,		/* 17d */
	TPM_UNDEFINED,		/* 17e */
	TPM_UNDEFINED,		/* 17f */
	TPM_UNDEFINED,		/* 180 */
	TPM_UNDEFINED,		/* 181 */
	TPM_MEDIUM,		/* 182 */
	TPM_UNDEFINED,		/* 183 */
	TPM_UNDEFINED,		/* 184 */
	TPM_MEDIUM,		/* 185 */
	TPM_MEDIUM,		/* 186 */
	TPM_UNDEFINED,		/* 187 */
	TPM_UNDEFINED,		/* 188 */
	TPM_UNDEFINED,		/* 189 */
	TPM_UNDEFINED,		/* 18a */
	TPM_UNDEFINED,		/* 18b */
	TPM_UNDEFINED,		/* 18c */
	TPM_UNDEFINED,		/* 18d */
	TPM_UNDEFINED,		/* 18e */
	TPM_UNDEFINED		/* 18f */
};

#define TPM2_PCR_READ_IN_SIZE \
	(sizeof(struct tpm_input_header) + \
	 sizeof(struct tpm2_pcr_read_in))

static const struct tpm_input_header tpm2_pcrread_header = {
	.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
	.length = cpu_to_be32(TPM2_PCR_READ_IN_SIZE),
	.ordinal = cpu_to_be32(TPM2_CC_PCR_READ)
};

/**
 * tpm2_pcr_read() - read a PCR value
 * @chip:	TPM chip to use.
 * @pcr_idx:	index of the PCR to read.
 * @ref_buf:	buffer to store the resulting hash,
 *
 * 0 is returned when the operation is successful. If a negative number is
 * returned it remarks a POSIX error code. If a positive number is returned
 * it remarks a TPM error.
 */
int tpm2_pcr_read(struct tpm_chip *chip, int pcr_idx, u8 *res_buf)
{
	int rc;
	struct tpm2_cmd cmd;
	u8 *buf;

	if (pcr_idx >= TPM2_PLATFORM_PCR)
		return -EINVAL;

	cmd.header.in = tpm2_pcrread_header;
	cmd.params.pcrread_in.pcr_selects_cnt = cpu_to_be32(1);
	cmd.params.pcrread_in.hash_alg = cpu_to_be16(TPM2_ALG_SHA1);
	cmd.params.pcrread_in.pcr_select_size = TPM2_PCR_SELECT_MIN;

	memset(cmd.params.pcrread_in.pcr_select, 0,
	       sizeof(cmd.params.pcrread_in.pcr_select));
	cmd.params.pcrread_in.pcr_select[pcr_idx >> 3] = 1 << (pcr_idx & 0x7);

	rc = tpm_transmit_cmd(chip, &cmd, sizeof(cmd),
			      "attempting to read a pcr value");
	if (rc == 0) {
		buf = cmd.params.pcrread_out.digest;
		memcpy(res_buf, buf, TPM_DIGEST_SIZE);
	}

	return rc;
}

#define TPM2_GET_PCREXTEND_IN_SIZE \
	(sizeof(struct tpm_input_header) + \
	 sizeof(struct tpm2_pcr_extend_in))

static const struct tpm_input_header tpm2_pcrextend_header = {
	.tag = cpu_to_be16(TPM2_ST_SESSIONS),
	.length = cpu_to_be32(TPM2_GET_PCREXTEND_IN_SIZE),
	.ordinal = cpu_to_be32(TPM2_CC_PCR_EXTEND)
};

/**
 * tpm2_pcr_extend() - extend a PCR value
 * @chip:	TPM chip to use.
 * @pcr_idx:	index of the PCR.
 * @hash:	hash value to use for the extend operation.
 *
 * 0 is returned when the operation is successful. If a negative number is
 * returned it remarks a POSIX error code. If a positive number is returned
 * it remarks a TPM error.
 */
int tpm2_pcr_extend(struct tpm_chip *chip, int pcr_idx, const u8 *hash)
{
	struct tpm2_cmd cmd;
	int rc;

	cmd.header.in = tpm2_pcrextend_header;
	cmd.params.pcrextend_in.pcr_idx = cpu_to_be32(pcr_idx);
	cmd.params.pcrextend_in.auth_area_size =
		cpu_to_be32(sizeof(struct tpm2_null_auth_area));
	cmd.params.pcrextend_in.auth_area.handle =
		cpu_to_be32(TPM2_RS_PW);
	cmd.params.pcrextend_in.auth_area.nonce_size = 0;
	cmd.params.pcrextend_in.auth_area.attributes = 0;
	cmd.params.pcrextend_in.auth_area.auth_size = 0;
	cmd.params.pcrextend_in.digest_cnt = cpu_to_be32(1);
	cmd.params.pcrextend_in.hash_alg = cpu_to_be16(TPM2_ALG_SHA1);
	memcpy(cmd.params.pcrextend_in.digest, hash, TPM_DIGEST_SIZE);

	rc = tpm_transmit_cmd(chip, &cmd, sizeof(cmd),
			      "attempting extend a PCR value");

	return rc;
}

#define TPM2_GETRANDOM_IN_SIZE \
	(sizeof(struct tpm_input_header) + \
	 sizeof(struct tpm2_get_random_in))

static const struct tpm_input_header tpm2_getrandom_header = {
	.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
	.length = cpu_to_be32(TPM2_GETRANDOM_IN_SIZE),
	.ordinal = cpu_to_be32(TPM2_CC_GET_RANDOM)
};

/**
 * tpm2_get_random() - get random bytes from the TPM RNG
 * @chip: TPM chip to use
 * @out: destination buffer for the random bytes
 * @max: the max number of bytes to write to @out
 *
 * 0 is returned when the operation is successful. If a negative number is
 * returned it remarks a POSIX error code. If a positive number is returned
 * it remarks a TPM error.
 */
int tpm2_get_random(struct tpm_chip *chip, u8 *out, size_t max)
{
	struct tpm2_cmd cmd;
	u32 recd;
	u32 num_bytes;
	int err;
	int total = 0;
	int retries = 5;
	u8 *dest = out;

	num_bytes = min_t(u32, max, sizeof(cmd.params.getrandom_out.buffer));

	if (!out || !num_bytes ||
	    max > sizeof(cmd.params.getrandom_out.buffer))
		return -EINVAL;

	do {
		cmd.header.in = tpm2_getrandom_header;
		cmd.params.getrandom_in.size = cpu_to_be16(num_bytes);

		err = tpm_transmit_cmd(chip, &cmd, sizeof(cmd),
				       "attempting get random");
		if (err)
			break;

		recd = min_t(u32, be16_to_cpu(cmd.params.getrandom_out.size),
			     num_bytes);
		memcpy(dest, cmd.params.getrandom_out.buffer, recd);

		dest += recd;
		total += recd;
		num_bytes -= recd;
	} while (retries-- && total < max);

	return total ? total : -EIO;
}

#define TPM2_GET_TPM_PT_IN_SIZE \
	(sizeof(struct tpm_input_header) + \
	 sizeof(struct tpm2_get_tpm_pt_in))

static const struct tpm_input_header tpm2_get_tpm_pt_header = {
	.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
	.length = cpu_to_be32(TPM2_GET_TPM_PT_IN_SIZE),
	.ordinal = cpu_to_be32(TPM2_CC_GET_CAPABILITY)
};

/**
 * tpm2_get_tpm_pt() - get value of a TPM_CAP_TPM_PROPERTIES type property
 * @chip:		TPM chip to use.
 * @property_id:	property ID.
 * @value:		output variable.
 * @desc:		passed to tpm_transmit_cmd()
 *
 * 0 is returned when the operation is successful. If a negative number is
 * returned it remarks a POSIX error code. If a positive number is returned
 * it remarks a TPM error.
 */
ssize_t tpm2_get_tpm_pt(struct tpm_chip *chip, u32 property_id,  u32 *value,
			const char *desc)
{
	struct tpm2_cmd cmd;
	int rc;

	cmd.header.in = tpm2_get_tpm_pt_header;
	cmd.params.get_tpm_pt_in.cap_id = cpu_to_be32(TPM2_CAP_TPM_PROPERTIES);
	cmd.params.get_tpm_pt_in.property_id = cpu_to_be32(property_id);
	cmd.params.get_tpm_pt_in.property_cnt = cpu_to_be32(1);

	rc = tpm_transmit_cmd(chip, &cmd, sizeof(cmd), desc);
	if (!rc)
		*value = cmd.params.get_tpm_pt_out.value;

	return rc;
}

#define TPM2_STARTUP_IN_SIZE \
	(sizeof(struct tpm_input_header) + \
	 sizeof(struct tpm2_startup_in))

static const struct tpm_input_header tpm2_startup_header = {
	.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
	.length = cpu_to_be32(TPM2_STARTUP_IN_SIZE),
	.ordinal = cpu_to_be32(TPM2_CC_STARTUP)
};

/**
 * tpm2_startup() - send startup command to the TPM chip
 * @chip:		TPM chip to use.
 * @startup_type	startup type. The value is either
 *			TPM_SU_CLEAR or TPM_SU_STATE.
 *
 * 0 is returned when the operation is successful. If a negative number is
 * returned it remarks a POSIX error code. If a positive number is returned
 * it remarks a TPM error.
 */
int tpm2_startup(struct tpm_chip *chip, u16 startup_type)
{
	struct tpm2_cmd cmd;

	cmd.header.in = tpm2_startup_header;

	cmd.params.startup_in.startup_type = cpu_to_be16(startup_type);
	return tpm_transmit_cmd(chip, &cmd, sizeof(cmd),
				"attempting to start the TPM");
}
EXPORT_SYMBOL_GPL(tpm2_startup);

#define TPM2_SHUTDOWN_IN_SIZE \
	(sizeof(struct tpm_input_header) + \
	 sizeof(struct tpm2_startup_in))

static const struct tpm_input_header tpm2_shutdown_header = {
	.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
	.length = cpu_to_be32(TPM2_SHUTDOWN_IN_SIZE),
	.ordinal = cpu_to_be32(TPM2_CC_SHUTDOWN)
};

/**
 * tpm2_shutdown() - send shutdown command to the TPM chip
 * @chip:		TPM chip to use.
 * @shutdown_type	shutdown type. The value is either
 *			TPM_SU_CLEAR or TPM_SU_STATE.
 */
void tpm2_shutdown(struct tpm_chip *chip, u16 shutdown_type)
{
	struct tpm2_cmd cmd;
	int rc;

	cmd.header.in = tpm2_shutdown_header;
	cmd.params.startup_in.startup_type = cpu_to_be16(shutdown_type);

	rc = tpm_transmit_cmd(chip, &cmd, sizeof(cmd), "stopping the TPM");

	/* In places where shutdown command is sent there's no much we can do
	 * except print the error code on a system failure.
	 */
	if (rc < 0)
		dev_warn(chip->pdev, "transmit returned %d while stopping the TPM",
			 rc);
}
EXPORT_SYMBOL_GPL(tpm2_shutdown);

/*
 * tpm2_calc_ordinal_duration() - maximum duration for a command
 * @chip:	TPM chip to use.
 * @ordinal:	command code number.
 *
 * 0 is returned when the operation is successful. If a negative number is
 * returned it remarks a POSIX error code. If a positive number is returned
 * it remarks a TPM error.
 */
unsigned long tpm2_calc_ordinal_duration(struct tpm_chip *chip, u32 ordinal)
{
	int index = TPM_UNDEFINED;
	int duration = 0;

	if (ordinal >= TPM2_CC_FIRST && ordinal <= TPM2_CC_LAST)
		index = tpm2_ordinal_duration[ordinal - TPM2_CC_FIRST];

	if (index != TPM_UNDEFINED)
		duration = chip->vendor.duration[index];

	if (duration <= 0)
		duration = 2 * 60 * HZ;

	return duration;
}
EXPORT_SYMBOL_GPL(tpm2_calc_ordinal_duration);

#define TPM2_SELF_TEST_IN_SIZE \
	(sizeof(struct tpm_input_header) + \
	 sizeof(struct tpm2_self_test_in))

static const struct tpm_input_header tpm2_selftest_header = {
	.tag = cpu_to_be16(TPM2_ST_NO_SESSIONS),
	.length = cpu_to_be32(TPM2_SELF_TEST_IN_SIZE),
	.ordinal = cpu_to_be32(TPM2_CC_SELF_TEST)
};

/**
 * tpm2_continue_selftest() - start a self test
 * @chip: TPM chip to use
 * @full: test all commands instead of testing only those that were not
 *        previously tested.
 *
 * 0 is returned when the operation is successful. If a negative number is
 * returned it remarks a POSIX error code. If a positive number is returned
 * it remarks a TPM error.
 */
static int tpm2_start_selftest(struct tpm_chip *chip, bool full)
{
	int rc;
	struct tpm2_cmd cmd;

	cmd.header.in = tpm2_selftest_header;
	cmd.params.selftest_in.full_test = full;

	rc = tpm_transmit_cmd(chip, &cmd, TPM2_SELF_TEST_IN_SIZE,
			      "continue selftest");

	/* At least some prototype chips seem to give RC_TESTING error
	 * immediately. This is a workaround for that.
	 */
	if (rc == TPM2_RC_TESTING) {
		dev_warn(chip->pdev, "Got RC_TESTING, ignoring\n");
		rc = 0;
	}

	return rc;
}

/**
 * tpm2_do_selftest() - run a full self test
 * @chip: TPM chip to use
 *
 * During the self test TPM2 commands return with the error code RC_TESTING.
 * Waiting is done by issuing PCR read until it executes successfully.
 *
 * 0 is returned when the operation is successful. If a negative number is
 * returned it remarks a POSIX error code. If a positive number is returned
 * it remarks a TPM error.
 */
int tpm2_do_selftest(struct tpm_chip *chip)
{
	int rc;
	unsigned int loops;
	unsigned int delay_msec = 100;
	unsigned long duration;
	struct tpm2_cmd cmd;
	int i;

	duration = tpm2_calc_ordinal_duration(chip, TPM2_CC_SELF_TEST);

	loops = jiffies_to_msecs(duration) / delay_msec;

	rc = tpm2_start_selftest(chip, true);
	if (rc)
		return rc;

	for (i = 0; i < loops; i++) {
		/* Attempt to read a PCR value */
		cmd.header.in = tpm2_pcrread_header;
		cmd.params.pcrread_in.pcr_selects_cnt = cpu_to_be32(1);
		cmd.params.pcrread_in.hash_alg = cpu_to_be16(TPM2_ALG_SHA1);
		cmd.params.pcrread_in.pcr_select_size = TPM2_PCR_SELECT_MIN;
		cmd.params.pcrread_in.pcr_select[0] = 0x01;
		cmd.params.pcrread_in.pcr_select[1] = 0x00;
		cmd.params.pcrread_in.pcr_select[2] = 0x00;

		rc = tpm_transmit_cmd(chip, (u8 *) &cmd, sizeof(cmd), NULL);
		if (rc < 0)
			break;

		rc = be32_to_cpu(cmd.header.out.return_code);
		if (rc != TPM2_RC_TESTING)
			break;

		msleep(delay_msec);
	}

	return rc;
}
EXPORT_SYMBOL_GPL(tpm2_do_selftest);

/**
 * tpm2_gen_interrupt() - generate an interrupt
 * @chip: TPM chip to use
 *
 * 0 is returned when the operation is successful. If a negative number is
 * returned it remarks a POSIX error code. If a positive number is returned
 * it remarks a TPM error.
 */
int tpm2_gen_interrupt(struct tpm_chip *chip)
{
	u32 dummy;

	return tpm2_get_tpm_pt(chip, 0x100, &dummy,
			       "attempting to generate an interrupt");
}
EXPORT_SYMBOL_GPL(tpm2_gen_interrupt);

/**
 * tpm2_probe() - probe TPM 2.0
 * @chip: TPM chip to use
 *
 * Send idempotent TPM 2.0 command and see whether TPM 2.0 chip replied based on
 * the reply tag.
 */
int tpm2_probe(struct tpm_chip *chip)
{
	struct tpm2_cmd cmd;
	int rc;

	cmd.header.in = tpm2_get_tpm_pt_header;
	cmd.params.get_tpm_pt_in.cap_id = cpu_to_be32(TPM2_CAP_TPM_PROPERTIES);
	cmd.params.get_tpm_pt_in.property_id = cpu_to_be32(0x100);
	cmd.params.get_tpm_pt_in.property_cnt = cpu_to_be32(1);

	rc = tpm_transmit(chip, (const char *) &cmd, sizeof(cmd));
	if (rc <  0)
		return rc;
	else if (rc < TPM_HEADER_SIZE)
		return -EFAULT;

	if (be16_to_cpu(cmd.header.out.tag) == TPM2_ST_NO_SESSIONS)
		chip->flags |= TPM_CHIP_FLAG_TPM2;

	return 0;
}
EXPORT_SYMBOL_GPL(tpm2_probe);
