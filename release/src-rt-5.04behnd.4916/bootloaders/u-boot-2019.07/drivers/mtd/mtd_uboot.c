// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */
#include <common.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <jffs2/jffs2.h> /* LEGACY */
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <mtd.h>

#define MTD_NAME_MAX_LEN 20

void board_mtdparts_default(const char **mtdids, const char **mtdparts);

static const char *get_mtdids(void)
{
	__maybe_unused const char *mtdparts = NULL;
	const char *mtdids = env_get("mtdids");

	if (mtdids)
		return mtdids;

#if defined(CONFIG_SYS_MTDPARTS_RUNTIME)
	board_mtdparts_default(&mtdids, &mtdparts);
#elif defined(MTDIDS_DEFAULT)
	mtdids = MTDIDS_DEFAULT;
#elif defined(CONFIG_MTDIDS_DEFAULT)
	mtdids = CONFIG_MTDIDS_DEFAULT;
#endif

	if (mtdids)
		env_set("mtdids", mtdids);

	return mtdids;
}

/**
 * mtd_search_alternate_name - Search an alternate name for @mtdname thanks to
 *                             the mtdids legacy environment variable.
 *
 * The mtdids string is a list of comma-separated 'dev_id=mtd_id' tupples.
 * Check if one of the mtd_id matches mtdname, in this case save dev_id in
 * altname.
 *
 * @mtdname: Current MTD device name
 * @altname: Alternate name to return
 * @max_len: Length of the alternate name buffer
 *
 * @return 0 on success, an error otherwise.
 */
int mtd_search_alternate_name(const char *mtdname, char *altname,
			      unsigned int max_len)
{
	const char *mtdids, *equal, *comma, *dev_id, *mtd_id;
	int dev_id_len, mtd_id_len;

	mtdids = get_mtdids();
	if (!mtdids)
		return -EINVAL;

	do {
		/* Find the '=' sign */
		dev_id = mtdids;
		equal = strchr(dev_id, '=');
		if (!equal)
			break;
		dev_id_len = equal - mtdids;
		mtd_id = equal + 1;

		/* Find the end of the tupple */
		comma = strchr(mtdids, ',');
		if (comma)
			mtd_id_len = comma - mtd_id;
		else
			mtd_id_len = &mtdids[strlen(mtdids)] - mtd_id + 1;

		if (!dev_id_len || !mtd_id_len)
			return -EINVAL;

		if (dev_id_len + 1 > max_len)
			continue;

		/* Compare the name we search with the current mtd_id */
		if (!strncmp(mtdname, mtd_id, mtd_id_len)) {
			strncpy(altname, dev_id, dev_id_len);
			altname[dev_id_len] = 0;

			return 0;
		}

		/* Go to the next tupple */
		mtdids = comma + 1;
	} while (comma);

	return -EINVAL;
}

#if IS_ENABLED(CONFIG_MTD)
static void mtd_probe_uclass_mtd_devs(void)
{
	struct udevice *dev;
	int idx = 0;

	/* Probe devices with DM compliant drivers */
	while (!uclass_find_device(UCLASS_MTD, idx, &dev) && dev) {
		mtd_probe(dev);
		idx++;
	}
}
#else
static void mtd_probe_uclass_mtd_devs(void) { }
#endif

#if defined(CONFIG_MTD_PARTITIONS)

#define MTDPARTS_MAXLEN         512

static const char *get_mtdparts(void)
{
	__maybe_unused const char *mtdids = NULL;
	static char tmp_parts[MTDPARTS_MAXLEN];
	const char *mtdparts = NULL;

	if (gd->flags & GD_FLG_ENV_READY)
		mtdparts = env_get("mtdparts");
	else if (env_get_f("mtdparts", tmp_parts, sizeof(tmp_parts)) != -1)
		mtdparts = tmp_parts;

	if (mtdparts)
		return mtdparts;

#if defined(CONFIG_SYS_MTDPARTS_RUNTIME)
	board_mtdparts_default(&mtdids, &mtdparts);
#elif defined(MTDPARTS_DEFAULT)
	mtdparts = MTDPARTS_DEFAULT;
#elif defined(CONFIG_MTDPARTS_DEFAULT)
	mtdparts = CONFIG_MTDPARTS_DEFAULT;
#endif

	if (mtdparts)
		env_set("mtdparts", mtdparts);

	return mtdparts;
}

