// SPDX-License-Identifier: GPL-2.0+
/*
 * cmd_gpt.c -- GPT (GUID Partition Table) handling command
 *
 * Copyright (C) 2015
 * Lukasz Majewski <l.majewski@majess.pl>
 *
 * Copyright (C) 2012 Samsung Electronics
 * author: Lukasz Majewski <l.majewski@samsung.com>
 * author: Piotr Wilczek <p.wilczek@samsung.com>
 */

#include <common.h>
#include <blk.h>
#include <env.h>
#include <log.h>
#include <malloc.h>
#include <command.h>
#include <part.h>
#include <part_efi.h>
#include <part.h>
#include <exports.h>
#include <uuid.h>
#include <linux/ctype.h>
#include <div64.h>
#include <memalign.h>
#include <linux/compat.h>
#include <linux/err.h>
#include <linux/sizes.h>
#include <stdlib.h>

static LIST_HEAD(disk_partitions);

/**
 * extract_env(): Expand env name from string format '&{env_name}'
 *                and return pointer to the env (if the env is set)
 *
 * @param str - pointer to string
 * @param env - pointer to pointer to extracted env
 *
 * Return: - zero on successful expand and env is set
 */
static int extract_env(const char *str, char **env)
{
	int ret = -1;
	char *e, *s;
#ifdef CONFIG_RANDOM_UUID
	char uuid_str[UUID_STR_LEN + 1];
#endif

	if (!str || strlen(str) < 4)
		return -1;

	if (!((strncmp(str, "${", 2) == 0) && (str[strlen(str) - 1] == '}')))
		return -1;

	s = strdup(str);
	if (s == NULL)
		return -1;

	memset(s + strlen(s) - 1, '\0', 1);
	memmove(s, s + 2, strlen(s) - 1);

	e = env_get(s);
	if (e == NULL) {
#ifdef CONFIG_RANDOM_UUID
		debug("%s unset. ", str);
		gen_rand_uuid_str(uuid_str, UUID_STR_FORMAT_GUID);
		env_set(s, uuid_str);

		e = env_get(s);
		if (e) {
			debug("Set to random.\n");
			ret = 0;
		} else {
			debug("Can't get random UUID.\n");
		}
#else
		debug("%s unset.\n", str);
#endif
	} else {
		debug("%s get from environment.\n", str);
		ret = 0;
	}

	*env = e;
	free(s);

	return ret;
}

/**
 * extract_val(): Extract value from a key=value pair list (comma separated).
 *                Only value for the given key is returend.
 *                Function allocates memory for the value, remember to free!
 *
 * @param str - pointer to string with key=values pairs
 * @param key - pointer to the key to search for
 *
 * Return: - pointer to allocated string with the value
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
 * found_key(): Found key without value in parameter list (comma separated).
 *
 * @param str - pointer to string with key
 * @param key - pointer to the key to search for
 *
 * Return: - true on found key
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

/**
 * calc_parts_list_len() - get size of partition table description
 *
 * @numparts:	number of partitions
 * Return:	string size including terminating NUL
 */
static int calc_parts_list_len(int numparts)
{
	/* number of hexadecimal digits of the lbaint_t representation */
	const int lbaint_size = 2 * sizeof(lbaint_t);
	int partlistlen;

	/* media description including terminating NUL */
	partlistlen = strlen("uuid_disk=;") + UUID_STR_LEN + 1;
	/* per-partition descriptions; numparts */
	partlistlen += numparts * (strlen("name=,") + PART_NAME_LEN);
	/* see part.h for definition of struct disk_partition */
	partlistlen += numparts * (strlen("start=0x,") + lbaint_size);
	partlistlen += numparts * (strlen("size=0x,") + lbaint_size);
	if (IS_ENABLED(CONFIG_PARTITION_UUIDS))
		partlistlen += numparts * (strlen("uuid=,") + UUID_STR_LEN);
	if (IS_ENABLED(CONFIG_PARTITION_TYPE_GUID))
		partlistlen += numparts * (strlen("type=;") + UUID_STR_LEN);
	debug("Length of partitions_list is %d for %d partitions\n",
	      partlistlen, numparts);
	return partlistlen;
}

#ifdef CONFIG_CMD_GPT_RENAME
static void del_gpt_info(void)
{
	struct list_head *pos = &disk_partitions;
	struct disk_part *curr;
	while (!list_empty(pos)) {
		curr = list_entry(pos->next, struct disk_part, list);
		list_del(pos->next);
		free(curr);
	}
}

