 /****************************************************************************
 *
 * Broadcom Proprietary and Confidential. (c) 2017 Broadcom.  All rights reserved.
 * The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to
 * you under the terms of the GNU General Public License version 2 (the
 * "GPL"), available at [http://www.broadcom.com/licenses/GPLv2.php], with
 * the following added to such license:
 *
 * As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy
 * and distribute the resulting executable under terms of your choice,
 * provided that you also meet, for each linked independent module, the
 * terms and conditions of the license of that module. An independent
 * module is a module which is not derived from this software. The special
 * exception does not apply to any modifications of the software.
 *
 * Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a
 * license other than the GPL, without Broadcom's express prior written
 * consent.
 *
 ****************************************************************************/
 /*******************************************************************************
 *
 * itc_user.c
 * Peter Sulc
 *
 *******************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/compat.h>

#include <itc_rpc.h>
#include <itc_user.h>
#include <itc_msg_q.h>

/* a few tidbits for user space device access */
#define RPC_CLASS		"brpc"
#define RPC_MAX_DEVS		1
#define RPC_USER_POOL_MSG_CNT	32
#define RPC_USER_MSG_MAX	8

static struct class		*rpc_class;
static int			rpc_major;
static dev_t			rpc_dev;
static struct cdev		rpc_cdev;
static struct device		*rpc_device;

static rpc_queue_msg_pool	*user_pool = 0;

struct fuser_service {
	int		ref_cnt;
	rpc_queue	*queue;
	pid_t		pid;
};

static struct fuser_service user_services[RPC_MAX_SERVICES];


static int rpc_file_open(struct inode *inode,
			 struct file *file);
static int rpc_file_release(struct inode *inode,
			    struct file *file);
static long rpc_file_ioctl(struct file *file,
			   unsigned int cmd,
			   unsigned long arg);
#ifdef CONFIG_COMPAT
static long compat_rpc_file_ioctl(struct file *file,
			   unsigned int cmd,
			   unsigned long arg);
#endif
static ssize_t rpc_file_read(struct file *file,
			     char __user *buf,
			     size_t count,
			     loff_t *ppos);
static ssize_t rpc_file_write(struct file *file,
			      const char __user *buf,
			      size_t count,
			      loff_t *ppos);


static const struct file_operations rpc_fops = {
	.owner =		THIS_MODULE,
	.open =			rpc_file_open,
	.release =		rpc_file_release,
	.unlocked_ioctl =	rpc_file_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = 	compat_rpc_file_ioctl,
#endif
	.read =			rpc_file_read,
	.write =		rpc_file_write,
};



/******************************************************************
 * user space services function table.
 * I cannot use the built in demux in itc_rpc because I cannot call
 * a function in user space. Therefore, I must pass this table to all
 * user space services and do the function demux up in user space.
 ******************************************************************/
static int rpc_rx(int tunnel, rpc_msg *msg);
static rpc_function user_services_table[256] = { [0 ... 255] = { rpc_rx, 0 } };

/************************************************
 * user space device io
 ************************************************/

static int rpc_file_open(struct inode *inode, struct file *file)
{
	file->private_data = 0;
	return 0;
}

static int rpc_file_release(struct inode *inode, struct file *file)
{
	int service = (int)(uintptr_t)file->private_data;
	if (service > 0 && service < RPC_MAX_SERVICES) {
		if (user_services[service].ref_cnt > 0) {
			user_services[service].ref_cnt--;
			if (user_services[service].ref_cnt == 0) {
				rpc_queue *fq = user_services[service].queue;
				if (fq) {
					rpc_queue_msg *qmsg = rpc_try_remove_head_from_queue(fq);
					while (qmsg)	{
						rpc_queue_msg_pool_free(user_pool, qmsg);
						qmsg = rpc_try_remove_head_from_queue(fq);
					}
				}
			}
		}
	}
	return 0;
}

