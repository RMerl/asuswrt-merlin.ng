// SPDX-License-Identifier: GPL-2.0

/*
 * Microcode patches for the CPM as supplied by Motorola.
 */

#include <linux/string.h>
#include <linux/io.h>
#include <asm/immap_8xx.h>
#include <asm/cpm_8xx.h>

/*
 *  USB SOF patch arrays.
 */
static uint patch_2000[] = {
	0x7fff0000, 0x7ffd0000, 0x7ffb0000, 0x49f7ba5b,
	0xba383ffb, 0xf9b8b46d, 0xe5ab4e07, 0xaf77bffe,
	0x3f7bbf79, 0xba5bba38, 0xe7676076, 0x60750000
};

static uint patch_2f00[] = {
	0x3030304c, 0xcab9e441, 0xa1aaf220
};

void cpm_load_patch(cpm8xx_t __iomem *cp)
{
	out_be16(&cp->cp_rccr, 0);

	memcpy_toio(cp->cp_dpmem, patch_2000, sizeof(patch_2000));
	memcpy_toio(cp->cp_dpmem + 0xf00, patch_2f00, sizeof(patch_2f00));

	out_be16(&cp->cp_cpmcr1, 0);
	out_be16(&cp->cp_cpmcr2, 0);
	out_be16(&cp->cp_cpmcr3, 0);
	out_be16(&cp->cp_cpmcr4, 0);

	out_be16(&cp->cp_rccr, 9);
}
