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
 * <dev-id>   := 'nand'|'nor'|'onenand'<dev-num>
 * <dev-num>  := mtd device number, 0...
 * <mtd-id>   := unique device tag used by linux kernel to find mtd device (mtd->name)
 *
 *
 * 'mtdparts' - partition list
 *
 * mtdparts=mtdparts=<mtd-def>[;<mtd-def>...]
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
 * mtdparts=mtdparts=edb7312-nor:-
 *
 * 1 NOR Flash with 2 partitions, 1 NAND with one
 * mtdids=nor0=edb7312-nor,nand0=edb7312-nand
 * mtdparts=mtdparts=edb7312-nor:256k(ARMboot)ro,-(root);edb7312-nand:-(home)
 *
 */

/*
 * JFFS2/CRAMFS support
 */
#include <common.h>
#include <command.h>
#include <env.h>
#if defined(CONFIG_CMD_FLASH)
#include <flash.h>
#endif
#include <image.h>
#include <malloc.h>
#include <jffs2/jffs2.h>
#include <linux/bug.h>
#include <linux/list.h>
#include <linux/ctype.h>
#include <cramfs/cramfs_fs.h>

#if defined(CONFIG_CMD_NAND)
#include <linux/mtd/rawnand.h>
#include <nand.h>
#endif

#if defined(CONFIG_CMD_ONENAND)
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>
#include <onenand_uboot.h>
#endif

/* enable/disable debugging messages */
#define	DEBUG_JFFS
#undef	DEBUG_JFFS

#ifdef  DEBUG_JFFS
# define DEBUGF(fmt, args...)	printf(fmt ,##args)
#else
# define DEBUGF(fmt, args...)
#endif

/* special size referring to all the remaining space in a partition */
#define SIZE_REMAINING		0xFFFFFFFF

/* special offset value, it is used when not provided by user
 *
 * this value is used temporarily during parsing, later such offests
 * are recalculated */
#define OFFSET_NOT_SPECIFIED	0xFFFFFFFF

/* minimum partition size */
#define MIN_PART_SIZE		4096

/* this flag needs to be set in part_info struct mask_flags
 * field for read-only partitions */
#define MTD_WRITEABLE_CMD		1

/* current active device and partition number */
#ifdef CONFIG_CMD_MTDPARTS
/* Use the ones declared in cmd_mtdparts.c */
extern struct mtd_device *current_mtd_dev;
extern u8 current_mtd_partnum;
#else
/* Use local ones */
struct mtd_device *current_mtd_dev = NULL;
u8 current_mtd_partnum = 0;
#endif

#if defined(CONFIG_CMD_CRAMFS)
extern int cramfs_check (struct part_info *info);
extern int cramfs_load (char *loadoffset, struct part_info *info, char *filename);
extern int cramfs_ls (struct part_info *info, char *filename);
extern int cramfs_info (struct part_info *info);
#else
/* defining empty macros for function names is ugly but avoids ifdef clutter
 * all over the code */
#define cramfs_check(x)		(0)
#define cramfs_load(x,y,z)	(-1)
#define cramfs_ls(x,y)		(0)
#define cramfs_info(x)		(0)
#endif

#ifndef CONFIG_CMD_MTDPARTS
/**
 * Check device number to be within valid range for given device type.
 *
 * @param dev device to validate
 * Return: 0 if device is valid, 1 otherwise
 */
static int mtd_device_validate(u8 type, u8 num, u32 *size)
{
	if (type == MTD_DEV_TYPE_NOR) {
#if defined(CONFIG_CMD_FLASH)
		if (num < CONFIG_SYS_MAX_FLASH_BANKS) {
			*size = flash_info[num].size;

			return 0;
		}

		printf("no such FLASH device: %s%d (valid range 0 ... %d\n",
				MTD_DEV_TYPE(type), num, CONFIG_SYS_MAX_FLASH_BANKS - 1);
#else
		printf("support for FLASH devices not present\n");
#endif
	} else if (type == MTD_DEV_TYPE_NAND) {
#if defined(CONFIG_JFFS2_NAND) && defined(CONFIG_CMD_NAND)
		struct mtd_info *mtd = get_nand_dev_by_index(num);
		if (mtd) {
			*size = mtd->size;
			return 0;
		}

		printf("no such NAND device: %s%d (valid range 0 ... %d)\n",
				MTD_DEV_TYPE(type), num, CONFIG_SYS_MAX_NAND_DEVICE - 1);
#else
		printf("support for NAND devices not present\n");
#endif
	} else if (type == MTD_DEV_TYPE_ONENAND) {
#if defined(CONFIG_CMD_ONENAND)
		*size = onenand_mtd.size;
		return 0;
#else
		printf("support for OneNAND devices not present\n");
#endif
	} else
		printf("Unknown defice type %d\n", type);

	return 1;
}