static struct disk_part *allocate_disk_part(struct disk_partition *info,
					    int partnum)
{
	struct disk_part *newpart;
	newpart = calloc(1, sizeof(struct disk_part));
	if (!newpart)
		return ERR_PTR(-ENOMEM);

	newpart->gpt_part_info.start = info->start;
	newpart->gpt_part_info.size = info->size;
	newpart->gpt_part_info.blksz = info->blksz;
	strncpy((char *)newpart->gpt_part_info.name, (const char *)info->name,
		PART_NAME_LEN);
	newpart->gpt_part_info.name[PART_NAME_LEN - 1] = '\0';
	strncpy((char *)newpart->gpt_part_info.type, (const char *)info->type,
		PART_TYPE_LEN);
	newpart->gpt_part_info.type[PART_TYPE_LEN - 1] = '\0';
	newpart->gpt_part_info.bootable = info->bootable;
	if (IS_ENABLED(CONFIG_PARTITION_UUIDS))
		disk_partition_set_uuid(&newpart->gpt_part_info,
					disk_partition_uuid(info));
	if (IS_ENABLED(CONFIG_PARTITION_TYPE_GUID))
		disk_partition_set_type_guid(&newpart->gpt_part_info,
					     disk_partition_type_guid(info));
	newpart->partnum = partnum;

	return newpart;
}

static void prettyprint_part_size(char *sizestr, lbaint_t partsize,
				  lbaint_t blksize)
{
	unsigned long long partbytes, partmegabytes;

	partbytes = partsize * blksize;
	partmegabytes = lldiv(partbytes, SZ_1M);
	snprintf(sizestr, 16, "%lluMiB", partmegabytes);
}

static void print_gpt_info(void)
{
	struct list_head *pos;
	struct disk_part *curr;
	char partstartstr[16];
	char partsizestr[16];

	list_for_each(pos, &disk_partitions) {
		curr = list_entry(pos, struct disk_part, list);
		prettyprint_part_size(partstartstr, curr->gpt_part_info.start,
				      curr->gpt_part_info.blksz);
		prettyprint_part_size(partsizestr, curr->gpt_part_info.size,
				      curr->gpt_part_info.blksz);

		printf("Partition %d:\n", curr->partnum);
		printf("Start %s, size %s\n", partstartstr, partsizestr);
		printf("Block size %lu, name %s\n", curr->gpt_part_info.blksz,
		       curr->gpt_part_info.name);
		printf("Type %s, bootable %d\n", curr->gpt_part_info.type,
		       curr->gpt_part_info.bootable & PART_BOOTABLE);
		if (CONFIG_IS_ENABLED(PARTITION_UUIDS))
			printf("UUID %s\n",
			       disk_partition_uuid(&curr->gpt_part_info));
		if (IS_ENABLED(CONFIG_PARTITION_TYPE_GUID))
			printf("Type GUID %s\n",
			       disk_partition_type_guid(&curr->gpt_part_info));
		printf("\n");
	}
}

/*
 * create the string that upstream 'gpt write' command will accept as an
 * argument
 *
 * From doc/README.gpt, Format of partitions layout:
 *    "uuid_disk=...;name=u-boot,size=60MiB,uuid=...;
 *	name=kernel,size=60MiB,uuid=...;"
 * The fields 'name' and 'size' are mandatory for every partition.
 * The field 'start' is optional. The fields 'uuid' and 'uuid_disk'
 * are optional if CONFIG_RANDOM_UUID is enabled.
 */
static int create_gpt_partitions_list(int numparts, const char *guid,
				      char *partitions_list)
{
	struct list_head *pos;
	struct disk_part *curr;
	char partstr[PART_NAME_LEN + 1];

	if (!partitions_list)
		return -EINVAL;

	strcpy(partitions_list, "uuid_disk=");
	strncat(partitions_list, guid, UUID_STR_LEN + 1);
	strcat(partitions_list, ";");

	list_for_each(pos, &disk_partitions) {
		curr = list_entry(pos, struct disk_part, list);
		strcat(partitions_list, "name=");
		strncat(partitions_list, (const char *)curr->gpt_part_info.name,
			PART_NAME_LEN + 1);
		sprintf(partstr, ",start=0x%llx",
			(unsigned long long)curr->gpt_part_info.start *
					    curr->gpt_part_info.blksz);
		/* one extra byte for NULL */
		strncat(partitions_list, partstr, PART_NAME_LEN + 1);
		sprintf(partstr, ",size=0x%llx",
			(unsigned long long)curr->gpt_part_info.size *
					    curr->gpt_part_info.blksz);
		strncat(partitions_list, partstr, PART_NAME_LEN + 1);

		if (IS_ENABLED(CONFIG_PARTITION_TYPE_GUID)) {
			strcat(partitions_list, ",type=");
			strncat(partitions_list,
				disk_partition_type_guid(&curr->gpt_part_info),
				UUID_STR_LEN + 1);
		}
		if (CONFIG_IS_ENABLED(PARTITION_UUIDS)) {
			strcat(partitions_list, ",uuid=");
			strncat(partitions_list,
				disk_partition_uuid(&curr->gpt_part_info),
				UUID_STR_LEN + 1);
		}
		if (curr->gpt_part_info.bootable & PART_BOOTABLE)
			strcat(partitions_list, ",bootable");
		strcat(partitions_list, ";");
	}
	return 0;
}

