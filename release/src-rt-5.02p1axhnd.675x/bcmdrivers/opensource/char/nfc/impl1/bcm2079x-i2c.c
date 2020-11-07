/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/poll.h>
#include <linux/seq_file.h>
#include <linux/delay.h>

#include "bcm2079x.h"

#include <board.h>
#include <bcm_intr.h>
#include <bcm_pinmux.h>
#include <bcm_map_part.h>
#include <linux/platform_device.h>

// do not change below
#define MAX_BUFFER_SIZE		780

/* Read data */
#define PACKET_HEADER_SIZE_NCI	(4)
#define PACKET_HEADER_SIZE_HCI	(3)
#define PACKET_TYPE_NCI		(0x10)
#define PACKET_TYPE_HCIEV	(0x04)
#define MAX_PACKET_SIZE		(PACKET_HEADER_SIZE_NCI + 255)

struct bcm2079x_platform_data {
	unsigned short irq_gpio;	// gpio for external irq
	unsigned short power_gpio;	// board-power gpio
	unsigned short wake_gpio;	// board-wake gpio
	unsigned short scl_pin, scl_dir;// i2c serial clock
	unsigned short sda_pin, sda_dir;// i2c serial data
};

struct bcm2079x_dev {
	wait_queue_head_t read_wq;	// poll/read wait_queue
	struct mutex read_mutex;	// i2c read/write mutex
	struct i2c_client *client;
	struct miscdevice bcm2079x_device;
	struct bcm2079x_platform_data pdata;
	bool irq_enabled;
};

static void bcm2079x_disable_irq(struct bcm2079x_dev *bcm2079x_dev)
{
	if (bcm2079x_dev->irq_enabled) {
		disable_irq_nosync(bcm2079x_dev->client->irq);
		bcm2079x_dev->irq_enabled = false;
	}
}

static void bcm2079x_enable_irq(struct bcm2079x_dev *bcm2079x_dev)
{
	if (!bcm2079x_dev->irq_enabled) {
		enable_irq(bcm2079x_dev->client->irq);
		bcm2079x_dev->irq_enabled = true;
	}
}

static void bcm2079x_mask_irq(struct bcm2079x_dev *bcm2079x_dev)
{
	BcmHalExternalIrqMask(bcm2079x_dev->client->irq);
}

static void bcm2079x_unmask_irq(struct bcm2079x_dev *bcm2079x_dev)
{
	BcmHalExternalIrqUnmask(bcm2079x_dev->client->irq);
}

static int bcm2079x_gpio_set(unsigned gpio, GPIO_STATE_t state) {
	if (gpio == BP_NOT_DEFINED)
		return -ENOTSUPP;
	kerSysSetGpioState(gpio, state);
	return 0;
}

static void set_client_addr(struct bcm2079x_dev *bcm2079x_dev, int addr)
{
	struct i2c_client *client = bcm2079x_dev->client;

	dev_info(&bcm2079x_dev->client->dev,
		"Set client device address from %#x flag = %02x, to %#x\n",
		client->addr, client->flags, addr);
	client->addr = addr;
	if (addr < 0x80)
		client->flags &= ~I2C_CLIENT_TEN;
	else
		// XXX only if driver supports 10-bit address feature?
		client->flags |= I2C_CLIENT_TEN;
}

static irqreturn_t bcm2079x_dev_irq_handler(int irq, void *dev_id)
{
	struct bcm2079x_dev *bcm2079x_dev = dev_id;
	int active = kerSysGetGpioValue(bcm2079x_dev->pdata.irq_gpio);

	if (active) {
		// Mask until read syscall
		// poll/read will check active before sleeping
		bcm2079x_mask_irq(bcm2079x_dev);
		// Wake up sleeping readers
		wake_up(&bcm2079x_dev->read_wq);
	}

	return IRQ_HANDLED;
}

static unsigned int bcm2079x_dev_poll(struct file *filp, poll_table *wait)
{
	struct bcm2079x_dev *bcm2079x_dev = filp->private_data;

	// nothing available to read?
	if (!kerSysGetGpioValue(bcm2079x_dev->pdata.irq_gpio))
		poll_wait(filp, &bcm2079x_dev->read_wq, wait);

	// still nothing available to read?
	if (!kerSysGetGpioValue(bcm2079x_dev->pdata.irq_gpio))
		return 0;
	return POLLIN | POLLRDNORM;
}

