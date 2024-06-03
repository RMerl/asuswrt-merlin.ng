// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include <common.h>
#include <i2c.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "high_speed_env_spec.h"
#include "board_env_spec.h"

#define	SERDES_VERSION	"2.1.5"
#define ENDED_OK	"High speed PHY - Ended Successfully\n"

static const u8 serdes_cfg[][SERDES_LAST_UNIT] = BIN_SERDES_CFG;

extern MV_BIN_SERDES_CFG *serdes_info_tbl[];

extern u8 rd78460gp_twsi_dev[];
extern u8 db88f78xx0rev2_twsi_dev[];

u32 pex_cfg_read(u32 pex_if, u32 bus, u32 dev, u32 func, u32 offs);
int pex_local_bus_num_set(u32 pex_if, u32 bus_num);
int pex_local_dev_num_set(u32 pex_if, u32 dev_num);

#define MV_BOARD_PEX_MODULE_ADDR		0x23
#define MV_BOARD_PEX_MODULE_ID			1
#define MV_BOARD_ETM_MODULE_ID			2

#define	PEX_MODULE_DETECT		1
#define	ETM_MODULE_DETECT               2

#define PEX_MODE_GET(satr)		((satr & 0x6) >> 1)
#define PEX_CAPABILITY_GET(satr, port)	((satr >> port) & 1)
#define MV_PEX_UNIT_TO_IF(pex_unit)	((pex_unit < 3) ? (pex_unit * 4) : 9)

/* Static parametes */
static int config_module;
static int switch_module;

/* Local function */
static u32 board_id_get(void)
{
#if defined(CONFIG_DB_88F78X60)
	return DB_88F78XX0_BP_ID;
#elif defined(CONFIG_RD_88F78460_SERVER)
	return RD_78460_SERVER_ID;
#elif defined(CONFIG_RD_78460_SERVER_REV2)
	return RD_78460_SERVER_REV2_ID;
#elif defined(CONFIG_DB_78X60_PCAC)
	return DB_78X60_PCAC_ID;
#elif defined(CONFIG_DB_88F78X60_REV2)
	return DB_88F78XX0_BP_REV2_ID;
#elif defined(CONFIG_RD_78460_NAS)
	return RD_78460_NAS_ID;
#elif defined(CONFIG_DB_78X60_AMC)
	return DB_78X60_AMC_ID;
#elif defined(CONFIG_DB_78X60_PCAC_REV2)
	return DB_78X60_PCAC_REV2_ID;
#elif defined(CONFIG_DB_784MP_GP)
	return DB_784MP_GP_ID;
#elif defined(CONFIG_RD_78460_CUSTOMER)
	return RD_78460_CUSTOMER_ID;
#else
	/*
	 * Return 0 here for custom board as this should not be used
	 * for custom boards.
	 */
	return 0;
#endif
}

__weak u8 board_sat_r_get(u8 dev_num, u8 reg)
{
	u8 data;
	u8 *dev;
	u32 board_id = board_id_get();
	int ret;

	switch (board_id) {
	case DB_78X60_AMC_ID:
	case DB_78X60_PCAC_REV2_ID:
	case RD_78460_CUSTOMER_ID:
	case RD_78460_SERVER_ID:
	case RD_78460_SERVER_REV2_ID:
	case DB_78X60_PCAC_ID:
		return (0x1 << 1) | 1;
	case FPGA_88F78XX0_ID:
	case RD_78460_NAS_ID:
		return (0x0 << 1) | 1;
	case DB_784MP_GP_ID:
		dev = rd78460gp_twsi_dev;

		break;
	case DB_88F78XX0_BP_ID:
	case DB_88F78XX0_BP_REV2_ID:
		dev = db88f78xx0rev2_twsi_dev;
		break;

	default:
		return 0;
	}

	/* Read MPP module ID */
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	ret = i2c_read(dev[dev_num], 0, 1, (u8 *)&data, 1);
	if (ret)
		return MV_ERROR;

	return data;
}

static int board_modules_scan(void)
{
	u8 val;
	u32 board_id = board_id_get();
	int ret;

	/* Perform scan only for DB board */
	if ((board_id == DB_88F78XX0_BP_ID) ||
	    (board_id == DB_88F78XX0_BP_REV2_ID)) {
		/* reset modules flags */
		config_module = 0;

		i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

		/* SERDES module (only PEX model is supported now) */
		ret = i2c_read(MV_BOARD_PEX_MODULE_ADDR, 0, 1, (u8 *)&val, 1);
		if (ret)
			return MV_ERROR;

		if (val == MV_BOARD_PEX_MODULE_ID)
			config_module = PEX_MODULE_DETECT;
		if (val == MV_BOARD_ETM_MODULE_ID)
			config_module = ETM_MODULE_DETECT;
	} else if (board_id == RD_78460_NAS_ID) {
		switch_module = 0;
		if ((reg_read(GPP_DATA_IN_REG(2)) & MV_GPP66) == 0x0)
			switch_module = 1;
	}

	return MV_OK;
}

u32 pex_max_unit_get(void)
{
	/*
	 * TODO:
	 * Right now only MV78460 is supported. Other SoC's might need
	 * a different value here.
	 */
	return MV_PEX_MAX_UNIT;
}

u32 pex_max_if_get(void)
{
	/*
	 * TODO:
	 * Right now only MV78460 is supported. Other SoC's might need
	 * a different value here.
	 */
	return MV_PEX_MAX_IF;
}

u8 board_cpu_freq_get(void)
{
	u32 sar;
	u32 sar_msb;

	sar = reg_read(MPP_SAMPLE_AT_RESET(0));
	sar_msb = reg_read(MPP_SAMPLE_AT_RESET(1));
	return ((sar_msb & 0x100000) >> 17) | ((sar & 0xe00000) >> 21);
}

__weak MV_BIN_SERDES_CFG *board_serdes_cfg_get(void)
{
	u32 board_id;
	u32 serdes_cfg_val = 0;	/* default */

	board_id = board_id_get();

	switch (board_id) {
	case DB_784MP_GP_ID:
		serdes_cfg_val = 0;
		break;
	}

	return &serdes_info_tbl[board_id - BOARD_ID_BASE][serdes_cfg_val];
}

u16 ctrl_model_get(void)
{
	/*
	 * SoC version can't be autodetected. So we need to rely on a define
	 * from the config system here.
	 */
#if defined(CONFIG_MV78230)
	return MV_78230_DEV_ID;
#elif defined(CONFIG_MV78260)
	return MV_78260_DEV_ID;
#else
	return MV_78460_DEV_ID;
#endif
}

u32 get_line_cfg(u32 line_num, MV_BIN_SERDES_CFG *info)
{
	if (line_num < 8)
		return (info->line0_7 >> (line_num << 2)) & 0xF;
	else
		return (info->line8_15 >> ((line_num - 8) << 2)) & 0xF;
}

static int serdes_max_lines_get(void)
{
	switch (ctrl_model_get()) {
	case MV_78230_DEV_ID:
		return 7;
	case MV_78260_DEV_ID:
		return 12;
	case MV_78460_DEV_ID:
		return 16;
	}

	return 0;
}

/*
 * Tests have shown that on some boards the default width of the
 * configuration pulse for the PEX link detection might lead to
 * non-established PCIe links (link down). Especially under certain
 * conditions (higher temperature) and with specific PCIe devices.
 * To enable a board-specific detection pulse width this weak
 * array "serdes_pex_pulse_width[4]" is introduced which can be
 * overwritten if needed by a board-specific version. If the board
 * code does not provide a non-weak version of this variable, the
 * default value will be used. So nothing is changed from the
 * current setup on the supported board.
 */
__weak u8 serdes_pex_pulse_width[4] = { 2, 2, 2, 2 };

