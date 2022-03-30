// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com
 *
 * Author: Felipe Balbi <balbi@ti.com>
 *
 * Based on board/ti/dra7xx/evm.c
 */

#include <common.h>
#include <palmas.h>
#include <sata.h>
#include <usb.h>
#include <asm/omap_common.h>
#include <asm/omap_sec_common.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/dra7xx_iodelay.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sata.h>
#include <asm/arch/gpio.h>
#include <asm/arch/omap.h>
#include <environment.h>
#include <usb.h>
#include <linux/usb/gadget.h>
#include <dwc3-uboot.h>
#include <dwc3-omap-uboot.h>
#include <ti-usb-phy-uboot.h>
#include <mmc.h>

#include "../common/board_detect.h"
#include "mux_data.h"

#define board_is_x15()		board_ti_is("BBRDX15_")
#define board_is_x15_revb1()	(board_ti_is("BBRDX15_") && \
				 !strncmp("B.10", board_ti_get_rev(), 3))
#define board_is_x15_revc()	(board_ti_is("BBRDX15_") && \
				 !strncmp("C.00", board_ti_get_rev(), 3))
#define board_is_am572x_evm()	board_ti_is("AM572PM_")
#define board_is_am572x_evm_reva3()	\
				(board_ti_is("AM572PM_") && \
				 !strncmp("A.30", board_ti_get_rev(), 3))
#define board_is_am574x_idk()	board_ti_is("AM574IDK")
#define board_is_am572x_idk()	board_ti_is("AM572IDK")
#define board_is_am571x_idk()	board_ti_is("AM571IDK")

#ifdef CONFIG_DRIVER_TI_CPSW
#include <cpsw.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define GPIO_ETH_LCD		GPIO_TO_PIN(2, 22)
/* GPIO 7_11 */
#define GPIO_DDR_VTT_EN 203

/* Touch screen controller to identify the LCD */
#define OSD_TS_FT_BUS_ADDRESS	0
#define OSD_TS_FT_CHIP_ADDRESS	0x38
#define OSD_TS_FT_REG_ID	0xA3
/*
 * Touchscreen IDs for various OSD panels
 * Ref: http://www.osddisplays.com/TI/OSD101T2587-53TS_A.1.pdf
 */
/* Used on newer osd101t2587 Panels */
#define OSD_TS_FT_ID_5x46	0x54
/* Used on older osd101t2045 Panels */
#define OSD_TS_FT_ID_5606	0x08

#define SYSINFO_BOARD_NAME_MAX_LEN	45

#define TPS65903X_PRIMARY_SECONDARY_PAD2	0xFB
#define TPS65903X_PAD2_POWERHOLD_MASK		0x20

const struct omap_sysinfo sysinfo = {
	"Board: UNKNOWN(BeagleBoard X15?) REV UNKNOWN\n"
};

static const struct dmm_lisa_map_regs beagle_x15_lisa_regs = {
	.dmm_lisa_map_3 = 0x80740300,
	.is_ma_present  = 0x1
};

static const struct dmm_lisa_map_regs am571x_idk_lisa_regs = {
	.dmm_lisa_map_3 = 0x80640100,
	.is_ma_present  = 0x1
};

static const struct dmm_lisa_map_regs am574x_idk_lisa_regs = {
	.dmm_lisa_map_2 = 0xc0600200,
	.dmm_lisa_map_3 = 0x80600100,
	.is_ma_present  = 0x1
};

void emif_get_dmm_regs(const struct dmm_lisa_map_regs **dmm_lisa_regs)
{
	if (board_is_am571x_idk())
		*dmm_lisa_regs = &am571x_idk_lisa_regs;
	else if (board_is_am574x_idk())
		*dmm_lisa_regs = &am574x_idk_lisa_regs;
	else
		*dmm_lisa_regs = &beagle_x15_lisa_regs;
}

static const struct emif_regs beagle_x15_emif1_ddr3_532mhz_emif_regs = {
	.sdram_config_init		= 0x61851b32,
	.sdram_config			= 0x61851b32,
	.sdram_config2			= 0x08000000,
	.ref_ctrl			= 0x000040F1,
	.ref_ctrl_final			= 0x00001035,
	.sdram_tim1			= 0xcccf36ab,
	.sdram_tim2			= 0x308f7fda,
	.sdram_tim3			= 0x409f88a8,
	.read_idle_ctrl			= 0x00050000,
	.zq_config			= 0x5007190b,
	.temp_alert_config		= 0x00000000,
	.emif_ddr_phy_ctlr_1_init 	= 0x0024400b,
	.emif_ddr_phy_ctlr_1		= 0x0e24400b,
	.emif_ddr_ext_phy_ctrl_1 	= 0x10040100,
	.emif_ddr_ext_phy_ctrl_2 	= 0x00910091,
	.emif_ddr_ext_phy_ctrl_3 	= 0x00950095,
	.emif_ddr_ext_phy_ctrl_4 	= 0x009b009b,
	.emif_ddr_ext_phy_ctrl_5 	= 0x009e009e,
	.emif_rd_wr_lvl_rmp_win		= 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl		= 0x80000000,
	.emif_rd_wr_lvl_ctl		= 0x00000000,
	.emif_rd_wr_exec_thresh		= 0x00000305
};

