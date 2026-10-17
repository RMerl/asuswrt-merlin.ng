// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2019 Variscite Ltd.
 * Copyright (C) 2019 Parthiban Nallathambi <parthitce@gmail.com>
 * Copyright (C) 2021 Marc Ferland, Amotus Solutions Inc., <ferlandm@amotus.ca>
 */

#include <init.h>
#include <net.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <dm.h>
#include <fsl_esdhc_imx.h>
#include <i2c_eeprom.h>
#include <linux/bitops.h>
#include <malloc.h>
#include <miiphy.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

#ifdef CONFIG_NAND_MXS
#define GPMI_PAD_CTRL0 (PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_100K_UP)
#define GPMI_PAD_CTRL1 (PAD_CTL_DSE_40ohm | PAD_CTL_SPEED_MED | \
			PAD_CTL_SRE_FAST)
#define GPMI_PAD_CTRL2 (GPMI_PAD_CTRL0 | GPMI_PAD_CTRL1)
static iomux_v3_cfg_t const nand_pads[] = {
	MX6_PAD_NAND_DATA00__RAWNAND_DATA00 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA01__RAWNAND_DATA01 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA02__RAWNAND_DATA02 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA03__RAWNAND_DATA03 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA04__RAWNAND_DATA04 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA05__RAWNAND_DATA05 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA06__RAWNAND_DATA06 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA07__RAWNAND_DATA07 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_CLE__RAWNAND_CLE | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_ALE__RAWNAND_ALE | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_CE0_B__RAWNAND_CE0_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_RE_B__RAWNAND_RE_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_WE_B__RAWNAND_WE_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_WP_B__RAWNAND_WP_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_READY_B__RAWNAND_READY_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DQS__RAWNAND_DQS | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
};

static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* config gpmi nand iomux */
	imx_iomux_v3_setup_multiple_pads(nand_pads, ARRAY_SIZE(nand_pads));

	clrbits_le32(&mxc_ccm->CCGR4,
		     MXC_CCM_CCGR4_RAWNAND_U_BCH_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_BCH_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_GPMI_IO_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_PL301_MX6QPER1_BCH_MASK);

	/*
	 * config gpmi and bch clock to 100 MHz
	 * bch/gpmi select PLL2 PFD2 400M
	 * 100M = 400M / 4
	 */
	clrbits_le32(&mxc_ccm->cscmr1,
		     MXC_CCM_CSCMR1_BCH_CLK_SEL |
		     MXC_CCM_CSCMR1_GPMI_CLK_SEL);
	clrsetbits_le32(&mxc_ccm->cscdr1,
			MXC_CCM_CSCDR1_BCH_PODF_MASK |
			MXC_CCM_CSCDR1_GPMI_PODF_MASK,
			(3 << MXC_CCM_CSCDR1_BCH_PODF_OFFSET) |
			(3 << MXC_CCM_CSCDR1_GPMI_PODF_OFFSET));

	/* enable gpmi and bch clock gating */
	setbits_le32(&mxc_ccm->CCGR4,
		     MXC_CCM_CCGR4_RAWNAND_U_BCH_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_BCH_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_GPMI_IO_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_PL301_MX6QPER1_BCH_MASK);

	/* enable apbh clock gating */
	setbits_le32(&mxc_ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
}
#endif

#ifdef CONFIG_FEC_MXC
static int setup_fec(int fec_id)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int ret;

	if (fec_id == 0) {
		/*
		 * Use 50M anatop loopback REF_CLK1 for ENET1,
		 * clear gpr1[13], set gpr1[17].
		 */
		clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC1_MASK,
				IOMUX_GPR1_FEC1_CLOCK_MUX1_SEL_MASK);
	} else {
		/*
		 * Use 50M anatop loopback REF_CLK2 for ENET2,
		 * clear gpr1[14], set gpr1[18].
		 */
		clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC2_MASK,
				IOMUX_GPR1_FEC2_CLOCK_MUX1_SEL_MASK);
	}

	ret = enable_fec_anatop_clock(fec_id, ENET_50MHZ);
	if (ret)
		return ret;

	enable_enet_clk(1);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	/*
	 * Defaults + Enable status LEDs (LED1: Activity, LED0: Link) & select
	 * 50 MHz RMII clock mode.
	 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1f, 0x8190);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
#endif /* CONFIG_FEC_MXC */

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_FEC_MXC
	setup_fec(CFG_FEC_ENET_DEV);
