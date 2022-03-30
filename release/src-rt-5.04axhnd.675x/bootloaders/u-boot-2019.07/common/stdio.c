// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 *
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 */

#include <config.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <stdarg.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <serial.h>
#include <splash.h>

#if defined(CONFIG_SYS_I2C)
#include <i2c.h>
#endif

#include <dm/device-internal.h>

DECLARE_GLOBAL_DATA_PTR;

static struct stdio_dev devs;
struct stdio_dev *stdio_devices[] = { NULL, NULL, NULL };
char *stdio_names[MAX_FILES] = { "stdin", "stdout", "stderr" };

#if defined(CONFIG_SPLASH_SCREEN) && !defined(CONFIG_SYS_DEVICE_NULLDEV)
#define	CONFIG_SYS_DEVICE_NULLDEV	1
#endif

#if CONFIG_IS_ENABLED(SYS_STDIO_DEREGISTER)
#define	CONFIG_SYS_DEVICE_NULLDEV	1
#endif

#ifdef CONFIG_SYS_DEVICE_NULLDEV
static void nulldev_putc(struct stdio_dev *dev, const char c)
{
	/* nulldev is empty! */
}

static void nulldev_puts(struct stdio_dev *dev, const char *s)
{
	/* nulldev is empty! */
}

static int nulldev_input(struct stdio_dev *dev)
{
	/* nulldev is empty! */
	return 0;
}
#endif

static void stdio_serial_putc(struct stdio_dev *dev, const char c)
{
	serial_putc(c);
}

static void stdio_serial_puts(struct stdio_dev *dev, const char *s)
{
	serial_puts(s);
}

static int stdio_serial_getc(struct stdio_dev *dev)
{
	return serial_getc();
}

static int stdio_serial_tstc(struct stdio_dev *dev)
{
	return serial_tstc();
}

/**************************************************************************
 * SYSTEM DRIVERS
 **************************************************************************
 */

static void drv_system_init (void)
{
	struct stdio_dev dev;

	memset (&dev, 0, sizeof (dev));

	strcpy (dev.name, "serial");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT;
	dev.putc = stdio_serial_putc;
	dev.puts = stdio_serial_puts;
	dev.getc = stdio_serial_getc;
	dev.tstc = stdio_serial_tstc;
	stdio_register (&dev);

#ifdef CONFIG_SYS_DEVICE_NULLDEV
	memset (&dev, 0, sizeof (dev));

	strcpy (dev.name, "nulldev");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT;
	dev.putc = nulldev_putc;
	dev.puts = nulldev_puts;
	dev.getc = nulldev_input;
	dev.tstc = nulldev_input;

	stdio_register (&dev);
#endif
}

/**************************************************************************
 * DEVICES
 **************************************************************************
 */
struct list_head* stdio_get_list(void)
{
	return &(devs.list);
}

#ifdef CONFIG_DM_VIDEO
/**
 * stdio_probe_device() - Find a device which provides the given stdio device
 *
 * This looks for a device of the given uclass which provides a particular
 * stdio device. It is currently really only useful for UCLASS_VIDEO.
 *
 * Ultimately we want to be able to probe a device by its stdio name. At
 * present devices register in their probe function (for video devices this
 * is done in vidconsole_post_probe()) and we don't know what name they will
 * use until they do so.
 * TODO(sjg@chromium.org): We should be able to determine the name before
 * probing, and probe the required device.
 *
 * @name:	stdio device name (e.g. "vidconsole")
 * id:		Uclass ID of device to look for (e.g. UCLASS_VIDEO)
 * @sdevp:	Returns stdout device, if found, else NULL
 * @return 0 if found, -ENOENT if no device found with that name, other -ve
 *	   on other error
 */
static int stdio_probe_device(const char *name, enum uclass_id id,
			      struct stdio_dev **sdevp)
{
	struct stdio_dev *sdev;
	struct udevice *dev;
	int seq, ret;

