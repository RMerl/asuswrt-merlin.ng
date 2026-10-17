/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Marek Behún <kabel@kernel.org>
 * Copyright (C) 2021 Pali Rohár <pali@kernel.org>
 */

#ifndef _MVEBU_MBOX_H
#define _MVEBU_MBOX_H

enum mbox_cmd {
	MBOX_CMD_GET_RANDOM	= 1,
	MBOX_CMD_BOARD_INFO,
	MBOX_CMD_ECDSA_PUB_KEY,
	MBOX_CMD_HASH,
	MBOX_CMD_SIGN,
	MBOX_CMD_VERIFY,

	MBOX_CMD_OTP_READ,
	MBOX_CMD_OTP_WRITE,

	MBOX_CMD_REBOOT,

	/* OTP read commands supported by Marvell fuse.bin firmware */
	MBOX_CMD_OTP_READ_1B	= 257,
	MBOX_CMD_OTP_READ_8B,
	MBOX_CMD_OTP_READ_32B,
	MBOX_CMD_OTP_READ_64B,
	MBOX_CMD_OTP_READ_256B,

	/* OTP write commands supported by Marvell fuse.bin firmware */
	MBOX_CMD_OTP_WRITE_1B	= 513,
	MBOX_CMD_OTP_WRITE_8B,
	MBOX_CMD_OTP_WRITE_32B,
	MBOX_CMD_OTP_WRITE_64B,
	MBOX_CMD_OTP_WRITE_256B,
};

int mbox_do_cmd(enum mbox_cmd cmd, u32 *in, int nin, u32 *out, int nout);

#endif
