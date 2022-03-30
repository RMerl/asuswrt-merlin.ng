// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Lokesh Vutla <lokeshvutla@ti.com>
 *
 * Based on previous work by:
 * Aneesh V       <aneesh@ti.com>
 * Steve Sakoman  <steve@sakoman.com>
 */
#include <common.h>
#include <palmas.h>
#include <sata.h>
#include <linux/string.h>
#include <asm/gpio.h>
#include <usb.h>
#include <linux/usb/gadget.h>
#include <asm/omap_common.h>
#include <asm/omap_sec_common.h>
#include <asm/arch/gpio.h>
#include <asm/arch/dra7xx_iodelay.h>
#include <asm/emif.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sata.h>
#include <environment.h>
#include <dwc3-uboot.h>
#include <dwc3-omap-uboot.h>
#include <i2c.h>
#include <ti-usb-phy-uboot.h>
#include <miiphy.h>

#include "mux_data.h"
#include "../common/board_detect.h"

#define board_is_dra76x_evm()		board_ti_is("DRA76/7x")
#define board_is_dra74x_evm()		board_ti_is("5777xCPU")
#define board_is_dra72x_evm()		board_ti_is("DRA72x-T")
#define board_is_dra71x_evm()		board_ti_is("DRA79x,D")
#define board_is_dra74x_revh_or_later() (board_is_dra74x_evm() &&	\
				(strncmp("H", board_ti_get_rev(), 1) <= 0))
#define board_is_dra72x_revc_or_later() (board_is_dra72x_evm() &&	\
				(strncmp("C", board_ti_get_rev(), 1) <= 0))
#define board_ti_get_emif_size()	board_ti_get_emif1_size() +	\
					board_ti_get_emif2_size()

#ifdef CONFIG_DRIVER_TI_CPSW
#include <cpsw.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* GPIO 7_11 */
#define GPIO_DDR_VTT_EN 203

#define SYSINFO_BOARD_NAME_MAX_LEN	37

/* I2C I/O Expander */
#define NAND_PCF8575_ADDR	0x21
#define NAND_PCF8575_I2C_BUS_NUM	0

const struct omap_sysinfo sysinfo = {
	"Board: UNKNOWN(DRA7 EVM) REV UNKNOWN\n"
};

