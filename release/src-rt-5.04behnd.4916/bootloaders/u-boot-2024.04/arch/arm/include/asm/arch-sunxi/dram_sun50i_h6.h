/*
 * H6 dram controller register and constant defines
 *
 * (C) Copyright 2017  Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_DRAM_SUN50I_H6_H
#define _SUNXI_DRAM_SUN50I_H6_H

#include <stdbool.h>
#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

enum sunxi_dram_type {
	SUNXI_DRAM_TYPE_DDR3 = 3,
	SUNXI_DRAM_TYPE_DDR4,
	SUNXI_DRAM_TYPE_LPDDR2 = 6,
	SUNXI_DRAM_TYPE_LPDDR3,
};

static inline bool sunxi_dram_is_lpddr(int type)
{
	return type >= SUNXI_DRAM_TYPE_LPDDR2;
}

/*
 * The following information is mainly retrieved by disassembly and some FPGA
 * test code of sun50iw3 platform.
 */
struct sunxi_mctl_com_reg {
	u32 cr;			/* 0x000 control register */
	u8 reserved_0x004[4];	/* 0x004 */
	u32 unk_0x008;		/* 0x008 */
	u32 tmr;		/* 0x00c timer register */
	u8 reserved_0x010[4];	/* 0x010 */
	u32 unk_0x014;		/* 0x014 */
	u8 reserved_0x018[8];	/* 0x018 */
	u32 maer0;		/* 0x020 master enable register 0 */
	u32 maer1;		/* 0x024 master enable register 1 */
	u32 maer2;		/* 0x028 master enable register 2 */
	u8 reserved_0x02c[468];	/* 0x02c */
	u32 bwcr;		/* 0x200 bandwidth control register */
	u8 reserved_0x204[12];	/* 0x204 */
	/*
	 * The last master configured by BSP libdram is at 0x49x, so the
	 * size of this struct array is set to 41 (0x29) now.
	 */
	struct {
		u32 cfg0;		/* 0x0 */
		u32 cfg1;		/* 0x4 */
		u8 reserved_0x8[8];	/* 0x8 */
	} master[41];		/* 0x210 + index * 0x10 */
};
check_member(sunxi_mctl_com_reg, master[40].reserved_0x8, 0x498);

/*
 * The following register information are retrieved from some similar DRAM
 * controllers, including the DRAM controllers in Allwinner A23/A80 SoCs,
 * Rockchip RK3328 SoC, NXP i.MX7 SoCs and Xilinx Zynq UltraScale+ SoCs.
 *
 * The DRAM controller in Allwinner A23/A80 SoCs and NXP i.MX7 SoCs seems
 * to be older than the one in Allwinner H6, as the DRAMTMG9 register
 * is missing in these SoCs. (From the product specifications of these
 * SoCs they're not capable of DDR4)
 *
 * Information sources:
 * - dram_sun9i.h and dram_sun8i_a23.h in the same directory.
 * - sdram_rk3328.h from the RK3328 TPL DRAM patchset
 * - i.MX 7Solo Applications Processor Reference Manual (IMX7SRM)
 * - Zynq UltraScale+ MPSoC Register Reference (UG1087)
 */
