/*
 * Unsorted Block Image commands
 *
 *  Copyright (C) 2008 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * Copyright 2008-2009 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <exports.h>
#include <malloc.h>
#include <memalign.h>
#include <mtd.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <dm/devres.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/err.h>
#include <ubi_uboot.h>
#include <linux/errno.h>
#include <jffs2/load_kernel.h>
#include <linux/log2.h>

#undef ubi_msg
#define ubi_msg(fmt, ...) printf("UBI: " fmt "\n", ##__VA_ARGS__)

/* Private own data */
static struct ubi_device *ubi;

#ifdef CONFIG_CMD_UBIFS
#include <ubifs_uboot.h>
#endif

static void display_volume_info(struct ubi_device *ubi)
{
	int i;

	for (i = 0; i < (ubi->vtbl_slots + 1); i++) {
		if (!ubi->volumes[i])
			continue;	/* Empty record */
		ubi_dump_vol_info(ubi->volumes[i]);
	}
}

static void display_ubi_info(struct ubi_device *ubi)
{
	ubi_msg("MTD device name:            \"%s\"", ubi->mtd->name);
	ubi_msg("MTD device size:            %llu MiB", ubi->flash_size >> 20);
	ubi_msg("physical eraseblock size:   %d bytes (%d KiB)",
			ubi->peb_size, ubi->peb_size >> 10);
	ubi_msg("logical eraseblock size:    %d bytes", ubi->leb_size);
	ubi_msg("number of good PEBs:        %d", ubi->good_peb_count);
	ubi_msg("number of bad PEBs:         %d", ubi->bad_peb_count);
	ubi_msg("smallest flash I/O unit:    %d", ubi->min_io_size);
	ubi_msg("VID header offset:          %d (aligned %d)",
			ubi->vid_hdr_offset, ubi->vid_hdr_aloffset);
	ubi_msg("data offset:                %d", ubi->leb_start);
	ubi_msg("max. allowed volumes:       %d", ubi->vtbl_slots);
	ubi_msg("wear-leveling threshold:    %d", CONFIG_MTD_UBI_WL_THRESHOLD);
	ubi_msg("number of internal volumes: %d", UBI_INT_VOL_COUNT);
	ubi_msg("number of user volumes:     %d",
			ubi->vol_count - UBI_INT_VOL_COUNT);
	ubi_msg("available PEBs:             %d", ubi->avail_pebs);
	ubi_msg("total number of reserved PEBs: %d", ubi->rsvd_pebs);
	ubi_msg("number of PEBs reserved for bad PEB handling: %d",
			ubi->beb_rsvd_pebs);
	ubi_msg("max/mean erase counter: %d/%d", ubi->max_ec, ubi->mean_ec);
}

static int ubi_info(int layout)
{
	if (layout)
		display_volume_info(ubi);
	else
		display_ubi_info(ubi);

	return 0;
}

static int ubi_list(const char *var, int numeric)
{
	size_t namelen, len, size;
	char *str, *str2;
	int i;

	if (!var) {
		for (i = 0; i < (ubi->vtbl_slots + 1); i++) {
			if (!ubi->volumes[i])
				continue;
			if (ubi->volumes[i]->vol_id >= UBI_INTERNAL_VOL_START)
				continue;
			printf("%d: %s\n",
			       ubi->volumes[i]->vol_id,
			       ubi->volumes[i]->name);
		}
		return 0;
	}

	len = 0;
	size = 16;
	str = malloc(size);
	if (!str)
		return 1;

	for (i = 0; i < (ubi->vtbl_slots + 1); i++) {
		if (!ubi->volumes[i])
			continue;
		if (ubi->volumes[i]->vol_id >= UBI_INTERNAL_VOL_START)
			continue;

		if (numeric)
			namelen = 10; /* strlen(stringify(INT_MAX)) */
		else
			namelen = strlen(ubi->volumes[i]->name);

		if (len + namelen + 1 > size) {
			size = roundup_pow_of_two(len + namelen + 1) * 2;
			str2 = realloc(str, size);
			if (!str2) {
				free(str);
				return 1;
			}
			str = str2;
		}

		if (len)
			str[len++] = ' ';

		if (numeric) {
			len += sprintf(str + len, "%d", ubi->volumes[i]->vol_id) + 1;
		} else {
			memcpy(str + len, ubi->volumes[i]->name, namelen);
			len += namelen;
			str[len] = 0;
		}
	}

	env_set(var, str);
	free(str);

	return 0;
}

