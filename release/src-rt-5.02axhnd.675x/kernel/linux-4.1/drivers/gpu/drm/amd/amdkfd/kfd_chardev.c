/*
 * Copyright 2014 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <linux/device.h>
#include <linux/export.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/compat.h>
#include <uapi/linux/kfd_ioctl.h>
#include <linux/time.h>
#include <linux/mm.h>
#include <uapi/asm-generic/mman-common.h>
#include <asm/processor.h>
#include "kfd_priv.h"
#include "kfd_device_queue_manager.h"

static long kfd_ioctl(struct file *, unsigned int, unsigned long);
static int kfd_open(struct inode *, struct file *);
static int kfd_mmap(struct file *, struct vm_area_struct *);

static const char kfd_dev_name[] = "kfd";

static const struct file_operations kfd_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = kfd_ioctl,
	.compat_ioctl = kfd_ioctl,
	.open = kfd_open,
	.mmap = kfd_mmap,
};

static int kfd_char_dev_major = -1;
static struct class *kfd_class;
struct device *kfd_device;

int kfd_chardev_init(void)
{
	int err = 0;

	kfd_char_dev_major = register_chrdev(0, kfd_dev_name, &kfd_fops);
	err = kfd_char_dev_major;
	if (err < 0)
		goto err_register_chrdev;

	kfd_class = class_create(THIS_MODULE, kfd_dev_name);
	err = PTR_ERR(kfd_class);
	if (IS_ERR(kfd_class))
		goto err_class_create;

	kfd_device = device_create(kfd_class, NULL,
					MKDEV(kfd_char_dev_major, 0),
					NULL, kfd_dev_name);
	err = PTR_ERR(kfd_device);
	if (IS_ERR(kfd_device))
		goto err_device_create;

	return 0;

err_device_create:
	class_destroy(kfd_class);
err_class_create:
	unregister_chrdev(kfd_char_dev_major, kfd_dev_name);
err_register_chrdev:
	return err;
}

void kfd_chardev_exit(void)
{
	device_destroy(kfd_class, MKDEV(kfd_char_dev_major, 0));
	class_destroy(kfd_class);
	unregister_chrdev(kfd_char_dev_major, kfd_dev_name);
}

struct device *kfd_chardev(void)
{
	return kfd_device;
}


static int kfd_open(struct inode *inode, struct file *filep)
{
	struct kfd_process *process;
	bool is_32bit_user_mode;

	if (iminor(inode) != 0)
		return -ENODEV;

	is_32bit_user_mode = is_compat_task();

	if (is_32bit_user_mode == true) {
		dev_warn(kfd_device,
			"Process %d (32-bit) failed to open /dev/kfd\n"
			"32-bit processes are not supported by amdkfd\n",
			current->pid);
		return -EPERM;
	}

	process = kfd_create_process(current);
	if (IS_ERR(process))
		return PTR_ERR(process);

	dev_dbg(kfd_device, "process %d opened, compat mode (32 bit) - %d\n",
		process->pasid, process->is_32bit_user_mode);

	return 0;
}

static int kfd_ioctl_get_version(struct file *filep, struct kfd_process *p,
					void *data)
{
	struct kfd_ioctl_get_version_args *args = data;
	int err = 0;

	args->major_version = KFD_IOCTL_MAJOR_VERSION;
	args->minor_version = KFD_IOCTL_MINOR_VERSION;

	return err;
}

static int set_queue_properties_from_user(struct queue_properties *q_properties,
				struct kfd_ioctl_create_queue_args *args)
{
	if (args->queue_percentage > KFD_MAX_QUEUE_PERCENTAGE) {
		pr_err("kfd: queue percentage must be between 0 to KFD_MAX_QUEUE_PERCENTAGE\n");
		return -EINVAL;
	}

	if (args->queue_priority > KFD_MAX_QUEUE_PRIORITY) {
		pr_err("kfd: queue priority must be between 0 to KFD_MAX_QUEUE_PRIORITY\n");
		return -EINVAL;
	}

	if ((args->ring_base_address) &&
		(!access_ok(VERIFY_WRITE,
			(const void __user *) args->ring_base_address,
			sizeof(uint64_t)))) {
		pr_err("kfd: can't access ring base address\n");
		return -EFAULT;
	}

	if (!is_power_of_2(args->ring_size) && (args->ring_size != 0)) {
		pr_err("kfd: ring size must be a power of 2 or 0\n");
		return -EINVAL;
	}

	if (!access_ok(VERIFY_WRITE,
			(const void __user *) args->read_pointer_address,
			sizeof(uint32_t))) {
		pr_err("kfd: can't access read pointer\n");
		return -EFAULT;
	}

	if (!access_ok(VERIFY_WRITE,
			(const void __user *) args->write_pointer_address,
			sizeof(uint32_t))) {
		pr_err("kfd: can't access write pointer\n");
		return -EFAULT;
	}

	if (args->eop_buffer_address &&
		!access_ok(VERIFY_WRITE,
			(const void __user *) args->eop_buffer_address,
			sizeof(uint32_t))) {
		pr_debug("kfd: can't access eop buffer");
		return -EFAULT;
	}

	if (args->ctx_save_restore_address &&
		!access_ok(VERIFY_WRITE,
			(const void __user *) args->ctx_save_restore_address,
			sizeof(uint32_t))) {
		pr_debug("kfd: can't access ctx save restore buffer");
		return -EFAULT;
	}

	q_properties->is_interop = false;
	q_properties->queue_percent = args->queue_percentage;
	q_properties->priority = args->queue_priority;
	q_properties->queue_address = args->ring_base_address;
	q_properties->queue_size = args->ring_size;
	q_properties->read_ptr = (uint32_t *) args->read_pointer_address;
	q_properties->write_ptr = (uint32_t *) args->write_pointer_address;
	q_properties->eop_ring_buffer_address = args->eop_buffer_address;
	q_properties->eop_ring_buffer_size = args->eop_buffer_size;
	q_properties->ctx_save_restore_area_address =
			args->ctx_save_restore_address;
	q_properties->ctx_save_restore_area_size = args->ctx_save_restore_size;
	if (args->queue_type == KFD_IOC_QUEUE_TYPE_COMPUTE ||
		args->queue_type == KFD_IOC_QUEUE_TYPE_COMPUTE_AQL)
		q_properties->type = KFD_QUEUE_TYPE_COMPUTE;
	else if (args->queue_type == KFD_IOC_QUEUE_TYPE_SDMA)
		q_properties->type = KFD_QUEUE_TYPE_SDMA;
	else
		return -ENOTSUPP;

	if (args->queue_type == KFD_IOC_QUEUE_TYPE_COMPUTE_AQL)
		q_properties->format = KFD_QUEUE_FORMAT_AQL;
	else
		q_properties->format = KFD_QUEUE_FORMAT_PM4;

	pr_debug("Queue Percentage (%d, %d)\n",
			q_properties->queue_percent, args->queue_percentage);

	pr_debug("Queue Priority (%d, %d)\n",
			q_properties->priority, args->queue_priority);

	pr_debug("Queue Address (0x%llX, 0x%llX)\n",
			q_properties->queue_address, args->ring_base_address);

	pr_debug("Queue Size (0x%llX, %u)\n",
			q_properties->queue_size, args->ring_size);

	pr_debug("Queue r/w Pointers (0x%llX, 0x%llX)\n",
			(uint64_t) q_properties->read_ptr,
			(uint64_t) q_properties->write_ptr);

	pr_debug("Queue Format (%d)\n", q_properties->format);

	pr_debug("Queue EOP (0x%llX)\n", q_properties->eop_ring_buffer_address);

	pr_debug("Queue CTX save arex (0x%llX)\n",
			q_properties->ctx_save_restore_area_address);

	return 0;
}

static int kfd_ioctl_create_queue(struct file *filep, struct kfd_process *p,
					void *data)
{
	struct kfd_ioctl_create_queue_args *args = data;
	struct kfd_dev *dev;
	int err = 0;
	unsigned int queue_id;
	struct kfd_process_device *pdd;
	struct queue_properties q_properties;

	memset(&q_properties, 0, sizeof(struct queue_properties));

	pr_debug("kfd: creating queue ioctl\n");

	err = set_queue_properties_from_user(&q_properties, args);
	if (err)
		return err;

	pr_debug("kfd: looking for gpu id 0x%x\n", args->gpu_id);
	dev = kfd_device_by_id(args->gpu_id);
	if (dev == NULL) {
		pr_debug("kfd: gpu id 0x%x was not found\n", args->gpu_id);
		return -EINVAL;
	}

	mutex_lock(&p->mutex);

	pdd = kfd_bind_process_to_device(dev, p);
	if (IS_ERR(pdd)) {
		err = -ESRCH;
		goto err_bind_process;
	}

	pr_debug("kfd: creating queue for PASID %d on GPU 0x%x\n",
			p->pasid,
			dev->id);

	err = pqm_create_queue(&p->pqm, dev, filep, &q_properties,
				0, q_properties.type, &queue_id);
	if (err != 0)
		goto err_create_queue;

	args->queue_id = queue_id;

	/* Return gpu_id as doorbell offset for mmap usage */
	args->doorbell_offset = args->gpu_id << PAGE_SHIFT;

	mutex_unlock(&p->mutex);

	pr_debug("kfd: queue id %d was created successfully\n", args->queue_id);

	pr_debug("ring buffer address == 0x%016llX\n",
			args->ring_base_address);

	pr_debug("read ptr address    == 0x%016llX\n",
			args->read_pointer_address);

	pr_debug("write ptr address   == 0x%016llX\n",
			args->write_pointer_address);

	return 0;

