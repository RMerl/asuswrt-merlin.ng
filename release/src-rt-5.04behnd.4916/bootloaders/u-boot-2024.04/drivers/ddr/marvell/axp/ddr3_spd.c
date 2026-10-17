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

#include "ddr3_init.h"

#if defined(MV88F78X60)
#include "ddr3_axp_config.h"
#elif defined(MV88F67XX)
#include "ddr3_a370_config.h"
#endif

#if defined(MV88F672X)
#include "ddr3_a375_config.h"
#endif

#ifdef DUNIT_SPD

/* DIMM SPD offsets */
#define SPD_DEV_TYPE_BYTE		2

#define SPD_MODULE_TYPE_BYTE		3
#define SPD_MODULE_MASK			0xf
#define SPD_MODULE_TYPE_RDIMM		1
#define SPD_MODULE_TYPE_UDIMM		2

#define SPD_DEV_DENSITY_BYTE		4
#define SPD_DEV_DENSITY_MASK		0xf

#define SPD_ROW_NUM_BYTE		5
#define SPD_ROW_NUM_MIN			12
#define SPD_ROW_NUM_OFF			3
#define SPD_ROW_NUM_MASK		(7 << SPD_ROW_NUM_OFF)

#define SPD_COL_NUM_BYTE		5
#define SPD_COL_NUM_MIN			9
#define SPD_COL_NUM_OFF			0
#define SPD_COL_NUM_MASK		(7 << SPD_COL_NUM_OFF)

#define SPD_MODULE_ORG_BYTE		7
#define SPD_MODULE_SDRAM_DEV_WIDTH_OFF	0
#define SPD_MODULE_SDRAM_DEV_WIDTH_MASK	(7 << SPD_MODULE_SDRAM_DEV_WIDTH_OFF)
#define SPD_MODULE_BANK_NUM_MIN		1
#define SPD_MODULE_BANK_NUM_OFF		3
#define SPD_MODULE_BANK_NUM_MASK	(7 << SPD_MODULE_BANK_NUM_OFF)

#define SPD_BUS_WIDTH_BYTE		8
#define SPD_BUS_WIDTH_OFF		0
#define SPD_BUS_WIDTH_MASK		(7 << SPD_BUS_WIDTH_OFF)
#define SPD_BUS_ECC_OFF			3
#define SPD_BUS_ECC_MASK		(3 << SPD_BUS_ECC_OFF)

#define SPD_MTB_DIVIDEND_BYTE		10
#define SPD_MTB_DIVISOR_BYTE		11
#define SPD_TCK_BYTE			12
#define SPD_SUP_CAS_LAT_LSB_BYTE	14
#define SPD_SUP_CAS_LAT_MSB_BYTE	15
#define SPD_TAA_BYTE			16
#define SPD_TWR_BYTE			17
#define SPD_TRCD_BYTE			18
#define SPD_TRRD_BYTE			19
#define SPD_TRP_BYTE			20

#define SPD_TRAS_MSB_BYTE		21
#define SPD_TRAS_MSB_MASK		0xf

#define SPD_TRC_MSB_BYTE		21
#define SPD_TRC_MSB_MASK		0xf0

#define SPD_TRAS_LSB_BYTE		22
#define SPD_TRC_LSB_BYTE		23
#define SPD_TRFC_LSB_BYTE		24
#define SPD_TRFC_MSB_BYTE		25
#define SPD_TWTR_BYTE			26
#define SPD_TRTP_BYTE			27

#define SPD_TFAW_MSB_BYTE		28
#define SPD_TFAW_MSB_MASK		0xf

#define SPD_TFAW_LSB_BYTE		29
#define SPD_OPT_FEATURES_BYTE		30
#define SPD_THERMAL_REFRESH_OPT_BYTE	31

#define SPD_ADDR_MAP_BYTE		63
#define SPD_ADDR_MAP_MIRROR_OFFS	0

#define SPD_RDIMM_RC_BYTE		69
#define SPD_RDIMM_RC_NIBBLE_MASK	0xF
#define SPD_RDIMM_RC_NUM		16

/* Dimm Memory Type values */
#define SPD_MEM_TYPE_SDRAM		0x4
#define SPD_MEM_TYPE_DDR1		0x7
#define SPD_MEM_TYPE_DDR2		0x8
#define SPD_MEM_TYPE_DDR3		0xB

#define DIMM_MODULE_MANU_OFFS		64
#define DIMM_MODULE_MANU_SIZE		8
#define DIMM_MODULE_VEN_OFFS		73
#define DIMM_MODULE_VEN_SIZE		25
#define DIMM_MODULE_ID_OFFS		99
#define DIMM_MODULE_ID_SIZE		18

/* enumeration for voltage levels. */
enum dimm_volt_if {
	TTL_5V_TOLERANT,
	LVTTL,
	HSTL_1_5V,
	SSTL_3_3V,
	SSTL_2_5V,
	VOLTAGE_UNKNOWN,
};

/* enumaration for SDRAM CAS Latencies. */
enum dimm_sdram_cas {
	SD_CL_1 = 1,
	SD_CL_2,
	SD_CL_3,
	SD_CL_4,
	SD_CL_5,
	SD_CL_6,
	SD_CL_7,
	SD_FAULT
};

/* enumeration for memory types */
enum memory_type {
	MEM_TYPE_SDRAM,
	MEM_TYPE_DDR1,
	MEM_TYPE_DDR2,
	MEM_TYPE_DDR3
};

/* DIMM information structure */
typedef struct dimm_info {
	/* DIMM dimensions */
	u32 num_of_module_ranks;
	u32 data_width;
	u32 rank_capacity;
	u32 num_of_devices;

	u32 sdram_width;
	u32 num_of_banks_on_each_device;
	u32 sdram_capacity;

	u32 num_of_row_addr;
	u32 num_of_col_addr;

	u32 addr_mirroring;

	u32 err_check_type;			/* ECC , PARITY.. */
	u32 type_info;				/* DDR2 only */

	/* DIMM timing parameters */
	u32 supported_cas_latencies;
	u32 refresh_interval;
	u32 min_cycle_time;
	u32 min_row_precharge_time;
	u32 min_row_active_to_row_active;
	u32 min_ras_to_cas_delay;
	u32 min_write_recovery_time;		/* DDR3/2 only */
	u32 min_write_to_read_cmd_delay;	/* DDR3/2 only */
	u32 min_read_to_prech_cmd_delay;	/* DDR3/2 only */
	u32 min_active_to_precharge;
	u32 min_refresh_recovery;		/* DDR3/2 only */
	u32 min_cas_lat_time;
	u32 min_four_active_win_delay;
	u8 dimm_rc[SPD_RDIMM_RC_NUM];

	/* DIMM vendor ID */
	u32 vendor;
} MV_DIMM_INFO;

static int ddr3_spd_sum_init(MV_DIMM_INFO *info, MV_DIMM_INFO *sum_info,
			     u32 dimm);
static u32 ddr3_get_max_val(u32 spd_val, u32 dimm_num, u32 static_val);
static u32 ddr3_get_min_val(u32 spd_val, u32 dimm_num, u32 static_val);
static int ddr3_spd_init(MV_DIMM_INFO *info, u32 dimm_addr, u32 dimm_width);
static u32 ddr3_div(u32 val, u32 divider, u32 sub);