static int ubi_check_volumename(const struct ubi_volume *vol, char *name)
{
	return strcmp(vol->name, name);
}

static int ubi_check(char *name)
{
	int i;

	for (i = 0; i < (ubi->vtbl_slots + 1); i++) {
		if (!ubi->volumes[i])
			continue;	/* Empty record */

		if (!ubi_check_volumename(ubi->volumes[i], name))
			return 0;
	}

	return 1;
}

static int verify_mkvol_req(const struct ubi_device *ubi,
			    const struct ubi_mkvol_req *req)
{
	int n, err = EINVAL;

	if (req->bytes < 0 || req->alignment < 0 || req->vol_type < 0 ||
	    req->name_len < 0)
		goto bad;

	if ((req->vol_id < 0 || req->vol_id >= ubi->vtbl_slots) &&
	    req->vol_id != UBI_VOL_NUM_AUTO)
		goto bad;

	if (req->alignment == 0)
		goto bad;

	if (req->bytes == 0) {
		printf("No space left in UBI device!\n");
		err = ENOMEM;
		goto bad;
	}

	if (req->vol_type != UBI_DYNAMIC_VOLUME &&
	    req->vol_type != UBI_STATIC_VOLUME)
		goto bad;

	if (req->alignment > ubi->leb_size)
		goto bad;

	n = req->alignment % ubi->min_io_size;
	if (req->alignment != 1 && n)
		goto bad;

	if (req->name_len > UBI_VOL_NAME_MAX) {
		printf("Name too long!\n");
		err = ENAMETOOLONG;
		goto bad;
	}

	return 0;
bad:
	return err;
}

static int ubi_create_vol(char *volume, int64_t size, int dynamic, int vol_id,
			  bool skipcheck)
{
	struct ubi_mkvol_req req;
	int err;

	if (dynamic)
		req.vol_type = UBI_DYNAMIC_VOLUME;
	else
		req.vol_type = UBI_STATIC_VOLUME;

	req.vol_id = vol_id;
	req.alignment = 1;
	req.bytes = size;

	strcpy(req.name, volume);
	req.name_len = strlen(volume);
	req.name[req.name_len] = '\0';
	req.flags = 0;
	if (skipcheck)
		req.flags |= UBI_VOL_SKIP_CRC_CHECK_FLG;

	/* It's duplicated at drivers/mtd/ubi/cdev.c */
	err = verify_mkvol_req(ubi, &req);
	if (err) {
		printf("verify_mkvol_req failed %d\n", err);
		return err;
	}
	printf("Creating %s volume %s of size %lld\n",
		dynamic ? "dynamic" : "static", volume, size);
	/* Call real ubi create volume */
	return ubi_create_volume(ubi, &req);
}

static struct ubi_volume *ubi_find_volume(char *volume)
{
	struct ubi_volume *vol = NULL;
	int i;

	for (i = 0; i < ubi->vtbl_slots; i++) {
		vol = ubi->volumes[i];
		if (vol && !strcmp(vol->name, volume))
			return vol;
	}

	printf("Volume %s not found!\n", volume);
	return NULL;
}

