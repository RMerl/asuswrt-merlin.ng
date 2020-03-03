/*
 *  linux/drivers/mmc/core/host.c
 *
 *  Copyright (C) 2003 Russell King, All Rights Reserved.
 *  Copyright (C) 2007-2008 Pierre Ossman
 *  Copyright (C) 2010 Linus Walleij
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  MMC host class device management
 */

#include <linux/device.h>
#include <linux/err.h>
#include <linux/idr.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pagemap.h>
#include <linux/export.h>
#include <linux/leds.h>
#include <linux/slab.h>
#include <linux/suspend.h>

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/slot-gpio.h>

#include "core.h"
#include "host.h"
#include "slot-gpio.h"
#include "pwrseq.h"

#define cls_dev_to_mmc_host(d)	container_of(d, struct mmc_host, class_dev)

static DEFINE_IDR(mmc_host_idr);
static DEFINE_SPINLOCK(mmc_host_lock);

static void mmc_host_classdev_release(struct device *dev)
{
	struct mmc_host *host = cls_dev_to_mmc_host(dev);
	spin_lock(&mmc_host_lock);
	idr_remove(&mmc_host_idr, host->index);
	spin_unlock(&mmc_host_lock);
	kfree(host);
}

static struct class mmc_host_class = {
	.name		= "mmc_host",
	.dev_release	= mmc_host_classdev_release,
};

int mmc_register_host_class(void)
{
	return class_register(&mmc_host_class);
}

void mmc_unregister_host_class(void)
{
	class_unregister(&mmc_host_class);
}

#ifdef CONFIG_MMC_CLKGATE
static ssize_t clkgate_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mmc_host *host = cls_dev_to_mmc_host(dev);
	return snprintf(buf, PAGE_SIZE, "%lu\n", host->clkgate_delay);
}

static ssize_t clkgate_delay_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct mmc_host *host = cls_dev_to_mmc_host(dev);
	unsigned long flags, value;

	if (kstrtoul(buf, 0, &value))
		return -EINVAL;

	spin_lock_irqsave(&host->clk_lock, flags);
	host->clkgate_delay = value;
	spin_unlock_irqrestore(&host->clk_lock, flags);
	return count;
}

/*
 * Enabling clock gating will make the core call out to the host
 * once up and once down when it performs a request or card operation
 * intermingled in any fashion. The driver will see this through
 * set_ios() operations with ios.clock field set to 0 to gate (disable)
 * the block clock, and to the old frequency to enable it again.
 */
static void mmc_host_clk_gate_delayed(struct mmc_host *host)
{
	unsigned long tick_ns;
	unsigned long freq = host->ios.clock;
	unsigned long flags;

	if (!freq) {
		pr_debug("%s: frequency set to 0 in disable function, "
			 "this means the clock is already disabled.\n",
			 mmc_hostname(host));
		return;
	}
	/*
	 * New requests may have appeared while we were scheduling,
	 * then there is no reason to delay the check before
	 * clk_disable().
	 */
	spin_lock_irqsave(&host->clk_lock, flags);

	/*
	 * Delay n bus cycles (at least 8 from MMC spec) before attempting
	 * to disable the MCI block clock. The reference count may have
	 * gone up again after this delay due to rescheduling!
	 */
	if (!host->clk_requests) {
		spin_unlock_irqrestore(&host->clk_lock, flags);
		tick_ns = DIV_ROUND_UP(1000000000, freq);
		ndelay(host->clk_delay * tick_ns);
	} else {
		/* New users appeared while waiting for this work */
		spin_unlock_irqrestore(&host->clk_lock, flags);
		return;
	}
	mutex_lock(&host->clk_gate_mutex);
	spin_lock_irqsave(&host->clk_lock, flags);
	if (!host->clk_requests) {
		spin_unlock_irqrestore(&host->clk_lock, flags);
		/* This will set host->ios.clock to 0 */
		mmc_gate_clock(host);
		spin_lock_irqsave(&host->clk_lock, flags);
		pr_debug("%s: gated MCI clock\n", mmc_hostname(host));
	}
	spin_unlock_irqrestore(&host->clk_lock, flags);
	mutex_unlock(&host->clk_gate_mutex);
}