extern u8 spd_data[SPD_SIZE];
extern u32 odt_config[ODT_OPT];
extern u16 odt_static[ODT_OPT][MAX_CS];
extern u16 odt_dynamic[ODT_OPT][MAX_CS];

#if !(defined(DB_88F6710) || defined(DB_88F6710_PCAC) || defined(RD_88F6710))
/*
 * Name:     ddr3_get_dimm_num - Find number of dimms and their addresses
 * Desc:
 * Args:     dimm_addr - array of dimm addresses
 * Notes:
 * Returns:  None.
 */
static u32 ddr3_get_dimm_num(u32 *dimm_addr)
{
	u32 dimm_cur_addr;
	u8 data[3];
	u32 dimm_num = 0;
	int ret;

	/* Read the dimm eeprom */
	for (dimm_cur_addr = MAX_DIMM_ADDR; dimm_cur_addr > MIN_DIMM_ADDR;
	     dimm_cur_addr--) {
		struct udevice *udev;

		data[SPD_DEV_TYPE_BYTE] = 0;

		/* Far-End DIMM must be connected */
		if ((dimm_num == 0) && (dimm_cur_addr < FAR_END_DIMM_ADDR))
			return 0;

		ret = i2c_get_chip_for_busnum(0, dimm_cur_addr, 1, &udev);
		if (ret)
			continue;

		ret = dm_i2c_read(udev, 0, data, 3);
		if (!ret) {
			if (data[SPD_DEV_TYPE_BYTE] == SPD_MEM_TYPE_DDR3) {
				dimm_addr[dimm_num] = dimm_cur_addr;
				dimm_num++;
			}
		}
	}

	return dimm_num;
}
#endif

/*
 * Name:     dimmSpdInit - Get the SPD parameters.
 * Desc:     Read the DIMM SPD parameters into given struct parameter.
 * Args:     dimmNum - DIMM number. See MV_BOARD_DIMM_NUM enumerator.
 *           info - DIMM information structure.
 * Notes:
 * Returns:  MV_OK if function could read DIMM parameters, 0 otherwise.
 */
