// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
 */

#include <config.h>
#include <common.h>
#include <api_public.h>

#if defined(CONFIG_CMD_USB) && defined(CONFIG_USB_STORAGE)
#include <usb.h>
#endif

#define DEBUG
#undef DEBUG

#ifdef DEBUG
#define debugf(fmt, args...) do { printf("%s(): ", __func__); printf(fmt, ##args); } while (0)
#else
#define debugf(fmt, args...)
#endif

#define errf(fmt, args...) do { printf("ERROR @ %s(): ", __func__); printf(fmt, ##args); } while (0)


#define ENUM_IDE	0
#define ENUM_USB	1
#define ENUM_SCSI	2
#define ENUM_MMC	3
#define ENUM_SATA	4
#define ENUM_MAX	5

struct stor_spec {
	int		max_dev;
	int		enum_started;
	int		enum_ended;
	int		type;	/* "external" type: DT_STOR_{IDE,USB,etc} */
	char		*name;
};

static struct stor_spec specs[ENUM_MAX] = { { 0, 0, 0, 0, NULL }, };

#ifndef CONFIG_SYS_MMC_MAX_DEVICE
#define CONFIG_SYS_MMC_MAX_DEVICE	1
#endif

void dev_stor_init(void)
{
#if defined(CONFIG_IDE)
	specs[ENUM_IDE].max_dev = CONFIG_SYS_IDE_MAXDEVICE;
	specs[ENUM_IDE].enum_started = 0;
	specs[ENUM_IDE].enum_ended = 0;
	specs[ENUM_IDE].type = DEV_TYP_STOR | DT_STOR_IDE;
	specs[ENUM_IDE].name = "ide";
#endif
#if defined(CONFIG_CMD_MMC)
	specs[ENUM_MMC].max_dev = CONFIG_SYS_MMC_MAX_DEVICE;
	specs[ENUM_MMC].enum_started = 0;
	specs[ENUM_MMC].enum_ended = 0;
	specs[ENUM_MMC].type = DEV_TYP_STOR | DT_STOR_MMC;
	specs[ENUM_MMC].name = "mmc";
#endif
#if defined(CONFIG_SATA)
	specs[ENUM_SATA].max_dev = CONFIG_SYS_SATA_MAX_DEVICE;
	specs[ENUM_SATA].enum_started = 0;
	specs[ENUM_SATA].enum_ended = 0;
	specs[ENUM_SATA].type = DEV_TYP_STOR | DT_STOR_SATA;
	specs[ENUM_SATA].name = "sata";
#endif
#if defined(CONFIG_SCSI)
	specs[ENUM_SCSI].max_dev = CONFIG_SYS_SCSI_MAX_DEVICE;
	specs[ENUM_SCSI].enum_started = 0;
	specs[ENUM_SCSI].enum_ended = 0;
	specs[ENUM_SCSI].type = DEV_TYP_STOR | DT_STOR_SCSI;
	specs[ENUM_SCSI].name = "scsi";
#endif
#if defined(CONFIG_CMD_USB) && defined(CONFIG_USB_STORAGE)
	specs[ENUM_USB].max_dev = USB_MAX_STOR_DEV;
	specs[ENUM_USB].enum_started = 0;
	specs[ENUM_USB].enum_ended = 0;
	specs[ENUM_USB].type = DEV_TYP_STOR | DT_STOR_USB;
	specs[ENUM_USB].name = "usb";
#endif
}

/*
 * Finds next available device in the storage group
 *
 * type:	storage group type - ENUM_IDE, ENUM_SCSI etc.
 *
 * more:	returns 0/1 depending if there are more devices in this group
 *		available (for future iterations)
 *
 * returns:	0/1 depending if device found in this iteration
 */
static int dev_stor_get(int type, int *more, struct device_info *di)
{
	struct blk_desc *dd;
	int found = 0;
	int found_last = 0;
	int i = 0;

	/* Wasn't configured for this type, return 0 directly */
	if (specs[type].name == NULL)
		return 0;

	if (di->cookie != NULL) {
		/* Find the last device we've returned  */
		for (i = 0; i < specs[type].max_dev; i++) {
			if (di->cookie ==
			    (void *)blk_get_dev(specs[type].name, i)) {
				i += 1;
				found_last = 1;
				break;
			}
		}

		if (!found_last)
			i = 0;
	}

	for (; i < specs[type].max_dev; i++) {
		di->cookie = (void *)blk_get_dev(specs[type].name, i);

		if (di->cookie != NULL) {
			found = 1;
			break;
		}
	}

	if (i == specs[type].max_dev)
		*more = 0;
	else
		*more = 1;

	if (found) {
		di->type = specs[type].type;

		dd = (struct blk_desc *)di->cookie;
		if (dd->type == DEV_TYPE_UNKNOWN) {
			debugf("device instance exists, but is not active..");
			found = 0;
		} else {
			di->di_stor.block_count = dd->lba;
			di->di_stor.block_size = dd->blksz;
		}
	} else {
		di->cookie = NULL;
	}

	return found;
}


