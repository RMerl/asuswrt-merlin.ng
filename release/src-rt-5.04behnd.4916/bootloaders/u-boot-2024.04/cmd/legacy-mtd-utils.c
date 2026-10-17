// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <jffs2/jffs2.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/string.h>
#include <mtd.h>

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

	if (*size == 0) {
		debug("ERROR: Invalid size 0\n");
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
