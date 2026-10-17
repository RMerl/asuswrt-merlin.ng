// SPDX-License-Identifier: GPL-2.0
/*
 * This is a driver for the eMemory EG004K32TQ028XW01 NeoFuse
 * One-Time-Programmable (OTP) memory used within the SiFive FU540.
 * It is documented in the FU540 manual here:
 * https://www.sifive.com/documentation/chips/freedom-u540-c000-manual/
 *
 * Copyright (C) 2018 Philipp Hug <philipp@hug.cx>
 * Copyright (C) 2018 Joey Hewitt <joey@joeyhewitt.com>
 *
 * Copyright (C) 2020 SiFive, Inc
 */

/*
 * The FU540 stores 4096x32 bit (16KiB) values.
 * Index 0x00-0xff are reserved for SiFive internal use. (first 1KiB)
 * Right now first 1KiB is used to store only serial number.
 */

#include <common.h>
#include <dm/device.h>
#include <dm/read.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <misc.h>
#include <linux/printk.h>

#define BYTES_PER_FUSE		4

#define PA_RESET_VAL		0x00
#define PAS_RESET_VAL		0x00
#define PAIO_RESET_VAL		0x00
#define PDIN_RESET_VAL		0x00
#define PTM_RESET_VAL		0x00

#define PCLK_ENABLE_VAL			BIT(0)
#define PCLK_DISABLE_VAL		0x00

#define PWE_WRITE_ENABLE		BIT(0)
#define PWE_WRITE_DISABLE		0x00

#define PTM_FUSE_PROGRAM_VAL		BIT(1)

#define PCE_ENABLE_INPUT		BIT(0)
#define PCE_DISABLE_INPUT		0x00

#define PPROG_ENABLE_INPUT		BIT(0)
#define PPROG_DISABLE_INPUT		0x00

#define PTRIM_ENABLE_INPUT		BIT(0)
#define PTRIM_DISABLE_INPUT		0x00

#define PDSTB_DEEP_STANDBY_ENABLE	BIT(0)
#define PDSTB_DEEP_STANDBY_DISABLE	0x00

/* Tpw - Program Pulse width delay */
#define TPW_DELAY			20

/* Tpwi - Program Pulse interval delay */
#define TPWI_DELAY			5

/* Tasp - Program address setup delay */
#define TASP_DELAY			1

/* Tcd - read data access delay */
#define TCD_DELAY			40

/* Tkl - clok pulse low delay */
#define TKL_DELAY			10

/* Tms - PTM mode setup delay */
#define TMS_DELAY			1

struct sifive_otp_regs {
	u32 pa;     /* Address input */
	u32 paio;   /* Program address input */
	u32 pas;    /* Program redundancy cell selection input */
	u32 pce;    /* OTP Macro enable input */
	u32 pclk;   /* Clock input */
	u32 pdin;   /* Write data input */
	u32 pdout;  /* Read data output */
	u32 pdstb;  /* Deep standby mode enable input (active low) */
	u32 pprog;  /* Program mode enable input */
	u32 ptc;    /* Test column enable input */
	u32 ptm;    /* Test mode enable input */
	u32 ptm_rep;/* Repair function test mode enable input */
	u32 ptr;    /* Test row enable input */
	u32 ptrim;  /* Repair function enable input */
	u32 pwe;    /* Write enable input (defines program cycle) */
};

struct sifive_otp_plat {
	struct sifive_otp_regs __iomem *regs;
	u32 total_fuses;
};

/*
 * offset and size are assumed aligned to the size of the fuses (32-bit).
 */
static int sifive_otp_read(struct udevice *dev, int offset,
			   void *buf, int size)
{
	struct sifive_otp_plat *plat = dev_get_plat(dev);
	struct sifive_otp_regs *regs = (struct sifive_otp_regs *)plat->regs;

	/* Check if offset and size are multiple of BYTES_PER_FUSE */
	if ((size % BYTES_PER_FUSE) || (offset % BYTES_PER_FUSE)) {
		printf("%s: size and offset must be multiple of 4.\n",
		       __func__);
		return -EINVAL;
	}

	int fuseidx = offset / BYTES_PER_FUSE;
	int fusecount = size / BYTES_PER_FUSE;

	/* check bounds */
	if (offset < 0 || size < 0)
		return -EINVAL;
	if (fuseidx >= plat->total_fuses)
		return -EINVAL;
	if ((fuseidx + fusecount) > plat->total_fuses)
		return -EINVAL;

	u32 fusebuf[fusecount];

	/* init OTP */
	writel(PDSTB_DEEP_STANDBY_ENABLE, &regs->pdstb);
	writel(PTRIM_ENABLE_INPUT, &regs->ptrim);
	writel(PCE_ENABLE_INPUT, &regs->pce);

	/* read all requested fuses */
	for (unsigned int i = 0; i < fusecount; i++, fuseidx++) {
		writel(fuseidx, &regs->pa);

		/* cycle clock to read */
		writel(PCLK_ENABLE_VAL, &regs->pclk);
		ndelay(TCD_DELAY * 1000);
		writel(PCLK_DISABLE_VAL, &regs->pclk);
		ndelay(TKL_DELAY * 1000);

		/* read the value */
		fusebuf[i] = readl(&regs->pdout);
	}

	/* shut down */
	writel(PCE_DISABLE_INPUT, &regs->pce);
	writel(PTRIM_DISABLE_INPUT, &regs->ptrim);
	writel(PDSTB_DEEP_STANDBY_DISABLE, &regs->pdstb);

	/* copy out */
	memcpy(buf, fusebuf, size);

	return size;
}