struct sunxi_mctl_ctl_reg {
	u32 mstr;		/* 0x000 */
	u32 statr;		/* 0x004 unused */
	u32 mstr1;		/* 0x008 unused */
	u32 unk_0x00c;		/* 0x00c */
	u32 mrctrl0;		/* 0x010 unused */
	u32 mrctrl1;		/* 0x014 unused */
	u32 mrstatr;		/* 0x018 unused */
	u32 mrctrl2;		/* 0x01c unused */
	u32 derateen;		/* 0x020 unused */
	u32 derateint;		/* 0x024 unused */
	u8 reserved_0x028[8];	/* 0x028 */
	u32 pwrctl;		/* 0x030 unused */
	u32 pwrtmg;		/* 0x034 unused */
	u32 hwlpctl;		/* 0x038 unused */
	u8 reserved_0x03c[20];	/* 0x03c */
	u32 rfshctl0;		/* 0x050 unused */
	u32 rfshctl1;		/* 0x054 unused */
	u8 reserved_0x058[8];	/* 0x05c */
	u32 rfshctl3;		/* 0x060 */
	u32 rfshtmg;		/* 0x064 */
	u8 reserved_0x068[104];	/* 0x068 reserved for ECC&CRC (from ZynqMP) */
	u32 init[8];		/* 0x0d0 */
	u32 dimmctl;		/* 0x0f0 unused */
	u32 rankctl;		/* 0x0f4 */
	u8 reserved_0x0f8[8];	/* 0x0f8 */
	u32 dramtmg[17];	/* 0x100 */
	u8 reserved_0x144[60];	/* 0x144 */
	u32 zqctl[3];		/* 0x180 */
	u32 zqstat;		/* 0x18c unused */
	u32 dfitmg0;		/* 0x190 */
	u32 dfitmg1;		/* 0x194 */
	u32 dfilpcfg[2];	/* 0x198 unused */
	u32 dfiupd[3];		/* 0x1a0 */
	u32 reserved_0x1ac;	/* 0x1ac */
	u32 dfimisc;		/* 0x1b0 */
	u32 dfitmg2;		/* 0x1b4 unused, may not exist */
	u8 reserved_0x1b8[8];	/* 0x1b8 */
	u32 dbictl;		/* 0x1c0 */
	u8 reserved_0x1c4[60];	/* 0x1c4 */
	u32 addrmap[12];	/* 0x200 */
	u8 reserved_0x230[16];	/* 0x230 */
	u32 odtcfg;		/* 0x240 */
	u32 odtmap;		/* 0x244 */
	u8 reserved_0x248[8];	/* 0x248 */
	u32 sched[2];		/* 0x250 */
	u8 reserved_0x258[180];	/* 0x258 */
	u32 dbgcmd;		/* 0x30c unused */
	u32 dbgstat;		/* 0x310 unused */
	u8 reserved_0x314[12];	/* 0x314 */
	u32 swctl;		/* 0x320 */
	u32 swstat;		/* 0x324 */
};
check_member(sunxi_mctl_ctl_reg, swstat, 0x324);

#define MSTR_DEVICETYPE_DDR3	BIT(0)
#define MSTR_DEVICETYPE_LPDDR2	BIT(2)
#define MSTR_DEVICETYPE_LPDDR3	BIT(3)
#define MSTR_DEVICETYPE_DDR4	BIT(4)
#define MSTR_DEVICETYPE_MASK	GENMASK(5, 0)
#define MSTR_2TMODE		BIT(10)
#define MSTR_BUSWIDTH_FULL	(0 << 12)
#define MSTR_BUSWIDTH_HALF	(1 << 12)
#define MSTR_ACTIVE_RANKS(x)	(((x == 2) ? 3 : 1) << 24)
#define MSTR_BURST_LENGTH(x)	(((x) >> 1) << 16)

/*
 * The following register information is based on Zynq UltraScale+
 * MPSoC Register Reference, as it's the currently only known
 * DDR PHY similar to the one used in H6; however although the
 * map is similar, the bit fields definitions are different.
 *
 * Other DesignWare DDR PHY's have similar register names, but the
 * offset and definitions are both different.
 */
