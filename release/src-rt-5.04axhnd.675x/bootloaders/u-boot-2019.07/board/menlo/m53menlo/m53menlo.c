// SPDX-License-Identifier: GPL-2.0+
/*
 * Menlosystems M53Menlo board
 *
 * Copyright (C) 2012-2017 Marek Vasut <marex@denx.de>
 * Copyright (C) 2014-2017 Olaf Mandel <o.mandel@menlosystems.com>
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux-mx53.h>
#include <asm/mach-imx/mx5_video.h>
#include <asm/mach-imx/video.h>
#include <asm/gpio.h>
#include <asm/spl.h>
#include <fdt_support.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <ipu_pixfmt.h>
#include <linux/errno.h>
#include <linux/fb.h>
#include <mmc.h>
#include <netdev.h>
#include <spl.h>
#include <splash.h>
#include <usb/ehci-ci.h>
#include <video_console.h>

DECLARE_GLOBAL_DATA_PTR;

static u32 mx53_dram_size[2];

ulong board_get_usable_ram_top(ulong total_size)
{
	/*
	 * WARNING: We must override get_effective_memsize() function here
	 * to report only the size of the first DRAM bank. This is to make
	 * U-Boot relocator place U-Boot into valid memory, that is, at the
	 * end of the first DRAM bank. If we did not override this function
	 * like so, U-Boot would be placed at the address of the first DRAM
	 * bank + total DRAM size - sizeof(uboot), which in the setup where
	 * each DRAM bank contains 512MiB of DRAM would result in placing
	 * U-Boot into invalid memory area close to the end of the first
	 * DRAM bank.
	 */
	return PHYS_SDRAM_2 + mx53_dram_size[1];
}

int dram_init(void)
{
	mx53_dram_size[0] = get_ram_size((void *)PHYS_SDRAM_1, 1 << 30);
	mx53_dram_size[1] = get_ram_size((void *)PHYS_SDRAM_2, 1 << 30);

	gd->ram_size = mx53_dram_size[0] + mx53_dram_size[1];

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = mx53_dram_size[0];

	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = mx53_dram_size[1];

	return 0;
}

static void setup_iomux_uart(void)
{
	static const iomux_v3_cfg_t uart_pads[] = {
		MX53_PAD_PATA_DMACK__UART1_RXD_MUX,
		MX53_PAD_PATA_DIOW__UART1_TXD_MUX,
	};

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
}

static void setup_iomux_fec(void)
{
	static const iomux_v3_cfg_t fec_pads[] = {
		/* MDIO pads */
		NEW_PAD_CTRL(MX53_PAD_FEC_MDIO__FEC_MDIO, PAD_CTL_HYS |
			PAD_CTL_DSE_HIGH | PAD_CTL_PUS_22K_UP | PAD_CTL_ODE),
		NEW_PAD_CTRL(MX53_PAD_FEC_MDC__FEC_MDC, PAD_CTL_DSE_HIGH),

		/* FEC 0 pads */
		NEW_PAD_CTRL(MX53_PAD_FEC_CRS_DV__FEC_RX_DV,
			     PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_REF_CLK__FEC_TX_CLK,
			     PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_RX_ER__FEC_RX_ER,
			     PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_TX_EN__FEC_TX_EN, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_RXD0__FEC_RDATA_0,
			     PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_RXD1__FEC_RDATA_1,
			     PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_TXD0__FEC_TDATA_0, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_TXD1__FEC_TDATA_1, PAD_CTL_DSE_HIGH),

		/* FEC 1 pads */
		NEW_PAD_CTRL(MX53_PAD_KEY_COL0__FEC_RDATA_3,
			     PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_KEY_ROW0__FEC_TX_ER,
			     PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_KEY_COL1__FEC_RX_CLK,
			     PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_KEY_ROW1__FEC_COL,
			     PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_KEY_COL2__FEC_RDATA_2,
			     PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_KEY_ROW2__FEC_TDATA_2, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_KEY_COL3__FEC_CRS,
			     PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_GPIO_19__FEC_TDATA_3, PAD_CTL_DSE_HIGH),
	};

	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));
}