int ddr3_spd_init(MV_DIMM_INFO *info, u32 dimm_addr, u32 dimm_width)
{
	u32 tmp;
	u32 time_base;
	int ret;
	__maybe_unused u32 rc;
	__maybe_unused u8 vendor_high, vendor_low;

	if (dimm_addr != 0) {
		struct udevice *udev;

		memset(spd_data, 0, SPD_SIZE * sizeof(u8));

		ret = i2c_get_chip_for_busnum(0, dimm_addr, 1, &udev);
		if (ret)
			return MV_DDR3_TRAINING_ERR_TWSI_FAIL;

		ret = dm_i2c_read(udev, 0, spd_data, SPD_SIZE);
		if (ret)
			return MV_DDR3_TRAINING_ERR_TWSI_FAIL;
	}

	/* Check if DDR3 */
	if (spd_data[SPD_DEV_TYPE_BYTE] != SPD_MEM_TYPE_DDR3)
		return MV_DDR3_TRAINING_ERR_TWSI_BAD_TYPE;

	/* Error Check Type */
	/* No byte for error check in DDR3 SPD, use DDR2 convention */
	info->err_check_type = 0;

	/* Check if ECC */
	if ((spd_data[SPD_BUS_WIDTH_BYTE] & 0x18) >> 3)
		info->err_check_type = 1;

	DEBUG_INIT_FULL_C("DRAM err_check_type ", info->err_check_type, 1);
	switch (spd_data[SPD_MODULE_TYPE_BYTE]) {
	case 1:
		/* support RDIMM */
		info->type_info = SPD_MODULE_TYPE_RDIMM;
		break;
	case 2:
		/* support UDIMM */
		info->type_info = SPD_MODULE_TYPE_UDIMM;
		break;
	case 11:		/* LRDIMM current not supported */
	default:
		info->type_info = (spd_data[SPD_MODULE_TYPE_BYTE]);
		break;
	}

	/* Size Calculations: */

	/* Number Of Row Addresses - 12/13/14/15/16 */
	info->num_of_row_addr =
		(spd_data[SPD_ROW_NUM_BYTE] & SPD_ROW_NUM_MASK) >>
		SPD_ROW_NUM_OFF;
	info->num_of_row_addr += SPD_ROW_NUM_MIN;
	DEBUG_INIT_FULL_C("DRAM num_of_row_addr ", info->num_of_row_addr, 2);

	/* Number Of Column Addresses - 9/10/11/12 */
	info->num_of_col_addr =
		(spd_data[SPD_COL_NUM_BYTE] & SPD_COL_NUM_MASK) >>
		SPD_COL_NUM_OFF;
	info->num_of_col_addr += SPD_COL_NUM_MIN;
	DEBUG_INIT_FULL_C("DRAM num_of_col_addr ", info->num_of_col_addr, 1);

	/* Number Of Ranks = number of CS on Dimm - 1/2/3/4 Ranks */
	info->num_of_module_ranks =
		(spd_data[SPD_MODULE_ORG_BYTE] & SPD_MODULE_BANK_NUM_MASK) >>
		SPD_MODULE_BANK_NUM_OFF;
	info->num_of_module_ranks += SPD_MODULE_BANK_NUM_MIN;
	DEBUG_INIT_FULL_C("DRAM numOfModuleBanks ", info->num_of_module_ranks,
			  1);

	/* Data Width - 8/16/32/64 bits */
	info->data_width =
		1 << (3 + (spd_data[SPD_BUS_WIDTH_BYTE] & SPD_BUS_WIDTH_MASK));
	DEBUG_INIT_FULL_C("DRAM data_width ", info->data_width, 1);

	/* Number Of Banks On Each Device - 8/16/32/64 banks */
	info->num_of_banks_on_each_device =
		1 << (3 + ((spd_data[SPD_DEV_DENSITY_BYTE] >> 4) & 0x7));
	DEBUG_INIT_FULL_C("DRAM num_of_banks_on_each_device ",
			  info->num_of_banks_on_each_device, 1);

	/* Total SDRAM capacity - 256Mb/512Mb/1Gb/2Gb/4Gb/8Gb/16Gb - MegaBits */
	info->sdram_capacity =
		spd_data[SPD_DEV_DENSITY_BYTE] & SPD_DEV_DENSITY_MASK;

	/* Sdram Width - 4/8/16/32 bits */
	info->sdram_width = 1 << (2 + (spd_data[SPD_MODULE_ORG_BYTE] &
				       SPD_MODULE_SDRAM_DEV_WIDTH_MASK));
	DEBUG_INIT_FULL_C("DRAM sdram_width ", info->sdram_width, 1);

	/* CS (Rank) Capacity - MB */
	/*
	 * DDR3 device uiDensity val are: (device capacity/8) *
	 * (Module_width/Device_width)
	 */
	/* Jedec SPD DDR3 - page 7, Save spd_data in Mb  - 2048=2GB */
	if (dimm_width == 32) {
		info->rank_capacity =
			((1 << info->sdram_capacity) * 256 *
			 (info->data_width / info->sdram_width)) << 16;
		/* CS size = CS size / 2  */
	} else {
		info->rank_capacity =
			((1 << info->sdram_capacity) * 256 *
			 (info->data_width / info->sdram_width) * 0x2) << 16;
		/* 0x2 =>  0x100000-1Mbit / 8-bit->byte / 0x10000  */
	}
	DEBUG_INIT_FULL_C("DRAM rank_capacity[31] ", info->rank_capacity, 1);

	/* Number of devices includeing Error correction */
	info->num_of_devices =
		((info->data_width / info->sdram_width) *
		 info->num_of_module_ranks) + info->err_check_type;
	DEBUG_INIT_FULL_C("DRAM num_of_devices  ", info->num_of_devices, 1);

	/* Address Mapping from Edge connector to DRAM - mirroring option */
	info->addr_mirroring =
		spd_data[SPD_ADDR_MAP_BYTE] & (1 << SPD_ADDR_MAP_MIRROR_OFFS);

	/* Timings - All in ps */

	time_base = (1000 * spd_data[SPD_MTB_DIVIDEND_BYTE]) /
		spd_data[SPD_MTB_DIVISOR_BYTE];

	/* Minimum Cycle Time At Max CasLatancy */
	info->min_cycle_time = spd_data[SPD_TCK_BYTE] * time_base;
	DEBUG_INIT_FULL_C("DRAM tCKmin ", info->min_cycle_time, 1);

	/* Refresh Interval */
	/* No byte for refresh interval in DDR3 SPD, use DDR2 convention */
	/*
	 * JEDEC param are 0 <= Tcase <= 85: 7.8uSec, 85 <= Tcase
	 * <= 95: 3.9uSec
	 */
	info->refresh_interval = 7800000;	/* Set to 7.8uSec */
	DEBUG_INIT_FULL_C("DRAM refresh_interval ", info->refresh_interval, 1);

	/* Suported Cas Latencies -  DDR 3: */

	/*
	 *         bit7 | bit6 | bit5 | bit4 | bit3 | bit2 | bit1 | bit0 *
	 *******-******-******-******-******-******-******-*******-*******
	 CAS =      11  |  10  |  9   |  8   |  7   |  6   |  5   |  4   *
	 *********************************************************-*******
	 *******-******-******-******-******-******-******-*******-*******
	 *        bit15 |bit14 |bit13 |bit12 |bit11 |bit10 | bit9 | bit8 *
	 *******-******-******-******-******-******-******-*******-*******
	 CAS =     TBD  |  18  |  17  |  16  |  15  |  14  |  13  |  12  *
	*/

	/* DDR3 include 2 byte of CAS support */
	info->supported_cas_latencies =
		(spd_data[SPD_SUP_CAS_LAT_MSB_BYTE] << 8) |
		spd_data[SPD_SUP_CAS_LAT_LSB_BYTE];
	DEBUG_INIT_FULL_C("DRAM supported_cas_latencies ",
			  info->supported_cas_latencies, 1);

	/* Minimum Cycle Time At Max CasLatancy */
	info->min_cas_lat_time = (spd_data[SPD_TAA_BYTE] * time_base);
	/*
	 * This field divided by the cycleTime will give us the CAS latency
	 * to config
	 */

	/*
	 * For DDR3 and DDR2 includes Write Recovery Time field.
	 * Other SDRAM ignore
	 */
	info->min_write_recovery_time = spd_data[SPD_TWR_BYTE] * time_base;
	DEBUG_INIT_FULL_C("DRAM min_write_recovery_time ",
			  info->min_write_recovery_time, 1);

	/* Mininmum Ras to Cas Delay */
	info->min_ras_to_cas_delay = spd_data[SPD_TRCD_BYTE] * time_base;
	DEBUG_INIT_FULL_C("DRAM min_ras_to_cas_delay ",
			  info->min_ras_to_cas_delay, 1);

	/* Minimum Row Active to Row Active Time */
	info->min_row_active_to_row_active =
	    spd_data[SPD_TRRD_BYTE] * time_base;
	DEBUG_INIT_FULL_C("DRAM min_row_active_to_row_active ",
			  info->min_row_active_to_row_active, 1);

	/* Minimum Row Precharge Delay Time */
	info->min_row_precharge_time = spd_data[SPD_TRP_BYTE] * time_base;
	DEBUG_INIT_FULL_C("DRAM min_row_precharge_time ",
			  info->min_row_precharge_time, 1);

	/* Minimum Active to Precharge Delay Time - tRAS   ps */
	info->min_active_to_precharge =
		(spd_data[SPD_TRAS_MSB_BYTE] & SPD_TRAS_MSB_MASK) << 8;
	info->min_active_to_precharge |= spd_data[SPD_TRAS_LSB_BYTE];
	info->min_active_to_precharge *= time_base;
	DEBUG_INIT_FULL_C("DRAM min_active_to_precharge ",
			  info->min_active_to_precharge, 1);

	/* Minimum Refresh Recovery Delay Time - tRFC  ps */
	info->min_refresh_recovery = spd_data[SPD_TRFC_MSB_BYTE] << 8;
	info->min_refresh_recovery |= spd_data[SPD_TRFC_LSB_BYTE];
	info->min_refresh_recovery *= time_base;
	DEBUG_INIT_FULL_C("DRAM min_refresh_recovery ",
			  info->min_refresh_recovery, 1);

	/*
	 * For DDR3 and DDR2 includes Internal Write To Read Command Delay
	 * field.
	 */
	info->min_write_to_read_cmd_delay = spd_data[SPD_TWTR_BYTE] * time_base;
	DEBUG_INIT_FULL_C("DRAM min_write_to_read_cmd_delay ",
			  info->min_write_to_read_cmd_delay, 1);

	/*
	 * For DDR3 and DDR2 includes Internal Read To Precharge Command Delay
	 * field.
	 */
	info->min_read_to_prech_cmd_delay = spd_data[SPD_TRTP_BYTE] * time_base;
	DEBUG_INIT_FULL_C("DRAM min_read_to_prech_cmd_delay ",
			  info->min_read_to_prech_cmd_delay, 1);

	/*
	 * For DDR3 includes Minimum Activate to Activate/Refresh Command
	 * field
	 */
	tmp = ((spd_data[SPD_TFAW_MSB_BYTE] & SPD_TFAW_MSB_MASK) << 8) |
		spd_data[SPD_TFAW_LSB_BYTE];
	info->min_four_active_win_delay = tmp * time_base;
	DEBUG_INIT_FULL_C("DRAM min_four_active_win_delay ",
			  info->min_four_active_win_delay, 1);

#if defined(MV88F78X60) || defined(MV88F672X)
	/* Registered DIMM support */
	if (info->type_info == SPD_MODULE_TYPE_RDIMM) {
		for (rc = 2; rc < 6; rc += 2) {
			tmp = spd_data[SPD_RDIMM_RC_BYTE + rc / 2];
			info->dimm_rc[rc] =
				spd_data[SPD_RDIMM_RC_BYTE + rc / 2] &
				SPD_RDIMM_RC_NIBBLE_MASK;
			info->dimm_rc[rc + 1] =
				(spd_data[SPD_RDIMM_RC_BYTE + rc / 2] >> 4) &
				SPD_RDIMM_RC_NIBBLE_MASK;
		}

		vendor_low = spd_data[66];
		vendor_high = spd_data[65];
		info->vendor = (vendor_high << 8) + vendor_low;
		DEBUG_INIT_C("DDR3 Training Sequence - Registered DIMM vendor ID 0x",
			     info->vendor, 4);

		info->dimm_rc[0] = RDIMM_RC0;
		info->dimm_rc[1] = RDIMM_RC1;
		info->dimm_rc[2] = RDIMM_RC2;
		info->dimm_rc[8] = RDIMM_RC8;
		info->dimm_rc[9] = RDIMM_RC9;
		info->dimm_rc[10] = RDIMM_RC10;
		info->dimm_rc[11] = RDIMM_RC11;
	}
#endif

	return MV_OK;
}