static int ubi_remove_vol(char *volume)
{
	int err, reserved_pebs, i;
	struct ubi_volume *vol;

	vol = ubi_find_volume(volume);
	if (vol == NULL)
		return ENODEV;

	printf("Remove UBI volume %s (id %d)\n", vol->name, vol->vol_id);

	if (ubi->ro_mode) {
		printf("It's read-only mode\n");
		err = EROFS;
		goto out_err;
	}

	err = ubi_change_vtbl_record(ubi, vol->vol_id, NULL);
	if (err) {
		printf("Error changing Vol tabel record err=%x\n", err);
		goto out_err;
	}
	reserved_pebs = vol->reserved_pebs;
	for (i = 0; i < vol->reserved_pebs; i++) {
		err = ubi_eba_unmap_leb(ubi, vol, i);
		if (err)
			goto out_err;
	}

	kfree(vol->eba_tbl);
	ubi->volumes[vol->vol_id]->eba_tbl = NULL;
	ubi->volumes[vol->vol_id] = NULL;

	ubi->rsvd_pebs -= reserved_pebs;
	ubi->avail_pebs += reserved_pebs;
	i = ubi->beb_rsvd_level - ubi->beb_rsvd_pebs;
	if (i > 0) {
		i = ubi->avail_pebs >= i ? i : ubi->avail_pebs;
		ubi->avail_pebs -= i;
		ubi->rsvd_pebs += i;
		ubi->beb_rsvd_pebs += i;
		if (i > 0)
			ubi_msg("reserve more %d PEBs", i);
	}
	ubi->vol_count -= 1;

	return 0;
out_err:
	ubi_err(ubi, "cannot remove volume %s, error %d", volume, err);
	if (err < 0)
		err = -err;
	return err;
}

static int ubi_rename_vol(char *oldname, char *newname)
{
	struct ubi_volume *vol;
	struct ubi_rename_entry rename;
	struct ubi_volume_desc desc;
	struct list_head list;

	vol = ubi_find_volume(oldname);
	if (!vol) {
		printf("%s: volume %s doesn't exist\n", __func__, oldname);
		return ENODEV;
	}

	if (!ubi_check(newname)) {
		printf("%s: volume %s already exist\n", __func__, newname);
		return EINVAL;
	}

	printf("Rename UBI volume %s to %s\n", oldname, newname);

	if (ubi->ro_mode) {
		printf("%s: ubi device is in read-only mode\n", __func__);
		return EROFS;
	}

	rename.new_name_len = strlen(newname);
	strcpy(rename.new_name, newname);
	rename.remove = 0;
	desc.vol = vol;
	desc.mode = 0;
	rename.desc = &desc;
	INIT_LIST_HEAD(&rename.list);
	INIT_LIST_HEAD(&list);
	list_add(&rename.list, &list);

	return ubi_rename_volumes(ubi, &list);
}

static int ubi_volume_continue_write(char *volume, void *buf, size_t size)
{
	int err = 1;
	struct ubi_volume *vol;

	vol = ubi_find_volume(volume);
	if (vol == NULL)
		return ENODEV;

	err = ubi_more_update_data(ubi, vol, buf, size);
	if (err < 0) {
		printf("Couldnt or partially wrote data\n");
		return -err;
	}

	if (err) {
		size = err;

		err = ubi_check_volume(ubi, vol->vol_id);
		if (err < 0)
			return -err;

		if (err) {
			ubi_warn(ubi, "volume %d on UBI device %d is corrupt",
				 vol->vol_id, ubi->ubi_num);
			vol->corrupted = 1;
		}

		vol->checked = 1;
		ubi_gluebi_updated(vol);
	}

	return 0;
}

int ubi_volume_begin_write(char *volume, void *buf, size_t size,
	size_t full_size)
{
	int err = 1;
	int rsvd_bytes = 0;
	struct ubi_volume *vol;

	vol = ubi_find_volume(volume);
	if (vol == NULL)
		return ENODEV;

	rsvd_bytes = vol->reserved_pebs * (ubi->leb_size - vol->data_pad);
	if (size > rsvd_bytes) {
		printf("size > volume size! Aborting!\n");
		return EINVAL;
	}

	err = ubi_start_update(ubi, vol, full_size);
	if (err < 0) {
		printf("Cannot start volume update\n");
		return -err;
	}

	return ubi_volume_continue_write(volume, buf, size);
}

