// SPDX-License-Identifier: GPL-2.0+
/*
 * DDR Configuration for AM33xx devices.
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 */

#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>

/**
 * Base address for EMIF instances
 */
static struct emif_reg_struct *emif_reg[2] = {
				(struct emif_reg_struct *)EMIF4_0_CFG_BASE,
				(struct emif_reg_struct *)EMIF4_1_CFG_BASE};

/**
 * Base addresses for DDR PHY cmd/data regs
 */
static struct ddr_cmd_regs *ddr_cmd_reg[2] = {
				(struct ddr_cmd_regs *)DDR_PHY_CMD_ADDR,
				(struct ddr_cmd_regs *)DDR_PHY_CMD_ADDR2};

static struct ddr_data_regs *ddr_data_reg[2] = {
				(struct ddr_data_regs *)DDR_PHY_DATA_ADDR,
				(struct ddr_data_regs *)DDR_PHY_DATA_ADDR2};

/**
 * Base address for ddr io control instances
 */
static struct ddr_cmdtctrl *ioctrl_reg = {
			(struct ddr_cmdtctrl *)DDR_CONTROL_BASE_ADDR};

static inline u32 get_mr(int nr, u32 cs, u32 mr_addr)
{
	u32 mr;

	mr_addr |= cs << EMIF_REG_CS_SHIFT;
	writel(mr_addr, &emif_reg[nr]->emif_lpddr2_mode_reg_cfg);

	mr = readl(&emif_reg[nr]->emif_lpddr2_mode_reg_data);
	debug("get_mr: EMIF1 cs %d mr %08x val 0x%x\n", cs, mr_addr, mr);
	if (((mr & 0x0000ff00) >>  8) == (mr & 0xff) &&
	    ((mr & 0x00ff0000) >> 16) == (mr & 0xff) &&
	    ((mr & 0xff000000) >> 24) == (mr & 0xff))
		return mr & 0xff;
	else
		return mr;
}

static inline void set_mr(int nr, u32 cs, u32 mr_addr, u32 mr_val)
{
	mr_addr |= cs << EMIF_REG_CS_SHIFT;
	writel(mr_addr, &emif_reg[nr]->emif_lpddr2_mode_reg_cfg);
	writel(mr_val, &emif_reg[nr]->emif_lpddr2_mode_reg_data);
}

static void configure_mr(int nr, u32 cs)
{
	u32 mr_addr;

	while (get_mr(nr, cs, LPDDR2_MR0) & LPDDR2_MR0_DAI_MASK)
		;
	set_mr(nr, cs, LPDDR2_MR10, 0x56);

	set_mr(nr, cs, LPDDR2_MR1, 0x43);
	set_mr(nr, cs, LPDDR2_MR2, 0x2);

	mr_addr = LPDDR2_MR2 | EMIF_REG_REFRESH_EN_MASK;
	set_mr(nr, cs, mr_addr, 0x2);
}

/*
 * Configure EMIF4D5 registers and MR registers For details about these magic
 * values please see the EMIF registers section of the TRM.
 */
