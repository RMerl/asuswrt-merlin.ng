/*
 * Remote processor messaging - sample client driver
 *
 * Copyright (C) 2011 Texas Instruments, Inc.
 * Copyright (C) 2011 Google, Inc.
 *
 * Ohad Ben-Cohen <ohad@wizery.com>
 * Brian Swetland <swetland@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rpmsg.h>

#define MSG		"hello world!"
#define MSG_LIMIT	100

static void rpmsg_sample_cb(struct rpmsg_channel *rpdev, void *data, int len,
						void *priv, u32 src)
{
	int ret;
	static int rx_count;

	dev_info(&rpdev->dev, "incoming msg %d (src: 0x%x)\n", ++rx_count, src);

	print_hex_dump(KERN_DEBUG, __func__, DUMP_PREFIX_NONE, 16, 1,
		       data, len,  true);

	/* samples should not live forever */
	if (rx_count >= MSG_LIMIT) {
		dev_info(&rpdev->dev, "goodbye!\n");
		return;
	}

	/* send a new message now */
	ret = rpmsg_send(rpdev, MSG, strlen(MSG));
	if (ret)
		dev_err(&rpdev->dev, "rpmsg_send failed: %d\n", ret);
}

static int rpmsg_sample_probe(struct rpmsg_channel *rpdev)
{
	int ret;

	dev_info(&rpdev->dev, "new channel: 0x%x -> 0x%x!\n",
					rpdev->src, rpdev->dst);

	/* send a message to our remote processor */
	ret = rpmsg_send(rpdev, MSG, strlen(MSG));
	if (ret) {
		dev_err(&rpdev->dev, "rpmsg_send failed: %d\n", ret);
		return ret;
	}

	return 0;
}

static void rpmsg_sample_remove(struct rpmsg_channel *rpdev)
{
	dev_info(&rpdev->dev, "rpmsg sample client driver is removed\n");
}

static struct rpmsg_device_id rpmsg_driver_sample_id_table[] = {
	{ .name	= "rpmsg-client-sample" },
	{ },
};
MODULE_DEVICE_TABLE(rpmsg, rpmsg_driver_sample_id_table);

static struct rpmsg_driver rpmsg_sample_client = {
	.drv.name	= KBUILD_MODNAME,
	.drv.owner	= THIS_MODULE,
	.id_table	= rpmsg_driver_sample_id_table,
	.probe		= rpmsg_sample_probe,
	.callback	= rpmsg_sample_cb,
	.remove		= rpmsg_sample_remove,
};

static int __init rpmsg_client_sample_init(void)
{
	return register_rpmsg_driver(&rpmsg_sample_client);
}
module_init(rpmsg_client_sample_init);

static void __exit rpmsg_client_sample_fini(void)
{
	unregister_rpmsg_driver(&rpmsg_sample_client);
}
module_exit(rpmsg_client_sample_fini);

MODULE_DESCRIPTION("Remote processor messaging sample client driver");
MODULE_LICENSE("GPL v2");
