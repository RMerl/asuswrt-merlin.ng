/**
 * \file drm_fops.c
 * File operations for DRM
 *
 * \author Rickard E. (Rik) Faith <faith@valinux.com>
 * \author Daryll Strauss <daryll@valinux.com>
 * \author Gareth Hughes <gareth@valinux.com>
 */

/*
 * Created: Mon Jan  4 08:58:31 1999 by faith@valinux.com
 *
 * Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
 * Copyright 2000 VA Linux Systems, Inc., Sunnyvale, California.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <drm/drmP.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/module.h>
#include "drm_legacy.h"
#include "drm_internal.h"

/* from BKL pushdown */
DEFINE_MUTEX(drm_global_mutex);

static int drm_open_helper(struct file *filp, struct drm_minor *minor);

static int drm_setup(struct drm_device * dev)
{
	int ret;

	if (dev->driver->firstopen &&
	    !drm_core_check_feature(dev, DRIVER_MODESET)) {
		ret = dev->driver->firstopen(dev);
		if (ret != 0)
			return ret;
	}

	ret = drm_legacy_dma_setup(dev);
	if (ret < 0)
		return ret;


	DRM_DEBUG("\n");
	return 0;
}

/**
 * Open file.
 *
 * \param inode device inode
 * \param filp file pointer.
 * \return zero on success or a negative number on failure.
 *
 * Searches the DRM device with the same minor number, calls open_helper(), and
 * increments the device open count. If the open count was previous at zero,
 * i.e., it's the first that the device is open, then calls setup().
 */
int drm_open(struct inode *inode, struct file *filp)
{
	struct drm_device *dev;
	struct drm_minor *minor;
	int retcode;
	int need_setup = 0;

	minor = drm_minor_acquire(iminor(inode));
	if (IS_ERR(minor))
		return PTR_ERR(minor);

	dev = minor->dev;
	if (!dev->open_count++)
		need_setup = 1;

	/* share address_space across all char-devs of a single device */
	filp->f_mapping = dev->anon_inode->i_mapping;

	retcode = drm_open_helper(filp, minor);
	if (retcode)
		goto err_undo;
	if (need_setup) {
		retcode = drm_setup(dev);
		if (retcode)
			goto err_undo;
	}
	return 0;

err_undo:
	dev->open_count--;
	drm_minor_release(minor);
	return retcode;
}
EXPORT_SYMBOL(drm_open);

/**
 * Check whether DRI will run on this CPU.
 *
 * \return non-zero if the DRI will run on this CPU, or zero otherwise.
 */
static int drm_cpu_valid(void)
{
#if defined(__sparc__) && !defined(__sparc_v9__)
	return 0;		/* No cmpxchg before v9 sparc. */
#endif
	return 1;
}

/**
 * Called whenever a process opens /dev/drm.
 *
 * \param filp file pointer.
 * \param minor acquired minor-object.
 * \return zero on success or a negative number on failure.
 *
 * Creates and initializes a drm_file structure for the file private data in \p
 * filp and add it into the double linked list in \p dev.
 */
