// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Josua Mayer <josua@solid-run.com>
 *
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * Copyright (C) 2013 Jon Nettleton <jon.nettleton@gmail.com>
 *
 * Based on SPL code from Solidrun tree, which is:
 * Author: Tungyi Lin <tungyilin1127@gmail.com>
 *
 * Derived from EDM_CF_IMX6 code by TechNexion,Inc
 * Ported to SolidRun microSOM by Rabeeh Khoury <rabeeh@solid-run.com>
 */

#include <common.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/mxc_hdmi.h>
#include <env.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/sata.h>
#include <asm/mach-imx/video.h>
#include <asm/sections.h>
#include <mmc.h>
#include <fsl_esdhc_imx.h>
#include <malloc.h>
#include <asm/arch/crm_regs.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <spl.h>
#include <usb.h>
#include <usb/ehci-ci.h>
#include <netdev.h>
#include <phy.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USB_H1_VBUS	IMX_GPIO_NR(1, 0)

enum board_type {
	CUBOXI          = 0x00,
	HUMMINGBOARD    = 0x01,
	HUMMINGBOARD2   = 0x02,
	UNKNOWN         = 0x03,
};

static struct gpio_desc board_detect_desc[5];

#define MEM_STRIDE 0x4000000
static u32 get_ram_size_stride_test(u32 *base, u32 maxsize)
{
        volatile u32 *addr;
        u32          save[64];
        u32          cnt;
        u32          size;
        int          i = 0;

        /* First save the data */
        for (cnt = 0; cnt < maxsize; cnt += MEM_STRIDE) {
                addr = (volatile u32 *)((u32)base + cnt);       /* pointer arith! */
                sync ();
                save[i++] = *addr;
                sync ();
        }

        /* First write a signature */
        * (volatile u32 *)base = 0x12345678;
        for (size = MEM_STRIDE; size < maxsize; size += MEM_STRIDE) {
                * (volatile u32 *)((u32)base + size) = size;
                sync ();
                if (* (volatile u32 *)((u32)base) == size) {	/* We reached the overlapping address */
                        break;
                }
        }

        /* Restore the data */
        for (cnt = (maxsize - MEM_STRIDE); i > 0; cnt -= MEM_STRIDE) {
                addr = (volatile u32 *)((u32)base + cnt);       /* pointer arith! */
                sync ();
                *addr = save[i--];
                sync ();
        }

        return (size);
}

int dram_init(void)
{
	u32 max_size = imx_ddr_size();

	gd->ram_size = get_ram_size_stride_test((u32 *) CFG_SYS_SDRAM_BASE,
						(u32)max_size);

	return 0;
}

