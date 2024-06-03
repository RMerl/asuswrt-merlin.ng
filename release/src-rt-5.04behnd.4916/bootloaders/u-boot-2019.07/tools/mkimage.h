/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2004
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
 */

#ifndef _MKIIMAGE_H_
#define _MKIIMAGE_H_

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

#undef MKIMAGE_DEBUG

#ifdef MKIMAGE_DEBUG
#define debug(fmt,args...)	printf (fmt ,##args)
#else
#define debug(fmt,args...)
#endif /* MKIMAGE_DEBUG */

static inline void *map_sysmem(ulong paddr, unsigned long len)
{
	return (void *)(uintptr_t)paddr;
}

static inline ulong map_to_sysmem(void *ptr)
{
	return (ulong)(uintptr_t)ptr;
}

#define MKIMAGE_TMPFILE_SUFFIX		".tmp"
#define MKIMAGE_MAX_TMPFILE_LEN		256
#define MKIMAGE_DEFAULT_DTC_OPTIONS	"-I dts -O dtb -p 500"
#define MKIMAGE_MAX_DTC_CMDLINE_LEN	512

#endif /* _MKIIMAGE_H_ */