static int drm_open_helper(struct file *filp, struct drm_minor *minor)
{
	struct drm_device *dev = minor->dev;
	struct drm_file *priv;
	int ret;

	if (filp->f_flags & O_EXCL)
		return -EBUSY;	/* No exclusive opens */
	if (!drm_cpu_valid())
		return -EINVAL;
	if (dev->switch_power_state != DRM_SWITCH_POWER_ON && dev->switch_power_state != DRM_SWITCH_POWER_DYNAMIC_OFF)
		return -EINVAL;

	DRM_DEBUG("pid = %d, minor = %d\n", task_pid_nr(current), minor->index);

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	filp->private_data = priv;
	priv->filp = filp;
	priv->uid = current_euid();
	priv->pid = get_pid(task_pid(current));
	priv->minor = minor;

	/* for compatibility root is always authenticated */
	priv->authenticated = capable(CAP_SYS_ADMIN);
	priv->lock_count = 0;

	INIT_LIST_HEAD(&priv->lhead);
	INIT_LIST_HEAD(&priv->fbs);
	mutex_init(&priv->fbs_lock);
	INIT_LIST_HEAD(&priv->event_list);
	init_waitqueue_head(&priv->event_wait);
	priv->event_space = 4096; /* set aside 4k for event buffer */

	if (drm_core_check_feature(dev, DRIVER_GEM))
		drm_gem_open(dev, priv);

	if (drm_core_check_feature(dev, DRIVER_PRIME))
		drm_prime_init_file_private(&priv->prime);

	if (dev->driver->open) {
		ret = dev->driver->open(dev, priv);
		if (ret < 0)
			goto out_prime_destroy;
	}

	/* if there is no current master make this fd it, but do not create
	 * any master object for render clients */
	mutex_lock(&dev->master_mutex);
	if (drm_is_primary_client(priv) && !priv->minor->master) {
		/* create a new master */
		priv->minor->master = drm_master_create(priv->minor);
		if (!priv->minor->master) {
			ret = -ENOMEM;
			goto out_close;
		}

		priv->is_master = 1;
		/* take another reference for the copy in the local file priv */
		priv->master = drm_master_get(priv->minor->master);
		priv->authenticated = 1;

		if (dev->driver->master_create) {
			ret = dev->driver->master_create(dev, priv->master);
			if (ret) {
				/* drop both references if this fails */
				drm_master_put(&priv->minor->master);
				drm_master_put(&priv->master);
				goto out_close;
			}
		}
		if (dev->driver->master_set) {
			ret = dev->driver->master_set(dev, priv, true);
			if (ret) {
				/* drop both references if this fails */
				drm_master_put(&priv->minor->master);
				drm_master_put(&priv->master);
				goto out_close;
			}
		}
	} else if (drm_is_primary_client(priv)) {
		/* get a reference to the master */
		priv->master = drm_master_get(priv->minor->master);
	}
	mutex_unlock(&dev->master_mutex);

	mutex_lock(&dev->struct_mutex);
	list_add(&priv->lhead, &dev->filelist);
	mutex_unlock(&dev->struct_mutex);

#ifdef __alpha__
	/*
	 * Default the hose
	 */
	if (!dev->hose) {
		struct pci_dev *pci_dev;
		pci_dev = pci_get_class(PCI_CLASS_DISPLAY_VGA << 8, NULL);
		if (pci_dev) {
			dev->hose = pci_dev->sysdata;
			pci_dev_put(pci_dev);
		}
		if (!dev->hose) {
			struct pci_bus *b = list_entry(pci_root_buses.next,
				struct pci_bus, node);
			if (b)
				dev->hose = b->sysdata;
		}
	}
#endif

	return 0;

out_close:
	mutex_unlock(&dev->master_mutex);
	if (dev->driver->postclose)
		dev->driver->postclose(dev, priv);
out_prime_destroy:
	if (drm_core_check_feature(dev, DRIVER_PRIME))
		drm_prime_destroy_file_private(&priv->prime);
	if (drm_core_check_feature(dev, DRIVER_GEM))
		drm_gem_release(dev, priv);
	put_pid(priv->pid);
	kfree(priv);
	filp->private_data = NULL;
	return ret;
}

static void drm_master_release(struct drm_device *dev, struct file *filp)
{
	struct drm_file *file_priv = filp->private_data;

	if (drm_legacy_i_have_hw_lock(dev, file_priv)) {
		DRM_DEBUG("File %p released, freeing lock for context %d\n",
			  filp, _DRM_LOCKING_CONTEXT(file_priv->master->lock.hw_lock->lock));
		drm_legacy_lock_free(&file_priv->master->lock,
				     _DRM_LOCKING_CONTEXT(file_priv->master->lock.hw_lock->lock));
	}
}