int serdes_phy_config(void)
{
	int status = MV_OK;
	u32 line_cfg;
	u8 line_num;
	/* addr/value for each line @ every setup step */
	u32 addr[16][11], val[16][11];
	u8 pex_unit, pex_line_num;
	u8 sgmii_port = 0;
	u32 tmp;
	u32 in_direct;
	u8 max_serdes_lines;
	MV_BIN_SERDES_CFG *info;
	u8 satr11;
	u8 sata_port;
	u8 freq;
	u8 device_rev;
	u32 rx_high_imp_mode;
	u16 ctrl_mode;
	u32 pex_if;
	u32 pex_if_num;

	/*
	 * Get max. serdes lines count
	 */
	max_serdes_lines = serdes_max_lines_get();
	if (max_serdes_lines == 0)
		return MV_OK;

	satr11 = board_sat_r_get(1, 1);
	if ((u8) MV_ERROR == (u8) satr11)
		return MV_ERROR;

	board_modules_scan();
	memset(addr, 0, sizeof(addr));
	memset(val, 0, sizeof(val));

	/* Check if DRAM is already initialized  */
	if (reg_read(REG_BOOTROM_ROUTINE_ADDR) &
	    (1 << REG_BOOTROM_ROUTINE_DRAM_INIT_OFFS)) {
		DEBUG_INIT_S("High speed PHY - Version: ");
		DEBUG_INIT_S(SERDES_VERSION);
		DEBUG_INIT_S(" - 2nd boot - Skip\n");
		return MV_OK;
	}
	DEBUG_INIT_S("High speed PHY - Version: ");
	DEBUG_INIT_S(SERDES_VERSION);
	DEBUG_INIT_S(" (COM-PHY-V20)\n");

	/*
	 * AVS :  disable AVS for frequency less than 1333
	 */
	freq = board_cpu_freq_get();
	device_rev = mv_ctrl_rev_get();

	if (device_rev == 2) {	/*   for B0 only */
		u32 cpu_avs;
		u8 fabric_freq;
		cpu_avs = reg_read(CPU_AVS_CONTROL2_REG);
		DEBUG_RD_REG(CPU_AVS_CONTROL2_REG, cpu_avs);
		cpu_avs &= ~(1 << 9);

		if ((0x4 == freq) || (0xB == freq)) {
			u32 tmp2;

			tmp2 = reg_read(CPU_AVS_CONTROL0_REG);
			DEBUG_RD_REG(CPU_AVS_CONTROL0_REG, tmp2);
			/* cpu upper limit = 1.1V  cpu lower limit = 0.9125V  */
			tmp2 |= 0x0FF;
			reg_write(CPU_AVS_CONTROL0_REG, tmp2);
			DEBUG_WR_REG(CPU_AVS_CONTROL0_REG, tmp2);
			cpu_avs |= (1 << 9);	/* cpu avs enable */
			cpu_avs |= (1 << 18);	/* AvsAvddDetEn enable  */
			fabric_freq = (reg_read(MPP_SAMPLE_AT_RESET(0)) &
				       SAR0_FABRIC_FREQ_MASK) >> SAR0_FABRIC_FREQ_OFFSET;
			if ((0xB == freq) && (5 == fabric_freq)) {
				u32 core_avs;

				core_avs = reg_read(CORE_AVS_CONTROL_0REG);
				DEBUG_RD_REG(CORE_AVS_CONTROL_0REG, core_avs);

				/*
				 * Set core lower limit = 0.9V &
				 * core upper limit = 0.9125V
				 */
				core_avs &= ~(0xff);
				core_avs |= 0x0E;
				reg_write(CORE_AVS_CONTROL_0REG, core_avs);
				DEBUG_WR_REG(CORE_AVS_CONTROL_0REG, core_avs);

				core_avs = reg_read(CORE_AVS_CONTROL_2REG);
				DEBUG_RD_REG(CORE_AVS_CONTROL_2REG, core_avs);
				core_avs |= (1 << 9);	/*  core AVS enable  */
				reg_write(CORE_AVS_CONTROL_2REG, core_avs);
				DEBUG_WR_REG(CORE_AVS_CONTROL_2REG, core_avs);

				tmp2 = reg_read(GENERAL_PURPOSE_RESERVED0_REG);
				DEBUG_RD_REG(GENERAL_PURPOSE_RESERVED0_REG,
					     tmp2);
				tmp2 |= 0x1;	/*  AvsCoreAvddDetEn enable   */
				reg_write(GENERAL_PURPOSE_RESERVED0_REG, tmp2);
				DEBUG_WR_REG(GENERAL_PURPOSE_RESERVED0_REG,
					     tmp2);
			}
		}
		reg_write(CPU_AVS_CONTROL2_REG, cpu_avs);
		DEBUG_WR_REG(CPU_AVS_CONTROL2_REG, cpu_avs);
	}

	info = board_serdes_cfg_get();

	if (info == NULL) {
		DEBUG_INIT_S("Hight speed PHY Error #1\n");
		return MV_ERROR;
	}
	DEBUG_INIT_FULL_S("info->line0_7= 0x");
	DEBUG_INIT_FULL_D(info->line0_7, 8);
	DEBUG_INIT_FULL_S("   info->line8_15= 0x");
	DEBUG_INIT_FULL_D(info->line8_15, 8);
	DEBUG_INIT_FULL_S("\n");

	if (config_module & ETM_MODULE_DETECT) {	/* step 0.9 ETM */
		DEBUG_INIT_FULL_S("ETM module detect Step 0.9:\n");
		reg_write(SERDES_LINE_MUX_REG_0_7, 0x11111111);
		DEBUG_WR_REG(SERDES_LINE_MUX_REG_0_7, 0x11111111);
		info->pex_mode[1] = PEX_BUS_DISABLED;	/* pex unit 1 is configure for ETM */
		mdelay(100);
		reg_write(PEX_PHY_ACCESS_REG(1), (0x002 << 16) | 0xf44d);	/* SETM0 - start calibration         */
		DEBUG_WR_REG(PEX_PHY_ACCESS_REG(1), (0x002 << 16) | 0xf44d);	/* SETM0 - start calibration         */
		reg_write(PEX_PHY_ACCESS_REG(1), (0x302 << 16) | 0xf44d);	/* SETM1 - start calibration         */
		DEBUG_WR_REG(PEX_PHY_ACCESS_REG(1), (0x302 << 16) | 0xf44d);	/* SETM1 - start calibration         */
		reg_write(PEX_PHY_ACCESS_REG(1), (0x001 << 16) | 0xf801);	/* SETM0 - SATA mode & 25MHz ref clk */
		DEBUG_WR_REG(PEX_PHY_ACCESS_REG(1), (0x001 << 16) | 0xf801);	/* SETM0 - SATA mode & 25MHz ref clk */
		reg_write(PEX_PHY_ACCESS_REG(1), (0x301 << 16) | 0xf801);	/* SETM1 - SATA mode & 25MHz ref clk */
		DEBUG_WR_REG(PEX_PHY_ACCESS_REG(1), (0x301 << 16) | 0xf801);	/* SETM1 - SATA mode & 25MHz ref clk */
		reg_write(PEX_PHY_ACCESS_REG(1), (0x011 << 16) | 0x0BFF);	/* SETM0 - G3 full swing AMP         */
		DEBUG_WR_REG(PEX_PHY_ACCESS_REG(1), (0x011 << 16) | 0x0BFF);	/* SETM0 - G3 full swing AMP         */
		reg_write(PEX_PHY_ACCESS_REG(1), (0x311 << 16) | 0x0BFF);	/* SETM1 - G3 full swing AMP         */
		DEBUG_WR_REG(PEX_PHY_ACCESS_REG(1), (0x311 << 16) | 0x0BFF);	/* SETM1 - G3 full swing AMP         */
		reg_write(PEX_PHY_ACCESS_REG(1), (0x023 << 16) | 0x0800);	/* SETM0 - 40 data bit width         */
		DEBUG_WR_REG(PEX_PHY_ACCESS_REG(1), (0x023 << 16) | 0x0800);	/* SETM0 - 40 data bit width         */
		reg_write(PEX_PHY_ACCESS_REG(1), (0x323 << 16) | 0x0800);	/* SETM1 - 40 data bit width         */
		DEBUG_WR_REG(PEX_PHY_ACCESS_REG(1), (0x323 << 16) | 0x0800);	/* SETM1 - 40 data bit width         */
		reg_write(PEX_PHY_ACCESS_REG(1), (0x046 << 16) | 0x0400);	/* lane0(serdes4)                    */
		DEBUG_WR_REG(PEX_PHY_ACCESS_REG(1), (0x046 << 16) | 0x0400);	/* lane0(serdes4)                    */
		reg_write(PEX_PHY_ACCESS_REG(1), (0x346 << 16) | 0x0400);	/* lane3(serdes7)                    */
		DEBUG_WR_REG(PEX_PHY_ACCESS_REG(1), (0x346 << 16) | 0x0400);	/* lane3(serdes7)                    */
	}

	/* STEP -1 [PEX-Only] First phase of PEX-PIPE Configuration: */
	DEBUG_INIT_FULL_S("Step 1: First phase of PEX-PIPE Configuration\n");
	for (pex_unit = 0; pex_unit < pex_max_unit_get(); pex_unit++) {
		if (info->pex_mode[pex_unit] == PEX_BUS_DISABLED)
			continue;

		/* 1.   GLOB_CLK_CTRL Reset and Clock Control */
		reg_write(PEX_PHY_ACCESS_REG(pex_unit), (0xC1 << 16) | 0x25);
		DEBUG_WR_REG(PEX_PHY_ACCESS_REG(pex_unit), (0xC1 << 16) | 0x25);

		/* 2.   GLOB_TEST_CTRL Test Mode Control */
		if (info->pex_mode[pex_unit] == PEX_BUS_MODE_X4) {
			reg_write(PEX_PHY_ACCESS_REG(pex_unit),
				  (0xC2 << 16) | 0x200);
			DEBUG_WR_REG(PEX_PHY_ACCESS_REG(pex_unit),
				     (0xC2 << 16) | 0x200);
		}

		/* 3.   GLOB_CLK_SRC_LO Clock Source Low */
		if (info->pex_mode[pex_unit] == PEX_BUS_MODE_X1) {
			reg_write(PEX_PHY_ACCESS_REG(pex_unit),
				  (0xC3 << 16) | 0x0F);
			DEBUG_WR_REG(PEX_PHY_ACCESS_REG(pex_unit),
				     (0xC3 << 16) | 0x0F);
		}

		reg_write(PEX_PHY_ACCESS_REG(pex_unit), (0xC5 << 16) | 0x11F);
		DEBUG_WR_REG(PEX_PHY_ACCESS_REG(pex_unit),
			     (0xC5 << 16) | 0x11F);
	}

	/*
	 * 2 Configure the desire PIN_PHY_GEN and do power down to the PU_PLL,
	 * PU_RX,PU_TX. (bits[12:5])
	 */
	DEBUG_INIT_FULL_S("Step 2: Configure the desire PIN_PHY_GEN\n");
	for (line_num = 0; line_num < max_serdes_lines; line_num++) {
		line_cfg = get_line_cfg(line_num, info);
		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_UNCONNECTED])
			continue;
		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_PEX])
			continue;
		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SATA]) {
			switch (line_num) {
			case 4:
			case 6:
				sata_port = 0;
				break;
			case 5:
				sata_port = 1;
				break;
			default:
				DEBUG_INIT_C
				    ("SATA port error for serdes line: ",
				     line_num, 2);
				return MV_ERROR;
			}
			tmp = reg_read(SATA_LP_PHY_EXT_CTRL_REG(sata_port));
			DEBUG_RD_REG(SATA_LP_PHY_EXT_CTRL_REG(sata_port), tmp);
			tmp &= ~((0x1ff << 5) | 0x7);
			tmp |= ((info->bus_speed & (1 << line_num)) != 0) ?
				(0x11 << 5) : 0x0;

			reg_write(SATA_LP_PHY_EXT_CTRL_REG(sata_port), tmp);
			DEBUG_WR_REG(SATA_LP_PHY_EXT_CTRL_REG(sata_port), tmp);
		}

		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_QSGMII]) {
			/*
			 * 4) Configure the desire PIN_PHY_GEN and do power
			 * down to the PU_PLL,PU_RX,PU_TX. (bits[12:5])
			 */
			tmp = reg_read(SGMII_SERDES_CFG_REG(0));
			DEBUG_RD_REG(SGMII_SERDES_CFG_REG(0), tmp);
			tmp &= ~((0x1ff << 5) | 0x7);
			tmp |= 0x660;
			reg_write(SGMII_SERDES_CFG_REG(0), tmp);
			DEBUG_WR_REG(SGMII_SERDES_CFG_REG(0), tmp);
			continue;
		}

		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SGMII0])
			sgmii_port = 0;
		else if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SGMII1])
			sgmii_port = 1;
		else if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SGMII2])
			sgmii_port = 2;
		else if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SGMII3])
			sgmii_port = 3;
		else
			continue;

		tmp = reg_read(SGMII_SERDES_CFG_REG(sgmii_port));
		DEBUG_RD_REG(SGMII_SERDES_CFG_REG(sgmii_port), tmp);
		tmp &= ~((0x1ff << 5) | 0x7);
		tmp |= (((info->bus_speed & (1 << line_num)) != 0) ?
			(0x88 << 5) : (0x66 << 5));
		reg_write(SGMII_SERDES_CFG_REG(sgmii_port), tmp);
		DEBUG_WR_REG(SGMII_SERDES_CFG_REG(sgmii_port), tmp);
	}

	/* Step 3 - QSGMII enable */
	DEBUG_INIT_FULL_S("Step 3 QSGMII enable\n");
	for (line_num = 0; line_num < max_serdes_lines; line_num++) {
		line_cfg = get_line_cfg(line_num, info);
		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_QSGMII]) {
			/* QSGMII Active bit set to true */
			tmp = reg_read(QSGMII_CONTROL_1_REG);
			DEBUG_RD_REG(QSGMII_CONTROL_1_REG, tmp);
			tmp |= (1 << 30);
#ifdef ERRATA_GL_6572255
			tmp |= (1 << 27);
#endif
			reg_write(QSGMII_CONTROL_1_REG, tmp);
			DEBUG_WR_REG(QSGMII_CONTROL_1_REG, tmp);
		}
	}

	/* Step 4 - configure SERDES MUXes */
	DEBUG_INIT_FULL_S("Step 4: Configure SERDES MUXes\n");
	if (config_module & ETM_MODULE_DETECT) {
		reg_write(SERDES_LINE_MUX_REG_0_7, 0x40041111);
		DEBUG_WR_REG(SERDES_LINE_MUX_REG_0_7, 0x40041111);
	} else {
		reg_write(SERDES_LINE_MUX_REG_0_7, info->line0_7);
		DEBUG_WR_REG(SERDES_LINE_MUX_REG_0_7, info->line0_7);
	}
	reg_write(SERDES_LINE_MUX_REG_8_15, info->line8_15);
	DEBUG_WR_REG(SERDES_LINE_MUX_REG_8_15, info->line8_15);

	/* Step 5: Activate the RX High Impedance Mode  */
	DEBUG_INIT_FULL_S("Step 5: Activate the RX High Impedance Mode\n");
	rx_high_imp_mode = 0x8080;
	if (device_rev == 2)	/*   for B0 only */
		rx_high_imp_mode |= 4;

	for (line_num = 0; line_num < max_serdes_lines; line_num++) {
		/* for each serdes lane */
		DEBUG_INIT_FULL_S("SERDES  ");
		DEBUG_INIT_FULL_D_10(line_num, 2);
		line_cfg = get_line_cfg(line_num, info);
		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_UNCONNECTED]) {
			DEBUG_INIT_FULL_S(" unconnected ***\n");
			continue;
		}
		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_PEX]) {
			pex_unit = line_num >> 2;
			pex_line_num = line_num % 4;
			DEBUG_INIT_FULL_S(" - PEX unit ");
			DEBUG_INIT_FULL_D_10(pex_unit, 1);
			DEBUG_INIT_FULL_S(" line=  ");
			DEBUG_INIT_FULL_D_10(pex_line_num, 1);
			DEBUG_INIT_FULL_S("\n");

			/* Needed for PEX_PHY_ACCESS_REG macro */
			if ((line_num > 7) &&
			    (info->pex_mode[3] == PEX_BUS_MODE_X8))
				/* lines 8 - 15 are belong to PEX3 in x8 mode */
				pex_unit = 3;

			if (info->pex_mode[pex_unit] == PEX_BUS_DISABLED)
				continue;

			/*
			 * 8)  Activate the RX High Impedance Mode field
			 * (bit [2]) in register /PCIe_USB Control (Each MAC
			 * contain different Access to reach its
			 * Serdes-Regfile).
			 * [PEX-Only] Set bit[12]: The analog part latches idle
			 * if PU_TX = 1 and PU_PLL =1.
			 */

			/* Termination enable */
			if (info->pex_mode[pex_unit] == PEX_BUS_MODE_X1) {
				in_direct = (0x48 << 16) | (pex_line_num << 24) |
					0x1000 | rx_high_imp_mode;	/* x1 */
			} else if ((info->pex_mode[pex_unit] ==
				    PEX_BUS_MODE_X4) && (pex_line_num == 0))
				in_direct = (0x48 << 16) | (pex_line_num << 24) |
					0x1000 | (rx_high_imp_mode & 0xff);	/* x4 */
			else
				in_direct = 0;

			if (in_direct) {
				reg_write(PEX_PHY_ACCESS_REG(pex_unit),
					  in_direct);
				DEBUG_WR_REG(PEX_PHY_ACCESS_REG(pex_unit),
					     in_direct);
			}

			continue;
		}

		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SATA]) {
			/*
			 * port 0 for serdes lines 4,6,  and port 1 for
			 * serdes lines 5
			 */
			sata_port = line_num & 1;
			DEBUG_INIT_FULL_S(" - SATA port  ");
			DEBUG_INIT_FULL_D_10(sata_port, 2);
			DEBUG_INIT_FULL_S("\n");
			reg_write(SATA_COMPHY_CTRL_REG(sata_port),
				  rx_high_imp_mode);
			DEBUG_WR_REG(SATA_COMPHY_CTRL_REG(sata_port),
				     rx_high_imp_mode);
			continue;
		}

		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_QSGMII]) {
			DEBUG_INIT_FULL_S(" - QSGMII\n");
			reg_write(SGMII_COMPHY_CTRL_REG(0), rx_high_imp_mode);
			DEBUG_WR_REG(SGMII_COMPHY_CTRL_REG(0),
				     rx_high_imp_mode);
			continue;
		}

		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SGMII0])
			sgmii_port = 0;
		else if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SGMII1])
			sgmii_port = 1;
		else if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SGMII2])
			sgmii_port = 2;
		else if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SGMII3])
			sgmii_port = 3;
		else
			continue;
		DEBUG_INIT_FULL_S(" - SGMII port  ");
		DEBUG_INIT_FULL_D_10(sgmii_port, 2);
		DEBUG_INIT_FULL_S("\n");
		reg_write(SGMII_COMPHY_CTRL_REG(sgmii_port), rx_high_imp_mode);
		DEBUG_WR_REG(SGMII_COMPHY_CTRL_REG(sgmii_port),
			     rx_high_imp_mode);
	}			/* for each serdes lane */

	/* Step 6 [PEX-Only] PEX-Main configuration (X4 or X1): */
	DEBUG_INIT_FULL_S("Step 6: [PEX-Only] PEX-Main configuration (X4 or X1)\n");
	tmp = reg_read(SOC_CTRL_REG);
	DEBUG_RD_REG(SOC_CTRL_REG, tmp);
	tmp &= 0x200;
	if (info->pex_mode[0] == PEX_BUS_MODE_X1)
		tmp |= PCIE0_QUADX1_EN;
	if (info->pex_mode[1] == PEX_BUS_MODE_X1)
		tmp |= PCIE1_QUADX1_EN;
	if (((reg_read(MPP_SAMPLE_AT_RESET(0)) & PEX_CLK_100MHZ_MASK) >>
	     PEX_CLK_100MHZ_OFFSET) == 0x1)
		tmp |= (PCIE0_CLK_OUT_EN_MASK | PCIE1_CLK_OUT_EN_MASK);

	reg_write(SOC_CTRL_REG, tmp);
	DEBUG_WR_REG(SOC_CTRL_REG, tmp);

	/* 6.2 PCI Express Link Capabilities */
	DEBUG_INIT_FULL_S("Step 6.2: [PEX-Only] PCI Express Link Capabilities\n");

	for (line_num = 0; line_num < max_serdes_lines; line_num++) {
		line_cfg = get_line_cfg(line_num, info);

		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_PEX]) {
			/*
			 * PCI Express Control
			 * 0xX1A00 [0]:
			 * 0x0 X4-Link.
			 * 0x1 X1-Link
			 */
			pex_unit = line_num >> 2;
			pex_if = MV_SERDES_NUM_TO_PEX_NUM(line_num);
			if (info->pex_mode[pex_unit] == PEX_BUS_DISABLED)
				continue;

			/*  set Common Clock Configuration */
			tmp = reg_read(PEX_LINK_CTRL_STATUS_REG(pex_if));
			DEBUG_RD_REG(PEX_LINK_CTRL_STATUS_REG(pex_if), tmp);
			tmp |= (1 << 6);
			reg_write(PEX_LINK_CTRL_STATUS_REG(pex_if), tmp);
			DEBUG_WR_REG(PEX_LINK_CTRL_STATUS_REG(pex_if), tmp);

			tmp = reg_read(PEX_LINK_CAPABILITIES_REG(pex_if));
			DEBUG_RD_REG(PEX_LINK_CAPABILITIES_REG(pex_if), tmp);
			tmp &= ~(0x3FF);
			if (info->pex_mode[pex_unit] == PEX_BUS_MODE_X1)
				tmp |= (0x1 << 4);
			if (info->pex_mode[pex_unit] == PEX_BUS_MODE_X4)
				tmp |= (0x4 << 4);
			if (0 == PEX_CAPABILITY_GET(satr11, pex_unit))
				tmp |= 0x1;
			else
				tmp |= 0x2;
			DEBUG_INIT_FULL_S("Step 6.2: PEX ");
			DEBUG_INIT_FULL_D(pex_if, 1);
			DEBUG_INIT_FULL_C(" set GEN", (tmp & 3), 1);
			reg_write(PEX_LINK_CAPABILITIES_REG(pex_if), tmp);
			DEBUG_WR_REG(PEX_LINK_CAPABILITIES_REG(pex_if), tmp);

			/*
			 * If pex is X4, no need to pass thru the other
			 * 3X1 serdes lines
			 */
			if (info->pex_mode[pex_unit] == PEX_BUS_MODE_X4)
				line_num += 3;
		}
	}

	/*
	 * Step 7 [PEX-X4 Only] To create PEX-Link that contain 4-lanes you
	 * need to config the register SOC_Misc/General Purpose2
	 * (Address= 182F8)
	 */
	DEBUG_INIT_FULL_S("Step 7: [PEX-X4 Only] To create PEX-Link\n");
	tmp = reg_read(GEN_PURP_RES_2_REG);
	DEBUG_RD_REG(GEN_PURP_RES_2_REG, tmp);

	tmp &= 0xFFFF0000;
	if (info->pex_mode[0] == PEX_BUS_MODE_X4)
		tmp |= 0x0000000F;

	if (info->pex_mode[1] == PEX_BUS_MODE_X4)
		tmp |= 0x000000F0;

	if (info->pex_mode[2] == PEX_BUS_MODE_X4)
		tmp |= 0x00000F00;

	if (info->pex_mode[3] == PEX_BUS_MODE_X4)
		tmp |= 0x0000F000;

	reg_write(GEN_PURP_RES_2_REG, tmp);
	DEBUG_WR_REG(GEN_PURP_RES_2_REG, tmp);

	/* Steps  8 , 9 ,10 - use prepared REG addresses and values */
	DEBUG_INIT_FULL_S("Steps 7,8,9,10 and 11\n");

	/* Prepare PHY parameters for each step according to  MUX selection */
	for (line_num = 0; line_num < max_serdes_lines; line_num++) {
		/* for each serdes lane */

		line_cfg = get_line_cfg(line_num, info);

		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_UNCONNECTED])
			continue;

		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_PEX]) {
			pex_unit = line_num >> 2;
			pex_line_num = line_num % 4;

			if (info->pex_mode[pex_unit] == PEX_BUS_DISABLED)
				continue;
			/*
			 * 8)   Configure the desire PHY_MODE (bits [7:5])
			 * and REF_FREF_SEL (bits[4:0]) in the register Power
			 * and PLL Control (Each MAC contain different Access
			 * to reach its Serdes-Regfile).
			 */
			if (((info->pex_mode[pex_unit] == PEX_BUS_MODE_X4) &&
			     (0 == pex_line_num))
			    || ((info->pex_mode[pex_unit] == PEX_BUS_MODE_X1))) {
				reg_write(PEX_PHY_ACCESS_REG(pex_unit),
					  (0x01 << 16) | (pex_line_num << 24) |
					  0xFC60);
				DEBUG_WR_REG(PEX_PHY_ACCESS_REG(pex_unit),
					     (0x01 << 16) | (pex_line_num << 24)
					     | 0xFC60);
				/*
				 * Step 8.1: [PEX-Only] Configure Max PLL Rate
				 * (bit 8 in  KVCO Calibration Control and
				 * bits[10:9] in
				 */
				/* Use Maximum PLL Rate(Bit 8) */
				reg_write(PEX_PHY_ACCESS_REG(pex_unit),
					  (0x02 << 16) | (1 << 31) |
					  (pex_line_num << 24)); /* read command */
				DEBUG_WR_REG(PEX_PHY_ACCESS_REG(pex_unit),
					     (0x02 << 16) | (1 << 31) |
					     (pex_line_num << 24));
				tmp = reg_read(PEX_PHY_ACCESS_REG(pex_unit));
				DEBUG_RD_REG(PEX_PHY_ACCESS_REG(pex_unit), tmp);
				tmp &= ~(1 << 31);
				tmp |= (1 << 8);
				reg_write(PEX_PHY_ACCESS_REG(pex_unit), tmp);
				DEBUG_WR_REG(PEX_PHY_ACCESS_REG(pex_unit), tmp);

				/* Use Maximum PLL Rate(Bits [10:9]) */
				reg_write(PEX_PHY_ACCESS_REG(pex_unit),
					  (0x81 << 16) | (1 << 31) |
					  (pex_line_num << 24)); /* read command */
				DEBUG_WR_REG(PEX_PHY_ACCESS_REG(pex_unit),
					     (0x81 << 16) | (1 << 31) |
					     (pex_line_num << 24));
				tmp = reg_read(PEX_PHY_ACCESS_REG(pex_unit));
				DEBUG_RD_REG(PEX_PHY_ACCESS_REG(pex_unit), tmp);
				tmp &= ~(1 << 31);
				tmp |= (3 << 9);
				reg_write(PEX_PHY_ACCESS_REG(pex_unit), tmp);
				DEBUG_WR_REG(PEX_PHY_ACCESS_REG(pex_unit), tmp);
			}

			continue;
		}

		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SATA]) {
			/*
			 * Port 0 for serdes lines 4,6,  and port 1 for serdes
			 * lines 5
			 */
			sata_port = line_num & 1;

			/*
			 * 8) Configure the desire PHY_MODE (bits [7:5]) and
			 * REF_FREF_SEL (bits[4:0]) in the register Power
			 * and PLL Control (Each MAC contain different Access
			 * to reach its Serdes-Regfile).
			 */
			reg_write(SATA_PWR_PLL_CTRL_REG(sata_port), 0xF801);
			DEBUG_WR_REG(SATA_PWR_PLL_CTRL_REG(sata_port), 0xF801);

			/*  9)  Configure the desire SEL_BITS  */
			reg_write(SATA_DIG_LP_ENA_REG(sata_port), 0x400);
			DEBUG_WR_REG(SATA_DIG_LP_ENA_REG(sata_port), 0x400);

			/* 10)  Configure the desire REFCLK_SEL */

			reg_write(SATA_REF_CLK_SEL_REG(sata_port), 0x400);
			DEBUG_WR_REG(SATA_REF_CLK_SEL_REG(sata_port), 0x400);

			/* 11)  Power up to the PU_PLL,PU_RX,PU_TX.   */
			tmp = reg_read(SATA_LP_PHY_EXT_CTRL_REG(sata_port));
			DEBUG_RD_REG(SATA_LP_PHY_EXT_CTRL_REG(sata_port), tmp);
			tmp |= 7;
			reg_write(SATA_LP_PHY_EXT_CTRL_REG(sata_port), tmp);
			DEBUG_WR_REG(SATA_LP_PHY_EXT_CTRL_REG(sata_port), tmp);

			continue;
		}

		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_QSGMII]) {
			/*
			 * 8)   Configure the desire PHY_MODE (bits [7:5])
			 * and REF_FREF_SEL (bits[4:0]) in the register
			 */
			reg_write(SGMII_PWR_PLL_CTRL_REG(0), 0xF881);
			DEBUG_WR_REG(SGMII_PWR_PLL_CTRL_REG(0), 0xF881);

			/*
			 * 9)   Configure the desire SEL_BITS (bits [11:0]
			 * in register
			 */
			reg_write(SGMII_DIG_LP_ENA_REG(0), 0x400);
			DEBUG_WR_REG(SGMII_DIG_LP_ENA_REG(0), 0x400);

			/*
			 * 10)  Configure the desire REFCLK_SEL (bit [10])
			 * in register
			 */
			reg_write(SGMII_REF_CLK_SEL_REG(0), 0x400);
			DEBUG_WR_REG(SGMII_REF_CLK_SEL_REG(0), 0x400);

			/* 11)  Power up to the PU_PLL,PU_RX,PU_TX.  */
			tmp = reg_read(SGMII_SERDES_CFG_REG(0));
			DEBUG_RD_REG(SGMII_SERDES_CFG_REG(0), tmp);
			tmp |= 7;
			reg_write(SGMII_SERDES_CFG_REG(0), tmp);
			DEBUG_WR_REG(SGMII_SERDES_CFG_REG(0), tmp);
			continue;
		}

		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SGMII0])
			sgmii_port = 0;
		else if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SGMII1])
			sgmii_port = 1;
		else if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SGMII2])
			sgmii_port = 2;
		else if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SGMII3])
			sgmii_port = 3;
		else
			continue;

		/*
		 * 8)   Configure the desire PHY_MODE (bits [7:5]) and
		 * REF_FREF_SEL (bits[4:0]) in the register
		 */
		reg_write(SGMII_PWR_PLL_CTRL_REG(sgmii_port), 0xF881);
		DEBUG_WR_REG(SGMII_PWR_PLL_CTRL_REG(sgmii_port), 0xF881);

		/* 9)   Configure the desire SEL_BITS (bits [11:0] in register */
		reg_write(SGMII_DIG_LP_ENA_REG(sgmii_port), 0);
		DEBUG_WR_REG(SGMII_DIG_LP_ENA_REG(sgmii_port), 0);

		/* 10)  Configure the desire REFCLK_SEL (bit [10]) in register  */
		reg_write(SGMII_REF_CLK_SEL_REG(sgmii_port), 0x400);
		DEBUG_WR_REG(SGMII_REF_CLK_SEL_REG(sgmii_port), 0x400);

		/* 11)  Power up to the PU_PLL,PU_RX,PU_TX.  */
		tmp = reg_read(SGMII_SERDES_CFG_REG(sgmii_port));
		DEBUG_RD_REG(SGMII_SERDES_CFG_REG(sgmii_port), tmp);
		tmp |= 7;
		reg_write(SGMII_SERDES_CFG_REG(sgmii_port), tmp);
		DEBUG_WR_REG(SGMII_SERDES_CFG_REG(sgmii_port), tmp);

	}			/* for each serdes lane */

	/* Step 12 [PEX-Only] Last phase of PEX-PIPE Configuration */
	DEBUG_INIT_FULL_S("Steps 12: [PEX-Only] Last phase of PEX-PIPE Configuration\n");
	for (line_num = 0; line_num < max_serdes_lines; line_num++) {
		/* for each serdes lane */

		line_cfg = get_line_cfg(line_num, info);

		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_UNCONNECTED])
			continue;

		if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_PEX]) {
			pex_unit = line_num >> 2;
			pex_line_num = line_num % 4;
			if (0 == pex_line_num) {
				/*
				 * Configure the detection pulse with before
				 * the reset is deasserted
				 */

				/* Read the old value (indirect access) */
				reg_write(PEX_PHY_ACCESS_REG(pex_unit),
					  (0x48 << 16) | (1 << 31) |
					  (pex_line_num << 24));
				tmp = reg_read(PEX_PHY_ACCESS_REG(pex_unit));
				tmp &= ~(1 << 31);	/* Clear read */
				tmp &= ~(3 << 6);	/* Mask width */
				/* Insert new detection pulse width */
				tmp |= serdes_pex_pulse_width[pex_unit] << 6;
				/* Write value back */
				reg_write(PEX_PHY_ACCESS_REG(pex_unit), tmp);

				reg_write(PEX_PHY_ACCESS_REG(pex_unit),
					  (0xC1 << 16) | 0x24);
				DEBUG_WR_REG(PEX_PHY_ACCESS_REG(pex_unit),
					     (0xC1 << 16) | 0x24);
			}
		}
	}

	/*--------------------------------------------------------------*/
	/* Step 13: Wait 15ms before checking results */
	DEBUG_INIT_FULL_S("Steps 13: Wait 15ms before checking results");
	mdelay(15);
	tmp = 20;
	while (tmp) {
		status = MV_OK;
		for (line_num = 0; line_num < max_serdes_lines; line_num++) {
			u32 tmp;
			line_cfg = get_line_cfg(line_num, info);
			if (line_cfg ==
			    serdes_cfg[line_num][SERDES_UNIT_UNCONNECTED])
				continue;

			if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_PEX])
				continue;

			if (line_cfg == serdes_cfg[line_num][SERDES_UNIT_SATA]) {
				/*
				 * Port 0 for serdes lines 4,6,  and port 1
				 * for serdes lines 5
				 */
				sata_port = line_num & 1;

				tmp =
				    reg_read(SATA_LP_PHY_EXT_STAT_REG
					     (sata_port));
				DEBUG_RD_REG(SATA_LP_PHY_EXT_STAT_REG
					     (sata_port), tmp);
				if ((tmp & 0x7) != 0x7)
					status = MV_ERROR;
				continue;
			}

			if (line_cfg ==
			    serdes_cfg[line_num][SERDES_UNIT_QSGMII]) {
				tmp = reg_read(SGMII_SERDES_STAT_REG(0));
				DEBUG_RD_REG(SGMII_SERDES_STAT_REG(0), tmp);
				if ((tmp & 0x7) != 0x7)
					status = MV_ERROR;
				continue;
			}

			if (line_cfg ==
			    serdes_cfg[line_num][SERDES_UNIT_SGMII0])
				sgmii_port = 0;
			else if (line_cfg ==
				 serdes_cfg[line_num][SERDES_UNIT_SGMII1])
				sgmii_port = 1;
			else if (line_cfg ==
				 serdes_cfg[line_num][SERDES_UNIT_SGMII2])
				sgmii_port = 2;
			else if (line_cfg ==
				 serdes_cfg[line_num][SERDES_UNIT_SGMII3])
				sgmii_port = 3;
			else
				continue;

			tmp = reg_read(SGMII_SERDES_STAT_REG(sgmii_port));
			DEBUG_RD_REG(SGMII_SERDES_STAT_REG(sgmii_port), tmp);
			if ((tmp & 0x7) != 0x7)
				status = MV_ERROR;
		}

		if (status == MV_OK)
			break;
		mdelay(5);
		tmp--;
	}

	/*
	 * Step14 [PEX-Only]  In order to configure RC/EP mode please write
	 * to register 0x0060 bits
	 */
	DEBUG_INIT_FULL_S("Steps 14: [PEX-Only]  In order to configure\n");
	for (pex_unit = 0; pex_unit < pex_max_unit_get(); pex_unit++) {
		if (info->pex_mode[pex_unit] == PEX_BUS_DISABLED)
			continue;
		tmp =
		    reg_read(PEX_CAPABILITIES_REG(MV_PEX_UNIT_TO_IF(pex_unit)));
		DEBUG_RD_REG(PEX_CAPABILITIES_REG(MV_PEX_UNIT_TO_IF(pex_unit)),
			     tmp);
		tmp &= ~(0xf << 20);
		if (info->pex_type == MV_PEX_ROOT_COMPLEX)
			tmp |= (0x4 << 20);
		else
			tmp |= (0x1 << 20);
		reg_write(PEX_CAPABILITIES_REG(MV_PEX_UNIT_TO_IF(pex_unit)),
			  tmp);
		DEBUG_WR_REG(PEX_CAPABILITIES_REG(MV_PEX_UNIT_TO_IF(pex_unit)),
			     tmp);
	}

	/*
	 * Step 15 [PEX-Only] Only for EP mode set to Zero bits 19 and 16 of
	 * register 0x1a60
	 */
	DEBUG_INIT_FULL_S("Steps 15: [PEX-Only]  In order to configure\n");
	for (pex_unit = 0; pex_unit < pex_max_unit_get(); pex_unit++) {
		if (info->pex_mode[pex_unit] == PEX_BUS_DISABLED)
			continue;
		if (info->pex_type == MV_PEX_END_POINT) {
			tmp =
			    reg_read(PEX_DBG_CTRL_REG
				     (MV_PEX_UNIT_TO_IF(pex_unit)));
			DEBUG_RD_REG(PEX_DBG_CTRL_REG
				     (MV_PEX_UNIT_TO_IF(pex_unit)), tmp);
			tmp &= 0xfff6ffff;
			reg_write(PEX_DBG_CTRL_REG(MV_PEX_UNIT_TO_IF(pex_unit)),
				  tmp);
			DEBUG_WR_REG(PEX_DBG_CTRL_REG
				     (MV_PEX_UNIT_TO_IF(pex_unit)), tmp);
		}
	}

	if (info->serdes_m_phy_change) {
		MV_SERDES_CHANGE_M_PHY *serdes_m_phy_change;
		u32 bus_speed;
		for (line_num = 0; line_num < max_serdes_lines; line_num++) {
			line_cfg = get_line_cfg(line_num, info);
			if (line_cfg ==
			    serdes_cfg[line_num][SERDES_UNIT_UNCONNECTED])
				continue;
			serdes_m_phy_change = info->serdes_m_phy_change;
			bus_speed = info->bus_speed & (1 << line_num);
			while (serdes_m_phy_change->type !=
			       SERDES_UNIT_UNCONNECTED) {
				switch (serdes_m_phy_change->type) {
				case SERDES_UNIT_PEX:
					if (line_cfg != SERDES_UNIT_PEX)
						break;
					pex_unit = line_num >> 2;
					pex_line_num = line_num % 4;
					if (info->pex_mode[pex_unit] ==
					    PEX_BUS_DISABLED)
						break;
					if ((info->pex_mode[pex_unit] ==
					     PEX_BUS_MODE_X4) && pex_line_num)
						break;

					if (bus_speed) {
						reg_write(PEX_PHY_ACCESS_REG
							  (pex_unit),
							  (pex_line_num << 24) |
							  serdes_m_phy_change->val_hi_speed);
						DEBUG_WR_REG(PEX_PHY_ACCESS_REG
							     (pex_unit),
							     (pex_line_num <<
							      24) |
							     serdes_m_phy_change->val_hi_speed);
					} else {
						reg_write(PEX_PHY_ACCESS_REG
							  (pex_unit),
							  (pex_line_num << 24) |
							  serdes_m_phy_change->val_low_speed);
						DEBUG_WR_REG(PEX_PHY_ACCESS_REG
							     (pex_unit),
							     (pex_line_num <<
							      24) |
							     serdes_m_phy_change->val_low_speed);
					}
					break;
				case SERDES_UNIT_SATA:
					if (line_cfg != SERDES_UNIT_SATA)
						break;
					/*
					 * Port 0 for serdes lines 4,6,  and
					 * port 1 for serdes lines 5
					 */
					sata_port = line_num & 1;
					if (bus_speed) {
						reg_write(SATA_BASE_REG
							  (sata_port) |
							  serdes_m_phy_change->reg_hi_speed,
							  serdes_m_phy_change->val_hi_speed);
						DEBUG_WR_REG(SATA_BASE_REG
							     (sata_port) |
							     serdes_m_phy_change->reg_hi_speed,
							     serdes_m_phy_change->val_hi_speed);
					} else {
						reg_write(SATA_BASE_REG
							  (sata_port) |
							  serdes_m_phy_change->reg_low_speed,
							  serdes_m_phy_change->val_low_speed);
						DEBUG_WR_REG(SATA_BASE_REG
							     (sata_port) |
							     serdes_m_phy_change->reg_low_speed,
							     serdes_m_phy_change->val_low_speed);
					}
					break;
				case SERDES_UNIT_SGMII0:
				case SERDES_UNIT_SGMII1:
				case SERDES_UNIT_SGMII2:
				case SERDES_UNIT_SGMII3:
					if (line_cfg == serdes_cfg[line_num]
					    [SERDES_UNIT_SGMII0])
						sgmii_port = 0;
					else if (line_cfg ==
						 serdes_cfg[line_num]
						 [SERDES_UNIT_SGMII1])
						sgmii_port = 1;
					else if (line_cfg ==
						 serdes_cfg[line_num]
						 [SERDES_UNIT_SGMII2])
						sgmii_port = 2;
					else if (line_cfg ==
						 serdes_cfg[line_num]
						 [SERDES_UNIT_SGMII3])
						sgmii_port = 3;
					else
						break;
					if (bus_speed) {
						reg_write(MV_ETH_REGS_BASE
							  (sgmii_port) |
							  serdes_m_phy_change->reg_hi_speed,
							  serdes_m_phy_change->val_hi_speed);
						DEBUG_WR_REG(MV_ETH_REGS_BASE
							     (sgmii_port) |
							     serdes_m_phy_change->reg_hi_speed,
							     serdes_m_phy_change->val_hi_speed);
					} else {
						reg_write(MV_ETH_REGS_BASE
							  (sgmii_port) |
							  serdes_m_phy_change->reg_low_speed,
							  serdes_m_phy_change->val_low_speed);
						DEBUG_WR_REG(MV_ETH_REGS_BASE
							     (sgmii_port) |
							     serdes_m_phy_change->reg_low_speed,
							     serdes_m_phy_change->val_low_speed);
					}
					break;
				case SERDES_UNIT_QSGMII:
					if (line_cfg != SERDES_UNIT_QSGMII)
						break;
					if (bus_speed) {
						reg_write
						    (serdes_m_phy_change->reg_hi_speed,
						     serdes_m_phy_change->val_hi_speed);
						DEBUG_WR_REG
						    (serdes_m_phy_change->reg_hi_speed,
						     serdes_m_phy_change->val_hi_speed);
					} else {
						reg_write
						    (serdes_m_phy_change->reg_low_speed,
						     serdes_m_phy_change->val_low_speed);
						DEBUG_WR_REG
						    (serdes_m_phy_change->reg_low_speed,
						     serdes_m_phy_change->val_low_speed);
					}
					break;
				default:
					break;
				}
				serdes_m_phy_change++;
			}
		}
	}

	/* Step 16 [PEX-Only] Training Enable */
	DEBUG_INIT_FULL_S("Steps 16: [PEX-Only] Training Enable");
	tmp = reg_read(SOC_CTRL_REG);
	DEBUG_RD_REG(SOC_CTRL_REG, tmp);
	tmp &= ~(0x0F);
	for (pex_unit = 0; pex_unit < pex_max_unit_get(); pex_unit++) {
		reg_write(PEX_CAUSE_REG(pex_unit), 0);
		DEBUG_WR_REG(PEX_CAUSE_REG(pex_unit), 0);
		if (info->pex_mode[pex_unit] != PEX_BUS_DISABLED)
			tmp |= (0x1 << pex_unit);
	}
	reg_write(SOC_CTRL_REG, tmp);
	DEBUG_WR_REG(SOC_CTRL_REG, tmp);

	/* Step 17: Speed change to target speed and width */
	{
		u32 tmp_reg, tmp_pex_reg;
		u32 addr;
		u32 first_busno, next_busno;
		u32 max_link_width = 0;
		u32 neg_link_width = 0;
		pex_if_num = pex_max_if_get();
		mdelay(150);
		DEBUG_INIT_FULL_C("step 17: max_if= 0x", pex_if_num, 1);
		next_busno = 0;
		for (pex_if = 0; pex_if < pex_if_num; pex_if++) {
			line_num = (pex_if <= 8) ? pex_if : 12;
			line_cfg = get_line_cfg(line_num, info);
			if (line_cfg != serdes_cfg[line_num][SERDES_UNIT_PEX])
				continue;
			pex_unit = (pex_if < 9) ? (pex_if >> 2) : 3;
			DEBUG_INIT_FULL_S("step 17:  PEX");
			DEBUG_INIT_FULL_D(pex_if, 1);
			DEBUG_INIT_FULL_C("  pex_unit= ", pex_unit, 1);

			if (info->pex_mode[pex_unit] == PEX_BUS_DISABLED) {
				DEBUG_INIT_FULL_C("PEX disabled interface ",
						  pex_if, 1);
				if (pex_if < 8)
					pex_if += 3;
				continue;
			}
			first_busno = next_busno;
			if ((info->pex_type == MV_PEX_END_POINT) &&
			    (0 == pex_if)) {
				if ((pex_if < 8) && (info->pex_mode[pex_unit] ==
						     PEX_BUS_MODE_X4))
					pex_if += 3;
				continue;
			}

			tmp = reg_read(PEX_DBG_STATUS_REG(pex_if));
			DEBUG_RD_REG(PEX_DBG_STATUS_REG(pex_if), tmp);
			if ((tmp & 0x7f) == 0x7e) {
				next_busno++;
				tmp = reg_read(PEX_LINK_CAPABILITIES_REG(pex_if));
				max_link_width = tmp;
				DEBUG_RD_REG((PEX_LINK_CAPABILITIES_REG
					      (pex_if)), tmp);
				max_link_width = ((max_link_width >> 4) & 0x3F);
				neg_link_width =
				    reg_read(PEX_LINK_CTRL_STATUS_REG(pex_if));
				DEBUG_RD_REG((PEX_LINK_CTRL_STATUS_REG(pex_if)),
					     neg_link_width);
				neg_link_width = ((neg_link_width >> 20) & 0x3F);
				if (max_link_width > neg_link_width) {
					tmp &= ~(0x3F << 4);
					tmp |= (neg_link_width << 4);
					reg_write(PEX_LINK_CAPABILITIES_REG
						  (pex_if), tmp);
					DEBUG_WR_REG((PEX_LINK_CAPABILITIES_REG
						      (pex_if)), tmp);
					mdelay(1);	/* wait 1ms before reading  capability for speed */
					DEBUG_INIT_S("PEX");
					DEBUG_INIT_D(pex_if, 1);
					DEBUG_INIT_C(": change width to X",
						     neg_link_width, 1);
				}
				tmp_pex_reg =
				    reg_read((PEX_CFG_DIRECT_ACCESS
					      (pex_if,
					       PEX_LINK_CAPABILITY_REG)));
				DEBUG_RD_REG((PEX_CFG_DIRECT_ACCESS
					      (pex_if,
					       PEX_LINK_CAPABILITY_REG)),
					     tmp_pex_reg);
				tmp_pex_reg &= (0xF);
				if (tmp_pex_reg == 0x2) {
					tmp_reg =
					    (reg_read
					     (PEX_CFG_DIRECT_ACCESS
					      (pex_if,
					       PEX_LINK_CTRL_STAT_REG)) &
					     0xF0000) >> 16;
					DEBUG_RD_REG(PEX_CFG_DIRECT_ACCESS
						     (pex_if,
						      PEX_LINK_CTRL_STAT_REG),
						     tmp_pex_reg);
					/* check if the link established is GEN1 */
					if (tmp_reg == 0x1) {
						pex_local_bus_num_set(pex_if,
								      first_busno);
						pex_local_dev_num_set(pex_if,
								      1);

						DEBUG_INIT_FULL_S("** Link is Gen1, check the EP capability\n");
						/* link is Gen1, check the EP capability */
						addr =
						    pex_cfg_read(pex_if,
								 first_busno, 0,
								 0,
								 0x34) & 0xFF;
						DEBUG_INIT_FULL_C("pex_cfg_read: return addr=0x%x",
						     addr, 4);
						if (addr == 0xff) {
							DEBUG_INIT_FULL_C("pex_cfg_read: return 0xff -->PEX (%d): Detected No Link.",
									  pex_if, 1);
							continue;
						}
						while ((pex_cfg_read
							(pex_if, first_busno, 0,
							 0,
							 addr) & 0xFF) !=
						       0x10) {
							addr =
							    (pex_cfg_read
							     (pex_if,
							      first_busno, 0, 0,
							      addr) & 0xFF00) >>
							    8;
						}
						if ((pex_cfg_read
						     (pex_if, first_busno, 0, 0,
						      addr + 0xC) & 0xF) >=
						    0x2) {
							tmp =
							    reg_read
							    (PEX_LINK_CTRL_STATUS2_REG
							     (pex_if));
							DEBUG_RD_REG
							    (PEX_LINK_CTRL_STATUS2_REG
							     (pex_if), tmp);
							tmp &= ~(0x1 | 1 << 1);
							tmp |= (1 << 1);
							reg_write
							    (PEX_LINK_CTRL_STATUS2_REG
							     (pex_if), tmp);
							DEBUG_WR_REG
							    (PEX_LINK_CTRL_STATUS2_REG
							     (pex_if), tmp);

							tmp =
							    reg_read
							    (PEX_CTRL_REG
							     (pex_if));
							DEBUG_RD_REG
							    (PEX_CTRL_REG
							     (pex_if), tmp);
							tmp |= (1 << 10);
							reg_write(PEX_CTRL_REG
								  (pex_if),
								  tmp);
							DEBUG_WR_REG
							    (PEX_CTRL_REG
							     (pex_if), tmp);
							mdelay(10);	/* We need to wait 10ms before reading the PEX_DBG_STATUS_REG in order not to read the status of the former state */
							DEBUG_INIT_FULL_S
							    ("Gen2 client!\n");
						} else {
							DEBUG_INIT_FULL_S
							    ("GEN1 client!\n");
						}
					}
				}
			} else {
				DEBUG_INIT_FULL_S("PEX");
				DEBUG_INIT_FULL_D(pex_if, 1);
				DEBUG_INIT_FULL_S(" : Detected No Link. Status Reg(0x");
				DEBUG_INIT_FULL_D(PEX_DBG_STATUS_REG(pex_if),
						  8);
				DEBUG_INIT_FULL_C(") = 0x", tmp, 8);
			}

			if ((pex_if < 8) &&
			    (info->pex_mode[pex_unit] == PEX_BUS_MODE_X4))
				pex_if += 3;
		}
	}

	/* Step 18: update pex DEVICE ID */
	{
		u32 devId;
		pex_if_num = pex_max_if_get();
		ctrl_mode = ctrl_model_get();
		for (pex_if = 0; pex_if < pex_if_num; pex_if++) {
			pex_unit = (pex_if < 9) ? (pex_if >> 2) : 3;
			if (info->pex_mode[pex_unit] == PEX_BUS_DISABLED) {
				if ((pex_if < 8) &&
				    (info->pex_mode[pex_unit] == PEX_BUS_MODE_X4))
					pex_if += 3;
				continue;
			}

			devId = reg_read(PEX_CFG_DIRECT_ACCESS(
						 pex_if, PEX_DEVICE_AND_VENDOR_ID));
			devId &= 0xFFFF;
			devId |= ((ctrl_mode << 16) & 0xffff0000);
			DEBUG_INIT_FULL_S("Update Device ID PEX");
			DEBUG_INIT_FULL_D(pex_if, 1);
			DEBUG_INIT_FULL_D(devId, 8);
			DEBUG_INIT_FULL_S("\n");
			reg_write(PEX_CFG_DIRECT_ACCESS
				  (pex_if, PEX_DEVICE_AND_VENDOR_ID), devId);
			if ((pex_if < 8) &&
			    (info->pex_mode[pex_unit] == PEX_BUS_MODE_X4))
				pex_if += 3;
		}
		DEBUG_INIT_FULL_S("Update PEX Device ID 0x");
		DEBUG_INIT_FULL_D(ctrl_mode, 4);
		DEBUG_INIT_FULL_S("0\n");
	}
	tmp = reg_read(PEX_DBG_STATUS_REG(0));
	DEBUG_RD_REG(PEX_DBG_STATUS_REG(0), tmp);

	DEBUG_INIT_S(ENDED_OK);
	return MV_OK;
}