static ssize_t rpc_file_read(struct file *file,
			     char __user *buf,
			     size_t count,
			     loff_t *ppos)
{
	return 0;
}
static ssize_t rpc_file_write(struct file *file,
			      const char __user *buf,
			      size_t count,
			      loff_t *ppos)
{
	return 0;
}
static long rpc_file_ioctl(struct file *file,
			   unsigned int cmd,
			   unsigned long arg)
{
	int		service, status;
	rpc_queue	*fq;
	rpc_msg_user	umsg;

	if (cmd == ITC_IOCTL_SERVICE_REGISTER) {
		service = arg;
		if (service <= 0 || service >= RPC_MAX_SERVICES) {
			pr_err("%s: invalid service %d\n", __func__, service);
			return -EFAULT;
		}
		if (user_services[service].ref_cnt > 0) {
			pr_err("service %d is already registered by process %d\n",
			       service, user_services[service].pid);
			pr_err("%s returning %d\n", __func__, -EACCES);
			return -EACCES;
		}
		fq = user_services[service].queue;
		if (!fq) {
			fq = kmalloc(sizeof(rpc_queue), GFP_KERNEL);
			if (fq == NULL) {
				pr_err("%s: failed to allocate kernel memory\n", __func__);
				return -ENOMEM;
			}
			rpc_init_queue(fq);
			user_services[service].queue = fq;
			rpc_register_functions(service,
					       user_services_table,
					       sizeof(user_services_table)/sizeof(rpc_function));
		}
		file->private_data = (void *)(uintptr_t)service;
		user_services[service].ref_cnt++;
		user_services[service].pid = current->pid;
		return 0;
	}
	if (cmd == ITC_IOCTL_GET_TUNNEL_ID) {
		char tunnel_name[ITC_TUNNEL_NAME_MAX];
		if (copy_from_user(tunnel_name, (void __user *)arg, ITC_TUNNEL_NAME_MAX)) {
			pr_err("%s: copy_from_user failed\n", __func__);
			return -EFAULT;
		}
		return rpc_get_fifo_tunnel_id(tunnel_name);
	}
	if (cmd == ITC_IOCTL_SERVICE_LISTEN) {
		service = (int)(uintptr_t)file->private_data;
		if (service > 0 && service < RPC_MAX_SERVICES) {
			fq = user_services[service].queue;
			if (fq) {
				rpc_queue_msg *qmsg = rpc_remove_head_from_queue(fq);
				if (qmsg) {
					int ret;
					umsg.msg = qmsg->msg;
					umsg.tunnel = qmsg->tunnel;
					ret = copy_to_user((void *)arg, &umsg, sizeof(umsg));
					rpc_queue_msg_pool_free(user_pool, qmsg);
					return ret;
				}
				else {
					return -EINTR;	/* ctrl-c ? */
				}
			}
			pr_err("%s: no registered service\n", __func__);
			return -EFAULT;
		}
		pr_err("%s: invalid service\n", __func__);
		return -EFAULT;
	}
	if (cmd == ITC_IOCTL_SEND_REPLY) {
		if (copy_from_user(&umsg, (void __user *)arg, sizeof(umsg))) {
			pr_err("%s: copy_from_user failed\n", __func__);
			return -EFAULT;
		}
		rpc_send_reply(umsg.tunnel, &umsg.msg);
		return 0;
	}
	if (cmd == ITC_IOCTL_SEND_MESSAGE) {
		if (copy_from_user(&umsg, (void __user *)arg, sizeof(umsg))) {
			pr_err("%s: copy_from_user failed\n", __func__);
			return -EFAULT;
		}
		rpc_send_message(umsg.tunnel, &umsg.msg, true);
		return 0;
	}
	if (cmd == ITC_IOCTL_SEND_REQUEST) {
		if (copy_from_user(&umsg, (void __user *)arg, sizeof(umsg))) {
			pr_err("%s: copy_from_user failed\n", __func__);
			return -EFAULT;
		}
		status = rpc_send_request(umsg.tunnel, &umsg.msg);
		if (status)	{
			pr_err("%s: rpc_send_request status %d\n", __func__, status);
			return status;
		}
		if (copy_to_user((void __user *)arg, &umsg, sizeof(umsg))) {
			pr_err("%s: copy_to_user failed\n", __func__);
			return -EFAULT;
		}
		return 0;
	}
	if (cmd == ITC_IOCTL_SEND_REQUEST_TIMEOUT) {
		if (copy_from_user(&umsg, (void __user *)arg, sizeof(umsg))) {
			pr_err("%s: copy_from_user failed\n", __func__);
			return -EFAULT;
		}
		status = rpc_send_request_timeout(umsg.tunnel, &umsg.msg, umsg.timeout);
		if (status)	{
			pr_err("%s: rpc_send_request_timeout status %d\n", __func__, status);
			return status;
		}
		if (copy_to_user((void __user *)arg, &umsg, sizeof(umsg))) {
			pr_err("%s: copy_to_user failed\n", __func__);
			return -EFAULT;
		}
		return 0;
	}
	return -EFAULT;
}
#ifdef CONFIG_COMPAT
static long compat_rpc_file_ioctl(struct file *file,
			   unsigned int cmd,
			   unsigned long arg)
{
	unsigned long argp;
	if (cmd == ITC_IOCTL_SERVICE_REGISTER)
		argp = arg;
	else
		argp = (unsigned long)compat_ptr(arg);
	return rpc_file_ioctl(file, cmd, argp);
}
#endif
static int rpc_rx(int tunnel, rpc_msg *msg)
{
	int service = rpc_msg_service(msg);
	rpc_queue *fq = user_services[service].queue;
	if (fq) {
		if (fq->sema.count <= RPC_USER_MSG_MAX) {
			rpc_queue_msg *qmsg = rpc_queue_msg_pool_alloc(user_pool);
			memcpy(&qmsg->msg, msg, sizeof(rpc_msg));
			qmsg->tunnel = tunnel;
			rpc_add_to_queue_tail(fq, qmsg);
			return 0;
		}
		pr_err("%s: user queue for service %d is full\n", __func__, service);
		return -1;
	}
	pr_err("%s: no user queue for service %d\n", __func__, service);
	return -1;
}