/*
 * Name:     ddr3_spd_sum_init - Get the SPD parameters.
 * Desc:     Read the DIMM SPD parameters into given struct parameter.
 * Args:     dimmNum - DIMM number. See MV_BOARD_DIMM_NUM enumerator.
 *           info - DIMM information structure.
 * Notes:
 * Returns:  MV_OK if function could read DIMM parameters, 0 otherwise.
 */
int ddr3_spd_sum_init(MV_DIMM_INFO *info, MV_DIMM_INFO *sum_info, u32 dimm)
{
	if (dimm == 0) {
		memcpy(sum_info, info, sizeof(MV_DIMM_INFO));
		return MV_OK;
	}
	if (sum_info->type_info != info->type_info) {
		DEBUG_INIT_S("DDR3 Dimm Compare - DIMM type does not match - FAIL\n");
		return MV_DDR3_TRAINING_ERR_DIMM_TYPE_NO_MATCH;
	}
	if (sum_info->err_check_type > info->err_check_type) {
		sum_info->err_check_type = info->err_check_type;
		DEBUG_INIT_S("DDR3 Dimm Compare - ECC does not match. ECC is disabled\n");
	}
	if (sum_info->data_width != info->data_width) {
		DEBUG_INIT_S("DDR3 Dimm Compare - DRAM bus width does not match - FAIL\n");
		return MV_DDR3_TRAINING_ERR_BUS_WIDTH_NOT_MATCH;
	}
	if (sum_info->min_cycle_time < info->min_cycle_time)
		sum_info->min_cycle_time = info->min_cycle_time;
	if (sum_info->refresh_interval < info->refresh_interval)
		sum_info->refresh_interval = info->refresh_interval;
	sum_info->supported_cas_latencies &= info->supported_cas_latencies;
	if (sum_info->min_cas_lat_time < info->min_cas_lat_time)
		sum_info->min_cas_lat_time = info->min_cas_lat_time;
	if (sum_info->min_write_recovery_time < info->min_write_recovery_time)
		sum_info->min_write_recovery_time =
		    info->min_write_recovery_time;
	if (sum_info->min_ras_to_cas_delay < info->min_ras_to_cas_delay)
		sum_info->min_ras_to_cas_delay = info->min_ras_to_cas_delay;
	if (sum_info->min_row_active_to_row_active <
	    info->min_row_active_to_row_active)
		sum_info->min_row_active_to_row_active =
		    info->min_row_active_to_row_active;
	if (sum_info->min_row_precharge_time < info->min_row_precharge_time)
		sum_info->min_row_precharge_time = info->min_row_precharge_time;
	if (sum_info->min_active_to_precharge < info->min_active_to_precharge)
		sum_info->min_active_to_precharge =
		    info->min_active_to_precharge;
	if (sum_info->min_refresh_recovery < info->min_refresh_recovery)
		sum_info->min_refresh_recovery = info->min_refresh_recovery;
	if (sum_info->min_write_to_read_cmd_delay <
	    info->min_write_to_read_cmd_delay)
		sum_info->min_write_to_read_cmd_delay =
		    info->min_write_to_read_cmd_delay;
	if (sum_info->min_read_to_prech_cmd_delay <
	    info->min_read_to_prech_cmd_delay)
		sum_info->min_read_to_prech_cmd_delay =
		    info->min_read_to_prech_cmd_delay;
	if (sum_info->min_four_active_win_delay <
	    info->min_four_active_win_delay)
		sum_info->min_four_active_win_delay =
		    info->min_four_active_win_delay;
	if (sum_info->min_write_to_read_cmd_delay <
	    info->min_write_to_read_cmd_delay)
		sum_info->min_write_to_read_cmd_delay =
			info->min_write_to_read_cmd_delay;

	return MV_OK;
}

/*
 * Name:     ddr3_dunit_setup
 * Desc:     Set the controller with the timing values.
 * Args:     ecc_ena - User ECC setup
 * Notes:
 * Returns:
 */
