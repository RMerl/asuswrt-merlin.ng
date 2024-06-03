/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _HIGH_SPEED_ENV_SPEC_H
#define _HIGH_SPEED_ENV_SPEC_H

#include "seq_exec.h"

/*
 * For setting or clearing a certain bit (bit is a number between 0 and 31)
 * in the data
 */
#define SET_BIT(data, bit)		((data) | (0x1 << (bit)))
#define CLEAR_BIT(data, bit)		((data) & (~(0x1 << (bit))))

#define MAX_SERDES_LANES		7	/* as in a39x */

/* Serdes revision */
/* Serdes revision 1.2 (for A38x-Z1) */
#define MV_SERDES_REV_1_2		0x0
/* Serdes revision 2.1 (for A39x-Z1, A38x-A0) */
#define MV_SERDES_REV_2_1		0x1
#define MV_SERDES_REV_NA		0xff

#define	SERDES_REGS_LANE_BASE_OFFSET(lane)	(0x800 * (lane))

#define PEX_X4_ENABLE_OFFS						\
	(hws_ctrl_serdes_rev_get() == MV_SERDES_REV_1_2 ? 18 : 31)

/* Serdes lane types */
enum serdes_type {
	PEX0,
	PEX1,
	PEX2,
	PEX3,
	SATA0,
	SATA1,
	SATA2,
	SATA3,
	SGMII0,
	SGMII1,
	SGMII2,
	QSGMII,
	USB3_HOST0,
	USB3_HOST1,
	USB3_DEVICE,
	SGMII3,
	XAUI,
	RXAUI,
	DEFAULT_SERDES,
	LAST_SERDES_TYPE
};

/* Serdes baud rates */
enum serdes_speed {
	SERDES_SPEED_1_25_GBPS,
	SERDES_SPEED_1_5_GBPS,
	SERDES_SPEED_2_5_GBPS,
	SERDES_SPEED_3_GBPS,
	SERDES_SPEED_3_125_GBPS,
	SERDES_SPEED_5_GBPS,
	SERDES_SPEED_6_GBPS,
	SERDES_SPEED_6_25_GBPS,
	LAST_SERDES_SPEED
};

/* Serdes modes */
enum serdes_mode {
	PEX_ROOT_COMPLEX_X1,
	PEX_ROOT_COMPLEX_X4,
	PEX_END_POINT_X1,
	PEX_END_POINT_X4,

	SERDES_DEFAULT_MODE, /* not pex */

	SERDES_LAST_MODE
};

struct serdes_map {
	enum serdes_type	serdes_type;
	enum serdes_speed	serdes_speed;
	enum serdes_mode	serdes_mode;
	int			swap_rx;
	int			swap_tx;
};

/* Serdes ref clock options */
enum ref_clock {
	REF_CLOCK_25MHZ,
	REF_CLOCK_100MHZ,
	REF_CLOCK_40MHZ,
	REF_CLOCK_UNSUPPORTED
};

/* Serdes sequences */
enum serdes_seq {
	SATA_PORT_0_ONLY_POWER_UP_SEQ,
	SATA_PORT_1_ONLY_POWER_UP_SEQ,
	SATA_POWER_UP_SEQ,
	SATA_1_5_SPEED_CONFIG_SEQ,
	SATA_3_SPEED_CONFIG_SEQ,
	SATA_6_SPEED_CONFIG_SEQ,
	SATA_ELECTRICAL_CONFIG_SEQ,
	SATA_TX_CONFIG_SEQ1,
	SATA_PORT_0_ONLY_TX_CONFIG_SEQ,
	SATA_PORT_1_ONLY_TX_CONFIG_SEQ,
	SATA_TX_CONFIG_SEQ2,

	SGMII_POWER_UP_SEQ,
	SGMII_1_25_SPEED_CONFIG_SEQ,
	SGMII_3_125_SPEED_CONFIG_SEQ,
	SGMII_ELECTRICAL_CONFIG_SEQ,
	SGMII_TX_CONFIG_SEQ1,
	SGMII_TX_CONFIG_SEQ2,

	PEX_POWER_UP_SEQ,
	PEX_2_5_SPEED_CONFIG_SEQ,
	PEX_5_SPEED_CONFIG_SEQ,
	PEX_ELECTRICAL_CONFIG_SEQ,
	PEX_TX_CONFIG_SEQ1,
	PEX_TX_CONFIG_SEQ2,
	PEX_TX_CONFIG_SEQ3,
	PEX_BY_4_CONFIG_SEQ,
	PEX_CONFIG_REF_CLOCK_25MHZ_SEQ,
	PEX_CONFIG_REF_CLOCK_100MHZ_SEQ,
	PEX_CONFIG_REF_CLOCK_40MHZ_SEQ,

	USB3_POWER_UP_SEQ,
	USB3_HOST_SPEED_CONFIG_SEQ,
	USB3_DEVICE_SPEED_CONFIG_SEQ,
	USB3_ELECTRICAL_CONFIG_SEQ,
	USB3_TX_CONFIG_SEQ1,
	USB3_TX_CONFIG_SEQ2,
	USB3_TX_CONFIG_SEQ3,
	USB3_DEVICE_CONFIG_SEQ,

