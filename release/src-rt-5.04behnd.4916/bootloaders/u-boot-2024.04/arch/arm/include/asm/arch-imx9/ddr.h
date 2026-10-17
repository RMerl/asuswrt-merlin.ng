/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 NXP
 */

#ifndef __ASM_ARCH_IMX8M_DDR_H
#define __ASM_ARCH_IMX8M_DDR_H

#include <asm/io.h>
#include <asm/types.h>

#define DDR_CTL_BASE			0x4E300000
#define DDR_PHY_BASE			0x4E100000
#define DDRMIX_BLK_CTRL_BASE		0x4E010000

#define REG_DDR_SDRAM_MD_CNTL	(DDR_CTL_BASE + 0x120)
#define REG_DDR_CS0_BNDS        (DDR_CTL_BASE + 0x0)
#define REG_DDR_CS1_BNDS        (DDR_CTL_BASE + 0x8)
#define REG_DDRDSR_2			(DDR_CTL_BASE + 0xB24)
#define REG_DDR_TIMING_CFG_0	(DDR_CTL_BASE + 0x104)
#define REG_DDR_SDRAM_CFG		(DDR_CTL_BASE + 0x110)
#define REG_DDR_TIMING_CFG_4	(DDR_CTL_BASE + 0x160)
#define REG_DDR_DEBUG_19		(DDR_CTL_BASE + 0xF48)
#define REG_DDR_SDRAM_CFG_3	(DDR_CTL_BASE + 0x260)
#define REG_DDR_SDRAM_CFG_4	(DDR_CTL_BASE + 0x264)
#define REG_DDR_SDRAM_MD_CNTL_2	(DDR_CTL_BASE + 0x270)
#define REG_DDR_SDRAM_MPR4	(DDR_CTL_BASE + 0x28C)
#define REG_DDR_SDRAM_MPR5	(DDR_CTL_BASE + 0x290)

#define REG_DDR_ERR_EN		(DDR_CTL_BASE + 0x1000)

#define SRC_BASE_ADDR			(0x44460000)
#define SRC_DPHY_BASE_ADDR		(SRC_BASE_ADDR + 0x1400)
#define REG_SRC_DPHY_SW_CTRL		(SRC_DPHY_BASE_ADDR + 0x20)
#define REG_SRC_DPHY_SINGLE_RESET_SW_CTRL	(SRC_DPHY_BASE_ADDR + 0x24)

#define IP2APB_DDRPHY_IPS_BASE_ADDR(X)	(DDR_PHY_BASE + ((X) * 0x2000000))
#define DDRPHY_MEM(X)			(DDR_PHY_BASE + ((X) * 0x2000000) + 0x50000)

/* PHY State */
enum pstate {
	PS0,
	PS1,
	PS2,
	PS3,
};

enum msg_response {
	TRAIN_SUCCESS = 0x7,
	TRAIN_STREAM_START = 0x8,
	TRAIN_FAIL = 0xff,
};

/* user data type */
enum fw_type {
	FW_1D_IMAGE,
	FW_2D_IMAGE,
};

struct dram_cfg_param {
	unsigned int reg;
	unsigned int val;
};

struct dram_fsp_cfg {
	struct dram_cfg_param ddrc_cfg[20];
	struct dram_cfg_param mr_cfg[10];
	unsigned int bypass;
};

struct dram_fsp_msg {
	unsigned int drate;
	enum fw_type fw_type;
	struct dram_cfg_param *fsp_cfg;
	unsigned int fsp_cfg_num;
};

struct dram_timing_info {
	/* umctl2 config */
	struct dram_cfg_param *ddrc_cfg;
	unsigned int ddrc_cfg_num;
	/* fsp config */
	struct dram_fsp_cfg *fsp_cfg;
	unsigned int fsp_cfg_num;
	/* ddrphy config */
	struct dram_cfg_param *ddrphy_cfg;
	unsigned int ddrphy_cfg_num;
	/* ddr fsp train info */
	struct dram_fsp_msg *fsp_msg;
	unsigned int fsp_msg_num;
	/* ddr phy trained CSR */
	struct dram_cfg_param *ddrphy_trained_csr;
	unsigned int ddrphy_trained_csr_num;
	/* ddr phy PIE */
	struct dram_cfg_param *ddrphy_pie;
	unsigned int ddrphy_pie_num;
	/* initialized drate table */
	unsigned int fsp_table[4];
};

extern struct dram_timing_info dram_timing;

void ddr_load_train_firmware(enum fw_type type);
int ddr_init(struct dram_timing_info *timing_info);
int ddr_cfg_phy(struct dram_timing_info *timing_info);
void load_lpddr4_phy_pie(void);
void ddrphy_trained_csr_save(struct dram_cfg_param *param, unsigned int num);
void *dram_config_save(struct dram_timing_info *info, unsigned long base);
void board_dram_ecc_scrub(void);
void ddrc_inline_ecc_scrub(unsigned int start_address,
			   unsigned int range_address);
void ddrc_inline_ecc_scrub_end(unsigned int start_address,
			       unsigned int range_address);

/* utils function for ddr phy training */
int wait_ddrphy_training_complete(void);
void ddrphy_init_set_dfi_clk(unsigned int drate);
void ddrphy_init_read_msg_block(enum fw_type type);

void get_trained_CDD(unsigned int fsp);

ulong ddrphy_addr_remap(u32 paddr_apb_from_ctlr);

static inline void reg32_write(unsigned long addr, u32 val)
{
	writel(val, addr);
}

static inline u32 reg32_read(unsigned long addr)
{
	return readl(addr);
}

static inline void reg32setbit(unsigned long addr, u32 bit)
{
	setbits_le32(addr, (1 << bit));
}

#define dwc_ddrphy_apb_wr(addr, data) \
	reg32_write(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + ddrphy_addr_remap(addr), data)
#define dwc_ddrphy_apb_rd(addr) \
	reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + ddrphy_addr_remap(addr))

extern struct dram_cfg_param ddrphy_trained_csr[];
extern u32 ddrphy_trained_csr_num;

#endif