void config_sdram_emif4d5(const struct emif_regs *regs, int nr)
{
#ifdef CONFIG_AM43XX
	struct prm_device_inst *prm_device =
			(struct prm_device_inst *)PRM_DEVICE_INST;
#endif

	writel(0xA0, &emif_reg[nr]->emif_pwr_mgmt_ctrl);
	writel(0xA0, &emif_reg[nr]->emif_pwr_mgmt_ctrl_shdw);
	writel(regs->zq_config, &emif_reg[nr]->emif_zq_config);

	writel(regs->temp_alert_config, &emif_reg[nr]->emif_temp_alert_config);
	writel(regs->emif_rd_wr_lvl_rmp_win,
	       &emif_reg[nr]->emif_rd_wr_lvl_rmp_win);
	writel(regs->emif_rd_wr_lvl_rmp_ctl,
	       &emif_reg[nr]->emif_rd_wr_lvl_rmp_ctl);
	writel(regs->emif_rd_wr_lvl_ctl, &emif_reg[nr]->emif_rd_wr_lvl_ctl);
	writel(regs->emif_rd_wr_exec_thresh,
	       &emif_reg[nr]->emif_rd_wr_exec_thresh);

	/*
	 * for most SOCs these registers won't need to be changed so only
	 * write to these registers if someone explicitly has set the
	 * register's value.
	 */
	if(regs->emif_cos_config) {
		writel(regs->emif_prio_class_serv_map, &emif_reg[nr]->emif_prio_class_serv_map);
		writel(regs->emif_connect_id_serv_1_map, &emif_reg[nr]->emif_connect_id_serv_1_map);
		writel(regs->emif_connect_id_serv_2_map, &emif_reg[nr]->emif_connect_id_serv_2_map);
		writel(regs->emif_cos_config, &emif_reg[nr]->emif_cos_config);
	}

	/*
	 * Sequence to ensure that the PHY is in a known state prior to
	 * startting hardware leveling.  Also acts as to latch some state from
	 * the EMIF into the PHY.
	 */
	writel(0x2011, &emif_reg[nr]->emif_iodft_tlgc);
	writel(0x2411, &emif_reg[nr]->emif_iodft_tlgc);
	writel(0x2011, &emif_reg[nr]->emif_iodft_tlgc);

	clrbits_le32(&emif_reg[nr]->emif_sdram_ref_ctrl,
			EMIF_REG_INITREF_DIS_MASK);

	writel(regs->sdram_config, &emif_reg[nr]->emif_sdram_config);
	writel(regs->sdram_config, &cstat->secure_emif_sdram_config);

	/* Wait 1ms because of L3 timeout error */
	udelay(1000);

	writel(regs->ref_ctrl, &emif_reg[nr]->emif_sdram_ref_ctrl);
	writel(regs->ref_ctrl, &emif_reg[nr]->emif_sdram_ref_ctrl_shdw);

#ifdef CONFIG_AM43XX
	/*
	 * Disable EMIF_DEVOFF
	 * -> Cold Boot: This is just rewriting the default register value.
	 * -> RTC Resume: Must disable DEVOFF before leveling.
	 */
	writel(0, &prm_device->emif_ctrl);
#endif

	/* Perform hardware leveling for DDR3 */
	if (emif_sdram_type(regs->sdram_config) == EMIF_SDRAM_TYPE_DDR3) {
		writel(readl(&emif_reg[nr]->emif_ddr_ext_phy_ctrl_36) |
		       0x100, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_36);
		writel(readl(&emif_reg[nr]->emif_ddr_ext_phy_ctrl_36_shdw) |
		       0x100, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_36_shdw);

		writel(0x80000000, &emif_reg[nr]->emif_rd_wr_lvl_rmp_ctl);

		/* Enable read leveling */
		writel(0x80000000, &emif_reg[nr]->emif_rd_wr_lvl_ctl);

		/* Wait 1ms because of L3 timeout error */
		udelay(1000);

		/*
		 * Enable full read and write leveling.  Wait for read and write
		 * leveling bit to clear RDWRLVLFULL_START bit 31
		 */
		while ((readl(&emif_reg[nr]->emif_rd_wr_lvl_ctl) & 0x80000000)
		      != 0)
			;

		/* Check the timeout register to see if leveling is complete */
		if ((readl(&emif_reg[nr]->emif_status) & 0x70) != 0)
			puts("DDR3 H/W leveling incomplete with errors\n");

	} else {
		/* DDR2 */
		configure_mr(nr, 0);
		configure_mr(nr, 1);
	}
}

/**
 * Configure SDRAM
 */
void config_sdram(const struct emif_regs *regs, int nr)
{
#ifdef CONFIG_TI816X
	writel(regs->sdram_config, &emif_reg[nr]->emif_sdram_config);
	writel(regs->emif_ddr_phy_ctlr_1, &emif_reg[nr]->emif_ddr_phy_ctrl_1);
	writel(regs->emif_ddr_phy_ctlr_1, &emif_reg[nr]->emif_ddr_phy_ctrl_1_shdw);
	writel(0x0000613B, &emif_reg[nr]->emif_sdram_ref_ctrl);   /* initially a large refresh period */
	writel(0x1000613B, &emif_reg[nr]->emif_sdram_ref_ctrl);   /* trigger initialization           */
	writel(regs->ref_ctrl, &emif_reg[nr]->emif_sdram_ref_ctrl);
#else
	if (regs->zq_config) {
		writel(regs->zq_config, &emif_reg[nr]->emif_zq_config);
		writel(regs->sdram_config, &cstat->secure_emif_sdram_config);
		writel(regs->sdram_config, &emif_reg[nr]->emif_sdram_config);

		/* Trigger initialization */
		writel(0x00003100, &emif_reg[nr]->emif_sdram_ref_ctrl);
		/* Wait 1ms because of L3 timeout error */
		udelay(1000);

		/* Write proper sdram_ref_cref_ctrl value */
		writel(regs->ref_ctrl, &emif_reg[nr]->emif_sdram_ref_ctrl);
		writel(regs->ref_ctrl, &emif_reg[nr]->emif_sdram_ref_ctrl_shdw);
	}
	writel(regs->ref_ctrl, &emif_reg[nr]->emif_sdram_ref_ctrl);
	writel(regs->ref_ctrl, &emif_reg[nr]->emif_sdram_ref_ctrl_shdw);
	writel(regs->sdram_config, &emif_reg[nr]->emif_sdram_config);

	/* Write REG_COS_COUNT_1, REG_COS_COUNT_2, and REG_PR_OLD_COUNT. */
	if (regs->ocp_config)
		writel(regs->ocp_config, &emif_reg[nr]->emif_l3_config);
#endif
}

