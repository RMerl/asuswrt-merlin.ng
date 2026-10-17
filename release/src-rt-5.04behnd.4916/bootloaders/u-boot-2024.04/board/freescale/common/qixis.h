/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2011 Freescale Semiconductor
 * Copyright 2020 NXP
 * Author: Shengzhou Liu <Shengzhou.Liu@freescale.com>
 *
 * This file provides support for the QIXIS of some Freescale reference boards.
 */

#ifndef __QIXIS_H_
#define __QIXIS_H_

struct qixis {
	u8 id;      /* ID value uniquely identifying each QDS board type */
	u8 arch;    /* Board version information */
	u8 scver;   /* QIXIS Version Register */
	u8 model;   /* Information of software programming model version */
	u8 tagdata;
	u8 ctl_sys;
	u8 aux;         /* Auxiliary Register,0x06 */
	u8 clk_spd;
	u8 stat_dut;
	u8 stat_sys;
	u8 stat_alrm;
	u8 present;
	u8 present2;    /* Presence Status Register 2,0x0c */
	u8 rcw_ctl;
	u8 ctl_led;
	u8 i2cblk;
	u8 rcfg_ctl;    /* Reconfig Control Register,0x10 */
	u8 rcfg_st;
	u8 dcm_ad;
	u8 dcm_da;
	u8 dcmd;
	u8 dmsg;
	u8 gdc;
	u8 gdd;         /* DCM Debug Data Register,0x17 */
	u8 dmack;
	u8 res1;
	u8 sdhc1;
	u8 sdhc2;
	u8 stat_pres3;
	u8 los_stat;
	u8 usb_ctl;
	u8 watch;       /* Watchdog Register,0x1F */
	u8 pwr_ctl[2];  /* Power Control Register,0x20 */
	u8 res2[2];
	u8 pwr_stat[4]; /* Power Status Register,0x24 */
	u8 res3[8];
	u8 clk_spd2[2];  /* SYSCLK clock Speed Register,0x30 */
	u8 res4[2];
	u8 sclk[3];  /* Clock Configuration Registers,0x34 */
	u8 res5;
	u8 dclk[3];
	u8 res6;
	u8 clk_dspd[3];
	u8 res7;
	u8 rst_ctl;     /* Reset Control Register,0x40 */
	u8 rst_stat;    /* Reset Status Register */
	u8 rst_rsn;     /* Reset Reason Register */
	u8 rst_frc[2];  /* Reset Force Registers,0x43 */
	u8 res8[11];
	u8 brdcfg[16];  /* Board Configuration Register,0x50 */
	u8 dutcfg[16];
	u8 rcw_ad[2];   /* RCW SRAM Address Registers,0x70 */
	u8 rcw_data;
	u8 res9[5];
	u8 post_ctl;
	u8 post_stat;
	u8 post_dat[2];
	u8 pi_d[4];
	u8 gpio_io[4];
	u8 gpio_dir[4];
	u8 res10[20];
	u8 rjtag_ctl;
	u8 rjtag_dat;
	u8 res11[2];
	u8 trig_src[4];
	u8 trig_dst[4];
	u8 trig_stat;
	u8 res12[3];
	u8 trig_ctr[4];
	u8 res13[16];
	u8 clk_freq[6];	/* Clock Measurement Registers */
	u8 res_c6[8];
	u8 clk_base[2];	/* Clock Frequency Base Reg */
	u8 res_d0[8];
	u8 cms[2];	/* Core Management Space Address Register, 0xD8 */
	u8 res_c0[6];
	u8 aux2[4];	/* Auxiliary Registers,0xE0 */
	u8 res14[10];
	u8 aux_ad;
	u8 aux_da;
	u8 res15[16];
};

u8 qixis_read(unsigned int reg);
void qixis_write(unsigned int reg, u8 value);
u16 qixis_read_minor(void);
char *qixis_read_time(char *result);
char *qixis_read_tag(char *buf);
const char *byte_to_binary_mask(u8 val, u8 mask, char *buf);
#ifdef CFG_SYS_I2C_FPGA_ADDR
u8 qixis_read_i2c(unsigned int reg);
void qixis_write_i2c(unsigned int reg, u8 value);
#endif