// blocking read returning one datagram at a time
static ssize_t bcm2079x_dev_read(struct file *filp, char __user *buf,
				 size_t count, loff_t *offset)
{
	struct bcm2079x_dev *bcm2079x_dev = filp->private_data;
	unsigned char kern_buf[MAX_BUFFER_SIZE];
	int max_len = 4 + 255, pkt_len, rc;

	// wait until something available to read
	rc = wait_event_interruptible(bcm2079x_dev->read_wq,
		kerSysGetGpioValue(bcm2079x_dev->pdata.irq_gpio));
	if (rc) return rc;

	// acquire mutex for access to i2c bus
	rc = mutex_lock_interruptible(&bcm2079x_dev->read_mutex);
	if (rc) return rc;

	// bcm63000_xfer fifo limits "packets" length for hw i2c
	if (max_len > count) max_len = count;
	rc = i2c_master_recv(bcm2079x_dev->client, kern_buf, max_len);

	mutex_unlock(&bcm2079x_dev->read_mutex);
	if (rc < 0) return rc;

	// re-enable interrupt
	bcm2079x_unmask_irq(bcm2079x_dev);

	// validate packet header byte
	switch (kern_buf[0]) {
	case PACKET_TYPE_NCI:
		pkt_len = PACKET_HEADER_SIZE_NCI + kern_buf[PACKET_HEADER_SIZE_NCI - 1];
		break;
	case PACKET_TYPE_HCIEV:
		pkt_len = PACKET_HEADER_SIZE_HCI + kern_buf[PACKET_HEADER_SIZE_HCI - 1];
		break;
	default:
		pkt_len = max_len;
	}

	// packet too large?
	if (pkt_len > max_len) return -EFBIG;

	// buffer too small?
	if (pkt_len > count) return -ENOSPC;

	if (copy_to_user(buf, kern_buf, pkt_len)) {
		pr_debug("failed to copy to user %d\n", pkt_len);
		return -EFAULT;
	}

	return pkt_len;
}

static ssize_t bcm2079x_dev_write(struct file *filp, const char __user *buf,
				  size_t count, loff_t *offset)
{
	struct bcm2079x_dev *bcm2079x_dev = filp->private_data;
	char tmp[MAX_BUFFER_SIZE];
	int rc;

	if (count > MAX_BUFFER_SIZE) {
		static int warned;
		if (!warned) {
			dev_err(&bcm2079x_dev->client->dev,
				"out of memory\n");
			warned++;
		}
		return -ENOMEM;
	}

	if (copy_from_user(tmp, buf, count)) {
		pr_debug("failed to copy from user space\n");
		return -EFAULT;
	}

	// acquire mutex for access to i2c bus
	rc = mutex_lock_interruptible(&bcm2079x_dev->read_mutex);
	if (rc) return rc;

	// Write data
	rc = i2c_master_send(bcm2079x_dev->client, tmp, count);

	mutex_unlock(&bcm2079x_dev->read_mutex);

	if (rc != count) {
		pr_debug("write %d failed %d\n", count, rc);
		rc = -EIO;
	}

	return rc;
}

static int bcm2079x_dev_open(struct inode *inode, struct file *filp)
{
	struct bcm2079x_dev *bcm2079x_dev =
		container_of(filp->private_data, struct bcm2079x_dev, bcm2079x_device);

	filp->private_data = bcm2079x_dev;

	bcm2079x_enable_irq(bcm2079x_dev);
	bcm2079x_unmask_irq(bcm2079x_dev);

	// disable sleep (!wake) and enable power
	bcm2079x_gpio_set(bcm2079x_dev->pdata.wake_gpio, kGpioInactive);
	bcm2079x_gpio_set(bcm2079x_dev->pdata.power_gpio, kGpioActive);
	return 0;
}

static int bcm2079x_dev_close(struct inode *inode, struct file *filp)
{
	struct bcm2079x_dev *bcm2079x_dev = filp->private_data;

	bcm2079x_mask_irq(bcm2079x_dev);
	bcm2079x_disable_irq(bcm2079x_dev);
	return 0;
}