static iomux_v3_cfg_t const uart1_pads[] = {
	IOMUX_PADS(PAD_CSI0_DAT10__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT11__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static iomux_v3_cfg_t const usdhc2_pads[] = {
	IOMUX_PADS(PAD_SD2_CLK__SD2_CLK	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_CMD__SD2_CMD	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT0__SD2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT1__SD2_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT2__SD2_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT3__SD2_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};

static iomux_v3_cfg_t const usdhc3_pads[] = {
	IOMUX_PADS(PAD_SD3_CLK__SD3_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CMD__SD3_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT1__SD3_DATA1      | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT2__SD3_DATA2      | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT3__SD3_DATA3      | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT4__SD3_DATA4      | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT5__SD3_DATA5      | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT6__SD3_DATA6      | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT7__SD3_DATA7      | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_RST__SD3_RESET       | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};

static iomux_v3_cfg_t const board_detect[] = {
	/* These pins are for sensing if it is a CuBox-i or a HummingBoard */
	IOMUX_PADS(PAD_KEY_ROW1__GPIO4_IO09  | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA4__GPIO3_IO04   | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT0__GPIO2_IO08  | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static iomux_v3_cfg_t const som_rev_detect[] = {
	/* These pins are for sensing if it is a CuBox-i or a HummingBoard */
	IOMUX_PADS(PAD_CSI0_DAT14__GPIO6_IO00  | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT18__GPIO6_IO04  | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart1_pads);
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

#ifdef CONFIG_VIDEO_IPUV3
static void do_enable_hdmi(struct display_info_t const *dev)
{
	imx_enable_hdmi_phy();
}

struct display_info_t const displays[] = {
	{
		.bus	= -1,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= detect_hdmi,
		.enable	= do_enable_hdmi,
		.mode	= {
			.name           = "HDMI",
			/* 1024x768@60Hz (VESA)*/
			.refresh        = 60,
			.xres           = 1024,
			.yres           = 768,
			.pixclock       = 15384,
			.left_margin    = 160,
			.right_margin   = 24,
			.upper_margin   = 29,
			.lower_margin   = 3,
			.hsync_len      = 136,
			.vsync_len      = 6,
			.sync           = FB_SYNC_EXT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	}
};

size_t display_count = ARRAY_SIZE(displays);

static int setup_display(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int reg;
	int timeout = 100000;

	enable_ipu_clock();
	imx_setup_hdmi();

	/* set video pll to 455MHz (24MHz * (37+11/12) / 2) */
	setbits_le32(&ccm->analog_pll_video, BM_ANADIG_PLL_VIDEO_POWERDOWN);

	reg = readl(&ccm->analog_pll_video);
	reg &= ~BM_ANADIG_PLL_VIDEO_DIV_SELECT;
	reg |= BF_ANADIG_PLL_VIDEO_DIV_SELECT(37);
	reg &= ~BM_ANADIG_PLL_VIDEO_POST_DIV_SELECT;
	reg |= BF_ANADIG_PLL_VIDEO_POST_DIV_SELECT(1);
	writel(reg, &ccm->analog_pll_video);

	writel(BF_ANADIG_PLL_VIDEO_NUM_A(11), &ccm->analog_pll_video_num);
	writel(BF_ANADIG_PLL_VIDEO_DENOM_B(12), &ccm->analog_pll_video_denom);

	reg &= ~BM_ANADIG_PLL_VIDEO_POWERDOWN;
	writel(reg, &ccm->analog_pll_video);

	while (timeout--)
		if (readl(&ccm->analog_pll_video) & BM_ANADIG_PLL_VIDEO_LOCK)
			break;
	if (timeout < 0) {
		printf("Warning: video pll lock timeout!\n");
		return -ETIMEDOUT;
	}

	reg = readl(&ccm->analog_pll_video);
	reg |= BM_ANADIG_PLL_VIDEO_ENABLE;
	reg &= ~BM_ANADIG_PLL_VIDEO_BYPASS;
	writel(reg, &ccm->analog_pll_video);

	/* gate ipu1_di0_clk */
	clrbits_le32(&ccm->CCGR3, MXC_CCM_CCGR3_LDB_DI0_MASK);

	/* select video_pll clock / 7  for ipu1_di0_clk -> 65MHz pixclock */
	reg = readl(&ccm->chsccdr);
	reg &= ~(MXC_CCM_CHSCCDR_IPU1_DI0_PRE_CLK_SEL_MASK |
		 MXC_CCM_CHSCCDR_IPU1_DI0_PODF_MASK |
		 MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_MASK);
	reg |= (2 << MXC_CCM_CHSCCDR_IPU1_DI0_PRE_CLK_SEL_OFFSET) |
	       (6 << MXC_CCM_CHSCCDR_IPU1_DI0_PODF_OFFSET) |
	       (0 << MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	writel(reg, &ccm->chsccdr);

	/* enable ipu1_di0_clk */
	setbits_le32(&ccm->CCGR3, MXC_CCM_CCGR3_LDB_DI0_MASK);

	return 0;
}
#endif /* CONFIG_VIDEO_IPUV3 */

static int setup_fec(void)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int ret;

	ret = enable_fec_anatop_clock(0, ENET_25MHZ);
	if (ret)
		return ret;

	/* set gpr1[ENET_CLK_SEL] */
	setbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);

	return 0;
}

int board_early_init_f(void)
{
	setup_iomux_uart();

	if (CONFIG_IS_ENABLED(SATA))
		setup_sata();
	setup_fec();

	return 0;
}

int board_init(void)
{
	int ret = 0;

	/* address of boot parameters */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_VIDEO_IPUV3
	ret = setup_display();
#endif

	return ret;
}

static int request_detect_gpios(void)
{
	int node;
	int ret;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0,
		"solidrun,hummingboard-detect");
	if (node < 0)
		return -ENODEV;

	ret = gpio_request_list_by_name_nodev(offset_to_ofnode(node),
		"detect-gpios", board_detect_desc,
		ARRAY_SIZE(board_detect_desc), GPIOD_IS_IN);

	return ret;
}

static int free_detect_gpios(void)
{
	return gpio_free_list_nodev(board_detect_desc,
		ARRAY_SIZE(board_detect_desc));
}

static enum board_type board_type(void)
{
	int val1, val2, val3;

	SETUP_IOMUX_PADS(board_detect);

	/*
	 * Machine selection -
	 * Machine      val1, val2, val3
	 * ----------------------------
	 * HB2            x     x    0
	 * HB rev 3.x     x     0    x
	 * CBi            0     1    x
	 * HB             1     1    x
	 */

	val3 = !!dm_gpio_get_value(&board_detect_desc[0]);

	if (val3 == 0)
		return HUMMINGBOARD2;

	val2 = !!dm_gpio_get_value(&board_detect_desc[1]);

	if (val2 == 0)
		return HUMMINGBOARD;

	val1 = !!dm_gpio_get_value(&board_detect_desc[2]);

	if (val1 == 0) {
		return CUBOXI;
	} else {
		return HUMMINGBOARD;
	}
}

static bool is_rev_15_som(void)
{
	int val1, val2;
	SETUP_IOMUX_PADS(som_rev_detect);

	val1 = !!dm_gpio_get_value(&board_detect_desc[3]);
	val2 = !!dm_gpio_get_value(&board_detect_desc[4]);

	if (val1 == 1 && val2 == 0)
		return true;

	return false;
}

static bool has_emmc(void)
{
	struct mmc *mmc;
	mmc = find_mmc_device(2);
	if (!mmc)
		return 0;
	return (mmc_get_op_cond(mmc, true) < 0) ? 0 : 1;
}

/* Override the default implementation, DT model is not accurate */
int checkboard(void)
{
	request_detect_gpios();

	switch (board_type()) {
	case CUBOXI:
		puts("Board: MX6 Cubox-i");
		break;
	case HUMMINGBOARD:
		puts("Board: MX6 HummingBoard");
		break;
	case HUMMINGBOARD2:
		puts("Board: MX6 HummingBoard2");
		break;
	case UNKNOWN:
	default:
		puts("Board: Unknown\n");
		goto out;
	}

	if (is_rev_15_som())
		puts(" (som rev 1.5)\n");
	else
		puts("\n");

	free_detect_gpios();
out:
	return 0;
}

static int find_ethernet_phy(void)
{
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;
	int phy_addr = -ENOENT;

#ifdef CONFIG_FEC_MXC
	bus = fec_get_miibus(ENET_BASE_ADDR, -1);
	if (!bus)
		return -ENOENT;

	// scan address 0, 1, 4
	phydev = phy_find_by_mask(bus, 0b00010011);
	if (!phydev) {
		free(bus);
		return -ENOENT;
	}
	pr_debug("%s: detected ethernet phy at address %d\n", __func__, phydev->addr);
	phy_addr = phydev->addr;

	free(phydev);
#endif

	return phy_addr;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
/*
 * Configure the correct ethernet PHYs nodes in device-tree:
 * - AR8035 at addresses 0 or 4: Cubox
 * - AR8035 at address 0: HummingBoard, HummingBoard 2
 * - ADIN1300 at address 1: since SoM rev 1.9
 */
int ft_board_setup(void *fdt, struct bd_info *bd)
{
	int node_phy0, node_phy1, node_phy4;
	int ret, phy;
	bool enable_phy0 = false, enable_phy1 = false, enable_phy4 = false;
	enum board_type board;

	// detect device
	request_detect_gpios();
	board = board_type();
	free_detect_gpios();

	// detect phy
	phy = find_ethernet_phy();
	if (phy == 0 || phy == 4) {
		enable_phy0 = true;
		switch (board) {
		case HUMMINGBOARD:
		case HUMMINGBOARD2:
			/* atheros phy may appear only at address 0 */
			break;
		case CUBOXI:
		case UNKNOWN:
		default:
			/* atheros phy may appear at either address 0 or 4 */
			enable_phy4 = true;
		}
	} else if (phy == 1) {
		enable_phy1 = true;
	} else {
		pr_err("%s: couldn't detect ethernet phy, not patching dtb!\n", __func__);
		return 0;
	}

	// update all phy nodes status
	node_phy0 = fdt_path_offset(fdt, "/soc/bus@2100000/ethernet@2188000/mdio/ethernet-phy@0");
	ret = fdt_setprop_string(fdt, node_phy0, "status", enable_phy0 ? "okay" : "disabled");
	if (ret < 0 && enable_phy0)
		pr_err("%s: failed to enable ethernet phy at address 0 in dtb!\n", __func__);
	node_phy1 = fdt_path_offset(fdt, "/soc/bus@2100000/ethernet@2188000/mdio/ethernet-phy@1");
	ret = fdt_setprop_string(fdt, node_phy1, "status", enable_phy1 ? "okay" : "disabled");
	if (ret < 0 && enable_phy1)
		pr_err("%s: failed to enable ethernet phy at address 1 in dtb!\n", __func__);
	node_phy4 = fdt_path_offset(fdt, "/soc/bus@2100000/ethernet@2188000/mdio/ethernet-phy@4");
	ret = fdt_setprop_string(fdt, node_phy4, "status", enable_phy4 ? "okay" : "disabled");
	if (ret < 0 && enable_phy4)
		pr_err("%s: failed to enable ethernet phy at address 4 in dtb!\n", __func__);

	return 0;
}
#endif

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	request_detect_gpios();

	switch (board_type()) {
	case CUBOXI:
		env_set("board_name", "CUBOXI");
		break;
	case HUMMINGBOARD:
		env_set("board_name", "HUMMINGBOARD");
		break;
	case HUMMINGBOARD2:
		env_set("board_name", "HUMMINGBOARD2");
		break;
	case UNKNOWN:
	default:
		env_set("board_name", "CUBOXI");
	}

	if (is_mx6dq())
		env_set("board_rev", "MX6Q");
	else
		env_set("board_rev", "MX6DL");

	if (is_rev_15_som())
		env_set("som_rev", "V15");

	if (has_emmc())
		env_set("has_emmc", "yes");

	free_detect_gpios();
#endif

	return 0;
}

/*
 * This is not a perfect match. Avoid dependency on the DM GPIO driver needed
 * for accurate board detection. Hummingboard2 DT is good enough for U-Boot on
 * all Hummingboard/Cubox-i platforms.
 */
int board_fit_config_name_match(const char *name)
{
	char tmp_name[36];

	snprintf(tmp_name, sizeof(tmp_name), "%s-hummingboard2-emmc-som-v15",
			is_mx6dq() ? "imx6q" : "imx6dl");

	return strcmp(name, tmp_name);
}

void board_boot_order(u32 *spl_boot_list)
{
	struct src *psrc = (struct src *)SRC_BASE_ADDR;
	unsigned int reg = readl(&psrc->sbmr1) >> 11;
	u32 boot_mode = imx6_src_get_boot_mode() & IMX6_BMODE_MASK;
	unsigned int bmode = readl(&src_base->sbmr2);

	/* If bmode is serial or USB phy is active, return serial */
	if (((bmode >> 24) & 0x03) == 0x01 || is_usbotg_phy_active()) {
		spl_boot_list[0] = BOOT_DEVICE_BOARD;
		return;
	}

	switch (boot_mode >> IMX6_BMODE_SHIFT) {
	case IMX6_BMODE_SD:
	case IMX6_BMODE_ESD:
	case IMX6_BMODE_MMC:
	case IMX6_BMODE_EMMC:
		/*
		 * Upon reading BOOT_CFG register the following map is done:
		 * Bit 11 and 12 of BOOT_CFG register can determine the current
		 * mmc port
		 * 0x1                  SD2
		 * 0x2                  SD3
		 */

		reg &= 0x3; /* Only care about bottom 2 bits */
		switch (reg) {
		case 1:
			SETUP_IOMUX_PADS(usdhc2_pads);
			spl_boot_list[0] = BOOT_DEVICE_MMC1;
			break;
		case 2:
			SETUP_IOMUX_PADS(usdhc3_pads);
			spl_boot_list[0] = BOOT_DEVICE_MMC2;
			break;
		}
		break;
	default:
		/* By default use USB downloader */
		spl_boot_list[0] = BOOT_DEVICE_BOARD;
		break;
	}

	/* As a last resort, use serial downloader */
	spl_boot_list[1] = BOOT_DEVICE_BOARD;
}

#ifdef CONFIG_SPL_BUILD
#include <asm/arch/mx6-ddr.h>
static const struct mx6dq_iomux_ddr_regs mx6q_ddr_ioregs = {
	.dram_sdclk_0 =  0x00020030,
	.dram_sdclk_1 =  0x00020030,
	.dram_cas =  0x00020030,
	.dram_ras =  0x00020030,
	.dram_reset =  0x000c0030,
	.dram_sdcke0 =  0x00003000,
	.dram_sdcke1 =  0x00003000,
	.dram_sdba2 =  0x00000000,
	.dram_sdodt0 =  0x00003030,
	.dram_sdodt1 =  0x00003030,
	.dram_sdqs0 =  0x00000030,
	.dram_sdqs1 =  0x00000030,
	.dram_sdqs2 =  0x00000030,
	.dram_sdqs3 =  0x00000030,
	.dram_sdqs4 =  0x00000030,
	.dram_sdqs5 =  0x00000030,
	.dram_sdqs6 =  0x00000030,
	.dram_sdqs7 =  0x00000030,
	.dram_dqm0 =  0x00020030,
	.dram_dqm1 =  0x00020030,
	.dram_dqm2 =  0x00020030,
	.dram_dqm3 =  0x00020030,
	.dram_dqm4 =  0x00020030,
	.dram_dqm5 =  0x00020030,
	.dram_dqm6 =  0x00020030,
	.dram_dqm7 =  0x00020030,
};

static const struct mx6sdl_iomux_ddr_regs mx6dl_ddr_ioregs = {
	.dram_sdclk_0 = 0x00000028,
	.dram_sdclk_1 = 0x00000028,
	.dram_cas =	0x00000028,
	.dram_ras =	0x00000028,
	.dram_reset =	0x000c0028,
	.dram_sdcke0 =	0x00003000,
	.dram_sdcke1 =	0x00003000,
	.dram_sdba2 =	0x00000000,
	.dram_sdodt0 =	0x00003030,
	.dram_sdodt1 =	0x00003030,
	.dram_sdqs0 =	0x00000028,
	.dram_sdqs1 =	0x00000028,
	.dram_sdqs2 =	0x00000028,
	.dram_sdqs3 =	0x00000028,
	.dram_sdqs4 =	0x00000028,
	.dram_sdqs5 =	0x00000028,
	.dram_sdqs6 =	0x00000028,
	.dram_sdqs7 =	0x00000028,
	.dram_dqm0 =	0x00000028,
	.dram_dqm1 =	0x00000028,
	.dram_dqm2 =	0x00000028,
	.dram_dqm3 =	0x00000028,
	.dram_dqm4 =	0x00000028,
	.dram_dqm5 =	0x00000028,
	.dram_dqm6 =	0x00000028,
	.dram_dqm7 =	0x00000028,
};

static const struct mx6dq_iomux_grp_regs mx6q_grp_ioregs = {
	.grp_ddr_type =  0x000C0000,
	.grp_ddrmode_ctl =  0x00020000,
	.grp_ddrpke =  0x00000000,
	.grp_addds =  0x00000030,
	.grp_ctlds =  0x00000030,
	.grp_ddrmode =  0x00020000,
	.grp_b0ds =  0x00000030,
	.grp_b1ds =  0x00000030,
	.grp_b2ds =  0x00000030,
	.grp_b3ds =  0x00000030,
	.grp_b4ds =  0x00000030,
	.grp_b5ds =  0x00000030,
	.grp_b6ds =  0x00000030,
	.grp_b7ds =  0x00000030,
};

static const struct mx6sdl_iomux_grp_regs mx6sdl_grp_ioregs = {
	.grp_ddr_type = 0x000c0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke = 0x00000000,
	.grp_addds = 0x00000028,
	.grp_ctlds = 0x00000028,
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = 0x00000028,
	.grp_b1ds = 0x00000028,
	.grp_b2ds = 0x00000028,
	.grp_b3ds = 0x00000028,
	.grp_b4ds = 0x00000028,
	.grp_b5ds = 0x00000028,
	.grp_b6ds = 0x00000028,
	.grp_b7ds = 0x00000028,
};

/* microSOM with Dual processor and 1GB memory */
static const struct mx6_mmdc_calibration mx6q_1g_mmcd_calib = {
	.p0_mpwldectrl0 =  0x00000000,
	.p0_mpwldectrl1 =  0x00000000,
	.p1_mpwldectrl0 =  0x00000000,
	.p1_mpwldectrl1 =  0x00000000,
	.p0_mpdgctrl0 =    0x0314031c,
	.p0_mpdgctrl1 =    0x023e0304,
	.p1_mpdgctrl0 =    0x03240330,
	.p1_mpdgctrl1 =    0x03180260,
	.p0_mprddlctl =    0x3630323c,
	.p1_mprddlctl =    0x3436283a,
	.p0_mpwrdlctl =    0x36344038,
	.p1_mpwrdlctl =    0x422a423c,
};

/* microSOM with Quad processor and 2GB memory */
static const struct mx6_mmdc_calibration mx6q_2g_mmcd_calib = {
	.p0_mpwldectrl0 =  0x00000000,
	.p0_mpwldectrl1 =  0x00000000,
	.p1_mpwldectrl0 =  0x00000000,
	.p1_mpwldectrl1 =  0x00000000,
	.p0_mpdgctrl0 =    0x0314031c,
	.p0_mpdgctrl1 =    0x023e0304,
	.p1_mpdgctrl0 =    0x03240330,
	.p1_mpdgctrl1 =    0x03180260,
	.p0_mprddlctl =    0x3630323c,
	.p1_mprddlctl =    0x3436283a,
	.p0_mpwrdlctl =    0x36344038,
	.p1_mpwrdlctl =    0x422a423c,
};

/* microSOM with Solo processor and 512MB memory */
static const struct mx6_mmdc_calibration mx6dl_512m_mmcd_calib = {
	.p0_mpwldectrl0 = 0x0045004D,
	.p0_mpwldectrl1 = 0x003A0047,
	.p0_mpdgctrl0 =   0x023C0224,
	.p0_mpdgctrl1 =   0x02000220,
	.p0_mprddlctl =   0x44444846,
	.p0_mpwrdlctl =   0x32343032,
};

/* microSOM with Dual lite processor and 1GB memory */
static const struct mx6_mmdc_calibration mx6dl_1g_mmcd_calib = {
	.p0_mpwldectrl0 =  0x0045004D,
	.p0_mpwldectrl1 =  0x003A0047,
	.p1_mpwldectrl0 =  0x001F001F,
	.p1_mpwldectrl1 =  0x00210035,
	.p0_mpdgctrl0 =    0x023C0224,
	.p0_mpdgctrl1 =    0x02000220,
	.p1_mpdgctrl0 =    0x02200220,
	.p1_mpdgctrl1 =    0x02040208,
	.p0_mprddlctl =    0x44444846,
	.p1_mprddlctl =    0x4042463C,
	.p0_mpwrdlctl =    0x32343032,
	.p1_mpwrdlctl =    0x36363430,
};

static struct mx6_ddr3_cfg mem_ddr_2g = {
	.mem_speed = 1600,
	.density   = 2,
	.width     = 16,
	.banks     = 8,
	.rowaddr   = 14,
	.coladdr   = 10,
	.pagesz    = 2,
	.trcd      = 1375,
	.trcmin    = 4875,
	.trasmin   = 3500,
};

static struct mx6_ddr3_cfg mem_ddr_4g = {
	.mem_speed = 1600,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 16,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

static void spl_dram_init(int width)
{
	struct mx6_ddr_sysinfo sysinfo = {
		/* width of data bus: 0=16, 1=32, 2=64 */
		.dsize = width / 32,
		/* config for full 4GB range so that get_mem_size() works */
		.cs_density = 32,	/* 32Gb per CS */
		.ncs = 1,		/* single chip select */
		.cs1_mirror = 0,
		.rtt_wr = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Wr = RZQ/4 */
		.rtt_nom = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Nom = RZQ/4 */
		.walat = 1,	/* Write additional latency */
		.ralat = 5,	/* Read additional latency */
		.mif3_mode = 3,	/* Command prediction working mode */
		.bi_on = 1,	/* Bank interleaving enabled */
		.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
		.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
		.ddr_type = DDR_TYPE_DDR3,
		.refsel = 1,	/* Refresh cycles at 32KHz */
		.refr = 7,	/* 8 refresh commands per refresh cycle */
	};

	if (is_mx6dq())
		mx6dq_dram_iocfg(width, &mx6q_ddr_ioregs, &mx6q_grp_ioregs);
	else
		mx6sdl_dram_iocfg(width, &mx6dl_ddr_ioregs, &mx6sdl_grp_ioregs);

	if (is_cpu_type(MXC_CPU_MX6D))
		mx6_dram_cfg(&sysinfo, &mx6q_1g_mmcd_calib, &mem_ddr_2g);
	else if (is_cpu_type(MXC_CPU_MX6Q))
		mx6_dram_cfg(&sysinfo, &mx6q_2g_mmcd_calib, &mem_ddr_4g);
	else if (is_cpu_type(MXC_CPU_MX6DL))
		mx6_dram_cfg(&sysinfo, &mx6dl_1g_mmcd_calib, &mem_ddr_2g);
	else if (is_cpu_type(MXC_CPU_MX6SOLO))
		mx6_dram_cfg(&sysinfo, &mx6dl_512m_mmcd_calib, &mem_ddr_2g);
}

void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* iomux and setup of i2c */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* Enable device tree and early DM support*/
	spl_early_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* DDR initialization */
	if (is_cpu_type(MXC_CPU_MX6SOLO))
		spl_dram_init(32);
	else
		spl_dram_init(64);

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}
#endif