struct sunxi_mctl_phy_reg {
	u32 ver;		/* 0x000 guess based on similar PHYs */
	u32 pir;		/* 0x004 */
	u8 reserved_0x008[8];	/* 0x008 */
	/*
	 * The ZynqMP manual didn't document PGCR1, however this register
	 * exists on H6 and referenced by libdram.
	 */
	u32 pgcr[8];		/* 0x010 */
	/*
	 * By comparing the hardware and the ZynqMP manual, the PGSR seems
	 * to start at 0x34 on H6.
	 */
	u8 reserved_0x030[4];	/* 0x030 */
	u32 pgsr[3];		/* 0x034 */
	u32 ptr[7];		/* 0x040 */
	/*
	 * According to ZynqMP reference there's PLLCR0~6 in this area,
	 * but they're tagged "Type B PLL Only" and H6 seems to have
	 * no them.
	 * 0x080 is not present in ZynqMP reference but it seems to be
	 * present on H6.
	 */
	u8 reserved_0x05c[36];	/* 0x05c */
	u32 unk_0x080;		/* 0x080 */
	u8 reserved_0x084[4];	/* 0x084 */
	u32 dxccr;		/* 0x088 */
	u8 reserved_0x08c[4];	/* 0x08c */
	u32 dsgcr;		/* 0x090 */
	u8 reserved_0x094[4];	/* 0x094 */
	u32 odtcr;		/* 0x098 */
	u8 reserved_0x09c[4];	/* 0x09c */
	u32 aacr;		/* 0x0a0 */
	u8 reserved_0x0a4[32];	/* 0x0a4 */
	u32 gpr1;		/* 0x0c4 */
	u8 reserved_0x0c8[56];	/* 0x0c8 */
	u32 dcr;		/* 0x100 */
	u8 reserved_0x104[12];	/* 0x104 */
	u32 dtpr[7];		/* 0x110 */
	u8 reserved_0x12c[20];	/* 0x12c */
	u32 rdimmgcr[3];	/* 0x140 */
	u8 reserved_0x14c[4];	/* 0x14c */
	u32 rdimmcr[5];		/* 0x150 */
	u8 reserved_0x164[4];	/* 0x164 */
	u32 schcr[2];		/* 0x168 */
	u8 reserved_0x170[16];	/* 0x170 */
	/*
	 * The ZynqMP manual documents MR0~7, 11~14 and 22.
	 */
	u32 mr[23];		/* 0x180 */
	u8 reserved_0x1dc[36];	/* 0x1dc */
	u32 dtcr[2];		/* 0x200 */
	u32 dtar[3];		/* 0x208 */
	u8 reserved_0x214[4];	/* 0x214 */
	u32 dtdr[2];		/* 0x218 */
	u8 reserved_0x220[16];	/* 0x220 */
	u32 dtedr0;		/* 0x230 */
	u32 dtedr1;		/* 0x234 */
	u32 dtedr2;		/* 0x238 */
	u32 vtdr;		/* 0x23c */
	u32 catr[2];		/* 0x240 */
	u8 reserved_0x248[8];
	u32 dqsdr[3];		/* 0x250 */
	u32 dtedr3;		/* 0x25c */
	u8 reserved_0x260[160];	/* 0x260 */
	u32 dcuar;		/* 0x300 */
	u32 dcudr;		/* 0x304 */
	u32 dcurr;		/* 0x308 */
	u32 dculr;		/* 0x30c */
	u32 dcugcr;		/* 0x310 */
	u32 dcutpr;		/* 0x314 */
	u32 dcusr[2];		/* 0x318 */
	u8 reserved_0x320[444];	/* 0x320 */
	u32 rankidr;		/* 0x4dc */
	u32 riocr[6];		/* 0x4e0 */
	u8 reserved_0x4f8[8];	/* 0x4f8 */
	u32 aciocr[6];		/* 0x500 */
	u8 reserved_0x518[8];	/* 0x518 */
	u32 iovcr[2];		/* 0x520 */
	u32 vtcr[2];		/* 0x528 */
	u8 reserved_0x530[16];	/* 0x530 */
	u32 acbdlr[17];		/* 0x540 */
	u32 aclcdlr;		/* 0x584 */
	u8 reserved_0x588[24];	/* 0x588 */
	u32 acmdlr[2];		/* 0x5a0 */
	u8 reserved_0x5a8[216];	/* 0x5a8 */
	struct {
		u32 zqcr;	/* 0x00 only the first one valid */
		u32 zqpr[2];	/* 0x04 */
		u32 zqdr[2];	/* 0x0c */
		u32 zqor[2];	/* 0x14 */
		u32 zqsr;	/* 0x1c */
	} zq[2];		/* 0x680, 0x6a0 */
	u8 reserved_0x6c0[64];	/* 0x6c0 */
	struct {
		u32 gcr[7];		/* 0x00 */
		u8 reserved_0x1c[36];	/* 0x1c */
		u32 bdlr0;		/* 0x40 */
		u32 bdlr1;		/* 0x44 */
		u32 bdlr2;		/* 0x48 */
		u8 reserved_0x4c[4];	/* 0x4c */
		u32 bdlr3;		/* 0x50 */
		u32 bdlr4;		/* 0x54 */
		u32 bdlr5;		/* 0x58 */
		u8 reserved_0x5c[4];	/* 0x5c */
		u32 bdlr6;		/* 0x60 */
		u8 reserved_0x64[28];	/* 0x64 */
		u32 lcdlr[6];		/* 0x80 */
		u8 reserved_0x98[8];	/* 0x98 */
		u32 mdlr[2];		/* 0xa0 */
		u8 reserved_0xa8[24];	/* 0xa8 */
		u32 gtr0;		/* 0xc0 */
		u8 reserved_0xc4[12];	/* 0xc4 */
		/*
		 * DXnRSR0 is not documented in ZynqMP manual but
		 * it's used in libdram.
		 */
		u32 rsr[4];		/* 0xd0 */
		u32 gsr[4];		/* 0xe0 */
		u8 reserved_0xf0[16];	/* 0xf0 */
	} dx[4];		/* 0x700, 0x800, 0x900, 0xa00 */
};
check_member(sunxi_mctl_phy_reg, dx[3].reserved_0xf0, 0xaf0);

