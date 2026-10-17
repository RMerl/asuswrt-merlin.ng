// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <common.h>
#include <asm/arch-armada8k/cache_llc.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <dm/device.h>

#define DEVICE_ID_REG			0x7F90004C
#define DEVICE_ID_MASK			0xffff0
#define REV_ID_MASK			0xf
#define DEVICE_ID_OFFSET		4
#define REV_ID_OFFSET			0

#define DEVICE_SAR_REG			0x944F8204

#define DEVICE_ID_SUB_REV		(MVEBU_REGISTER(0x2400230))
#define DEVICE_ID_SUB_REV_OFFSET	7
#define DEVICE_ID_SUB_REV_MASK		(0xffff << DEVICE_ID_SUB_REV_OFFSET)

#define AC5X_DEV_ID			0x9800

struct soc_info {
	u32 dev_id;
	u32 rev_id;
	char *soc_name;
};

static struct soc_info soc_info_table[] = {
	/* Two reserved entries for unidentified devices - don't change */
	{ 0xB4FF, 0x0, "Unidentified Alleycat5"},
	{ 0x98FF, 0x0, "Unidentified Alleycat5x"},

	{ 0xB400, 0x2, "Alleycat5-plus  98DX2538-A2"},
	{ 0xB401, 0x2, "Alleycat5-plus  98DX2535-A2"},
	{ 0xB402, 0x2, "Alleycat5-plus  98DX2532-A2"},
	{ 0xB403, 0x2, "Alleycat5-plus  98DX2531-A2"},
	{ 0xB408, 0x2, "Alleycat5  98DX2528-A2"},
	{ 0xB409, 0x2, "Alleycat5  98DX2525-A2"},
	{ 0xB40A, 0x2, "Alleycat5  98DX2522-A2"},
	{ 0xB40B, 0x2, "Alleycat5  98DX2521-A2"},
	{ 0xB410, 0x2, "Alleycat5-lite  98DX2518-A2"},
	{ 0xB411, 0x2, "Alleycat5-lite  98DX2515-A2"},
	{ 0xB412, 0x2, "Alleycat5-lite  98DX2512-A2"},
	{ 0xB413, 0x2, "Alleycat5-lite  98DX2511-A2"},

	{ 0xB400, 0x1, "Alleycat5-plus  98DX2538-A1"},
	{ 0xB401, 0x1, "Alleycat5-plus  98DX2535-A1"},
	{ 0xB402, 0x1, "Alleycat5-plus  98DX2532-A1"},
	{ 0xB403, 0x1, "Alleycat5-plus  98DX2531-A1"},
	{ 0xB408, 0x1, "Alleycat5  98DX2528-A1"},
	{ 0xB409, 0x1, "Alleycat5  98DX2525-A1"},
	{ 0xB40A, 0x1, "Alleycat5  98DX2522-A1"},
	{ 0xB40B, 0x1, "Alleycat5  98DX2521-A1"},
	{ 0xB410, 0x1, "Alleycat5-lite  98DX2518-A1"},
	{ 0xB411, 0x1, "Alleycat5-lite  98DX2515-A1"},
	{ 0xB412, 0x1, "Alleycat5-lite  98DX2512-A1"},
	{ 0xB413, 0x1, "Alleycat5-lite  98DX2511-A1"},
	{ 0x9800, 0x1, "Alleycat5X 98DX3500M-A1"},
	{ 0x9806, 0x1, "Alleycat5X 98DX3501M-A1"},
	{ 0x9801, 0x1, "Alleycat5X 98DX3510M-A1"},
	{ 0x9802, 0x1, "Alleycat5X 98DX3520M-A1"},
	{ 0x9803, 0x1, "Alleycat5X 98DX3530M-A1"},
	{ 0x9804, 0x1, "Alleycat5X 98DX3540M-A1"},
	{ 0x9805, 0x1, "Alleycat5X 98DX3550M-A1"},
	{ 0x9820, 0x1, "Alleycat5X 98DX3500-A1"},
	{ 0x9826, 0x1, "Alleycat5X 98DX3501-A1"},
	{ 0x9821, 0x1, "Alleycat5X 98DX3510-A1"},
	{ 0x9861, 0x1, "Alleycat5X 98DX3510H-A1"},
	{ 0x9841, 0x1, "Alleycat5X 98DX3510MH-A1"},
	{ 0x9822, 0x1, "Alleycat5X 98DX3520-A1"},
	{ 0x9823, 0x1, "Alleycat5X 98DX3530-A1"},
	{ 0x9863, 0x1, "Alleycat5X 98DX3530H-A1"},
	{ 0x9824, 0x1, "Alleycat5X 98DX3540-A1"},
	{ 0x9825, 0x1, "Alleycat5X 98DX3550-A1"},

