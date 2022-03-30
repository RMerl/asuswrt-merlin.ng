// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009-2013 ADVANSEE
 * Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
 *
 * Based on the mpc512x iim code:
 * Copyright 2008 Silicon Turnkey Express, Inc.
 * Martha Marx <mmarx@silicontkx.com>
 */

#include <common.h>
#include <fuse.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#if defined(CONFIG_MX51) || defined(CONFIG_MX53)
#include <asm/arch/clock.h>
#endif

/* FSL IIM-specific constants */
#define STAT_BUSY		0x80
#define STAT_PRGD		0x02
#define STAT_SNSD		0x01

#define STATM_PRGD_M		0x02
#define STATM_SNSD_M		0x01

#define ERR_PRGE		0x80
#define ERR_WPE			0x40
#define ERR_OPE			0x20
#define ERR_RPE			0x10
#define ERR_WLRE		0x08
#define ERR_SNSE		0x04
#define ERR_PARITYE		0x02

#define EMASK_PRGE_M		0x80
#define EMASK_WPE_M		0x40
#define EMASK_OPE_M		0x20
#define EMASK_RPE_M		0x10
#define EMASK_WLRE_M		0x08
#define EMASK_SNSE_M		0x04
#define EMASK_PARITYE_M		0x02

#define FCTL_DPC		0x80
#define FCTL_PRG_LENGTH_MASK	0x70
#define FCTL_ESNS_N		0x08
#define FCTL_ESNS_0		0x04
#define FCTL_ESNS_1		0x02
#define FCTL_PRG		0x01

#define UA_A_BANK_MASK		0x38
#define UA_A_ROWH_MASK		0x07

#define LA_A_ROWL_MASK		0xf8
#define LA_A_BIT_MASK		0x07

#define PREV_PROD_REV_MASK	0xf8
#define PREV_PROD_VT_MASK	0x07

/* Select the correct accessors depending on endianness */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define iim_read32		in_le32
#define iim_write32		out_le32
#define iim_clrsetbits32	clrsetbits_le32
#define iim_clrbits32		clrbits_le32
#define iim_setbits32		setbits_le32
#elif __BYTE_ORDER == __BIG_ENDIAN
#define iim_read32		in_be32
#define iim_write32		out_be32
#define iim_clrsetbits32	clrsetbits_be32
#define iim_clrbits32		clrbits_be32
#define iim_setbits32		setbits_be32
#else
#error Endianess is not defined: please fix to continue
#endif

/* IIM control registers */
struct fsl_iim {
	u32 stat;
	u32 statm;
	u32 err;
	u32 emask;
	u32 fctl;
	u32 ua;
	u32 la;
	u32 sdat;
	u32 prev;
	u32 srev;
	u32 prg_p;
	u32 scs[0x1f5];
	struct {
		u32 word[0x100];
	} bank[8];
};

#if !defined(CONFIG_MX51) && !defined(CONFIG_MX53)
#define enable_efuse_prog_supply(enable)
#endif

static int prepare_access(struct fsl_iim **regs, u32 bank, u32 word, int assert,
				const char *caller)
{
	*regs = (struct fsl_iim *)IIM_BASE_ADDR;

	if (bank >= ARRAY_SIZE((*regs)->bank) ||
			word >= ARRAY_SIZE((*regs)->bank[0].word) ||
			!assert) {
		printf("fsl_iim %s(): Invalid argument\n", caller);
		return -EINVAL;
	}

	return 0;
}

static void clear_status(struct fsl_iim *regs)
{
	iim_setbits32(&regs->stat, 0);
	iim_setbits32(&regs->err, 0);
}

static void finish_access(struct fsl_iim *regs, u32 *stat, u32 *err)
{
	*stat = iim_read32(&regs->stat);
	*err = iim_read32(&regs->err);
	clear_status(regs);
}

static int prepare_read(struct fsl_iim **regs, u32 bank, u32 word, u32 *val,
			const char *caller)
{
	int ret;

	ret = prepare_access(regs, bank, word, val != NULL, caller);
	if (ret)
		return ret;

	clear_status(*regs);

	return 0;
}

