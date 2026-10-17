// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014, NVIDIA CORPORATION. All rights reserved.
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <div64.h>
#include <dfu.h>
#include <spi.h>
#include <spi_flash.h>
#include <jffs2/load_kernel.h>
#include <linux/mtd/mtd.h>
#include <linux/ctype.h>

static int dfu_get_medium_size_sf(struct dfu_entity *dfu, u64 *size)
{
	*size = dfu->data.sf.size;

	return 0;
}

static int dfu_read_medium_sf(struct dfu_entity *dfu, u64 offset, void *buf,
			      long *len)
{
	long seglen = *len;
	int ret;

	if (seglen > (16 << 20))
		seglen = (16 << 20);

	ret = spi_flash_read(dfu->data.sf.dev, dfu->data.sf.start + offset,
		seglen, buf);
	if (!ret)
		*len = seglen;

	return ret;
}

static u64 find_sector(struct dfu_entity *dfu, u64 start, u64 offset)
{
	return (lldiv((start + offset), dfu->data.sf.dev->sector_size)) *
		dfu->data.sf.dev->sector_size;
}

static int dfu_write_medium_sf(struct dfu_entity *dfu,
			       u64 offset, void *buf, long *len)
{
	int ret;

	ret = spi_flash_erase(dfu->data.sf.dev,
			      find_sector(dfu, dfu->data.sf.start, offset),
			      dfu->data.sf.dev->sector_size);
	if (ret)
		return ret;

	ret = spi_flash_write(dfu->data.sf.dev, dfu->data.sf.start + offset,
			      *len, buf);
	if (ret)
		return ret;

	return 0;
}

static int dfu_flush_medium_sf(struct dfu_entity *dfu)
{
	u64 off, length;

	if (!CONFIG_IS_ENABLED(DFU_SF_PART) || !dfu->data.sf.ubi)
		return 0;

	/* in case of ubi partition, erase rest of the partition */
	off = find_sector(dfu, dfu->data.sf.start, dfu->offset);
	/* last write ended with unaligned length jump to next */
	if (off != dfu->data.sf.start + dfu->offset)
		off += dfu->data.sf.dev->sector_size;
	length = dfu->data.sf.start + dfu->data.sf.size - off;
	if (length)
		return spi_flash_erase(dfu->data.sf.dev, off, length);

	return 0;
}

static unsigned int dfu_polltimeout_sf(struct dfu_entity *dfu)
{
	/*
	 * Currently, Poll Timeout != 0 is only needed on nand
	 * ubi partition, as sectors which are not used need
	 * to be erased
	 */
	if (CONFIG_IS_ENABLED(DFU_SF_PART) && dfu->data.sf.ubi)
		return DFU_MANIFEST_POLL_TIMEOUT;

	return DFU_DEFAULT_POLL_TIMEOUT;
}

static void dfu_free_entity_sf(struct dfu_entity *dfu)
{
	/*
	 * In the DM case it is not necessary to free the SPI device.
	 * For the non-DM case we must ensure that the the SPI device is only
	 * freed once.
	 */
	if (!CONFIG_IS_ENABLED(DM_SPI_FLASH)) {
		struct spi_flash *dev = dfu->data.sf.dev;

		if (!dev)
			return;
		dfu->data.sf.dev = NULL;
		list_for_each_entry(dfu, &dfu_list, list) {
			if (dfu->data.sf.dev == dev)
				dfu->data.sf.dev = NULL;
		}
		spi_flash_free(dev);
	}
}

static struct spi_flash *parse_dev(char *devstr)
{
	unsigned int bus;
	unsigned int cs;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	char *s, *endp;
	struct spi_flash *dev;

	s = strsep(&devstr, ":");
	if (!s || !*s || (bus = simple_strtoul(s, &endp, 0), *endp)) {
		printf("Invalid SPI bus %s\n", s);
		return NULL;
	}

	s = strsep(&devstr, ":");
	if (!s || !*s || (cs = simple_strtoul(s, &endp, 0), *endp)) {
		printf("Invalid SPI chip-select %s\n", s);
		return NULL;
	}

	s = strsep(&devstr, ":");
	if (s && *s) {
		speed = simple_strtoul(s, &endp, 0);
		if (*endp || !speed) {
			printf("Invalid SPI speed %s\n", s);
			return NULL;
		}
	}

	s = strsep(&devstr, ":");
	if (s && *s) {
		mode = simple_strtoul(s, &endp, 0);
		if (*endp || mode > 3) {
			printf("Invalid SPI mode %s\n", s);
			return NULL;
		}
	}

	dev = spi_flash_probe(bus, cs, speed, mode);
	if (!dev) {
		printf("Failed to create SPI flash at %u:%u:%u:%u\n",
		       bus, cs, speed, mode);
		return NULL;
	}

	return dev;
}

int dfu_fill_entity_sf(struct dfu_entity *dfu, char *devstr, char **argv, int argc)
{
	char *s;
	char *devstr_bkup = strdup(devstr);

	dfu->data.sf.dev = parse_dev(devstr_bkup);
	free(devstr_bkup);
	if (!dfu->data.sf.dev)
		return -ENODEV;

	dfu->dev_type = DFU_DEV_SF;
	dfu->max_buf_size = dfu->data.sf.dev->sector_size;

	if (argc != 3)
		return -EINVAL;
	if (!strcmp(argv[0], "raw")) {
		dfu->layout = DFU_RAW_ADDR;
		dfu->data.sf.start = hextoul(argv[1], &s);
		if (*s)
			return -EINVAL;
		dfu->data.sf.size = hextoul(argv[2], &s);
		if (*s)
			return -EINVAL;
	} else if (CONFIG_IS_ENABLED(DFU_SF_PART) &&
		   (!strcmp(argv[0], "part") || !strcmp(argv[0], "partubi"))) {
		char mtd_id[32];
		struct mtd_device *mtd_dev;
		u8 part_num;
		struct part_info *pi;
		int ret, dev, part;

		dfu->layout = DFU_RAW_ADDR;

		dev = dectoul(argv[1], &s);
		if (*s)
			return -EINVAL;
		part = dectoul(argv[2], &s);
		if (*s)
			return -EINVAL;

		sprintf(mtd_id, "%s%d,%d", "nor", dev, part - 1);
		printf("using id '%s'\n", mtd_id);

		mtdparts_init();

		ret = find_dev_and_part(mtd_id, &mtd_dev, &part_num, &pi);
		if (ret != 0) {
			printf("Could not locate '%s'\n", mtd_id);
			return -1;
		}
		dfu->data.sf.start = pi->offset;
		dfu->data.sf.size = pi->size;
		if (!strcmp(argv[0], "partubi"))
			dfu->data.sf.ubi = 1;
	} else {
		printf("%s: Memory layout (%s) not supported!\n", __func__, argv[0]);
		spi_flash_free(dfu->data.sf.dev);
		return -1;
	}

	dfu->get_medium_size = dfu_get_medium_size_sf;
	dfu->read_medium = dfu_read_medium_sf;
	dfu->write_medium = dfu_write_medium_sf;
	dfu->flush_medium = dfu_flush_medium_sf;
	dfu->poll_timeout = dfu_polltimeout_sf;
	dfu->free_entity = dfu_free_entity_sf;

	/* initial state */
	dfu->inited = 0;

	return 0;
}