	{ 0xB400, 0x0, "Alleycat5-plus  98DX2538-A0"},
	{ 0xB401, 0x0, "Alleycat5-plus  98DX2535-A0"},
	{ 0xB402, 0x0, "Alleycat5-plus  98DX2532-A0"},
	{ 0xB403, 0x0, "Alleycat5-plus  98DX2531-A0"},
	{ 0xB408, 0x0, "Alleycat5  98DX2528-A0"},
	{ 0xB409, 0x0, "Alleycat5  98DX2525-A0"},
	{ 0xB40A, 0x0, "Alleycat5  98DX2522-A0"},
	{ 0xB40B, 0x0, "Alleycat5  98DX2521-A0"},
	{ 0xB410, 0x0, "Alleycat5-lite  98DX2518-A0"},
	{ 0xB411, 0x0, "Alleycat5-lite  98DX2515-A0"},
	{ 0xB412, 0x0, "Alleycat5-lite  98DX2512-A0"},
	{ 0xB413, 0x0, "Alleycat5-lite  98DX2511-A0"},
	{ 0x9800, 0x0, "Alleycat5X 98DX3500M-A0"},
	{ 0x9806, 0x0, "Alleycat5X 98DX3501M-A0"},
	{ 0x9801, 0x0, "Alleycat5X 98DX3510M-A0"},
	{ 0x9802, 0x0, "Alleycat5X 98DX3520M-A0"},
	{ 0x9803, 0x0, "Alleycat5X 98DX3530M-A0"},
	{ 0x9804, 0x0, "Alleycat5X 98DX3540M-A0"},
	{ 0x9805, 0x0, "Alleycat5X 98DX3550M-A0"},
	{ 0x9820, 0x0, "Alleycat5X 98DX3500-A0"},
	{ 0x9826, 0x0, "Alleycat5X 98DX3501-A0"},
	{ 0x9821, 0x0, "Alleycat5X 98DX3510-A0"},
	{ 0x9861, 0x0, "Alleycat5X 98DX3510H-A0"},
	{ 0x9841, 0x0, "Alleycat5X 98DX3510MH-A0"},
	{ 0x9822, 0x0, "Alleycat5X 98DX3520-A0"},
	{ 0x9823, 0x0, "Alleycat5X 98DX3530-A0"},
	{ 0x9863, 0x0, "Alleycat5X 98DX3530H-A0"},
	{ 0x9824, 0x0, "Alleycat5X 98DX3540-A0"},
	{ 0x9825, 0x0, "Alleycat5X 98DX3550-A0"},
};

#define BIT_VAL(b)          ((1ULL << ((b) + 1)) - 1)
#define BIT_RANGE(bl, bh)   (BIT_VAL(bh) - BIT_VAL((bl) - 1))

#define PLL_MAX_CHOICE	4

#define CPU_TYPE_AC5    0
#define CPU_TYPE_AC5x   1
#define CPU_TYPE_LAST   2

enum mvebu_sar_opts {
	SAR_CPU_FREQ = 0,
	SAR_DDR_FREQ,
	SAR_AP_FABRIC_FREQ,
	SAR_CP_FABRIC_FREQ,
	SAR_CP0_PCIE0_CLK,
	SAR_CP0_PCIE1_CLK,
	SAR_CP1_PCIE0_CLK,
	SAR_CP1_PCIE1_CLK,
	SAR_BOOT_SRC,
	SAR_MAX_IDX
};

static const u32 pll_freq_tbl[CPU_TYPE_LAST][SAR_AP_FABRIC_FREQ + 1][PLL_MAX_CHOICE] = {
	[CPU_TYPE_AC5] = {
		[SAR_CPU_FREQ] = {
			800, 1200, 1400, 1000
		},
		[SAR_DDR_FREQ] = {
			1200, 800, 0, 0
		},
		[SAR_AP_FABRIC_FREQ] = {
			396, 290, 197, 0
		},
	},
	[CPU_TYPE_AC5x] = {
		[SAR_CPU_FREQ] = {
			800, 1200, 1500, 1600
		},
		[SAR_DDR_FREQ] = {
			1200, 800, 0, 0
		},
		[SAR_AP_FABRIC_FREQ] = {
			0, 0, 0, 0
		}
	}
};

static const u32 soc_sar_masks_tbl[CPU_TYPE_LAST][SAR_AP_FABRIC_FREQ + 1] = {
	[CPU_TYPE_AC5] = {
		[SAR_CPU_FREQ] = BIT_RANGE(18, 20),
		[SAR_DDR_FREQ] = BIT_RANGE(16, 17),
		[SAR_AP_FABRIC_FREQ] = BIT_RANGE(22, 23),
	},
	[CPU_TYPE_AC5x] = {
		[SAR_CPU_FREQ] = BIT_RANGE(8, 10),
		[SAR_DDR_FREQ] = BIT_RANGE(6, 7),
		[SAR_AP_FABRIC_FREQ] = 1,
	},
};