static long bcm2079x_dev_unlocked_ioctl(struct file *filp,
					unsigned int cmd, unsigned long arg)
{
	struct bcm2079x_dev *bcm2079x_dev = filp->private_data;

	switch (cmd) {
	case BCMNFC_POWER_CTL:
		return bcm2079x_gpio_set(bcm2079x_dev->pdata.power_gpio, arg);
	case BCMNFC_WAKE_CTL:
		return bcm2079x_gpio_set(bcm2079x_dev->pdata.wake_gpio, arg);
	case BCMNFC_SET_ADDR:
		set_client_addr(bcm2079x_dev, arg);
		return 0;
	case 0x5401: case 0x5402: case 0x540b:
		return -ENOTTY; // ignore TCGETA/TCSETA
	}

	dev_err(&bcm2079x_dev->client->dev,
		"unknown cmd (%x, %lx)\n", cmd, arg);
	return -ENOSYS;
}

static const struct i2c_device_id bcm2079x_id[] = {
	{ "bcm2079x-i2c", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, bcm2079x_id);

static const struct file_operations bcm2079x_dev_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.poll		= bcm2079x_dev_poll,
	.read		= bcm2079x_dev_read,
	.write		= bcm2079x_dev_write,
	.open		= bcm2079x_dev_open,
	.release	= bcm2079x_dev_close,
	.unlocked_ioctl	= bcm2079x_dev_unlocked_ioctl,
};

static struct bcm2079x_platform_data bcm2079x_pdata;

static struct i2c_board_info __initdata i2c_bcm2079x[] = {
	{
		.type	= "bcm2079x-i2c",
		.flags	= 0, /*I2C_CLIENT_TEN*/
		.irq	= 0, /*INTERRUPT_ID_EXTERNAL_X*/
		.platform_data	= &bcm2079x_pdata,
	},
};

static struct i2c_client *bcm2079x_i2c_client;

static int bcm2079x_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct bcm2079x_platform_data *platform_data;
	struct bcm2079x_dev *bcm2079x_dev;
	int rc;

	platform_data = client->dev.platform_data;

	if (platform_data == NULL) {
		dev_err(&client->dev,
			"nfc probe fail\n");
		return -ENODEV;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev,
			"need I2C_FUNC_I2C\n");
		return -ENODEV;
	}

	bcm2079x_dev = kzalloc(sizeof *bcm2079x_dev, GFP_KERNEL);
	if (bcm2079x_dev == NULL) {
		dev_err(&client->dev,
			"failed to allocate memory for module data\n");
		rc = -ENOMEM;
		goto err_exit;
	}

	bcm2079x_dev->pdata = *platform_data;
	bcm2079x_dev->client = client;

	/* init mutex and queues */
	init_waitqueue_head(&bcm2079x_dev->read_wq);
	mutex_init(&bcm2079x_dev->read_mutex);

	bcm2079x_dev->bcm2079x_device.minor = 10; /* MISC_DYNAMIC_MINOR */
	bcm2079x_dev->bcm2079x_device.name = bcm2079x_id[0].name;
	bcm2079x_dev->bcm2079x_device.fops = &bcm2079x_dev_fops;

	rc = misc_register(&bcm2079x_dev->bcm2079x_device);
	if (rc) {
		dev_err(&client->dev, "misc_register failed\n");
		goto err_misc_register;
	}

	dev_info(&client->dev,
		"registered with minor %d\n", bcm2079x_dev->bcm2079x_device.minor);

	/* request irq.  the irq is set whenever the chip has data available
	 * for reading.  it is cleared when all data has been read.
	 */
	dev_info(&client->dev,
		"requesting IRQ %d\n", client->irq);
	bcm2079x_dev->irq_enabled = true;

	rc = BcmHalMapInterruptEx((FN_HANDLER)bcm2079x_dev_irq_handler,
		(void*)bcm2079x_dev, client->irq, bcm2079x_id[0].name,
		INTR_REARM_YES, INTR_AFFINITY_DEFAULT);
	if (rc) {
		dev_err(&client->dev, "request_irq failed\n");
		goto err_request_irq_failed;
	}

	bcm2079x_disable_irq(bcm2079x_dev);
	i2c_set_clientdata(client, bcm2079x_dev);
	return 0;