/*
 * Internal work. Work to disable the clock at some later point.
 */
static void mmc_host_clk_gate_work(struct work_struct *work)
{
	struct mmc_host *host = container_of(work, struct mmc_host,
					      clk_gate_work.work);

	mmc_host_clk_gate_delayed(host);
}

/**
 *	mmc_host_clk_hold - ungate hardware MCI clocks
 *	@host: host to ungate.
 *
 *	Makes sure the host ios.clock is restored to a non-zero value
 *	past this call.	Increase clock reference count and ungate clock
 *	if we're the first user.
 */
void mmc_host_clk_hold(struct mmc_host *host)
{
	unsigned long flags;

	/* cancel any clock gating work scheduled by mmc_host_clk_release() */
	cancel_delayed_work_sync(&host->clk_gate_work);
	mutex_lock(&host->clk_gate_mutex);
	spin_lock_irqsave(&host->clk_lock, flags);
	if (host->clk_gated) {
		spin_unlock_irqrestore(&host->clk_lock, flags);
		mmc_ungate_clock(host);
		spin_lock_irqsave(&host->clk_lock, flags);
		pr_debug("%s: ungated MCI clock\n", mmc_hostname(host));
	}
	host->clk_requests++;
	spin_unlock_irqrestore(&host->clk_lock, flags);
	mutex_unlock(&host->clk_gate_mutex);
}

/**
 *	mmc_host_may_gate_card - check if this card may be gated
 *	@card: card to check.
 */
static bool mmc_host_may_gate_card(struct mmc_card *card)
{
	/* If there is no card we may gate it */
	if (!card)
		return true;
	/*
	 * Don't gate SDIO cards! These need to be clocked at all times
	 * since they may be independent systems generating interrupts
	 * and other events. The clock requests counter from the core will
	 * go down to zero since the core does not need it, but we will not
	 * gate the clock, because there is somebody out there that may still
	 * be using it.
	 */
	return !(card->quirks & MMC_QUIRK_BROKEN_CLK_GATING);
}

/**
 *	mmc_host_clk_release - gate off hardware MCI clocks
 *	@host: host to gate.
 *
 *	Calls the host driver with ios.clock set to zero as often as possible
 *	in order to gate off hardware MCI clocks. Decrease clock reference
 *	count and schedule disabling of clock.
 */
void mmc_host_clk_release(struct mmc_host *host)
{
	unsigned long flags;

	spin_lock_irqsave(&host->clk_lock, flags);
	host->clk_requests--;
	if (mmc_host_may_gate_card(host->card) &&
	    !host->clk_requests)
		schedule_delayed_work(&host->clk_gate_work,
				      msecs_to_jiffies(host->clkgate_delay));
	spin_unlock_irqrestore(&host->clk_lock, flags);
}

/**
 *	mmc_host_clk_rate - get current clock frequency setting
 *	@host: host to get the clock frequency for.
 *
 *	Returns current clock frequency regardless of gating.
 */
unsigned int mmc_host_clk_rate(struct mmc_host *host)
{
	unsigned long freq;
	unsigned long flags;

	spin_lock_irqsave(&host->clk_lock, flags);
	if (host->clk_gated)
		freq = host->clk_old;
	else
		freq = host->ios.clock;
	spin_unlock_irqrestore(&host->clk_lock, flags);
	return freq;
}

/**
 *	mmc_host_clk_init - set up clock gating code
 *	@host: host with potential clock to control
 */
static inline void mmc_host_clk_init(struct mmc_host *host)
{
	host->clk_requests = 0;
	/* Hold MCI clock for 8 cycles by default */
	host->clk_delay = 8;
	/*
	 * Default clock gating delay is 0ms to avoid wasting power.
	 * This value can be tuned by writing into sysfs entry.
	 */
	host->clkgate_delay = 0;
	host->clk_gated = false;
	INIT_DELAYED_WORK(&host->clk_gate_work, mmc_host_clk_gate_work);
	spin_lock_init(&host->clk_lock);
	mutex_init(&host->clk_gate_mutex);
}