int ubi_volume_write(char *volume, void *buf, size_t size)
{
	return ubi_volume_begin_write(volume, buf, size, size);
}

int ubi_volume_read(char *volume, char *buf, size_t size)
{
	int err, lnum, off, len, tbuf_size;
	void *tbuf;
	unsigned long long tmp;
	struct ubi_volume *vol;
	loff_t offp = 0;
	size_t len_read;

	vol = ubi_find_volume(volume);
	if (vol == NULL)
		return ENODEV;

	if (vol->updating) {
		printf("updating");
		return EBUSY;
	}
	if (vol->upd_marker) {
		printf("damaged volume, update marker is set");
		return EBADF;
	}
	if (offp == vol->used_bytes)
		return 0;

	if (size == 0) {
		printf("No size specified -> Using max size (%lld)\n", vol->used_bytes);
		size = vol->used_bytes;
	}

	printf("Read %zu bytes from volume %s to %p\n", size, volume, buf);

	if (vol->corrupted)
		printf("read from corrupted volume %d", vol->vol_id);
	if (offp + size > vol->used_bytes)
		size = vol->used_bytes - offp;

	tbuf_size = vol->usable_leb_size;
	if (size < tbuf_size)
		tbuf_size = ALIGN(size, ubi->min_io_size);
	tbuf = malloc_cache_aligned(tbuf_size);
	if (!tbuf) {
		printf("NO MEM\n");
		return ENOMEM;
	}
	len = size > tbuf_size ? tbuf_size : size;

	tmp = offp;
	off = do_div(tmp, vol->usable_leb_size);
	lnum = tmp;
	len_read = size;
	do {
		if (off + len >= vol->usable_leb_size)
			len = vol->usable_leb_size - off;

		err = ubi_eba_read_leb(ubi, vol, lnum, tbuf, off, len, 0);
		if (err) {
			printf("read err %x\n", err);
			err = -err;
			break;
		}
		off += len;
		if (off == vol->usable_leb_size) {
			lnum += 1;
			off -= vol->usable_leb_size;
		}

		size -= len;
		offp += len;

		memcpy(buf, tbuf, len);

		buf += len;
		len = size > tbuf_size ? tbuf_size : size;
	} while (size);

	if (!size)
		env_set_hex("filesize", len_read);

	free(tbuf);
	return err;
}

static int ubi_dev_scan(struct mtd_info *info, const char *vid_header_offset)
{
	char ubi_mtd_param_buffer[80];
	int err;

	if (!vid_header_offset)
		sprintf(ubi_mtd_param_buffer, "%s", info->name);
	else
		sprintf(ubi_mtd_param_buffer, "%s,%s", info->name,
			vid_header_offset);

	err = ubi_mtd_param_parse(ubi_mtd_param_buffer, NULL);
	if (err)
		return -err;

	err = ubi_init();
	if (err)
		return -err;

	return 0;
}

static int ubi_set_skip_check(char *volume, bool skip_check)
{
	struct ubi_vtbl_record vtbl_rec;
	struct ubi_volume *vol;

	vol = ubi_find_volume(volume);
	if (!vol)
		return ENODEV;

	printf("%sing skip_check on volume %s\n",
	       skip_check ? "Sett" : "Clear", volume);

	vtbl_rec = ubi->vtbl[vol->vol_id];
	if (skip_check) {
		vtbl_rec.flags |= UBI_VTBL_SKIP_CRC_CHECK_FLG;
		vol->skip_check = 1;
	} else {
		vtbl_rec.flags &= ~UBI_VTBL_SKIP_CRC_CHECK_FLG;
		vol->skip_check = 0;
	}

	return ubi_change_vtbl_record(ubi, vol->vol_id, &vtbl_rec);
}

