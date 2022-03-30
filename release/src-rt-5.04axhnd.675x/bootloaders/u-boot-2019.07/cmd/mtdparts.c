// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Robert Schwebel, Pengutronix, <r.schwebel@pengutronix.de>
 *
 * (C) Copyright 2003
 * Kai-Uwe Bloem, Auerswald GmbH & Co KG, <linux-development@auerswald.de>
 *
 * (C) Copyright 2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 *   Added support for reading flash partition table from environment.
 *   Parsing routines are based on driver/mtd/cmdline.c from the linux 2.4
 *   kernel tree.
 *
 * (C) Copyright 2008
 * Harald Welte, OpenMoko, Inc., Harald Welte <laforge@openmoko.org>
 *
 *   $Id: cmdlinepart.c,v 1.17 2004/11/26 11:18:47 lavinen Exp $
 *   Copyright 2002 SYSGO Real-Time Solutions GmbH
 */

/*
 * Three environment variables are used by the parsing routines:
 *
 * 'partition' - keeps current partition identifier
 *
 * partition  := <part-id>
 * <part-id>  := <dev-id>,part_num
 *
 *
 * 'mtdids' - linux kernel mtd device id <-> u-boot device id mapping
 *
 * mtdids=<idmap>[,<idmap>,...]
 *
 * <idmap>    := <dev-id>=<mtd-id>
 * <dev-id>   := 'nand'|'nor'|'onenand'|'spi-nand'<dev-num>
 * <dev-num>  := mtd device number, 0...
 * <mtd-id>   := unique device tag used by linux kernel to find mtd device (mtd->name)
 *
 *
 * 'mtdparts' - partition list
 *
 * mtdparts=[mtdparts=]<mtd-def>[;<mtd-def>...]
 *
 * <mtd-def>  := <mtd-id>:<part-def>[,<part-def>...]
 * <mtd-id>   := unique device tag used by linux kernel to find mtd device (mtd->name)
 * <part-def> := <size>[@<offset>][<name>][<ro-flag>]
 * <size>     := standard linux memsize OR '-' to denote all remaining space
 * <offset>   := partition start offset within the device
 * <name>     := '(' NAME ')'
 * <ro-flag>  := when set to 'ro' makes partition read-only (not used, passed to kernel)
 *
 * Notes:
 * - each <mtd-id> used in mtdparts must albo exist in 'mtddis' mapping
 * - if the above variables are not set defaults for a given target are used
 *
 * Examples:
 *
 * 1 NOR Flash, with 1 single writable partition:
 * mtdids=nor0=edb7312-nor
 * mtdparts=[mtdparts=]edb7312-nor:-
 *
 * 1 NOR Flash with 2 partitions, 1 NAND with one
 * mtdids=nor0=edb7312-nor,nand0=edb7312-nand
 * mtdparts=[mtdparts=]edb7312-nor:256k(ARMboot)ro,-(root);edb7312-nand:-(home)
 *
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <jffs2/load_kernel.h>
#include <linux/list.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>

#if defined(CONFIG_CMD_NAND)
#include <linux/mtd/rawnand.h>
#include <nand.h>
#endif

#if defined(CONFIG_CMD_ONENAND)
#include <linux/mtd/onenand.h>
#include <onenand_uboot.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* special size referring to all the remaining space in a partition */
#define SIZE_REMAINING		(~0llu)

/* special offset value, it is used when not provided by user
 *
 * this value is used temporarily during parsing, later such offests
 * are recalculated */
#define OFFSET_NOT_SPECIFIED	(~0llu)

/* minimum partition size */
#define MIN_PART_SIZE		4096

/* this flag needs to be set in part_info struct mask_flags
 * field for read-only partitions */
#define MTD_WRITEABLE_CMD		1

/* default values for mtdids and mtdparts variables */
#if !defined(MTDIDS_DEFAULT)
#ifdef CONFIG_MTDIDS_DEFAULT
#define MTDIDS_DEFAULT CONFIG_MTDIDS_DEFAULT
#else
#define MTDIDS_DEFAULT NULL
#endif
#endif
#if !defined(MTDPARTS_DEFAULT)
#ifdef CONFIG_MTDPARTS_DEFAULT
#define MTDPARTS_DEFAULT CONFIG_MTDPARTS_DEFAULT
#else
#define MTDPARTS_DEFAULT NULL
#endif
#endif
#if defined(CONFIG_SYS_MTDPARTS_RUNTIME)
extern void board_mtdparts_default(const char **mtdids, const char **mtdparts);
#endif
static const char *mtdids_default = MTDIDS_DEFAULT;
static const char *mtdparts_default = MTDPARTS_DEFAULT;

/* copies of last seen 'mtdids', 'mtdparts' and 'partition' env variables */
#define MTDIDS_MAXLEN		128
#define MTDPARTS_MAXLEN		512
#define PARTITION_MAXLEN	16
static char last_ids[MTDIDS_MAXLEN + 1];
static char last_parts[MTDPARTS_MAXLEN + 1];
static char last_partition[PARTITION_MAXLEN + 1];

/* low level jffs2 cache cleaning routine */
extern void jffs2_free_cache(struct part_info *part);

/* mtdids mapping list, filled by parse_ids() */
static struct list_head mtdids;

/* device/partition list, parse_cmdline() parses into here */
static struct list_head devices;

/* current active device and partition number */
struct mtd_device *current_mtd_dev = NULL;
u8 current_mtd_partnum = 0;

u8 use_defaults;

static struct part_info* mtd_part_info(struct mtd_device *dev, unsigned int part_num);

/* command line only routines */
static struct mtdids* id_find_by_mtd_id(const char *mtd_id, unsigned int mtd_id_len);
static int device_del(struct mtd_device *dev);

/**
 * Parses a string into a number.  The number stored at ptr is
 * potentially suffixed with K (for kilobytes, or 1024 bytes),
 * M (for megabytes, or 1048576 bytes), or G (for gigabytes, or
 * 1073741824).  If the number is suffixed with K, M, or G, then
 * the return value is the number multiplied by one kilobyte, one
 * megabyte, or one gigabyte, respectively.
 *
 * @param ptr where parse begins
 * @param retptr output pointer to next char after parse completes (output)
 * @return resulting unsigned int
 */
static u64 memsize_parse (const char *const ptr, const char **retptr)
{
	u64 ret = simple_strtoull(ptr, (char **)retptr, 0);

	switch (**retptr) {
		case 'G':
		case 'g':
			ret <<= 10;
			/* Fallthrough */
		case 'M':
		case 'm':
			ret <<= 10;
			/* Fallthrough */
		case 'K':
		case 'k':
			ret <<= 10;
			(*retptr)++;
			/* Fallthrough */
		default:
			break;
	}

	return ret;
}

/**
 * Format string describing supplied size. This routine does the opposite job
 * to memsize_parse(). Size in bytes is converted to string and if possible
 * shortened by using k (kilobytes), m (megabytes) or g (gigabytes) suffix.
 *
 * Note, that this routine does not check for buffer overflow, it's the caller
 * who must assure enough space.
 *
 * @param buf output buffer
 * @param size size to be converted to string
 */
static void memsize_format(char *buf, u64 size)
{
#define SIZE_GB ((u32)1024*1024*1024)
#define SIZE_MB ((u32)1024*1024)
#define SIZE_KB ((u32)1024)

	if ((size % SIZE_GB) == 0)
		sprintf(buf, "%llug", size/SIZE_GB);
	else if ((size % SIZE_MB) == 0)
		sprintf(buf, "%llum", size/SIZE_MB);
	else if (size % SIZE_KB == 0)
		sprintf(buf, "%lluk", size/SIZE_KB);
	else
		sprintf(buf, "%llu", size);
}

/**
 * This routine does global indexing of all partitions. Resulting index for
 * current partition is saved in 'mtddevnum'. Current partition name in
 * 'mtddevname'.
 */
