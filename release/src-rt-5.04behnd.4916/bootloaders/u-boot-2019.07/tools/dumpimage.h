/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Based on mkimage.c.
 *
 * Written by Guilherme Maciel Ferreira <guilherme.maciel.ferreira@gmail.com>
 */

#ifndef _DUMPIMAGE_H_
#define _DUMPIMAGE_H_

#include "os_support.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <u-boot/sha1.h>
#include "fdt_host.h"
#include "imagetool.h"

#undef DUMPIMAGE_DEBUG

#ifdef DUMPIMAGE_DEBUG
#define debug(fmt, args...)	printf(fmt, ##args)
#else
#define debug(fmt, args...)
#endif /* DUMPIMAGE_DEBUG */

#endif /* _DUMPIMAGE_H_ */