/*
 * Caution:
 * OTP can be written only once, so use carefully.
 *
 * offset and size are assumed aligned to the size of the fuses (32-bit).
 */
static int sifive_otp_write(struct udevice *dev, int offset,
			    const void *buf, int size)
{
	struct sifive_otp_plat *plat = dev_get_plat(dev);
	struct sifive_otp_regs *regs = (struct sifive_otp_regs *)plat->regs;

	/* Check if offset and size are multiple of BYTES_PER_FUSE */
	if ((size % BYTES_PER_FUSE) || (offset % BYTES_PER_FUSE)) {
		printf("%s: size and offset must be multiple of 4.\n",
		       __func__);
		return -EINVAL;
	}

	int fuseidx = offset / BYTES_PER_FUSE;
	int fusecount = size / BYTES_PER_FUSE;
	u32 *write_buf = (u32 *)buf;
	u32 write_data;
	int i, pas, bit;

	/* check bounds */
	if (offset < 0 || size < 0)
		return -EINVAL;
	if (fuseidx >= plat->total_fuses)
		return -EINVAL;
	if ((fuseidx + fusecount) > plat->total_fuses)
		return -EINVAL;

	/* init OTP */
	writel(PDSTB_DEEP_STANDBY_ENABLE, &regs->pdstb);
	writel(PTRIM_ENABLE_INPUT, &regs->ptrim);

	/* reset registers */
	writel(PCLK_DISABLE_VAL, &regs->pclk);
	writel(PA_RESET_VAL, &regs->pa);
	writel(PAS_RESET_VAL, &regs->pas);
	writel(PAIO_RESET_VAL, &regs->paio);
	writel(PDIN_RESET_VAL, &regs->pdin);
	writel(PWE_WRITE_DISABLE, &regs->pwe);
	writel(PTM_FUSE_PROGRAM_VAL, &regs->ptm);
	ndelay(TMS_DELAY * 1000);

	writel(PCE_ENABLE_INPUT, &regs->pce);
	writel(PPROG_ENABLE_INPUT, &regs->pprog);

	/* write all requested fuses */
	for (i = 0; i < fusecount; i++, fuseidx++) {
		writel(fuseidx, &regs->pa);
		write_data = *(write_buf++);

		for (pas = 0; pas < 2; pas++) {
			writel(pas, &regs->pas);

			for (bit = 0; bit < 32; bit++) {
				writel(bit, &regs->paio);
				writel(((write_data >> bit) & 1),
				       &regs->pdin);
				ndelay(TASP_DELAY * 1000);

				writel(PWE_WRITE_ENABLE, &regs->pwe);
				udelay(TPW_DELAY);
				writel(PWE_WRITE_DISABLE, &regs->pwe);
				udelay(TPWI_DELAY);
			}
		}

		writel(PAS_RESET_VAL, &regs->pas);
	}

	/* shut down */
	writel(PWE_WRITE_DISABLE, &regs->pwe);
	writel(PPROG_DISABLE_INPUT, &regs->pprog);
	writel(PCE_DISABLE_INPUT, &regs->pce);
	writel(PTM_RESET_VAL, &regs->ptm);

	writel(PTRIM_DISABLE_INPUT, &regs->ptrim);
	writel(PDSTB_DEEP_STANDBY_DISABLE, &regs->pdstb);

	return size;
}

static int sifive_otp_of_to_plat(struct udevice *dev)
{
	struct sifive_otp_plat *plat = dev_get_plat(dev);
	int ret;

	plat->regs = dev_read_addr_ptr(dev);

	ret = dev_read_u32(dev, "fuse-count", &plat->total_fuses);
	if (ret < 0) {
		pr_err("\"fuse-count\" not found\n");
		return ret;
	}

	return 0;
}

static const struct misc_ops sifive_otp_ops = {
	.read = sifive_otp_read,
	.write = sifive_otp_write,
};

static const struct udevice_id sifive_otp_ids[] = {
	{ .compatible = "sifive,fu540-c000-otp" },
	{}
};

U_BOOT_DRIVER(sifive_otp) = {
	.name = "sifive_otp",
	.id = UCLASS_MISC,
	.of_match = sifive_otp_ids,
	.of_to_plat = sifive_otp_of_to_plat,
	.plat_auto	= sizeof(struct sifive_otp_plat),
	.ops = &sifive_otp_ops,
};
