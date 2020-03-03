/*
 * vivid-radio-tx.c - radio transmitter support functions.
 *
 * Copyright 2014 Cisco Systems, Inc. and/or its affiliates. All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <linux/v4l2-dv-timings.h>
#include <media/v4l2-common.h>
#include <media/v4l2-event.h>
#include <media/v4l2-dv-timings.h>

#include "vivid-core.h"
#include "vivid-ctrls.h"
#include "vivid-radio-common.h"
#include "vivid-radio-tx.h"

ssize_t vivid_radio_tx_write(struct file *file, const char __user *buf,
			  size_t size, loff_t *offset)
{
	struct vivid_dev *dev = video_drvdata(file);
	struct v4l2_rds_data *data = dev->rds_gen.data;
	struct timespec ts;
	unsigned blk;
	int i;

	if (dev->radio_tx_rds_controls)
		return -EINVAL;

	if (size < sizeof(*data))
		return -EINVAL;
	size = sizeof(*data) * (size / sizeof(*data));

	if (mutex_lock_interruptible(&dev->mutex))
		return -ERESTARTSYS;
	if (dev->radio_tx_rds_owner &&
	    file->private_data != dev->radio_tx_rds_owner) {
		mutex_unlock(&dev->mutex);
		return -EBUSY;
	}
	dev->radio_tx_rds_owner = file->private_data;

retry:
	ktime_get_ts(&ts);
	ts = timespec_sub(ts, dev->radio_rds_init_ts);
	blk = ts.tv_sec * 100 + ts.tv_nsec / 10000000;
	blk = (blk * VIVID_RDS_GEN_BLOCKS) / 500;
	if (blk - VIVID_RDS_GEN_BLOCKS >= dev->radio_tx_rds_last_block)
		dev->radio_tx_rds_last_block = blk - VIVID_RDS_GEN_BLOCKS + 1;

	/*
	 * No data is available if there hasn't been time to get new data,
	 * or if the RDS receiver has been disabled, or if we use the data
	 * from the RDS transmitter and that RDS transmitter has been disabled,
	 * or if the signal quality is too weak.
	 */
	if (blk == dev->radio_tx_rds_last_block ||
	    !(dev->radio_tx_subchans & V4L2_TUNER_SUB_RDS)) {
		mutex_unlock(&dev->mutex);
		if (file->f_flags & O_NONBLOCK)
			return -EWOULDBLOCK;
		if (msleep_interruptible(20) && signal_pending(current))
			return -EINTR;
		if (mutex_lock_interruptible(&dev->mutex))
			return -ERESTARTSYS;
		goto retry;
	}

	for (i = 0; i < size && blk > dev->radio_tx_rds_last_block;
			dev->radio_tx_rds_last_block++) {
		unsigned data_blk = dev->radio_tx_rds_last_block % VIVID_RDS_GEN_BLOCKS;
		struct v4l2_rds_data rds;

		if (copy_from_user(&rds, buf + i, sizeof(rds))) {
			i = -EFAULT;
			break;
		}
		i += sizeof(rds);
		if (!dev->radio_rds_loop)
			continue;
		if ((rds.block & V4L2_RDS_BLOCK_MSK) == V4L2_RDS_BLOCK_INVALID ||
		    (rds.block & V4L2_RDS_BLOCK_ERROR))
			continue;
		rds.block &= V4L2_RDS_BLOCK_MSK;
		data[data_blk] = rds;
	}
	mutex_unlock(&dev->mutex);
	return i;
}

unsigned int vivid_radio_tx_poll(struct file *file, struct poll_table_struct *wait)
{
	return POLLOUT | POLLWRNORM | v4l2_ctrl_poll(file, wait);
}

int vidioc_g_modulator(struct file *file, void *fh, struct v4l2_modulator *a)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (a->index > 0)
		return -EINVAL;

	strlcpy(a->name, "AM/FM/SW Transmitter", sizeof(a->name));
	a->capability = V4L2_TUNER_CAP_LOW | V4L2_TUNER_CAP_STEREO |
			V4L2_TUNER_CAP_FREQ_BANDS | V4L2_TUNER_CAP_RDS |
			(dev->radio_tx_rds_controls ?
				V4L2_TUNER_CAP_RDS_CONTROLS :
				V4L2_TUNER_CAP_RDS_BLOCK_IO);
	a->rangelow = AM_FREQ_RANGE_LOW;
	a->rangehigh = FM_FREQ_RANGE_HIGH;
	a->txsubchans = dev->radio_tx_subchans;
	return 0;
}

int vidioc_s_modulator(struct file *file, void *fh, const struct v4l2_modulator *a)
{
	struct vivid_dev *dev = video_drvdata(file);

	if (a->index)
		return -EINVAL;
	if (a->txsubchans & ~0x13)
		return -EINVAL;
	dev->radio_tx_subchans = a->txsubchans;
	return 0;
}