static void index_partitions(void)
{
	u16 mtddevnum;
	struct part_info *part;
	struct list_head *dentry;
	struct mtd_device *dev;

	debug("--- index partitions ---\n");

	if (current_mtd_dev) {
		mtddevnum = 0;
		list_for_each(dentry, &devices) {
			dev = list_entry(dentry, struct mtd_device, link);
			if (dev == current_mtd_dev) {
				mtddevnum += current_mtd_partnum;
				env_set_ulong("mtddevnum", mtddevnum);
				debug("=> mtddevnum %d,\n", mtddevnum);
				break;
			}
			mtddevnum += dev->num_parts;
		}

		part = mtd_part_info(current_mtd_dev, current_mtd_partnum);
		if (part) {
			env_set("mtddevname", part->name);

			debug("=> mtddevname %s\n", part->name);
		} else {
			env_set("mtddevname", NULL);

			debug("=> mtddevname NULL\n");
		}
	} else {
		env_set("mtddevnum", NULL);
		env_set("mtddevname", NULL);

		debug("=> mtddevnum NULL\n=> mtddevname NULL\n");
	}
}

/**
 * Save current device and partition in environment variable 'partition'.
 */
static void current_save(void)
{
	char buf[16];

	debug("--- current_save ---\n");

	if (current_mtd_dev) {
		sprintf(buf, "%s%d,%d", MTD_DEV_TYPE(current_mtd_dev->id->type),
					current_mtd_dev->id->num, current_mtd_partnum);

		env_set("partition", buf);
		strncpy(last_partition, buf, 16);

		debug("=> partition %s\n", buf);
	} else {
		env_set("partition", NULL);
		last_partition[0] = '\0';

		debug("=> partition NULL\n");
	}
	index_partitions();
}


/**
 * Produce a mtd_info given a type and num.
 *
 * @param type mtd type
 * @param num mtd number
 * @param mtd a pointer to an mtd_info instance (output)
 * @return 0 if device is valid, 1 otherwise
 */
static int get_mtd_info(u8 type, u8 num, struct mtd_info **mtd)
{
	char mtd_dev[16];

	sprintf(mtd_dev, "%s%d", MTD_DEV_TYPE(type), num);
	*mtd = get_mtd_device_nm(mtd_dev);
	if (IS_ERR(*mtd)) {
		printf("Device %s not found!\n", mtd_dev);
		return 1;
	}
	put_mtd_device(*mtd);

	return 0;
}

/**
 * Performs sanity check for supplied flash partition.
 * Table of existing MTD flash devices is searched and partition device
 * is located. Alignment with the granularity of nand erasesize is verified.
 *
 * @param id of the parent device
 * @param part partition to validate
 * @return 0 if partition is valid, 1 otherwise
 */
static int part_validate_eraseblock(struct mtdids *id, struct part_info *part)
{
	struct mtd_info *mtd = NULL;
	int i, j;
	ulong start;
	u64 offset, size;

	if (get_mtd_info(id->type, id->num, &mtd))
		return 1;

	part->sector_size = mtd->erasesize;

	if (!mtd->numeraseregions) {
		/*
		 * Only one eraseregion (NAND, SPI-NAND, OneNAND or uniform NOR),
		 * checking for alignment is easy here
		 */
		offset = part->offset;
		if (do_div(offset, mtd->erasesize)) {
			printf("%s%d: partition (%s) start offset"
			       "alignment incorrect\n",
			       MTD_DEV_TYPE(id->type), id->num, part->name);
			return 1;
		}

		size = part->size;
		if (do_div(size, mtd->erasesize)) {
			printf("%s%d: partition (%s) size alignment incorrect\n",
			       MTD_DEV_TYPE(id->type), id->num, part->name);
			return 1;
		}
	} else {
		/*
		 * Multiple eraseregions (non-uniform NOR),
		 * checking for alignment is more complex here
		 */

		/* Check start alignment */
		for (i = 0; i < mtd->numeraseregions; i++) {
			start = mtd->eraseregions[i].offset;
			for (j = 0; j < mtd->eraseregions[i].numblocks; j++) {
				if (part->offset == start)
					goto start_ok;
				start += mtd->eraseregions[i].erasesize;
			}
		}

		printf("%s%d: partition (%s) start offset alignment incorrect\n",
		       MTD_DEV_TYPE(id->type), id->num, part->name);
		return 1;

	start_ok:

		/* Check end/size alignment */
		for (i = 0; i < mtd->numeraseregions; i++) {
			start = mtd->eraseregions[i].offset;
			for (j = 0; j < mtd->eraseregions[i].numblocks; j++) {
				if ((part->offset + part->size) == start)
					goto end_ok;
				start += mtd->eraseregions[i].erasesize;
			}
		}
		/* Check last sector alignment */
		if ((part->offset + part->size) == start)
			goto end_ok;

		printf("%s%d: partition (%s) size alignment incorrect\n",
		       MTD_DEV_TYPE(id->type), id->num, part->name);
		return 1;

	end_ok:
		return 0;
	}

	return 0;
}


/**
 * Performs sanity check for supplied partition. Offset and size are
 * verified to be within valid range. Partition type is checked and
 * part_validate_eraseblock() is called with the argument of part.
 *
 * @param id of the parent device
 * @param part partition to validate
 * @return 0 if partition is valid, 1 otherwise
 */
static int part_validate(struct mtdids *id, struct part_info *part)
{
	if (part->size == SIZE_REMAINING)
		part->size = id->size - part->offset;

	if (part->offset > id->size) {
		printf("%s: offset %08llx beyond flash size %08llx\n",
				id->mtd_id, part->offset, id->size);
		return 1;
	}

	if ((part->offset + part->size) <= part->offset) {
		printf("%s%d: partition (%s) size too big\n",
				MTD_DEV_TYPE(id->type), id->num, part->name);
		return 1;
	}

	if (part->offset + part->size > id->size) {
		printf("%s: partitioning exceeds flash size\n", id->mtd_id);
		return 1;
	}

	/*
	 * Now we need to check if the partition starts and ends on
	 * sector (eraseblock) regions
	 */
	return part_validate_eraseblock(id, part);
}

/**
 * Delete selected partition from the partition list of the specified device.
 *
 * @param dev device to delete partition from
 * @param part partition to delete
 * @return 0 on success, 1 otherwise
 */
static int part_del(struct mtd_device *dev, struct part_info *part)
{
	u8 current_save_needed = 0;

	/* if there is only one partition, remove whole device */
	if (dev->num_parts == 1)
		return device_del(dev);

	/* otherwise just delete this partition */

	if (dev == current_mtd_dev) {
		/* we are modyfing partitions for the current device,
		 * update current */
		struct part_info *curr_pi;
		curr_pi = mtd_part_info(current_mtd_dev, current_mtd_partnum);

		if (curr_pi) {
			if (curr_pi == part) {
				printf("current partition deleted, resetting current to 0\n");
				current_mtd_partnum = 0;
			} else if (part->offset <= curr_pi->offset) {
				current_mtd_partnum--;
			}
			current_save_needed = 1;
		}
	}

	list_del(&part->link);
	free(part);
	dev->num_parts--;

	if (current_save_needed > 0)
		current_save();
	else
		index_partitions();

	return 0;
}

/**
 * Delete all partitions from parts head list, free memory.
 *
 * @param head list of partitions to delete
 */
static void part_delall(struct list_head *head)
{
	struct list_head *entry, *n;
	struct part_info *part_tmp;

	/* clean tmp_list and free allocated memory */
	list_for_each_safe(entry, n, head) {
		part_tmp = list_entry(entry, struct part_info, link);

		list_del(entry);
		free(part_tmp);
	}
}

/**
 * Add new partition to the supplied partition list. Make sure partitions are
 * sorted by offset in ascending order.
 *
 * @param head list this partition is to be added to
 * @param new partition to be added
 */