static int mtd_del_parts(struct mtd_info *mtd, bool quiet)
{
	int ret;

	if (!mtd_has_partitions(mtd))
		return 0;

	/* do not delete partitions if they are in use. */
	if (mtd_partitions_used(mtd)) {
		if (!quiet)
			printf("\"%s\" partitions still in use, can't delete them\n",
			       mtd->name);
		return -EACCES;
	}

	ret = del_mtd_partitions(mtd);
	if (ret)
		return ret;

	return 1;
}

static bool mtd_del_all_parts_failed;

static void mtd_del_all_parts(void)
{
	struct mtd_info *mtd;
	int ret = 0;

	mtd_del_all_parts_failed = false;

	/*
	 * It is not safe to remove entries from the mtd_for_each_device loop
	 * as it uses idr indexes and the partitions removal is done in bulk
	 * (all partitions of one device at the same time), so break and
	 * iterate from start each time a new partition is found and deleted.
	 */
	do {
		mtd_for_each_device(mtd) {
			ret = mtd_del_parts(mtd, false);
			if (ret > 0)
				break;
			else if (ret < 0)
				mtd_del_all_parts_failed = true;
		}
	} while (ret > 0);
}

int mtd_probe_devices(void)
{
	static char *old_mtdparts;
	static char *old_mtdids;
	const char *mtdparts = get_mtdparts();
	const char *mtdids = get_mtdids();
	const char *mtdparts_next = mtdparts;
	struct mtd_info *mtd;

	mtd_probe_uclass_mtd_devs();

	/*
	 * Check if mtdparts/mtdids changed, if the MTD dev list was updated
	 * or if our previous attempt to delete existing partititions failed.
	 * In any of these cases we want to update the partitions, otherwise,
	 * everything is up-to-date and we can return 0 directly.
	 */
	if ((!mtdparts && !old_mtdparts && !mtdids && !old_mtdids) ||
	    (mtdparts && old_mtdparts && mtdids && old_mtdids &&
	     !mtd_dev_list_updated() && !mtd_del_all_parts_failed &&
	     !strcmp(mtdparts, old_mtdparts) &&
	     !strcmp(mtdids, old_mtdids)))
		return 0;

	/* Update the local copy of mtdparts */
	free(old_mtdparts);
	free(old_mtdids);
	old_mtdparts = strdup(mtdparts);
	old_mtdids = strdup(mtdids);

	/*
	 * Remove all old parts. Note that partition removal can fail in case
	 * one of the partition is still being used by an MTD user, so this
	 * does not guarantee that all old partitions are gone.
	 */
	mtd_del_all_parts();

	/*
	 * Call mtd_dev_list_updated() to clear updates generated by our own
	 * parts removal loop.
	 */
	mtd_dev_list_updated();

	/* If either mtdparts or mtdids is empty, then exit */
	if (!mtdparts || !mtdids)
		return 0;

	/* Start the parsing by ignoring the extra 'mtdparts=' prefix, if any */
	if (!strncmp(mtdparts, "mtdparts=", sizeof("mtdparts=") - 1))
		mtdparts += 9;

	/* For each MTD device in mtdparts */
	for (; mtdparts[0] != '\0'; mtdparts = mtdparts_next) {
		char mtd_name[MTD_NAME_MAX_LEN], *colon;
		struct mtd_partition *parts;
		unsigned int mtd_name_len;
		int nparts, ret;

		mtdparts_next = strchr(mtdparts, ';');
		if (!mtdparts_next)
			mtdparts_next = mtdparts + strlen(mtdparts);
		else
			mtdparts_next++;

		colon = strchr(mtdparts, ':');
		if (colon > mtdparts_next)
			colon = NULL;

		if (!colon) {
			printf("Wrong mtdparts: %s\n", mtdparts);
			return -EINVAL;
		}

		mtd_name_len = (unsigned int)(colon - mtdparts);
		if (mtd_name_len + 1 > sizeof(mtd_name)) {
			printf("MTD name too long: %s\n", mtdparts);
			return -EINVAL;
		}

		strncpy(mtd_name, mtdparts, mtd_name_len);
		mtd_name[mtd_name_len] = '\0';
		/* Move the pointer forward (including the ':') */
		mtdparts += mtd_name_len + 1;
		mtd = get_mtd_device_nm(mtd_name);
		if (IS_ERR_OR_NULL(mtd)) {
			char linux_name[MTD_NAME_MAX_LEN];

			/*
			 * The MTD device named "mtd_name" does not exist. Try
			 * to find a correspondance with an MTD device having
			 * the same type and number as defined in the mtdids.
			 */
			debug("No device named %s\n", mtd_name);
			ret = mtd_search_alternate_name(mtd_name, linux_name,
							MTD_NAME_MAX_LEN);
			if (!ret)
				mtd = get_mtd_device_nm(linux_name);

			/*
			 * If no device could be found, move the mtdparts
			 * pointer forward until the next set of partitions.
			 */
			if (ret || IS_ERR_OR_NULL(mtd)) {
				printf("Could not find a valid device for %s\n",
				       mtd_name);
				mtdparts = mtdparts_next;
				continue;
			}
		}

		/*
		 * Call mtd_del_parts() again, even if it's already been called
		 * in mtd_del_all_parts(). We need to know if old partitions are
		 * still around (because they are still being used by someone),
		 * and if they are, we shouldn't create new partitions, so just
		 * skip this MTD device and try the next one.
		 */
		ret = mtd_del_parts(mtd, true);
		if (ret < 0)
			continue;

		/*
		 * Parse the MTD device partitions. It will update the mtdparts
		 * pointer, create an array of parts (that must be freed), and
		 * return the number of partition structures in the array.
		 */
		ret = mtd_parse_partitions(mtd, &mtdparts, &parts, &nparts);
		if (ret) {
			printf("Could not parse device %s\n", mtd->name);
			put_mtd_device(mtd);
			return -EINVAL;
		}

		if (!nparts)
			continue;

		/* Create the new MTD partitions */
		add_mtd_partitions(mtd, parts, nparts);

		/* Free the structures allocated during the parsing */
		mtd_free_parsed_partitions(parts, nparts);

		put_mtd_device(mtd);
	}

	/*
	 * Call mtd_dev_list_updated() to clear updates generated by our own
	 * parts registration loop.
	 */
	mtd_dev_list_updated();

	return 0;
}
#else
int mtd_probe_devices(void)
{
	mtd_probe_uclass_mtd_devs();

	return 0;
}
#endif /* defined(CONFIG_MTD_PARTITIONS) */

