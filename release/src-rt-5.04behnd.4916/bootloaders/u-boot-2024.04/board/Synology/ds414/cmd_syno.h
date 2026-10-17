/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Commands to deal with Synology specifics.
 *
 * Copyright (C) 2021  Phil Sutter <phil@nwl.cc>
 */

#ifndef _CMD_SYNO_H
#define _CMD_SYNO_H

#define SYNO_ETHADDR_MAX	4
#define SYNO_SN_TAG		"SN="
#define SYNO_CHKSUM_TAG		"CHK="

int do_syno_populate(int argc, char *const argv[]);

#endif /* _CMD_SYNO_H */