static void enable_lvds_clock(struct display_info_t const *dev, const u8 hclk)
{
	static struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)MXC_CCM_BASE;
	int ret;

	/* For ETM0430G0DH6 model, this must be enabled before the clock. */
	gpio_direction_output(IMX_GPIO_NR(6, 0), 1);

	/*
	 * Set LVDS clock to 33.28 MHz for the display. The PLL4 is set to
	 * 233 MHz, divided by 7 by setting CCM_CSCMR2 LDB_DI0_IPU_DIV=1 .
	 */
	ret = mxc_set_clock(MXC_HCLK, hclk, MXC_LDB_CLK);
	if (ret)
		puts("IPU:   Failed to configure LDB clock\n");

	/* Configure CCM_CSCMR2 */
	clrsetbits_le32(&mxc_ccm->cscmr2,
			(0x7 << 26) | BIT(10) | BIT(8),
			(0x5 << 26) | BIT(10) | BIT(8));

	/* Configure LDB_CTRL */
	writel(0x201, 0x53fa8008);
}

static void enable_lvds_etm0430g0dh6(struct display_info_t const *dev)
{
	gpio_request(IMX_GPIO_NR(6, 0), "LCD");

	/* For ETM0430G0DH6 model, this must be enabled before the clock. */
	gpio_direction_output(IMX_GPIO_NR(6, 0), 1);

	/*
	 * Set LVDS clock to 9 MHz for the display. The PLL4 is set to
	 * 63 MHz, divided by 7 by setting CCM_CSCMR2 LDB_DI0_IPU_DIV=1 .
	 */
	enable_lvds_clock(dev, 63);
}

static void enable_lvds_etm0700g0dh6(struct display_info_t const *dev)
{
	gpio_request(IMX_GPIO_NR(6, 0), "LCD");

	/*
	 * Set LVDS clock to 33.28 MHz for the display. The PLL4 is set to
	 * 233 MHz, divided by 7 by setting CCM_CSCMR2 LDB_DI0_IPU_DIV=1 .
	 */
	enable_lvds_clock(dev, 233);

	/* For ETM0700G0DH6 model, this may be enabled after the clock. */
	gpio_direction_output(IMX_GPIO_NR(6, 0), 1);
}

static const char *lvds_compat_string;

static int detect_lvds(struct display_info_t const *dev)
{
	u8 touchid[23];
	u8 *touchptr = &touchid[0];
	int ret;

	ret = i2c_set_bus_num(0);
	if (ret)
		return 0;

	/* Touchscreen is at address 0x38, ID register is 0xbb. */
	ret = i2c_read(0x38, 0xbb, 1, touchid, sizeof(touchid));
	if (ret)
		return 0;

	/* EP0430 prefixes the response with 0xbb, skip it. */
	if (*touchptr == 0xbb)
		touchptr++;

	/* Skip the 'EP' prefix. */
	touchptr += 2;

	ret = !memcmp(touchptr, &dev->mode.name[7], 4);
	if (ret)
		lvds_compat_string = dev->mode.name;

	return ret;
}

void board_preboot_os(void)
{
	/* Power off the LCD to prevent awful color flicker */
	gpio_direction_output(IMX_GPIO_NR(6, 0), 0);
}

int ft_board_setup(void *blob, bd_t *bd)
{
	if (lvds_compat_string)
		do_fixup_by_path_string(blob, "/panel", "compatible",
					lvds_compat_string);

	return 0;
}