static int ubi_detach(void)
{
#ifdef CONFIG_CMD_UBIFS
	/*
	 * Automatically unmount UBIFS partition when user
	 * changes the UBI device. Otherwise the following
	 * UBIFS commands will crash.
	 */
	if (ubifs_is_mounted())
		cmd_ubifs_umount();
#endif

	/*
	 * Call ubi_exit() before re-initializing the UBI subsystem
	 */
	if (ubi)
		ubi_exit();

	ubi = NULL;

	return 0;
}

int ubi_part(char *part_name, const char *vid_header_offset)
{
	struct mtd_info *mtd;
	int err = 0;

	if (ubi && ubi->mtd && !strcmp(ubi->mtd->name, part_name)) {
		printf("UBI partition '%s' already selected\n", part_name);
		return 0;
	}

	ubi_detach();

	mtd_probe_devices();
	mtd = get_mtd_device_nm(part_name);
	if (IS_ERR(mtd)) {
		printf("Partition %s not found!\n", part_name);
		return 1;
	}
	put_mtd_device(mtd);

	err = ubi_dev_scan(mtd, vid_header_offset);
	if (err) {
		printf("UBI init error %d\n", err);
		printf("Please check, if the correct MTD partition is used (size big enough?)\n");
		return err;
	}

	ubi = ubi_devices[0];

	return 0;
}

static int do_ubi(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int64_t size = 0;
	ulong addr = 0;
	bool skipcheck = false;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "detach") == 0)
		return ubi_detach();

	if (strcmp(argv[1], "part") == 0) {
		const char *vid_header_offset = NULL;

		/* Print current partition */
		if (argc == 2) {
			if (!ubi) {
				printf("Error, no UBI device selected!\n");
				return 1;
			}

			printf("Device %d: %s, MTD partition %s\n",
			       ubi->ubi_num, ubi->ubi_name, ubi->mtd->name);
			return 0;
		}

		if (argc < 3)
			return CMD_RET_USAGE;

		if (argc > 3)
			vid_header_offset = argv[3];

		return ubi_part(argv[2], vid_header_offset);
	}

	if ((strcmp(argv[1], "part") != 0) && !ubi) {
		printf("Error, no UBI device selected!\n");
		return 1;
	}

	if (strcmp(argv[1], "info") == 0) {
		int layout = 0;
		if (argc > 2 && !strncmp(argv[2], "l", 1))
			layout = 1;
		return ubi_info(layout);
	}

	if (strcmp(argv[1], "list") == 0) {
		int numeric = 0;
		if (argc >= 3 && argv[2][0] == '-') {
			if (strcmp(argv[2], "-numeric") == 0)
				numeric = 1;
			else
				return CMD_RET_USAGE;
		}
		if (!numeric && argc != 2 && argc != 3)
			return CMD_RET_USAGE;
		if (numeric && argc != 3 && argc != 4)
			return CMD_RET_USAGE;
		return ubi_list(argv[numeric ? 3 : 2], numeric);
	}

	if (strcmp(argv[1], "check") == 0) {
		if (argc > 2)
			return ubi_check(argv[2]);

		printf("Error, no volume name passed\n");
		return 1;
	}

	if (strncmp(argv[1], "create", 6) == 0) {
		int dynamic = 1;	/* default: dynamic volume */
		int id = UBI_VOL_NUM_AUTO;

		/* Use maximum available size */
		size = 0;

		/* E.g., create volume with "skipcheck" bit set */
		if (argc == 7) {
			skipcheck = strncmp(argv[6], "--skipcheck", 11) == 0;
			argc--;
		}

		/* E.g., create volume size type vol_id */
		if (argc == 6) {
			id = simple_strtoull(argv[5], NULL, 16);
			argc--;
		}

		/* E.g., create volume size type */
		if (argc == 5) {
			if (strncmp(argv[4], "s", 1) == 0)
				dynamic = 0;
			else if (strncmp(argv[4], "d", 1) != 0) {
				printf("Incorrect type\n");
				return 1;
			}
			argc--;
		}
		/* E.g., create volume size */
		if (argc == 4) {
			if (argv[3][0] != '-')
				size = simple_strtoull(argv[3], NULL, 16);
			argc--;
		}
		/* Use maximum available size */
		if (!size) {
			size = (int64_t)ubi->avail_pebs * ubi->leb_size;
			printf("No size specified -> Using max size (%lld)\n", size);
		}
		/* E.g., create volume */
		if (argc == 3) {
			return ubi_create_vol(argv[2], size, dynamic, id,
					      skipcheck);
		}
	}

	if (strncmp(argv[1], "remove", 6) == 0) {
		/* E.g., remove volume */
		if (argc == 3)
			return ubi_remove_vol(argv[2]);
	}

	if (IS_ENABLED(CONFIG_CMD_UBI_RENAME) && !strncmp(argv[1], "rename", 6))
		return ubi_rename_vol(argv[2], argv[3]);

	if (strncmp(argv[1], "skipcheck", 9) == 0) {
		/* E.g., change skip_check flag */
		if (argc == 4) {
			skipcheck = strncmp(argv[3], "on", 2) == 0;
			return ubi_set_skip_check(argv[2], skipcheck);
		}
	}

	if (strncmp(argv[1], "write", 5) == 0) {
		int ret;

		if (argc < 5) {
			printf("Please see usage\n");
			return 1;
		}

		addr = hextoul(argv[2], NULL);
		size = hextoul(argv[4], NULL);

		if (strlen(argv[1]) == 10 &&
		    strncmp(argv[1] + 5, ".part", 5) == 0) {
			if (argc < 6) {
				ret = ubi_volume_continue_write(argv[3],
						(void *)addr, size);
			} else {
				size_t full_size;
				full_size = hextoul(argv[5], NULL);
				ret = ubi_volume_begin_write(argv[3],
						(void *)addr, size, full_size);
			}
		} else {
			ret = ubi_volume_write(argv[3], (void *)addr, size);
		}
		if (!ret) {
			printf("%lld bytes written to volume %s\n", size,
			       argv[3]);
		}

		return ret;
	}

	if (strncmp(argv[1], "read", 4) == 0) {
		size = 0;

		/* E.g., read volume size */
		if (argc == 5) {
			size = hextoul(argv[4], NULL);
			argc--;
		}

		/* E.g., read volume */
		if (argc == 4) {
			addr = hextoul(argv[2], NULL);
			argc--;
		}

		if (argc == 3) {
			return ubi_volume_read(argv[3], (char *)addr, size);
		}
	}

	printf("Please see usage\n");
	return 1;
}