int ddr3_dunit_setup(u32 ecc_ena, u32 hclk_time, u32 *ddr_width)
{
	u32 reg, tmp, cwl;
	u32 ddr_clk_time;
	MV_DIMM_INFO dimm_info[2];
	MV_DIMM_INFO sum_info;
	u32 stat_val, spd_val;
	u32 cs, cl, cs_num, cs_ena;
	u32 dimm_num = 0;
	int status;
	u32 rc;
	__maybe_unused u32 dimm_cnt, cs_count, dimm;
	__maybe_unused u32 dimm_addr[2] = { 0, 0 };

#if defined(DB_88F6710) || defined(DB_88F6710_PCAC) || defined(RD_88F6710)
	/* Armada 370 - SPD is not available on DIMM */
	/*
	 * Set MC registers according to Static SPD values Values -
	 * must be set manually
	 */
	/*
	 * We only have one optional DIMM for the DB and we already got the
	 * SPD matching values
	 */
	status = ddr3_spd_init(&dimm_info[0], 0, *ddr_width);
	if (MV_OK != status)
		return status;

	dimm_num = 1;
	/* Use JP8 to enable multiCS support for Armada 370 DB */
	if (!ddr3_check_config(EEPROM_MODULE_ADDR, CONFIG_MULTI_CS))
		dimm_info[0].num_of_module_ranks = 1;
	status = ddr3_spd_sum_init(&dimm_info[0], &sum_info, 0);
	if (MV_OK != status)
		return status;
#else
	/* Dynamic D-Unit Setup - Read SPD values */
#ifdef DUNIT_SPD
	dimm_num = ddr3_get_dimm_num(dimm_addr);
	if (dimm_num == 0) {
#ifdef MIXED_DIMM_STATIC
		DEBUG_INIT_S("DDR3 Training Sequence - No DIMMs detected\n");
#else
		DEBUG_INIT_S("DDR3 Training Sequence - FAILED (Wrong DIMMs Setup)\n");
		return MV_DDR3_TRAINING_ERR_BAD_DIMM_SETUP;
#endif
	} else {
		DEBUG_INIT_C("DDR3 Training Sequence - Number of DIMMs detected: ",
			     dimm_num, 1);
	}

	for (dimm = 0; dimm < dimm_num; dimm++) {
		status = ddr3_spd_init(&dimm_info[dimm], dimm_addr[dimm],
				       *ddr_width);
		if (MV_OK != status)
			return status;
		status = ddr3_spd_sum_init(&dimm_info[dimm], &sum_info, dimm);
		if (MV_OK != status)
			return status;
	}
#endif
#endif

	/* Set number of enabled CS */
	cs_num = 0;
#ifdef DUNIT_STATIC
	cs_num = ddr3_get_cs_num_from_reg();
#endif
#ifdef DUNIT_SPD
	for (dimm = 0; dimm < dimm_num; dimm++)
		cs_num += dimm_info[dimm].num_of_module_ranks;
#endif
	if (cs_num > MAX_CS) {
		DEBUG_INIT_C("DDR3 Training Sequence - Number of CS exceed limit -  ",
			     MAX_CS, 1);
		return MV_DDR3_TRAINING_ERR_MAX_CS_LIMIT;
	}

	/* Set bitmap of enabled CS */
	cs_ena = 0;
#ifdef DUNIT_STATIC
	cs_ena = ddr3_get_cs_ena_from_reg();
#endif
#ifdef DUNIT_SPD
	dimm = 0;

	if (dimm_num) {
		for (cs = 0; cs < MAX_CS; cs += 2) {
			if (((1 << cs) & DIMM_CS_BITMAP) &&
			    !(cs_ena & (1 << cs))) {
				if (dimm_info[dimm].num_of_module_ranks == 1)
					cs_ena |= (0x1 << cs);
				else if (dimm_info[dimm].num_of_module_ranks == 2)
					cs_ena |= (0x3 << cs);
				else if (dimm_info[dimm].num_of_module_ranks == 3)
					cs_ena |= (0x7 << cs);
				else if (dimm_info[dimm].num_of_module_ranks == 4)
					cs_ena |= (0xF << cs);

				dimm++;
				if (dimm == dimm_num)
					break;
			}
		}
	}
#endif

	if (cs_ena > 0xF) {
		DEBUG_INIT_C("DDR3 Training Sequence - Number of enabled CS exceed limit -  ",
			     MAX_CS, 1);
		return MV_DDR3_TRAINING_ERR_MAX_ENA_CS_LIMIT;
	}

	DEBUG_INIT_FULL_C("DDR3 - DUNIT-SET - Number of CS = ", cs_num, 1);

	/* Check Ratio - '1' - 2:1, '0' - 1:1 */
	if (reg_read(REG_DDR_IO_ADDR) & (1 << REG_DDR_IO_CLK_RATIO_OFFS))
		ddr_clk_time = hclk_time / 2;
	else
		ddr_clk_time = hclk_time;

#ifdef DUNIT_STATIC
	/* Get target CL value from set register */
	reg = (reg_read(REG_DDR3_MR0_ADDR) >> 2);
	reg = ((((reg >> 1) & 0xE)) | (reg & 0x1)) & 0xF;

	cl = ddr3_get_max_val(ddr3_div(sum_info.min_cas_lat_time,
				       ddr_clk_time, 0),
			      dimm_num, ddr3_valid_cl_to_cl(reg));
#else
	cl = ddr3_div(sum_info.min_cas_lat_time, ddr_clk_time, 0);
#endif
	if (cl < 5)
		cl = 5;

	DEBUG_INIT_FULL_C("DDR3 - DUNIT-SET - Cas Latency = ", cl, 1);

	/* {0x00001400} -   DDR SDRAM Configuration Register */
	reg = 0x73004000;
	stat_val = ddr3_get_static_mc_value(
		REG_SDRAM_CONFIG_ADDR, REG_SDRAM_CONFIG_ECC_OFFS, 0x1, 0, 0);
	if (ecc_ena && ddr3_get_min_val(sum_info.err_check_type, dimm_num,
					stat_val)) {
		reg |= (1 << REG_SDRAM_CONFIG_ECC_OFFS);
		reg |= (1 << REG_SDRAM_CONFIG_IERR_OFFS);
		DEBUG_INIT_FULL_S("DDR3 - DUNIT-SET - ECC Enabled\n");
	} else {
		DEBUG_INIT_FULL_S("DDR3 - DUNIT-SET - ECC Disabled\n");
	}

	if (sum_info.type_info == SPD_MODULE_TYPE_RDIMM) {
#ifdef DUNIT_STATIC
		DEBUG_INIT_S("DDR3 Training Sequence - FAIL - Illegal R-DIMM setup\n");
		return MV_DDR3_TRAINING_ERR_BAD_R_DIMM_SETUP;
#endif
		reg |= (1 << REG_SDRAM_CONFIG_REGDIMM_OFFS);
		DEBUG_INIT_FULL_S("DDR3 - DUNIT-SET - R-DIMM\n");
	} else {
		DEBUG_INIT_FULL_S("DDR3 - DUNIT-SET - U-DIMM\n");
	}

#ifndef MV88F67XX
#ifdef DUNIT_STATIC
	if (ddr3_get_min_val(sum_info.data_width, dimm_num, BUS_WIDTH) == 64) {
#else
	if (*ddr_width == 64) {
#endif
		reg |= (1 << REG_SDRAM_CONFIG_WIDTH_OFFS);
		DEBUG_INIT_FULL_S("DDR3 - DUNIT-SET - Datawidth - 64Bits\n");
	} else {
		DEBUG_INIT_FULL_S("DDR3 - DUNIT-SET - Datawidth - 32Bits\n");
	}
#else
	DEBUG_INIT_FULL_S("DDR3 - DUNIT-SET - Datawidth - 16Bits\n");
#endif

#if defined(MV88F672X)
	if (*ddr_width == 32) {
		reg |= (1 << REG_SDRAM_CONFIG_WIDTH_OFFS);
		DEBUG_INIT_FULL_S("DDR3 - DUNIT-SET - Datawidth - 32Bits\n");
	} else {
		DEBUG_INIT_FULL_S("DDR3 - DUNIT-SET - Datawidth - 16Bits\n");
	}
#endif
	stat_val = ddr3_get_static_mc_value(REG_SDRAM_CONFIG_ADDR, 0,
					       REG_SDRAM_CONFIG_RFRS_MASK, 0, 0);
	tmp = ddr3_get_min_val(sum_info.refresh_interval / hclk_time,
			       dimm_num, stat_val);

#ifdef TREFI_USER_EN
	tmp = min(TREFI_USER / hclk_time, tmp);
#endif

	DEBUG_INIT_FULL_C("DDR3 - DUNIT-SET - RefreshInterval/Hclk = ", tmp, 4);
	reg |= tmp;

	if (cl != 3)
		reg |= (1 << 16);	/*  If 2:1 need to set P2DWr */

#if defined(MV88F672X)
	reg |= (1 << 27);	/* PhyRfRST = Disable */
#endif
	reg_write(REG_SDRAM_CONFIG_ADDR, reg);

	/*{0x00001404}  -   DDR SDRAM Configuration Register */
	reg = 0x3630B800;
#ifdef DUNIT_SPD
	reg |= (DRAM_2T << REG_DUNIT_CTRL_LOW_2T_OFFS);
#endif
	reg_write(REG_DUNIT_CTRL_LOW_ADDR, reg);

	/* {0x00001408}  -   DDR SDRAM Timing (Low) Register */
	reg = 0x0;

	/* tRAS - (0:3,20) */
	spd_val = ddr3_div(sum_info.min_active_to_precharge,
			    ddr_clk_time, 1);
	stat_val = ddr3_get_static_mc_value(REG_SDRAM_TIMING_LOW_ADDR,
					    0, 0xF, 16, 0x10);
	tmp = ddr3_get_max_val(spd_val, dimm_num, stat_val);
	DEBUG_INIT_FULL_C("DDR3 - DUNIT-SET - tRAS-1 = ", tmp, 1);
	reg |= (tmp & 0xF);
	reg |= ((tmp & 0x10) << 16);	/* to bit 20 */

	/* tRCD - (4:7) */
	spd_val = ddr3_div(sum_info.min_ras_to_cas_delay, ddr_clk_time, 1);
	stat_val = ddr3_get_static_mc_value(REG_SDRAM_TIMING_LOW_ADDR,
					    4, 0xF, 0, 0);
	tmp = ddr3_get_max_val(spd_val, dimm_num, stat_val);
	DEBUG_INIT_FULL_C("DDR3 - DUNIT-SET - tRCD-1 = ", tmp, 1);
	reg |= ((tmp & 0xF) << 4);

	/* tRP - (8:11) */
	spd_val = ddr3_div(sum_info.min_row_precharge_time, ddr_clk_time, 1);
	stat_val = ddr3_get_static_mc_value(REG_SDRAM_TIMING_LOW_ADDR,
					    8, 0xF, 0, 0);
	tmp = ddr3_get_max_val(spd_val, dimm_num, stat_val);
	DEBUG_INIT_FULL_C("DDR3 - DUNIT-SET - tRP-1 = ", tmp, 1);
	reg |= ((tmp & 0xF) << 8);

	/* tWR - (12:15) */
	spd_val = ddr3_div(sum_info.min_write_recovery_time, ddr_clk_time, 1);
	stat_val = ddr3_get_static_mc_value(REG_SDRAM_TIMING_LOW_ADDR,
					    12, 0xF, 0, 0);
	tmp = ddr3_get_max_val(spd_val, dimm_num, stat_val);
	DEBUG_INIT_FULL_C("DDR3 - DUNIT-SET - tWR-1 = ", tmp, 1);
	reg |= ((tmp & 0xF) << 12);

	/* tWTR - (16:19) */
	spd_val = ddr3_div(sum_info.min_write_to_read_cmd_delay, ddr_clk_time, 1);
	stat_val = ddr3_get_static_mc_value(REG_SDRAM_TIMING_LOW_ADDR,
					    16, 0xF, 0, 0);
	tmp = ddr3_get_max_val(spd_val, dimm_num, stat_val);
	DEBUG_INIT_FULL_C("DDR3 - DUNIT-SET - tWTR-1 = ", tmp, 1);
	reg |= ((tmp & 0xF) << 16);

	/* tRRD - (24:27) */
	spd_val = ddr3_div(sum_info.min_row_active_to_row_active, ddr_clk_time, 1);
	stat_val = ddr3_get_static_mc_value(REG_SDRAM_TIMING_LOW_ADDR,
					    24, 0xF, 0, 0);
	tmp = ddr3_get_max_val(spd_val, dimm_num, stat_val);
	DEBUG_INIT_FULL_C("DDR3 - DUNIT-SET - tRRD-1 = ", tmp, 1);
	reg |= ((tmp & 0xF) << 24);

	/* tRTP - (28:31) */
	spd_val = ddr3_div(sum_info.min_read_to_prech_cmd_delay, ddr_clk_time, 1);
	stat_val = ddr3_get_static_mc_value(REG_SDRAM_TIMING_LOW_ADDR,
					    28, 0xF, 0, 0);
	tmp = ddr3_get_max_val(spd_val, dimm_num, stat_val);
	DEBUG_INIT_FULL_C("DDR3 - DUNIT-SET - tRTP-1 = ", tmp, 1);
	reg |= ((tmp & 0xF) << 28);

	if (cl < 7)
		reg = 0x33137663;

	reg_write(REG_SDRAM_TIMING_LOW_ADDR, reg);

	/*{0x0000140C}  -   DDR SDRAM Timing (High) Register */
	/* Add cycles to R2R W2W */
	reg = 0x39F8FF80;

	/* tRFC - (0:6,16:18) */
	spd_val = ddr3_div(sum_info.min_refresh_recovery, ddr_clk_time, 1);
	stat_val = ddr3_get_static_mc_value(REG_SDRAM_TIMING_HIGH_ADDR,
					    0, 0x7F, 9, 0x380);
	tmp = ddr3_get_max_val(spd_val, dimm_num, stat_val);
	DEBUG_INIT_FULL_C("DDR3 - DUNIT-SET - tRFC-1 = ", tmp, 1);
	reg |= (tmp & 0x7F);
	reg |= ((tmp & 0x380) << 9);	/* to bit 16 */
	reg_write(REG_SDRAM_TIMING_HIGH_ADDR, reg);

	/*{0x00001410}  -   DDR SDRAM Address Control Register */
	reg = 0x000F0000;

	/* tFAW - (24:28)  */
#if (defined(MV88F78X60) || defined(MV88F672X))
	tmp = sum_info.min_four_active_win_delay;
	spd_val = ddr3_div(tmp, ddr_clk_time, 0);
	stat_val = ddr3_get_static_mc_value(REG_SDRAM_ADDRESS_CTRL_ADDR,
					    24, 0x3F, 0, 0);
	tmp = ddr3_get_max_val(spd_val, dimm_num, stat_val);
	DEBUG_INIT_FULL_C("DDR3 - DUNIT-SET - tFAW = ", tmp, 1);
	reg |= ((tmp & 0x3F) << 24);
#else
	tmp = sum_info.min_four_active_win_delay -
		4 * (sum_info.min_row_active_to_row_active);
	spd_val = ddr3_div(tmp, ddr_clk_time, 0);
	stat_val = ddr3_get_static_mc_value(REG_SDRAM_ADDRESS_CTRL_ADDR,
					    24, 0x1F, 0, 0);
	tmp = ddr3_get_max_val(spd_val, dimm_num, stat_val);
	DEBUG_INIT_FULL_C("DDR3 - DUNIT-SET - tFAW-4*tRRD = ", tmp, 1);
	reg |= ((tmp & 0x1F) << 24);
#endif

	/* SDRAM device capacity */
#ifdef DUNIT_STATIC
	reg |= (reg_read(REG_SDRAM_ADDRESS_CTRL_ADDR) & 0xF0FFFF);
#endif

#ifdef DUNIT_SPD
	cs_count = 0;
	dimm_cnt = 0;
	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs) & DIMM_CS_BITMAP) {
			if (dimm_info[dimm_cnt].num_of_module_ranks == cs_count) {
				dimm_cnt++;
				cs_count = 0;
			}
			cs_count++;
			if (dimm_info[dimm_cnt].sdram_capacity < 0x3) {
				reg |= ((dimm_info[dimm_cnt].sdram_capacity + 1) <<
					(REG_SDRAM_ADDRESS_SIZE_OFFS +
					 (REG_SDRAM_ADDRESS_CTRL_STRUCT_OFFS * cs)));
			} else if (dimm_info[dimm_cnt].sdram_capacity > 0x3) {
				reg |= ((dimm_info[dimm_cnt].sdram_capacity & 0x3) <<
					(REG_SDRAM_ADDRESS_SIZE_OFFS +
					 (REG_SDRAM_ADDRESS_CTRL_STRUCT_OFFS * cs)));
				reg |= ((dimm_info[dimm_cnt].sdram_capacity & 0x4) <<
					(REG_SDRAM_ADDRESS_SIZE_HIGH_OFFS + cs));
			}
		}
	}

	/* SDRAM device structure */
	cs_count = 0;
	dimm_cnt = 0;
	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs) & DIMM_CS_BITMAP) {
			if (dimm_info[dimm_cnt].num_of_module_ranks == cs_count) {
				dimm_cnt++;
				cs_count = 0;
			}
			cs_count++;
			if (dimm_info[dimm_cnt].sdram_width == 16)
				reg |= (1 << (REG_SDRAM_ADDRESS_CTRL_STRUCT_OFFS * cs));
		}
	}
