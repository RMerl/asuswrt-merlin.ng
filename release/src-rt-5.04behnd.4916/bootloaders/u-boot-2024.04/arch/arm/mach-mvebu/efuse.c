// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Reinhard Pfau <reinhard.pfau@gdsys.cc>
 */

#include <config.h>
#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/efuse.h>
#include <asm/arch/soc.h>
#include <asm/gpio.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/mbus.h>

#if defined(CONFIG_MVEBU_EFUSE_FAKE)
#define DRY_RUN
#else
#undef DRY_RUN
#endif

#define MBUS_EFUSE_BASE 0xF6000000
#define MBUS_EFUSE_SIZE BIT(20)

#define MVEBU_EFUSE_CONTROL (MVEBU_REGISTER(0xE4008))

enum {
	MVEBU_EFUSE_CTRL_PROGRAM_ENABLE = (1 << 31),
	MVEBU_EFUSE_LD1_SELECT          = (1 <<  6),
};

struct mvebu_hd_efuse {
	u32 bits_31_0;
	u32 bits_63_32;
	u32 bit64;
	u32 reserved0;
};

#ifndef DRY_RUN
static struct mvebu_hd_efuse *efuses =
	(struct mvebu_hd_efuse *)(MBUS_EFUSE_BASE + 0xF9000);
static u32 *ld_efuses = (void *)MBUS_EFUSE_BASE + 0xF8F00;
#else
static struct mvebu_hd_efuse efuses[EFUSE_LINE_MAX + 1];
static u32 ld_efuses[EFUSE_LD_WORDS];
#endif

static int efuse_initialised;

static struct mvebu_hd_efuse *get_efuse_line(int nr)
{
	if (nr < 0 || nr > 63 || !efuse_initialised)
		return NULL;

	return efuses + nr;
}

#ifndef DRY_RUN
static int vhv_gpio;
#endif

static int enable_efuse_program(void)
{
#ifndef DRY_RUN
	if (CONFIG_MVEBU_EFUSE_VHV_GPIO[0]) {
		if (gpio_lookup_name(CONFIG_MVEBU_EFUSE_VHV_GPIO, NULL, NULL, &vhv_gpio)) {
			printf("Error: VHV gpio lookup failed\n");
			return -EOPNOTSUPP;
		}
		if (gpio_request(vhv_gpio, CONFIG_MVEBU_EFUSE_VHV_GPIO)) {
			printf("Error: VHV gpio request failed\n");
			return -EOPNOTSUPP;
		}
		if (gpio_direction_output(vhv_gpio,
		    IS_ENABLED(CONFIG_MVEBU_EFUSE_VHV_GPIO_ACTIVE_LOW) ? 0 : 1)) {
			printf("Error: VHV gpio enable failed\n");
			return -EINVAL;
		}
		mdelay(5); /* Wait for the VHV power to stabilize */
	}

	setbits_le32(MVEBU_EFUSE_CONTROL, MVEBU_EFUSE_CTRL_PROGRAM_ENABLE);
#endif

	return 0;
}

static void disable_efuse_program(void)
{
#ifndef DRY_RUN
	clrbits_le32(MVEBU_EFUSE_CONTROL, MVEBU_EFUSE_CTRL_PROGRAM_ENABLE);

	if (CONFIG_MVEBU_EFUSE_VHV_GPIO[0]) {
		if (gpio_direction_output(vhv_gpio,
		    IS_ENABLED(CONFIG_MVEBU_EFUSE_VHV_GPIO_ACTIVE_LOW) ? 1 : 0))
			printf("Error: VHV gpio disable failed\n");
		gpio_free(vhv_gpio);
		vhv_gpio = 0;
	}
#endif
}

static int do_prog_efuse(struct mvebu_hd_efuse *efuse,
			 struct efuse_val *new_val, u32 mask0, u32 mask1)
{
	struct efuse_val val;

	val.dwords.d[0] = readl(&efuse->bits_31_0);
	val.dwords.d[1] = readl(&efuse->bits_63_32);
	val.lock = readl(&efuse->bit64);

	if (val.lock & 1)
		return -EPERM;

	val.dwords.d[0] |= (new_val->dwords.d[0] & mask0);
	val.dwords.d[1] |= (new_val->dwords.d[1] & mask1);
	val.lock |= new_val->lock;

	writel(val.dwords.d[0], &efuse->bits_31_0);
	mdelay(1);
	writel(val.dwords.d[1], &efuse->bits_63_32);
	mdelay(1);
	writel(val.lock, &efuse->bit64);
	mdelay(5);

	return 0;
}

static int prog_efuse(int nr, struct efuse_val *new_val, u32 mask0, u32 mask1)
{
	struct mvebu_hd_efuse *efuse;
	int res = 0;

	res = mvebu_efuse_init_hw();
	if (res)
		return res;

	efuse = get_efuse_line(nr);
	if (!efuse)
		return -ENODEV;

	if (!new_val)
		return -EINVAL;

	/* only write a fuse line with lock bit */
	if (!new_val->lock)
		return -EINVAL;

	/* according to specs ECC protection bits must be 0 on write */
	if (new_val->bytes.d[7] & 0xFE)
		return -EINVAL;

	if (!new_val->dwords.d[0] && !new_val->dwords.d[1] && (mask0 | mask1))
		return 0;

	res = enable_efuse_program();
	if (res)
		return res;

	res = do_prog_efuse(efuse, new_val, mask0, mask1);

	disable_efuse_program();

	return res;
}