err_create_queue:
err_bind_process:
	mutex_unlock(&p->mutex);
	return err;
}

static int kfd_ioctl_destroy_queue(struct file *filp, struct kfd_process *p,
					void *data)
{
	int retval;
	struct kfd_ioctl_destroy_queue_args *args = data;

	pr_debug("kfd: destroying queue id %d for PASID %d\n",
				args->queue_id,
				p->pasid);

	mutex_lock(&p->mutex);

	retval = pqm_destroy_queue(&p->pqm, args->queue_id);

	mutex_unlock(&p->mutex);
	return retval;
}

static int kfd_ioctl_update_queue(struct file *filp, struct kfd_process *p,
					void *data)
{
	int retval;
	struct kfd_ioctl_update_queue_args *args = data;
	struct queue_properties properties;

	if (args->queue_percentage > KFD_MAX_QUEUE_PERCENTAGE) {
		pr_err("kfd: queue percentage must be between 0 to KFD_MAX_QUEUE_PERCENTAGE\n");
		return -EINVAL;
	}

	if (args->queue_priority > KFD_MAX_QUEUE_PRIORITY) {
		pr_err("kfd: queue priority must be between 0 to KFD_MAX_QUEUE_PRIORITY\n");
		return -EINVAL;
	}

	if ((args->ring_base_address) &&
		(!access_ok(VERIFY_WRITE,
			(const void __user *) args->ring_base_address,
			sizeof(uint64_t)))) {
		pr_err("kfd: can't access ring base address\n");
		return -EFAULT;
	}

	if (!is_power_of_2(args->ring_size) && (args->ring_size != 0)) {
		pr_err("kfd: ring size must be a power of 2 or 0\n");
		return -EINVAL;
	}

	properties.queue_address = args->ring_base_address;
	properties.queue_size = args->ring_size;
	properties.queue_percent = args->queue_percentage;
	properties.priority = args->queue_priority;

	pr_debug("kfd: updating queue id %d for PASID %d\n",
			args->queue_id, p->pasid);

	mutex_lock(&p->mutex);

	retval = pqm_update_queue(&p->pqm, args->queue_id, &properties);

	mutex_unlock(&p->mutex);

	return retval;
}