/*
 * read partition info into disk_partitions list where
 * it can be printed or modified
 */
static int get_gpt_info(struct blk_desc *dev_desc)
{
	/* start partition numbering at 1, as U-Boot does */
	int valid_parts = 0, p, ret;
	struct disk_partition info;
	struct disk_part *new_disk_part;

	/*
	 * Always re-read partition info from device, in case
	 * it has changed
	 */
	INIT_LIST_HEAD(&disk_partitions);

	for (p = 1; p <= MAX_SEARCH_PARTITIONS; p++) {
		ret = part_get_info(dev_desc, p, &info);
		if (ret)
			continue;

		/* Add 1 here because counter is zero-based but p1 is
		   the first partition */
		new_disk_part = allocate_disk_part(&info, valid_parts+1);
		if (IS_ERR(new_disk_part))
			goto out;

		list_add_tail(&new_disk_part->list, &disk_partitions);
		valid_parts++;
	}
	if (valid_parts == 0) {
		printf("** No valid partitions found **\n");
		goto out;
	}
	return valid_parts;
 out:
	if (valid_parts >= 1)
		del_gpt_info();
	return -ENODEV;
}

/* a wrapper to test get_gpt_info */
static int do_get_gpt_info(struct blk_desc *dev_desc, char * const namestr)
{
	int numparts;
	numparts = get_gpt_info(dev_desc);

	numparts = get_gpt_info(dev_desc);

	if (numparts > 0) {
		if (namestr) {
			char disk_guid[UUID_STR_LEN + 1];
			char *partitions_list;
			int partlistlen;
			int ret = -1;

			ret = get_disk_guid(dev_desc, disk_guid);
			if (ret < 0)
				return ret;

			partlistlen = calc_parts_list_len(numparts);
			partitions_list = malloc(partlistlen);
			if (!partitions_list) {
				del_gpt_info();
				return -ENOMEM;
			}
			memset(partitions_list, '\0', partlistlen);

			ret = create_gpt_partitions_list(numparts, disk_guid,
							 partitions_list);
			if (ret < 0)
				printf("Error: Could not create partition list string!\n");
			else
				env_set(namestr, partitions_list);

			free(partitions_list);
		} else {
			print_gpt_info();
		}
		del_gpt_info();
		return 0;
	}
	return numparts;
}
#endif

/**
 * set_gpt_info(): Fill partition information from string
 *		function allocates memory, remember to free!
 *
 * @param dev_desc - pointer block device descriptor
 * @param str_part - pointer to string with partition information
 * @param str_disk_guid - pointer to pointer to allocated string with disk guid
 * @param partitions - pointer to pointer to allocated partitions array
 * @param parts_count - number of partitions
 *
 * Return: - zero on success, otherwise error
 *
 */