/* PEX configuration space read write */

/*
 * pex_cfg_read - Read from configuration space
 *
 * DESCRIPTION:
 *       This function performs a 32 bit read from PEX configuration space.
 *       It supports both type 0 and type 1 of Configuration Transactions
 *       (local and over bridge). In order to read from local bus segment, use
 *       bus number retrieved from mvPexLocalBusNumGet(). Other bus numbers
 *       will result configuration transaction of type 1 (over bridge).
 *
 * INPUT:
 *       pex_if   - PEX interface number.
 *       bus     - PEX segment bus number.
 *       dev     - PEX device number.
 *       func    - Function number.
 *       offss - Register offset.
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       32bit register data, 0xffffffff on error
 *
 */
u32 pex_cfg_read(u32 pex_if, u32 bus, u32 dev, u32 func, u32 offs)
{
	u32 pex_data = 0;
	u32 local_dev, local_bus;
	u32 val;

	if (pex_if >= MV_PEX_MAX_IF)
		return 0xFFFFFFFF;

	if (dev >= MAX_PEX_DEVICES) {
		DEBUG_INIT_C("pex_cfg_read: ERR. device number illigal ", dev,
			     1);
		return 0xFFFFFFFF;
	}

	if (func >= MAX_PEX_FUNCS) {
		DEBUG_INIT_C("pex_cfg_read: ERR. function num illigal ", func,
			     1);
		return 0xFFFFFFFF;
	}

	if (bus >= MAX_PEX_BUSSES) {
		DEBUG_INIT_C("pex_cfg_read: ERR. bus number illigal ", bus, 1);
		return MV_ERROR;
	}
	val = reg_read(PEX_STATUS_REG(pex_if));

	local_dev =
	    ((val & PXSR_PEX_DEV_NUM_MASK) >> PXSR_PEX_DEV_NUM_OFFS);
	local_bus =
	    ((val & PXSR_PEX_BUS_NUM_MASK) >> PXSR_PEX_BUS_NUM_OFFS);

	/* Speed up the process. In case on no link, return MV_ERROR */
	if ((dev != local_dev) || (bus != local_bus)) {
		pex_data = reg_read(PEX_STATUS_REG(pex_if));

		if ((pex_data & PXSR_DL_DOWN))
			return MV_ERROR;
	}

	/*
	 * In PCI Express we have only one device number
	 * and this number is the first number we encounter else that the
	 * local_dev spec pex define return on config read/write on any device
	 */
	if (bus == local_bus) {
		if (local_dev == 0) {
			/*
			 * If local dev is 0 then the first number we encounter
			 * after 0 is 1
			 */
			if ((dev != 1) && (dev != local_dev))
				return MV_ERROR;
		} else {
			/*
			 * If local dev is not 0 then the first number we
			 * encounter is 0
			 */
			if ((dev != 0) && (dev != local_dev))
				return MV_ERROR;
		}
	}

	/* Creating PEX address to be passed */
	pex_data = (bus << PXCAR_BUS_NUM_OFFS);
	pex_data |= (dev << PXCAR_DEVICE_NUM_OFFS);
	pex_data |= (func << PXCAR_FUNC_NUM_OFFS);
	pex_data |= (offs & PXCAR_REG_NUM_MASK);	/* lgacy register space */
	/* extended register space */
	pex_data |= (((offs & PXCAR_REAL_EXT_REG_NUM_MASK) >>
		     PXCAR_REAL_EXT_REG_NUM_OFFS) << PXCAR_EXT_REG_NUM_OFFS);

	pex_data |= PXCAR_CONFIG_EN;

	/* Write the address to the PEX configuration address register */
	reg_write(PEX_CFG_ADDR_REG(pex_if), pex_data);

	/*
	 * In order to let the PEX controller absorbed the address of the read
	 * transaction we perform a validity check that the address was written
	 */
	if (pex_data != reg_read(PEX_CFG_ADDR_REG(pex_if)))
		return MV_ERROR;

	/* cleaning Master Abort */
	reg_bit_set(PEX_CFG_DIRECT_ACCESS(pex_if, PEX_STATUS_AND_COMMAND),
		    PXSAC_MABORT);
	/* Read the Data returned in the PEX Data register */
	pex_data = reg_read(PEX_CFG_DATA_REG(pex_if));

	DEBUG_INIT_FULL_C(" --> ", pex_data, 4);

	return pex_data;
}

