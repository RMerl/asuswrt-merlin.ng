/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 */
#ifndef _BOOTCOUNT_H__
#define _BOOTCOUNT_H__

#include <common.h>
#include <asm/io.h>
#include <asm/byteorder.h>

#ifdef CONFIG_DM_BOOTCOUNT

struct bootcount_ops {
	/**
	 * get() - get the current bootcount value
	 *
	 * Returns the current counter value of the bootcount backing
	 * store.
	 *
	 * @dev:	Device to read from
	 * @bootcount:	Address to put the current bootcount value
	 */
	int (*get)(struct udevice *dev, u32 *bootcount);

	/**
	 * set() - set a bootcount value (e.g. to reset or increment)
	 *
	 * Sets the value in the bootcount backing store.
	 *
	 * @dev:	Device to read from
	 * @bootcount:	New bootcount value to store
	 */
	int (*set)(struct udevice *dev, const u32 bootcount);
};

/* Access the operations for a bootcount device */
#define bootcount_get_ops(dev)	((struct bootcount_ops *)(dev)->driver->ops)

/**
 * dm_bootcount_get() - Read the current value from a bootcount storage
 *
 * @dev:	Device to read from
 * @bootcount:	Place to put the current bootcount
 * @return 0 if OK, -ve on error
 */
int dm_bootcount_get(struct udevice *dev, u32 *bootcount);

/**
 * dm_bootcount_set() - Write a value to a bootcount storage
 *
 * @dev:	Device to read from
 * @bootcount:  Value to be written to the backing storage
 * @return 0 if OK, -ve on error
 */
int dm_bootcount_set(struct udevice *dev, u32 bootcount);

#endif

#if defined(CONFIG_SPL_BOOTCOUNT_LIMIT) || defined(CONFIG_BOOTCOUNT_LIMIT)

#if !defined(CONFIG_SYS_BOOTCOUNT_LE) && !defined(CONFIG_SYS_BOOTCOUNT_BE)
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define CONFIG_SYS_BOOTCOUNT_LE
# else
#  define CONFIG_SYS_BOOTCOUNT_BE
# endif
#endif

#ifdef CONFIG_SYS_BOOTCOUNT_LE
static inline void raw_bootcount_store(volatile u32 *addr, u32 data)
{
	out_le32(addr, data);
}

static inline u32 raw_bootcount_load(volatile u32 *addr)
{
	return in_le32(addr);
}
#else
static inline void raw_bootcount_store(volatile u32 *addr, u32 data)
{
	out_be32(addr, data);
}

static inline u32 raw_bootcount_load(volatile u32 *addr)
{
	return in_be32(addr);
}
#endif

DECLARE_GLOBAL_DATA_PTR;
static inline int bootcount_error(void)
{
	unsigned long bootcount = bootcount_load();
	unsigned long bootlimit = env_get_ulong("bootlimit", 10, 0);

	if (bootlimit && bootcount > bootlimit) {
		printf("Warning: Bootlimit (%lu) exceeded.", bootlimit);
		if (!(gd->flags & GD_FLG_SPL_INIT))
			printf(" Using altbootcmd.");
		printf("\n");

		return 1;
	}

	return 0;
}

static inline void bootcount_inc(void)
{
	unsigned long bootcount = bootcount_load();

	if (gd->flags & GD_FLG_SPL_INIT) {
		bootcount_store(++bootcount);
		return;
	}

#ifndef CONFIG_SPL_BUILD
	/* Only increment bootcount when no bootcount support in SPL */
#ifndef CONFIG_SPL_BOOTCOUNT_LIMIT
	bootcount_store(++bootcount);
#endif
	env_set_ulong("bootcount", bootcount);
#endif /* !CONFIG_SPL_BUILD */
}

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_LIMIT)
void bootcount_store(ulong a) {};
ulong bootcount_load(void) { return 0; }
#endif /* CONFIG_SPL_BUILD && !CONFIG_SPL_BOOTCOUNT_LIMIT */
#else
static inline int bootcount_error(void) { return 0; }
static inline void bootcount_inc(void) {}
#endif /* CONFIG_SPL_BOOTCOUNT_LIMIT || CONFIG_BOOTCOUNT_LIMIT */
#endif /* _BOOTCOUNT_H__ */
