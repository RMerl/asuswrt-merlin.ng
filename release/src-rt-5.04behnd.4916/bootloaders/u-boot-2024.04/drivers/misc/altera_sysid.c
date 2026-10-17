// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <errno.h>
#include <misc.h>
#include <linux/time.h>
#include <asm/io.h>

struct altera_sysid_regs {
	u32	id;		/* The system build id */
	u32	timestamp;	/* Timestamp */
};

struct altera_sysid_plat {
	struct altera_sysid_regs *regs;
};

void display_sysid(void)
{
	struct udevice *dev;
	u32 sysid[2];
	struct tm t;
	char asc[32];
	time_t stamp;
	int ret;

	/* the first misc device will be used */
	ret = uclass_first_device_err(UCLASS_MISC, &dev);
	if (ret)
		return;
	ret = misc_read(dev, 0, &sysid, sizeof(sysid));
	if (ret < 0)
		return;

	stamp = sysid[1];
	localtime_r(&stamp, &t);
	asctime_r(&t, asc);
	printf("SYSID: %08x, %s", sysid[0], asc);
}

int do_sysid(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	display_sysid();
	return 0;
}

U_BOOT_CMD(
	sysid,	1,	1,	do_sysid,
	"display Nios-II system id",
	""
);

static int altera_sysid_read(struct udevice *dev,
			     int offset, void *buf, int size)
{
	struct altera_sysid_plat *plat = dev_get_plat(dev);
	struct altera_sysid_regs *const regs = plat->regs;
	u32 *sysid = buf;

	sysid[0] = readl(&regs->id);
	sysid[1] = readl(&regs->timestamp);

	return 0;
}

static int altera_sysid_of_to_plat(struct udevice *dev)
{
	struct altera_sysid_plat *plat = dev_get_plat(dev);

	plat->regs = map_physmem(dev_read_addr(dev),
				 sizeof(struct altera_sysid_regs),
				 MAP_NOCACHE);

	return 0;
}

static const struct misc_ops altera_sysid_ops = {
	.read = altera_sysid_read,
};

static const struct udevice_id altera_sysid_ids[] = {
	{ .compatible = "altr,sysid-1.0" },
	{}
};

U_BOOT_DRIVER(altera_sysid) = {
	.name	= "altera_sysid",
	.id	= UCLASS_MISC,
	.of_match = altera_sysid_ids,
	.of_to_plat = altera_sysid_of_to_plat,
	.plat_auto	= sizeof(struct altera_sysid_plat),
	.ops	= &altera_sysid_ops,
};