static int set_gpt_info(struct blk_desc *dev_desc,
			const char *str_part,
			char **str_disk_guid,
			struct disk_partition **partitions,
			u8 *parts_count)
{
	char *tok, *str, *s;
	int i;
	char *val, *p;
	int p_count;
	struct disk_partition *parts;
	int errno = 0;
	uint64_t size_ll, start_ll;
	lbaint_t offset = 0;
	int max_str_part = calc_parts_list_len(MAX_SEARCH_PARTITIONS);

	debug("%s:  lba num: 0x%x %d\n", __func__,
	      (unsigned int)dev_desc->lba, (unsigned int)dev_desc->lba);

	if (str_part == NULL)
		return -1;

	str = strdup(str_part);
	if (str == NULL)
		return -ENOMEM;

	/* extract disk guid */
	s = str;
	val = extract_val(str, "uuid_disk");
	if (!val) {
#ifdef CONFIG_RANDOM_UUID
		*str_disk_guid = malloc(UUID_STR_LEN + 1);
		if (*str_disk_guid == NULL)
			return -ENOMEM;
		gen_rand_uuid_str(*str_disk_guid, UUID_STR_FORMAT_STD);
#else
		free(str);
		return -2;
#endif
	} else {
		val = strsep(&val, ";");
		if (extract_env(val, &p))
			p = val;
		*str_disk_guid = strdup(p);
		free(val);
		/* Move s to first partition */
		strsep(&s, ";");
	}
	if (s == NULL) {
		printf("Error: is the partitions string NULL-terminated?\n");
		return -EINVAL;
	}
	if (strnlen(s, max_str_part) == 0)
		return -3;

	i = strnlen(s, max_str_part) - 1;
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

		/* uuid */
		val = extract_val(tok, "uuid");
		if (!val) {
			/* 'uuid' is optional if random uuid's are enabled */
#ifdef CONFIG_RANDOM_UUID
			gen_rand_uuid_str(parts[i].uuid, UUID_STR_FORMAT_STD);
#else
			errno = -4;
			goto err;
#endif
		} else {
			if (extract_env(val, &p))
				p = val;
			if (strnlen(p, max_str_part) >= sizeof(parts[i].uuid)) {
				printf("Wrong uuid format for partition %d\n", i);
				errno = -4;
				goto err;
			}
			strncpy((char *)parts[i].uuid, p, max_str_part);
			free(val);
		}
#ifdef CONFIG_PARTITION_TYPE_GUID
		/* guid */
		val = extract_val(tok, "type");
		if (val) {
			/* 'type' is optional */
			if (extract_env(val, &p))
				p = val;
			if (strnlen(p, max_str_part) >= sizeof(parts[i].type_guid)) {
				printf("Wrong type guid format for partition %d\n",
				       i);
				errno = -4;
				goto err;
			}
			strncpy((char *)parts[i].type_guid, p, max_str_part);
			free(val);
		}
#endif
		/* name */
		val = extract_val(tok, "name");
		if (!val) { /* name is mandatory */
			errno = -4;
			goto err;
		}
		if (extract_env(val, &p))
			p = val;
		if (strnlen(p, max_str_part) >= sizeof(parts[i].name)) {
			errno = -4;
			goto err;
		}
		strncpy((char *)parts[i].name, p, max_str_part);
		free(val);

		/* size */
		val = extract_val(tok, "size");
		if (!val) { /* 'size' is mandatory */
			errno = -4;
			goto err;
		}
		if (extract_env(val, &p))
			p = val;
		if ((strcmp(p, "-") == 0)) {
			/* Let part efi module to auto extend the size */
			parts[i].size = 0;
		} else {
			size_ll = ustrtoull(p, &p, 0);
			parts[i].size = lldiv(size_ll, dev_desc->blksz);
		}

		free(val);

		/* start address */
		val = extract_val(tok, "start");
		if (val) { /* start address is optional */
			if (extract_env(val, &p))
				p = val;
			start_ll = ustrtoull(p, &p, 0);
			parts[i].start = lldiv(start_ll, dev_desc->blksz);
			free(val);
		}

		offset += parts[i].size + parts[i].start;

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
	free(*str_disk_guid);
	free(parts);

	return errno;
}

static int gpt_repair(struct blk_desc *blk_dev_desc)
{
	int ret = 0;

	ret = gpt_repair_headers(blk_dev_desc);

	return ret;
}

static int gpt_default(struct blk_desc *blk_dev_desc, const char *str_part)
{
	int ret;
	char *str_disk_guid;
	u8 part_count = 0;
	struct disk_partition *partitions = NULL;

	/* fill partitions */
	ret = set_gpt_info(blk_dev_desc, str_part,
			&str_disk_guid, &partitions, &part_count);
	if (ret) {
		if (ret == -1)
			printf("No partition list provided\n");
		if (ret == -2)
			printf("Missing disk guid\n");
		if ((ret == -3) || (ret == -4))
			printf("Partition list incomplete\n");
		return -1;
	}

	/* save partitions layout to disk */
	ret = gpt_restore(blk_dev_desc, str_disk_guid, partitions, part_count);
	free(str_disk_guid);
	free(partitions);

	return ret;
}