U_BOOT_CMD(
	ubi, 7, 1, do_ubi,
	"ubi commands",
	"detach"
		" - detach ubi from a mtd partition\n"
	"ubi part [part] [offset]\n"
		" - Show or set current partition (with optional VID"
		" header offset)\n"
	"ubi info [l[ayout]]"
		" - Display volume and ubi layout information\n"
	"ubi list [flags]"
		" - print the list of volumes\n"
	"ubi list [flags] <varname>"
		" - set environment variable to the list of volumes"
		" (flags can be -numeric)\n"
	"ubi check volumename"
		" - check if volumename exists\n"
	"ubi create[vol] volume [size] [type] [id] [--skipcheck]\n"
		" - create volume name with size ('-' for maximum"
		" available size)\n"
	"ubi write[vol] address volume size"
		" - Write volume from address with size\n"
	"ubi write.part address volume size [fullsize]\n"
		" - Write part of a volume from address\n"
	"ubi read[vol] address volume [size]"
		" - Read volume to address with size\n"
	"ubi remove[vol] volume"
		" - Remove volume\n"
#if IS_ENABLED(CONFIG_CMD_UBI_RENAME)
	"ubi rename oldname newname\n"
#endif
	"ubi skipcheck volume on/off - Set or clear skip_check flag in volume header\n"
	"[Legends]\n"
	" volume: character name\n"
	" size: specified in bytes\n"
	" type: s[tatic] or d[ynamic] (default=dynamic)"
);
