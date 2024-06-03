// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <axp_pmic.h>
#include <errno.h>

#ifdef CONFIG_MACH_SUN6I
int sunxi_get_ss_bonding_id(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	static int bonding_id = -1;

	if (bonding_id != -1)
		return bonding_id;

	/* Enable Security System */
	setbits_le32(&ccm->ahb_reset0_cfg, 1 << AHB_RESET_OFFSET_SS);
	setbits_le32(&ccm->ahb_gate0, 1 << AHB_GATE_OFFSET_SS);

	bonding_id = readl(SUNXI_SS_BASE);
	bonding_id = (bonding_id >> 16) & 0x7;

	/* Disable Security System again */
	clrbits_le32(&ccm->ahb_gate0, 1 << AHB_GATE_OFFSET_SS);
	clrbits_le32(&ccm->ahb_reset0_cfg, 1 << AHB_RESET_OFFSET_SS);

	return bonding_id;
}
#endif

#ifdef CONFIG_MACH_SUN8I
uint sunxi_get_sram_id(void)
{
	uint id;

	/* Unlock sram info reg, read it, relock */
	setbits_le32(SUNXI_SRAMC_BASE + 0x24, (1 << 15));
	id = readl(SUNXI_SRAMC_BASE + 0x24) >> 16;
	clrbits_le32(SUNXI_SRAMC_BASE + 0x24, (1 << 15));

	return id;
}
#endif

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
#ifdef CONFIG_MACH_SUN4I
	puts("CPU:   Allwinner A10 (SUN4I)\n");
#elif defined CONFIG_MACH_SUN5I
	u32 val = readl(SUNXI_SID_BASE + 0x08);
	switch ((val >> 12) & 0xf) {
	case 0: puts("CPU:   Allwinner A12 (SUN5I)\n"); break;
	case 3: puts("CPU:   Allwinner A13 (SUN5I)\n"); break;
	case 7: puts("CPU:   Allwinner A10s (SUN5I)\n"); break;
	default: puts("CPU:   Allwinner A1X (SUN5I)\n");
	}
#elif defined CONFIG_MACH_SUN6I
	switch (sunxi_get_ss_bonding_id()) {
	case SUNXI_SS_BOND_ID_A31:
		puts("CPU:   Allwinner A31 (SUN6I)\n");
		break;
	case SUNXI_SS_BOND_ID_A31S:
		puts("CPU:   Allwinner A31s (SUN6I)\n");
		break;
	default:
		printf("CPU:   Allwinner A31? (SUN6I, id: %d)\n",
		       sunxi_get_ss_bonding_id());
	}
#elif defined CONFIG_MACH_SUN7I
	puts("CPU:   Allwinner A20 (SUN7I)\n");
#elif defined CONFIG_MACH_SUN8I_A23
	printf("CPU:   Allwinner A23 (SUN8I %04x)\n", sunxi_get_sram_id());
#elif defined CONFIG_MACH_SUN8I_A33
	printf("CPU:   Allwinner A33 (SUN8I %04x)\n", sunxi_get_sram_id());
#elif defined CONFIG_MACH_SUN8I_A83T
	printf("CPU:   Allwinner A83T (SUN8I %04x)\n", sunxi_get_sram_id());
#elif defined CONFIG_MACH_SUN8I_H3
	printf("CPU:   Allwinner H3 (SUN8I %04x)\n", sunxi_get_sram_id());
#elif defined CONFIG_MACH_SUN8I_R40
	printf("CPU:   Allwinner R40 (SUN8I %04x)\n", sunxi_get_sram_id());
#elif defined CONFIG_MACH_SUN8I_V3S
	printf("CPU:   Allwinner V3s (SUN8I %04x)\n", sunxi_get_sram_id());
#elif defined CONFIG_MACH_SUN9I
	puts("CPU:   Allwinner A80 (SUN9I)\n");
#elif defined CONFIG_MACH_SUN50I
	puts("CPU:   Allwinner A64 (SUN50I)\n");
#elif defined CONFIG_MACH_SUN50I_H5
	puts("CPU:   Allwinner H5 (SUN50I)\n");
#elif defined CONFIG_MACH_SUN50I_H6
	puts("CPU:   Allwinner H6 (SUN50I)\n");
#else
#warning Please update cpu_info.c with correct CPU information
	puts("CPU:   SUNXI Family\n");
#endif
	return 0;
}
#endif

#ifdef CONFIG_MACH_SUN8I_H3

#define SIDC_PRCTL 0x40
#define SIDC_RDKEY 0x60

#define SIDC_OP_LOCK 0xAC

uint32_t sun8i_efuse_read(uint32_t offset)
{
	uint32_t reg_val;

	reg_val = readl(SUNXI_SIDC_BASE + SIDC_PRCTL);
	reg_val &= ~(((0x1ff) << 16) | 0x3);
	reg_val |= (offset << 16);
	writel(reg_val, SUNXI_SIDC_BASE + SIDC_PRCTL);

	reg_val &= ~(((0xff) << 8) | 0x3);
	reg_val |= (SIDC_OP_LOCK << 8) | 0x2;
	writel(reg_val, SUNXI_SIDC_BASE + SIDC_PRCTL);

	while (readl(SUNXI_SIDC_BASE + SIDC_PRCTL) & 0x2);

	reg_val &= ~(((0x1ff) << 16) | ((0xff) << 8) | 0x3);
	writel(reg_val, SUNXI_SIDC_BASE + SIDC_PRCTL);

	reg_val = readl(SUNXI_SIDC_BASE + SIDC_RDKEY);
	return reg_val;
}
#endif

int sunxi_get_sid(unsigned int *sid)
{
#ifdef CONFIG_AXP221_POWER
	return axp_get_sid(sid);
#elif defined CONFIG_MACH_SUN8I_H3
	/*
	 * H3 SID controller has a bug, which makes the initial value of
	 * SUNXI_SID_BASE at boot wrong.
	 * Read the value directly from SID controller, in order to get
	 * the correct value, and also refresh the wrong value at
	 * SUNXI_SID_BASE.
	 */
	int i;

	for (i = 0; i< 4; i++)
		sid[i] = sun8i_efuse_read(i * 4);

	return 0;
#elif defined SUNXI_SID_BASE
	int i;

	for (i = 0; i< 4; i++)
		sid[i] = readl((ulong)SUNXI_SID_BASE + 4 * i);

	return 0;
#else
	return -ENODEV;
#endif
}