/* returns: ENUM_IDE, ENUM_USB etc. based on struct blk_desc */

static int dev_stor_type(struct blk_desc *dd)
{
	int i, j;

	for (i = ENUM_IDE; i < ENUM_MAX; i++)
		for (j = 0; j < specs[i].max_dev; j++)
			if (dd == blk_get_dev(specs[i].name, j))
				return i;

	return ENUM_MAX;
}


/* returns: 0/1 whether cookie points to some device in this group */

static int dev_is_stor(int type, struct device_info *di)
{
	return (dev_stor_type(di->cookie) == type) ? 1 : 0;
}


static int dev_enum_stor(int type, struct device_info *di)
{
	int found = 0, more = 0;

	debugf("called, type %d\n", type);

	/*
	 * Formulae for enumerating storage devices:
	 * 1. if cookie (hint from previous enum call) is NULL we start again
	 *    with enumeration, so return the first available device, done.
	 *
	 * 2. if cookie is not NULL, check if it identifies some device in
	 *    this group:
	 *
	 * 2a. if cookie is a storage device from our group (IDE, USB etc.),
	 *     return next available (if exists) in this group
	 *
	 * 2b. if it isn't device from our group, check if such devices were
	 *     ever enumerated before:
	 *     - if not, return the first available device from this group
	 *     - else return 0
	 */

	if (di->cookie == NULL) {
		debugf("group%d - enum restart\n", type);

		/*
		 * 1. Enumeration (re-)started: take the first available
		 * device, if exists
		 */
		found = dev_stor_get(type, &more, di);
		specs[type].enum_started = 1;

	} else if (dev_is_stor(type, di)) {
		debugf("group%d - enum continued for the next device\n", type);

		if (specs[type].enum_ended) {
			debugf("group%d - nothing more to enum!\n", type);
			return 0;
		}

		/* 2a. Attempt to take a next available device in the group */
		found = dev_stor_get(type, &more, di);

	} else {
		if (specs[type].enum_ended) {
			debugf("group %d - already enumerated, skipping\n", type);
			return 0;
		}

		debugf("group%d - first time enum\n", type);

		if (specs[type].enum_started == 0) {
			/*
			 * 2b.  If enumerating devices in this group did not
			 * happen before, it means the cookie pointed to a
			 * device from some other group (another storage
			 * group, or network); in this case try to take the
			 * first available device from our group
			 */
			specs[type].enum_started = 1;

			/*
			 * Attempt to take the first device in this group:
			 *'first element' flag is set
			 */
			found = dev_stor_get(type, &more, di);

		} else {
			errf("group%d - out of order iteration\n", type);
			found = 0;
			more = 0;
		}
	}

	/*
	 * If there are no more devices in this group, consider its
	 * enumeration finished
	 */
	specs[type].enum_ended = (!more) ? 1 : 0;

	if (found)
		debugf("device found, returning cookie 0x%08x\n",
		       (u_int32_t)di->cookie);
	else
		debugf("no device found\n");

	return found;
}

void dev_enum_reset(void)
{
	int i;

	for (i = 0; i < ENUM_MAX; i ++) {
		specs[i].enum_started = 0;
		specs[i].enum_ended = 0;
	}
}

int dev_enum_storage(struct device_info *di)
{
	int i;

	/* check: ide, usb, scsi, mmc */
	for (i = ENUM_IDE; i < ENUM_MAX; i ++) {
		if (dev_enum_stor(i, di))
			return 1;
	}

	return 0;
}

static int dev_stor_is_valid(int type, struct blk_desc *dd)
{
	int i;

	for (i = 0; i < specs[type].max_dev; i++)
		if (dd == blk_get_dev(specs[type].name, i))
			if (dd->type != DEV_TYPE_UNKNOWN)
				return 1;

	return 0;
}


int dev_open_stor(void *cookie)
{
	int type = dev_stor_type(cookie);

	if (type == ENUM_MAX)
		return API_ENODEV;

	if (dev_stor_is_valid(type, (struct blk_desc *)cookie))
		return 0;

	return API_ENODEV;
}


int dev_close_stor(void *cookie)
{
	/*
	 * Not much to do as we actually do not alter storage devices upon
	 * close
	 */
	return 0;
}


lbasize_t dev_read_stor(void *cookie, void *buf, lbasize_t len, lbastart_t start)
{
	int type;
	struct blk_desc *dd = (struct blk_desc *)cookie;

	if ((type = dev_stor_type(dd)) == ENUM_MAX)
		return 0;

	if (!dev_stor_is_valid(type, dd))
		return 0;

#ifdef CONFIG_BLK
	return blk_dread(dd, start, len, buf);
#else
	if ((dd->block_read) == NULL) {
		debugf("no block_read() for device 0x%08x\n", cookie);
		return 0;
	}

	return dd->block_read(dd, start, len, buf);
#endif	/* defined(CONFIG_BLK) */
}
