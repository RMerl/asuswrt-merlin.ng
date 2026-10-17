/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2020 Foundries Ltd */

#ifndef __TA_RPC_TEST_H
#define __TA_RPC_TEST_H

#define TA_RPC_TEST_UUID { 0x48420575, 0x96ca, 0x401a, \
		      { 0x89, 0x91, 0x1e, 0xfd, 0xce, 0xbd, 0x7d, 0x04 } }

/*
 * Does a reverse RPC call for I2C read
 *
 * in		params[0].value.a:	bus number
 * in		params[0].value.b:	chip address
 * in		params[0].value.c:	control flags
 * inout	params[1].u.memref:	buffer to read data
 */
#define TA_RPC_TEST_CMD_I2C_READ	0

/*
 * Does a reverse RPC call for I2C write
 *
 * in		params[0].value.a:	bus number
 * in		params[0].value.b:	chip address
 * in		params[0].value.c:	control flags
 * inout	params[1].u.memref:	buffer with data to write
 */
#define TA_RPC_TEST_CMD_I2C_WRITE	1

#endif /* __TA_RPC_TEST_H */