/*
 * pex_local_bus_num_set - Set PEX interface local bus number.
 *
 * DESCRIPTION:
 *       This function sets given PEX interface its local bus number.
 *       Note: In case the PEX interface is PEX-X, the information is read-only.
 *
 * INPUT:
 *       pex_if  - PEX interface number.
 *       bus_num - Bus number.
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       MV_NOT_ALLOWED in case PEX interface is PEX-X.
 *		MV_BAD_PARAM on bad parameters ,
 *       otherwise MV_OK
 *
 */
int pex_local_bus_num_set(u32 pex_if, u32 bus_num)
{
	u32 val;

	if (bus_num >= MAX_PEX_BUSSES) {
		DEBUG_INIT_C("pex_local_bus_num_set: ERR. bus number illigal %d\n",
		     bus_num, 4);
		return MV_ERROR;
	}

	val = reg_read(PEX_STATUS_REG(pex_if));
	val &= ~PXSR_PEX_BUS_NUM_MASK;
	val |= (bus_num << PXSR_PEX_BUS_NUM_OFFS) & PXSR_PEX_BUS_NUM_MASK;
	reg_write(PEX_STATUS_REG(pex_if), val);

	return MV_OK;
}

/*
 * pex_local_dev_num_set - Set PEX interface local device number.
 *
 * DESCRIPTION:
 *       This function sets given PEX interface its local device number.
 *       Note: In case the PEX interface is PEX-X, the information is read-only.
 *
 * INPUT:
 *       pex_if  - PEX interface number.
 *       dev_num - Device number.
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       MV_NOT_ALLOWED in case PEX interface is PEX-X.
 *		MV_BAD_PARAM on bad parameters ,
 *       otherwise MV_OK
 *
 */
int pex_local_dev_num_set(u32 pex_if, u32 dev_num)
{
	u32 val;

	if (pex_if >= MV_PEX_MAX_IF)
		return MV_BAD_PARAM;

	val = reg_read(PEX_STATUS_REG(pex_if));
	val &= ~PXSR_PEX_DEV_NUM_MASK;
	val |= (dev_num << PXSR_PEX_DEV_NUM_OFFS) & PXSR_PEX_DEV_NUM_MASK;
	reg_write(PEX_STATUS_REG(pex_if), val);

	return MV_OK;
}