static int gpt_verify(struct blk_desc *blk_dev_desc, const char *str_part)
{
	ALLOC_CACHE_ALIGN_BUFFER_PAD(gpt_header, gpt_head, 1,
				     blk_dev_desc->blksz);
	struct disk_partition *partitions = NULL;
	gpt_entry *gpt_pte = NULL;
	char *str_disk_guid;
	u8 part_count = 0;
	int ret = 0;

	/* fill partitions */
	ret = set_gpt_info(blk_dev_desc, str_part,
			&str_disk_guid, &partitions, &part_count);
	if (ret) {
		if (ret == -1) {
			printf("No partition list provided - only basic check\n");
			ret = gpt_verify_headers(blk_dev_desc, gpt_head,
						 &gpt_pte);
			goto out;
		}
		if (ret == -2)
			printf("Missing disk guid\n");
		if ((ret == -3) || (ret == -4))
			printf("Partition list incomplete\n");
		return -1;
	}

	/* Check partition layout with provided pattern */
	ret = gpt_verify_partitions(blk_dev_desc, partitions, part_count,
				    gpt_head, &gpt_pte);
	free(str_disk_guid);
	free(partitions);
 out:
	free(gpt_pte);
	return ret;
}

/**
 * gpt_enumerate() - Enumerate partition names into environment variable.
 *
 * Enumerate partition names. Partition names are stored in gpt_partition_list
 * environment variable. Each partition name is delimited by space.
 *
 * @desc: block device descriptor
 *
 * @Return: '0' on success and -ve error on failure
 */
static int gpt_enumerate(struct blk_desc *desc)
{
	struct part_driver *first_drv, *part_drv;
	int str_len = 0, tmp_len;
	char part_list[2048];
	int n_drvs;
	char *ptr;

	part_list[0] = 0;
	n_drvs = part_driver_get_count();
	if (!n_drvs) {
		printf("Failed to get partition driver count\n");
		return -ENOENT;
	}

	first_drv = part_driver_get_first();
	for (part_drv = first_drv; part_drv != first_drv + n_drvs; part_drv++) {
		struct disk_partition pinfo;
		int ret;
		int i;

		if (part_drv->test(desc))
			continue;

		for (i = 1; i < part_drv->max_entries; i++) {
			ret = part_drv->get_info(desc, i, &pinfo);
			if (ret)
				continue;

			ptr = &part_list[str_len];
			tmp_len = strlen((const char *)pinfo.name);
			str_len += tmp_len;
			/* +1 for space */
			str_len++;
			if (str_len > sizeof(part_list)) {
				printf("Error insufficient memory\n");
				return -ENOMEM;
			}
			strcpy(ptr, (const char *)pinfo.name);
			/* One byte for space(" ") delimiter */
			ptr[tmp_len] = ' ';
		}
		if (*part_list)
			part_list[strlen(part_list) - 1] = 0;
		break;
	}
	debug("setenv gpt_partition_list %s\n", part_list);

	return env_set("gpt_partition_list", part_list);
}

/**
 * gpt_setenv_part_variables() - setup partition environmental variables
 *
 * Setup the gpt_partition_name, gpt_partition_entry, gpt_partition_addr
 * and gpt_partition_size, gpt_partition_bootable environment variables.
 *
 * @pinfo: pointer to disk partition
 * @i: partition entry
 *
 * @Return: '0' on success and -ENOENT on failure
 */
static int gpt_setenv_part_variables(struct disk_partition *pinfo, int i)
{
	int ret;

	ret = env_set_hex("gpt_partition_addr", pinfo->start);
	if (ret)
		goto fail;

	ret = env_set_hex("gpt_partition_size", pinfo->size);
	if (ret)
		goto fail;

	ret = env_set_hex("gpt_partition_entry", i);
	if (ret)
		goto fail;

	ret = env_set("gpt_partition_name", (const char *)pinfo->name);
	if (ret)
		goto fail;

	ret = env_set_ulong("gpt_partition_bootable", !!(pinfo->bootable & PART_BOOTABLE));
	if (ret)
		goto fail;

	return 0;

fail:
	return -ENOENT;
}

/**
 * gpt_setenv() - Dynamically setup environment variables.
 *
 * Dynamically setup environment variables for name, index, offset and size
 * for partition in GPT table after running "gpt setenv" for a partition name.
 *
 * @desc: block device descriptor
 * @name: partition name
 *
 * @Return: '0' on success and -ve err on failure
 */
