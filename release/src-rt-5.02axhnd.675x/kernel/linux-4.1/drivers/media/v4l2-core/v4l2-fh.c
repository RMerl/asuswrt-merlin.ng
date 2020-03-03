/*
 * v4l2-fh.c
 *
 * V4L2 file handles.
 *
 * Copyright (C) 2009--2010 Nokia Corporation.
 *
 * Contact: Sakari Ailus <sakari.ailus@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <linux/bitops.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-event.h>
#include <media/v4l2-ioctl.h>

void v4l2_fh_init(struct v4l2_fh *fh, struct video_device *vdev)
{
	fh->vdev = vdev;
	/* Inherit from video_device. May be overridden by the driver. */
	fh->ctrl_handler = vdev->ctrl_handler;
	INIT_LIST_HEAD(&fh->list);
	set_bit(V4L2_FL_USES_V4L2_FH, &fh->vdev->flags);
	/*
	 * determine_valid_ioctls() does not know if struct v4l2_fh
	 * is used by this driver, but here we do. So enable the
	 * prio ioctls here.
	 */
	set_bit(_IOC_NR(VIDIOC_G_PRIORITY), vdev->valid_ioctls);
	set_bit(_IOC_NR(VIDIOC_S_PRIORITY), vdev->valid_ioctls);
	fh->prio = V4L2_PRIORITY_UNSET;
	init_waitqueue_head(&fh->wait);
	INIT_LIST_HEAD(&fh->available);
	INIT_LIST_HEAD(&fh->subscribed);
	fh->sequence = -1;
}
EXPORT_SYMBOL_GPL(v4l2_fh_init);

void v4l2_fh_add(struct v4l2_fh *fh)
{
	unsigned long flags;

	v4l2_prio_open(fh->vdev->prio, &fh->prio);
	spin_lock_irqsave(&fh->vdev->fh_lock, flags);
	list_add(&fh->list, &fh->vdev->fh_list);
	spin_unlock_irqrestore(&fh->vdev->fh_lock, flags);
}
EXPORT_SYMBOL_GPL(v4l2_fh_add);

int v4l2_fh_open(struct file *filp)
{
	struct video_device *vdev = video_devdata(filp);
	struct v4l2_fh *fh = kzalloc(sizeof(*fh), GFP_KERNEL);

	filp->private_data = fh;
	if (fh == NULL)
		return -ENOMEM;
	v4l2_fh_init(fh, vdev);
	v4l2_fh_add(fh);
	return 0;
}
EXPORT_SYMBOL_GPL(v4l2_fh_open);

void v4l2_fh_del(struct v4l2_fh *fh)
{
	unsigned long flags;

	spin_lock_irqsave(&fh->vdev->fh_lock, flags);
	list_del_init(&fh->list);
	spin_unlock_irqrestore(&fh->vdev->fh_lock, flags);
	v4l2_prio_close(fh->vdev->prio, fh->prio);
}
EXPORT_SYMBOL_GPL(v4l2_fh_del);

void v4l2_fh_exit(struct v4l2_fh *fh)
{
	if (fh->vdev == NULL)
		return;
	v4l2_event_unsubscribe_all(fh);
	fh->vdev = NULL;
}
EXPORT_SYMBOL_GPL(v4l2_fh_exit);

int v4l2_fh_release(struct file *filp)
{
	struct v4l2_fh *fh = filp->private_data;

	if (fh) {
		v4l2_fh_del(fh);
		v4l2_fh_exit(fh);
		kfree(fh);
	}
	return 0;
}
EXPORT_SYMBOL_GPL(v4l2_fh_release);

int v4l2_fh_is_singular(struct v4l2_fh *fh)
{
	unsigned long flags;
	int is_singular;

	if (fh == NULL || fh->vdev == NULL)
		return 0;
	spin_lock_irqsave(&fh->vdev->fh_lock, flags);
	is_singular = list_is_singular(&fh->list);
	spin_unlock_irqrestore(&fh->vdev->fh_lock, flags);
	return is_singular;
}
EXPORT_SYMBOL_GPL(v4l2_fh_is_singular);