/* Legacy */

static int get_part(const char *partname, int *idx, loff_t *off, loff_t *size,
		loff_t *maxsize, int devtype)
{
#ifdef CONFIG_CMD_MTDPARTS
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;
	int ret;

	ret = mtdparts_init();
	if (ret)
		return ret;

	ret = find_dev_and_part(partname, &dev, &pnum, &part);
	if (ret)
		return ret;

	if (dev->id->type != devtype) {
		printf("not same typ %d != %d\n", dev->id->type, devtype);
		return -1;
	}

	*off = part->offset;
	*size = part->size;
	*maxsize = part->size;
	*idx = dev->id->num;

	return 0;
#else
	puts("mtdparts support missing.\n");
	return -1;
#endif
}

int mtd_arg_off(const char *arg, int *idx, loff_t *off, loff_t *size,
		loff_t *maxsize, int devtype, uint64_t chipsize)
{
	if (!str2off(arg, off))
		return get_part(arg, idx, off, size, maxsize, devtype);

	if (*off >= chipsize) {
		puts("Offset exceeds device limit\n");
		return -1;
	}

	*maxsize = chipsize - *off;
	*size = *maxsize;
	return 0;
}

int mtd_arg_off_size(int argc, char *const argv[], int *idx, loff_t *off,
		     loff_t *size, loff_t *maxsize, int devtype,
		     uint64_t chipsize)
{
	int ret;

	if (argc == 0) {
		*off = 0;
		*size = chipsize;
		*maxsize = *size;
		goto print;
	}

	ret = mtd_arg_off(argv[0], idx, off, size, maxsize, devtype,
			  chipsize);
	if (ret)
		return ret;

	if (argc == 1)
		goto print;

	if (!str2off(argv[1], size)) {
		printf("'%s' is not a number\n", argv[1]);
		return -1;
	}

	if (*size > *maxsize) {
		puts("Size exceeds partition or device limit\n");
		return -1;
	}

print:
	printf("device %d ", *idx);
	if (*size == chipsize)
		puts("whole chip\n");
	else
		printf("offset 0x%llx, size 0x%llx\n",
		       (unsigned long long)*off, (unsigned long long)*size);
	return 0;
}
