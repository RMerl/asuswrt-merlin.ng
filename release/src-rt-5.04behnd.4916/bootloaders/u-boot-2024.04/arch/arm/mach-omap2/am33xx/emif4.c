// SPDX-License-Identifier: GPL-2.0+
/*
 * emif4.c
 *
 * AM33XX emif4 configuration file
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - https://www.ti.com/
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>

static struct vtp_reg *vtpreg[2] = {
				(struct vtp_reg *)VTP0_CTRL_ADDR,
				(struct vtp_reg *)VTP1_CTRL_ADDR};
#ifdef CONFIG_AM33XX
static struct ddr_ctrl *ddrctrl = (struct ddr_ctrl *)DDR_CTRL_ADDR;
#endif
#ifdef CONFIG_AM43XX
static struct ddr_ctrl *ddrctrl = (struct ddr_ctrl *)DDR_CTRL_ADDR;
static struct cm_device_inst *cm_device =
				(struct cm_device_inst *)CM_DEVICE_INST;
#endif

static void config_vtp(int nr)
{
	writel(readl(&vtpreg[nr]->vtp0ctrlreg) | VTP_CTRL_ENABLE,
			&vtpreg[nr]->vtp0ctrlreg);
	writel(readl(&vtpreg[nr]->vtp0ctrlreg) & (~VTP_CTRL_START_EN),
			&vtpreg[nr]->vtp0ctrlreg);
	writel(readl(&vtpreg[nr]->vtp0ctrlreg) | VTP_CTRL_START_EN,
			&vtpreg[nr]->vtp0ctrlreg);

	/* Poll for READY */
	while ((readl(&vtpreg[nr]->vtp0ctrlreg) & VTP_CTRL_READY) !=
			VTP_CTRL_READY)
		;
}

void __weak ddr_pll_config(unsigned int ddrpll_m)
{
}

void config_ddr(unsigned int pll, const struct ctrl_ioregs *ioregs,
		const struct ddr_data *data, const struct cmd_control *ctrl,
		const struct emif_regs *regs, int nr)
{
	ddr_pll_config(pll);
	config_vtp(nr);
	config_cmd_ctrl(ctrl, nr);

	config_ddr_data(data, nr);
#ifdef CONFIG_AM33XX
	config_io_ctrl(ioregs);

	/* Set CKE to be controlled by EMIF/DDR PHY */
	writel(DDR_CKE_CTRL_NORMAL, &ddrctrl->ddrckectrl);

#endif
#ifdef CONFIG_AM43XX
	writel(readl(&cm_device->cm_dll_ctrl) & ~0x1, &cm_device->cm_dll_ctrl);
	while ((readl(&cm_device->cm_dll_ctrl) & CM_DLL_READYST) == 0)
		;

	config_io_ctrl(ioregs);

	/* Set CKE to be controlled by EMIF/DDR PHY */
	writel(DDR_CKE_CTRL_NORMAL, &ddrctrl->ddrckectrl);

	if (emif_sdram_type(regs->sdram_config) == EMIF_SDRAM_TYPE_DDR3)
#ifndef CONFIG_SPL_RTC_DDR_SUPPORT
		/* Allow EMIF to control DDR_RESET */
		writel(0x00000000, &ddrctrl->ddrioctrl);
#else
		/* Override EMIF DDR_RESET control */
		writel(0x80000000, &ddrctrl->ddrioctrl);
#endif /* CONFIG_SPL_RTC_DDR_SUPPORT */
#endif

	/* Program EMIF instance */
	config_ddr_phy(regs, nr);
	set_sdram_timings(regs, nr);
	if (get_emif_rev(EMIF1_BASE) == EMIF_4D5)
		config_sdram_emif4d5(regs, nr);
	else
		config_sdram(regs, nr);
}
