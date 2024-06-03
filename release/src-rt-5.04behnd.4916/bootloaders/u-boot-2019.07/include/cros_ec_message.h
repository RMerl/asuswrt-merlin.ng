/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Chromium OS Matrix Keyboard Message Protocol definitions
 *
 * Copyright (c) 2012 The Chromium OS Authors.
 */

#ifndef _CROS_MESSAGE_H
#define _CROS_MESSAGE_H

/*
 * Command interface between EC and AP, for LPC, I2C and SPI interfaces.
 *
 * This is copied from the Chromium OS Open Source Embedded Controller code.
 */
enum {
	/* The header byte, which follows the preamble */
	MSG_HEADER	= 0xec,

	MSG_HEADER_BYTES	= 3,
	MSG_TRAILER_BYTES	= 2,
	MSG_PROTO_BYTES		= MSG_HEADER_BYTES + MSG_TRAILER_BYTES,

	/* Max length of messages */
	MSG_BYTES		= EC_PROTO2_MAX_PARAM_SIZE + MSG_PROTO_BYTES,
};

#endif