static int part_sort_add(struct mtd_device *dev, struct part_info *part)
{
	struct list_head *entry;
	struct part_info *new_pi, *curr_pi;

	/* link partition to parrent dev */
	part->dev = dev;

	if (list_empty(&dev->parts)) {
		debug("part_sort_add: list empty\n");
		list_add(&part->link, &dev->parts);
		dev->num_parts++;
		index_partitions();
		return 0;
	}

	new_pi = list_entry(&part->link, struct part_info, link);

	/* get current partition info if we are updating current device */
	curr_pi = NULL;
	if (dev == current_mtd_dev)
		curr_pi = mtd_part_info(current_mtd_dev, current_mtd_partnum);

	list_for_each(entry, &dev->parts) {
		struct part_info *pi;

		pi = list_entry(entry, struct part_info, link);

		/* be compliant with kernel cmdline, allow only one partition at offset zero */
		if ((new_pi->offset == pi->offset) && (pi->offset == 0)) {
			printf("cannot add second partition at offset 0\n");
			return 1;
		}

		if (new_pi->offset <= pi->offset) {
			list_add_tail(&part->link, entry);
			dev->num_parts++;

			if (curr_pi && (pi->offset <= curr_pi->offset)) {
				/* we are modyfing partitions for the current
				 * device, update current */
				current_mtd_partnum++;
				current_save();
			} else {
				index_partitions();
			}
			return 0;
		}
	}

	list_add_tail(&part->link, &dev->parts);
	dev->num_parts++;
	index_partitions();
	return 0;
}

/**
 * Add provided partition to the partition list of a given device.
 *
 * @param dev device to which partition is added
 * @param part partition to be added
 * @return 0 on success, 1 otherwise
 */
static int part_add(struct mtd_device *dev, struct part_info *part)
{
	/* verify alignment and size */
	if (part_validate(dev->id, part) != 0)
		return 1;

	/* partition is ok, add it to the list */
	if (part_sort_add(dev, part) != 0)
		return 1;

	return 0;
}

/**
 * Parse one partition definition, allocate memory and return pointer to this
 * location in retpart.
 *
 * @param partdef pointer to the partition definition string i.e. <part-def>
 * @param ret output pointer to next char after parse completes (output)
 * @param retpart pointer to the allocated partition (output)
 * @return 0 on success, 1 otherwise
 */
static int part_parse(const char *const partdef, const char **ret, struct part_info **retpart)
{
	struct part_info *part;
	u64 size;
	u64 offset;
	const char *name;
	int name_len;
	unsigned int mask_flags;
	const char *p;

	p = partdef;
	*retpart = NULL;
	*ret = NULL;

	/* fetch the partition size */
	if (*p == '-') {
		/* assign all remaining space to this partition */
		debug("'-': remaining size assigned\n");
		size = SIZE_REMAINING;
		p++;
	} else {
		size = memsize_parse(p, &p);
		if (size < MIN_PART_SIZE) {
			printf("partition size too small (%llx)\n", size);
			return 1;
		}
	}

	/* check for offset */
	offset = OFFSET_NOT_SPECIFIED;
	if (*p == '@') {
		p++;
		offset = memsize_parse(p, &p);
	}

	/* now look for the name */
	if (*p == '(') {
		name = ++p;
		if ((p = strchr(name, ')')) == NULL) {
			printf("no closing ) found in partition name\n");
			return 1;
		}
		name_len = p - name + 1;
		if ((name_len - 1) == 0) {
			printf("empty partition name\n");
			return 1;
		}
		p++;
	} else {
		/* 0x00000000@0x00000000 */
		name_len = 22;
		name = NULL;
	}

	/* test for options */
	mask_flags = 0;
	if (strncmp(p, "ro", 2) == 0) {
		mask_flags |= MTD_WRITEABLE_CMD;
		p += 2;
	}

	/* check for next partition definition */
	if (*p == ',') {
		if (size == SIZE_REMAINING) {
			*ret = NULL;
			printf("no partitions allowed after a fill-up partition\n");
			return 1;
		}
		*ret = ++p;
	} else if ((*p == ';') || (*p == '\0')) {
		*ret = p;
	} else {
		printf("unexpected character '%c' at the end of partition\n", *p);
		*ret = NULL;
		return 1;
	}

	/*  allocate memory */
	part = (struct part_info *)malloc(sizeof(struct part_info) + name_len);
	if (!part) {
		printf("out of memory\n");
		return 1;
	}
	memset(part, 0, sizeof(struct part_info) + name_len);
	part->size = size;
	part->offset = offset;
	part->mask_flags = mask_flags;
	part->name = (char *)(part + 1);

	if (name) {
		/* copy user provided name */
		strncpy(part->name, name, name_len - 1);
		part->auto_name = 0;
	} else {
		/* auto generated name in form of size@offset */
		snprintf(part->name, name_len, "0x%08llx@0x%08llx", size, offset);
		part->auto_name = 1;
	}

	part->name[name_len - 1] = '\0';
	INIT_LIST_HEAD(&part->link);

	debug("+ partition: name %-22s size 0x%08llx offset 0x%08llx mask flags %d\n",
			part->name, part->size,
			part->offset, part->mask_flags);

	*retpart = part;
	return 0;
}

/**
 * Check device number to be within valid range for given device type.
 *
 * @param type mtd type
 * @param num mtd number
 * @param size a pointer to the size of the mtd device (output)
 * @return 0 if device is valid, 1 otherwise
 */
static int mtd_device_validate(u8 type, u8 num, u64 *size)
{
	struct mtd_info *mtd = NULL;

	if (get_mtd_info(type, num, &mtd))
		return 1;

	*size = mtd->size;

	return 0;
}

/**
 * Delete all mtd devices from a supplied devices list, free memory allocated for
 * each device and delete all device partitions.
 *
 * @return 0 on success, 1 otherwise
 */
static int device_delall(struct list_head *head)
{
	struct list_head *entry, *n;
	struct mtd_device *dev_tmp;

	/* clean devices list */
	list_for_each_safe(entry, n, head) {
		dev_tmp = list_entry(entry, struct mtd_device, link);
		list_del(entry);
		part_delall(&dev_tmp->parts);
		free(dev_tmp);
	}
	INIT_LIST_HEAD(&devices);

	return 0;
}

/**
 * If provided device exists it's partitions are deleted, device is removed
 * from device list and device memory is freed.
 *
 * @param dev device to be deleted
 * @return 0 on success, 1 otherwise
 */
static int device_del(struct mtd_device *dev)
{
	part_delall(&dev->parts);
	list_del(&dev->link);
	free(dev);

	if (dev == current_mtd_dev) {
		/* we just deleted current device */
		if (list_empty(&devices)) {
			current_mtd_dev = NULL;
		} else {
			/* reset first partition from first dev from the
			 * devices list as current */
			current_mtd_dev = list_entry(devices.next, struct mtd_device, link);
			current_mtd_partnum = 0;
		}
		current_save();
		return 0;
	}

	index_partitions();
	return 0;
}

/**
 * Search global device list and return pointer to the device of type and num
 * specified.
 *
 * @param type device type
 * @param num device number
 * @return NULL if requested device does not exist
 */
struct mtd_device *device_find(u8 type, u8 num)
{
	struct list_head *entry;
	struct mtd_device *dev_tmp;

	list_for_each(entry, &devices) {
		dev_tmp = list_entry(entry, struct mtd_device, link);

		if ((dev_tmp->id->type == type) && (dev_tmp->id->num == num))
			return dev_tmp;
	}

	return NULL;
}

/**
 * Add specified device to the global device list.
 *
 * @param dev device to be added
 */
static void device_add(struct mtd_device *dev)
{
	u8 current_save_needed = 0;

	if (list_empty(&devices)) {
		current_mtd_dev = dev;
		current_mtd_partnum = 0;
		current_save_needed = 1;
	}

	list_add_tail(&dev->link, &devices);

	if (current_save_needed > 0)
		current_save();
	else
		index_partitions();
}

/**
 * Parse device type, name and mtd-id. If syntax is ok allocate memory and
 * return pointer to the device structure.
 *
 * @param mtd_dev pointer to the device definition string i.e. <mtd-dev>
 * @param ret output pointer to next char after parse completes (output)
 * @param retdev pointer to the allocated device (output)
 * @return 0 on success, 1 otherwise
 */