/**
 * Set SDRAM timings
 */
void set_sdram_timings(const struct emif_regs *regs, int nr)
{
	writel(regs->sdram_tim1, &emif_reg[nr]->emif_sdram_tim_1);
	writel(regs->sdram_tim1, &emif_reg[nr]->emif_sdram_tim_1_shdw);
	writel(regs->sdram_tim2, &emif_reg[nr]->emif_sdram_tim_2);
	writel(regs->sdram_tim2, &emif_reg[nr]->emif_sdram_tim_2_shdw);
	writel(regs->sdram_tim3, &emif_reg[nr]->emif_sdram_tim_3);
	writel(regs->sdram_tim3, &emif_reg[nr]->emif_sdram_tim_3_shdw);
}

/*
 * Configure EXT PHY registers for software leveling
 */
static void ext_phy_settings_swlvl(const struct emif_regs *regs, int nr)
{
	u32 *ext_phy_ctrl_base = 0;
	u32 *emif_ext_phy_ctrl_base = 0;
	__maybe_unused const u32 *ext_phy_ctrl_const_regs;
	u32 i = 0;
	__maybe_unused u32 size;

	ext_phy_ctrl_base = (u32 *)&(regs->emif_ddr_ext_phy_ctrl_1);
	emif_ext_phy_ctrl_base =
			(u32 *)&(emif_reg[nr]->emif_ddr_ext_phy_ctrl_1);

	/* Configure external phy control timing registers */
	for (i = 0; i < EMIF_EXT_PHY_CTRL_TIMING_REG; i++) {
		writel(*ext_phy_ctrl_base, emif_ext_phy_ctrl_base++);
		/* Update shadow registers */
		writel(*ext_phy_ctrl_base++, emif_ext_phy_ctrl_base++);
	}

#ifdef CONFIG_AM43XX
	/*
	 * External phy 6-24 registers do not change with ddr frequency.
	 * These only need to be set on DDR2 on AM43xx.
	 */
	emif_get_ext_phy_ctrl_const_regs(&ext_phy_ctrl_const_regs, &size);

	if (!size)
		return;

	for (i = 0; i < size; i++) {
		writel(ext_phy_ctrl_const_regs[i], emif_ext_phy_ctrl_base++);
		/* Update shadow registers */
		writel(ext_phy_ctrl_const_regs[i], emif_ext_phy_ctrl_base++);
	}
#endif
}

/*
 * Configure EXT PHY registers for hardware leveling
 */
static void ext_phy_settings_hwlvl(const struct emif_regs *regs, int nr)
{
	/*
	 * Enable hardware leveling on the EMIF.  For details about these
	 * magic values please see the EMIF registers section of the TRM.
	 */
	if (regs->emif_ddr_phy_ctlr_1 & 0x00040000) {
		/* PHY_INVERT_CLKOUT = 1 */
		writel(0x00040100, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_1);
		writel(0x00040100, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_1_shdw);
	} else {
		/* PHY_INVERT_CLKOUT = 0 */
		writel(0x08020080, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_1);
		writel(0x08020080, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_1_shdw);
	}

	writel(0x00000000, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_22);
	writel(0x00000000, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_22_shdw);
	writel(0x00600020, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_23);
	writel(0x00600020, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_23_shdw);
	writel(0x40010080, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_24);
	writel(0x40010080, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_24_shdw);
	writel(0x08102040, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_25);
	writel(0x08102040, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_25_shdw);
	writel(0x00200020, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_26);
	writel(0x00200020, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_26_shdw);
	writel(0x00200020, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_27);
	writel(0x00200020, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_27_shdw);
	writel(0x00200020, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_28);
	writel(0x00200020, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_28_shdw);
	writel(0x00200020, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_29);
	writel(0x00200020, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_29_shdw);
	writel(0x00200020, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_30);
	writel(0x00200020, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_30_shdw);
	writel(0x00000000, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_31);
	writel(0x00000000, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_31_shdw);
	writel(0x00000000, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_32);
	writel(0x00000000, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_32_shdw);
	writel(0x00000000, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_33);
	writel(0x00000000, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_33_shdw);
	writel(0x00000000, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_34);
	writel(0x00000000, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_34_shdw);
	writel(0x00000000, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_35);
	writel(0x00000000, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_35_shdw);
	writel(0x00000077, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_36);
	writel(0x00000077, &emif_reg[nr]->emif_ddr_ext_phy_ctrl_36_shdw);

	/*
	 * Sequence to ensure that the PHY is again in a known state after
	 * hardware leveling.
	 */
	writel(0x2011, &emif_reg[nr]->emif_iodft_tlgc);
	writel(0x2411, &emif_reg[nr]->emif_iodft_tlgc);
	writel(0x2011, &emif_reg[nr]->emif_iodft_tlgc);
}