/* Ext phy ctrl regs 1-35 */
static const u32 beagle_x15_emif1_ddr3_ext_phy_ctrl_const_regs[] = {
	0x10040100,
	0x00910091,
	0x00950095,
	0x009B009B,
	0x009E009E,
	0x00980098,
	0x00340034,
	0x00350035,
	0x00340034,
	0x00310031,
	0x00340034,
	0x007F007F,
	0x007F007F,
	0x007F007F,
	0x007F007F,
	0x007F007F,
	0x00480048,
	0x004A004A,
	0x00520052,
	0x00550055,
	0x00500050,
	0x00000000,
	0x00600020,
	0x40011080,
	0x08102040,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0
};

static const struct emif_regs beagle_x15_emif2_ddr3_532mhz_emif_regs = {
	.sdram_config_init		= 0x61851b32,
	.sdram_config			= 0x61851b32,
	.sdram_config2			= 0x08000000,
	.ref_ctrl			= 0x000040F1,
	.ref_ctrl_final			= 0x00001035,
	.sdram_tim1			= 0xcccf36b3,
	.sdram_tim2			= 0x308f7fda,
	.sdram_tim3			= 0x407f88a8,
	.read_idle_ctrl			= 0x00050000,
	.zq_config			= 0x5007190b,
	.temp_alert_config		= 0x00000000,
	.emif_ddr_phy_ctlr_1_init 	= 0x0024400b,
	.emif_ddr_phy_ctlr_1		= 0x0e24400b,
	.emif_ddr_ext_phy_ctrl_1 	= 0x10040100,
	.emif_ddr_ext_phy_ctrl_2 	= 0x00910091,
	.emif_ddr_ext_phy_ctrl_3 	= 0x00950095,
	.emif_ddr_ext_phy_ctrl_4 	= 0x009b009b,
	.emif_ddr_ext_phy_ctrl_5 	= 0x009e009e,
	.emif_rd_wr_lvl_rmp_win		= 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl		= 0x80000000,
	.emif_rd_wr_lvl_ctl		= 0x00000000,
	.emif_rd_wr_exec_thresh		= 0x00000305
};

static const u32 beagle_x15_emif2_ddr3_ext_phy_ctrl_const_regs[] = {
	0x10040100,
	0x00910091,
	0x00950095,
	0x009B009B,
	0x009E009E,
	0x00980098,
	0x00340034,
	0x00350035,
	0x00340034,
	0x00310031,
	0x00340034,
	0x007F007F,
	0x007F007F,
	0x007F007F,
	0x007F007F,
	0x007F007F,
	0x00480048,
	0x004A004A,
	0x00520052,
	0x00550055,
	0x00500050,
	0x00000000,
	0x00600020,
	0x40011080,
	0x08102040,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0
};

static const struct emif_regs am571x_emif1_ddr3_666mhz_emif_regs = {
	.sdram_config_init		= 0x61863332,
	.sdram_config			= 0x61863332,
	.sdram_config2			= 0x08000000,
	.ref_ctrl			= 0x0000514d,
	.ref_ctrl_final			= 0x0000144a,
	.sdram_tim1			= 0xd333887c,
	.sdram_tim2			= 0x30b37fe3,
	.sdram_tim3			= 0x409f8ad8,
	.read_idle_ctrl			= 0x00050000,
	.zq_config			= 0x5007190b,
	.temp_alert_config		= 0x00000000,
	.emif_ddr_phy_ctlr_1_init	= 0x0024400f,
	.emif_ddr_phy_ctlr_1		= 0x0e24400f,
	.emif_ddr_ext_phy_ctrl_1	= 0x10040100,
	.emif_ddr_ext_phy_ctrl_2	= 0x00910091,
	.emif_ddr_ext_phy_ctrl_3	= 0x00950095,
	.emif_ddr_ext_phy_ctrl_4	= 0x009b009b,
	.emif_ddr_ext_phy_ctrl_5	= 0x009e009e,
	.emif_rd_wr_lvl_rmp_win		= 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl		= 0x80000000,
	.emif_rd_wr_lvl_ctl		= 0x00000000,
	.emif_rd_wr_exec_thresh		= 0x00000305
};