#endif

#ifdef CONFIG_NAND_MXS
	setup_gpmi_nand();
#endif
	return 0;
}

/* length of strings stored in the eeprom */
#define DART6UL_PN_LEN   16
#define DART6UL_ASSY_LEN 16
#define DART6UL_DATE_LEN 12

/* eeprom content, 512 bytes */
struct dart6ul_info {
	u32 magic;
	u8 partnumber[DART6UL_PN_LEN];
	u8 assy[DART6UL_ASSY_LEN];
	u8 date[DART6UL_DATE_LEN];
	u32 custom_addr_val[32];
	struct cmd {
		u8 addr;
		u8 index;
	} custom_cmd[150];
	u8 res[33];
	u8 som_info;
	u8 ddr_size;
	u8 crc;
} __attribute__ ((__packed__));

#define DART6UL_INFO_STORAGE_GET(n) ((n) & 0x3)
#define DART6UL_INFO_WIFI_GET(n)    ((n) >> 2 & 0x1)
#define DART6UL_INFO_REV_GET(n)     ((n) >> 3 & 0x3)
#define DART6UL_DDRSIZE(n)          ((n) * SZ_128M)
#define DART6UL_INFO_MAGIC          0x32524156

static const char *som_info_storage_to_str(u8 som_info)
{
	switch (DART6UL_INFO_STORAGE_GET(som_info)) {
	case 0x0: return "none (SD only)";
	case 0x1: return "NAND";
	case 0x2: return "eMMC";
	default: return "unknown";
	}
}

static const char *som_info_rev_to_str(u8 som_info)
{
	switch (DART6UL_INFO_REV_GET(som_info)) {
	case 0x0: return "2.4G";
	case 0x1: return "5G";
	default: return "unknown";
	}
}

int checkboard(void)
{
	const char *path = "eeprom0";
	struct dart6ul_info *info;
	struct udevice *dev;
	int ret, off;

	off = fdt_path_offset(gd->fdt_blob, path);
	if (off < 0) {
		printf("%s: fdt_path_offset() failed: %d\n", __func__, off);
		return off;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_I2C_EEPROM, off, &dev);
	if (ret) {
		printf("%s: uclass_get_device_by_of_offset() failed: %d\n", __func__, ret);
		return ret;
	}

	info = malloc(sizeof(struct dart6ul_info));
	if (!info)
		return -ENOMEM;

	ret = i2c_eeprom_read(dev, 0, (uint8_t *)info,
			      sizeof(struct dart6ul_info));
	if (ret) {
		printf("%s: i2c_eeprom_read() failed: %d\n", __func__, ret);
		free(info);
		return ret;
	}

	if (info->magic != DART6UL_INFO_MAGIC) {
		printf("Board: Invalid board info magic: 0x%08x, expected 0x%08x\n",
		       info->magic, DART6UL_INFO_MAGIC);
		/* do not fail if the content is invalid */
		free(info);
		return 0;
	}

	/* make sure strings are null terminated */
	info->partnumber[DART6UL_PN_LEN - 1] = '\0';
	info->assy[DART6UL_ASSY_LEN - 1] = '\0';
	info->date[DART6UL_DATE_LEN - 1] = '\0';

	printf("Board: PN: %s, Assy: %s, Date: %s\n"
	       "       Storage: %s, Wifi: %s, DDR: %d MiB, Rev: %s\n",
	       info->partnumber,
	       info->assy,
	       info->date,
	       som_info_storage_to_str(info->som_info),
	       DART6UL_INFO_WIFI_GET(info->som_info) ? "yes" : "no",
	       DART6UL_DDRSIZE(info->ddr_size) / SZ_1M,
	       som_info_rev_to_str(info->som_info));

	free(info);

	return 0;
}