static int device_parse(const char *const mtd_dev, const char **ret, struct mtd_device **retdev)
{
	struct mtd_device *dev;
	struct part_info *part;
	struct mtdids *id;
	const char *mtd_id;
	unsigned int mtd_id_len;
	const char *p;
	const char *pend;
	LIST_HEAD(tmp_list);
	struct list_head *entry, *n;
	u16 num_parts;
	u64 offset;
	int err = 1;

	debug("===device_parse===\n");

	assert(retdev);
	*retdev = NULL;

	if (ret)
		*ret = NULL;

	/* fetch <mtd-id> */
	mtd_id = p = mtd_dev;
	if (!(p = strchr(mtd_id, ':'))) {
		printf("no <mtd-id> identifier\n");
		return 1;
	}
	mtd_id_len = p - mtd_id + 1;
	p++;

	/* verify if we have a valid device specified */
	if ((id = id_find_by_mtd_id(mtd_id, mtd_id_len - 1)) == NULL) {
		printf("invalid mtd device '%.*s'\n", mtd_id_len - 1, mtd_id);
		return 1;
	}

	pend = strchr(p, ';');
	debug("dev type = %d (%s), dev num = %d, mtd-id = %s\n",
			id->type, MTD_DEV_TYPE(id->type),
			id->num, id->mtd_id);
	debug("parsing partitions %.*s\n", (int)(pend ? pend - p : strlen(p)), p);

	/* parse partitions */
	num_parts = 0;

	offset = 0;
	if ((dev = device_find(id->type, id->num)) != NULL) {
		/* if device already exists start at the end of the last partition */
		part = list_entry(dev->parts.prev, struct part_info, link);
		offset = part->offset + part->size;
	}

	while (p && (*p != '\0') && (*p != ';')) {
		err = 1;
		if ((part_parse(p, &p, &part) != 0) || (!part))
			break;

		/* calculate offset when not specified */
		if (part->offset == OFFSET_NOT_SPECIFIED)
			part->offset = offset;
		else
			offset = part->offset;

		/* verify alignment and size */
		if (part_validate(id, part) != 0)
			break;

		offset += part->size;

		/* partition is ok, add it to the list */
		list_add_tail(&part->link, &tmp_list);
		num_parts++;
		err = 0;
	}
	if (err == 1) {
		part_delall(&tmp_list);
		return 1;
	}

	debug("\ntotal partitions: %d\n", num_parts);

	/* check for next device presence */
	if (p) {
		if (*p == ';') {
			if (ret)
				*ret = ++p;
		} else if (*p == '\0') {
			if (ret)
				*ret = p;
		} else {
			printf("unexpected character '%c' at the end of device\n", *p);
			if (ret)
				*ret = NULL;
			return 1;
		}
	}

	/* allocate memory for mtd_device structure */
	if ((dev = (struct mtd_device *)malloc(sizeof(struct mtd_device))) == NULL) {
		printf("out of memory\n");
		return 1;
	}
	memset(dev, 0, sizeof(struct mtd_device));
	dev->id = id;
	dev->num_parts = 0; /* part_sort_add increments num_parts */
	INIT_LIST_HEAD(&dev->parts);
	INIT_LIST_HEAD(&dev->link);

	/* move partitions from tmp_list to dev->parts */
	list_for_each_safe(entry, n, &tmp_list) {
		part = list_entry(entry, struct part_info, link);
		list_del(entry);
		if (part_sort_add(dev, part) != 0) {
			device_del(dev);
			return 1;
		}
	}

	*retdev = dev;

	debug("===\n\n");
	return 0;
}

/**
 * Initialize global device list.
 *
 * @return 0 on success, 1 otherwise
 */
static int mtd_devices_init(void)
{
	last_parts[0] = '\0';
	current_mtd_dev = NULL;
	current_save();

	return device_delall(&devices);
}

/*
 * Search global mtdids list and find id of requested type and number.
 *
 * @return pointer to the id if it exists, NULL otherwise
 */
static struct mtdids* id_find(u8 type, u8 num)
{
	struct list_head *entry;
	struct mtdids *id;

	list_for_each(entry, &mtdids) {
		id = list_entry(entry, struct mtdids, link);

		if ((id->type == type) && (id->num == num))
			return id;
	}

	return NULL;
}

/**
 * Search global mtdids list and find id of a requested mtd_id.
 *
 * Note: first argument is not null terminated.
 *
 * @param mtd_id string containing requested mtd_id
 * @param mtd_id_len length of supplied mtd_id
 * @return pointer to the id if it exists, NULL otherwise
 */
static struct mtdids* id_find_by_mtd_id(const char *mtd_id, unsigned int mtd_id_len)
{
	struct list_head *entry;
	struct mtdids *id;

	debug("--- id_find_by_mtd_id: '%.*s' (len = %d)\n",
			mtd_id_len, mtd_id, mtd_id_len);

	list_for_each(entry, &mtdids) {
		id = list_entry(entry, struct mtdids, link);

		debug("entry: '%s' (len = %zu)\n",
				id->mtd_id, strlen(id->mtd_id));

		if (mtd_id_len != strlen(id->mtd_id))
			continue;
		if (strncmp(id->mtd_id, mtd_id, mtd_id_len) == 0)
			return id;
	}

	return NULL;
}

/**
 * Parse device id string <dev-id> := 'nand'|'nor'|'onenand'|'spi-nand'<dev-num>,
 * return device type and number.
 *
 * @param id string describing device id
 * @param ret_id output pointer to next char after parse completes (output)
 * @param dev_type parsed device type (output)
 * @param dev_num parsed device number (output)
 * @return 0 on success, 1 otherwise
 */
int mtd_id_parse(const char *id, const char **ret_id, u8 *dev_type,
		 u8 *dev_num)
{
	const char *p = id;

	*dev_type = 0;
	if (strncmp(p, "nand", 4) == 0) {
		*dev_type = MTD_DEV_TYPE_NAND;
		p += 4;
	} else if (strncmp(p, "nor", 3) == 0) {
		*dev_type = MTD_DEV_TYPE_NOR;
		p += 3;
	} else if (strncmp(p, "onenand", 7) == 0) {
		*dev_type = MTD_DEV_TYPE_ONENAND;
		p += 7;
	} else if (strncmp(p, "spi-nand", 8) == 0) {
		*dev_type = MTD_DEV_TYPE_SPINAND;
		p += 8;
	} else {
		printf("incorrect device type in %s\n", id);
		return 1;
	}

	if (!isdigit(*p)) {
		printf("incorrect device number in %s\n", id);
		return 1;
	}

	*dev_num = simple_strtoul(p, (char **)&p, 0);
	if (ret_id)
		*ret_id = p;
	return 0;
}

/**
 * Process all devices and generate corresponding mtdparts string describing
 * all partitions on all devices.
 *
 * @param buf output buffer holding generated mtdparts string (output)
 * @param buflen buffer size
 * @return 0 on success, 1 otherwise
 */