#endif
	reg_write(REG_SDRAM_ADDRESS_CTRL_ADDR, reg);

	/*{0x00001418}  -   DDR SDRAM Operation Register */
	reg = 0xF00;
	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs))
			reg &= ~(1 << (cs + REG_SDRAM_OPERATION_CS_OFFS));
	}
	reg_write(REG_SDRAM_OPERATION_ADDR, reg);

	/*{0x00001420}  -   DDR SDRAM Extended Mode Register */
	reg = 0x00000004;
	reg_write(REG_SDRAM_EXT_MODE_ADDR, reg);

	/*{0x00001424}  -   DDR Controller Control (High) Register */
#if (defined(MV88F78X60) || defined(MV88F672X))
	reg = 0x0000D3FF;
#else
	reg = 0x0100D1FF;
#endif
	reg_write(REG_DDR_CONT_HIGH_ADDR, reg);

	/*{0x0000142C}  -   DDR3 Timing Register */
	reg = 0x014C2F38;
#if defined(MV88F78X60) || defined(MV88F672X)
	reg = 0x1FEC2F38;
#endif
	reg_write(0x142C, reg);

	/*{0x00001484}  - MBus CPU Block Register */
#ifdef MV88F67XX
	if (reg_read(REG_DDR_IO_ADDR) & (1 << REG_DDR_IO_CLK_RATIO_OFFS))
		reg_write(REG_MBUS_CPU_BLOCK_ADDR, 0x0000E907);