static const struct emif_regs am574x_emif1_ddr3_666mhz_emif_ecc_regs = {
	.sdram_config_init		= 0x61863332,
	.sdram_config			= 0x61863332,
	.sdram_config2			= 0x08000000,
	.ref_ctrl			= 0x0000514d,
	.ref_ctrl_final			= 0x0000144a,
	.sdram_tim1			= 0xd333887c,
	.sdram_tim2			= 0x30b37fe3,
	.sdram_tim3			= 0x409f8ad8,
	.read_idle_ctrl			= 0x00050000,
	.zq_config			= 0x5007190b,
	.temp_alert_config		= 0x00000000,
	.emif_ddr_phy_ctlr_1_init	= 0x0024400f,
	.emif_ddr_phy_ctlr_1		= 0x0e24400f,
	.emif_ddr_ext_phy_ctrl_1	= 0x10040100,
	.emif_ddr_ext_phy_ctrl_2	= 0x00910091,
	.emif_ddr_ext_phy_ctrl_3	= 0x00950095,
	.emif_ddr_ext_phy_ctrl_4	= 0x009b009b,
	.emif_ddr_ext_phy_ctrl_5	= 0x009e009e,
	.emif_rd_wr_lvl_rmp_win		= 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl		= 0x80000000,
	.emif_rd_wr_lvl_ctl		= 0x00000000,
	.emif_rd_wr_exec_thresh		= 0x00000305,
	.emif_ecc_ctrl_reg		= 0xD0000001,
	.emif_ecc_address_range_1	= 0x3FFF0000,
	.emif_ecc_address_range_2	= 0x00000000
};

void emif_get_reg_dump(u32 emif_nr, const struct emif_regs **regs)
{
	switch (emif_nr) {
	case 1:
		if (board_is_am571x_idk())
			*regs = &am571x_emif1_ddr3_666mhz_emif_regs;
		else if (board_is_am574x_idk())
			*regs = &am574x_emif1_ddr3_666mhz_emif_ecc_regs;
		else
			*regs = &beagle_x15_emif1_ddr3_532mhz_emif_regs;
		break;
	case 2:
		if (board_is_am574x_idk())
			*regs = &am571x_emif1_ddr3_666mhz_emif_regs;
		else
			*regs = &beagle_x15_emif2_ddr3_532mhz_emif_regs;
		break;
	}
}

void emif_get_ext_phy_ctrl_const_regs(u32 emif_nr, const u32 **regs, u32 *size)
{
	switch (emif_nr) {
	case 1:
		*regs = beagle_x15_emif1_ddr3_ext_phy_ctrl_const_regs;
		*size = ARRAY_SIZE(beagle_x15_emif1_ddr3_ext_phy_ctrl_const_regs);
		break;
	case 2:
		*regs = beagle_x15_emif2_ddr3_ext_phy_ctrl_const_regs;
		*size = ARRAY_SIZE(beagle_x15_emif2_ddr3_ext_phy_ctrl_const_regs);
		break;
	}
}

struct vcores_data beagle_x15_volts = {
	.mpu.value[OPP_NOM]	= VDD_MPU_DRA7_NOM,
	.mpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_MPU_NOM,
	.mpu.efuse.reg_bits     = DRA752_EFUSE_REGBITS,
	.mpu.addr		= TPS659038_REG_ADDR_SMPS12,
	.mpu.pmic		= &tps659038,
	.mpu.abb_tx_done_mask	= OMAP_ABB_MPU_TXDONE_MASK,

	.eve.value[OPP_NOM]	= VDD_EVE_DRA7_NOM,
	.eve.value[OPP_OD]	= VDD_EVE_DRA7_OD,
	.eve.value[OPP_HIGH]	= VDD_EVE_DRA7_HIGH,
	.eve.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_DSPEVE_NOM,
	.eve.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_DSPEVE_OD,
	.eve.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_DSPEVE_HIGH,
	.eve.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.eve.addr		= TPS659038_REG_ADDR_SMPS45,
	.eve.pmic		= &tps659038,
	.eve.abb_tx_done_mask	= OMAP_ABB_EVE_TXDONE_MASK,

	.gpu.value[OPP_NOM]	= VDD_GPU_DRA7_NOM,
	.gpu.value[OPP_OD]	= VDD_GPU_DRA7_OD,
	.gpu.value[OPP_HIGH]	= VDD_GPU_DRA7_HIGH,
	.gpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_GPU_NOM,
	.gpu.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_GPU_OD,
	.gpu.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_GPU_HIGH,
	.gpu.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.gpu.addr		= TPS659038_REG_ADDR_SMPS45,
	.gpu.pmic		= &tps659038,
	.gpu.abb_tx_done_mask	= OMAP_ABB_GPU_TXDONE_MASK,

