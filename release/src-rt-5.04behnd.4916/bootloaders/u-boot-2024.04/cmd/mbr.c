// SPDX-License-Identifier: GPL-2.0+
/*
 * cmd_mbr.c -- MBR (Master Boot Record) handling command
 *
 * Copyright (C) 2020 Samsung Electronics
 * author: Marek Szyprowski <m.szyprowski@samsung.com>
 *
 * based on the gpt command.
 */

#include <common.h>
#include <blk.h>
#include <command.h>
#include <malloc.h>
#include <part.h>

/**
 * extract_val() - Extract a value from the key=value pair list
 * @str: pointer to string with key=values pairs
 * @key: pointer to the key to search for
 *
 * The list of parameters is come separated, only a value for
 * the given key is returend.
 *
 * Function allocates memory for the value, remember to free!
 *
 * Return: Pointer to allocated string with the value.
 */
static char *extract_val(const char *str, const char *key)
{
	char *v, *k;
	char *s, *strcopy;
	char *new = NULL;

	strcopy = strdup(str);
	if (strcopy == NULL)
		return NULL;

	s = strcopy;
	while (s) {
		v = strsep(&s, ",");
		if (!v)
			break;
		k = strsep(&v, "=");
		if (!k)
			break;
		if  (strcmp(k, key) == 0) {
			new = strdup(v);
			break;
		}
	}

	free(strcopy);

	return new;
}

/**
 * found_key() - Search for a key without a value in the parameter list
 * @str: pointer to string with key
 * @key: pointer to the key to search for
 *
 * The list of parameters is come separated.
 *
 * Return: True if key has been found.
 */
static bool found_key(const char *str, const char *key)
{
	char *k;
	char *s, *strcopy;
	bool result = false;

	strcopy = strdup(str);
	if (!strcopy)
		return NULL;

	s = strcopy;
	while (s) {
		k = strsep(&s, ",");
		if (!k)
			break;
		if  (strcmp(k, key) == 0) {
			result = true;
			break;
		}
	}

	free(strcopy);

	return result;
}

static int str_to_partitions(const char *str_part, int blksz,
	unsigned long *disk_uuid, struct disk_partition **partitions,
	int *parts_count)
{
	char *tok, *str, *s;
	int i;
	char *val, *p;
	int p_count;
	struct disk_partition *parts;
	int errno = 0;
	uint64_t size_ll, start_ll;

	if (str_part == NULL)
		return -1;

	str = strdup(str_part);
	if (str == NULL)
		return -ENOMEM;

	/* extract disk guid */
	s = str;
	val = extract_val(str, "uuid_disk");
	if (val) {
		val = strsep(&val, ";");
		p = val;
		*disk_uuid = ustrtoull(p, &p, 0);
		free(val);
		/* Move s to first partition */
		strsep(&s, ";");
	}
	if (s == NULL) {
		printf("Error: is the partitions string NULL-terminated?\n");
		return -EINVAL;
	}

	/* remove the optional semicolon at the end of the string */
	i = strlen(s) - 1;
	if (s[i] == ';')
		s[i] = '\0';

	/* calculate expected number of partitions */
	p_count = 1;
	p = s;
	while (*p) {
		if (*p++ == ';')
			p_count++;
	}

	/* allocate memory for partitions */
	parts = calloc(sizeof(struct disk_partition), p_count);
	if (parts == NULL)
		return -ENOMEM;

	/* retrieve partitions data from string */
	for (i = 0; i < p_count; i++) {
		tok = strsep(&s, ";");

		if (tok == NULL)
			break;

		/* size */
		val = extract_val(tok, "size");
		if (!val) { /* 'size' is mandatory */
			errno = -4;
			goto err;
		}
		p = val;
		if ((strcmp(p, "-") == 0)) {
			/* auto extend the size */
			parts[i].size = 0;
		} else {
			size_ll = ustrtoull(p, &p, 0);
			parts[i].size = size_ll / blksz;
		}
		free(val);

		/* start address */
		val = extract_val(tok, "start");
		if (val) { /* start address is optional */
			p = val;
			start_ll = ustrtoull(p, &p, 0);
			parts[i].start = start_ll / blksz;
			free(val);
		}

		/* system id */
		val = extract_val(tok, "id");
		if (!val) { /* '' is mandatory */
			errno = -4;
			goto err;
		}
		p = val;
		parts[i].sys_ind = ustrtoul(p, &p, 0);
		free(val);

		/* bootable */
		if (found_key(tok, "bootable"))
			parts[i].bootable = PART_BOOTABLE;
	}

	*parts_count = p_count;
	*partitions = parts;
	free(str);

	return 0;
err:
	free(str);
	free(parts);

	return errno;
}