#endif

	/*
	 * In case of mixed dimm and on-board devices setup paramters will
	 * be taken statically
	 */
	/*{0x00001494}  -   DDR SDRAM ODT Control (Low) Register */
	reg = odt_config[cs_ena];
	reg_write(REG_SDRAM_ODT_CTRL_LOW_ADDR, reg);

	/*{0x00001498}  -   DDR SDRAM ODT Control (High) Register */
	reg = 0x00000000;
	reg_write(REG_SDRAM_ODT_CTRL_HIGH_ADDR, reg);

	/*{0x0000149C}  -   DDR Dunit ODT Control Register */
	reg = cs_ena;
	reg_write(REG_DUNIT_ODT_CTRL_ADDR, reg);

	/*{0x000014A0}  -   DDR Dunit ODT Control Register */
#if defined(MV88F672X)
	reg = 0x000006A9;
	reg_write(REG_DRAM_FIFO_CTRL_ADDR, reg);
#endif

	/*{0x000014C0}  -   DRAM address and Control Driving Strenght */
	reg_write(REG_DRAM_ADDR_CTRL_DRIVE_STRENGTH_ADDR, 0x192435e9);

	/*{0x000014C4}  -   DRAM Data and DQS Driving Strenght */
	reg_write(REG_DRAM_DATA_DQS_DRIVE_STRENGTH_ADDR, 0xB2C35E9);

#if (defined(MV88F78X60) || defined(MV88F672X))
	/*{0x000014CC}  -   DRAM Main Pads Calibration Machine Control Register */
	reg = reg_read(REG_DRAM_MAIN_PADS_CAL_ADDR);
	reg_write(REG_DRAM_MAIN_PADS_CAL_ADDR, reg | (1 << 0));
#endif

#if defined(MV88F672X)
	/* DRAM Main Pads Calibration Machine Control Register */
	/* 0x14CC[4:3] - CalUpdateControl = IntOnly */
	reg = reg_read(REG_DRAM_MAIN_PADS_CAL_ADDR);
	reg &= 0xFFFFFFE7;
	reg |= (1 << 3);
	reg_write(REG_DRAM_MAIN_PADS_CAL_ADDR, reg);
#endif

#ifdef DUNIT_SPD
	cs_count = 0;
	dimm_cnt = 0;
	for (cs = 0; cs < MAX_CS; cs++) {
		if ((1 << cs) & DIMM_CS_BITMAP) {
			if ((1 << cs) & cs_ena) {
				if (dimm_info[dimm_cnt].num_of_module_ranks ==
				    cs_count) {
					dimm_cnt++;
					cs_count = 0;
				}
				cs_count++;
				reg_write(REG_CS_SIZE_SCRATCH_ADDR + (cs * 0x8),
					  dimm_info[dimm_cnt].rank_capacity - 1);
			} else {
				reg_write(REG_CS_SIZE_SCRATCH_ADDR + (cs * 0x8), 0);
			}
		}
	}
#endif

	/*{0x00020184}  -   Close FastPath - 2G */
	reg_write(REG_FASTPATH_WIN_0_CTRL_ADDR, 0);

	/*{0x00001538}  -    Read Data Sample Delays Register */
	reg = 0;
	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs))
			reg |= (cl << (REG_READ_DATA_SAMPLE_DELAYS_OFFS * cs));
	}

	reg_write(REG_READ_DATA_SAMPLE_DELAYS_ADDR, reg);
	DEBUG_INIT_FULL_C("DDR3 - SPD-SET - Read Data Sample Delays = ", reg,
			  1);

	/*{0x0000153C}  -   Read Data Ready Delay Register */
	reg = 0;
	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs)) {
			reg |= ((cl + 2) <<
				(REG_READ_DATA_READY_DELAYS_OFFS * cs));
		}
	}
	reg_write(REG_READ_DATA_READY_DELAYS_ADDR, reg);
	DEBUG_INIT_FULL_C("DDR3 - SPD-SET - Read Data Ready Delays = ", reg, 1);

	/* Set MR registers */
	/* MR0 */
	reg = 0x00000600;
	tmp = ddr3_cl_to_valid_cl(cl);
	reg |= ((tmp & 0x1) << 2);
	reg |= ((tmp & 0xE) << 3);	/* to bit 4 */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs)) {
			reg_write(REG_DDR3_MR0_CS_ADDR +
				  (cs << MR_CS_ADDR_OFFS), reg);
		}
	}

	/* MR1 */
	reg = 0x00000044 & REG_DDR3_MR1_ODT_MASK;
	if (cs_num > 1)
		reg = 0x00000046 & REG_DDR3_MR1_ODT_MASK;

	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs)) {
			reg |= odt_static[cs_ena][cs];
			reg_write(REG_DDR3_MR1_CS_ADDR +
				  (cs << MR_CS_ADDR_OFFS), reg);
		}
	}

	/* MR2 */
	if (reg_read(REG_DDR_IO_ADDR) & (1 << REG_DDR_IO_CLK_RATIO_OFFS))
		tmp = hclk_time / 2;
	else
		tmp = hclk_time;

	if (tmp >= 2500)
		cwl = 5;	/* CWL = 5 */
	else if (tmp >= 1875 && tmp < 2500)
		cwl = 6;	/* CWL = 6 */
	else if (tmp >= 1500 && tmp < 1875)
		cwl = 7;	/* CWL = 7 */
	else if (tmp >= 1250 && tmp < 1500)
		cwl = 8;	/* CWL = 8 */
	else if (tmp >= 1070 && tmp < 1250)
		cwl = 9;	/* CWL = 9 */
	else if (tmp >= 935 && tmp < 1070)
		cwl = 10;	/* CWL = 10 */
	else if (tmp >= 833 && tmp < 935)
		cwl = 11;	/* CWL = 11 */
	else if (tmp >= 750 && tmp < 833)
		cwl = 12;	/* CWL = 12 */
	else {
		cwl = 12;	/* CWL = 12 */
		printf("Unsupported hclk %d MHz\n", tmp);
	}

	reg = ((cwl - 5) << REG_DDR3_MR2_CWL_OFFS);

	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs)) {
			reg &= REG_DDR3_MR2_ODT_MASK;
			reg |= odt_dynamic[cs_ena][cs];
			reg_write(REG_DDR3_MR2_CS_ADDR +
				  (cs << MR_CS_ADDR_OFFS), reg);
		}
	}

	/* MR3 */
	reg = 0x00000000;
	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs)) {
			reg_write(REG_DDR3_MR3_CS_ADDR +
				  (cs << MR_CS_ADDR_OFFS), reg);
		}
	}

	/* {0x00001428}  -   DDR ODT Timing (Low) Register */
	reg = 0;
	reg |= (((cl - cwl + 1) & 0xF) << 4);
	reg |= (((cl - cwl + 6) & 0xF) << 8);
	reg |= ((((cl - cwl + 6) >> 4) & 0x1) << 21);
	reg |= (((cl - 1) & 0xF) << 12);
	reg |= (((cl + 6) & 0x1F) << 16);
	reg_write(REG_ODT_TIME_LOW_ADDR, reg);

	/* {0x0000147C}  -   DDR ODT Timing (High) Register */
	reg = 0x00000071;
	reg |= ((cwl - 1) << 8);
	reg |= ((cwl + 5) << 12);
	reg_write(REG_ODT_TIME_HIGH_ADDR, reg);