struct display_info_t const displays[] = {
	{
		.bus	= 0,
		.addr	= 0,
		.detect	= detect_lvds,
		.enable	= enable_lvds_etm0430g0dh6,
		.pixfmt	= IPU_PIX_FMT_RGB666,
		.mode	= {
			.name		= "edt,etm0430g0dh6",
			.refresh	= 60,
			.xres		= 480,
			.yres		= 272,
			.pixclock	= 111111, /* picosecond (9 MHz) */
			.left_margin	= 2,
			.right_margin	= 2,
			.upper_margin	= 2,
			.lower_margin	= 2,
			.hsync_len	= 41,
			.vsync_len	= 10,
			.sync		= 0x40000000,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	}, {
		.bus	= 0,
		.addr	= 0,
		.detect	= detect_lvds,
		.enable	= enable_lvds_etm0700g0dh6,
		.pixfmt	= IPU_PIX_FMT_RGB666,
		.mode	= {
			.name		= "edt,etm0700g0dh6",
			.refresh	= 60,
			.xres		= 800,
			.yres		= 480,
			.pixclock	= 30048, /* picosecond (33.28 MHz) */
			.left_margin	= 40,
			.right_margin	= 88,
			.upper_margin	= 10,
			.lower_margin	= 33,
			.hsync_len	= 128,
			.vsync_len	= 2,
			.sync		= FB_SYNC_EXT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	}
};

size_t display_count = ARRAY_SIZE(displays);

#ifdef CONFIG_SPLASH_SCREEN
static struct splash_location default_splash_locations[] = {
	{
		.name		= "mmc_fs",
		.storage	= SPLASH_STORAGE_MMC,
		.flags		= SPLASH_STORAGE_FS,
		.devpart	= "0:1",
	},
};

int splash_screen_prepare(void)
{
	return splash_source_load(default_splash_locations,
				  ARRAY_SIZE(default_splash_locations));
}
#endif

int board_late_init(void)
{
#if defined(CONFIG_VIDEO_IPUV3)
	struct udevice *dev;
	int xpos, ypos, ret;
	char *s;
	void *dst;
	ulong addr, len;

	splash_get_pos(&xpos, &ypos);

	s = env_get("splashimage");
	if (!s)
		return 0;

	addr = simple_strtoul(s, NULL, 16);
	dst = malloc(CONFIG_SYS_VIDEO_LOGO_MAX_SIZE);
	if (!dst)
		return -ENOMEM;

	ret = splash_screen_prepare();
	if (ret < 0)
		return ret;

	len = CONFIG_SYS_VIDEO_LOGO_MAX_SIZE;
	ret = gunzip(dst + 2, CONFIG_SYS_VIDEO_LOGO_MAX_SIZE - 2,
		     (uchar *)addr, &len);
	if (ret) {
		printf("Error: no valid bmp or bmp.gz image at %lx\n", addr);
		free(dst);
		return ret;
	}

	ret = uclass_get_device(UCLASS_VIDEO, 0, &dev);
	if (ret)
		return ret;

	ret = video_bmp_display(dev, (ulong)dst + 2, xpos, ypos, true);
	if (ret)
		return ret;
#endif
	return 0;
}

#define I2C_PAD_CTRL	(PAD_CTL_SRE_FAST | PAD_CTL_DSE_HIGH | \
			 PAD_CTL_PUS_100K_UP | PAD_CTL_ODE)

static void setup_iomux_i2c(void)
{
	static const iomux_v3_cfg_t i2c_pads[] = {
		/* I2C1 */
		NEW_PAD_CTRL(MX53_PAD_EIM_D28__I2C1_SDA, I2C_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_EIM_D21__I2C1_SCL, I2C_PAD_CTRL),
		/* I2C2 */
		NEW_PAD_CTRL(MX53_PAD_EIM_D16__I2C2_SDA, I2C_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_EIM_EB2__I2C2_SCL, I2C_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(i2c_pads, ARRAY_SIZE(i2c_pads));
}

static void setup_iomux_video(void)
{
	static const iomux_v3_cfg_t lcd_pads[] = {
		MX53_PAD_LVDS0_TX3_P__LDB_LVDS0_TX3,
		MX53_PAD_LVDS0_CLK_P__LDB_LVDS0_CLK,
		MX53_PAD_LVDS0_TX2_P__LDB_LVDS0_TX2,
		MX53_PAD_LVDS0_TX1_P__LDB_LVDS0_TX1,
		MX53_PAD_LVDS0_TX0_P__LDB_LVDS0_TX0,
	};

	imx_iomux_v3_setup_multiple_pads(lcd_pads, ARRAY_SIZE(lcd_pads));
}

static void setup_iomux_nand(void)
{
	static const iomux_v3_cfg_t nand_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_NANDF_WE_B__EMI_NANDF_WE_B,
			     PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_NANDF_RE_B__EMI_NANDF_RE_B,
			     PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_NANDF_CLE__EMI_NANDF_CLE,
			     PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_NANDF_ALE__EMI_NANDF_ALE,
			     PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_NANDF_WP_B__EMI_NANDF_WP_B,
			     PAD_CTL_PUS_100K_UP),
		NEW_PAD_CTRL(MX53_PAD_NANDF_RB0__EMI_NANDF_RB_0,
			     PAD_CTL_PUS_100K_UP),
		NEW_PAD_CTRL(MX53_PAD_NANDF_CS0__EMI_NANDF_CS_0,
			     PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA0__EMI_NANDF_D_0,
			     PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA1__EMI_NANDF_D_1,
			     PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA2__EMI_NANDF_D_2,
			     PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA3__EMI_NANDF_D_3,
			     PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA4__EMI_NANDF_D_4,
			     PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA5__EMI_NANDF_D_5,
			     PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA6__EMI_NANDF_D_6,
			     PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA7__EMI_NANDF_D_7,
			     PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
	};

	imx_iomux_v3_setup_multiple_pads(nand_pads, ARRAY_SIZE(nand_pads));
}

static void m53_set_clock(void)
{
	int ret;
	const u32 ref_clk = MXC_HCLK;
	const u32 dramclk = 400;
	u32 cpuclk;

	gpio_request(IMX_GPIO_NR(4, 0), "CPUCLK");

	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX53_PAD_GPIO_10__GPIO4_0,
					    PAD_CTL_DSE_HIGH | PAD_CTL_PKE));
	gpio_direction_input(IMX_GPIO_NR(4, 0));