/**
 * Configure DDR PHY
 */
void config_ddr_phy(const struct emif_regs *regs, int nr)
{
	/*
	 * Disable initialization and refreshes for now until we finish
	 * programming EMIF regs and set time between rising edge of
	 * DDR_RESET to rising edge of DDR_CKE to > 500us per memory spec.
	 * We currently hardcode a value based on a max expected frequency
	 * of 400MHz.
	 */
	writel(EMIF_REG_INITREF_DIS_MASK | 0x3100,
		&emif_reg[nr]->emif_sdram_ref_ctrl);

	writel(regs->emif_ddr_phy_ctlr_1,
		&emif_reg[nr]->emif_ddr_phy_ctrl_1);
	writel(regs->emif_ddr_phy_ctlr_1,
		&emif_reg[nr]->emif_ddr_phy_ctrl_1_shdw);

	if (get_emif_rev((u32)emif_reg[nr]) == EMIF_4D5) {
		if (emif_sdram_type(regs->sdram_config) == EMIF_SDRAM_TYPE_DDR3)
			ext_phy_settings_hwlvl(regs, nr);
		else
			ext_phy_settings_swlvl(regs, nr);
	}
}

/**
 * Configure DDR CMD control registers
 */
void config_cmd_ctrl(const struct cmd_control *cmd, int nr)
{
	if (!cmd)
		return;

	writel(cmd->cmd0csratio, &ddr_cmd_reg[nr]->cm0csratio);
	writel(cmd->cmd0iclkout, &ddr_cmd_reg[nr]->cm0iclkout);

	writel(cmd->cmd1csratio, &ddr_cmd_reg[nr]->cm1csratio);
	writel(cmd->cmd1iclkout, &ddr_cmd_reg[nr]->cm1iclkout);

	writel(cmd->cmd2csratio, &ddr_cmd_reg[nr]->cm2csratio);
	writel(cmd->cmd2iclkout, &ddr_cmd_reg[nr]->cm2iclkout);
}

/**
 * Configure DDR DATA registers
 */
void config_ddr_data(const struct ddr_data *data, int nr)
{
	int i;

	if (!data)
		return;

	for (i = 0; i < DDR_DATA_REGS_NR; i++) {
		writel(data->datardsratio0,
			&(ddr_data_reg[nr]+i)->dt0rdsratio0);
		writel(data->datawdsratio0,
			&(ddr_data_reg[nr]+i)->dt0wdsratio0);
		writel(data->datawiratio0,
			&(ddr_data_reg[nr]+i)->dt0wiratio0);
		writel(data->datagiratio0,
			&(ddr_data_reg[nr]+i)->dt0giratio0);
		writel(data->datafwsratio0,
			&(ddr_data_reg[nr]+i)->dt0fwsratio0);
		writel(data->datawrsratio0,
			&(ddr_data_reg[nr]+i)->dt0wrsratio0);
	}
}

void config_io_ctrl(const struct ctrl_ioregs *ioregs)
{
	if (!ioregs)
		return;

	writel(ioregs->cm0ioctl, &ioctrl_reg->cm0ioctl);
	writel(ioregs->cm1ioctl, &ioctrl_reg->cm1ioctl);
	writel(ioregs->cm2ioctl, &ioctrl_reg->cm2ioctl);
	writel(ioregs->dt0ioctl, &ioctrl_reg->dt0ioctl);
	writel(ioregs->dt1ioctl, &ioctrl_reg->dt1ioctl);
#ifdef CONFIG_AM43XX
	writel(ioregs->dt2ioctrl, &ioctrl_reg->dt2ioctrl);
	writel(ioregs->dt3ioctrl, &ioctrl_reg->dt3ioctrl);
	writel(ioregs->emif_sdram_config_ext,
	       &ioctrl_reg->emif_sdram_config_ext);
#endif
}
