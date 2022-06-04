/*
 * Copyright (C) 2017 David Oberhollenzer - sigma star gmbh
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; see the file COPYING. If not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA.
 *
 * Author: David Oberhollenzer <david.oberhollenzer@sigma-star.at>
 */
#include "lsmtd.h"

struct ubi_node *ubi_dev;
int num_ubi_devices;

struct mtd_node *mtd_dev;
int num_mtd_devices;

static int compare_mtd(const void *l, const void *r)
{
	const struct mtd_node *a = l, *b = r;

	switch (sort_by->type) {
	case COL_DEVNAME:
		return a->info.mtd_num - b->info.mtd_num;
	case COL_DEVNUM:
		if (a->info.major == b->info.major)
			return a->info.minor - b->info.minor;
		return a->info.major - b->info.major;
	case COL_TYPE:
		return strcmp(a->info.type_str, b->info.type_str);
	case COL_NAME:
		return strcmp(a->info.name, b->info.name);
	case COL_SIZE:
		if (a->info.size < b->info.size)
			return -1;
		if (a->info.size > b->info.size)
			return 1;
		return 0;
	case COL_EBSIZE:
		return a->info.eb_size - b->info.eb_size;
	case COL_EBCOUNT:
		return a->info.eb_cnt - b->info.eb_cnt;
	case COL_MINIO:
		return a->info.min_io_size - b->info.min_io_size;
	case COL_SUBSIZE:
		return a->info.subpage_size - b->info.subpage_size;
	case COL_OOBSIZE:
		return a->info.oob_size - b->info.oob_size;
	case COL_RO:
		return !a->info.writable - !b->info.writable;
	case COL_BB:
		return a->info.bb_allowed - b->info.bb_allowed;
	case COL_REGION:
		return a->info.region_cnt - b->info.region_cnt;
	}
	return 0;
}

static int compare_ubi_vol(const void *l, const void *r)
{
	const struct ubi_vol_info *a = l, *b = r;
	long long all, bll;

	switch (sort_by->type) {
	case COL_DEVNAME:
		if (a->dev_num == b->dev_num)
			return a->vol_id - b->vol_id;
		return a->dev_num - b->dev_num;
	case COL_DEVNUM:
		if (a->major == b->major)
			return a->minor - b->minor;
		return a->major - b->major;
	case COL_TYPE:
		if (a->type == b->type)
			return 0;
		return a->type == UBI_DYNAMIC_VOLUME ? 1 : -1;
	case COL_NAME:
		return strcmp(a->name, b->name);
	case COL_SIZE:
		all = a->rsvd_bytes;
		bll = b->rsvd_bytes;
		goto out_ll;
	case COL_EBSIZE:
		return a->leb_size - b->leb_size;
	case COL_EBCOUNT:
		return a->rsvd_lebs - b->rsvd_lebs;
	case COL_FREE:
	case COL_FREE_LEB:
		all = (a->rsvd_bytes - a->data_bytes);
		bll = (b->rsvd_bytes - b->data_bytes);
		goto out_ll;
	case COL_CORRUPTED:
		return a->corrupted - b->corrupted;
	}
	return 0;
out_ll:
	return (all < bll) ? -1 : ((all > bll) ? 1 : 0);
}

static int scan_ubi_device(libubi_t lib_ubi, struct ubi_node *dev)
{
	int lo = dev->info.lowest_vol_id, hi = dev->info.highest_vol_id;
	int i, idx = 0, dev_num = dev->info.dev_num;
	struct ubi_vol_info vol_info;

	if (!dev->info.vol_count)
		return 0;

	dev->vol_info = xcalloc(dev->info.vol_count, sizeof(dev->vol_info[0]));

	for (i = lo; i <= hi; ++i) {
		if (ubi_get_vol_info1(lib_ubi, dev_num, i, &vol_info)) {
			if (errno == ENOENT)
				continue;
			perror("ubi_get_vol_info1");
			return -1;
		}

		dev->vol_info[idx++] = vol_info;
	}

	if (sort_by)
		qsort(dev->vol_info, idx, sizeof(vol_info), compare_ubi_vol);
	return 0;
}

int scan_ubi(libubi_t lib_ubi)
{
	struct ubi_dev_info dev_info;
	struct ubi_info info;
	int i, j;

	if (ubi_get_info(lib_ubi, &info))
		return -1;

	if (!info.dev_count)
		return 0;

	ubi_dev = xcalloc(info.dev_count, sizeof(ubi_dev[0]));

	for (i = info.lowest_dev_num; i <= info.highest_dev_num; ++i) {
		if (!ubi_dev_present(lib_ubi, i))
			continue;

		if (ubi_get_dev_info1(lib_ubi, i, &dev_info)) {
			perror("ubi_get_dev_info1");
			return -1;
		}

		for (j = 0; j < num_mtd_devices; ++j) {
			if (mtd_dev[j].info.mtd_num == dev_info.mtd_num)
				break;
		}

		if (j == num_mtd_devices) {
			fprintf(stderr, "Cannot find mtd device %d refered to "
				"by ubi device %d\n", dev_info.mtd_num,
				dev_info.dev_num);
			return -1;
		}

		ubi_dev[num_ubi_devices].info = dev_info;
		mtd_dev[j].ubi = ubi_dev + num_ubi_devices;

		if (scan_ubi_device(lib_ubi, ubi_dev + num_ubi_devices))
			return -1;

		++num_ubi_devices;
	}
	return 0;
}

int scan_mtd(libmtd_t lib_mtd)
{
	struct mtd_dev_info dev_info;
	struct mtd_info info;
	int i, idx = 0;

	if (mtd_get_info(lib_mtd, &info))
		return -1;

	if (!info.mtd_dev_cnt)
		return 0;

	mtd_dev = xcalloc(info.mtd_dev_cnt, sizeof(mtd_dev[0]));

	for (i = info.lowest_mtd_num; i <= info.highest_mtd_num; ++i) {
		if (!mtd_dev_present(lib_mtd, i))
			continue;

		if (mtd_get_dev_info1(lib_mtd, i, &dev_info)) {
			perror("mtd_get_dev_info1");
			return -1;
		}

		memcpy(&(mtd_dev[idx++].info), &dev_info, sizeof(dev_info));
	}

	num_mtd_devices = idx;

	if (sort_by)
		qsort(mtd_dev, num_mtd_devices, sizeof(*mtd_dev), compare_mtd);
	return 0;
}

void scan_free(void)
{
	int i;

	for (i = 0; i < num_ubi_devices; ++i)
		free(ubi_dev[i].vol_info);

	free(ubi_dev);
	free(mtd_dev);
}
