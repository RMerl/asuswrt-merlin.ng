/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) Aspeed Technology Inc.
 */
#ifndef _ASM_ARCH_SCU_AST2600_H
#define _ASM_ARCH_SCU_AST2600_H

#define SCU_UNLOCK_KEY			0x1688a8a8

#define SCU_CLKGATE1_EMMC			BIT(27)
#define SCU_CLKGATE1_ACRY			BIT(24)
#define SCU_CLKGATE1_MAC2			BIT(21)
#define SCU_CLKGATE1_MAC1			BIT(20)
#define SCU_CLKGATE1_USB_HUB			BIT(14)
#define SCU_CLKGATE1_HACE			BIT(13)
#define SCU_CLKGATE1_USB_HOST2			BIT(7)

#define SCU_CLKGATE2_FSI			BIT(30)
#define SCU_CLKGATE2_MAC4			BIT(21)
#define SCU_CLKGATE2_MAC3			BIT(20)
#define SCU_CLKGATE2_SDIO			BIT(4)

#define SCU_DRAM_HDSHK_SOC_INIT			BIT(7)
#define SCU_DRAM_HDSHK_RDY			BIT(6)

#define SCU_CLKSRC1_ECC_RSA_DIV_MASK		GENMASK(27, 26)
#define SCU_CLKSRC1_ECC_RSA_DIV_SHIFT		26
#define SCU_CLKSRC1_PCLK_DIV_MASK		GENMASK(25, 23)
#define SCU_CLKSRC1_PCLK_DIV_SHIFT		23
#define SCU_CLKSRC1_BCLK_DIV_MASK		GENMASK(22, 20)
#define SCU_CLKSRC1_BCLK_DIV_SHIFT		20
#define SCU_CLKSRC1_ECC_RSA			BIT(19)
#define SCU_CLKSRC1_MAC_DIV_MASK		GENMASK(18, 16)
#define SCU_CLKSRC1_MAC_DIV_SHIFT		16
#define SCU_CLKSRC1_EMMC_EN			BIT(15)
#define SCU_CLKSRC1_EMMC_DIV_MASK		GENMASK(14, 12)
#define SCU_CLKSRC1_EMMC_DIV_SHIFT		12
#define SCU_CLKSRC1_EMMC			BIT(11)

#define SCU_CLKSRC2_RMII12			BIT(19)
#define SCU_CLKSRC2_RMII12_DIV_MASK		GENMASK(18, 16)
#define SCU_CLKSRC2_RMII12_DIV_SHIFT		16
#define SCU_CLKSRC2_UART5			BIT(14)

#define SCU_CLKSRC4_SDIO_EN			BIT(31)
#define SCU_CLKSRC4_SDIO_DIV_MASK		GENMASK(30, 28)
#define SCU_CLKSRC4_SDIO_DIV_SHIFT		28
#define SCU_CLKSRC4_MAC_DIV_MASK		GENMASK(26, 24)
#define SCU_CLKSRC4_MAC_DIV_SHIFT		24
#define SCU_CLKSRC4_RMII34_DIV_MASK		GENMASK(18, 16)
#define SCU_CLKSRC4_RMII34_DIV_SHIFT		16
#define SCU_CLKSRC4_PCLK_DIV_MASK		GENMASK(11, 9)
#define SCU_CLKSRC4_PCLK_DIV_SHIFT		9
#define SCU_CLKSRC4_SDIO			BIT(8)
#define SCU_CLKSRC4_UART6			BIT(5)
#define SCU_CLKSRC4_UART4			BIT(3)
#define SCU_CLKSRC4_UART3			BIT(2)
#define SCU_CLKSRC4_UART2			BIT(1)
#define SCU_CLKSRC4_UART1			BIT(0)

#define SCU_CLKSRC5_UART13			BIT(12)
#define SCU_CLKSRC5_UART12			BIT(11)
#define SCU_CLKSRC5_UART11			BIT(10)
#define SCU_CLKSRC5_UART10			BIT(9)
#define SCU_CLKSRC5_UART9			BIT(8)
#define SCU_CLKSRC5_UART8			BIT(7)
#define SCU_CLKSRC5_UART7			BIT(6)
#define SCU_CLKSRC5_HUXCLK_MASK			GENMASK(5, 3)
#define SCU_CLKSRC5_HUXCLK_SHIFT		3
#define SCU_CLKSRC5_UXCLK_MASK			GENMASK(2, 0)
#define SCU_CLKSRC5_UXCLK_SHIFT			0