static int do_write_mbr(struct blk_desc *dev, const char *str)
{
	unsigned long disk_uuid = 0;
	struct disk_partition *partitions;
	int blksz = dev->blksz;
	int count;

	if (str_to_partitions(str, blksz, &disk_uuid, &partitions, &count)) {
		printf("MBR: failed to setup partitions from \"%s\"\n", str);
		return -1;
	}

	if (layout_mbr_partitions(partitions, count, dev->lba)) {
		printf("MBR: failed to layout partitions on the device\n");
		free(partitions);
		return -1;
	}

	if (write_mbr_partitions(dev, partitions, count, disk_uuid)) {
		printf("MBR: failed to write partitions to the device\n");
		free(partitions);
		return -1;
	}

	return 0;
}

static int do_verify_mbr(struct blk_desc *dev, const char *str)
{
	unsigned long disk_uuid = 0;
	struct disk_partition *partitions;
	int blksz = dev->blksz;
	int count, i, ret = 1;

	if (str_to_partitions(str, blksz, &disk_uuid, &partitions, &count)) {
		printf("MBR: failed to setup partitions from \"%s\"\n", str);
		return -1;
	}

	for (i = 0; i < count; i++) {
		struct disk_partition p;

		if (part_get_info_by_type(dev, i + 1, PART_TYPE_DOS, &p))
			goto fail;

		if ((partitions[i].size && p.size != partitions[i].size) ||
		    (partitions[i].start && p.start != partitions[i].start) ||
		    p.sys_ind != partitions[i].sys_ind)
			goto fail;
	}
	ret = 0;
fail:
	free(partitions);
	return ret;
}

static int do_mbr(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	const char *parts = NULL;
	int ret = CMD_RET_SUCCESS;
	int dev = 0;
	char *ep;
	struct blk_desc *blk_dev_desc = NULL;

	if (argc != 4 && argc != 5)
		return CMD_RET_USAGE;

	dev = (int)dectoul(argv[3], &ep);
	if (!ep || ep[0] != '\0') {
		printf("'%s' is not a number\n", argv[3]);
		return CMD_RET_USAGE;
	}
	blk_dev_desc = blk_get_dev(argv[2], dev);
	if (!blk_dev_desc) {
		printf("%s: %s dev %d NOT available\n",
		       __func__, argv[2], dev);
		return CMD_RET_FAILURE;
	}

	if ((strcmp(argv[1], "write") == 0)) {
		parts = (argc == 5) ? argv[4] : env_get("mbr_parts");
		printf("MBR: write ");
		ret = do_write_mbr(blk_dev_desc, parts);
	} else if ((strcmp(argv[1], "verify") == 0)) {
		printf("MBR: verify ");
		parts = (argc == 5) ? argv[4] : env_get("mbr_parts");
		ret = do_verify_mbr(blk_dev_desc, parts);
	} else {
		return CMD_RET_USAGE;
	}

	if (ret) {
		printf("error!\n");
		return CMD_RET_FAILURE;
	}

	printf("success!\n");
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(mbr, CONFIG_SYS_MAXARGS, 1, do_mbr,
	"MBR (Master Boot Record)",
	"<command> <interface> <dev> <partitions_list>\n"
	" - MBR partition table restoration utility\n"
	" Restore or check partition information on a device connected\n"
	" to the given block interface\n"
	" Example usage:\n"
	" mbr write mmc 0 [\"${mbr_parts}\"]\n"
	" mbr verify mmc 0 [\"${partitions}\"]\n"
);