void rpc_user_cleanup(void)
{
	if (rpc_class)
		class_destroy(rpc_class);

	if (rpc_major)
		unregister_chrdev_region(MKDEV(rpc_major, 0), RPC_MAX_DEVS);
}


int __init rpc_user_init(void)
{
	int status;

	if (user_pool) {
		pr_info("Benign Warning: Redundant rpc_init()\n");
		return 0;
	}
	user_pool = rpc_queue_msg_pool_create(RPC_USER_POOL_MSG_CNT);

	rpc_class = class_create(THIS_MODULE, RPC_CLASS);
	if (IS_ERR(rpc_class)) {
		pr_err("class_create() failed for rpc_class\n");
		return PTR_ERR(rpc_class);
	}
	status = alloc_chrdev_region(&rpc_dev, 0, RPC_MAX_DEVS, RPC_CLASS);
	rpc_major = MAJOR(rpc_dev);
	if (status < 0) {
		pr_err("%s: can't alloc chrdev region\n", __func__);
		class_destroy(rpc_class);
		return status;
	}
	cdev_init(&rpc_cdev, &rpc_fops);
	status = cdev_add(&rpc_cdev, rpc_dev, 1);
	if (status < 0) {
		pr_err("can't register major %d\n", rpc_major);
		return status;
	}
	rpc_device = device_create(rpc_class, NULL, MKDEV(rpc_major, 0), NULL, "brpc%d", 0);
	if (IS_ERR(rpc_device)) {
		pr_err("%s: can't register class device\n", __func__);
		rpc_device = NULL;
		return PTR_ERR(rpc_device);
	}
	return 0;
}

module_init(rpc_user_init);
module_exit(rpc_user_cleanup);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("rpc user space driver");
