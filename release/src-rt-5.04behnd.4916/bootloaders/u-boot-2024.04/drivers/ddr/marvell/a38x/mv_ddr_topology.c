// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */
#include "ddr_ml_wrapper.h"
#include "mv_ddr_plat.h"

#include "mv_ddr_topology.h"
#include "mv_ddr_common.h"
#include "mv_ddr_spd.h"
#include "ddr_topology_def.h"
#include "ddr3_training_ip_db.h"
#include "ddr3_training_ip.h"
#include "mv_ddr_training_db.h"

unsigned int mv_ddr_cl_calc(unsigned int taa_min, unsigned int tclk)
{
	unsigned int cl = ceil_div(taa_min, tclk);

	return mv_ddr_spd_supported_cl_get(cl);

}

unsigned int mv_ddr_cwl_calc(unsigned int tclk)
{
	unsigned int cwl;

	if (tclk >= 1250)
		cwl = 9;
	else if (tclk >= 1071)
		cwl = 10;
	else if (tclk >= 938)
		cwl = 11;
	else if (tclk >= 833)
		cwl = 12;
	else if (tclk >= 750)
		cwl = 14;
	else if (tclk >= 625)
		cwl = 16;
	else
		cwl = 0;

	return cwl;
}

int mv_ddr_topology_map_update(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);
	unsigned int octets_per_if_num = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	enum mv_ddr_speed_bin speed_bin_index;
	enum mv_ddr_freq freq = MV_DDR_FREQ_LAST;
	unsigned int tclk;
	unsigned char val = 0;
	int i;

	if (iface_params->memory_freq == MV_DDR_FREQ_SAR)
		iface_params->memory_freq = mv_ddr_init_freq_get();

	if (tm->cfg_src == MV_DDR_CFG_SPD) {
		/* check dram device type */
		val = mv_ddr_spd_dev_type_get(&tm->spd_data);
		if (val != MV_DDR_SPD_DEV_TYPE_DDR4) {
			printf("mv_ddr: unsupported dram device type found\n");
			return -1;
		}

		/* update topology map with timing data */
		if (mv_ddr_spd_timing_calc(&tm->spd_data, tm->timing_data) > 0) {
			printf("mv_ddr: negative timing data found\n");
			return -1;
		}

		/* update device width in topology map */
		iface_params->bus_width = mv_ddr_spd_dev_width_get(&tm->spd_data);

		/* update die capacity in topology map */
		iface_params->memory_size = mv_ddr_spd_die_capacity_get(&tm->spd_data);

		/* update bus bit mask in topology map */
		tm->bus_act_mask = mv_ddr_bus_bit_mask_get();

		/* update cs bit mask in topology map */
		val = mv_ddr_spd_cs_bit_mask_get(&tm->spd_data);
		for (i = 0; i < octets_per_if_num; i++)
			iface_params->as_bus_params[i].cs_bitmask = val;

		/* check dram module type */
		val = mv_ddr_spd_module_type_get(&tm->spd_data);
		switch (val) {
		case MV_DDR_SPD_MODULE_TYPE_UDIMM:
		case MV_DDR_SPD_MODULE_TYPE_SO_DIMM:
		case MV_DDR_SPD_MODULE_TYPE_MINI_UDIMM:
		case MV_DDR_SPD_MODULE_TYPE_72BIT_SO_UDIMM:
		case MV_DDR_SPD_MODULE_TYPE_16BIT_SO_DIMM:
		case MV_DDR_SPD_MODULE_TYPE_32BIT_SO_DIMM:
			break;
		default:
			printf("mv_ddr: unsupported dram module type found\n");
			return -1;
		}

		/* update mirror bit mask in topology map */
		val = mv_ddr_spd_mem_mirror_get(&tm->spd_data);
		for (i = 0; i < octets_per_if_num; i++)
			iface_params->as_bus_params[i].mirror_enable_bitmask = val << 1;

		tclk = 1000000 / mv_ddr_freq_get(iface_params->memory_freq);
		/* update cas write latency (cwl) */
		val = mv_ddr_cwl_calc(tclk);
		if (val == 0) {
			printf("mv_ddr: unsupported cas write latency value found\n");
			return -1;
		}
		iface_params->cas_wl = val;

		/* update cas latency (cl) */
		mv_ddr_spd_supported_cls_calc(&tm->spd_data);
		val = mv_ddr_cl_calc(tm->timing_data[MV_DDR_TAA_MIN], tclk);
		if (val == 0) {
			printf("mv_ddr: unsupported cas latency value found\n");
			return -1;
		}
		iface_params->cas_l = val;
	} else if (tm->cfg_src == MV_DDR_CFG_DEFAULT) {
		/* set cas and cas-write latencies per speed bin, if they unset */
		speed_bin_index = iface_params->speed_bin_index;
		freq = iface_params->memory_freq;

		if (tm->twin_die_combined == COMBINED) {
			iface_params->bus_width = MV_DDR_DEV_WIDTH_8BIT;
			iface_params->memory_size -= 1;
		}

		if (iface_params->cas_l == 0)
			iface_params->cas_l = mv_ddr_cl_val_get(speed_bin_index, freq);

		if (iface_params->cas_wl == 0)
			iface_params->cas_wl = mv_ddr_cwl_val_get(speed_bin_index, freq);
	}

	return 0;
}