/**
 * Parse device id string <dev-id> := 'nand'|'nor'|'onenand'<dev-num>,
 * return device type and number.
 *
 * @param id string describing device id
 * @param ret_id output pointer to next char after parse completes (output)
 * @param dev_type parsed device type (output)
 * @param dev_num parsed device number (output)
 * Return: 0 on success, 1 otherwise
 */
static int mtd_id_parse(const char *id, const char **ret_id, u8 *dev_type, u8 *dev_num)
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

/*
 * 'Static' version of command line mtdparts_init() routine. Single partition on
 * a single device configuration.
 */

/**
 * Calculate sector size.
 *
 * Return: sector size
 */
static inline u32 get_part_sector_size_nand(struct mtdids *id)
{
#if defined(CONFIG_JFFS2_NAND) && defined(CONFIG_CMD_NAND)
	struct mtd_info *mtd;

	mtd = get_nand_dev_by_index(id->num);

	return mtd->erasesize;
#else
	BUG();
	return 0;
#endif
}

static inline u32 get_part_sector_size_nor(struct mtdids *id, struct part_info *part)
{
#if defined(CONFIG_CMD_FLASH)
	u32 end_phys, start_phys, sector_size = 0, size = 0;
	int i;
	flash_info_t *flash;

	flash = &flash_info[id->num];

	start_phys = flash->start[0] + part->offset;
	end_phys = start_phys + part->size - 1;

	for (i = 0; i < flash->sector_count; i++) {
		if (flash->start[i] >= end_phys)
			break;

		if (flash->start[i] >= start_phys) {
			if (i == flash->sector_count - 1) {
				size = flash->start[0] + flash->size - flash->start[i];
			} else {
				size = flash->start[i+1] - flash->start[i];
			}

			if (sector_size < size)
				sector_size = size;
		}
	}

	return sector_size;
#else
	BUG();
	return 0;
#endif
}

static inline u32 get_part_sector_size_onenand(void)
{
#if defined(CONFIG_CMD_ONENAND)
	struct mtd_info *mtd;

	mtd = &onenand_mtd;

	return mtd->erasesize;
#else
	BUG();
	return 0;
#endif
}

static inline u32 get_part_sector_size(struct mtdids *id, struct part_info *part)
{
	if (id->type == MTD_DEV_TYPE_NAND)
		return get_part_sector_size_nand(id);
	else if (id->type == MTD_DEV_TYPE_NOR)
		return get_part_sector_size_nor(id, part);
	else if (id->type == MTD_DEV_TYPE_ONENAND)
		return get_part_sector_size_onenand();
	else
		DEBUGF("Error: Unknown device type.\n");

	return 0;
}

/**
 * Parse and initialize global mtdids mapping and create global
 * device/partition list.
 *
 * 'Static' version of command line mtdparts_init() routine. Single partition on
 * a single device configuration.
 *
 * Return: 0 on success, 1 otherwise
 */
int mtdparts_init(void)
{
	static int initialized = 0;
	u32 size;
	char *dev_name;

	DEBUGF("\n---mtdparts_init---\n");
	if (!initialized) {
		struct mtdids *id;
		struct part_info *part;

		initialized = 1;
		current_mtd_dev = (struct mtd_device *)
			malloc(sizeof(struct mtd_device) +
					sizeof(struct part_info) +
					sizeof(struct mtdids));
		if (!current_mtd_dev) {
			printf("out of memory\n");
			return 1;
		}
		memset(current_mtd_dev, 0, sizeof(struct mtd_device) +
		       sizeof(struct part_info) + sizeof(struct mtdids));

		id = (struct mtdids *)(current_mtd_dev + 1);
		part = (struct part_info *)(id + 1);

		/* id */
		id->mtd_id = "single part";

		dev_name = CONFIG_JFFS2_DEV;

		if ((mtd_id_parse(dev_name, NULL, &id->type, &id->num) != 0) ||
				(mtd_device_validate(id->type, id->num, &size) != 0)) {
			printf("incorrect device: %s%d\n", MTD_DEV_TYPE(id->type), id->num);
			free(current_mtd_dev);
			return 1;
		}
		id->size = size;
		INIT_LIST_HEAD(&id->link);

		DEBUGF("dev id: type = %d, num = %d, size = 0x%08lx, mtd_id = %s\n",
				id->type, id->num, id->size, id->mtd_id);

		/* partition */
		part->name = "static";
		part->auto_name = 0;

		part->size = CONFIG_JFFS2_PART_SIZE;

		part->offset = CONFIG_JFFS2_PART_OFFSET;

		part->dev = current_mtd_dev;
		INIT_LIST_HEAD(&part->link);

		/* recalculate size if needed */
		if (part->size == SIZE_REMAINING)
			part->size = id->size - part->offset;

		part->sector_size = get_part_sector_size(id, part);

		DEBUGF("part  : name = %s, size = 0x%08lx, offset = 0x%08lx\n",
				part->name, part->size, part->offset);

		/* device */
		current_mtd_dev->id = id;
		INIT_LIST_HEAD(&current_mtd_dev->link);
		current_mtd_dev->num_parts = 1;
		INIT_LIST_HEAD(&current_mtd_dev->parts);
		list_add(&part->link, &current_mtd_dev->parts);
	}

	return 0;
}
#endif /* #ifndef CONFIG_CMD_MTDPARTS */

