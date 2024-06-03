/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */

/*
 * Header file for using yaffs in an application via
 * a direct interface.
 */


#ifndef __YAFFS_OSGLUE_H__
#define __YAFFS_OSGLUE_H__


#include "yportenv.h"

void yaffsfs_Lock(void);
void yaffsfs_Unlock(void);

u32 yaffsfs_CurrentTime(void);

void yaffsfs_SetError(int err);

void *yaffsfs_malloc(size_t size);
void yaffsfs_free(void *ptr);

void yaffsfs_OSInitialisation(void);


#endif
