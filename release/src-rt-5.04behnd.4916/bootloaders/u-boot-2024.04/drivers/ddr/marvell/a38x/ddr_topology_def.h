/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _DDR_TOPOLOGY_DEF_H
#define _DDR_TOPOLOGY_DEF_H

#include "ddr3_training_ip_def.h"
#include "mv_ddr_topology.h"
#include "mv_ddr_spd.h"
#include "ddr3_logging_def.h"

#define MV_DDR_MAX_BUS_NUM	9
#define MV_DDR_MAX_IFACE_NUM	1

enum mv_ddr_twin_die {
	NOT_COMBINED,
	COMBINED,
};

struct bus_params {
	/* Chip Select (CS) bitmask (bits 0-CS0, bit 1- CS1 ...) */
	u8 cs_bitmask;

	/*
	 * mirror enable/disable
	 * (bits 0-CS0 mirroring, bit 1- CS1 mirroring ...)
	 */
	int mirror_enable_bitmask;

	/* DQS Swap (polarity) - true if enable */
	int is_dqs_swap;

	/* CK swap (polarity) - true if enable */
	int is_ck_swap;
};

struct if_params {
	/* bus configuration */
	struct bus_params as_bus_params[MV_DDR_MAX_BUS_NUM];

	/* Speed Bin Table */
	enum mv_ddr_speed_bin speed_bin_index;

	/* sdram device width */
	enum mv_ddr_dev_width bus_width;

	/* total sdram capacity per die, megabits */
	enum mv_ddr_die_capacity memory_size;

	/* The DDR frequency for each interfaces */
	enum mv_ddr_freq memory_freq;

	/*
	 * delay CAS Write Latency
	 * - 0 for using default value (jedec suggested)
	 */
	u8 cas_wl;

	/*
	 * delay CAS Latency
	 * - 0 for using default value (jedec suggested)
	 */
	u8 cas_l;

	/* operation temperature */
	enum mv_ddr_temperature interface_temp;

	/* 2T vs 1T mode (by default computed from number of CSs) */
	enum mv_ddr_timing timing;
};

/* memory electrical configuration */
struct mv_ddr_mem_edata {
	enum mv_ddr_rtt_nom_park_evalue rtt_nom;
	enum mv_ddr_rtt_nom_park_evalue rtt_park[MAX_CS_NUM];
	enum mv_ddr_rtt_wr_evalue rtt_wr[MAX_CS_NUM];
	enum mv_ddr_dic_evalue dic;
};

/* phy electrical configuration */
struct mv_ddr_phy_edata {
	enum mv_ddr_ohm_evalue drv_data_p;
	enum mv_ddr_ohm_evalue drv_data_n;
	enum mv_ddr_ohm_evalue drv_ctrl_p;
	enum mv_ddr_ohm_evalue drv_ctrl_n;
	enum mv_ddr_ohm_evalue odt_p[MAX_CS_NUM];
	enum mv_ddr_ohm_evalue odt_n[MAX_CS_NUM];
};

/* mac electrical configuration */
struct mv_ddr_mac_edata {
	enum mv_ddr_odt_cfg_evalue odt_cfg_pat;
	enum mv_ddr_odt_cfg_evalue odt_cfg_wr;
	enum mv_ddr_odt_cfg_evalue odt_cfg_rd;
};

struct mv_ddr_edata {
	struct mv_ddr_mem_edata mem_edata;
	struct mv_ddr_phy_edata phy_edata;
	struct mv_ddr_mac_edata mac_edata;
};

struct mv_ddr_topology_map {
	/* debug level configuration */
	enum mv_ddr_debug_level debug_level;

	/* Number of interfaces (default is 12) */
	u8 if_act_mask;

	/* Controller configuration per interface */
	struct if_params interface_params[MV_DDR_MAX_IFACE_NUM];

	/* Bit mask for active buses */
	u16 bus_act_mask;

	/* source of ddr configuration data */
	enum mv_ddr_cfg_src cfg_src;

	/* ddr twin-die */
	enum mv_ddr_twin_die twin_die_combined;

	/* raw spd data */
	union mv_ddr_spd_data spd_data;

	/* timing parameters */
	unsigned int timing_data[MV_DDR_TDATA_LAST];

	/* electrical configuration */
	struct mv_ddr_edata edata;

	/* electrical parameters */
	unsigned int electrical_data[MV_DDR_EDATA_LAST];

	/* ODT configuration */
	u32 odt_config;

	/* Clock enable mask */
	u32 clk_enable;

	/* Clock delay */
	int ck_delay;
};

enum mv_ddr_iface_mode {
	MV_DDR_RAR_ENA,
	MV_DDR_RAR_DIS,
};

enum mv_ddr_iface_state {
	MV_DDR_IFACE_NRDY,	/* not ready */
	MV_DDR_IFACE_INIT,	/* init'd */
	MV_DDR_IFACE_RDY,	/* ready */
	MV_DDR_IFACE_DNE	/* does not exist */
};

enum mv_ddr_validation {
	MV_DDR_VAL_DIS,
	MV_DDR_VAL_RX,
	MV_DDR_VAL_TX,
	MV_DDR_VAL_RX_TX,
	MV_DDR_MEMORY_CHECK
};

enum mv_ddr_sscg {
	SSCG_EN,
	SSCG_DIS,
};

struct mv_ddr_iface {
	/* base addr of ap ddr interface belongs to */
	unsigned int ap_base;

	/* ddr interface id */
	unsigned int id;

	/* ddr interface state */
	enum mv_ddr_iface_state state;

	/* ddr interface mode (rar enabled/disabled) */
	enum mv_ddr_iface_mode iface_mode;

	/* ddr interface base address */
	unsigned long long iface_base_addr;

	/* ddr interface size - ddr flow will update this parameter */
	unsigned long long iface_byte_size;

	/* ddr i2c spd data address */
	unsigned int spd_data_addr;

	/* ddr i2c spd page 0 select address */
	unsigned int spd_page_sel_addr;

	/* ddr interface validation mode */
	enum mv_ddr_validation validation;

	/* ddr interface validation mode */
	enum mv_ddr_sscg sscg;

	/* ddr interface topology map */
	struct mv_ddr_topology_map tm;

};

struct mv_ddr_iface *mv_ddr_iface_get(void);

/* DDR3 training global configuration parameters */
struct tune_train_params {
	u32 ck_delay;
	u32 phy_reg3_val;
	u32 g_zpri_data;
	u32 g_znri_data;
	u32 g_zpri_ctrl;
	u32 g_znri_ctrl;
	u32 g_zpodt_data;
	u32 g_znodt_data;
	u32 g_zpodt_ctrl;
	u32 g_znodt_ctrl;
	u32 g_dic;
	u32 g_odt_config;
	u32 g_rtt_nom;
	u32 g_rtt_wr;
	u32 g_rtt_park;
};

#endif /* _DDR_TOPOLOGY_DEF_H */