static int gpt_setenv(struct blk_desc *desc, const char *name)
{
	struct part_driver *first_drv, *part_drv;
	int n_drvs;
	int ret = -1;

	n_drvs = part_driver_get_count();
	if (!n_drvs) {
		printf("Failed to get partition driver count\n");
		goto fail;
	}

	first_drv = part_driver_get_first();
	for (part_drv = first_drv; part_drv != first_drv + n_drvs; part_drv++) {
		struct disk_partition pinfo;
		int i;

		for (i = 1; i < part_drv->max_entries; i++) {
			ret = part_drv->get_info(desc, i, &pinfo);
			if (ret)
				continue;

			if (!strcmp(name, (const char *)pinfo.name)) {
				/* match found, setup environment variables */
				ret = gpt_setenv_part_variables(&pinfo, i);
				if (ret)
					goto fail;

				return 0;
			}
		}
	}

fail:
	return ret;
}

static int do_disk_guid(struct blk_desc *dev_desc, char * const namestr)
{
	int ret;
	char disk_guid[UUID_STR_LEN + 1];

	ret = get_disk_guid(dev_desc, disk_guid);
	if (ret < 0)
		return CMD_RET_FAILURE;

	if (namestr)
		env_set(namestr, disk_guid);
	else
		printf("%s\n", disk_guid);

	return ret;
}

#ifdef CONFIG_CMD_GPT_RENAME
static int do_rename_gpt_parts(struct blk_desc *dev_desc, char *subcomm,
			       char *name1, char *name2)
{
	struct list_head *pos;
	struct disk_part *curr;
	struct disk_partition *new_partitions = NULL;
	char disk_guid[UUID_STR_LEN + 1];
	char *partitions_list, *str_disk_guid = NULL;
	u8 part_count = 0;
	int partlistlen, ret, numparts = 0, partnum, i = 1, ctr1 = 0, ctr2 = 0;

	if (!subcomm || !name1 || !name2 ||
	    (strcmp(subcomm, "swap") && strcmp(subcomm, "rename") &&
	     strcmp(subcomm, "transpose")))
		return -EINVAL;

	ret = get_disk_guid(dev_desc, disk_guid);
	if (ret < 0)
		return ret;
	/*
	 * Allocates disk_partitions, requiring matching call to del_gpt_info()
	 * if successful.
	 */
	numparts = get_gpt_info(dev_desc);
	if (numparts <=  0)
		return numparts ? numparts : -ENODEV;

	partlistlen = calc_parts_list_len(numparts);
	partitions_list = malloc(partlistlen);
	if (!partitions_list) {
		del_gpt_info();
		return -ENOMEM;
	}
	memset(partitions_list, '\0', partlistlen);

	ret = create_gpt_partitions_list(numparts, disk_guid, partitions_list);
	if (ret < 0) {
		free(partitions_list);
		return ret;
	}
	/*
	 * Uncomment the following line to print a string that 'gpt write'
	 * or 'gpt verify' will accept as input.
	 */
	debug("OLD partitions_list is %s with %u chars\n", partitions_list,
	      (unsigned)strlen(partitions_list));

	/* set_gpt_info allocates new_partitions and str_disk_guid */
	ret = set_gpt_info(dev_desc, partitions_list, &str_disk_guid,
			   &new_partitions, &part_count);
	if (ret < 0)
		goto out;

	if (!strcmp(subcomm, "swap")) {
		if ((strlen(name1) > PART_NAME_LEN) || (strlen(name2) > PART_NAME_LEN)) {
			printf("Names longer than %d characters are truncated.\n", PART_NAME_LEN);
			ret = -EINVAL;
			goto out;
		}
		list_for_each(pos, &disk_partitions) {
			curr = list_entry(pos, struct disk_part, list);
			if (!strcmp((char *)curr->gpt_part_info.name, name1)) {
				strcpy((char *)curr->gpt_part_info.name, name2);
				ctr1++;
			} else if (!strcmp((char *)curr->gpt_part_info.name, name2)) {
				strcpy((char *)curr->gpt_part_info.name, name1);
				ctr2++;
			}
		}
		if ((ctr1 + ctr2 < 2) || (ctr1 != ctr2)) {
			printf("Cannot swap partition names except in pairs.\n");
			ret = -EINVAL;
			goto out;
		}
	} else if (!strcmp(subcomm, "transpose")) {
		int idx1, idx2;
		struct disk_partition* first = NULL;
		struct disk_partition* second= NULL;
		struct disk_partition tmp_part;

		idx1 = simple_strtoul(name1, NULL, 10);
		idx2 = simple_strtoul(name2, NULL, 10);
		if (idx1 == idx2) {
			printf("Cannot swap partition with itself\n");
			ret = -EINVAL;
			goto out;
		}

		list_for_each(pos, &disk_partitions) {
			curr = list_entry(pos, struct disk_part, list);
			if (curr->partnum == idx1)
				first = &curr->gpt_part_info;
			else if (curr->partnum == idx2)
				second = &curr->gpt_part_info;
		}
		if (!first) {
			printf("Illegal partition number %s\n", name1);
			ret = -EINVAL;
			goto out;
		}
		if (!second) {
			printf("Illegal partition number %s\n", name2);
			ret = -EINVAL;
			goto out;
		}

		tmp_part = *first;
		*first = *second;
		*second = tmp_part;
	} else { /* rename */
		if (strlen(name2) > PART_NAME_LEN) {
			printf("Names longer than %d characters are truncated.\n", PART_NAME_LEN);
			ret = -EINVAL;
			goto out;
		}
		partnum = (int)simple_strtol(name1, NULL, 10);
		if ((partnum < 0) || (partnum > numparts)) {
			printf("Illegal partition number %s\n", name1);
			ret = -EINVAL;
			goto out;
		}
		ret = part_get_info(dev_desc, partnum, new_partitions);
		if (ret < 0)
			goto out;

		/* U-Boot partition numbering starts at 1 */
		list_for_each(pos, &disk_partitions) {
			curr = list_entry(pos, struct disk_part, list);
			if (i == partnum) {
				strcpy((char *)curr->gpt_part_info.name, name2);
				break;
			}
			i++;
		}
	}

	ret = create_gpt_partitions_list(numparts, disk_guid, partitions_list);
	if (ret < 0)
		goto out;
	debug("NEW partitions_list is %s with %u chars\n", partitions_list,
	      (unsigned)strlen(partitions_list));

	ret = set_gpt_info(dev_desc, partitions_list, &str_disk_guid,
			   &new_partitions, &part_count);
	/*
	 * Even though valid pointers are here passed into set_gpt_info(),
	 * it mallocs again, and there's no way to tell which failed.
	 */
	if (ret < 0)
		goto out;

	debug("Writing new partition table\n");
	ret = gpt_restore(dev_desc, disk_guid, new_partitions, numparts);
	if (ret < 0) {
		printf("Writing new partition table failed\n");
		goto out;
	}

	debug("Reading back new partition table\n");
	/*
	 * Empty the existing disk_partitions list, as otherwise the memory in
	 * the original list is unreachable.
	 */
	del_gpt_info();
	numparts = get_gpt_info(dev_desc);
	if (numparts <=  0) {
		ret = numparts ? numparts : -ENODEV;
		goto out;
	}
	printf("new partition table with %d partitions is:\n", numparts);
	print_gpt_info();
 out:
	del_gpt_info();
#ifdef CONFIG_RANDOM_UUID
	free(str_disk_guid);
#endif
	free(new_partitions);
	free(partitions_list);
	return ret;
}