/**
 *	mmc_host_clk_exit - shut down clock gating code
 *	@host: host with potential clock to control
 */
static inline void mmc_host_clk_exit(struct mmc_host *host)
{
	/*
	 * Wait for any outstanding gate and then make sure we're
	 * ungated before exiting.
	 */
	if (cancel_delayed_work_sync(&host->clk_gate_work))
		mmc_host_clk_gate_delayed(host);
	if (host->clk_gated)
		mmc_host_clk_hold(host);
	/* There should be only one user now */
	WARN_ON(host->clk_requests > 1);
}

static inline void mmc_host_clk_sysfs_init(struct mmc_host *host)
{
	host->clkgate_delay_attr.show = clkgate_delay_show;
	host->clkgate_delay_attr.store = clkgate_delay_store;
	sysfs_attr_init(&host->clkgate_delay_attr.attr);
	host->clkgate_delay_attr.attr.name = "clkgate_delay";
	host->clkgate_delay_attr.attr.mode = S_IRUGO | S_IWUSR;
	if (device_create_file(&host->class_dev, &host->clkgate_delay_attr))
		pr_err("%s: Failed to create clkgate_delay sysfs entry\n",
				mmc_hostname(host));
}
#else

static inline void mmc_host_clk_init(struct mmc_host *host)
{
}

static inline void mmc_host_clk_exit(struct mmc_host *host)
{
}

static inline void mmc_host_clk_sysfs_init(struct mmc_host *host)
{
}

#endif

/**
 *	mmc_of_parse() - parse host's device-tree node
 *	@host: host whose node should be parsed.
 *
 * To keep the rest of the MMC subsystem unaware of whether DT has been
 * used to to instantiate and configure this host instance or not, we
 * parse the properties and set respective generic mmc-host flags and
 * parameters.
 */