	*sdevp = NULL;
	seq = trailing_strtoln(name, NULL);
	if (seq == -1)
		seq = 0;
	ret = uclass_get_device_by_seq(id, seq, &dev);
	if (ret == -ENODEV)
		ret = uclass_first_device_err(id, &dev);
	if (ret) {
		debug("No %s device for seq %d (%s)\n", uclass_get_name(id),
		      seq, name);
		return ret;
	}
	/* The device should be be the last one registered */
	sdev = list_empty(&devs.list) ? NULL :
			list_last_entry(&devs.list, struct stdio_dev, list);
	if (!sdev || strcmp(sdev->name, name)) {
		debug("Device '%s' did not register with stdio as '%s'\n",
		      dev->name, name);
		return -ENOENT;
	}
	*sdevp = sdev;

	return 0;
}
#endif

struct stdio_dev *stdio_get_by_name(const char *name)
{
	struct list_head *pos;
	struct stdio_dev *sdev;

	if (!name)
		return NULL;

	list_for_each(pos, &(devs.list)) {
		sdev = list_entry(pos, struct stdio_dev, list);
		if (strcmp(sdev->name, name) == 0)
			return sdev;
	}
#ifdef CONFIG_DM_VIDEO
	/*
	 * We did not find a suitable stdio device. If there is a video
	 * driver with a name starting with 'vidconsole', we can try probing
	 * that in the hope that it will produce the required stdio device.
	 *
	 * This function is sometimes called with the entire value of
	 * 'stdout', which may include a list of devices separate by commas.
	 * Obviously this is not going to work, so we ignore that case. The
	 * call path in that case is console_init_r() -> search_device() ->
	 * stdio_get_by_name().
	 */
	if (!strncmp(name, "vidconsole", 10) && !strchr(name, ',') &&
	    !stdio_probe_device(name, UCLASS_VIDEO, &sdev))
		return sdev;
#endif

	return NULL;
}

struct stdio_dev* stdio_clone(struct stdio_dev *dev)
{
	struct stdio_dev *_dev;

	if(!dev)
		return NULL;

	_dev = calloc(1, sizeof(struct stdio_dev));

	if(!_dev)
		return NULL;

	memcpy(_dev, dev, sizeof(struct stdio_dev));

	return _dev;
}

int stdio_register_dev(struct stdio_dev *dev, struct stdio_dev **devp)
{
	struct stdio_dev *_dev;

	_dev = stdio_clone(dev);
	if(!_dev)
		return -ENODEV;
	list_add_tail(&(_dev->list), &(devs.list));
	if (devp)
		*devp = _dev;

	return 0;
}

int stdio_register(struct stdio_dev *dev)
{
	return stdio_register_dev(dev, NULL);
}

/* deregister the device "devname".
 * returns 0 if success, -1 if device is assigned and 1 if devname not found
 */
#if CONFIG_IS_ENABLED(SYS_STDIO_DEREGISTER)
int stdio_deregister_dev(struct stdio_dev *dev, int force)
{
	int l;
	struct list_head *pos;
	char temp_names[3][16];

	/* get stdio devices (ListRemoveItem changes the dev list) */
	for (l=0 ; l< MAX_FILES; l++) {
		if (stdio_devices[l] == dev) {
			if (force) {
				strcpy(temp_names[l], "nulldev");
				continue;
			}
			/* Device is assigned -> report error */
			return -1;
		}
		memcpy (&temp_names[l][0],
			stdio_devices[l]->name,
			sizeof(temp_names[l]));
	}

	list_del(&(dev->list));
	free(dev);

	/* reassign Device list */
	list_for_each(pos, &(devs.list)) {
		dev = list_entry(pos, struct stdio_dev, list);
		for (l=0 ; l< MAX_FILES; l++) {
			if(strcmp(dev->name, temp_names[l]) == 0)
				stdio_devices[l] = dev;
		}
	}
	return 0;
}