	.core.value[OPP_NOM]	= VDD_CORE_DRA7_NOM,
	.core.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_CORE_NOM,
	.core.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.core.addr		= TPS659038_REG_ADDR_SMPS6,
	.core.pmic		= &tps659038,

	.iva.value[OPP_NOM]	= VDD_IVA_DRA7_NOM,
	.iva.value[OPP_OD]	= VDD_IVA_DRA7_OD,
	.iva.value[OPP_HIGH]	= VDD_IVA_DRA7_HIGH,
	.iva.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_IVA_NOM,
	.iva.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_IVA_OD,
	.iva.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_IVA_HIGH,
	.iva.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.iva.addr		= TPS659038_REG_ADDR_SMPS45,
	.iva.pmic		= &tps659038,
	.iva.abb_tx_done_mask	= OMAP_ABB_IVA_TXDONE_MASK,
};

struct vcores_data am572x_idk_volts = {
	.mpu.value[OPP_NOM]	= VDD_MPU_DRA7_NOM,
	.mpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_MPU_NOM,
	.mpu.efuse.reg_bits     = DRA752_EFUSE_REGBITS,
	.mpu.addr		= TPS659038_REG_ADDR_SMPS12,
	.mpu.pmic		= &tps659038,
	.mpu.abb_tx_done_mask	= OMAP_ABB_MPU_TXDONE_MASK,

	.eve.value[OPP_NOM]	= VDD_EVE_DRA7_NOM,
	.eve.value[OPP_OD]	= VDD_EVE_DRA7_OD,
	.eve.value[OPP_HIGH]	= VDD_EVE_DRA7_HIGH,
	.eve.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_DSPEVE_NOM,
	.eve.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_DSPEVE_OD,
	.eve.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_DSPEVE_HIGH,
	.eve.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.eve.addr		= TPS659038_REG_ADDR_SMPS45,
	.eve.pmic		= &tps659038,
	.eve.abb_tx_done_mask	= OMAP_ABB_EVE_TXDONE_MASK,

	.gpu.value[OPP_NOM]	= VDD_GPU_DRA7_NOM,
	.gpu.value[OPP_OD]	= VDD_GPU_DRA7_OD,
	.gpu.value[OPP_HIGH]	= VDD_GPU_DRA7_HIGH,
	.gpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_GPU_NOM,
	.gpu.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_GPU_OD,
	.gpu.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_GPU_HIGH,
	.gpu.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.gpu.addr		= TPS659038_REG_ADDR_SMPS6,
	.gpu.pmic		= &tps659038,
	.gpu.abb_tx_done_mask	= OMAP_ABB_GPU_TXDONE_MASK,

	.core.value[OPP_NOM]	= VDD_CORE_DRA7_NOM,
	.core.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_CORE_NOM,
	.core.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.core.addr		= TPS659038_REG_ADDR_SMPS7,
	.core.pmic		= &tps659038,

	.iva.value[OPP_NOM]	= VDD_IVA_DRA7_NOM,
	.iva.value[OPP_OD]	= VDD_IVA_DRA7_OD,
	.iva.value[OPP_HIGH]	= VDD_IVA_DRA7_HIGH,
	.iva.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_IVA_NOM,
	.iva.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_IVA_OD,
	.iva.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_IVA_HIGH,
	.iva.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.iva.addr		= TPS659038_REG_ADDR_SMPS8,
	.iva.pmic		= &tps659038,
	.iva.abb_tx_done_mask	= OMAP_ABB_IVA_TXDONE_MASK,
};

struct vcores_data am571x_idk_volts = {
	.mpu.value[OPP_NOM]	= VDD_MPU_DRA7_NOM,
	.mpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_MPU_NOM,
	.mpu.efuse.reg_bits     = DRA752_EFUSE_REGBITS,
	.mpu.addr		= TPS659038_REG_ADDR_SMPS12,
	.mpu.pmic		= &tps659038,
	.mpu.abb_tx_done_mask = OMAP_ABB_MPU_TXDONE_MASK,

	.eve.value[OPP_NOM]	= VDD_EVE_DRA7_NOM,
	.eve.value[OPP_OD]	= VDD_EVE_DRA7_OD,
	.eve.value[OPP_HIGH]	= VDD_EVE_DRA7_HIGH,
	.eve.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_DSPEVE_NOM,
	.eve.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_DSPEVE_OD,
	.eve.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_DSPEVE_HIGH,
	.eve.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.eve.addr		= TPS659038_REG_ADDR_SMPS45,
	.eve.pmic		= &tps659038,
	.eve.abb_tx_done_mask	= OMAP_ABB_EVE_TXDONE_MASK,