int mmc_of_parse(struct mmc_host *host)
{
	struct device_node *np;
	u32 bus_width;
	int len, ret;
	bool cd_cap_invert, cd_gpio_invert = false;
	bool ro_cap_invert, ro_gpio_invert = false;

	if (!host->parent || !host->parent->of_node)
		return 0;

	np = host->parent->of_node;

	/* "bus-width" is translated to MMC_CAP_*_BIT_DATA flags */
	if (of_property_read_u32(np, "bus-width", &bus_width) < 0) {
		dev_dbg(host->parent,
			"\"bus-width\" property is missing, assuming 1 bit.\n");
		bus_width = 1;
	}

	switch (bus_width) {
	case 8:
		host->caps |= MMC_CAP_8_BIT_DATA;
		/* Hosts capable of 8-bit transfers can also do 4 bits */
	case 4:
		host->caps |= MMC_CAP_4_BIT_DATA;
		break;
	case 1:
		break;
	default:
		dev_err(host->parent,
			"Invalid \"bus-width\" value %u!\n", bus_width);
		return -EINVAL;
	}

	/* f_max is obtained from the optional "max-frequency" property */
	of_property_read_u32(np, "max-frequency", &host->f_max);

	/*
	 * Configure CD and WP pins. They are both by default active low to
	 * match the SDHCI spec. If GPIOs are provided for CD and / or WP, the
	 * mmc-gpio helpers are used to attach, configure and use them. If
	 * polarity inversion is specified in DT, one of MMC_CAP2_CD_ACTIVE_HIGH
	 * and MMC_CAP2_RO_ACTIVE_HIGH capability-2 flags is set. If the
	 * "broken-cd" property is provided, the MMC_CAP_NEEDS_POLL capability
	 * is set. If the "non-removable" property is found, the
	 * MMC_CAP_NONREMOVABLE capability is set and no card-detection
	 * configuration is performed.
	 */

	/* Parse Card Detection */
	if (of_find_property(np, "non-removable", &len)) {
		host->caps |= MMC_CAP_NONREMOVABLE;
	} else {
		cd_cap_invert = of_property_read_bool(np, "cd-inverted");

		if (of_find_property(np, "broken-cd", &len))
			host->caps |= MMC_CAP_NEEDS_POLL;

		ret = mmc_gpiod_request_cd(host, "cd", 0, true,
					   0, &cd_gpio_invert);
		if (!ret)
			dev_info(host->parent, "Got CD GPIO\n");
		else if (ret != -ENOENT && ret != -ENOSYS)
			return ret;

		/*
		 * There are two ways to flag that the CD line is inverted:
		 * through the cd-inverted flag and by the GPIO line itself
		 * being inverted from the GPIO subsystem. This is a leftover
		 * from the times when the GPIO subsystem did not make it
		 * possible to flag a line as inverted.
		 *
		 * If the capability on the host AND the GPIO line are
		 * both inverted, the end result is that the CD line is
		 * not inverted.
		 */
		if (cd_cap_invert ^ cd_gpio_invert)
			host->caps2 |= MMC_CAP2_CD_ACTIVE_HIGH;
	}

	/* Parse Write Protection */
	ro_cap_invert = of_property_read_bool(np, "wp-inverted");

	ret = mmc_gpiod_request_ro(host, "wp", 0, false, 0, &ro_gpio_invert);
	if (!ret)
		dev_info(host->parent, "Got WP GPIO\n");
	else if (ret != -ENOENT && ret != -ENOSYS)
		return ret;

	/* See the comment on CD inversion above */
	if (ro_cap_invert ^ ro_gpio_invert)
		host->caps2 |= MMC_CAP2_RO_ACTIVE_HIGH;

	if (of_find_property(np, "cap-sd-highspeed", &len))
		host->caps |= MMC_CAP_SD_HIGHSPEED;
	if (of_find_property(np, "cap-mmc-highspeed", &len))
		host->caps |= MMC_CAP_MMC_HIGHSPEED;
	if (of_find_property(np, "sd-uhs-sdr12", &len))
		host->caps |= MMC_CAP_UHS_SDR12;
	if (of_find_property(np, "sd-uhs-sdr25", &len))
		host->caps |= MMC_CAP_UHS_SDR25;
	if (of_find_property(np, "sd-uhs-sdr50", &len))
		host->caps |= MMC_CAP_UHS_SDR50;
	if (of_find_property(np, "sd-uhs-sdr104", &len))
		host->caps |= MMC_CAP_UHS_SDR104;
	if (of_find_property(np, "sd-uhs-ddr50", &len))
		host->caps |= MMC_CAP_UHS_DDR50;
	if (of_find_property(np, "cap-power-off-card", &len))
		host->caps |= MMC_CAP_POWER_OFF_CARD;
	if (of_find_property(np, "cap-sdio-irq", &len))
		host->caps |= MMC_CAP_SDIO_IRQ;
	if (of_find_property(np, "full-pwr-cycle", &len))
		host->caps2 |= MMC_CAP2_FULL_PWR_CYCLE;
	if (of_find_property(np, "keep-power-in-suspend", &len))
		host->pm_caps |= MMC_PM_KEEP_POWER;
	if (of_find_property(np, "enable-sdio-wakeup", &len))
		host->pm_caps |= MMC_PM_WAKE_SDIO_IRQ;
	if (of_find_property(np, "mmc-ddr-1_8v", &len))
		host->caps |= MMC_CAP_1_8V_DDR;
	if (of_find_property(np, "mmc-ddr-1_2v", &len))
		host->caps |= MMC_CAP_1_2V_DDR;
	if (of_find_property(np, "mmc-hs200-1_8v", &len))
		host->caps2 |= MMC_CAP2_HS200_1_8V_SDR;
	if (of_find_property(np, "mmc-hs200-1_2v", &len))
		host->caps2 |= MMC_CAP2_HS200_1_2V_SDR;
	if (of_find_property(np, "mmc-hs400-1_8v", &len))
		host->caps2 |= MMC_CAP2_HS400_1_8V | MMC_CAP2_HS200_1_8V_SDR;
	if (of_find_property(np, "mmc-hs400-1_2v", &len))
		host->caps2 |= MMC_CAP2_HS400_1_2V | MMC_CAP2_HS200_1_2V_SDR;

	host->dsr_req = !of_property_read_u32(np, "dsr", &host->dsr);
	if (host->dsr_req && (host->dsr & ~0xffff)) {
		dev_err(host->parent,
			"device tree specified broken value for DSR: 0x%x, ignoring\n",
			host->dsr);
		host->dsr_req = 0;
	}

	return mmc_pwrseq_alloc(host);
}