/**
 * Return pointer to the partition of a requested number from a requested
 * device.
 *
 * @param dev device that is to be searched for a partition
 * @param part_num requested partition number
 * Return: pointer to the part_info, NULL otherwise
 */
static struct part_info* jffs2_part_info(struct mtd_device *dev, unsigned int part_num)
{
	struct list_head *entry;
	struct part_info *part;
	int num;

	if (!dev)
		return NULL;

	DEBUGF("\n--- jffs2_part_info: partition number %d for device %s%d (%s)\n",
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

/**
 * Routine implementing fsload u-boot command. This routine tries to load
 * a requested file from jffs2/cramfs filesystem on a current partition.
 *
 * @param cmdtp command internal data
 * @param flag command flag
 * @param argc number of arguments supplied to the command
 * @param argv arguments list
 * Return: 0 on success, 1 otherwise
 */
int do_jffs2_fsload(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	char *fsname;
	char *filename;
	int size;
	struct part_info *part;
	ulong offset = image_load_addr;

	/* pre-set Boot file name */
	filename = env_get("bootfile");
	if (!filename)
		filename = "uImage";

	if (argc == 2) {
		filename = argv[1];
	}
	if (argc == 3) {
		offset = hextoul(argv[1], NULL);
		image_load_addr = offset;
		filename = argv[2];
	}

	/* make sure we are in sync with env variables */
	if (mtdparts_init() !=0)
		return 1;

	if ((part = jffs2_part_info(current_mtd_dev, current_mtd_partnum))){

		/* check partition type for cramfs */
		fsname = (cramfs_check(part) ? "CRAMFS" : "JFFS2");
		printf("### %s loading '%s' to 0x%lx\n", fsname, filename, offset);

		if (cramfs_check(part)) {
			size = cramfs_load ((char *) offset, part, filename);
		} else {
			/* if this is not cramfs assume jffs2 */
			size = jffs2_1pass_load((char *)offset, part, filename);
		}

		if (size > 0) {
			printf("### %s load complete: %d bytes loaded to 0x%lx\n",
				fsname, size, offset);
			env_set_hex("filesize", size);
		} else {
			printf("### %s LOAD ERROR<%x> for %s!\n", fsname, size, filename);
		}

		return !(size > 0);
	}
	return 1;
}

/**
 * Routine implementing u-boot ls command which lists content of a given
 * directory on a current partition.
 *
 * @param cmdtp command internal data
 * @param flag command flag
 * @param argc number of arguments supplied to the command
 * @param argv arguments list
 * Return: 0 on success, 1 otherwise
 */
int do_jffs2_ls(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char *filename = "/";
	int ret;
	struct part_info *part;

	if (argc == 2)
		filename = argv[1];

	/* make sure we are in sync with env variables */
	if (mtdparts_init() !=0)
		return 1;

	if ((part = jffs2_part_info(current_mtd_dev, current_mtd_partnum))){

		/* check partition type for cramfs */
		if (cramfs_check(part)) {
			ret = cramfs_ls (part, filename);
		} else {
			/* if this is not cramfs assume jffs2 */
			ret = jffs2_1pass_ls(part, filename);
		}

		return ret ? 0 : 1;
	}
	return 1;
}

/**
 * Routine implementing u-boot fsinfo command. This routine prints out
 * miscellaneous filesystem informations/statistics.
 *
 * @param cmdtp command internal data
 * @param flag command flag
 * @param argc number of arguments supplied to the command
 * @param argv arguments list
 * Return: 0 on success, 1 otherwise
 */
int do_jffs2_fsinfo(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	struct part_info *part;
	char *fsname;
	int ret;

	/* make sure we are in sync with env variables */
	if (mtdparts_init() !=0)
		return 1;

	if ((part = jffs2_part_info(current_mtd_dev, current_mtd_partnum))){

		/* check partition type for cramfs */
		fsname = (cramfs_check(part) ? "CRAMFS" : "JFFS2");
		printf("### filesystem type is %s\n", fsname);

		if (cramfs_check(part)) {
			ret = cramfs_info (part);
		} else {
			/* if this is not cramfs assume jffs2 */
			ret = jffs2_1pass_info(part);
		}

		return ret ? 0 : 1;
	}
	return 1;
}

/***************************************************/
U_BOOT_CMD(
	fsload,	3,	0,	do_jffs2_fsload,
	"load binary file from a filesystem image",
	"[ off ] [ filename ]\n"
	"    - load binary file from flash bank\n"
	"      with offset 'off'"
);
U_BOOT_CMD(
	fsls,	2,	1,	do_jffs2_ls,
	"list files in a directory (default /)",
	"[ directory ]"
);

U_BOOT_CMD(
	fsinfo,	1,	1,	do_jffs2_fsinfo,
	"print information about filesystems",
	""
);
/***************************************************/