	/* GPIO10 selects modules' CPU speed, 1 = 1200MHz ; 0 = 800MHz */
	cpuclk = gpio_get_value(IMX_GPIO_NR(4, 0)) ? 1200 : 800;

	ret = mxc_set_clock(ref_clk, cpuclk, MXC_ARM_CLK);
	if (ret)
		printf("CPU:   Switch CPU clock to %dMHz failed\n", cpuclk);

	ret = mxc_set_clock(ref_clk, dramclk, MXC_PERIPH_CLK);
	if (ret) {
		printf("CPU:   Switch peripheral clock to %dMHz failed\n",
		       dramclk);
	}

	ret = mxc_set_clock(ref_clk, dramclk, MXC_DDR_CLK);
	if (ret)
		printf("CPU:   Switch DDR clock to %dMHz failed\n", dramclk);
}

static void m53_set_nand(void)
{
	u32 i;

	/* NAND flash is muxed on ATA pins */
	setbits_le32(M4IF_BASE_ADDR + 0xc, M4IF_GENP_WEIM_MM_MASK);

	/* Wait for Grant/Ack sequence (see EIM_CSnGCR2:MUX16_BYP_GRANT) */
	for (i = 0x4; i < 0x94; i += 0x18) {
		clrbits_le32(WEIM_BASE_ADDR + i,
			     WEIM_GCR2_MUX16_BYP_GRANT_MASK);
	}

	mxc_set_clock(0, 33, MXC_NFC_CLK);
	enable_nfc_clk(1);
}

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_iomux_fec();
	setup_iomux_i2c();
	setup_iomux_nand();
	setup_iomux_video();

	m53_set_clock();

	mxc_set_sata_internal_clock();

	/* NAND clock @ 33MHz */
	m53_set_nand();

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int checkboard(void)
{
	puts("Board: Menlosystems M53Menlo\n");

	return 0;
}

/*
 * NAND SPL
 */
#ifdef CONFIG_SPL_BUILD
void spl_board_init(void)
{
	setup_iomux_nand();
	m53_set_clock();
	m53_set_nand();
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NAND;
}
#endif