int fuse_read(u32 bank, u32 word, u32 *val)
{
	struct fsl_iim *regs;
	u32 stat, err;
	int ret;

	ret = prepare_read(&regs, bank, word, val, __func__);
	if (ret)
		return ret;

	*val = iim_read32(&regs->bank[bank].word[word]);
	finish_access(regs, &stat, &err);

	if (err & ERR_RPE) {
		puts("fsl_iim fuse_read(): Read protect error\n");
		return -EIO;
	}

	return 0;
}

static void direct_access(struct fsl_iim *regs, u32 bank, u32 word, u32 bit,
				u32 fctl, u32 *stat, u32 *err)
{
	iim_write32(&regs->ua, bank << 3 | word >> 5);
	iim_write32(&regs->la, (word << 3 | bit) & 0xff);
	if (fctl == FCTL_PRG)
		iim_write32(&regs->prg_p, 0xaa);
	iim_setbits32(&regs->fctl, fctl);
	while (iim_read32(&regs->stat) & STAT_BUSY)
		udelay(20);
	finish_access(regs, stat, err);
}

int fuse_sense(u32 bank, u32 word, u32 *val)
{
	struct fsl_iim *regs;
	u32 stat, err;
	int ret;

	ret = prepare_read(&regs, bank, word, val, __func__);
	if (ret)
		return ret;

	direct_access(regs, bank, word, 0, FCTL_ESNS_N, &stat, &err);

	if (err & ERR_SNSE) {
		puts("fsl_iim fuse_sense(): Explicit sense cycle error\n");
		return -EIO;
	}

	if (!(stat & STAT_SNSD)) {
		puts("fsl_iim fuse_sense(): Explicit sense cycle did not complete\n");
		return -EIO;
	}

	*val = iim_read32(&regs->sdat);
	return 0;
}

static int prog_bit(struct fsl_iim *regs, u32 bank, u32 word, u32 bit)
{
	u32 stat, err;

	clear_status(regs);
	direct_access(regs, bank, word, bit, FCTL_PRG, &stat, &err);
	iim_write32(&regs->prg_p, 0x00);

	if (err & ERR_PRGE) {
		puts("fsl_iim fuse_prog(): Program error\n");
		return -EIO;
	}

	if (err & ERR_WPE) {
		puts("fsl_iim fuse_prog(): Write protect error\n");
		return -EIO;
	}

	if (!(stat & STAT_PRGD)) {
		puts("fsl_iim fuse_prog(): Program did not complete\n");
		return -EIO;
	}

	return 0;
}

static int prepare_write(struct fsl_iim **regs, u32 bank, u32 word, u32 val,
				const char *caller)
{
	return prepare_access(regs, bank, word, !(val & ~0xff), caller);
}

int fuse_prog(u32 bank, u32 word, u32 val)
{
	struct fsl_iim *regs;
	u32 bit;
	int ret;

	ret = prepare_write(&regs, bank, word, val, __func__);
	if (ret)
		return ret;

	enable_efuse_prog_supply(1);
	for (bit = 0; val; bit++, val >>= 1)
		if (val & 0x01) {
			ret = prog_bit(regs, bank, word, bit);
			if (ret) {
				enable_efuse_prog_supply(0);
				return ret;
			}
		}
	enable_efuse_prog_supply(0);

	return 0;
}

int fuse_override(u32 bank, u32 word, u32 val)
{
	struct fsl_iim *regs;
	u32 stat, err;
	int ret;

	ret = prepare_write(&regs, bank, word, val, __func__);
	if (ret)
		return ret;

	clear_status(regs);
	iim_write32(&regs->bank[bank].word[word], val);
	finish_access(regs, &stat, &err);

	if (err & ERR_OPE) {
		puts("fsl_iim fuse_override(): Override protect error\n");
		return -EIO;
	}

	return 0;
}
