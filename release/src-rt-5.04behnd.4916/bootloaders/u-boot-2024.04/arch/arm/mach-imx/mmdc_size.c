// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <asm/io.h>

#if defined(CONFIG_MX53)
#define MEMCTL_BASE	ESDCTL_BASE_ADDR
#elif defined(CONFIG_MX6)
#define MEMCTL_BASE	MMDC_P0_BASE_ADDR
#elif defined(CONFIG_MX7ULP)
#define MEMCTL_BASE	MMDC0_RBASE
#endif
static const unsigned char col_lookup[] = {9, 10, 11, 8, 12, 9, 9, 9};
static const unsigned char bank_lookup[] = {3, 2};

/* these MMDC registers are common to the IMX53 and IMX6 */
struct esd_mmdc_regs {
	u32 ctl;
	u32 pdc;
	u32 otc;
	u32 cfg0;
	u32 cfg1;
	u32 cfg2;
	u32 misc;
};

#define ESD_MMDC_CTL_GET_ROW(mdctl)    ((mdctl >> 24) & 7)
#define ESD_MMDC_CTL_GET_COLUMN(mdctl) ((mdctl >> 20) & 7)
#define ESD_MMDC_CTL_GET_WIDTH(mdctl)  ((mdctl >> 16) & 3)
#define ESD_MMDC_CTL_GET_CS1(mdctl)    ((mdctl >> 30) & 1)
#define ESD_MMDC_MISC_GET_BANK(mdmisc) ((mdmisc >> 5) & 1)

/*
 * imx_ddr_size - return size in bytes of DRAM according MMDC config
 * The MMDC MDCTL register holds the number of bits for row, col, and data
 * width and the MMDC MDMISC register holds the number of banks. Combine
 * all these bits to determine the meme size the MMDC has been configured for
 */
unsigned int imx_ddr_size(void)
{
	struct esd_mmdc_regs *mem = (struct esd_mmdc_regs *)MEMCTL_BASE;
	unsigned int ctl = readl(&mem->ctl);
	unsigned int misc = readl(&mem->misc);
	int bits = 11 + 0 + 0 + 1;      /* row + col + bank + width */

	bits += ESD_MMDC_CTL_GET_ROW(ctl);
	bits += col_lookup[ESD_MMDC_CTL_GET_COLUMN(ctl)];
	bits += bank_lookup[ESD_MMDC_MISC_GET_BANK(misc)];
	bits += ESD_MMDC_CTL_GET_WIDTH(ctl);
	bits += ESD_MMDC_CTL_GET_CS1(ctl);

	/* The MX6 can do only 3840 MiB of DRAM */
	if (bits == 32)
		return 0xf0000000;

	return 1 << bits;
}