unsigned short mv_ddr_bus_bit_mask_get(void)
{
	unsigned short pri_and_ext_bus_width = 0x0;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int octets_per_if_num = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);

	if (tm->cfg_src == MV_DDR_CFG_SPD) {
		if (tm->bus_act_mask == MV_DDR_32BIT_ECC_PUP8_BUS_MASK)
			tm->spd_data.byte_fields.byte_13.bit_fields.primary_bus_width = MV_DDR_PRI_BUS_WIDTH_32;

		enum mv_ddr_pri_bus_width pri_bus_width = mv_ddr_spd_pri_bus_width_get(&tm->spd_data);
		enum mv_ddr_bus_width_ext bus_width_ext = mv_ddr_spd_bus_width_ext_get(&tm->spd_data);

		switch (pri_bus_width) {
		case MV_DDR_PRI_BUS_WIDTH_16:
			pri_and_ext_bus_width = BUS_MASK_16BIT;
			break;
		case MV_DDR_PRI_BUS_WIDTH_32: /*each bit represents byte, so 0xf-is means 4 bytes-32 bit*/
			pri_and_ext_bus_width = BUS_MASK_32BIT;
			break;
		case MV_DDR_PRI_BUS_WIDTH_64:
			pri_and_ext_bus_width = MV_DDR_64BIT_BUS_MASK;
			break;
		default:
			pri_and_ext_bus_width = 0x0;
		}

		if (bus_width_ext == MV_DDR_BUS_WIDTH_EXT_8)
			pri_and_ext_bus_width |= 1 << (octets_per_if_num - 1);
	}

	return pri_and_ext_bus_width;
}

unsigned int mv_ddr_if_bus_width_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int bus_width;

	switch (tm->bus_act_mask) {
	case BUS_MASK_16BIT:
	case BUS_MASK_16BIT_ECC:
	case BUS_MASK_16BIT_ECC_PUP3:
		bus_width = 16;
		break;
	case BUS_MASK_32BIT:
	case BUS_MASK_32BIT_ECC:
	case MV_DDR_32BIT_ECC_PUP8_BUS_MASK:
		bus_width = 32;
		break;
	case MV_DDR_64BIT_BUS_MASK:
	case MV_DDR_64BIT_ECC_PUP8_BUS_MASK:
		bus_width = 64;
		break;
	default:
		printf("mv_ddr: unsupported bus active mask parameter found\n");
		bus_width = 0;
	}

	return bus_width;
}

unsigned int mv_ddr_cs_num_get(void)
{
	unsigned int cs_num = 0;
	unsigned int cs, sphy;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);
	unsigned int sphy_max = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);

	for (sphy = 0; sphy < sphy_max; sphy++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, sphy);
		break;
	}

	for (cs = 0; cs < MAX_CS_NUM; cs++) {
		VALIDATE_ACTIVE(iface_params->as_bus_params[sphy].cs_bitmask, cs);
		cs_num++;
	}

	return cs_num;
}

int mv_ddr_is_ecc_ena(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (DDR3_IS_ECC_PUP4_MODE(tm->bus_act_mask) ||
	    DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask) ||
	    DDR3_IS_ECC_PUP8_MODE(tm->bus_act_mask))
		return 1;
	else
		return 0;
}