#define SCU_PINCTRL1_EMMC_MASK			GENMASK(31, 24)
#define SCU_PINCTRL1_EMMC_SHIFT			24

#define SCU_PINCTRL16_MAC4_DRIVING_MASK		GENMASK(3, 2)
#define SCU_PINCTRL16_MAC4_DRIVING_SHIFT	2
#define SCU_PINCTRL16_MAC3_DRIVING_MASK		GENMASK(1, 0)
#define SCU_PINCTRL16_MAC3_DRIVING_SHIFT	0

#define SCU_HWSTRAP1_CPU_AXI_CLK_RATIO		BIT(16)
#define SCU_HWSTRAP1_VGA_MEM_MASK		GENMASK(14, 13)
#define SCU_HWSTRAP1_VGA_MEM_SHIFT		13
#define SCU_HWSTRAP1_AXI_AHB_CLK_RATIO_MASK	GENMASK(12, 11)
#define SCU_HWSTRAP1_AXI_AHB_CLK_RATIO_SHIFT	11
#define SCU_HWSTRAP1_CPU_FREQ_MASK		GENMASK(10, 8)
#define SCU_HWSTRAP1_CPU_FREQ_SHIFT		8
#define SCU_HWSTRAP1_MAC2_INTF			BIT(7)
#define SCU_HWSTRAP1_MAC1_INTF			BIT(6)
#define SCU_HWSTRAP1_BOOT_EMMC			BIT(2)

#define SCU_HWSTRAP2_BOOT_UART			BIT(8)

#define SCU_EFUSE_DIS_DP			BIT(17)
#define SCU_EFUSE_DIS_VGA			BIT(14)
#define SCU_EFUSE_DIS_PCIE_EP			BIT(13)
#define SCU_EFUSE_DIS_USB			BIT(12)
#define SCU_EFUSE_DIS_RVAS			BIT(10)
#define SCU_EFUSE_DIS_VIDEO_DEC			BIT(9)
#define SCU_EFUSE_DIS_VIDEO			BIT(8)
#define SCU_EFUSE_DIS_PCIE_RC			BIT(7)
#define SCU_EFUSE_DIS_CM3			BIT(6)
#define SCU_EFUSE_DIS_CA7			BIT(5)

#define SCU_PLL_RST				BIT(25)
#define SCU_PLL_BYPASS				BIT(24)
#define SCU_PLL_OFF				BIT(23)
#define SCU_PLL_DIV_MASK			GENMASK(22, 19)
#define SCU_PLL_DIV_SHIFT			19
#define SCU_PLL_DENUM_MASK			GENMASK(18, 13)
#define SCU_PLL_DENUM_SHIFT			13
#define SCU_PLL_NUM_MASK			GENMASK(12, 0)
#define SCU_PLL_NUM_SHIFT			0

#define SCU_UART_CLKGEN_N_MASK			GENMASK(17, 8)
#define SCU_UART_CLKGEN_N_SHIFT			8
#define SCU_UART_CLKGEN_R_MASK			GENMASK(7, 0)
#define SCU_UART_CLKGEN_R_SHIFT			0

#define SCU_HUART_CLKGEN_N_MASK			GENMASK(17, 8)
#define SCU_HUART_CLKGEN_N_SHIFT		8
#define SCU_HUART_CLKGEN_R_MASK			GENMASK(7, 0)
#define SCU_HUART_CLKGEN_R_SHIFT		0

#define SCU_MISC_CTRL1_UART5_DIV		BIT(12)