static void drm_events_release(struct drm_file *file_priv)
{
	struct drm_device *dev = file_priv->minor->dev;
	struct drm_pending_event *e, *et;
	struct drm_pending_vblank_event *v, *vt;
	unsigned long flags;

	spin_lock_irqsave(&dev->event_lock, flags);

	/* Remove pending flips */
	list_for_each_entry_safe(v, vt, &dev->vblank_event_list, base.link)
		if (v->base.file_priv == file_priv) {
			list_del(&v->base.link);
			drm_vblank_put(dev, v->pipe);
			v->base.destroy(&v->base);
		}

	/* Remove unconsumed events */
	list_for_each_entry_safe(e, et, &file_priv->event_list, link) {
		list_del(&e->link);
		e->destroy(e);
	}

	spin_unlock_irqrestore(&dev->event_lock, flags);
}

/**
 * drm_legacy_dev_reinit
 *
 * Reinitializes a legacy/ums drm device in it's lastclose function.
 */
static void drm_legacy_dev_reinit(struct drm_device *dev)
{
	if (drm_core_check_feature(dev, DRIVER_MODESET))
		return;

	dev->sigdata.lock = NULL;

	dev->context_flag = 0;
	dev->last_context = 0;
	dev->if_version = 0;
}

/**
 * Take down the DRM device.
 *
 * \param dev DRM device structure.
 *
 * Frees every resource in \p dev.
 *
 * \sa drm_device
 */
int drm_lastclose(struct drm_device * dev)
{
	DRM_DEBUG("\n");

	if (dev->driver->lastclose)
		dev->driver->lastclose(dev);
	DRM_DEBUG("driver lastclose completed\n");

	if (dev->irq_enabled && !drm_core_check_feature(dev, DRIVER_MODESET))
		drm_irq_uninstall(dev);

	mutex_lock(&dev->struct_mutex);

	drm_agp_clear(dev);

	drm_legacy_sg_cleanup(dev);
	drm_legacy_vma_flush(dev);
	drm_legacy_dma_takedown(dev);

	mutex_unlock(&dev->struct_mutex);

	drm_legacy_dev_reinit(dev);

	DRM_DEBUG("lastclose completed\n");
	return 0;
}

/**
 * Release file.
 *
 * \param inode device inode
 * \param file_priv DRM file private.
 * \return zero on success or a negative number on failure.
 *
 * If the hardware lock is held then free it, and take it again for the kernel
 * context since it's necessary to reclaim buffers. Unlink the file private
 * data from its list and free it. Decreases the open count and if it reaches
 * zero calls drm_lastclose().
 */