	.gpu.value[OPP_NOM]	= VDD_GPU_DRA7_NOM,
	.gpu.value[OPP_OD]	= VDD_GPU_DRA7_OD,
	.gpu.value[OPP_HIGH]	= VDD_GPU_DRA7_HIGH,
	.gpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_GPU_NOM,
	.gpu.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_GPU_OD,
	.gpu.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_GPU_HIGH,
	.gpu.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.gpu.addr		= TPS659038_REG_ADDR_SMPS6,
	.gpu.pmic		= &tps659038,
	.gpu.abb_tx_done_mask	= OMAP_ABB_GPU_TXDONE_MASK,

	.core.value[OPP_NOM]	= VDD_CORE_DRA7_NOM,
	.core.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_CORE_NOM,
	.core.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.core.addr		= TPS659038_REG_ADDR_SMPS7,
	.core.pmic		= &tps659038,

	.iva.value[OPP_NOM]	= VDD_IVA_DRA7_NOM,
	.iva.value[OPP_OD]	= VDD_IVA_DRA7_OD,
	.iva.value[OPP_HIGH]	= VDD_IVA_DRA7_HIGH,
	.iva.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_IVA_NOM,
	.iva.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_IVA_OD,
	.iva.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_IVA_HIGH,
	.iva.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.iva.addr		= TPS659038_REG_ADDR_SMPS45,
	.iva.pmic		= &tps659038,
	.iva.abb_tx_done_mask	= OMAP_ABB_IVA_TXDONE_MASK,
};

int get_voltrail_opp(int rail_offset)
{
	int opp;

	switch (rail_offset) {
	case VOLT_MPU:
		opp = DRA7_MPU_OPP;
		break;
	case VOLT_CORE:
		opp = DRA7_CORE_OPP;
		break;
	case VOLT_GPU:
		opp = DRA7_GPU_OPP;
		break;
	case VOLT_EVE:
		opp = DRA7_DSPEVE_OPP;
		break;
	case VOLT_IVA:
		opp = DRA7_IVA_OPP;
		break;
	default:
		opp = OPP_NOM;
	}

	return opp;
}


#ifdef CONFIG_SPL_BUILD
/* No env to setup for SPL */
static inline void setup_board_eeprom_env(void) { }

/* Override function to read eeprom information */
void do_board_detect(void)
{
	int rc;

	rc = ti_i2c_eeprom_am_get(CONFIG_EEPROM_BUS_ADDRESS,
				  CONFIG_EEPROM_CHIP_ADDRESS);
	if (rc)
		printf("ti_i2c_eeprom_init failed %d\n", rc);
}

#else	/* CONFIG_SPL_BUILD */

/* Override function to read eeprom information: actual i2c read done by SPL*/
void do_board_detect(void)
{
	char *bname = NULL;
	int rc;

	rc = ti_i2c_eeprom_am_get(CONFIG_EEPROM_BUS_ADDRESS,
				  CONFIG_EEPROM_CHIP_ADDRESS);
	if (rc)
		printf("ti_i2c_eeprom_init failed %d\n", rc);

	if (board_is_x15())
		bname = "BeagleBoard X15";
	else if (board_is_am572x_evm())
		bname = "AM572x EVM";
	else if (board_is_am574x_idk())
		bname = "AM574x IDK";
	else if (board_is_am572x_idk())
		bname = "AM572x IDK";
	else if (board_is_am571x_idk())
		bname = "AM571x IDK";

	if (bname)
		snprintf(sysinfo.board_string, SYSINFO_BOARD_NAME_MAX_LEN,
			 "Board: %s REV %s\n", bname, board_ti_get_rev());
}

static void setup_board_eeprom_env(void)
{
	char *name = "beagle_x15";
	int rc;

	rc = ti_i2c_eeprom_am_get(CONFIG_EEPROM_BUS_ADDRESS,
				  CONFIG_EEPROM_CHIP_ADDRESS);
	if (rc)
		goto invalid_eeprom;

	if (board_is_x15()) {
		if (board_is_x15_revb1())
			name = "beagle_x15_revb1";
		else if (board_is_x15_revc())
			name = "beagle_x15_revc";
		else
			name = "beagle_x15";
	} else if (board_is_am572x_evm()) {
		if (board_is_am572x_evm_reva3())
			name = "am57xx_evm_reva3";
		else
			name = "am57xx_evm";
	} else if (board_is_am574x_idk()) {
		name = "am574x_idk";
	} else if (board_is_am572x_idk()) {
		name = "am572x_idk";
	} else if (board_is_am571x_idk()) {
		name = "am571x_idk";
	} else {
		printf("Unidentified board claims %s in eeprom header\n",
		       board_ti_get_name());
	}

invalid_eeprom:
	set_board_info_env(name);
}