int mv_ddr_ck_delay_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (tm->ck_delay)
		return tm->ck_delay;

	return -1;
}

/* translate topology map definition to real memory size in bits */
static unsigned int mem_size[] = {
	ADDR_SIZE_512MB,
	ADDR_SIZE_1GB,
	ADDR_SIZE_2GB,
	ADDR_SIZE_4GB,
	ADDR_SIZE_8GB,
	ADDR_SIZE_16GB
	/* TODO: add capacity up to 256GB */
};

unsigned long long mv_ddr_mem_sz_per_cs_get(void)
{
	unsigned long long mem_sz_per_cs;
	unsigned int i, sphys, sphys_per_dunit;
	unsigned int sphy_max = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	struct if_params *iface_params = &(tm->interface_params[0]);

	/* calc number of active subphys excl. ecc one */
	for (i = 0, sphys = 0; i < sphy_max - 1; i++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, i);
		sphys++;
	}

	/* calc number of subphys per ddr unit */
	if (iface_params->bus_width == MV_DDR_DEV_WIDTH_8BIT)
		sphys_per_dunit = MV_DDR_ONE_SPHY_PER_DUNIT;
	else if (iface_params->bus_width == MV_DDR_DEV_WIDTH_16BIT)
		sphys_per_dunit = MV_DDR_TWO_SPHY_PER_DUNIT;
	else {
		printf("mv_ddr: unsupported bus width type found\n");
		return 0;
	}

	/* calc dram size per cs */
	mem_sz_per_cs = (unsigned long long)mem_size[iface_params->memory_size] *
			(unsigned long long)sphys /
			(unsigned long long)sphys_per_dunit;
	return mem_sz_per_cs;
}

unsigned long long mv_ddr_mem_sz_get(void)
{
	unsigned long long tot_mem_sz = 0;
	unsigned long long mem_sz_per_cs = 0;
	unsigned long long max_cs = mv_ddr_cs_num_get();

	mem_sz_per_cs = mv_ddr_mem_sz_per_cs_get();
	tot_mem_sz = max_cs * mem_sz_per_cs;

	return tot_mem_sz;
}

unsigned int mv_ddr_rtt_nom_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int rtt_nom = tm->edata.mem_edata.rtt_nom;

	if (rtt_nom >= MV_DDR_RTT_NOM_PARK_RZQ_LAST) {
		printf("error: %s: unsupported rtt_nom parameter found\n", __func__);
		rtt_nom = PARAM_UNDEFINED;
	}

	return rtt_nom;
}

unsigned int mv_ddr_rtt_park_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int cs_num = mv_ddr_cs_num_get();
	unsigned int rtt_park = MV_DDR_RTT_NOM_PARK_RZQ_LAST;

	if (cs_num > 0 && cs_num <= MAX_CS_NUM)
		rtt_park = tm->edata.mem_edata.rtt_park[cs_num - 1];

	if (rtt_park >= MV_DDR_RTT_NOM_PARK_RZQ_LAST) {
		printf("error: %s: unsupported rtt_park parameter found\n", __func__);
		rtt_park = PARAM_UNDEFINED;
	}

	return rtt_park;
}

unsigned int mv_ddr_rtt_wr_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int cs_num = mv_ddr_cs_num_get();
	unsigned int rtt_wr = MV_DDR_RTT_WR_RZQ_LAST;

	if (cs_num > 0 && cs_num <= MAX_CS_NUM)
		rtt_wr = tm->edata.mem_edata.rtt_wr[cs_num - 1];

	if (rtt_wr >= MV_DDR_RTT_WR_RZQ_LAST) {
		printf("error: %s: unsupported rtt_wr parameter found\n", __func__);
		rtt_wr = PARAM_UNDEFINED;
	}

	return rtt_wr;
}

unsigned int mv_ddr_dic_get(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	unsigned int dic = tm->edata.mem_edata.dic;

	if (dic >= MV_DDR_DIC_RZQ_LAST) {
		printf("error: %s: unsupported dic parameter found\n", __func__);
		dic = PARAM_UNDEFINED;
	}

	return dic;
}
