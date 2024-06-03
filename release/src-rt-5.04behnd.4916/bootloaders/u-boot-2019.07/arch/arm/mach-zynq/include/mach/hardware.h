/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Xilinx Inc.
 */

#ifndef _ASM_ARCH_HARDWARE_H
#define _ASM_ARCH_HARDWARE_H

#define ZYNQ_SYS_CTRL_BASEADDR		0xF8000000
#define ZYNQ_DEV_CFG_APB_BASEADDR	0xF8007000
#define ZYNQ_SCU_BASEADDR		0xF8F00000
#define ZYNQ_QSPI_BASEADDR		0xE000D000
#define ZYNQ_SMC_BASEADDR		0xE000E000
#define ZYNQ_NAND_BASEADDR		0xE1000000
#define ZYNQ_DDRC_BASEADDR		0xF8006000
#define ZYNQ_EFUSE_BASEADDR		0xF800D000
#define ZYNQ_USB_BASEADDR0		0xE0002000
#define ZYNQ_USB_BASEADDR1		0xE0003000
#define ZYNQ_OCM_BASEADDR		0xFFFC0000

/* Bootmode setting values */
#define ZYNQ_BM_MASK		0x7
#define ZYNQ_BM_QSPI		0x1
#define ZYNQ_BM_NOR		0x2
#define ZYNQ_BM_NAND		0x4
#define ZYNQ_BM_SD		0x5
#define ZYNQ_BM_JTAG		0x0

/* Reflect slcr offsets */
struct slcr_regs {
	u32 scl; /* 0x0 */
	u32 slcr_lock; /* 0x4 */
	u32 slcr_unlock; /* 0x8 */
	u32 reserved0_1[61];
	u32 arm_pll_ctrl; /* 0x100 */
	u32 ddr_pll_ctrl; /* 0x104 */
	u32 io_pll_ctrl; /* 0x108 */
	u32 reserved0_2[5];
	u32 arm_clk_ctrl; /* 0x120 */
	u32 ddr_clk_ctrl; /* 0x124 */
	u32 dci_clk_ctrl; /* 0x128 */
	u32 aper_clk_ctrl; /* 0x12c */
	u32 reserved0_3[2];
	u32 gem0_rclk_ctrl; /* 0x138 */
	u32 gem1_rclk_ctrl; /* 0x13c */
	u32 gem0_clk_ctrl; /* 0x140 */
	u32 gem1_clk_ctrl; /* 0x144 */
	u32 smc_clk_ctrl; /* 0x148 */
	u32 lqspi_clk_ctrl; /* 0x14c */
	u32 sdio_clk_ctrl; /* 0x150 */
	u32 uart_clk_ctrl; /* 0x154 */
	u32 spi_clk_ctrl; /* 0x158 */
	u32 can_clk_ctrl; /* 0x15c */
	u32 can_mioclk_ctrl; /* 0x160 */
	u32 dbg_clk_ctrl; /* 0x164 */
	u32 pcap_clk_ctrl; /* 0x168 */
	u32 reserved0_4[1];
	u32 fpga0_clk_ctrl; /* 0x170 */
	u32 reserved0_5[3];
	u32 fpga1_clk_ctrl; /* 0x180 */
	u32 reserved0_6[3];
	u32 fpga2_clk_ctrl; /* 0x190 */
	u32 reserved0_7[3];
	u32 fpga3_clk_ctrl; /* 0x1a0 */
	u32 reserved0_8[8];
	u32 clk_621_true; /* 0x1c4 */
	u32 reserved1[14];
	u32 pss_rst_ctrl; /* 0x200 */
	u32 reserved2[15];
	u32 fpga_rst_ctrl; /* 0x240 */
	u32 reserved3[5];
	u32 reboot_status; /* 0x258 */
	u32 boot_mode; /* 0x25c */
	u32 reserved4[116];
	u32 trust_zone; /* 0x430 */ /* FIXME */
	u32 reserved5_1[63];
	u32 pss_idcode; /* 0x530 */
	u32 reserved5_2[51];
	u32 ddr_urgent; /* 0x600 */
	u32 reserved6[6];
	u32 ddr_urgent_sel; /* 0x61c */
	u32 reserved7[56];
	u32 mio_pin[54]; /* 0x700 - 0x7D4 */
	u32 reserved8[74];
	u32 lvl_shftr_en; /* 0x900 */
	u32 reserved9[3];
	u32 ocm_cfg; /* 0x910 */
};

#define slcr_base ((struct slcr_regs *)ZYNQ_SYS_CTRL_BASEADDR)

struct devcfg_regs {
	u32 ctrl; /* 0x0 */
	u32 lock; /* 0x4 */
	u32 cfg; /* 0x8 */
	u32 int_sts; /* 0xc */
	u32 int_mask; /* 0x10 */
	u32 status; /* 0x14 */
	u32 dma_src_addr; /* 0x18 */
	u32 dma_dst_addr; /* 0x1c */
	u32 dma_src_len; /* 0x20 */
	u32 dma_dst_len; /* 0x24 */
	u32 rom_shadow; /* 0x28 */
	u32 reserved1[2];
	u32 unlock; /* 0x34 */
	u32 reserved2[18];
	u32 mctrl; /* 0x80 */
	u32 reserved3;
	u32 write_count; /* 0x88 */
	u32 read_count; /* 0x8c */
};

#define devcfg_base ((struct devcfg_regs *)ZYNQ_DEV_CFG_APB_BASEADDR)

struct scu_regs {
	u32 reserved1[16];
	u32 filter_start; /* 0x40 */
	u32 filter_end; /* 0x44 */
};

#define scu_base ((struct scu_regs *)ZYNQ_SCU_BASEADDR)

struct ddrc_regs {
	u32 ddrc_ctrl; /* 0x0 */
	u32 reserved[60];
	u32 ecc_scrub; /* 0xF4 */
};
#define ddrc_base ((struct ddrc_regs *)ZYNQ_DDRC_BASEADDR)

struct efuse_reg {
	u32 reserved1[4];
	u32 status;
	u32 reserved2[3];
};

#define efuse_base ((struct efuse_reg *)ZYNQ_EFUSE_BASEADDR)

#endif /* _ASM_ARCH_HARDWARE_H */