static int kfd_ioctl_set_memory_policy(struct file *filep,
					struct kfd_process *p, void *data)
{
	struct kfd_ioctl_set_memory_policy_args *args = data;
	struct kfd_dev *dev;
	int err = 0;
	struct kfd_process_device *pdd;
	enum cache_policy default_policy, alternate_policy;

	if (args->default_policy != KFD_IOC_CACHE_POLICY_COHERENT
	    && args->default_policy != KFD_IOC_CACHE_POLICY_NONCOHERENT) {
		return -EINVAL;
	}

	if (args->alternate_policy != KFD_IOC_CACHE_POLICY_COHERENT
	    && args->alternate_policy != KFD_IOC_CACHE_POLICY_NONCOHERENT) {
		return -EINVAL;
	}

	dev = kfd_device_by_id(args->gpu_id);
	if (dev == NULL)
		return -EINVAL;

	mutex_lock(&p->mutex);

	pdd = kfd_bind_process_to_device(dev, p);
	if (IS_ERR(pdd)) {
		err = -ESRCH;
		goto out;
	}

	default_policy = (args->default_policy == KFD_IOC_CACHE_POLICY_COHERENT)
			 ? cache_policy_coherent : cache_policy_noncoherent;

	alternate_policy =
		(args->alternate_policy == KFD_IOC_CACHE_POLICY_COHERENT)
		   ? cache_policy_coherent : cache_policy_noncoherent;