#define PIR_INIT	BIT(0)
#define PIR_ZCAL	BIT(1)
#define PIR_CA		BIT(2)
#define PIR_PLLINIT	BIT(4)
#define PIR_DCAL	BIT(5)
#define PIR_PHYRST	BIT(6)
#define PIR_DRAMRST	BIT(7)
#define PIR_DRAMINIT	BIT(8)
#define PIR_WL		BIT(9)
#define PIR_QSGATE	BIT(10)
#define PIR_WLADJ	BIT(11)
#define PIR_RDDSKW	BIT(12)
#define PIR_WRDSKW	BIT(13)
#define PIR_RDEYE	BIT(14)
#define PIR_WREYE	BIT(15)
#define PIR_VREF	BIT(17)
#define PIR_CTLDINIT	BIT(18)
#define PIR_DQS2DQ	BIT(20)
#define PIR_DCALPSE	BIT(29)
#define PIR_ZCALBYP	BIT(30)

#define DCR_LPDDR3	(1 << 0)
#define DCR_DDR3	(3 << 0)
#define DCR_DDR4	(4 << 0)
#define DCR_DDR8BANK	BIT(3)
#define DCR_DDR2T	BIT(28)

/*
 * The delay parameters allow to allegedly specify delay times of some
 * unknown unit for each individual bit trace in each of the four data bytes
 * the 32-bit wide access consists of. Also three control signals can be
 * adjusted individually.
 */
#define NR_OF_BYTE_LANES	(32 / BITS_PER_BYTE)
/* The eight data lines (DQn) plus DM, DQS, DQS/DM/DQ Output Enable and DQSN */
#define WR_LINES_PER_BYTE_LANE	(BITS_PER_BYTE + 4)
/*
 * The eight data lines (DQn) plus DM, DQS, DQS/DM/DQ Output Enable, DQSN,
 * Termination and Power down
 */
#define RD_LINES_PER_BYTE_LANE	(BITS_PER_BYTE + 6)
struct dram_para {
	u32 clk;
	enum sunxi_dram_type type;
	u8 cols;
	u8 rows;
	u8 ranks;
	u8 bus_full_width;
	const u8 dx_read_delays[NR_OF_BYTE_LANES][RD_LINES_PER_BYTE_LANE];
	const u8 dx_write_delays[NR_OF_BYTE_LANES][WR_LINES_PER_BYTE_LANE];
};


static inline int ns_to_t(int nanoseconds)
{
	const unsigned int ctrl_freq = CONFIG_DRAM_CLK / 2;

	return DIV_ROUND_UP(ctrl_freq * nanoseconds, 1000);
}

void mctl_set_timing_params(struct dram_para *para);

#endif /* _SUNXI_DRAM_SUN50I_H6_H */