int drm_release(struct inode *inode, struct file *filp)
{
	struct drm_file *file_priv = filp->private_data;
	struct drm_minor *minor = file_priv->minor;
	struct drm_device *dev = minor->dev;
	int retcode = 0;

	mutex_lock(&drm_global_mutex);

	DRM_DEBUG("open_count = %d\n", dev->open_count);

	mutex_lock(&dev->struct_mutex);
	list_del(&file_priv->lhead);
	mutex_unlock(&dev->struct_mutex);

	if (dev->driver->preclose)
		dev->driver->preclose(dev, file_priv);

	/* ========================================================
	 * Begin inline drm_release
	 */

	DRM_DEBUG("pid = %d, device = 0x%lx, open_count = %d\n",
		  task_pid_nr(current),
		  (long)old_encode_dev(file_priv->minor->kdev->devt),
		  dev->open_count);

	/* Release any auth tokens that might point to this file_priv,
	   (do that under the drm_global_mutex) */
	if (file_priv->magic)
		(void) drm_remove_magic(file_priv->master, file_priv->magic);

	/* if the master has gone away we can't do anything with the lock */
	if (file_priv->minor->master)
		drm_master_release(dev, filp);

	if (drm_core_check_feature(dev, DRIVER_HAVE_DMA))
		drm_legacy_reclaim_buffers(dev, file_priv);

	drm_events_release(file_priv);

	if (drm_core_check_feature(dev, DRIVER_MODESET))
		drm_fb_release(file_priv);

	if (drm_core_check_feature(dev, DRIVER_GEM))
		drm_gem_release(dev, file_priv);

	drm_legacy_ctxbitmap_flush(dev, file_priv);

	mutex_lock(&dev->master_mutex);

	if (file_priv->is_master) {
		struct drm_master *master = file_priv->master;

		/**
		 * Since the master is disappearing, so is the
		 * possibility to lock.
		 */
		mutex_lock(&dev->struct_mutex);
		if (master->lock.hw_lock) {
			if (dev->sigdata.lock == master->lock.hw_lock)
				dev->sigdata.lock = NULL;
			master->lock.hw_lock = NULL;
			master->lock.file_priv = NULL;
			wake_up_interruptible_all(&master->lock.lock_queue);
		}
		mutex_unlock(&dev->struct_mutex);

		if (file_priv->minor->master == file_priv->master) {
			/* drop the reference held my the minor */
			if (dev->driver->master_drop)
				dev->driver->master_drop(dev, file_priv, true);
			drm_master_put(&file_priv->minor->master);
		}
	}

	/* drop the master reference held by the file priv */
	if (file_priv->master)
		drm_master_put(&file_priv->master);
	file_priv->is_master = 0;
	mutex_unlock(&dev->master_mutex);

	if (dev->driver->postclose)
		dev->driver->postclose(dev, file_priv);


	if (drm_core_check_feature(dev, DRIVER_PRIME))
		drm_prime_destroy_file_private(&file_priv->prime);

	WARN_ON(!list_empty(&file_priv->event_list));

	put_pid(file_priv->pid);
	kfree(file_priv);

	/* ========================================================
	 * End inline drm_release
	 */

	if (!--dev->open_count) {
		retcode = drm_lastclose(dev);
		if (drm_device_is_unplugged(dev))
			drm_put_dev(dev);
	}
	mutex_unlock(&drm_global_mutex);

	drm_minor_release(minor);

	return retcode;
}
EXPORT_SYMBOL(drm_release);

ssize_t drm_read(struct file *filp, char __user *buffer,
		 size_t count, loff_t *offset)
{
	struct drm_file *file_priv = filp->private_data;
	struct drm_device *dev = file_priv->minor->dev;
	ssize_t ret = 0;

	if (!access_ok(VERIFY_WRITE, buffer, count))
		return -EFAULT;

	spin_lock_irq(&dev->event_lock);
	for (;;) {
		if (list_empty(&file_priv->event_list)) {
			if (ret)
				break;

			if (filp->f_flags & O_NONBLOCK) {
				ret = -EAGAIN;
				break;
			}

			spin_unlock_irq(&dev->event_lock);
			ret = wait_event_interruptible(file_priv->event_wait,
						       !list_empty(&file_priv->event_list));
			spin_lock_irq(&dev->event_lock);
			if (ret < 0)
				break;

			ret = 0;
		} else {
			struct drm_pending_event *e;

			e = list_first_entry(&file_priv->event_list,
					     struct drm_pending_event, link);
			if (e->event->length + ret > count)
				break;

			if (__copy_to_user_inatomic(buffer + ret,
						    e->event, e->event->length)) {
				if (ret == 0)
					ret = -EFAULT;
				break;
			}

			file_priv->event_space += e->event->length;
			ret += e->event->length;
			list_del(&e->link);
			e->destroy(e);
		}
	}
	spin_unlock_irq(&dev->event_lock);

	return ret;
}
EXPORT_SYMBOL(drm_read);

unsigned int drm_poll(struct file *filp, struct poll_table_struct *wait)
{
	struct drm_file *file_priv = filp->private_data;
	unsigned int mask = 0;

	poll_wait(filp, &file_priv->event_wait, wait);

	if (!list_empty(&file_priv->event_list))
		mask |= POLLIN | POLLRDNORM;

	return mask;
}
EXPORT_SYMBOL(drm_poll);