	if (!dev->dqm->ops.set_cache_memory_policy(dev->dqm,
				&pdd->qpd,
				default_policy,
				alternate_policy,
				(void __user *)args->alternate_aperture_base,
				args->alternate_aperture_size))
		err = -EINVAL;

out:
	mutex_unlock(&p->mutex);

	return err;
}

static int kfd_ioctl_get_clock_counters(struct file *filep,
				struct kfd_process *p, void *data)
{
	struct kfd_ioctl_get_clock_counters_args *args = data;
	struct kfd_dev *dev;
	struct timespec64 time;

	dev = kfd_device_by_id(args->gpu_id);
	if (dev == NULL)
		return -EINVAL;

	/* Reading GPU clock counter from KGD */
	args->gpu_clock_counter =
		dev->kfd2kgd->get_gpu_clock_counter(dev->kgd);

	/* No access to rdtsc. Using raw monotonic time */
	getrawmonotonic64(&time);
	args->cpu_clock_counter = (uint64_t)timespec64_to_ns(&time);

	get_monotonic_boottime64(&time);
	args->system_clock_counter = (uint64_t)timespec64_to_ns(&time);

	/* Since the counter is in nano-seconds we use 1GHz frequency */
	args->system_clock_freq = 1000000000;

	return 0;
}


static int kfd_ioctl_get_process_apertures(struct file *filp,
				struct kfd_process *p, void *data)
{
	struct kfd_ioctl_get_process_apertures_args *args = data;
	struct kfd_process_device_apertures *pAperture;
	struct kfd_process_device *pdd;

	dev_dbg(kfd_device, "get apertures for PASID %d", p->pasid);

	args->num_of_nodes = 0;

	mutex_lock(&p->mutex);

	/*if the process-device list isn't empty*/
	if (kfd_has_process_device_data(p)) {
		/* Run over all pdd of the process */
		pdd = kfd_get_first_process_device_data(p);
		do {
			pAperture =
				&args->process_apertures[args->num_of_nodes];
			pAperture->gpu_id = pdd->dev->id;
			pAperture->lds_base = pdd->lds_base;
			pAperture->lds_limit = pdd->lds_limit;
			pAperture->gpuvm_base = pdd->gpuvm_base;
			pAperture->gpuvm_limit = pdd->gpuvm_limit;
			pAperture->scratch_base = pdd->scratch_base;
			pAperture->scratch_limit = pdd->scratch_limit;

			dev_dbg(kfd_device,
				"node id %u\n", args->num_of_nodes);
			dev_dbg(kfd_device,
				"gpu id %u\n", pdd->dev->id);
			dev_dbg(kfd_device,
				"lds_base %llX\n", pdd->lds_base);
			dev_dbg(kfd_device,
				"lds_limit %llX\n", pdd->lds_limit);
			dev_dbg(kfd_device,
				"gpuvm_base %llX\n", pdd->gpuvm_base);
			dev_dbg(kfd_device,
				"gpuvm_limit %llX\n", pdd->gpuvm_limit);
			dev_dbg(kfd_device,
				"scratch_base %llX\n", pdd->scratch_base);
			dev_dbg(kfd_device,
				"scratch_limit %llX\n", pdd->scratch_limit);

			args->num_of_nodes++;
		} while ((pdd = kfd_get_next_process_device_data(p, pdd)) != NULL &&
				(args->num_of_nodes < NUM_OF_SUPPORTED_GPUS));
	}

	mutex_unlock(&p->mutex);

	return 0;
}