	USB2_POWER_UP_SEQ,

	SERDES_POWER_DOWN_SEQ,

	SGMII3_POWER_UP_SEQ,
	SGMII3_1_25_SPEED_CONFIG_SEQ,
	SGMII3_TX_CONFIG_SEQ1,
	SGMII3_TX_CONFIG_SEQ2,

	QSGMII_POWER_UP_SEQ,
	QSGMII_5_SPEED_CONFIG_SEQ,
	QSGMII_ELECTRICAL_CONFIG_SEQ,
	QSGMII_TX_CONFIG_SEQ1,
	QSGMII_TX_CONFIG_SEQ2,

	XAUI_POWER_UP_SEQ,
	XAUI_3_125_SPEED_CONFIG_SEQ,
	XAUI_ELECTRICAL_CONFIG_SEQ,
	XAUI_TX_CONFIG_SEQ1,
	XAUI_TX_CONFIG_SEQ2,

	RXAUI_POWER_UP_SEQ,
	RXAUI_6_25_SPEED_CONFIG_SEQ,
	RXAUI_ELECTRICAL_CONFIG_SEQ,
	RXAUI_TX_CONFIG_SEQ1,
	RXAUI_TX_CONFIG_SEQ2,

	SERDES_LAST_SEQ
};

/* The different sequence types for PEX and USB3 */
enum {
	PEX,
	USB3,
	LAST_PEX_USB_SEQ_TYPE
};

enum {
	PEXSERDES_SPEED_2_5_GBPS,
	PEXSERDES_SPEED_5_GBPS,
	USB3SERDES_SPEED_5_GBPS_HOST,
	USB3SERDES_SPEED_5_GBPS_DEVICE,
	LAST_PEX_USB_SPEED_SEQ_TYPE
};

/* The different sequence types for SATA and SGMII */
enum {
	SATA,
	SGMII,
	SGMII_3_125,
	LAST_SATA_SGMII_SEQ_TYPE
};

enum {
	QSGMII_SEQ_IDX,
	LAST_QSGMII_SEQ_TYPE
};

enum {
	XAUI_SEQ_IDX,
	RXAUI_SEQ_IDX,
	LAST_XAUI_RXAUI_SEQ_TYPE
};

enum {
	SATASERDES_SPEED_1_5_GBPS,
	SATASERDES_SPEED_3_GBPS,
	SATASERDES_SPEED_6_GBPS,
	SGMIISERDES_SPEED_1_25_GBPS,
	SGMIISERDES_SPEED_3_125_GBPS,
	LAST_SATA_SGMII_SPEED_SEQ_TYPE
};

extern u8 selectors_serdes_rev1_map[LAST_SERDES_TYPE][MAX_SERDES_LANES];
extern u8 selectors_serdes_rev2_map[LAST_SERDES_TYPE][MAX_SERDES_LANES];

u8 hws_ctrl_serdes_rev_get(void);
int mv_update_serdes_select_phy_mode_seq(void);
int hws_board_topology_load(struct serdes_map **serdes_map, u8 *count);
enum serdes_seq serdes_type_and_speed_to_speed_seq(enum serdes_type serdes_type,
						   enum serdes_speed baud_rate);
int hws_serdes_seq_init(void);
int hws_serdes_seq_db_init(void);
int hws_power_up_serdes_lanes(struct serdes_map *serdes_map, u8 count);
int hws_ctrl_high_speed_serdes_phy_config(void);
int serdes_power_up_ctrl(u32 serdes_num, int serdes_power_up,
			 enum serdes_type serdes_type,
			 enum serdes_speed baud_rate,
			 enum serdes_mode serdes_mode,
			 enum ref_clock ref_clock);
int serdes_power_up_ctrl_ext(u32 serdes_num, int serdes_power_up,
			     enum serdes_type serdes_type,
			     enum serdes_speed baud_rate,
			     enum serdes_mode serdes_mode,
			     enum ref_clock ref_clock);
u32 hws_serdes_silicon_ref_clock_get(void);
int hws_serdes_pex_ref_clock_get(enum serdes_type serdes_type,
				 enum ref_clock *ref_clock);
int hws_ref_clock_set(u32 serdes_num, enum serdes_type serdes_type,
		      enum ref_clock ref_clock);
int hws_update_serdes_phy_selectors(struct serdes_map *serdes_map, u8 count);
u32 hws_serdes_get_phy_selector_val(int serdes_num,
				    enum serdes_type serdes_type);
u32 hws_serdes_get_ref_clock_val(enum serdes_type serdes_type);
u32 hws_serdes_get_max_lane(void);
int hws_get_ext_base_addr(u32 serdes_num, u32 base_addr, u32 unit_base_offset,
			  u32 *unit_base_reg, u32 *unit_offset);
int hws_pex_tx_config_seq(const struct serdes_map *serdes_map, u8 count);
u32 hws_get_physical_serdes_num(u32 serdes_num);
int hws_is_serdes_active(u8 lane_num);

#endif /* _HIGH_SPEED_ENV_SPEC_H */