#endif	/* CONFIG_SPL_BUILD */

void vcores_init(void)
{
	if (board_is_am572x_idk() || board_is_am574x_idk())
		*omap_vcores = &am572x_idk_volts;
	else if (board_is_am571x_idk())
		*omap_vcores = &am571x_idk_volts;
	else
		*omap_vcores = &beagle_x15_volts;
}

void hw_data_init(void)
{
	*prcm = &dra7xx_prcm;
	if (is_dra72x())
		*dplls_data = &dra72x_dplls;
	else if (is_dra76x())
		*dplls_data = &dra76x_dplls;
	else
		*dplls_data = &dra7xx_dplls;
	*ctrl = &dra7xx_ctrl;
}

bool am571x_idk_needs_lcd(void)
{
	bool needs_lcd;

	gpio_request(GPIO_ETH_LCD, "nLCD_Detect");
	if (gpio_get_value(GPIO_ETH_LCD))
		needs_lcd = false;
	else
		needs_lcd = true;

	gpio_free(GPIO_ETH_LCD);

	return needs_lcd;
}

int board_init(void)
{
	gpmc_init();
	gd->bd->bi_boot_params = (CONFIG_SYS_SDRAM_BASE + 0x100);

	return 0;
}

void am57x_idk_lcd_detect(void)
{
	int r = -ENODEV;
	char *idk_lcd = "no";
	struct udevice *dev;

	/* Only valid for IDKs */
	if (board_is_x15() || board_is_am572x_evm())
		return;

	/* Only AM571x IDK has gpio control detect.. so check that */
	if (board_is_am571x_idk() && !am571x_idk_needs_lcd())
		goto out;

	r = i2c_get_chip_for_busnum(OSD_TS_FT_BUS_ADDRESS,
				    OSD_TS_FT_CHIP_ADDRESS, 1, &dev);
	if (r) {
		printf("%s: Failed to get I2C device %d/%d (ret %d)\n",
		       __func__, OSD_TS_FT_BUS_ADDRESS, OSD_TS_FT_CHIP_ADDRESS,
		       r);
		/* AM572x IDK has no explicit settings for optional LCD kit */
		if (board_is_am571x_idk())
			printf("%s: Touch screen detect failed: %d!\n",
			       __func__, r);
		goto out;
	}

	/* Read FT ID */
	r = dm_i2c_reg_read(dev, OSD_TS_FT_REG_ID);
	if (r < 0) {
		printf("%s: Touch screen ID read %d:0x%02x[0x%02x] failed:%d\n",
		       __func__, OSD_TS_FT_BUS_ADDRESS, OSD_TS_FT_CHIP_ADDRESS,
		       OSD_TS_FT_REG_ID, r);
		goto out;
	}

	switch (r) {
	case OSD_TS_FT_ID_5606:
		idk_lcd = "osd101t2045";
		break;
	case OSD_TS_FT_ID_5x46:
		idk_lcd = "osd101t2587";
		break;
	default:
		printf("%s: Unidentifed Touch screen ID 0x%02x\n",
		       __func__, r);
		/* we will let default be "no lcd" */
	}
out:
	env_set("idk_lcd", idk_lcd);
	return;
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
	setup_board_eeprom_env();
	u8 val;

	/*
	 * DEV_CTRL.DEV_ON = 1 please - else palmas switches off in 8 seconds
	 * This is the POWERHOLD-in-Low behavior.
	 */
	palmas_i2c_write_u8(TPS65903X_CHIP_P1, 0xA0, 0x1);

	/*
	 * Default FIT boot on HS devices. Non FIT images are not allowed
	 * on HS devices.
	 */
	if (get_device_type() == HS_DEVICE)
		env_set("boot_fit", "1");

	/*
	 * Set the GPIO7 Pad to POWERHOLD. This has higher priority
	 * over DEV_CTRL.DEV_ON bit. This can be reset in case of
	 * PMIC Power off. So to be on the safer side set it back
	 * to POWERHOLD mode irrespective of the current state.
	 */
	palmas_i2c_read_u8(TPS65903X_CHIP_P1, TPS65903X_PRIMARY_SECONDARY_PAD2,
			   &val);
	val = val | TPS65903X_PAD2_POWERHOLD_MASK;
	palmas_i2c_write_u8(TPS65903X_CHIP_P1, TPS65903X_PRIMARY_SECONDARY_PAD2,
			    val);

	omap_die_id_serial();
	omap_set_fastboot_vars();

	am57x_idk_lcd_detect();

#if !defined(CONFIG_SPL_BUILD)
	board_ti_set_ethaddr(2);
#endif

#if CONFIG_IS_ENABLED(DM_USB) && CONFIG_IS_ENABLED(OF_CONTROL)
	if (device_okay("/ocp/omap_dwc3_1@48880000"))
		enable_usb_clocks(0);
	if (device_okay("/ocp/omap_dwc3_2@488c0000"))
		enable_usb_clocks(1);
#endif
	return 0;
}

