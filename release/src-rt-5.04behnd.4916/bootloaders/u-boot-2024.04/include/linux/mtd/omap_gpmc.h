/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004-2008 Texas Instruments, <www.ti.com>
 * Rohit Choraria <rohitkc@ti.com>
 *
 * (C) Copyright 2013 Andreas Bießmann <andreas@biessmann.org>
 */
#ifndef __ASM_OMAP_GPMC_H
#define __ASM_OMAP_GPMC_H

/* Maximum Number of Chip Selects */
#define GPMC_CS_NUM	8

#define GPMC_BUF_EMPTY	0
#define GPMC_BUF_FULL	1
#define GPMC_MAX_SECTORS	8

enum omap_ecc {
	/* 1-bit  ECC calculation by Software, Error detection by Software */
	OMAP_ECC_HAM1_CODE_SW = 1, /* avoid un-initialized int can be 0x0 */
	/* 1-bit  ECC calculation by GPMC, Error detection by Software */
	/* ECC layout compatible to legacy ROMCODE. */
	OMAP_ECC_HAM1_CODE_HW,
	/* 4-bit  ECC calculation by GPMC, Error detection by Software */
	OMAP_ECC_BCH4_CODE_HW_DETECTION_SW,
	/* 4-bit  ECC calculation by GPMC, Error detection by ELM */
	OMAP_ECC_BCH4_CODE_HW,
	/* 8-bit  ECC calculation by GPMC, Error detection by Software */
	OMAP_ECC_BCH8_CODE_HW_DETECTION_SW,
	/* 8-bit  ECC calculation by GPMC, Error detection by ELM */
	OMAP_ECC_BCH8_CODE_HW,
	/* 16-bit  ECC calculation by GPMC, Error detection by ELM */
	OMAP_ECC_BCH16_CODE_HW,
};

struct gpmc_cs {
	u32 config1;		/* 0x00 */
	u32 config2;		/* 0x04 */
	u32 config3;		/* 0x08 */
	u32 config4;		/* 0x0C */
	u32 config5;		/* 0x10 */
	u32 config6;		/* 0x14 */
	u32 config7;		/* 0x18 */
	u32 nand_cmd;		/* 0x1C */
	u32 nand_adr;		/* 0x20 */
	u32 nand_dat;		/* 0x24 */
	u8 res[8];		/* blow up to 0x30 byte */
};

struct bch_res_0_3 {
	u32 bch_result_x[4];
};

struct bch_res_4_6 {
	u32 bch_result_x[3];
};

struct gpmc {
	u8 res1[0x10];
	u32 sysconfig;		/* 0x10 */
	u8 res2[0x4];
	u32 irqstatus;		/* 0x18 */
	u32 irqenable;		/* 0x1C */
	u8 res3[0x20];
	u32 timeout_control;	/* 0x40 */
	u8 res4[0xC];
	u32 config;		/* 0x50 */
	u32 status;		/* 0x54 */
	u8 res5[0x8];		/* 0x58 */
	struct gpmc_cs cs[8];	/* 0x60, 0x90, .. */
	u32 prefetch_config1;	/* 0x1E0 */
	u32 prefetch_config2;	/* 0x1E4 */
	u32 res6;		/* 0x1E8 */
	u32 prefetch_control;	/* 0x1EC */
	u32 prefetch_status;	/* 0x1F0 */
	u32 ecc_config;		/* 0x1F4 */
	u32 ecc_control;	/* 0x1F8 */
	u32 ecc_size_config;	/* 0x1FC */
	u32 ecc1_result;	/* 0x200 */
	u32 ecc2_result;	/* 0x204 */
	u32 ecc3_result;	/* 0x208 */
	u32 ecc4_result;	/* 0x20C */
	u32 ecc5_result;	/* 0x210 */
	u32 ecc6_result;	/* 0x214 */
	u32 ecc7_result;	/* 0x218 */
	u32 ecc8_result;	/* 0x21C */
	u32 ecc9_result;	/* 0x220 */
	u8 res7[12];		/* 0x224 */
	u32 testmomde_ctrl;	/* 0x230 */
	u8 res8[12];		/* 0x234 */
	struct bch_res_0_3 bch_result_0_3[GPMC_MAX_SECTORS]; /* 0x240,0x250, */
	u8 res9[16 * 4];	/* 0x2C0 - 0x2FF */
	struct bch_res_4_6 bch_result_4_6[GPMC_MAX_SECTORS]; /* 0x300,0x310, */
};

/* Used for board specific gpmc initialization */
extern const struct gpmc *gpmc_cfg;
extern char gpmc_cs0_flash;

#endif /* __ASM_OMAP_GPMC_H */