static const struct emif_regs emif1_ddr3_532_mhz_1cs = {
	.sdram_config_init              = 0x61851ab2,
	.sdram_config                   = 0x61851ab2,
	.sdram_config2			= 0x08000000,
	.ref_ctrl                       = 0x000040F1,
	.ref_ctrl_final			= 0x00001035,
	.sdram_tim1                     = 0xCCCF36B3,
	.sdram_tim2                     = 0x308F7FDA,
	.sdram_tim3                     = 0x427F88A8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x0007190B,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0024400B,
	.emif_ddr_phy_ctlr_1            = 0x0E24400B,
	.emif_ddr_ext_phy_ctrl_1        = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x00910091,
	.emif_ddr_ext_phy_ctrl_3        = 0x00950095,
	.emif_ddr_ext_phy_ctrl_4        = 0x009B009B,
	.emif_ddr_ext_phy_ctrl_5        = 0x009E009E,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

static const struct emif_regs emif2_ddr3_532_mhz_1cs = {
	.sdram_config_init              = 0x61851B32,
	.sdram_config                   = 0x61851B32,
	.sdram_config2			= 0x08000000,
	.ref_ctrl                       = 0x000040F1,
	.ref_ctrl_final			= 0x00001035,
	.sdram_tim1                     = 0xCCCF36B3,
	.sdram_tim2                     = 0x308F7FDA,
	.sdram_tim3                     = 0x427F88A8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x0007190B,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0024400B,
	.emif_ddr_phy_ctlr_1            = 0x0E24400B,
	.emif_ddr_ext_phy_ctrl_1        = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x00910091,
	.emif_ddr_ext_phy_ctrl_3        = 0x00950095,
	.emif_ddr_ext_phy_ctrl_4        = 0x009B009B,
	.emif_ddr_ext_phy_ctrl_5        = 0x009E009E,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

static const struct emif_regs emif_1_regs_ddr3_666_mhz_1cs_dra_es1 = {
	.sdram_config_init              = 0x61862B32,
	.sdram_config                   = 0x61862B32,
	.sdram_config2			= 0x08000000,
	.ref_ctrl                       = 0x0000514C,
	.ref_ctrl_final			= 0x0000144A,
	.sdram_tim1                     = 0xD113781C,
	.sdram_tim2                     = 0x30717FE3,
	.sdram_tim3                     = 0x409F86A8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x5007190B,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0024400D,
	.emif_ddr_phy_ctlr_1            = 0x0E24400D,
	.emif_ddr_ext_phy_ctrl_1        = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x00A400A4,
	.emif_ddr_ext_phy_ctrl_3        = 0x00A900A9,
	.emif_ddr_ext_phy_ctrl_4        = 0x00B000B0,
	.emif_ddr_ext_phy_ctrl_5        = 0x00B000B0,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

const struct emif_regs emif_1_regs_ddr3_666_mhz_1cs_dra_es2 = {
	.sdram_config_init              = 0x61862BB2,
	.sdram_config                   = 0x61862BB2,
	.sdram_config2			= 0x00000000,
	.ref_ctrl                       = 0x0000514D,
	.ref_ctrl_final			= 0x0000144A,
	.sdram_tim1                     = 0xD1137824,
	.sdram_tim2                     = 0x30B37FE3,
	.sdram_tim3                     = 0x409F8AD8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x5007190B,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0824400E,
	.emif_ddr_phy_ctlr_1            = 0x0E24400E,
	.emif_ddr_ext_phy_ctrl_1        = 0x04040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x006B009F,
	.emif_ddr_ext_phy_ctrl_3        = 0x006B00A2,
	.emif_ddr_ext_phy_ctrl_4        = 0x006B00A8,
	.emif_ddr_ext_phy_ctrl_5        = 0x006B00A8,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

const struct emif_regs emif1_ddr3_532_mhz_1cs_2G = {
	.sdram_config_init              = 0x61851ab2,
	.sdram_config                   = 0x61851ab2,
	.sdram_config2			= 0x08000000,
	.ref_ctrl                       = 0x000040F1,
	.ref_ctrl_final			= 0x00001035,
	.sdram_tim1                     = 0xCCCF36B3,
	.sdram_tim2                     = 0x30BF7FDA,
	.sdram_tim3                     = 0x427F8BA8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x0007190B,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0024400B,
	.emif_ddr_phy_ctlr_1            = 0x0E24400B,
	.emif_ddr_ext_phy_ctrl_1        = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x00910091,
	.emif_ddr_ext_phy_ctrl_3        = 0x00950095,
	.emif_ddr_ext_phy_ctrl_4        = 0x009B009B,
	.emif_ddr_ext_phy_ctrl_5        = 0x009E009E,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

const struct emif_regs emif2_ddr3_532_mhz_1cs_2G = {
	.sdram_config_init              = 0x61851B32,
	.sdram_config                   = 0x61851B32,
	.sdram_config2			= 0x08000000,
	.ref_ctrl                       = 0x000040F1,
	.ref_ctrl_final			= 0x00001035,
	.sdram_tim1                     = 0xCCCF36B3,
	.sdram_tim2                     = 0x308F7FDA,
	.sdram_tim3                     = 0x427F88A8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x0007190B,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0024400B,
	.emif_ddr_phy_ctlr_1            = 0x0E24400B,
	.emif_ddr_ext_phy_ctrl_1        = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x00910091,
	.emif_ddr_ext_phy_ctrl_3        = 0x00950095,
	.emif_ddr_ext_phy_ctrl_4        = 0x009B009B,
	.emif_ddr_ext_phy_ctrl_5        = 0x009E009E,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

const struct emif_regs emif_1_regs_ddr3_666_mhz_1cs_dra76 = {
	.sdram_config_init              = 0x61862B32,
	.sdram_config                   = 0x61862B32,
	.sdram_config2			= 0x00000000,
	.ref_ctrl                       = 0x0000514C,
	.ref_ctrl_final			= 0x0000144A,
	.sdram_tim1                     = 0xD113783C,
	.sdram_tim2                     = 0x30B47FE3,
	.sdram_tim3                     = 0x409F8AD8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x5007190B,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0824400D,
	.emif_ddr_phy_ctlr_1            = 0x0E24400D,
	.emif_ddr_ext_phy_ctrl_1        = 0x04040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x006B009F,
	.emif_ddr_ext_phy_ctrl_3        = 0x006B00A2,
	.emif_ddr_ext_phy_ctrl_4        = 0x006B00A8,
	.emif_ddr_ext_phy_ctrl_5        = 0x006B00A8,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

const struct emif_regs emif_2_regs_ddr3_666_mhz_1cs_dra76 = {
	.sdram_config_init              = 0x61862B32,
	.sdram_config                   = 0x61862B32,
	.sdram_config2			= 0x00000000,
	.ref_ctrl                       = 0x0000514C,
	.ref_ctrl_final			= 0x0000144A,
	.sdram_tim1                     = 0xD113781C,
	.sdram_tim2                     = 0x30B47FE3,
	.sdram_tim3                     = 0x409F8AD8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x5007190B,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0824400D,
	.emif_ddr_phy_ctlr_1            = 0x0E24400D,
	.emif_ddr_ext_phy_ctrl_1        = 0x04040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x006B009F,
	.emif_ddr_ext_phy_ctrl_3        = 0x006B00A2,
	.emif_ddr_ext_phy_ctrl_4        = 0x006B00A8,
	.emif_ddr_ext_phy_ctrl_5        = 0x006B00A8,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

void emif_get_reg_dump(u32 emif_nr, const struct emif_regs **regs)
{
	u64 ram_size;

	ram_size = board_ti_get_emif_size();

	switch (omap_revision()) {
	case DRA752_ES1_0:
	case DRA752_ES1_1:
	case DRA752_ES2_0:
		switch (emif_nr) {
		case 1:
			if (ram_size > CONFIG_MAX_MEM_MAPPED)
				*regs = &emif1_ddr3_532_mhz_1cs_2G;
			else
				*regs = &emif1_ddr3_532_mhz_1cs;
			break;
		case 2:
			if (ram_size > CONFIG_MAX_MEM_MAPPED)
				*regs = &emif2_ddr3_532_mhz_1cs_2G;
			else
				*regs = &emif2_ddr3_532_mhz_1cs;
			break;
		}
		break;
	case DRA762_ABZ_ES1_0:
	case DRA762_ACD_ES1_0:
	case DRA762_ES1_0:
		if (emif_nr == 1)
			*regs = &emif_1_regs_ddr3_666_mhz_1cs_dra76;
		else
			*regs = &emif_2_regs_ddr3_666_mhz_1cs_dra76;
		break;
	case DRA722_ES1_0:
	case DRA722_ES2_0:
	case DRA722_ES2_1:
		if (ram_size < CONFIG_MAX_MEM_MAPPED)
			*regs = &emif_1_regs_ddr3_666_mhz_1cs_dra_es1;
		else
			*regs = &emif_1_regs_ddr3_666_mhz_1cs_dra_es2;
		break;
	default:
		*regs = &emif1_ddr3_532_mhz_1cs;
	}
}

static const struct dmm_lisa_map_regs lisa_map_dra7_1536MB = {
	.dmm_lisa_map_0 = 0x0,
	.dmm_lisa_map_1 = 0x80640300,
	.dmm_lisa_map_2 = 0xC0500220,
	.dmm_lisa_map_3 = 0xFF020100,
	.is_ma_present	= 0x1
};

static const struct dmm_lisa_map_regs lisa_map_2G_x_2 = {
	.dmm_lisa_map_0 = 0x0,
	.dmm_lisa_map_1 = 0x0,
	.dmm_lisa_map_2 = 0x80600100,
	.dmm_lisa_map_3 = 0xFF020100,
	.is_ma_present	= 0x1
};

const struct dmm_lisa_map_regs lisa_map_dra7_2GB = {
	.dmm_lisa_map_0 = 0x0,
	.dmm_lisa_map_1 = 0x0,
	.dmm_lisa_map_2 = 0x80740300,
	.dmm_lisa_map_3 = 0xFF020100,
	.is_ma_present	= 0x1
};

/*
 * DRA722 EVM EMIF1 2GB CONFIGURATION
 * EMIF1 4 devices of 512Mb x 8 Micron
 */
const struct dmm_lisa_map_regs lisa_map_2G_x_4 = {
	.dmm_lisa_map_0 = 0x0,
	.dmm_lisa_map_1 = 0x0,
	.dmm_lisa_map_2 = 0x80700100,
	.dmm_lisa_map_3 = 0xFF020100,
	.is_ma_present	= 0x1
};

void emif_get_dmm_regs(const struct dmm_lisa_map_regs **dmm_lisa_regs)
{
	u64 ram_size;

	ram_size = board_ti_get_emif_size();

	switch (omap_revision()) {
	case DRA762_ABZ_ES1_0:
	case DRA762_ACD_ES1_0:
	case DRA762_ES1_0:
	case DRA752_ES1_0:
	case DRA752_ES1_1:
	case DRA752_ES2_0:
		if (ram_size > CONFIG_MAX_MEM_MAPPED)
			*dmm_lisa_regs = &lisa_map_dra7_2GB;
		else
			*dmm_lisa_regs = &lisa_map_dra7_1536MB;
		break;
	case DRA722_ES1_0:
	case DRA722_ES2_0:
	case DRA722_ES2_1:
	default:
		if (ram_size < CONFIG_MAX_MEM_MAPPED)
			*dmm_lisa_regs = &lisa_map_2G_x_2;
		else
			*dmm_lisa_regs = &lisa_map_2G_x_4;
		break;
	}
}

struct vcores_data dra752_volts = {
	.mpu.value[OPP_NOM]	= VDD_MPU_DRA7_NOM,
	.mpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_MPU_NOM,
	.mpu.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.mpu.addr	= TPS659038_REG_ADDR_SMPS12,
	.mpu.pmic	= &tps659038,
	.mpu.abb_tx_done_mask = OMAP_ABB_MPU_TXDONE_MASK,

	.eve.value[OPP_NOM]	= VDD_EVE_DRA7_NOM,
	.eve.value[OPP_OD]	= VDD_EVE_DRA7_OD,
	.eve.value[OPP_HIGH]	= VDD_EVE_DRA7_HIGH,
	.eve.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_DSPEVE_NOM,
	.eve.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_DSPEVE_OD,
	.eve.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_DSPEVE_HIGH,
	.eve.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.eve.addr	= TPS659038_REG_ADDR_SMPS45,
	.eve.pmic	= &tps659038,
	.eve.abb_tx_done_mask = OMAP_ABB_EVE_TXDONE_MASK,

	.gpu.value[OPP_NOM]	= VDD_GPU_DRA7_NOM,
	.gpu.value[OPP_OD]	= VDD_GPU_DRA7_OD,
	.gpu.value[OPP_HIGH]	= VDD_GPU_DRA7_HIGH,
	.gpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_GPU_NOM,
	.gpu.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_GPU_OD,
	.gpu.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_GPU_HIGH,
	.gpu.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.gpu.addr	= TPS659038_REG_ADDR_SMPS6,
	.gpu.pmic	= &tps659038,
	.gpu.abb_tx_done_mask = OMAP_ABB_GPU_TXDONE_MASK,

	.core.value[OPP_NOM]	= VDD_CORE_DRA7_NOM,
	.core.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_CORE_NOM,
	.core.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.core.addr	= TPS659038_REG_ADDR_SMPS7,
	.core.pmic	= &tps659038,

	.iva.value[OPP_NOM]	= VDD_IVA_DRA7_NOM,
	.iva.value[OPP_OD]	= VDD_IVA_DRA7_OD,
	.iva.value[OPP_HIGH]	= VDD_IVA_DRA7_HIGH,
	.iva.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_IVA_NOM,
	.iva.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_IVA_OD,
	.iva.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_IVA_HIGH,
	.iva.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.iva.addr	= TPS659038_REG_ADDR_SMPS8,
	.iva.pmic	= &tps659038,
	.iva.abb_tx_done_mask = OMAP_ABB_IVA_TXDONE_MASK,
};

struct vcores_data dra76x_volts = {
	.mpu.value[OPP_NOM]	= VDD_MPU_DRA7_NOM,
	.mpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_MPU_NOM,
	.mpu.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.mpu.addr	= LP87565_REG_ADDR_BUCK01,
	.mpu.pmic	= &lp87565,
	.mpu.abb_tx_done_mask = OMAP_ABB_MPU_TXDONE_MASK,

	.eve.value[OPP_NOM]	= VDD_EVE_DRA7_NOM,
	.eve.value[OPP_OD]	= VDD_EVE_DRA7_OD,
	.eve.value[OPP_HIGH]	= VDD_EVE_DRA7_HIGH,
	.eve.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_DSPEVE_NOM,
	.eve.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_DSPEVE_OD,
	.eve.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_DSPEVE_HIGH,
	.eve.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.eve.addr	= TPS65917_REG_ADDR_SMPS1,
	.eve.pmic	= &tps659038,
	.eve.abb_tx_done_mask = OMAP_ABB_EVE_TXDONE_MASK,

	.gpu.value[OPP_NOM]	= VDD_GPU_DRA7_NOM,
	.gpu.value[OPP_OD]	= VDD_GPU_DRA7_OD,
	.gpu.value[OPP_HIGH]	= VDD_GPU_DRA7_HIGH,
	.gpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_GPU_NOM,
	.gpu.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_GPU_OD,
	.gpu.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_GPU_HIGH,
	.gpu.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.gpu.addr	= LP87565_REG_ADDR_BUCK23,
	.gpu.pmic	= &lp87565,
	.gpu.abb_tx_done_mask = OMAP_ABB_GPU_TXDONE_MASK,

	.core.value[OPP_NOM]	= VDD_CORE_DRA7_NOM,
	.core.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_CORE_NOM,
	.core.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.core.addr	= TPS65917_REG_ADDR_SMPS3,
	.core.pmic	= &tps659038,

	.iva.value[OPP_NOM]	= VDD_IVA_DRA7_NOM,
	.iva.value[OPP_OD]	= VDD_IVA_DRA7_OD,
	.iva.value[OPP_HIGH]	= VDD_IVA_DRA7_HIGH,
	.iva.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_IVA_NOM,
	.iva.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_IVA_OD,
	.iva.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_IVA_HIGH,
	.iva.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.iva.addr	= TPS65917_REG_ADDR_SMPS4,
	.iva.pmic	= &tps659038,
	.iva.abb_tx_done_mask = OMAP_ABB_IVA_TXDONE_MASK,
};

struct vcores_data dra722_volts = {
	.mpu.value[OPP_NOM]	= VDD_MPU_DRA7_NOM,
	.mpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_MPU_NOM,
	.mpu.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.mpu.addr	= TPS65917_REG_ADDR_SMPS1,
	.mpu.pmic	= &tps659038,
	.mpu.abb_tx_done_mask = OMAP_ABB_MPU_TXDONE_MASK,

	.core.value[OPP_NOM]	= VDD_CORE_DRA7_NOM,
	.core.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_CORE_NOM,
	.core.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.core.addr	= TPS65917_REG_ADDR_SMPS2,
	.core.pmic	= &tps659038,

	/*
	 * The DSPEVE, GPU and IVA rails are usually grouped on DRA72x
	 * designs and powered by TPS65917 SMPS3, as on the J6Eco EVM.
	 */
	.gpu.value[OPP_NOM]	= VDD_GPU_DRA7_NOM,
	.gpu.value[OPP_OD]	= VDD_GPU_DRA7_OD,
	.gpu.value[OPP_HIGH]	= VDD_GPU_DRA7_HIGH,
	.gpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_GPU_NOM,
	.gpu.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_GPU_OD,
	.gpu.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_GPU_HIGH,
	.gpu.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.gpu.addr	= TPS65917_REG_ADDR_SMPS3,
	.gpu.pmic	= &tps659038,
	.gpu.abb_tx_done_mask = OMAP_ABB_GPU_TXDONE_MASK,

	.eve.value[OPP_NOM]	= VDD_EVE_DRA7_NOM,
	.eve.value[OPP_OD]	= VDD_EVE_DRA7_OD,
	.eve.value[OPP_HIGH]	= VDD_EVE_DRA7_HIGH,
	.eve.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_DSPEVE_NOM,
	.eve.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_DSPEVE_OD,
	.eve.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_DSPEVE_HIGH,
	.eve.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.eve.addr	= TPS65917_REG_ADDR_SMPS3,
	.eve.pmic	= &tps659038,
	.eve.abb_tx_done_mask = OMAP_ABB_EVE_TXDONE_MASK,

	.iva.value[OPP_NOM]	= VDD_IVA_DRA7_NOM,
	.iva.value[OPP_OD]	= VDD_IVA_DRA7_OD,
	.iva.value[OPP_HIGH]	= VDD_IVA_DRA7_HIGH,
	.iva.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_IVA_NOM,
	.iva.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_IVA_OD,
	.iva.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_IVA_HIGH,
	.iva.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.iva.addr	= TPS65917_REG_ADDR_SMPS3,
	.iva.pmic	= &tps659038,
	.iva.abb_tx_done_mask = OMAP_ABB_IVA_TXDONE_MASK,
};

struct vcores_data dra718_volts = {
	/*
	 * In the case of dra71x GPU MPU and CORE
	 * are all powered up by BUCK0 of LP873X PMIC
	 */
	.mpu.value[OPP_NOM]	= VDD_MPU_DRA7_NOM,
	.mpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_MPU_NOM,
	.mpu.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.mpu.addr	= LP873X_REG_ADDR_BUCK0,
	.mpu.pmic	= &lp8733,
	.mpu.abb_tx_done_mask = OMAP_ABB_MPU_TXDONE_MASK,

	.core.value[OPP_NOM]		= VDD_CORE_DRA7_NOM,
	.core.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_CORE_NOM,
	.core.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.core.addr	= LP873X_REG_ADDR_BUCK0,
	.core.pmic	= &lp8733,

	.gpu.value[OPP_NOM]	= VDD_GPU_DRA7_NOM,
	.gpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_GPU_NOM,
	.gpu.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.gpu.addr	= LP873X_REG_ADDR_BUCK0,
	.gpu.pmic	= &lp8733,
	.gpu.abb_tx_done_mask = OMAP_ABB_GPU_TXDONE_MASK,

	/*
	 * The DSPEVE and IVA rails are grouped on DRA71x-evm
	 * and are powered by BUCK1 of LP873X PMIC
	 */
	.eve.value[OPP_NOM]	= VDD_EVE_DRA7_NOM,
	.eve.value[OPP_HIGH]	= VDD_EVE_DRA7_HIGH,
	.eve.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_DSPEVE_NOM,
	.eve.efuse.reg[OPP_HIGH] = STD_FUSE_OPP_VMIN_DSPEVE_HIGH,
	.eve.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.eve.addr	= LP873X_REG_ADDR_BUCK1,
	.eve.pmic	= &lp8733,
	.eve.abb_tx_done_mask = OMAP_ABB_EVE_TXDONE_MASK,

	.iva.value[OPP_NOM]	= VDD_IVA_DRA7_NOM,
	.iva.value[OPP_HIGH]	= VDD_IVA_DRA7_HIGH,
	.iva.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_IVA_NOM,
	.iva.efuse.reg[OPP_HIGH] = STD_FUSE_OPP_VMIN_IVA_HIGH,
	.iva.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.iva.addr	= LP873X_REG_ADDR_BUCK1,
	.iva.pmic	= &lp8733,
	.iva.abb_tx_done_mask = OMAP_ABB_IVA_TXDONE_MASK,
};

int get_voltrail_opp(int rail_offset)
{
	int opp;

	switch (rail_offset) {
	case VOLT_MPU:
		opp = DRA7_MPU_OPP;
		/* DRA71x supports only OPP_NOM for MPU */
		if (board_is_dra71x_evm())
			opp = OPP_NOM;
		break;
	case VOLT_CORE:
		opp = DRA7_CORE_OPP;
		/* DRA71x supports only OPP_NOM for CORE */
		if (board_is_dra71x_evm())
			opp = OPP_NOM;
		break;
	case VOLT_GPU:
		opp = DRA7_GPU_OPP;
		/* DRA71x supports only OPP_NOM for GPU */
		if (board_is_dra71x_evm())
			opp = OPP_NOM;
		break;
	case VOLT_EVE:
		opp = DRA7_DSPEVE_OPP;
		/*
		 * DRA71x does not support OPP_OD for EVE.
		 * If OPP_OD is selected by menuconfig, fallback
		 * to OPP_NOM.
		 */
		if (board_is_dra71x_evm() && opp == OPP_OD)
			opp = OPP_NOM;
		break;
	case VOLT_IVA:
		opp = DRA7_IVA_OPP;
		/*
		 * DRA71x does not support OPP_OD for IVA.
		 * If OPP_OD is selected by menuconfig, fallback
		 * to OPP_NOM.
		 */
		if (board_is_dra71x_evm() && opp == OPP_OD)
			opp = OPP_NOM;
		break;
	default:
		opp = OPP_NOM;
	}

	return opp;
}

/**
 * @brief board_init
 *
 * @return 0
 */
int board_init(void)
{
	gpmc_init();
	gd->bd->bi_boot_params = (0x80000000 + 0x100); /* boot param addr */

	return 0;
}

int dram_init_banksize(void)
{
	u64 ram_size;

	ram_size = board_ti_get_emif_size();

	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = get_effective_memsize();
	if (ram_size > CONFIG_MAX_MEM_MAPPED) {
		gd->bd->bi_dram[1].start = 0x200000000;
		gd->bd->bi_dram[1].size = ram_size - CONFIG_MAX_MEM_MAPPED;
	}

	return 0;
}

#if CONFIG_IS_ENABLED(DM_USB) && CONFIG_IS_ENABLED(OF_CONTROL)
static int device_okay(const char *path)
{
	int node;

	node = fdt_path_offset(gd->fdt_blob, path);
	if (node < 0)
		return 0;

	return fdtdec_get_is_enabled(gd->fdt_blob, node);
}
#endif

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	char *name = "unknown";

	if (is_dra72x()) {
		if (board_is_dra72x_revc_or_later())
			name = "dra72x-revc";
		else if (board_is_dra71x_evm())
			name = "dra71x";
		else
			name = "dra72x";
	} else if (is_dra76x_abz()) {
		name = "dra76x_abz";
	} else if (is_dra76x_acd()) {
		name = "dra76x_acd";
	} else {
		name = "dra7xx";
	}

	set_board_info_env(name);

	/*
	 * Default FIT boot on HS devices. Non FIT images are not allowed
	 * on HS devices.
	 */
	if (get_device_type() == HS_DEVICE)
		env_set("boot_fit", "1");

	omap_die_id_serial();
	omap_set_fastboot_vars();

	/*
	 * Hook the LDO1 regulator to EN pin. This applies only to LP8733
	 * Rest all regulators are hooked to EN Pin at reset.
	 */
	if (board_is_dra71x_evm())
		palmas_i2c_write_u8(LP873X_I2C_SLAVE_ADDR, 0x9, 0x7);
#endif
#if CONFIG_IS_ENABLED(DM_USB) && CONFIG_IS_ENABLED(OF_CONTROL)
	if (device_okay("/ocp/omap_dwc3_1@48880000"))
		enable_usb_clocks(0);
	if (device_okay("/ocp/omap_dwc3_2@488c0000"))
		enable_usb_clocks(1);
#endif
	return 0;
}

#ifdef CONFIG_SPL_BUILD
void do_board_detect(void)
{
	int rc;

	rc = ti_i2c_eeprom_dra7_get(CONFIG_EEPROM_BUS_ADDRESS,
				    CONFIG_EEPROM_CHIP_ADDRESS);
	if (rc)
		printf("ti_i2c_eeprom_init failed %d\n", rc);
}

#else

void do_board_detect(void)
{
	char *bname = NULL;
	int rc;

	rc = ti_i2c_eeprom_dra7_get(CONFIG_EEPROM_BUS_ADDRESS,
				    CONFIG_EEPROM_CHIP_ADDRESS);
	if (rc)
		printf("ti_i2c_eeprom_init failed %d\n", rc);

	if (board_is_dra74x_evm()) {
		bname = "DRA74x EVM";
	} else if (board_is_dra72x_evm()) {
		bname = "DRA72x EVM";
	} else if (board_is_dra71x_evm()) {
		bname = "DRA71x EVM";
	} else if (board_is_dra76x_evm()) {
		bname = "DRA76x EVM";
	} else {
		/* If EEPROM is not populated */
		if (is_dra72x())
			bname = "DRA72x EVM";
		else
			bname = "DRA74x EVM";
	}

	if (bname)
		snprintf(sysinfo.board_string, SYSINFO_BOARD_NAME_MAX_LEN,
			 "Board: %s REV %s\n", bname, board_ti_get_rev());
}
#endif	/* CONFIG_SPL_BUILD */

void vcores_init(void)
{
	if (board_is_dra74x_evm()) {
		*omap_vcores = &dra752_volts;
	} else if (board_is_dra72x_evm()) {
		*omap_vcores = &dra722_volts;
	} else if (board_is_dra71x_evm()) {
		*omap_vcores = &dra718_volts;
	} else if (board_is_dra76x_evm()) {
		*omap_vcores = &dra76x_volts;
	} else {
		/* If EEPROM is not populated */
		if (is_dra72x())
			*omap_vcores = &dra722_volts;
		else
			*omap_vcores = &dra752_volts;
	}
}

void set_muxconf_regs(void)
{
	do_set_mux32((*ctrl)->control_padconf_core_base,
		     early_padconf, ARRAY_SIZE(early_padconf));
}

#if defined(CONFIG_NAND)
static int nand_sw_detect(void)
{
	int rc;
	uchar data[2];
	struct udevice *dev;

	rc = i2c_get_chip_for_busnum(NAND_PCF8575_I2C_BUS_NUM,
				     NAND_PCF8575_ADDR, 0, &dev);
	if (rc)
		return -1;

	rc = dm_i2c_read(dev, 0, (uint8_t *)&data, sizeof(data));
	if (rc)
		return -1;

	/* We are only interested in P10 and P11 on PCF8575 which is equal to
	 * bits 8 and 9.
	 */
	data[1] = data[1] & 0x3;

	/* Ensure only P11 is set and P10 is cleared. This ensures only
	 * NAND (P10) is configured and not NOR (P11) which are both low
	 * true signals. NAND and NOR settings should not be enabled at
	 * the same time.
	 */
	if (data[1] == 0x2)
		return 0;

	return -1;
}
#else
int nand_sw_detect(void)
{
	return -1;
}
#endif

#ifdef CONFIG_IODELAY_RECALIBRATION
void recalibrate_iodelay(void)
{
	struct pad_conf_entry const *pads, *delta_pads = NULL;
	struct iodelay_cfg_entry const *iodelay;
	int npads, niodelays, delta_npads = 0;
	int ret;

	switch (omap_revision()) {
	case DRA722_ES1_0:
	case DRA722_ES2_0:
	case DRA722_ES2_1:
		pads = dra72x_core_padconf_array_common;
		npads = ARRAY_SIZE(dra72x_core_padconf_array_common);
		if (board_is_dra71x_evm()) {
			pads = dra71x_core_padconf_array;
			npads = ARRAY_SIZE(dra71x_core_padconf_array);
			iodelay = dra71_iodelay_cfg_array;
			niodelays = ARRAY_SIZE(dra71_iodelay_cfg_array);
			/* If SW8 on the EVM is set to enable NAND then
			 * overwrite the pins used by VOUT3 with NAND.
			 */
			if (!nand_sw_detect()) {
				delta_pads = dra71x_nand_padconf_array;
				delta_npads =
					ARRAY_SIZE(dra71x_nand_padconf_array);
			} else {
				delta_pads = dra71x_vout3_padconf_array;
				delta_npads =
					ARRAY_SIZE(dra71x_vout3_padconf_array);
			}

		} else if (board_is_dra72x_revc_or_later()) {
			delta_pads = dra72x_rgmii_padconf_array_revc;
			delta_npads =
				ARRAY_SIZE(dra72x_rgmii_padconf_array_revc);
			iodelay = dra72_iodelay_cfg_array_revc;
			niodelays = ARRAY_SIZE(dra72_iodelay_cfg_array_revc);
		} else {
			delta_pads = dra72x_rgmii_padconf_array_revb;
			delta_npads =
				ARRAY_SIZE(dra72x_rgmii_padconf_array_revb);
			iodelay = dra72_iodelay_cfg_array_revb;
			niodelays = ARRAY_SIZE(dra72_iodelay_cfg_array_revb);
		}
		break;
	case DRA752_ES1_0:
	case DRA752_ES1_1:
		pads = dra74x_core_padconf_array;
		npads = ARRAY_SIZE(dra74x_core_padconf_array);
		iodelay = dra742_es1_1_iodelay_cfg_array;
		niodelays = ARRAY_SIZE(dra742_es1_1_iodelay_cfg_array);
		break;
	case DRA762_ACD_ES1_0:
	case DRA762_ES1_0:
		pads = dra76x_core_padconf_array;
		npads = ARRAY_SIZE(dra76x_core_padconf_array);
		iodelay = dra76x_es1_0_iodelay_cfg_array;
		niodelays = ARRAY_SIZE(dra76x_es1_0_iodelay_cfg_array);
		break;
	default:
	case DRA752_ES2_0:
	case DRA762_ABZ_ES1_0:
		pads = dra74x_core_padconf_array;
		npads = ARRAY_SIZE(dra74x_core_padconf_array);
		iodelay = dra742_es2_0_iodelay_cfg_array;
		niodelays = ARRAY_SIZE(dra742_es2_0_iodelay_cfg_array);
		/* Setup port1 and port2 for rgmii with 'no-id' mode */
		clrset_spare_register(1, 0, RGMII2_ID_MODE_N_MASK |
				      RGMII1_ID_MODE_N_MASK);
		break;
	}
	/* Setup I/O isolation */
	ret = __recalibrate_iodelay_start();
	if (ret)
		goto err;

	/* Do the muxing here */
	do_set_mux32((*ctrl)->control_padconf_core_base, pads, npads);

	/* Now do the weird minor deltas that should be safe */
	if (delta_npads)
		do_set_mux32((*ctrl)->control_padconf_core_base,
			     delta_pads, delta_npads);

	if (is_dra76x())
		/* Set mux for MCAN instead of DCAN1 */
		clrsetbits_le32((*ctrl)->control_core_control_spare_rw,
				MCAN_SEL_ALT_MASK, MCAN_SEL);

	/* Setup IOdelay configuration */
	ret = do_set_iodelay((*ctrl)->iodelay_config_base, iodelay, niodelays);
err:
	/* Closeup.. remove isolation */
	__recalibrate_iodelay_end(ret);
}
#endif

#if defined(CONFIG_MMC)
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0, 0, 0, -1, -1);
	omap_mmc_init(1, 0, 0, -1, -1);
	return 0;
}

void board_mmc_poweron_ldo(uint voltage)
{
	if (board_is_dra71x_evm()) {
		if (voltage == LDO_VOLT_3V0)
			voltage = 0x19;
		else if (voltage == LDO_VOLT_1V8)
			voltage = 0xa;
		lp873x_mmc1_poweron_ldo(voltage);
	} else if (board_is_dra76x_evm()) {
		palmas_mmc1_poweron_ldo(LDO4_VOLTAGE, LDO4_CTRL, voltage);
	} else {
		palmas_mmc1_poweron_ldo(LDO1_VOLTAGE, LDO1_CTRL, voltage);
	}
}

static const struct mmc_platform_fixups dra7x_es1_1_mmc1_fixups = {
	.hw_rev = "rev11",
	.unsupported_caps = MMC_CAP(MMC_HS_200) |
			    MMC_CAP(UHS_SDR104),
	.max_freq = 96000000,
};

static const struct mmc_platform_fixups dra7x_es1_1_mmc23_fixups = {
	.hw_rev = "rev11",
	.unsupported_caps = MMC_CAP(MMC_HS_200) |
			    MMC_CAP(UHS_SDR104) |
			    MMC_CAP(UHS_SDR50),
	.max_freq = 48000000,
};

const struct mmc_platform_fixups *platform_fixups_mmc(uint32_t addr)
{
	switch (omap_revision()) {
	case DRA752_ES1_0:
	case DRA752_ES1_1:
		if (addr == OMAP_HSMMC1_BASE)
			return &dra7x_es1_1_mmc1_fixups;
		else
			return &dra7x_es1_1_mmc23_fixups;
	default:
		return NULL;
	}
}
#endif

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_OS_BOOT)
int spl_start_uboot(void)
{
	/* break into full u-boot on 'c' */
	if (serial_tstc() && serial_getc() == 'c')
		return 1;

#ifdef CONFIG_SPL_ENV_SUPPORT
	env_init();
	env_load();
	if (env_get_yesno("boot_os") != 1)
		return 1;
#endif

	return 0;
}
#endif

#ifdef CONFIG_DRIVER_TI_CPSW
extern u32 *const omap_si_rev;

static void cpsw_control(int enabled)
{
	/* VTP can be added here */

	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 2,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 3,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 2,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};

int board_eth_init(bd_t *bis)
{
	int ret;
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;
	uint32_t ctrl_val;

	/* try reading mac address from efuse */
	mac_lo = readl((*ctrl)->control_core_mac_id_0_lo);
	mac_hi = readl((*ctrl)->control_core_mac_id_0_hi);
	mac_addr[0] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = mac_hi & 0xFF;
	mac_addr[3] = (mac_lo & 0xFF0000) >> 16;
	mac_addr[4] = (mac_lo & 0xFF00) >> 8;
	mac_addr[5] = mac_lo & 0xFF;

	if (!env_get("ethaddr")) {
		printf("<ethaddr> not set. Validating first E-fuse MAC\n");

		if (is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr("ethaddr", mac_addr);
	}

	mac_lo = readl((*ctrl)->control_core_mac_id_1_lo);
	mac_hi = readl((*ctrl)->control_core_mac_id_1_hi);
	mac_addr[0] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = mac_hi & 0xFF;
	mac_addr[3] = (mac_lo & 0xFF0000) >> 16;
	mac_addr[4] = (mac_lo & 0xFF00) >> 8;
	mac_addr[5] = mac_lo & 0xFF;

	if (!env_get("eth1addr")) {
		if (is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr("eth1addr", mac_addr);
	}

	ctrl_val = readl((*ctrl)->control_core_control_io1) & (~0x33);
	ctrl_val |= 0x22;
	writel(ctrl_val, (*ctrl)->control_core_control_io1);

	if (*omap_si_rev == DRA722_ES1_0)
		cpsw_data.active_slave = 1;

	if (board_is_dra72x_revc_or_later()) {
		cpsw_slaves[0].phy_if = PHY_INTERFACE_MODE_RGMII_ID;
		cpsw_slaves[1].phy_if = PHY_INTERFACE_MODE_RGMII_ID;
	}

	ret = cpsw_register(&cpsw_data);
	if (ret < 0)
		printf("Error %d registering CPSW switch\n", ret);

	return ret;
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
/* VTT regulator enable */
static inline void vtt_regulator_enable(void)
{
	if (omap_hw_init_context() == OMAP_INIT_CONTEXT_UBOOT_AFTER_SPL)
		return;

	/* Do not enable VTT for DRA722 or DRA76x */
	if (is_dra72x() || is_dra76x())
		return;

	/*
	 * EVM Rev G and later use gpio7_11 for DDR3 termination.
	 * This is safe enough to do on older revs.
	 */
	gpio_request(GPIO_DDR_VTT_EN, "ddr_vtt_en");
	gpio_direction_output(GPIO_DDR_VTT_EN, 1);
}

int board_early_init_f(void)
{
	vtt_regulator_enable();
	return 0;
}
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	return 0;
}
#endif

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	if (is_dra72x()) {
		if (board_is_dra71x_evm()) {
			if (!strcmp(name, "dra71-evm"))
				return 0;
		}else if(board_is_dra72x_revc_or_later()) {
			if (!strcmp(name, "dra72-evm-revc"))
				return 0;
		} else if (!strcmp(name, "dra72-evm")) {
			return 0;
		}
	} else if (is_dra76x_acd() && !strcmp(name, "dra76-evm")) {
		return 0;
	} else if (!is_dra72x() && !is_dra76x_acd() &&
		   !strcmp(name, "dra7-evm")) {
		return 0;
	}

	return -1;
}
#endif

#if CONFIG_IS_ENABLED(FASTBOOT) && !CONFIG_IS_ENABLED(ENV_IS_NOWHERE)
int fastboot_set_reboot_flag(void)
{
	printf("Setting reboot to fastboot flag ...\n");
	env_set("dofastboot", "1");
	env_save();
	return 0;
}
#endif

#ifdef CONFIG_TI_SECURE_DEVICE
void board_fit_image_post_process(void **p_image, size_t *p_size)
{
	secure_boot_verify_image(p_image, p_size);
}

void board_tee_image_process(ulong tee_image, size_t tee_size)
{
	secure_tee_install((u32)tee_image);
}

U_BOOT_FIT_LOADABLE_HANDLER(IH_TYPE_TEE, board_tee_image_process);
#endif