static int generate_mtdparts(char *buf, u32 buflen)
{
	struct list_head *pentry, *dentry;
	struct mtd_device *dev;
	struct part_info *part, *prev_part;
	char *p = buf;
	char tmpbuf[32];
	u64 size, offset;
	u32 len, part_cnt;
	u32 maxlen = buflen - 1;

	debug("--- generate_mtdparts ---\n");

	if (list_empty(&devices)) {
		buf[0] = '\0';
		return 0;
	}

	list_for_each(dentry, &devices) {
		dev = list_entry(dentry, struct mtd_device, link);

		/* copy mtd_id */
		len = strlen(dev->id->mtd_id) + 1;
		if (len > maxlen)
			goto cleanup;
		memcpy(p, dev->id->mtd_id, len - 1);
		p += len - 1;
		*(p++) = ':';
		maxlen -= len;

		/* format partitions */
		prev_part = NULL;
		part_cnt = 0;
		list_for_each(pentry, &dev->parts) {
			part = list_entry(pentry, struct part_info, link);
			size = part->size;
			offset = part->offset;
			part_cnt++;

			/* partition size */
			memsize_format(tmpbuf, size);
			len = strlen(tmpbuf);
			if (len > maxlen)
				goto cleanup;
			memcpy(p, tmpbuf, len);
			p += len;
			maxlen -= len;


			/* add offset only when there is a gap between
			 * partitions */
			if ((!prev_part && (offset != 0)) ||
					(prev_part && ((prev_part->offset + prev_part->size) != part->offset))) {

				memsize_format(tmpbuf, offset);
				len = strlen(tmpbuf) + 1;
				if (len > maxlen)
					goto cleanup;
				*(p++) = '@';
				memcpy(p, tmpbuf, len - 1);
				p += len - 1;
				maxlen -= len;
			}

			/* copy name only if user supplied */
			if(!part->auto_name) {
				len = strlen(part->name) + 2;
				if (len > maxlen)
					goto cleanup;

				*(p++) = '(';
				memcpy(p, part->name, len - 2);
				p += len - 2;
				*(p++) = ')';
				maxlen -= len;
			}

			/* ro mask flag */
			if (part->mask_flags && MTD_WRITEABLE_CMD) {
				len = 2;
				if (len > maxlen)
					goto cleanup;
				*(p++) = 'r';
				*(p++) = 'o';
				maxlen -= 2;
			}

			/* print ',' separator if there are other partitions
			 * following */
			if (dev->num_parts > part_cnt) {
				if (1 > maxlen)
					goto cleanup;
				*(p++) = ',';
				maxlen--;
			}
			prev_part = part;
		}
		/* print ';' separator if there are other devices following */
		if (dentry->next != &devices) {
			if (1 > maxlen)
				goto cleanup;
			*(p++) = ';';
			maxlen--;
		}
	}

	/* we still have at least one char left, as we decremented maxlen at
	 * the begining */
	*p = '\0';

	return 0;

cleanup:
	last_parts[0] = '\0';
	return 1;
}

/**
 * Call generate_mtdparts to process all devices and generate corresponding
 * mtdparts string, save it in mtdparts environment variable.
 *
 * @param buf output buffer holding generated mtdparts string (output)
 * @param buflen buffer size
 * @return 0 on success, 1 otherwise
 */
static int generate_mtdparts_save(char *buf, u32 buflen)
{
	int ret;

	ret = generate_mtdparts(buf, buflen);

	if ((buf[0] != '\0') && (ret == 0))
		env_set("mtdparts", buf);
	else
		env_set("mtdparts", NULL);

	return ret;
}

#if defined(CONFIG_CMD_MTDPARTS_SHOW_NET_SIZES)
/**
 * Get the net size (w/o bad blocks) of the given partition.
 *
 * @param mtd the mtd info
 * @param part the partition
 * @return the calculated net size of this partition
 */
static uint64_t net_part_size(struct mtd_info *mtd, struct part_info *part)
{
	uint64_t i, net_size = 0;

	if (!mtd->block_isbad)
		return part->size;

	for (i = 0; i < part->size; i += mtd->erasesize) {
		if (!mtd->block_isbad(mtd, part->offset + i))
			net_size += mtd->erasesize;
	}

	return net_size;
}
#endif