static int get_soc_type_rev(u32 *type, u32 *rev)
{
	*type = (readl(DEVICE_ID_REG) & DEVICE_ID_MASK) >> DEVICE_ID_OFFSET;
	*rev =  (readl(DEVICE_ID_REG) & REV_ID_MASK)    >> REV_ID_OFFSET;

	return 0;
}

static void get_one_sar_freq(int cpu_type, u32 sar_reg_val, enum mvebu_sar_opts sar_opt, u32 *freq)
{
	u32 mask;
	unsigned char choice;

	mask = soc_sar_masks_tbl[cpu_type][sar_opt];
	choice = (sar_reg_val & mask) >> (__builtin_ffs(mask) - 1);
	*freq = pll_freq_tbl[cpu_type][sar_opt][choice];
}

void get_sar_freq(struct sar_freq_modes *sar_freq)
{
	int cpu_type;
	u32 soc_type, rev;
	u32 sar_reg_val = readl(DEVICE_SAR_REG);

	get_soc_type_rev(&soc_type, &rev);
	cpu_type = (soc_type & 0xFF00) == AC5X_DEV_ID ? CPU_TYPE_AC5x : CPU_TYPE_AC5;

	get_one_sar_freq(cpu_type, sar_reg_val, SAR_CPU_FREQ, &sar_freq->p_clk);
	get_one_sar_freq(cpu_type, sar_reg_val, SAR_AP_FABRIC_FREQ, &sar_freq->nb_clk);
	get_one_sar_freq(cpu_type, sar_reg_val, SAR_DDR_FREQ, &sar_freq->d_clk);
}

static int get_soc_table_index(u32 *index)
{
	u32 soc_type;
	u32 rev, i, ret = 1;

	*index = 0;
	get_soc_type_rev(&soc_type, &rev);

	for (i = 0; i < ARRAY_SIZE(soc_info_table) && ret != 0; i++) {
		if (soc_type != soc_info_table[i].dev_id ||
		    rev != soc_info_table[i].rev_id)
			continue;

		*index = i;
		ret = 0;
	}

	if (ret && ((soc_type & 0xFF00) == AC5X_DEV_ID))
		*index = 1;

	return ret;
}

static int get_soc_name(char **soc_name)
{
	u32 index;

	get_soc_table_index(&index);
	*soc_name = soc_info_table[index].soc_name;

	return 0;
}

/* Print device's SoC name and AP & CP information */
void soc_print_device_info(void)
{
	char *soc_name = NULL;

	get_soc_name(&soc_name);

	printf("SoC: %s\n", soc_name);
}

void soc_print_clock_info(void)
{
	struct sar_freq_modes sar_freq;

	get_sar_freq(&sar_freq);
	printf("Clock:  CPU     %4d MHz\n", sar_freq.p_clk);
	printf("\tDDR     %4d MHz\n", sar_freq.d_clk);
	printf("\tFABRIC  %4d MHz\n", sar_freq.nb_clk);
	printf("\tMSS     %4d MHz\n", 200);
}

/* Return NAND clock in Hz */
u32 mvebu_get_nand_clock(void)
{
	return 400 * 1000000;
}

/*
 * Override of __weak int mach_cpu_init(void) :
 * SoC/machine dependent CPU setup
 */
int mach_cpu_init(void)
{
	u32 phy_i;
	u64 new_val, phy_base = 0x7F080800;

	/* Init USB PHY */
#define USB_STEPPING	0x20000
#define WRITE_MASK(addr, mask, val)		\
	{ new_val = (readl(addr) & (~(mask))) | (val);\
	writel(new_val, addr); }

	for (phy_i = 0; phy_i < 2; phy_i++, phy_base += USB_STEPPING) {
		WRITE_MASK(phy_base + 0x4,     0x3,	   0x2);
		WRITE_MASK(phy_base + 0xC,     0x3000000,   0x2000000);
		WRITE_MASK(phy_base + 0x1C,    0x3,         0x2);
		WRITE_MASK(phy_base + 0x0,     0x1FF007F,   0x600005);
		WRITE_MASK(phy_base + 0xC,     0x000F000,   0x0002000);
		/* Calibration Threshold Setting = 4*/
		WRITE_MASK(phy_base + 0x8,     0x700,	   0x400)
		WRITE_MASK(phy_base + 0x14,    0x000000F,   0x000000a);
		/* Change AMP to 4*/
		WRITE_MASK(phy_base + 0xC,     0x3700000,   0x3400000);
		WRITE_MASK(phy_base + 0x4,     0x3,	   0x3);
		/* Impedance calibration triggering is performed by USB probe */
	}

	return 0;
}