int mvebu_prog_ld_efuse(int ld1, u32 word, u32 val)
{
	int i, res;
	u32 line[EFUSE_LD_WORDS];

	res = mvebu_efuse_init_hw();
	if (res)
		return res;

	mvebu_read_ld_efuse(ld1, line);

	/* check if lock bit is already programmed */
	if (line[EFUSE_LD_WORDS - 1])
		return -EPERM;

	/* check if word is valid */
	if (word >= EFUSE_LD_WORDS)
		return -EINVAL;

	/* check if there is some bit for programming */
	if (val == (line[word] & val))
		return 0;

	res = enable_efuse_program();
	if (res)
		return res;

	mvebu_read_ld_efuse(ld1, line);
	line[word] |= val;

	for (i = 0; i < EFUSE_LD_WORDS; i++) {
		writel(line[i], ld_efuses + i);
		mdelay(1);
	}

	mdelay(5);

	disable_efuse_program();

	return 0;
}

int mvebu_efuse_init_hw(void)
{
	int ret;

	if (efuse_initialised)
		return 0;

	ret = mvebu_mbus_add_window_by_id(
		CPU_TARGET_SATA23_DFX, 0xA, MBUS_EFUSE_BASE, MBUS_EFUSE_SIZE);

	if (ret)
		return ret;

	efuse_initialised = 1;

	return 0;
}

int mvebu_read_efuse(int nr, struct efuse_val *val)
{
	struct mvebu_hd_efuse *efuse;
	int res;

	res = mvebu_efuse_init_hw();
	if (res)
		return res;

	efuse = get_efuse_line(nr);
	if (!efuse)
		return -ENODEV;

	if (!val)
		return -EINVAL;

	val->dwords.d[0] = readl(&efuse->bits_31_0);
	val->dwords.d[1] = readl(&efuse->bits_63_32);
	val->lock = readl(&efuse->bit64);
	return 0;
}

void mvebu_read_ld_efuse(int ld1, u32 *line)
{
	int i;

#ifndef DRY_RUN
	if (ld1)
		setbits_le32(MVEBU_EFUSE_CONTROL, MVEBU_EFUSE_LD1_SELECT);
	else
		clrbits_le32(MVEBU_EFUSE_CONTROL, MVEBU_EFUSE_LD1_SELECT);
#endif

	for (i = 0; i < EFUSE_LD_WORDS; i++)
		line[i] = readl(ld_efuses + i);
}

int mvebu_write_efuse(int nr, struct efuse_val *val)
{
	return prog_efuse(nr, val, ~0, ~0);
}

int mvebu_lock_efuse(int nr)
{
	struct efuse_val val = {
		.lock = 1,
	};

	return prog_efuse(nr, &val, 0, 0);
}

/*
 * wrapper funcs providing the fuse API
 *
 * we use the following mapping:
 *   "bank" ->	eFuse line
 *   "word" ->	0: bits 0-31
 *		1: bits 32-63
 *		2: bit 64 (lock)
 */

static struct efuse_val prog_val;
static int valid_prog_words;

int fuse_read(u32 bank, u32 word, u32 *val)
{
	struct efuse_val fuse_line;
	u32 ld_line[EFUSE_LD_WORDS];
	int res;

	if ((bank == EFUSE_LD0_LINE || bank == EFUSE_LD1_LINE) && word < EFUSE_LD_WORDS) {
		res = mvebu_efuse_init_hw();
		if (res)
			return res;
		mvebu_read_ld_efuse(bank == EFUSE_LD1_LINE, ld_line);
		*val = ld_line[word];
		return 0;
	}

	if (bank < EFUSE_LINE_MIN || bank > EFUSE_LINE_MAX || word > 2)
		return -EINVAL;

	res = mvebu_read_efuse(bank, &fuse_line);
	if (res)
		return res;

	if (word < 2)
		*val = fuse_line.dwords.d[word];
	else
		*val = fuse_line.lock;

	return res;
}

int fuse_sense(u32 bank, u32 word, u32 *val)
{
	/* not supported */
	return -ENOSYS;
}

int fuse_prog(u32 bank, u32 word, u32 val)
{
	int res = 0;

	if (bank == EFUSE_LD0_LINE || bank == EFUSE_LD1_LINE)
		return mvebu_prog_ld_efuse(bank == EFUSE_LD1_LINE, word, val);

	/*
	 * NOTE: Fuse line should be written as whole.
	 * So how can we do that with this API?
	 * For now: remember values for word == 0 and word == 1 and write the
	 * whole line when word == 2.
	 * This implies that we always require all 3 fuse prog cmds (one for
	 * for each word) to write a single fuse line.
	 * Exception is a single write to word 2 which will lock the fuse line.
	 *
	 * Hope that will be OK.
	 */

	if (bank < EFUSE_LINE_MIN || bank > EFUSE_LINE_MAX || word > 2)
		return -EINVAL;

	if (word < 2) {
		prog_val.dwords.d[word] = val;
		valid_prog_words |= (1 << word);
	} else if ((valid_prog_words & 3) == 0 && val) {
		res = mvebu_lock_efuse(bank);
		valid_prog_words = 0;
	} else if ((valid_prog_words & 3) != 3 || !val) {
		res = -EINVAL;
	} else {
		prog_val.lock = val != 0;
		res = mvebu_write_efuse(bank, &prog_val);
		valid_prog_words = 0;
	}

	return res;
}

int fuse_override(u32 bank, u32 word, u32 val)
{
	/* not supported */
	return -ENOSYS;
}