err_request_irq_failed:
	misc_deregister(&bcm2079x_dev->bcm2079x_device);
err_misc_register:
	mutex_destroy(&bcm2079x_dev->read_mutex);
	kfree(bcm2079x_dev);
err_exit:
	return rc;
}

static int bcm2079x_remove(struct i2c_client *client)
{
	struct bcm2079x_dev *bcm2079x_dev;

	bcm2079x_dev = i2c_get_clientdata(client);
	free_irq(client->irq, bcm2079x_dev);
	misc_deregister(&bcm2079x_dev->bcm2079x_device);
	mutex_destroy(&bcm2079x_dev->read_mutex);
	kfree(bcm2079x_dev);

	return 0;
}

// change scl direction if different
static void bb2079x_setscl_dir(struct bcm2079x_platform_data *pdata, int dir)
{
	if (pdata->scl_dir != dir) {
		if ((pdata->scl_dir = dir) != 0)
			kerSysSetGpioDir(pdata->scl_pin);
		else
			kerSysSetGpioDirInput(pdata->scl_pin);
	}
}

static void bb2079x_setscl(void *data, int state)
{
	struct bcm2079x_platform_data *pdata = data;
#if 1
	bb2079x_setscl_dir(pdata, 1);
	kerSysSetGpioState(pdata->scl_pin, state ? kGpioActive : kGpioInactive);
#else
	// set high by changing pin to input with pullup
	if (state) {
		bb2079x_setscl_dir(pdata, 0);
	} else {
		bb2079x_setscl_dir(pdata, 1);
		kerSysSetGpioState(pdata->scl_pin, kGpioInactive);
	}
#endif
}

static int bb2079x_getscl(void *data)
{
	struct bcm2079x_platform_data *pdata = data;
	return kerSysGetGpioValue(pdata->scl_pin);
}

// change sda direction if different
static void bb2079x_setsda_dir(struct bcm2079x_platform_data *pdata, int dir)
{
	if (pdata->sda_dir != dir) {
		if ((pdata->sda_dir = dir) != 0)
			kerSysSetGpioDir(pdata->sda_pin);
		else
			kerSysSetGpioDirInput(pdata->sda_pin);
	}
}

static void bb2079x_setsda(void *data, int state)
{
	struct bcm2079x_platform_data *pdata = data;
#if 0
	bb2079x_setsda_dir(pdata, 1);
	kerSysSetGpioState(pdata->sda_pin, state ? kGpioActive : kGpioInactive);
#else
	// set high by changing pin to input with pullup
	if (state) {
		bb2079x_setsda_dir(pdata, 0);
	} else {
		bb2079x_setsda_dir(pdata, 1);
		kerSysSetGpioState(pdata->sda_pin, kGpioInactive);
	}
#endif
}

static int bb2079x_getsda(void *data)
{
	struct bcm2079x_platform_data *pdata = data;
	bb2079x_setsda_dir(pdata, 0);
	return kerSysGetGpioValue(pdata->sda_pin);
}

static struct i2c_algo_bit_data bb2079x_data = {
	.data		= &bcm2079x_pdata,
	.setscl		= bb2079x_setscl,
	.getscl		= bb2079x_getscl,
	.setsda		= bb2079x_setsda,
	.getsda		= bb2079x_getsda,
	.timeout	= HZ / 10,
	.udelay		= 5,
};

static struct i2c_adapter bb2079x_adap = {
	.owner		= THIS_MODULE,
	.name		= "bcm2079x bitbang i2c",
	.algo_data	= &bb2079x_data,
};

static int bcm2079x_initseq(struct i2c_adapter *adap, unsigned short addr)
{
//	static char init[] = { 0x10, 0x20, 0x00, 0x01 };
	char reply[8];
	struct i2c_msg msg[] = {
//		{	.addr	= addr,
//			.buf	= init,
//			.len	= sizeof init,
//			.flags	= 0,
//		},
		{	.addr	= addr,
			.buf	= reply,
			.len	= sizeof reply,
			.flags	= I2C_M_RD,
		},
	};
	int err;

	err = i2c_transfer(adap, msg, ARRAY_SIZE(msg));
	if (err <= 0) {
		pr_err("%s addr %x, err %d\n", __func__, addr, err);
		return 0;
	}
	pr_err("%s addr %x, reply %08x%08x\n", __func__, addr,
		((unsigned *)reply)[0], ((unsigned *)reply)[1]);
	return err == ARRAY_SIZE(msg);
}