#ifdef DUNIT_SPD
	/*{0x000015E0} - DDR3 Rank Control Register */
	reg = cs_ena;
	cs_count = 0;
	dimm_cnt = 0;
	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs) & DIMM_CS_BITMAP) {
			if (dimm_info[dimm_cnt].num_of_module_ranks == cs_count) {
				dimm_cnt++;
				cs_count = 0;
			}
			cs_count++;

			if (dimm_info[dimm_cnt].addr_mirroring &&
			    (cs == 1 || cs == 3) &&
			    (sum_info.type_info != SPD_MODULE_TYPE_RDIMM)) {
				reg |= (1 << (REG_DDR3_RANK_CTRL_MIRROR_OFFS + cs));
				DEBUG_INIT_FULL_C("DDR3 - SPD-SET - Setting Address Mirroring for CS = ",
						  cs, 1);
			}
		}
	}
	reg_write(REG_DDR3_RANK_CTRL_ADDR, reg);
#endif

	/*{0xD00015E4}  -   ZQDS Configuration Register */
	reg = 0x00203c18;
	reg_write(REG_ZQC_CONF_ADDR, reg);

	/* {0x00015EC}  -   DDR PHY */
#if defined(MV88F78X60)
	reg = 0xF800AAA5;
	if (mv_ctrl_rev_get() == MV_78XX0_B0_REV)
		reg = 0xF800A225;
#else
	reg = 0xDE000025;
#if defined(MV88F672X)
	reg = 0xF800A225;
#endif
#endif
	reg_write(REG_DRAM_PHY_CONFIG_ADDR, reg);

#if (defined(MV88F78X60) || defined(MV88F672X))
	/* Registered DIMM support - supported only in AXP A0 devices */
	/* Currently supported for SPD detection only */
	/*
	 * Flow is according to the Registered DIMM chapter in the
	 * Functional Spec
	 */
	if (sum_info.type_info == SPD_MODULE_TYPE_RDIMM) {
		DEBUG_INIT_S("DDR3 Training Sequence - Registered DIMM detected\n");

		/* Set commands parity completion */
		reg = reg_read(REG_REGISTERED_DRAM_CTRL_ADDR);
		reg &= ~REG_REGISTERED_DRAM_CTRL_PARITY_MASK;
		reg |= 0x8;
		reg_write(REG_REGISTERED_DRAM_CTRL_ADDR, reg);

		/* De-assert M_RESETn and assert M_CKE */
		reg_write(REG_SDRAM_INIT_CTRL_ADDR,
			  1 << REG_SDRAM_INIT_CKE_ASSERT_OFFS);
		do {
			reg = (reg_read(REG_SDRAM_INIT_CTRL_ADDR)) &
				(1 << REG_SDRAM_INIT_CKE_ASSERT_OFFS);
		} while (reg);

		for (rc = 0; rc < SPD_RDIMM_RC_NUM; rc++) {
			if (rc != 6 && rc != 7) {
				/* Set CWA Command */
				reg = (REG_SDRAM_OPERATION_CMD_CWA &
				       ~(0xF << REG_SDRAM_OPERATION_CS_OFFS));
				reg |= ((dimm_info[0].dimm_rc[rc] &
					 REG_SDRAM_OPERATION_CWA_DATA_MASK) <<
					REG_SDRAM_OPERATION_CWA_DATA_OFFS);
				reg |= rc << REG_SDRAM_OPERATION_CWA_RC_OFFS;
				/* Configure - Set Delay - tSTAB/tMRD */
				if (rc == 2 || rc == 10)
					reg |= (0x1 << REG_SDRAM_OPERATION_CWA_DELAY_SEL_OFFS);
				/* 0x1418 - SDRAM Operation Register */
				reg_write(REG_SDRAM_OPERATION_ADDR, reg);

				/*
				 * Poll the "cmd" field in the SDRAM OP
				 * register for 0x0
				 */
				do {
					reg = reg_read(REG_SDRAM_OPERATION_ADDR) &
						(REG_SDRAM_OPERATION_CMD_MASK);
				} while (reg);
			}
		}
	}
#endif

	return MV_OK;
}

/*
 * Name:     ddr3_div - this function divides integers
 * Desc:
 * Args:     val - the value
 *           divider - the divider
 *           sub - substruction value
 * Notes:
 * Returns:  required value
 */
u32 ddr3_div(u32 val, u32 divider, u32 sub)
{
	return val / divider + (val % divider > 0 ? 1 : 0) - sub;
}

/*
 * Name:     ddr3_get_max_val
 * Desc:
 * Args:
 * Notes:
 * Returns:
 */
u32 ddr3_get_max_val(u32 spd_val, u32 dimm_num, u32 static_val)
{
#ifdef DUNIT_STATIC
	if (dimm_num > 0) {
		if (spd_val >= static_val)
			return spd_val;
		else
			return static_val;
	} else {
		return static_val;
	}
#else
	return spd_val;
#endif
}

/*
 * Name:     ddr3_get_min_val
 * Desc:
 * Args:
 * Notes:
 * Returns:
 */
u32 ddr3_get_min_val(u32 spd_val, u32 dimm_num, u32 static_val)
{
#ifdef DUNIT_STATIC
	if (dimm_num > 0) {
		if (spd_val <= static_val)
			return spd_val;
		else
			return static_val;
	} else
		return static_val;
#else
	return spd_val;
#endif
}
#endif
