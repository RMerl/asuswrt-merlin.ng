/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
/****************************************************************************
 * Author: Tim Ross <tim.ross@broadcom.com>
 * Author: Venky Selvaraj <venky.selvaraj@broadcom.com>
 *****************************************************************************/
#ifndef _VFBIO_H_
#define _VFBIO_H_

#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#ifndef CONFIG_BCM_VFBIO_MQ
#define CONFIG_BCM_VFBIO_MQ
#endif
#endif
#include <linux/hdreg.h>
#include <linux/blkdev.h>
#ifdef CONFIG_BCM_VFBIO_MQ
#include <linux/blk-mq.h>
#endif
#include <linux/platform_device.h>
#include <itc_rpc.h>
#include <vfbio_rpc.h>
#include <vfbio_lvm.h>
#include <vfbio.h>

#define ANSI_BLACK	"\e[30;1m"
#define ANSI_RED	"\e[31;1m"
#define ANSI_GREEN	"\e[32;1m"
#define ANSI_YELLOW	"\e[33;1m"
#define ANSI_BLUE	"\e[34;1m"
#define ANSI_MAGENTA	"\e[35;1m"
#define ANSI_CYAN	"\e[36;1m"
#define ANSI_WHITE	"\e[37;1m"
#define ANSI_RESET	"\e[0m"
#define BLK(str)	ANSI_BLK str ANSI_RESET
#define RED(str)	ANSI_RED str ANSI_RESET
#define GRN(str)	ANSI_GREEN str ANSI_RESET
#define YLW(str)	ANSI_YELLOW str ANSI_RESET
#define BLU(str)	ANSI_BLUE str ANSI_RESET
#define MAG(str)	ANSI_MAGENTA str ANSI_RESET
#define CYN(str)	ANSI_CYAN str ANSI_RESET
#define WHT(str)	ANSI_WHITE str ANSI_RESET

struct vfbio_device {
	/* The following fields must be set before calling vfbio_device_create. */
	int			tunnel;
	char		*name;
	u32			blk_sz;
	u32			n_blks;
	u8			lun;
	bool		read_only;

	/* From this point on the structure is populated by vfbio_device_create */
	struct list_head	list;
	u32			blk_sz_shift;

	struct gendisk		*disk;
	struct hd_geometry	geo;

#ifdef CONFIG_BCM_VFBIO_MQ
	struct blk_mq_tag_set	tset;

#else
	spinlock_t		q_lock;
#endif
	struct request_queue	*q;

	atomic_t		pending_ops;
	bool			waiting_on_sync;

	struct platform_device	*pdev;
};

extern struct list_head vfdevs_list;

int vfbio_lun_request_timeout_msg(struct vfbio_device *vfbio, rpc_msg *msg);

/* Create vfbio device */
int vfbio_device_create(struct device *dev, struct vfbio_device *vfbio);

/* Delete vfbio device */
void vfbio_device_delete(struct vfbio_device *vfbio);

/* vfbio device resize */
int vfbio_device_resize(struct vfbio_device *vfbio, u32 new_blks);

/* Get vfbio device by lun */
struct vfbio_device *vfbio_device_get(int lun);

/* Get vfbio device by name */
struct vfbio_device *vfbio_device_get_by_name(const char *name);

/* Get next device. Get first if lun<0 */
struct vfbio_device *vfbio_device_get_next(int lun);

/* Get volume info */
int vfbio_get_info(struct vfbio_device *vfbio, struct vfbio_lun_descr *descr);

int vfbio_lvm_ioctl_init(void);
void vfbio_lvm_ioctl_exit(void);

#endif