void set_muxconf_regs(void)
{
	do_set_mux32((*ctrl)->control_padconf_core_base,
		     early_padconf, ARRAY_SIZE(early_padconf));
}

#ifdef CONFIG_IODELAY_RECALIBRATION
void recalibrate_iodelay(void)
{
	const struct pad_conf_entry *pconf;
	const struct iodelay_cfg_entry *iod, *delta_iod;
	int pconf_sz, iod_sz, delta_iod_sz = 0;
	int ret;

	if (board_is_am572x_idk()) {
		pconf = core_padconf_array_essential_am572x_idk;
		pconf_sz = ARRAY_SIZE(core_padconf_array_essential_am572x_idk);
		iod = iodelay_cfg_array_am572x_idk;
		iod_sz = ARRAY_SIZE(iodelay_cfg_array_am572x_idk);
	} else if (board_is_am574x_idk()) {
		pconf = core_padconf_array_essential_am574x_idk;
		pconf_sz = ARRAY_SIZE(core_padconf_array_essential_am574x_idk);
		iod = iodelay_cfg_array_am574x_idk;
		iod_sz = ARRAY_SIZE(iodelay_cfg_array_am574x_idk);
	} else if (board_is_am571x_idk()) {
		pconf = core_padconf_array_essential_am571x_idk;
		pconf_sz = ARRAY_SIZE(core_padconf_array_essential_am571x_idk);
		iod = iodelay_cfg_array_am571x_idk;
		iod_sz = ARRAY_SIZE(iodelay_cfg_array_am571x_idk);
	} else {
		/* Common for X15/GPEVM */
		pconf = core_padconf_array_essential_x15;
		pconf_sz = ARRAY_SIZE(core_padconf_array_essential_x15);
		/* There never was an SR1.0 X15.. So.. */
		if (omap_revision() == DRA752_ES1_1) {
			iod = iodelay_cfg_array_x15_sr1_1;
			iod_sz = ARRAY_SIZE(iodelay_cfg_array_x15_sr1_1);
		} else {
			/* Since full production should switch to SR2.0  */
			iod = iodelay_cfg_array_x15_sr2_0;
			iod_sz = ARRAY_SIZE(iodelay_cfg_array_x15_sr2_0);
		}
	}

	/* Setup I/O isolation */
	ret = __recalibrate_iodelay_start();
	if (ret)
		goto err;

	/* Do the muxing here */
	do_set_mux32((*ctrl)->control_padconf_core_base, pconf, pconf_sz);

	/* Now do the weird minor deltas that should be safe */
	if (board_is_x15() || board_is_am572x_evm()) {
		if (board_is_x15_revb1() || board_is_am572x_evm_reva3() ||
		    board_is_x15_revc()) {
			pconf = core_padconf_array_delta_x15_sr2_0;
			pconf_sz = ARRAY_SIZE(core_padconf_array_delta_x15_sr2_0);
		} else {
			pconf = core_padconf_array_delta_x15_sr1_1;
			pconf_sz = ARRAY_SIZE(core_padconf_array_delta_x15_sr1_1);
		}
		do_set_mux32((*ctrl)->control_padconf_core_base, pconf, pconf_sz);
	}

	if (board_is_am571x_idk()) {
		if (am571x_idk_needs_lcd()) {
			pconf = core_padconf_array_vout_am571x_idk;
			pconf_sz = ARRAY_SIZE(core_padconf_array_vout_am571x_idk);
			delta_iod = iodelay_cfg_array_am571x_idk_4port;
			delta_iod_sz = ARRAY_SIZE(iodelay_cfg_array_am571x_idk_4port);

		} else {
			pconf = core_padconf_array_icss1eth_am571x_idk;
			pconf_sz = ARRAY_SIZE(core_padconf_array_icss1eth_am571x_idk);
		}
		do_set_mux32((*ctrl)->control_padconf_core_base, pconf, pconf_sz);
	}

	/* Setup IOdelay configuration */
	ret = do_set_iodelay((*ctrl)->iodelay_config_base, iod, iod_sz);
	if (delta_iod_sz)
		ret = do_set_iodelay((*ctrl)->iodelay_config_base, delta_iod,
				     delta_iod_sz);

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

static const struct mmc_platform_fixups am57x_es1_1_mmc1_fixups = {
	.hw_rev = "rev11",
	.unsupported_caps = MMC_CAP(MMC_HS_200) |
			    MMC_CAP(UHS_SDR104),
	.max_freq = 96000000,
};

static const struct mmc_platform_fixups am57x_es1_1_mmc23_fixups = {
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
			return &am57x_es1_1_mmc1_fixups;
		else
			return &am57x_es1_1_mmc23_fixups;
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

/* Delay value to add to calibrated value */
#define RGMII0_TXCTL_DLY_VAL		((0x3 << 5) + 0x8)
#define RGMII0_TXD0_DLY_VAL		((0x3 << 5) + 0x8)
#define RGMII0_TXD1_DLY_VAL		((0x3 << 5) + 0x2)
#define RGMII0_TXD2_DLY_VAL		((0x4 << 5) + 0x0)
#define RGMII0_TXD3_DLY_VAL		((0x4 << 5) + 0x0)
#define VIN2A_D13_DLY_VAL		((0x3 << 5) + 0x8)
#define VIN2A_D17_DLY_VAL		((0x3 << 5) + 0x8)
#define VIN2A_D16_DLY_VAL		((0x3 << 5) + 0x2)
#define VIN2A_D15_DLY_VAL		((0x4 << 5) + 0x0)
#define VIN2A_D14_DLY_VAL		((0x4 << 5) + 0x0)

static void cpsw_control(int enabled)
{
	/* VTP can be added here */
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 1,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 2,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 1,
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

static u64 mac_to_u64(u8 mac[6])
{
	int i;
	u64 addr = 0;

	for (i = 0; i < 6; i++) {
		addr <<= 8;
		addr |= mac[i];
	}

	return addr;
}

static void u64_to_mac(u64 addr, u8 mac[6])
{
	mac[5] = addr;
	mac[4] = addr >> 8;
	mac[3] = addr >> 16;
	mac[2] = addr >> 24;
	mac[1] = addr >> 32;
	mac[0] = addr >> 40;
}

int board_eth_init(bd_t *bis)
{
	int ret;
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;
	uint32_t ctrl_val;
	int i;
	u64 mac1, mac2;
	u8 mac_addr1[6], mac_addr2[6];
	int num_macs;

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

	/* The phy address for the AM57xx IDK are different than x15 */
	if (board_is_am572x_idk() || board_is_am571x_idk() ||
	    board_is_am574x_idk()) {
		cpsw_data.slave_data[0].phy_addr = 0;
		cpsw_data.slave_data[1].phy_addr = 1;
	}

	ret = cpsw_register(&cpsw_data);
	if (ret < 0)
		printf("Error %d registering CPSW switch\n", ret);

	/*
	 * Export any Ethernet MAC addresses from EEPROM.
	 * On AM57xx the 2 MAC addresses define the address range
	 */
	board_ti_get_eth_mac_addr(0, mac_addr1);
	board_ti_get_eth_mac_addr(1, mac_addr2);

	if (is_valid_ethaddr(mac_addr1) && is_valid_ethaddr(mac_addr2)) {
		mac1 = mac_to_u64(mac_addr1);
		mac2 = mac_to_u64(mac_addr2);

		/* must contain an address range */
		num_macs = mac2 - mac1 + 1;
		/* <= 50 to protect against user programming error */
		if (num_macs > 0 && num_macs <= 50) {
			for (i = 0; i < num_macs; i++) {
				u64_to_mac(mac1 + i, mac_addr);
				if (is_valid_ethaddr(mac_addr)) {
					eth_env_set_enetaddr_by_index("eth",
								      i + 2,
								      mac_addr);
				}
			}
		}
	}

	return ret;
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
/* VTT regulator enable */
static inline void vtt_regulator_enable(void)
{
	if (omap_hw_init_context() == OMAP_INIT_CONTEXT_UBOOT_AFTER_SPL)
		return;

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
	if (board_is_x15()) {
		if (board_is_x15_revb1()) {
			if (!strcmp(name, "am57xx-beagle-x15-revb1"))
				return 0;
		} else if (board_is_x15_revc()) {
			if (!strcmp(name, "am57xx-beagle-x15-revc"))
				return 0;
		} else if (!strcmp(name, "am57xx-beagle-x15")) {
			return 0;
		}
	} else if (board_is_am572x_evm() &&
		   !strcmp(name, "am57xx-beagle-x15")) {
		return 0;
	} else if (board_is_am572x_idk() && !strcmp(name, "am572x-idk")) {
		return 0;
	} else if (board_is_am574x_idk() && !strcmp(name, "am574x-idk")) {
		return 0;
	} else if (board_is_am571x_idk() && !strcmp(name, "am571x-idk")) {
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