#ifndef __ASSEMBLY__
struct ast2600_scu {
	uint32_t prot_key1;		/* 0x000 */
	uint32_t chip_id1;		/* 0x004 */
	uint32_t rsv_0x08;		/* 0x008 */
	uint32_t rsv_0x0c;		/* 0x00C */
	uint32_t prot_key2;		/* 0x010 */
	uint32_t chip_id2;		/* 0x014 */
	uint32_t rsv_0x18[10];		/* 0x018 ~ 0x03C */
	uint32_t modrst_ctrl1;		/* 0x040 */
	uint32_t modrst_clr1;		/* 0x044 */
	uint32_t rsv_0x48;		/* 0x048 */
	uint32_t rsv_0x4C;		/* 0x04C */
	uint32_t modrst_ctrl2;		/* 0x050 */
	uint32_t modrst_clr2;		/* 0x054 */
	uint32_t rsv_0x58;		/* 0x058 */
	uint32_t rsv_0x5C;		/* 0x05C */
	uint32_t extrst_sel1;		/* 0x060 */
	uint32_t sysrst_sts1_1;		/* 0x064 */
	uint32_t sysrst_sts1_2;		/* 0x068 */
	uint32_t sysrst_sts1_3;		/* 0x06C */
	uint32_t extrst_sel2;		/* 0x070 */
	uint32_t sysrst_sts2_1;		/* 0x074 */
	uint32_t sysrst_sts2_2;		/* 0x078 */
	uint32_t stsrst_sts3_2;		/* 0x07C */
	uint32_t clkgate_ctrl1;		/* 0x080 */
	uint32_t clkgate_clr1;		/* 0x084 */
	uint32_t rsv_0x88;		/* 0x088 */
	uint32_t rsv_0x8C;		/* 0x08C */
	uint32_t clkgate_ctrl2;		/* 0x090 */
	uint32_t clkgate_clr2;		/* 0x094 */
	uint32_t rsv_0x98[10];		/* 0x098 ~ 0x0BC */
	uint32_t misc_ctrl1;		/* 0x0C0 */
	uint32_t misc_ctrl2;		/* 0x0C4 */
	uint32_t debug_ctrl1;		/* 0x0C8 */
	uint32_t rsv_0xCC;		/* 0x0CC */
	uint32_t misc_ctrl3;		/* 0x0D0 */
	uint32_t misc_ctrl4;		/* 0x0D4 */
	uint32_t debug_ctrl2;		/* 0x0D8 */
	uint32_t rsv_0xdc[9];		/* 0x0DC ~ 0x0FC */
	uint32_t dram_hdshk;		/* 0x100 */
	uint32_t soc_scratch[3];	/* 0x104 ~ 0x10C */
	uint32_t rsv_0x110[4];		/* 0x110 ~ 0x11C*/
	uint32_t cpu_scratch_wp;	/* 0x120 */
	uint32_t rsv_0x124[23];		/* 0x124 */
	uint32_t smp_boot[12];		/* 0x180 */
	uint32_t cpu_scratch[20];	/* 0x1b0 */
	uint32_t hpll;			/* 0x200 */
	uint32_t hpll_ext;		/* 0x204 */
	uint32_t rsv_0x208[2];		/* 0x208 ~ 0x20C */
	uint32_t apll;			/* 0x210 */
	uint32_t apll_ext;		/* 0x214 */
	uint32_t rsv_0x218[2];		/* 0x218 ~ 0x21C */
	uint32_t mpll;			/* 0x220 */
	uint32_t mpll_ext;		/* 0x224 */
	uint32_t rsv_0x228[6];		/* 0x228 ~ 0x23C */
	uint32_t epll;			/* 0x240 */
	uint32_t epll_ext;		/* 0x244 */
	uint32_t rsv_0x248[6];		/* 0x248 ~ 0x25C */
	uint32_t dpll;			/* 0x260 */
	uint32_t dpll_ext;		/* 0x264 */
	uint32_t rsv_0x268[38];		/* 0x268 ~ 0x2FC */
	uint32_t clksrc1;		/* 0x300 */
	uint32_t clksrc2;		/* 0x304 */
	uint32_t clksrc3;		/* 0x308 */
	uint32_t rsv_0x30c;		/* 0x30C */
	uint32_t clksrc4;		/* 0x310 */
	uint32_t clksrc5;		/* 0x314 */
	uint32_t rsv_0x318[2];		/* 0x318 ~ 0x31C */
	uint32_t freq_counter_ctrl1;	/* 0x320 */
	uint32_t freq_counter_cmp1;	/* 0x324 */
	uint32_t rsv_0x328[2];		/* 0x328 ~ 0x32C */
	uint32_t freq_counter_ctrl2;	/* 0x330 */
	uint32_t freq_counter_cmp2;	/* 0x334 */
	uint32_t uart_clkgen;		/* 0x338 */
	uint32_t huart_clkgen;		/* 0x33C */
	uint32_t mac12_clk_delay;	/* 0x340 */
	uint32_t rsv_0x344;		/* 0x344 */
	uint32_t mac12_clk_delay_100M;	/* 0x348 */
	uint32_t mac12_clk_delay_10M;	/* 0x34C */
	uint32_t mac34_clk_delay;	/* 0x350 */
	uint32_t rsv_0x354;		/* 0x354 */
	uint32_t mac34_clk_delay_100M;	/* 0x358 */
	uint32_t mac34_clk_delay_10M;	/* 0x35C */
	uint32_t clkduty_meas_ctrl;	/* 0x360 */
	uint32_t clkduty1;		/* 0x364 */
	uint32_t clkduty2;		/* 0x368 */
	uint32_t clkduty_meas_res;	/* 0x36C */
	uint32_t clkduty_meas_ctrl2;	/* 0x370 */
	uint32_t clkduty3;		/* 0x374 */
	uint32_t rsv_0x378[34];		/* 0x378 ~ 0x3FC */
	uint32_t pinmux1;		/* 0x400 */
	uint32_t pinmux2;		/* 0x404 */
	uint32_t rsv_0x408;		/* 0x408 */
	uint32_t pinmux3;		/* 0x40C */
	uint32_t pinmux4;		/* 0x410 */
	uint32_t pinmux5;		/* 0x414 */
	uint32_t pinmux6;		/* 0x418 */
	uint32_t pinmux7;		/* 0x41C */
	uint32_t rsv_0x420[4];		/* 0x420 ~ 0x42C */
	uint32_t pinmux8;		/* 0x430 */
	uint32_t pinmux9;		/* 0x434 */
	uint32_t pinmux10;		/* 0x438 */
	uint32_t rsv_0x43c;		/* 0x43C */
	uint32_t pinmux12;		/* 0x440 */
	uint32_t pinmux13;		/* 0x444 */
	uint32_t rsv_0x448[2];		/* 0x448 ~ 0x44C */
	uint32_t pinmux14;		/* 0x450 */
	uint32_t pinmux15;		/* 0x454 */
	uint32_t pinmux16;		/* 0x458 */
	uint32_t rsv_0x45c[21];		/* 0x45C ~ 0x4AC */
	uint32_t pinmux17;		/* 0x4B0 */
	uint32_t pinmux18;		/* 0x4B4 */
	uint32_t pinmux19;		/* 0x4B8 */
	uint32_t pinmux20;		/* 0x4BC */
	uint32_t rsv_0x4c0[5];		/* 0x4C0 ~ 0x4D0 */
	uint32_t pinmux22;		/* 0x4D4 */
	uint32_t pinmux23;		/* 0x4D8 */
	uint32_t rsv_0x4dc[9];		/* 0x4DC ~ 0x4FC */
	uint32_t hwstrap1;		/* 0x500 */
	uint32_t hwstrap_clr1;		/* 0x504 */
	uint32_t hwstrap_prot1;		/* 0x508 */
	uint32_t rsv_0x50c;		/* 0x50C */
	uint32_t hwstrap2;		/* 0x510 */
	uint32_t hwstrap_clr2;		/* 0x514 */
	uint32_t hwstrap_prot2;		/* 0x518 */
	uint32_t rsv_0x51c;		/* 0x51C */
	uint32_t rng_ctrl;		/* 0x520 */
	uint32_t rng_data;		/* 0x524 */
	uint32_t rsv_0x528[6];		/* 0x528 ~ 0x53C */
	uint32_t pwr_save_wakeup_en1;	/* 0x540 */
	uint32_t pwr_save_wakeup_ctrl1;	/* 0x544 */
	uint32_t rsv_0x548[2];		/* 0x548 */
	uint32_t pwr_save_wakeup_en2;	/* 0x550 */
	uint32_t pwr_save_wakeup_ctrl2;	/* 0x554 */
	uint32_t rsv_0x558[2];		/* 0x558 */
	uint32_t intr1_ctrl_sts;	/* 0x560 */
	uint32_t rsv_0x564[3];		/* 0x564 */
	uint32_t intr2_ctrl_sts;	/* 0x570 */
	uint32_t rsv_0x574[7];		/* 0x574 ~ 0x58C */
	uint32_t otp_ctrl;		/* 0x590 */
	uint32_t efuse;			/* 0x594 */
	uint32_t rsv_0x598[6];		/* 0x598 */
	uint32_t chip_unique_id[8];	/* 0x5B0 */
	uint32_t rsv_0x5e0[8];		/* 0x5E0 ~ 0x5FC */
	uint32_t disgpio_in_pull_down0;	/* 0x610 */
	uint32_t disgpio_in_pull_down1;	/* 0x614 */
	uint32_t disgpio_in_pull_down2;	/* 0x618 */
	uint32_t disgpio_in_pull_down3;	/* 0x61C */
	uint32_t rsv_0x620[4];		/* 0x620 ~ 0x62C */
	uint32_t disgpio_in_pull_down4;	/* 0x630 */
	uint32_t disgpio_in_pull_down5;	/* 0x634 */
	uint32_t disgpio_in_pull_down6;	/* 0x638 */
	uint32_t rsv_0x63c[5];		/* 0x63C ~ 0x64C */
	uint32_t sli_driving_strength;	/* 0x650 */
	uint32_t rsv_0x654[107];	/* 0x654 ~ 0x7FC */
	uint32_t ca7_ctrl1;		/* 0x800 */
	uint32_t ca7_ctrl2;		/* 0x804 */
	uint32_t ca7_ctrl3;		/* 0x808 */
	uint32_t ca7_ctrl4;		/* 0x80C */
	uint32_t rsv_0x810[4];		/* 0x810 ~ 0x81C */
	uint32_t ca7_parity_chk;	/* 0x820 */
	uint32_t ca7_parity_clr;	/* 0x824 */
	uint32_t rsv_0x828[118];	/* 0x828 ~ 0x9FC */
	uint32_t cm3_ctrl;		/* 0xA00 */
	uint32_t cm3_base;		/* 0xA04 */
	uint32_t cm3_imem_addr;		/* 0xA08 */
	uint32_t cm3_dmem_addr;		/* 0xA0C */
	uint32_t rsv_0xa10[12];		/* 0xA10 ~ 0xA3C */
	uint32_t cm3_cache_area;	/* 0xA40 */
	uint32_t cm3_cache_invd_ctrl;	/* 0xA44 */
	uint32_t cm3_cache_func_ctrl;	/* 0xA48 */
	uint32_t rsv_0xa4c[108];	/* 0xA4C ~ 0xBFC */
	uint32_t pci_cfg[3];		/* 0xC00 */
	uint32_t rsv_0xc0c[5];		/* 0xC0C ~ 0xC1C */
	uint32_t pcie_cfg;		/* 0xC20 */
	uint32_t mmio_decode;		/* 0xC24 */
	uint32_t reloc_ctrl_decode[2];	/* 0xC28 */
	uint32_t rsv_0xc30[4];		/* 0xC30 ~ 0xC3C */
	uint32_t mbox_decode;		/* 0xC40 */
	uint32_t shared_sram_decode[2];	/* 0xC44 */
	uint32_t bmc_rev_id;		/* 0xC4C */
	uint32_t rsv_0xc50[5];		/* 0xC50 ~ 0xC60 */
	uint32_t bmc_device_id;		/* 0xC64 */
	uint32_t rsv_0xc68[102];	/* 0xC68 ~ 0xDFC */
	uint32_t vga_scratch1;		/* 0xE00 */
	uint32_t vga_scratch2;		/* 0xE04 */
	uint32_t vga_scratch3;		/* 0xE08 */
	uint32_t vga_scratch4;		/* 0xE0C */
	uint32_t rsv_0xe10[4];		/* 0xE10 ~ 0xE1C */
	uint32_t vga_scratch5;		/* 0xE20 */
	uint32_t vga_scratch6;		/* 0xE24 */
	uint32_t vga_scratch7;		/* 0xE28 */
	uint32_t vga_scratch8;		/* 0xE2C */
	uint32_t rsv_0xe30[52];		/* 0xE30 ~ 0xEFC */
	uint32_t wr_prot1;		/* 0xF00 */
	uint32_t wr_prot2;		/* 0xF04 */
	uint32_t wr_prot3;		/* 0xF08 */
	uint32_t wr_prot4;		/* 0xF0C */
	uint32_t wr_prot5;		/* 0xF10 */
	uint32_t wr_prot6;		/* 0xF18 */
	uint32_t wr_prot7;		/* 0xF1C */
	uint32_t wr_prot8;		/* 0xF20 */
	uint32_t wr_prot9;		/* 0xF24 */
	uint32_t rsv_0xf28[2];		/* 0xF28 ~ 0xF2C */
	uint32_t wr_prot10;		/* 0xF30 */
	uint32_t wr_prot11;		/* 0xF34 */
	uint32_t wr_prot12;		/* 0xF38 */
	uint32_t wr_prot13;		/* 0xF3C */
	uint32_t wr_prot14;		/* 0xF40 */
	uint32_t rsv_0xf44;		/* 0xF44 */
	uint32_t wr_prot15;		/* 0xF48 */
	uint32_t rsv_0xf4c[5];		/* 0xF4C ~ 0xF5C */
	uint32_t wr_prot16;		/* 0xF60 */
};
#endif
#endif
