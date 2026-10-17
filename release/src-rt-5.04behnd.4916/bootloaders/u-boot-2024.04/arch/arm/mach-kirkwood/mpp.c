/*
 * arch/arm/mach-kirkwood/mpp.c
 *
 * MPP functions for Marvell Kirkwood SoCs
 * Referenced from Linux kernel source
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <common.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>

static u32 kirkwood_variant(void)
{
	switch (readl(KW_REG_DEVICE_ID) & 0x03) {
	case 1:
		return MPP_F6192_MASK;
	case 2:
		return MPP_F6281_MASK;
	default:
		debug("MPP setup: unknown kirkwood variant\n");
		return 0;
	}
}

#define MPP_CTRL(i)	(KW_MPP_BASE + (i* 4))
#define MPP_NR_REGS	(1 + MPP_MAX/8)

void kirkwood_mpp_conf(const u32 *mpp_list, u32 *mpp_save)
{
	u32 mpp_ctrl[MPP_NR_REGS];
	unsigned int variant_mask;
	int i;

	variant_mask = kirkwood_variant();
	if (!variant_mask)
		return;

	debug( "initial MPP regs:");
	for (i = 0; i < MPP_NR_REGS; i++) {
		mpp_ctrl[i] = readl(MPP_CTRL(i));
		debug(" %08x", mpp_ctrl[i]);
	}
	debug("\n");


	while (*mpp_list) {
		unsigned int num = MPP_NUM(*mpp_list);
		unsigned int sel = MPP_SEL(*mpp_list);
		unsigned int sel_save;
		int shift;

		if (num > MPP_MAX) {
			debug("kirkwood_mpp_conf: invalid MPP "
					"number (%u)\n", num);
			continue;
		}
		if (!(*mpp_list & variant_mask)) {
			debug("kirkwood_mpp_conf: requested MPP%u config "
				"unavailable on this hardware\n", num);
			continue;
		}

		shift = (num & 7) << 2;

		if (mpp_save) {
			sel_save = (mpp_ctrl[num / 8] >> shift) & 0xf;
			*mpp_save = num | (sel_save << 8) | variant_mask;
			mpp_save++;
		}

		mpp_ctrl[num / 8] &= ~(0xf << shift);
		mpp_ctrl[num / 8] |= sel << shift;

		mpp_list++;
	}

	debug("  final MPP regs:");
	for (i = 0; i < MPP_NR_REGS; i++) {
		writel(mpp_ctrl[i], MPP_CTRL(i));
		debug(" %08x", mpp_ctrl[i]);
	}
	debug("\n");

}
