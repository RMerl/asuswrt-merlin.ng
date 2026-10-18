/*
 * cma_driver.h - Broadcom STB platform CMA driver (UAPI header)
 *
 * Copyright (C) 2009 - 2013 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef __UAPI_CMA_DRIVER_H__
#define __UAPI_CMA_DRIVER_H__

#include <linux/types.h>

#define CMA_NUM_RANGES		8
#define CMA_DEV_MAX		6
#define CMA_DEV_NAME		"brcm_cma"
#define CMA_CLASS_NAME		"brcm_cma"

#define MMAP_TYPE_NORMAL	0
#define MMAP_TYPE_UNCACHED	1
#define MMAP_TYPE_WC		2

#define CMA_DEV_MAGIC		'c'
#define CMA_DEV_IOC_GETMEM	_IOWR(CMA_DEV_MAGIC, 1, int)
#define CMA_DEV_IOC_PUTMEM	_IOWR(CMA_DEV_MAGIC, 2, int)
#define CMA_DEV_IOC_GETPHYSINFO _IOWR(CMA_DEV_MAGIC, 3, int)
#define CMA_DEV_IOC_GETNUMREGS	_IOWR(CMA_DEV_MAGIC, 4, int)
#define CMA_DEV_IOC_GETREGINFO	_IOWR(CMA_DEV_MAGIC, 5, int)
#define CMA_DEV_IOC_GET_PG_PROT	_IOWR(CMA_DEV_MAGIC, 6, int)
#define CMA_DEV_IOC_SET_PG_PROT	_IOWR(CMA_DEV_MAGIC, 7, int)
#define CMA_DEV_IOC_VERSION	_IOR(CMA_DEV_MAGIC, 8, __u32)
#define CMA_DEV_IOC_MAXNR	8

struct ioc_params {
	__u32 cma_dev_index;	/* [in] region */
	__u64 addr;		/* [in/out] addr */
	__u32 num_bytes;	/* [in/out] num. bytes */
	__u32 align_bytes;	/* [in] alignment */
	__u32 region;		/* [in] region */
	__u32 num_regions;	/* [out] num regions */
	__s32 memc;		/* [out] memc */
	__u32 status;		/* [out] status */
};

#endif /* __UAPI_CMA_DRIVER_H__ */
