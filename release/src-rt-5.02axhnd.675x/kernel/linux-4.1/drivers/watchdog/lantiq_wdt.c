/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 *  Based on EP93xx wdt driver
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/of_platform.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <lantiq_soc.h>

/*
 * Section 3.4 of the datasheet
 * The password sequence protects the WDT control register from unintended
 * write actions, which might cause malfunction of the WDT.
 *
 * essentially the following two magic passwords need to be written to allow
 * IO access to the WDT core
 */
#define LTQ_WDT_PW1		0x00BE0000
#define LTQ_WDT_PW2		0x00DC0000

#define LTQ_WDT_CR		0x0	/* watchdog control register */
#define LTQ_WDT_SR		0x8	/* watchdog status register */

#define LTQ_WDT_SR_EN		(0x1 << 31)	/* enable bit */
#define LTQ_WDT_SR_PWD		(0x3 << 26)	/* turn on power */
#define LTQ_WDT_SR_CLKDIV	(0x3 << 24)	/* turn on clock and set */
						/* divider to 0x40000 */
#define LTQ_WDT_DIVIDER		0x40000
#define LTQ_MAX_TIMEOUT		((1 << 16) - 1)	/* the reload field is 16 bit */

static bool nowayout = WATCHDOG_NOWAYOUT;

static void __iomem *ltq_wdt_membase;
static unsigned long ltq_io_region_clk_rate;

static unsigned long ltq_wdt_bootstatus;
static unsigned long ltq_wdt_in_use;
static int ltq_wdt_timeout = 30;
static int ltq_wdt_ok_to_close;

static void
ltq_wdt_enable(void)
{
	unsigned long int timeout = ltq_wdt_timeout *
			(ltq_io_region_clk_rate / LTQ_WDT_DIVIDER) + 0x1000;
	if (timeout > LTQ_MAX_TIMEOUT)
		timeout = LTQ_MAX_TIMEOUT;

	/* write the first password magic */
	ltq_w32(LTQ_WDT_PW1, ltq_wdt_membase + LTQ_WDT_CR);
	/* write the second magic plus the configuration and new timeout */
	ltq_w32(LTQ_WDT_SR_EN | LTQ_WDT_SR_PWD | LTQ_WDT_SR_CLKDIV |
		LTQ_WDT_PW2 | timeout, ltq_wdt_membase + LTQ_WDT_CR);
}

static void
ltq_wdt_disable(void)
{
	/* write the first password magic */
	ltq_w32(LTQ_WDT_PW1, ltq_wdt_membase + LTQ_WDT_CR);
	/*
	 * write the second password magic with no config
	 * this turns the watchdog off
	 */
	ltq_w32(LTQ_WDT_PW2, ltq_wdt_membase + LTQ_WDT_CR);
}

static ssize_t
ltq_wdt_write(struct file *file, const char __user *data,
		size_t len, loff_t *ppos)
{
	if (len) {
		if (!nowayout) {
			size_t i;

			ltq_wdt_ok_to_close = 0;
			for (i = 0; i != len; i++) {
				char c;

				if (get_user(c, data + i))
					return -EFAULT;
				if (c == 'V')
					ltq_wdt_ok_to_close = 1;
				else
					ltq_wdt_ok_to_close = 0;
			}
		}
		ltq_wdt_enable();
	}

	return len;
}

static struct watchdog_info ident = {
	.options = WDIOF_MAGICCLOSE | WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING |
			WDIOF_CARDRESET,
	.identity = "ltq_wdt",
};

static long
ltq_wdt_ioctl(struct file *file,
		unsigned int cmd, unsigned long arg)
{
	int ret = -ENOTTY;

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		ret = copy_to_user((struct watchdog_info __user *)arg, &ident,
				sizeof(ident)) ? -EFAULT : 0;
		break;

	case WDIOC_GETBOOTSTATUS:
		ret = put_user(ltq_wdt_bootstatus, (int __user *)arg);
		break;

	case WDIOC_GETSTATUS:
		ret = put_user(0, (int __user *)arg);
		break;

	case WDIOC_SETTIMEOUT:
		ret = get_user(ltq_wdt_timeout, (int __user *)arg);
		if (!ret)
			ltq_wdt_enable();
		/* intentional drop through */
	case WDIOC_GETTIMEOUT:
		ret = put_user(ltq_wdt_timeout, (int __user *)arg);
		break;

	case WDIOC_KEEPALIVE:
		ltq_wdt_enable();
		ret = 0;
		break;
	}
	return ret;
}

static int
ltq_wdt_open(struct inode *inode, struct file *file)
{
	if (test_and_set_bit(0, &ltq_wdt_in_use))
		return -EBUSY;
	ltq_wdt_in_use = 1;
	ltq_wdt_enable();

	return nonseekable_open(inode, file);
}

static int
ltq_wdt_release(struct inode *inode, struct file *file)
{
	if (ltq_wdt_ok_to_close)
		ltq_wdt_disable();
	else
		pr_err("watchdog closed without warning\n");
	ltq_wdt_ok_to_close = 0;
	clear_bit(0, &ltq_wdt_in_use);

	return 0;
}

static const struct file_operations ltq_wdt_fops = {
	.owner		= THIS_MODULE,
	.write		= ltq_wdt_write,
	.unlocked_ioctl	= ltq_wdt_ioctl,
	.open		= ltq_wdt_open,
	.release	= ltq_wdt_release,
	.llseek		= no_llseek,
};

static struct miscdevice ltq_wdt_miscdev = {
	.minor	= WATCHDOG_MINOR,
	.name	= "watchdog",
	.fops	= &ltq_wdt_fops,
};

static int
ltq_wdt_probe(struct platform_device *pdev)
{
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	struct clk *clk;

	ltq_wdt_membase = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ltq_wdt_membase))
		return PTR_ERR(ltq_wdt_membase);

	/* we do not need to enable the clock as it is always running */
	clk = clk_get_io();
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "Failed to get clock\n");
		return -ENOENT;
	}
	ltq_io_region_clk_rate = clk_get_rate(clk);
	clk_put(clk);

	/* find out if the watchdog caused the last reboot */
	if (ltq_reset_cause() == LTQ_RST_CAUSE_WDTRST)
		ltq_wdt_bootstatus = WDIOF_CARDRESET;

	dev_info(&pdev->dev, "Init done\n");
	return misc_register(&ltq_wdt_miscdev);
}

static int
ltq_wdt_remove(struct platform_device *pdev)
{
	misc_deregister(&ltq_wdt_miscdev);

	return 0;
}

static const struct of_device_id ltq_wdt_match[] = {
	{ .compatible = "lantiq,wdt" },
	{},
};
MODULE_DEVICE_TABLE(of, ltq_wdt_match);

static struct platform_driver ltq_wdt_driver = {
	.probe = ltq_wdt_probe,
	.remove = ltq_wdt_remove,
	.driver = {
		.name = "wdt",
		.of_match_table = ltq_wdt_match,
	},
};

module_platform_driver(ltq_wdt_driver);

module_param(nowayout, bool, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started");
MODULE_AUTHOR("John Crispin <blogic@openwrt.org>");
MODULE_DESCRIPTION("Lantiq SoC Watchdog");
MODULE_LICENSE("GPL");
