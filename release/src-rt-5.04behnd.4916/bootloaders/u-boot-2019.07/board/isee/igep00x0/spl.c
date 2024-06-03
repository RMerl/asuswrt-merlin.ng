// SPDX-License-Identifier: GPL-2.0+

#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <jffs2/load_kernel.h>
#include <linux/mtd/rawnand.h>
#include "igep00x0.h"

/*
 * Routine: get_board_mem_timings
 * Description: If we use SPL then there is no x-loader nor config header
 * so we have to setup the DDR timings ourself on both banks.
 */
void get_board_mem_timings(struct board_sdrc_timings *timings)
{
	int mfr, id, err = identify_nand_chip(&mfr, &id);

	timings->mr = MICRON_V_MR_165;
	if (!err) {
		switch (mfr) {
		case NAND_MFR_HYNIX:
			timings->mcfg = HYNIX_V_MCFG_200(256 << 20);
			timings->ctrla = HYNIX_V_ACTIMA_200;
			timings->ctrlb = HYNIX_V_ACTIMB_200;
			break;
		case NAND_MFR_MICRON:
			timings->mcfg = MICRON_V_MCFG_200(256 << 20);
			timings->ctrla = MICRON_V_ACTIMA_200;
			timings->ctrlb = MICRON_V_ACTIMB_200;
			break;
		default:
			/* Should not happen... */
			break;
		}
		timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_200MHz;
		gpmc_cs0_flash = MTD_DEV_TYPE_NAND;
	} else {
		if (get_cpu_family() == CPU_OMAP34XX) {
			timings->mcfg = NUMONYX_V_MCFG_165(256 << 20);
			timings->ctrla = NUMONYX_V_ACTIMA_165;
			timings->ctrlb = NUMONYX_V_ACTIMB_165;
			timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
		} else {
			timings->mcfg = NUMONYX_V_MCFG_200(256 << 20);
			timings->ctrla = NUMONYX_V_ACTIMA_200;
			timings->ctrlb = NUMONYX_V_ACTIMB_200;
			timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_200MHz;
		}
		gpmc_cs0_flash = MTD_DEV_TYPE_ONENAND;
	}
}

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	/* break into full u-boot on 'c' */
	if (serial_tstc() && serial_getc() == 'c')
		return 1;

	return 0;
}
#endif
