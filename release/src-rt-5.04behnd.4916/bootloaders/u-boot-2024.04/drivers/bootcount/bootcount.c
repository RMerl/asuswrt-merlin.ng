// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010-2012
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 */

#include <bootcount.h>
#include <cpu_func.h>
#include <asm/cache.h>
#include <linux/compiler.h>

#if !defined(CONFIG_DM_BOOTCOUNT)
/* Now implement the generic default functions */
__weak void bootcount_store(ulong a)
{
	void *reg = (void *)CONFIG_SYS_BOOTCOUNT_ADDR;
	uintptr_t flush_start = rounddown(CONFIG_SYS_BOOTCOUNT_ADDR,
					  CONFIG_SYS_CACHELINE_SIZE);
	uintptr_t flush_end;

#if defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD)
	raw_bootcount_store(reg, (CONFIG_SYS_BOOTCOUNT_MAGIC & 0xffff0000) | a);

	flush_end = roundup(CONFIG_SYS_BOOTCOUNT_ADDR + 4,
			    CONFIG_SYS_CACHELINE_SIZE);
#else
	raw_bootcount_store(reg, a);
	raw_bootcount_store(reg + 4, CONFIG_SYS_BOOTCOUNT_MAGIC);

	flush_end = roundup(CONFIG_SYS_BOOTCOUNT_ADDR + 8,
			    CONFIG_SYS_CACHELINE_SIZE);
#endif /* defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD */
	flush_dcache_range(flush_start, flush_end);
}

__weak ulong bootcount_load(void)
{
	void *reg = (void *)CONFIG_SYS_BOOTCOUNT_ADDR;

#if defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD)
	u32 tmp = raw_bootcount_load(reg);

	if ((tmp & 0xffff0000) != (CONFIG_SYS_BOOTCOUNT_MAGIC & 0xffff0000))
		return 0;
	else
		return (tmp & 0x0000ffff);
#else
	if (raw_bootcount_load(reg + 4) != CONFIG_SYS_BOOTCOUNT_MAGIC)
		return 0;
	else
		return raw_bootcount_load(reg);
#endif /* defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD) */
}
#else
#include <dm.h>

/*
 * struct bootcount_mem_priv - private bootcount mem driver data
 *
 * @base: base address used for bootcounter
 * @singleword: if true use only one 32 bit word for bootcounter
 */
struct bootcount_mem_priv {
	phys_addr_t base;
	bool singleword;
};

static int bootcount_mem_get(struct udevice *dev, u32 *a)
{
	struct bootcount_mem_priv *priv = dev_get_priv(dev);
	void *reg = (void *)priv->base;
	u32 magic = CONFIG_SYS_BOOTCOUNT_MAGIC;

	if (priv->singleword) {
		u32 tmp = raw_bootcount_load(reg);

		if ((tmp & 0xffff0000) != (magic & 0xffff0000))
			return -ENODEV;

		*a = (tmp & 0x0000ffff);
	} else {
		if (raw_bootcount_load(reg + 4) != magic)
			return -ENODEV;

		*a = raw_bootcount_load(reg);
	}

	return 0;
};

static int bootcount_mem_set(struct udevice *dev, const u32 a)
{
	struct bootcount_mem_priv *priv = dev_get_priv(dev);
	void *reg = (void *)priv->base;
	u32 magic = CONFIG_SYS_BOOTCOUNT_MAGIC;
	uintptr_t flush_start = rounddown(priv->base,
					  CONFIG_SYS_CACHELINE_SIZE);
	uintptr_t flush_end;

	if (priv->singleword) {
		raw_bootcount_store(reg, (magic & 0xffff0000) | a);
		flush_end = roundup(priv->base + 4,
				    CONFIG_SYS_CACHELINE_SIZE);
	} else {
		raw_bootcount_store(reg, a);
		raw_bootcount_store(reg + 4, magic);
		flush_end = roundup(priv->base + 8,
				    CONFIG_SYS_CACHELINE_SIZE);
	}
	flush_dcache_range(flush_start, flush_end);

	return 0;
};

static const struct bootcount_ops bootcount_mem_ops = {
	.get = bootcount_mem_get,
	.set = bootcount_mem_set,
};

static int bootcount_mem_probe(struct udevice *dev)
{
	struct bootcount_mem_priv *priv = dev_get_priv(dev);

	priv->base = (phys_addr_t)dev_read_addr(dev);
	if (dev_read_bool(dev, "single-word"))
		priv->singleword = true;

	return 0;
}

static const struct udevice_id bootcount_mem_ids[] = {
	{ .compatible = "u-boot,bootcount" },
	{ }
};

U_BOOT_DRIVER(bootcount_mem) = {
	.name	= "bootcount-mem",
	.id	= UCLASS_BOOTCOUNT,
	.priv_auto	= sizeof(struct bootcount_mem_priv),
	.probe	= bootcount_mem_probe,
	.of_match = bootcount_mem_ids,
	.ops	= &bootcount_mem_ops,
};
#endif
