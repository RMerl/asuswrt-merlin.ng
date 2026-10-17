/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 *
 */

#include <sysinfo.h>

enum {
	BOARD_HWVERSION = SYSINFO_ID_BOARD_MODEL,
	BOARD_MULTICHANNEL = SYSINFO_ID_USER,
	BOARD_VARIANT
};

enum {
	VAR_CON,
	VAR_CPU,
};