int stdio_deregister(const char *devname, int force)
{
	struct stdio_dev *dev;

	dev = stdio_get_by_name(devname);

	if (!dev) /* device not found */
		return -ENODEV;

	return stdio_deregister_dev(dev, force);
}
#endif /* CONFIG_IS_ENABLED(SYS_STDIO_DEREGISTER) */

int stdio_init_tables(void)
{
#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	/* already relocated for current ARM implementation */
	ulong relocation_offset = gd->reloc_off;
	int i;

	/* relocate device name pointers */
	for (i = 0; i < (sizeof (stdio_names) / sizeof (char *)); ++i) {
		stdio_names[i] = (char *) (((ulong) stdio_names[i]) +
						relocation_offset);
	}
#endif /* CONFIG_NEEDS_MANUAL_RELOC */

	/* Initialize the list */
	INIT_LIST_HEAD(&(devs.list));

	return 0;
}

int stdio_add_devices(void)
{
#ifdef CONFIG_DM_KEYBOARD
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	/*
	 * For now we probe all the devices here. At some point this should be
	 * done only when the devices are required - e.g. we have a list of
	 * input devices to start up in the stdin environment variable. That
	 * work probably makes more sense when stdio itself is converted to
	 * driver model.
	 *
	 * TODO(sjg@chromium.org): Convert changing uclass_first_device() etc.
	 * to return the device even on error. Then we could use that here.
	 */
	ret = uclass_get(UCLASS_KEYBOARD, &uc);
	if (ret)
		return ret;

	/* Don't report errors to the caller - assume that they are non-fatal */
	uclass_foreach_dev(dev, uc) {
		ret = device_probe(dev);
		if (ret)
			printf("Failed to probe keyboard '%s'\n", dev->name);
	}
#endif
#ifdef CONFIG_SYS_I2C
	i2c_init_all();
#else
#endif
#ifdef CONFIG_DM_VIDEO
	/*
	 * If the console setting is not in environment variables then
	 * console_init_r() will not be calling iomux_doenv() (which calls
	 * search_device()). So we will not dynamically add devices by
	 * calling stdio_probe_device().
	 *
	 * So just probe all video devices now so that whichever one is
	 * required will be available.
	 */
#ifndef CONFIG_SYS_CONSOLE_IS_IN_ENV
	struct udevice *vdev;
# ifndef CONFIG_DM_KEYBOARD
	int ret;
# endif

	for (ret = uclass_first_device(UCLASS_VIDEO, &vdev);
	     vdev;
	     ret = uclass_next_device(&vdev))
		;
	if (ret)
		printf("%s: Video device failed (ret=%d)\n", __func__, ret);
#endif /* !CONFIG_SYS_CONSOLE_IS_IN_ENV */
#if defined(CONFIG_SPLASH_SCREEN) && defined(CONFIG_CMD_BMP)
	splash_display();
#endif /* CONFIG_SPLASH_SCREEN && CONFIG_CMD_BMP */
#else
# if defined(CONFIG_LCD)
	drv_lcd_init ();
# endif
# if defined(CONFIG_VIDEO) || defined(CONFIG_CFB_CONSOLE)
	drv_video_init ();
# endif
#endif /* CONFIG_DM_VIDEO */
#if defined(CONFIG_KEYBOARD) && !defined(CONFIG_DM_KEYBOARD)
	drv_keyboard_init ();
#endif
	drv_system_init ();
	serial_stdio_init ();
#ifdef CONFIG_USB_TTY
	drv_usbtty_init ();
#endif
#ifdef CONFIG_NETCONSOLE
	drv_nc_init ();
#endif
#ifdef CONFIG_JTAG_CONSOLE
	drv_jtag_console_init ();
#endif
#ifdef CONFIG_CBMEM_CONSOLE
	cbmemc_init();
#endif

	return 0;
}

int stdio_init(void)
{
	stdio_init_tables();
	stdio_add_devices();

	return 0;
}
