/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015-2016 Marvell International Ltd.
 */

#ifndef _COMPHY_CORE_H_
#define _COMPHY_CORE_H_

#include <fdtdec.h>
#include <mvebu/comphy.h>
#include <dt-bindings/comphy/comphy_data.h>

#if defined(DEBUG)
#define debug_enter()	printf("----> Enter %s\n", __func__);
#define debug_exit()	printf("<---- Exit  %s\n", __func__);
#else
#define debug_enter()
#define debug_exit()
#endif

#define MAX_LANE_OPTIONS			10
#define MAX_UTMI_PHY_COUNT			6

struct comphy_map {
	u32 type;
	u32 speed;
	u32 invert;
	bool clk_src;
	bool end_point;
};

struct comphy_mux_options {
	u32 type;
	u32 mux_value;
};

struct comphy_mux_data {
	u32 max_lane_values;
	struct comphy_mux_options mux_values[MAX_LANE_OPTIONS];
};

struct chip_serdes_phy_config {
	struct comphy_mux_data *mux_data;
	int (*comphy_init_map)(int, struct chip_serdes_phy_config *);
	int (*ptr_comphy_chip_init)(struct chip_serdes_phy_config *,
				    struct comphy_map *);
	int (*rx_training)(struct chip_serdes_phy_config *, u32);
	void __iomem *comphy_base_addr;
	void __iomem *hpipe3_base_addr;
	u32 comphy_lanes_count;
	u32 comphy_mux_bitcount;
	const fdt32_t *comphy_mux_lane_order;
	u32 cp_index;
	struct comphy_map comphy_map_data[MAX_LANE_OPTIONS];
};

/* Register helper functions */
static inline void reg_set_silent(void __iomem *addr, u32 data, u32 mask)
{
	u32 reg_data;

	reg_data = readl(addr);
	reg_data &= ~mask;
	reg_data |= data;
	writel(reg_data, addr);
}

static inline void reg_set(void __iomem *addr, u32 data, u32 mask)
{
	debug("Write to address = %#010lx, data = %#010x (mask = %#010x) - ",
	      (unsigned long)addr, data, mask);
	debug("old value = %#010x ==> ", readl(addr));
	reg_set_silent(addr, data, mask);
	debug("new value %#010x\n", readl(addr));
}

static inline void reg_set_silent16(void __iomem *addr, u16 data, u16 mask)
{
	u16 reg_data;

	reg_data = readw(addr);
	reg_data &= ~mask;
	reg_data |= data;
	writew(reg_data, addr);
}

static inline void reg_set16(void __iomem *addr, u16 data, u16 mask)
{
	debug("Write to address = %#010lx, data = %#06x (mask = %#06x) - ",
	      (unsigned long)addr, data, mask);
	debug("old value = %#06x ==> ", readw(addr));
	reg_set_silent16(addr, data, mask);
	debug("new value %#06x\n", readw(addr));
}

/* SoC specific init functions */
#ifdef CONFIG_ARMADA_3700
int comphy_a3700_init_serdes_map(int node, struct chip_serdes_phy_config *cfg);
int comphy_a3700_init(struct chip_serdes_phy_config *ptr_chip_cfg,
		      struct comphy_map *serdes_map);
#else
static inline int
comphy_a3700_init_serdes_map(int node, struct chip_serdes_phy_config *cfg)
{
	/*
	 * This function should never be called in this configuration, so
	 * lets return an error here.
	 */
	return -1;
}

static inline int comphy_a3700_init(struct chip_serdes_phy_config *ptr_chip_cfg,
				    struct comphy_map *serdes_map)
{
	/*
	 * This function should never be called in this configuration, so
	 * lets return an error here.
	 */
	return -1;
}
#endif

#ifdef CONFIG_ARMADA_8K
int comphy_cp110_init_serdes_map(int node, struct chip_serdes_phy_config *cfg);
int comphy_cp110_init(struct chip_serdes_phy_config *ptr_chip_cfg,
		      struct comphy_map *serdes_map);
int comphy_cp110_sfi_rx_training(struct chip_serdes_phy_config *ptr_chip_cfg,
				 u32 lane);
#else
static inline int
comphy_cp110_init_serdes_map(int node, struct chip_serdes_phy_config *cfg)
{
	/*
	 * This function should never be called in this configuration, so
	 * lets return an error here.
	 */
	return -1;
}

static inline int comphy_cp110_init(struct chip_serdes_phy_config *ptr_chip_cfg,
		      struct comphy_map *serdes_map)
{
	/*
	 * This function should never be called in this configuration, so
	 * lets return an error here.
	 */
	return -1;
}

static inline int comphy_cp110_sfi_rx_training(
	struct chip_serdes_phy_config *ptr_chip_cfg,
	u32 lane)
{
	/*
	 * This function should never be called in this configuration, so
	 * lets return an error here.
	 */
	return -1;
}
#endif

void comphy_dedicated_phys_init(void);

/* MUX function */
void comphy_mux_init(struct chip_serdes_phy_config *ptr_chip_cfg,
		     struct comphy_map *comphy_map_data,
		     void __iomem *selector_base);

void comphy_pcie_config_set(u32 comphy_max_count,
			    struct comphy_map *serdes_map);
void comphy_pcie_config_detect(u32 comphy_max_count,
			       struct comphy_map *serdes_map);
void comphy_pcie_unit_general_config(u32 pex_index);

#endif /* _COMPHY_CORE_H_ */
