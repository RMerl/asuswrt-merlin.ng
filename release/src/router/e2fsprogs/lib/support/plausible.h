/*
 * plausible.h --- header file defining prototypes for helper functions
 * used by tune2fs and mke2fs
 *
 * Copyright 2014 by Oracle, Inc.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#ifndef PLAUSIBLE_H_
#define PLAUSIBLE_H_

/*
 * Flags for check_plausibility()
 */
#define CHECK_BLOCK_DEV	0x0001
#define CREATE_FILE	0x0002
#define CHECK_FS_EXIST	0x0004
#define VERBOSE_CREATE	0x0008
#define NO_SIZE		0x0010
#define QUIET_CHECK	0x0020

extern int check_plausibility(const char *device, int flags,
			      int *ret_is_dev);

#endif /* PLAUSIBLE_H_ */