/*
 * module load/unload record keeping
 */

static struct i2c_driver bcm2079x_driver = {
	.id_table = bcm2079x_id,
	.probe	= bcm2079x_probe,
	.remove = bcm2079x_remove,
	.driver = {
		.owner	= THIS_MODULE,
		.name	= bcm2079x_id[0].name,
	},
};

static int __init bcm2079x_dev_init(void)
{
	struct i2c_adapter *adap;
	unsigned short ext_irq;
	int busnum = 0; // CHECKME: Hardcoded value
	int err;

	// must have an interrupt pin defined
	if (BpGetNfcExtIntr(&ext_irq) != BP_SUCCESS) {
		pr_err("failed to get nfc interrupt\n");
		return -ENOENT;
	}

	ext_irq &= ~BP_EXT_INTR_FLAGS_MASK;
	i2c_bcm2079x->irq = INTERRUPT_ID_EXTERNAL_0 + ext_irq;
	bcm2079x_pdata.irq_gpio = MAP_EXT_IRQ_TO_GPIO(ext_irq);

	BpGetNfcWakeGpio(&bcm2079x_pdata.wake_gpio);
	bcm2079x_gpio_set(bcm2079x_pdata.wake_gpio, kGpioInactive);

	BpGetNfcPowerGpio(&bcm2079x_pdata.power_gpio);
	bcm2079x_gpio_set(bcm2079x_pdata.power_gpio, kGpioActive);

	// add bit-bang bus if i2c clock/data specified
	BpGetBitbangSclGpio(&bcm2079x_pdata.scl_pin);
	BpGetBitbangSdaGpio(&bcm2079x_pdata.sda_pin);
	if (bcm2079x_pdata.scl_pin != BP_NOT_DEFINED &&
	    bcm2079x_pdata.sda_pin != BP_NOT_DEFINED) {

		// scl always output for master
		bcm2079x_pdata.scl_dir = 0;
		bb2079x_setscl_dir(&bcm2079x_pdata, 1);
		bcm2079x_pdata.sda_dir = 0;
		bb2079x_setsda_dir(&bcm2079x_pdata, 1);

		// add bit-banged bus
		err = i2c_bit_add_bus(&bb2079x_adap);
		if (err < 0) {
			pr_err("failed to add bus %d\n", err);
			return err;
		}

		busnum = bb2079x_adap.nr;
	}

	err = i2c_add_driver(&bcm2079x_driver);
	if (err) {
		pr_err("failed to add driver %d", err);
		if (busnum) i2c_del_adapter(&bb2079x_adap);
		return err;
	}

	adap = i2c_get_adapter(busnum);
	if (!adap) {
		pr_err("failed to get adapter i2c bus %d\n", busnum);
		if (busnum) i2c_del_adapter(&bb2079x_adap);
		return -ENOENT;
	}

	// probe for the device at the known 8-bit addresses
	bcm2079x_i2c_client = i2c_new_probed_device(adap, i2c_bcm2079x,
		I2C_ADDRS(0x76, 0x65, 0x77, 0x66), bcm2079x_initseq);

	if (!bcm2079x_i2c_client) {
		pr_err("failed to register to i2c bus %d\n", busnum);
		i2c_put_adapter(adap);
		if (busnum) i2c_del_adapter(&bb2079x_adap);
		i2c_del_driver(&bcm2079x_driver);
		return -ENOENT;
	}

	i2c_put_adapter(adap);
	return 0;
}
module_init(bcm2079x_dev_init);

static void __exit bcm2079x_dev_exit(void)
{
	if (bcm2079x_i2c_client != NULL) {
		i2c_unregister_device(bcm2079x_i2c_client);
		i2c_del_adapter(&bb2079x_adap);
		bcm2079x_i2c_client = NULL;
		bcm2079x_gpio_set(bcm2079x_pdata.power_gpio, kGpioInactive);
	}

	i2c_del_driver(&bcm2079x_driver);
}
module_exit(bcm2079x_dev_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("NFC bcm2079x driver");
MODULE_LICENSE("GPL");