/**
 * gpt_set_bootable() - Set bootable flags for partitions
 *
 * Sets the bootable flag for any partition names in the comma separated list of
 * partition names. Any partitions not in the list have their bootable flag
 * cleared
 *
 * @desc: block device descriptor
 * @name: Comma separated list of partition names
 *
 * @Return: '0' on success and -ve error on failure
 */
static int gpt_set_bootable(struct blk_desc *blk_dev_desc, char *const part_list)
{
	char *name;
	char disk_guid[UUID_STR_LEN + 1];
	struct list_head *pos;
	struct disk_part *curr;
	struct disk_partition *partitions = NULL;
	int part_count = 0;
	int ret = get_disk_guid(blk_dev_desc, disk_guid);

	if (ret < 0)
		return ret;

	ret = get_gpt_info(blk_dev_desc);
	if (ret <= 0)
		goto out;

	part_count = ret;
	partitions = malloc(sizeof(*partitions) * part_count);
	if (!partitions) {
		ret = -ENOMEM;
		goto out;
	}

	/* Copy partitions and clear bootable flag */
	part_count = 0;
	list_for_each(pos, &disk_partitions) {
		curr = list_entry(pos, struct disk_part, list);
		partitions[part_count] = curr->gpt_part_info;
		partitions[part_count].bootable &= ~PART_BOOTABLE;
		part_count++;
	}

	name = strtok(part_list, ",");
	while (name) {
		bool found = false;

		for (int i = 0; i < part_count; i++) {
			if (strcmp((char *)partitions[i].name, name) == 0) {
				partitions[i].bootable |= PART_BOOTABLE;
				found = true;
			}
		}

		if (!found) {
			printf("Warning: No partition matching '%s' found\n",
			       name);
		}

		name = strtok(NULL, ",");
	}

	ret = gpt_restore(blk_dev_desc, disk_guid, partitions, part_count);

out:
	del_gpt_info();

	if (partitions)
		free(partitions);

	return ret;
}
#endif

