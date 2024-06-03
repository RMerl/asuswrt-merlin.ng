/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef _KWCPU_H
#define _KWCPU_H

#include <asm/system.h>

#ifndef __ASSEMBLY__

#define KWCPU_WIN_CTRL_DATA(size, target, attr, en) (en | (target << 4) \
			| (attr << 8) | (kw_winctrl_calcsize(size) << 16))

#define KWGBE_PORT_SERIAL_CONTROL1_REG(_x)	\
		((_x ? KW_EGIGA1_BASE : KW_EGIGA0_BASE) + 0x44c)

#define KW_REG_PCIE_DEVID		(KW_REG_PCIE_BASE + 0x00)
#define KW_REG_PCIE_REVID		(KW_REG_PCIE_BASE + 0x08)
#define KW_REG_DEVICE_ID		(KW_MPP_BASE + 0x34)
#define KW_REG_SYSRST_CNT		(KW_MPP_BASE + 0x50)
#define SYSRST_CNT_1SEC_VAL		(25*1000000)
#define KW_REG_MPP_OUT_DRV_REG		(KW_MPP_BASE + 0xE0)

enum memory_bank {
	BANK0,
	BANK1,
	BANK2,
	BANK3
};

enum kwcpu_winen {
	KWCPU_WIN_DISABLE,
	KWCPU_WIN_ENABLE
};

enum kwcpu_target {
	KWCPU_TARGET_RESERVED,
	KWCPU_TARGET_MEMORY,
	KWCPU_TARGET_1RESERVED,
	KWCPU_TARGET_SASRAM,
	KWCPU_TARGET_PCIE
};

enum kwcpu_attrib {
	KWCPU_ATTR_SASRAM = 0x01,
	KWCPU_ATTR_DRAM_CS0 = 0x0e,
	KWCPU_ATTR_DRAM_CS1 = 0x0d,
	KWCPU_ATTR_DRAM_CS2 = 0x0b,
	KWCPU_ATTR_DRAM_CS3 = 0x07,
	KWCPU_ATTR_NANDFLASH = 0x2f,
	KWCPU_ATTR_SPIFLASH = 0x1e,
	KWCPU_ATTR_BOOTROM = 0x1d,
	KWCPU_ATTR_PCIE_IO = 0xe0,
	KWCPU_ATTR_PCIE_MEM = 0xe8
};

/*
 * Default Device Address MAP BAR values
 */
#define KW_DEFADR_PCI_MEM	0x90000000
#define KW_DEFADR_PCI_IO	0xC0000000
#define KW_DEFADR_SASRAM	0xC8010000
#define KW_DEFADR_NANDF		0xD8000000
#define KW_DEFADR_SPIF		0xE8000000
#define KW_DEFADR_BOOTROM	0xF8000000

struct mbus_win {
	u32 base;
	u32 size;
	u8 target;
	u8 attr;
};

/*
 * read feroceon/sheeva core extra feature register
 * using co-proc instruction
 */
static inline unsigned int readfr_extra_feature_reg(void)
{
	unsigned int val;
	asm volatile ("mrc p15, 1, %0, c15, c1, 0 @ readfr exfr":"=r"
			(val)::"cc");
	return val;
}

/*
 * write feroceon/sheeva core extra feature register
 * using co-proc instruction
 */
static inline void writefr_extra_feature_reg(unsigned int val)
{
	asm volatile ("mcr p15, 1, %0, c15, c1, 0 @ writefr exfr"::"r"
			(val):"cc");
	isb();
}

/*
 * MBus-L to Mbus Bridge Registers
 * Ref: Datasheet sec:A.3
 */
struct kwwin_registers {
	u32 ctrl;
	u32 base;
	u32 remap_lo;
	u32 remap_hi;
};

/*
 * CPU control and status Registers
 * Ref: Datasheet sec:A.3.2
 */
struct kwcpu_registers {
	u32 config;	/*0x20100 */
	u32 ctrl_stat;	/*0x20104 */
	u32 rstoutn_mask; /* 0x20108 */
	u32 sys_soft_rst; /* 0x2010C */
	u32 ahb_mbus_cause_irq; /* 0x20110 */
	u32 ahb_mbus_mask_irq; /* 0x20114 */
	u32 pad1[2];
	u32 ftdll_config; /* 0x20120 */
	u32 pad2;
	u32 l2_cfg;	/* 0x20128 */
};

/*
 * GPIO Registers
 * Ref: Datasheet sec:A.19
 */
struct kwgpio_registers {
	u32 dout;
	u32 oe;
	u32 blink_en;
	u32 din_pol;
	u32 din;
	u32 irq_cause;
	u32 irq_mask;
	u32 irq_level;
};

/* Needed for dynamic (board-specific) mbus configuration */
extern struct mvebu_mbus_state mbus_state;

/*
 * functions
 */
unsigned int mvebu_sdram_bar(enum memory_bank bank);
unsigned int mvebu_sdram_bs(enum memory_bank bank);
void mvebu_sdram_size_adjust(enum memory_bank bank);
int mvebu_mbus_probe(struct mbus_win windows[], int count);
void mvebu_config_gpio(unsigned int gpp0_oe_val, unsigned int gpp1_oe_val,
		unsigned int gpp0_oe, unsigned int gpp1_oe);
int kw_config_mpp(unsigned int mpp0_7, unsigned int mpp8_15,
		unsigned int mpp16_23, unsigned int mpp24_31,
		unsigned int mpp32_39, unsigned int mpp40_47,
		unsigned int mpp48_55);
unsigned int kw_winctrl_calcsize(unsigned int sizeval);
#endif /* __ASSEMBLY__ */
#endif /* _KWCPU_H */
