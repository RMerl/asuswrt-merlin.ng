/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2013-2015 Freescale Semiconductor Inc.
 */
#ifndef __FSL_DPMNG_H
#define __FSL_DPMNG_H

/* Management Complex General API
 * Contains general API for the Management Complex firmware
 */

struct fsl_mc_io;

/**
 * Management Complex firmware version information
 */
#define MC_VER_MAJOR 9
#define MC_VER_MINOR 0

/**
 * struct mc_versoin
 * @major: Major version number: incremented on API compatibility changes
 * @minor: Minor version number: incremented on API additions (that are
 *		backward compatible); reset when major version is incremented
 * @revision: Internal revision number: incremented on implementation changes
 *		and/or bug fixes that have no impact on API
 */
struct mc_version {
	uint32_t major;
	uint32_t minor;
	uint32_t revision;
};

/**
 * mc_get_version() - Retrieves the Management Complex firmware
 *			version information
 * @mc_io:		Pointer to opaque I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @mc_ver_info:	Returned version information structure
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int mc_get_version(struct fsl_mc_io	*mc_io,
		   uint32_t		cmd_flags,
		   struct mc_version	*mc_ver_info);

#endif /* __FSL_DPMNG_H */
