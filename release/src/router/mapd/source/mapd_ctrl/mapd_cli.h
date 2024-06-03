/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mapd_cli.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __mapd_cli_H__
#define __mapd_cli_H__

enum param_type {
	STRING_TYPE,
	INTEGER_TYPE,
};

struct mapd_cli_cmd {
	const char *cmd;
	int (*cmd_handler)(struct mapd_interface_ctrl *ctrl, int argc, char *argv[]);
	const char *usage;
};

struct mapd_cli_get_param {
	const char *param;
	const char *usage;
};

struct mapd_cli_set_param {
	const char *param;
	const char *usage;
	enum param_type type;
};

#endif /* __mapd_cli_H__ */