#define AMDKFD_IOCTL_DEF(ioctl, _func, _flags) \
	[_IOC_NR(ioctl)] = {.cmd = ioctl, .func = _func, .flags = _flags, .cmd_drv = 0, .name = #ioctl}

/** Ioctl table */
static const struct amdkfd_ioctl_desc amdkfd_ioctls[] = {
	AMDKFD_IOCTL_DEF(AMDKFD_IOC_GET_VERSION,
			kfd_ioctl_get_version, 0),

	AMDKFD_IOCTL_DEF(AMDKFD_IOC_CREATE_QUEUE,
			kfd_ioctl_create_queue, 0),

	AMDKFD_IOCTL_DEF(AMDKFD_IOC_DESTROY_QUEUE,
			kfd_ioctl_destroy_queue, 0),

	AMDKFD_IOCTL_DEF(AMDKFD_IOC_SET_MEMORY_POLICY,
			kfd_ioctl_set_memory_policy, 0),

	AMDKFD_IOCTL_DEF(AMDKFD_IOC_GET_CLOCK_COUNTERS,
			kfd_ioctl_get_clock_counters, 0),

	AMDKFD_IOCTL_DEF(AMDKFD_IOC_GET_PROCESS_APERTURES,
			kfd_ioctl_get_process_apertures, 0),

	AMDKFD_IOCTL_DEF(AMDKFD_IOC_UPDATE_QUEUE,
			kfd_ioctl_update_queue, 0),
};

#define AMDKFD_CORE_IOCTL_COUNT	ARRAY_SIZE(amdkfd_ioctls)

static long kfd_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	struct kfd_process *process;
	amdkfd_ioctl_t *func;
	const struct amdkfd_ioctl_desc *ioctl = NULL;
	unsigned int nr = _IOC_NR(cmd);
	char stack_kdata[128];
	char *kdata = NULL;
	unsigned int usize, asize;
	int retcode = -EINVAL;

	if (nr >= AMDKFD_CORE_IOCTL_COUNT)
		goto err_i1;

	if ((nr >= AMDKFD_COMMAND_START) && (nr < AMDKFD_COMMAND_END)) {
		u32 amdkfd_size;

		ioctl = &amdkfd_ioctls[nr];

		amdkfd_size = _IOC_SIZE(ioctl->cmd);
		usize = asize = _IOC_SIZE(cmd);
		if (amdkfd_size > asize)
			asize = amdkfd_size;

		cmd = ioctl->cmd;
	} else
		goto err_i1;

	dev_dbg(kfd_device, "ioctl cmd 0x%x (#%d), arg 0x%lx\n", cmd, nr, arg);

	process = kfd_get_process(current);
	if (IS_ERR(process)) {
		dev_dbg(kfd_device, "no process\n");
		goto err_i1;
	}

	/* Do not trust userspace, use our own definition */
	func = ioctl->func;

	if (unlikely(!func)) {
		dev_dbg(kfd_device, "no function\n");
		retcode = -EINVAL;
		goto err_i1;
	}

	if (cmd & (IOC_IN | IOC_OUT)) {
		if (asize <= sizeof(stack_kdata)) {
			kdata = stack_kdata;
		} else {
			kdata = kmalloc(asize, GFP_KERNEL);
			if (!kdata) {
				retcode = -ENOMEM;
				goto err_i1;
			}
		}
		if (asize > usize)
			memset(kdata + usize, 0, asize - usize);
	}

	if (cmd & IOC_IN) {
		if (copy_from_user(kdata, (void __user *)arg, usize) != 0) {
			retcode = -EFAULT;
			goto err_i1;
		}
	} else if (cmd & IOC_OUT) {
		memset(kdata, 0, usize);
	}

	retcode = func(filep, process, kdata);

	if (cmd & IOC_OUT)
		if (copy_to_user((void __user *)arg, kdata, usize) != 0)
			retcode = -EFAULT;

err_i1:
	if (!ioctl)
		dev_dbg(kfd_device, "invalid ioctl: pid=%d, cmd=0x%02x, nr=0x%02x\n",
			  task_pid_nr(current), cmd, nr);

	if (kdata != stack_kdata)
		kfree(kdata);

	if (retcode)
		dev_dbg(kfd_device, "ret = %d\n", retcode);

	return retcode;
}

static int kfd_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct kfd_process *process;

	process = kfd_get_process(current);
	if (IS_ERR(process))
		return PTR_ERR(process);

	return kfd_doorbell_mmap(process, vma);
}