static void print_partition_table(void)
{
	struct list_head *dentry, *pentry;
	struct part_info *part;
	struct mtd_device *dev;
	int part_num;

	list_for_each(dentry, &devices) {
		dev = list_entry(dentry, struct mtd_device, link);
		/* list partitions for given device */
		part_num = 0;
#if defined(CONFIG_CMD_MTDPARTS_SHOW_NET_SIZES)
		struct mtd_info *mtd;

		if (get_mtd_info(dev->id->type, dev->id->num, &mtd))
			return;

		printf("\ndevice %s%d <%s>, # parts = %d\n",
				MTD_DEV_TYPE(dev->id->type), dev->id->num,
				dev->id->mtd_id, dev->num_parts);
		printf(" #: name\t\tsize\t\tnet size\toffset\t\tmask_flags\n");

		list_for_each(pentry, &dev->parts) {
			u32 net_size;
			char *size_note;

			part = list_entry(pentry, struct part_info, link);
			net_size = net_part_size(mtd, part);
			size_note = part->size == net_size ? " " : " (!)";
			printf("%2d: %-20s0x%08x\t0x%08x%s\t0x%08x\t%d\n",
					part_num, part->name, part->size,
					net_size, size_note, part->offset,
					part->mask_flags);
#else /* !defined(CONFIG_CMD_MTDPARTS_SHOW_NET_SIZES) */
		printf("\ndevice %s%d <%s>, # parts = %d\n",
				MTD_DEV_TYPE(dev->id->type), dev->id->num,
				dev->id->mtd_id, dev->num_parts);
		printf(" #: name\t\tsize\t\toffset\t\tmask_flags\n");

		list_for_each(pentry, &dev->parts) {
			part = list_entry(pentry, struct part_info, link);
			printf("%2d: %-20s0x%08llx\t0x%08llx\t%d\n",
					part_num, part->name, part->size,
					part->offset, part->mask_flags);
#endif /* defined(CONFIG_CMD_MTDPARTS_SHOW_NET_SIZES) */
			part_num++;
		}
	}

	if (list_empty(&devices))
		printf("no partitions defined\n");
}

/**
 * Format and print out a partition list for each device from global device
 * list.
 */
static void list_partitions(void)
{
	struct part_info *part;

	debug("\n---list_partitions---\n");
	print_partition_table();

	/* current_mtd_dev is not NULL only when we have non empty device list */
	if (current_mtd_dev) {
		part = mtd_part_info(current_mtd_dev, current_mtd_partnum);
		if (part) {
			printf("\nactive partition: %s%d,%d - (%s) 0x%08llx @ 0x%08llx\n",
					MTD_DEV_TYPE(current_mtd_dev->id->type),
					current_mtd_dev->id->num, current_mtd_partnum,
					part->name, part->size, part->offset);
		} else {
			printf("could not get current partition info\n\n");
		}
	}

	printf("\ndefaults:\n");
	printf("mtdids  : %s\n",
		mtdids_default ? mtdids_default : "none");
	/*
	 * Using printf() here results in printbuffer overflow
	 * if default mtdparts string is greater than console
	 * printbuffer. Use puts() to prevent system crashes.
	 */
	puts("mtdparts: ");
	puts(mtdparts_default ? mtdparts_default : "none");
	puts("\n");
}

/**
 * Given partition identifier in form of <dev_type><dev_num>,<part_num> find
 * corresponding device and verify partition number.
 *
 * @param id string describing device and partition or partition name
 * @param dev pointer to the requested device (output)
 * @param part_num verified partition number (output)
 * @param part pointer to requested partition (output)
 * @return 0 on success, 1 otherwise
 */
int find_dev_and_part(const char *id, struct mtd_device **dev,
		u8 *part_num, struct part_info **part)
{
	struct list_head *dentry, *pentry;
	u8 type, dnum, pnum;
	const char *p;

	debug("--- find_dev_and_part ---\nid = %s\n", id);

	list_for_each(dentry, &devices) {
		*part_num = 0;
		*dev = list_entry(dentry, struct mtd_device, link);
		list_for_each(pentry, &(*dev)->parts) {
			*part = list_entry(pentry, struct part_info, link);
			if (strcmp((*part)->name, id) == 0)
				return 0;
			(*part_num)++;
		}
	}

	p = id;
	*dev = NULL;
	*part = NULL;
	*part_num = 0;

	if (mtd_id_parse(p, &p, &type, &dnum) != 0)
		return 1;

	if ((*p++ != ',') || (*p == '\0')) {
		printf("no partition number specified\n");
		return 1;
	}
	pnum = simple_strtoul(p, (char **)&p, 0);
	if (*p != '\0') {
		printf("unexpected trailing character '%c'\n", *p);
		return 1;
	}

	if ((*dev = device_find(type, dnum)) == NULL) {
		printf("no such device %s%d\n", MTD_DEV_TYPE(type), dnum);
		return 1;
	}

	if ((*part = mtd_part_info(*dev, pnum)) == NULL) {
		printf("no such partition\n");
		*dev = NULL;
		return 1;
	}

	*part_num = pnum;

	return 0;
}

/**
 * Find and delete partition. For partition id format see find_dev_and_part().
 *
 * @param id string describing device and partition
 * @return 0 on success, 1 otherwise
 */
static int delete_partition(const char *id)
{
	u8 pnum;
	struct mtd_device *dev;
	struct part_info *part;

	if (find_dev_and_part(id, &dev, &pnum, &part) == 0) {

		debug("delete_partition: device = %s%d, partition %d = (%s) 0x%08llx@0x%08llx\n",
				MTD_DEV_TYPE(dev->id->type), dev->id->num, pnum,
				part->name, part->size, part->offset);

		if (part_del(dev, part) != 0)
			return 1;

		if (generate_mtdparts_save(last_parts, MTDPARTS_MAXLEN) != 0) {
			printf("generated mtdparts too long, resetting to null\n");
			return 1;
		}
		return 0;
	}

	printf("partition %s not found\n", id);
	return 1;
}

#if defined(CONFIG_CMD_MTDPARTS_SPREAD)
/**
 * Increase the size of the given partition so that it's net size is at least
 * as large as the size member and such that the next partition would start on a
 * good block if it were adjacent to this partition.
 *
 * @param mtd the mtd device
 * @param part the partition
 * @param next_offset pointer to the offset of the next partition after this
 *                    partition's size has been modified (output)
 */
static void spread_partition(struct mtd_info *mtd, struct part_info *part,
			     uint64_t *next_offset)
{
	uint64_t net_size, padding_size = 0;
	int truncated;

	mtd_get_len_incl_bad(mtd, part->offset, part->size, &net_size,
			     &truncated);

	/*
	 * Absorb bad blocks immediately following this
	 * partition also into the partition, such that
	 * the next partition starts with a good block.
	 */
	if (!truncated) {
		mtd_get_len_incl_bad(mtd, part->offset + net_size,
				     mtd->erasesize, &padding_size, &truncated);
		if (truncated)
			padding_size = 0;
		else
			padding_size -= mtd->erasesize;
	}

	if (truncated) {
		printf("truncated partition %s to %lld bytes\n", part->name,
		       (uint64_t) net_size + padding_size);
	}

	part->size = net_size + padding_size;
	*next_offset = part->offset + part->size;
}

/**
 * Adjust all of the partition sizes, such that all partitions are at least
 * as big as their mtdparts environment variable sizes and they each start
 * on a good block.
 *
 * @return 0 on success, 1 otherwise
 */
static int spread_partitions(void)
{
	struct list_head *dentry, *pentry;
	struct mtd_device *dev;
	struct part_info *part;
	struct mtd_info *mtd;
	int part_num;
	uint64_t cur_offs;

	list_for_each(dentry, &devices) {
		dev = list_entry(dentry, struct mtd_device, link);

		if (get_mtd_info(dev->id->type, dev->id->num, &mtd))
			return 1;

		part_num = 0;
		cur_offs = 0;
		list_for_each(pentry, &dev->parts) {
			part = list_entry(pentry, struct part_info, link);

			debug("spread_partitions: device = %s%d, partition %d ="
				" (%s) 0x%08llx@0x%08llx\n",
				MTD_DEV_TYPE(dev->id->type), dev->id->num,
				part_num, part->name, part->size,
				part->offset);

			if (cur_offs > part->offset)
				part->offset = cur_offs;

			spread_partition(mtd, part, &cur_offs);

			part_num++;
		}
	}

	index_partitions();

	if (generate_mtdparts_save(last_parts, MTDPARTS_MAXLEN) != 0) {
		printf("generated mtdparts too long, resetting to null\n");
		return 1;
	}
	return 0;
}
#endif /* CONFIG_CMD_MTDPARTS_SPREAD */

/**
 * The mtdparts variable tends to be long. If we need to access it
 * before the env is relocated, then we need to use our own stack
 * buffer.  gd->env_buf will be too small.
 *
 * @param buf temporary buffer pointer MTDPARTS_MAXLEN long
 * @return mtdparts variable string, NULL if not found
 */
static const char *env_get_mtdparts(char *buf)
{
	if (gd->flags & GD_FLG_ENV_READY)
		return env_get("mtdparts");
	if (env_get_f("mtdparts", buf, MTDPARTS_MAXLEN) != -1)
		return buf;
	return NULL;
}

/**
 * Accept character string describing mtd partitions and call device_parse()
 * for each entry. Add created devices to the global devices list.
 *
 * @param mtdparts string specifing mtd partitions
 * @return 0 on success, 1 otherwise
 */
static int parse_mtdparts(const char *const mtdparts)
{
	const char *p;
	struct mtd_device *dev;
	int err = 1;
	char tmp_parts[MTDPARTS_MAXLEN];

	debug("\n---parse_mtdparts---\nmtdparts = %s\n\n", mtdparts);

	/* delete all devices and partitions */
	if (mtd_devices_init() != 0) {
		printf("could not initialise device list\n");
		return err;
	}

	/* re-read 'mtdparts' variable, mtd_devices_init may be updating env */
	p = env_get_mtdparts(tmp_parts);
	if (!p)
		p = mtdparts;

	/* Skip the useless prefix, if any */
	if (strncmp(p, "mtdparts=", 9) == 0)
		p += 9;

	while (*p != '\0') {
		err = 1;
		if ((device_parse(p, &p, &dev) != 0) || (!dev))
			break;

		debug("+ device: %s\t%d\t%s\n", MTD_DEV_TYPE(dev->id->type),
				dev->id->num, dev->id->mtd_id);

		/* check if parsed device is already on the list */
		if (device_find(dev->id->type, dev->id->num) != NULL) {
			printf("device %s%d redefined, please correct mtdparts variable\n",
					MTD_DEV_TYPE(dev->id->type), dev->id->num);
			break;
		}

		list_add_tail(&dev->link, &devices);
		err = 0;
	}
	if (err == 1) {
		free(dev);
		device_delall(&devices);
	}

	return err;
}

/**
 * Parse provided string describing mtdids mapping (see file header for mtdids
 * variable format). Allocate memory for each entry and add all found entries
 * to the global mtdids list.
 *
 * @param ids mapping string
 * @return 0 on success, 1 otherwise
 */
static int parse_mtdids(const char *const ids)
{
	const char *p = ids;
	const char *mtd_id;
	int mtd_id_len;
	struct mtdids *id;
	struct list_head *entry, *n;
	struct mtdids *id_tmp;
	u8 type, num;
	u64 size;
	int ret = 1;

	debug("\n---parse_mtdids---\nmtdids = %s\n\n", ids);

	/* clean global mtdids list */
	list_for_each_safe(entry, n, &mtdids) {
		id_tmp = list_entry(entry, struct mtdids, link);
		debug("mtdids del: %d %d\n", id_tmp->type, id_tmp->num);
		list_del(entry);
		free(id_tmp);
	}
	last_ids[0] = '\0';
	INIT_LIST_HEAD(&mtdids);

	while(p && (*p != '\0')) {

		ret = 1;
		/* parse 'nor'|'nand'|'onenand'|'spi-nand'<dev-num> */
		if (mtd_id_parse(p, &p, &type, &num) != 0)
			break;

		if (*p != '=') {
			printf("mtdids: incorrect <dev-num>\n");
			break;
		}
		p++;

		/* check if requested device exists */
		if (mtd_device_validate(type, num, &size) != 0)
			return 1;

		/* locate <mtd-id> */
		mtd_id = p;
		if ((p = strchr(mtd_id, ',')) != NULL) {
			mtd_id_len = p - mtd_id + 1;
			p++;
		} else {
			mtd_id_len = strlen(mtd_id) + 1;
		}
		if (mtd_id_len == 0) {
			printf("mtdids: no <mtd-id> identifier\n");
			break;
		}

		/* check if this id is already on the list */
		int double_entry = 0;
		list_for_each(entry, &mtdids) {
			id_tmp = list_entry(entry, struct mtdids, link);
			if ((id_tmp->type == type) && (id_tmp->num == num)) {
				double_entry = 1;
				break;
			}
		}
		if (double_entry) {
			printf("device id %s%d redefined, please correct mtdids variable\n",
					MTD_DEV_TYPE(type), num);
			break;
		}

		/* allocate mtdids structure */
		if (!(id = (struct mtdids *)malloc(sizeof(struct mtdids) + mtd_id_len))) {
			printf("out of memory\n");
			break;
		}
		memset(id, 0, sizeof(struct mtdids) + mtd_id_len);
		id->num = num;
		id->type = type;
		id->size = size;
		id->mtd_id = (char *)(id + 1);
		strncpy(id->mtd_id, mtd_id, mtd_id_len - 1);
		id->mtd_id[mtd_id_len - 1] = '\0';
		INIT_LIST_HEAD(&id->link);

		debug("+ id %s%d\t%16lld bytes\t%s\n",
				MTD_DEV_TYPE(id->type), id->num,
				id->size, id->mtd_id);

		list_add_tail(&id->link, &mtdids);
		ret = 0;
	}
	if (ret == 1) {
		/* clean mtdids list and free allocated memory */
		list_for_each_safe(entry, n, &mtdids) {
			id_tmp = list_entry(entry, struct mtdids, link);
			list_del(entry);
			free(id_tmp);
		}
		return 1;
	}

	return 0;
}


/**
 * Parse and initialize global mtdids mapping and create global
 * device/partition list.
 *
 * @return 0 on success, 1 otherwise
 */
int mtdparts_init(void)
{
	static int initialized = 0;
	const char *ids, *parts;
	const char *current_partition;
	int ids_changed;
	char tmp_ep[PARTITION_MAXLEN + 1];
	char tmp_parts[MTDPARTS_MAXLEN];

	debug("\n---mtdparts_init---\n");
	if (!initialized) {
		INIT_LIST_HEAD(&mtdids);
		INIT_LIST_HEAD(&devices);
		memset(last_ids, 0, sizeof(last_ids));
		memset(last_parts, 0, sizeof(last_parts));
		memset(last_partition, 0, sizeof(last_partition));
#if defined(CONFIG_SYS_MTDPARTS_RUNTIME)
		board_mtdparts_default(&mtdids_default, &mtdparts_default);
#endif
		use_defaults = 1;
		initialized = 1;
	}

	/* get variables */
	ids = env_get("mtdids");
	parts = env_get_mtdparts(tmp_parts);
	current_partition = env_get("partition");

	/* save it for later parsing, cannot rely on current partition pointer
	 * as 'partition' variable may be updated during init */
	memset(tmp_parts, 0, sizeof(tmp_parts));
	memset(tmp_ep, 0, sizeof(tmp_ep));
	if (current_partition)
		strncpy(tmp_ep, current_partition, PARTITION_MAXLEN);

	debug("last_ids  : %s\n", last_ids);
	debug("env_ids   : %s\n", ids);
	debug("last_parts: %s\n", last_parts);
	debug("env_parts : %s\n\n", parts);

	debug("last_partition : %s\n", last_partition);
	debug("env_partition  : %s\n", current_partition);

	/* if mtdids variable is empty try to use defaults */
	if (!ids) {
		if (mtdids_default) {
			debug("mtdids variable not defined, using default\n");
			ids = mtdids_default;
			env_set("mtdids", (char *)ids);
		} else {
			printf("mtdids not defined, no default present\n");
			return 1;
		}
	}
	if (strlen(ids) > MTDIDS_MAXLEN - 1) {
		printf("mtdids too long (> %d)\n", MTDIDS_MAXLEN);
		return 1;
	}

	/* use defaults when mtdparts variable is not defined
	 * once mtdparts is saved environment, drop use_defaults flag */
	if (!parts) {
		if (mtdparts_default && use_defaults) {
			parts = mtdparts_default;
			if (env_set("mtdparts", (char *)parts) == 0)
				use_defaults = 0;
		} else
			printf("mtdparts variable not set, see 'help mtdparts'\n");
	}

	if (parts && (strlen(parts) > MTDPARTS_MAXLEN - 1)) {
		printf("mtdparts too long (> %d)\n", MTDPARTS_MAXLEN);
		return 1;
	}

	/* check if we have already parsed those mtdids */
	if ((last_ids[0] != '\0') && (strcmp(last_ids, ids) == 0)) {
		ids_changed = 0;
	} else {
		ids_changed = 1;

		if (parse_mtdids(ids) != 0) {
			mtd_devices_init();
			return 1;
		}

		/* ok it's good, save new ids */
		strncpy(last_ids, ids, MTDIDS_MAXLEN);
	}

	/* parse partitions if either mtdparts or mtdids were updated */
	if (parts && ((last_parts[0] == '\0') || ((strcmp(last_parts, parts) != 0)) || ids_changed)) {
		if (parse_mtdparts(parts) != 0)
			return 1;

		if (list_empty(&devices)) {
			printf("mtdparts_init: no valid partitions\n");
			return 1;
		}

		/* ok it's good, save new parts */
		strncpy(last_parts, parts, MTDPARTS_MAXLEN);

		/* reset first partition from first dev from the list as current */
		current_mtd_dev = list_entry(devices.next, struct mtd_device, link);
		current_mtd_partnum = 0;
		current_save();

		debug("mtdparts_init: current_mtd_dev  = %s%d, current_mtd_partnum = %d\n",
				MTD_DEV_TYPE(current_mtd_dev->id->type),
				current_mtd_dev->id->num, current_mtd_partnum);
	}

	/* mtdparts variable was reset to NULL, delete all devices/partitions */
	if (!parts && (last_parts[0] != '\0'))
		return mtd_devices_init();

	/* do not process current partition if mtdparts variable is null */
	if (!parts)
		return 0;

	/* is current partition set in environment? if so, use it */
	if ((tmp_ep[0] != '\0') && (strcmp(tmp_ep, last_partition) != 0)) {
		struct part_info *p;
		struct mtd_device *cdev;
		u8 pnum;

		debug("--- getting current partition: %s\n", tmp_ep);

		if (find_dev_and_part(tmp_ep, &cdev, &pnum, &p) == 0) {
			current_mtd_dev = cdev;
			current_mtd_partnum = pnum;
			current_save();
		}
	} else if (env_get("partition") == NULL) {
		debug("no partition variable set, setting...\n");
		current_save();
	}

	return 0;
}

/**
 * Return pointer to the partition of a requested number from a requested
 * device.
 *
 * @param dev device that is to be searched for a partition
 * @param part_num requested partition number
 * @return pointer to the part_info, NULL otherwise
 */
static struct part_info* mtd_part_info(struct mtd_device *dev, unsigned int part_num)
{
	struct list_head *entry;
	struct part_info *part;
	int num;

	if (!dev)
		return NULL;

	debug("\n--- mtd_part_info: partition number %d for device %s%d (%s)\n",
			part_num, MTD_DEV_TYPE(dev->id->type),
			dev->id->num, dev->id->mtd_id);

	if (part_num >= dev->num_parts) {
		printf("invalid partition number %d for device %s%d (%s)\n",
				part_num, MTD_DEV_TYPE(dev->id->type),
				dev->id->num, dev->id->mtd_id);
		return NULL;
	}

	/* locate partition number, return it */
	num = 0;
	list_for_each(entry, &dev->parts) {
		part = list_entry(entry, struct part_info, link);

		if (part_num == num++) {
			return part;
		}
	}

	return NULL;
}

/***************************************************/
/* U-Boot commands				   */
/***************************************************/
/* command line only */
/**
 * Routine implementing u-boot chpart command. Sets new current partition based
 * on the user supplied partition id. For partition id format see find_dev_and_part().
 *
 * @param cmdtp command internal data
 * @param flag command flag
 * @param argc number of arguments supplied to the command
 * @param argv arguments list
 * @return 0 on success, 1 otherwise
 */
static int do_chpart(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
/* command line only */
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;

	if (mtdparts_init() !=0)
		return 1;

	if (argc < 2) {
		printf("no partition id specified\n");
		return 1;
	}

	if (find_dev_and_part(argv[1], &dev, &pnum, &part) != 0)
		return 1;

	current_mtd_dev = dev;
	current_mtd_partnum = pnum;
	current_save();

	printf("partition changed to %s%d,%d\n",
			MTD_DEV_TYPE(dev->id->type), dev->id->num, pnum);

	return 0;
}

/**
 * Routine implementing u-boot mtdparts command. Initialize/update default global
 * partition list and process user partition request (list, add, del).
 *
 * @param cmdtp command internal data
 * @param flag command flag
 * @param argc number of arguments supplied to the command
 * @param argv arguments list
 * @return 0 on success, 1 otherwise
 */
static int do_mtdparts(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	if (argc == 2) {
		if (strcmp(argv[1], "default") == 0) {
			env_set("mtdids", NULL);
			env_set("mtdparts", NULL);
			env_set("partition", NULL);
			use_defaults = 1;

			mtdparts_init();
			return 0;
		} else if (strcmp(argv[1], "delall") == 0) {
			/* this may be the first run, initialize lists if needed */
			mtdparts_init();

			env_set("mtdparts", NULL);

			/* mtd_devices_init() calls current_save() */
			return mtd_devices_init();
		}
	}

	/* make sure we are in sync with env variables */
	if (mtdparts_init() != 0)
		return 1;

	if (argc == 1) {
		list_partitions();
		return 0;
	}

	/* mtdparts add <mtd-dev> <size>[@<offset>] <name> [ro] */
	if (((argc == 5) || (argc == 6)) && (strncmp(argv[1], "add", 3) == 0)) {
#define PART_ADD_DESC_MAXLEN 64
		char tmpbuf[PART_ADD_DESC_MAXLEN];
#if defined(CONFIG_CMD_MTDPARTS_SPREAD)
		struct mtd_info *mtd;
		uint64_t next_offset;
#endif
		u8 type, num, len;
		struct mtd_device *dev;
		struct mtd_device *dev_tmp;
		struct mtdids *id;
		struct part_info *p;

		if (mtd_id_parse(argv[2], NULL, &type, &num) != 0)
			return 1;

		if ((id = id_find(type, num)) == NULL) {
			printf("no such device %s defined in mtdids variable\n", argv[2]);
			return 1;
		}

		len = strlen(id->mtd_id) + 1;	/* 'mtd_id:' */
		len += strlen(argv[3]);		/* size@offset */
		len += strlen(argv[4]) + 2;	/* '(' name ')' */
		if (argv[5] && (strlen(argv[5]) == 2))
			len += 2;		/* 'ro' */

		if (len >= PART_ADD_DESC_MAXLEN) {
			printf("too long partition description\n");
			return 1;
		}
		sprintf(tmpbuf, "%s:%s(%s)%s",
				id->mtd_id, argv[3], argv[4], argv[5] ? argv[5] : "");
		debug("add tmpbuf: %s\n", tmpbuf);

		if ((device_parse(tmpbuf, NULL, &dev) != 0) || (!dev))
			return 1;

		debug("+ %s\t%d\t%s\n", MTD_DEV_TYPE(dev->id->type),
				dev->id->num, dev->id->mtd_id);

		p = list_entry(dev->parts.next, struct part_info, link);

#if defined(CONFIG_CMD_MTDPARTS_SPREAD)
		if (get_mtd_info(dev->id->type, dev->id->num, &mtd))
			return 1;

		if (!strcmp(&argv[1][3], ".spread")) {
			spread_partition(mtd, p, &next_offset);
			debug("increased %s to %llu bytes\n", p->name, p->size);
		}
#endif

		dev_tmp = device_find(dev->id->type, dev->id->num);
		if (dev_tmp == NULL) {
			device_add(dev);
		} else if (part_add(dev_tmp, p) != 0) {
			/* merge new partition with existing ones*/
			device_del(dev);
			return 1;
		}

		if (generate_mtdparts_save(last_parts, MTDPARTS_MAXLEN) != 0) {
			printf("generated mtdparts too long, resetting to null\n");
			return 1;
		}

		return 0;
	}

	/* mtdparts del part-id */
	if ((argc == 3) && (strcmp(argv[1], "del") == 0)) {
		debug("del: part-id = %s\n", argv[2]);

		return delete_partition(argv[2]);
	}

#if defined(CONFIG_CMD_MTDPARTS_SPREAD)
	if ((argc == 2) && (strcmp(argv[1], "spread") == 0))
		return spread_partitions();
#endif /* CONFIG_CMD_MTDPARTS_SPREAD */

	return CMD_RET_USAGE;
}

/***************************************************/
U_BOOT_CMD(
	chpart,	2,	0,	do_chpart,
	"change active partition",
	"part-id\n"
	"    - change active partition (e.g. part-id = nand0,1)"
);

#ifdef CONFIG_SYS_LONGHELP
static char mtdparts_help_text[] =
	"\n"
	"    - list partition table\n"
	"mtdparts delall\n"
	"    - delete all partitions\n"
	"mtdparts del part-id\n"
	"    - delete partition (e.g. part-id = nand0,1)\n"
	"mtdparts add <mtd-dev> <size>[@<offset>] [<name>] [ro]\n"
	"    - add partition\n"
#if defined(CONFIG_CMD_MTDPARTS_SPREAD)
	"mtdparts add.spread <mtd-dev> <size>[@<offset>] [<name>] [ro]\n"
	"    - add partition, padding size by skipping bad blocks\n"
#endif
	"mtdparts default\n"
	"    - reset partition table to defaults\n"
#if defined(CONFIG_CMD_MTDPARTS_SPREAD)
	"mtdparts spread\n"
	"    - adjust the sizes of the partitions so they are\n"
	"      at least as big as the mtdparts variable specifies\n"
	"      and they each start on a good block\n\n"
#else
	"\n"
#endif /* CONFIG_CMD_MTDPARTS_SPREAD */
	"-----\n\n"
	"this command uses three environment variables:\n\n"
	"'partition' - keeps current partition identifier\n\n"
	"partition  := <part-id>\n"
	"<part-id>  := <dev-id>,part_num\n\n"
	"'mtdids' - linux kernel mtd device id <-> u-boot device id mapping\n\n"
	"mtdids=<idmap>[,<idmap>,...]\n\n"
	"<idmap>    := <dev-id>=<mtd-id>\n"
	"<dev-id>   := 'nand'|'nor'|'onenand'|'spi-nand'<dev-num>\n"
	"<dev-num>  := mtd device number, 0...\n"
	"<mtd-id>   := unique device tag used by linux kernel to find mtd device (mtd->name)\n\n"
	"'mtdparts' - partition list\n\n"
	"mtdparts=mtdparts=<mtd-def>[;<mtd-def>...]\n\n"
	"<mtd-def>  := <mtd-id>:<part-def>[,<part-def>...]\n"
	"<mtd-id>   := unique device tag used by linux kernel to find mtd device (mtd->name)\n"
	"<part-def> := <size>[@<offset>][<name>][<ro-flag>]\n"
	"<size>     := standard linux memsize OR '-' to denote all remaining space\n"
	"<offset>   := partition start offset within the device\n"
	"<name>     := '(' NAME ')'\n"
	"<ro-flag>  := when set to 'ro' makes partition read-only (not used, passed to kernel)";
#endif

U_BOOT_CMD(
	mtdparts,	6,	0,	do_mtdparts,
	"define flash/nand partitions", mtdparts_help_text
);
/***************************************************/