EXPORT_SYMBOL(mmc_of_parse);

/**
 *	mmc_alloc_host - initialise the per-host structure.
 *	@extra: sizeof private data structure
 *	@dev: pointer to host device model structure
 *
 *	Initialise the per-host structure.
 */
struct mmc_host *mmc_alloc_host(int extra, struct device *dev)
{
	int err;
	struct mmc_host *host;

	host = kzalloc(sizeof(struct mmc_host) + extra, GFP_KERNEL);
	if (!host)
		return NULL;

	/* scanning will be enabled when we're ready */
	host->rescan_disable = 1;
	idr_preload(GFP_KERNEL);
	spin_lock(&mmc_host_lock);
	err = idr_alloc(&mmc_host_idr, host, 0, 0, GFP_NOWAIT);
	if (err >= 0)
		host->index = err;
	spin_unlock(&mmc_host_lock);
	idr_preload_end();
	if (err < 0) {
		kfree(host);
		return NULL;
	}

	dev_set_name(&host->class_dev, "mmc%d", host->index);

	host->parent = dev;
	host->class_dev.parent = dev;
	host->class_dev.class = &mmc_host_class;
	device_initialize(&host->class_dev);

	if (mmc_gpio_alloc(host)) {
		put_device(&host->class_dev);
		return NULL;
	}

	mmc_host_clk_init(host);

	spin_lock_init(&host->lock);
	init_waitqueue_head(&host->wq);
	INIT_DELAYED_WORK(&host->detect, mmc_rescan);
#ifdef CONFIG_PM
	host->pm_notify.notifier_call = mmc_pm_notify;
#endif

	/*
	 * By default, hosts do not support SGIO or large requests.
	 * They have to set these according to their abilities.
	 */
	host->max_segs = 1;
	host->max_seg_size = PAGE_CACHE_SIZE;

	host->max_req_size = PAGE_CACHE_SIZE;
	host->max_blk_size = 512;
	host->max_blk_count = PAGE_CACHE_SIZE / 512;

	return host;
}

EXPORT_SYMBOL(mmc_alloc_host);

/**
 *	mmc_add_host - initialise host hardware
 *	@host: mmc host
 *
 *	Register the host with the driver model. The host must be
 *	prepared to start servicing requests before this function
 *	completes.
 */
int mmc_add_host(struct mmc_host *host)
{
	int err;

	WARN_ON((host->caps & MMC_CAP_SDIO_IRQ) &&
		!host->ops->enable_sdio_irq);

	err = device_add(&host->class_dev);
	if (err)
		return err;

	led_trigger_register_simple(dev_name(&host->class_dev), &host->led);

#ifdef CONFIG_DEBUG_FS
	mmc_add_host_debugfs(host);
#endif
	mmc_host_clk_sysfs_init(host);

	mmc_start_host(host);
	register_pm_notifier(&host->pm_notify);

	return 0;
}

EXPORT_SYMBOL(mmc_add_host);

/**
 *	mmc_remove_host - remove host hardware
 *	@host: mmc host
 *
 *	Unregister and remove all cards associated with this host,
 *	and power down the MMC bus. No new requests will be issued
 *	after this function has returned.
 */
void mmc_remove_host(struct mmc_host *host)
{
	unregister_pm_notifier(&host->pm_notify);
	mmc_stop_host(host);

#ifdef CONFIG_DEBUG_FS
	mmc_remove_host_debugfs(host);
#endif

	device_del(&host->class_dev);

	led_trigger_unregister_simple(host->led);

	mmc_host_clk_exit(host);
}

EXPORT_SYMBOL(mmc_remove_host);

/**
 *	mmc_free_host - free the host structure
 *	@host: mmc host
 *
 *	Free the host once all references to it have been dropped.
 */
void mmc_free_host(struct mmc_host *host)
{
	mmc_pwrseq_free(host);
	put_device(&host->class_dev);
}

EXPORT_SYMBOL(mmc_free_host);
