/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009-2013 ADVANSEE
 * Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
 *
 * Based on the mpc512x iim code:
 * Copyright 2008 Silicon Turnkey Express, Inc.
 * Martha Marx <mmarx@silicontkx.com>
 */

#ifndef _FUSE_H_
#define _FUSE_H_

/*
 * Read/Sense/Program/Override interface:
 *   bank:    Fuse bank
 *   word:    Fuse word within the bank
 *   val:     Value to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int fuse_read(u32 bank, u32 word, u32 *val);
int fuse_sense(u32 bank, u32 word, u32 *val);
int fuse_prog(u32 bank, u32 word, u32 val);
int fuse_override(u32 bank, u32 word, u32 val);

#endif	/* _FUSE_H_ */