#if defined(CONFIG_QIXIS_I2C_ACCESS) && defined(CFG_SYS_I2C_FPGA_ADDR)
#define QIXIS_READ(reg) qixis_read_i2c(offsetof(struct qixis, reg))
#define QIXIS_WRITE(reg, value) \
	qixis_write_i2c(offsetof(struct qixis, reg), value)
#else
#define QIXIS_READ(reg) qixis_read(offsetof(struct qixis, reg))
#define QIXIS_WRITE(reg, value) qixis_write(offsetof(struct qixis, reg), value)
#endif

#ifdef CFG_SYS_I2C_FPGA_ADDR
#define QIXIS_READ_I2C(reg) qixis_read_i2c(offsetof(struct qixis, reg))
#define QIXIS_WRITE_I2C(reg, value) \
			qixis_write_i2c(offsetof(struct qixis, reg), value)
#endif

/* Use for SDHC adapter card type identification and operation */
#define QIXIS_SDID_MASK                         0x07

#define QIXIS_ESDHC_ADAPTER_TYPE_EMMC45         0x1	/* eMMC Card Rev4.5 */
#define QIXIS_ESDHC_ADAPTER_TYPE_SDMMC_LEGACY   0x2	/* SD/MMC Legacy Card */
#define QIXIS_ESDHC_ADAPTER_TYPE_EMMC44         0x3	/* eMMC Card Rev4.4 */
#define QIXIS_ESDHC_ADAPTER_TYPE_RSV            0x4	/* Reserved */
#define QIXIS_ESDHC_ADAPTER_TYPE_MMC            0x5	/* MMC Card */
#define QIXIS_ESDHC_ADAPTER_TYPE_SD             0x6	/* SD Card Rev2.0 3.0 */
#define QIXIS_ESDHC_NO_ADAPTER                  0x7	/* No Card is Present*/

#define QIXIS_SDHC1_S1V3	0x80	/* SDHC1: SDHC1 3.3V power control */
#define QIXIS_SDHC1_VS		0x30	/* BRDCFG11: route to SDHC1_VS */

#define QIXIS_SDCLKIN		0x08
#define QIXIS_SDCLKOUT		0x02
#define QIXIS_DAT5_6_7		0X02
#define QIXIS_DAT4		0X01

#define QIXIS_EVDD_BY_SDHC_VS	0x0c

#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS) || \
defined(CONFIG_TARGET_LX2160ARDB)
#define QIXIS_XMAP_MASK			0x07
#define QIXIS_RST_CTL_RESET_EN		0x30
#define QIXIS_LBMAP_DFLTBANK		0x00
#define QIXIS_LBMAP_ALTBANK		0x20
#define QIXIS_LBMAP_QSPI		0x00
#define QIXIS_RCW_SRC_QSPI		0xff
#define QIXIS_RST_CTL_RESET		0x31
#define QIXIS_RCFG_CTL_RECONFIG_IDLE	0x20
#define QIXIS_RCFG_CTL_RECONFIG_START	0x21
#define QIXIS_RCFG_CTL_WATCHDOG_ENBLE	0x08
#define QIXIS_LBMAP_MASK		0x0f
#define QIXIS_LBMAP_SD
#define QIXIS_LBMAP_EMMC
#define QIXIS_RCW_SRC_SD		0x08
#define QIXIS_RCW_SRC_EMMC         0x09
#define NON_EXTENDED_DUTCFG
#endif

#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
#define QIXIS_SDID_MASK			0x07
#define QIXIS_ESDHC_NO_ADAPTER		0x7
#endif

/*
 * implementation of CONFIG_ESDHC_DETECT_QUIRK Macro.
 */
static inline u8 qixis_esdhc_detect_quirk(void)
{
	/*
	 * SDHC1 Card ID:
	 * Specifies the type of card installed in the SDHC1 adapter slot.
	 * 000= (reserved)
	 * 001= eMMC V4.5 adapter is installed.
	 * 010= SD/MMC 3.3V adapter is installed.
	 * 011= eMMC V4.4 adapter is installed.
	 * 100= eMMC V5.0 adapter is installed.
	 * 101= MMC card/Legacy (3.3V) adapter is installed.
	 * 110= SDCard V2/V3 adapter installed.
	 * 111= no adapter is installed.
	 */
	return ((QIXIS_READ(sdhc1) & QIXIS_SDID_MASK) !=
		 QIXIS_ESDHC_NO_ADAPTER);
}

#endif