/**
 * do_gpt(): Perform GPT operations
 *
 * @param cmdtp - command name
 * @param flag
 * @param argc
 * @param argv
 *
 * Return: zero on success; otherwise error
 */
static int do_gpt(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = CMD_RET_SUCCESS;
	int dev = 0;
	char *ep;
	struct blk_desc *blk_dev_desc = NULL;

#ifndef CONFIG_CMD_GPT_RENAME
	if (argc < 4 || argc > 5)
#else
	if (argc < 4 || argc > 6)
#endif
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

	if (strcmp(argv[1], "repair") == 0) {
		printf("Repairing GPT: ");
		ret = gpt_repair(blk_dev_desc);
	} else if ((strcmp(argv[1], "write") == 0) && (argc == 5)) {
		printf("Writing GPT: ");
		ret = gpt_default(blk_dev_desc, argv[4]);
	} else if ((strcmp(argv[1], "verify") == 0)) {
		ret = gpt_verify(blk_dev_desc, argv[4]);
		printf("Verify GPT: ");
	} else if ((strcmp(argv[1], "setenv") == 0)) {
		ret = gpt_setenv(blk_dev_desc, argv[4]);
	} else if ((strcmp(argv[1], "enumerate") == 0)) {
		ret = gpt_enumerate(blk_dev_desc);
	} else if (strcmp(argv[1], "guid") == 0) {
		ret = do_disk_guid(blk_dev_desc, argv[4]);
#ifdef CONFIG_CMD_GPT_RENAME
	} else if (strcmp(argv[1], "read") == 0) {
		ret = do_get_gpt_info(blk_dev_desc, (argc == 5) ? argv[4] : NULL);
	} else if ((strcmp(argv[1], "swap") == 0) ||
		   (strcmp(argv[1], "rename") == 0) ||
		   (strcmp(argv[1], "transpose") == 0)) {
		ret = do_rename_gpt_parts(blk_dev_desc, argv[1], argv[4], argv[5]);
	} else if ((strcmp(argv[1], "set-bootable") == 0)) {
		ret = gpt_set_bootable(blk_dev_desc, argv[4]);
#endif
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

U_BOOT_CMD(gpt, CONFIG_SYS_MAXARGS, 1, do_gpt,
	"GUID Partition Table",
	"<command> <interface> <dev> <partitions_list>\n"
	" - GUID partition table restoration and validity check\n"
	" Restore or verify GPT information on a device connected\n"
	" to interface\n"
	" Example usage:\n"
	" gpt repair mmc 0\n"
	"    - repair the GPT on the device\n"
	" gpt write mmc 0 $partitions\n"
	"    - write the GPT to device\n"
	" gpt verify mmc 0 $partitions\n"
	"    - verify the GPT on device against $partitions\n"
	" gpt setenv mmc 0 $name\n"
	"    - setup environment variables for partition $name:\n"
	"      gpt_partition_addr, gpt_partition_size,\n"
	"      gpt_partition_name, gpt_partition_entry,\n"
	"      gpt_partition_bootable\n"
	" gpt enumerate mmc 0\n"
	"    - store list of partitions to gpt_partition_list environment variable\n"
	" gpt guid <interface> <dev>\n"
	"    - print disk GUID\n"
	" gpt guid <interface> <dev> <varname>\n"
	"    - set environment variable to disk GUID\n"
	" Example usage:\n"
	" gpt guid mmc 0\n"
	" gpt guid mmc 0 varname\n"
#ifdef CONFIG_CMD_GPT_RENAME
	"gpt partition renaming commands:\n"
	" gpt read <interface> <dev> [<varname>]\n"
	"    - read GPT into a data structure for manipulation\n"
	"    - read GPT partitions into environment variable\n"
	" gpt swap <interface> <dev> <name1> <name2>\n"
	"    - change all partitions named name1 to name2\n"
	"      and vice-versa\n"
	" gpt transpose <interface> <dev> <part1> <part2>\n"
	"    - Swap the order of the entries for part1 and part2 in the partition table\n"
	" gpt rename <interface> <dev> <part> <name>\n"
	"    - rename the specified partition\n"
	" gpt set-bootable <interface> <dev> <list>\n"
	"    - make partition names in list bootable\n"
	" Example usage:\n"
	" gpt swap mmc 0 foo bar\n"
	" gpt rename mmc 0 3 foo\n"
	" gpt set-bootable mmc 0 boot_a,boot_b\n"
	" gpt transpose mmc 0 1 2\n"
#endif
);
