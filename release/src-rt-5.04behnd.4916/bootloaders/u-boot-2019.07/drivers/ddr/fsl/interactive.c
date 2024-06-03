// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2018 NXP Semiconductor
 */

/*
 * Generic driver for Freescale DDR/DDR2/DDR3 memory controller.
 * Based on code from spd_sdram.c
 * Author: James Yang [at freescale.com]
 *         York Sun [at freescale.com]
 */

#include <common.h>
#include <cli.h>
#include <linux/ctype.h>
#include <asm/types.h>
#include <asm/io.h>

#include <fsl_ddr_sdram.h>
#include <fsl_ddr.h>

/* Option parameter Structures */
struct options_string {
	const char *option_name;
	size_t offset;
	unsigned int size;
	const char printhex;
};

static unsigned int picos_to_mhz(unsigned int picos)
{
	return 1000000 / picos;
}

static void print_option_table(const struct options_string *table,
			 int table_size,
			 const void *base)
{
	unsigned int i;
	unsigned int *ptr;
	unsigned long long *ptr_l;

	for (i = 0; i < table_size; i++) {
		switch (table[i].size) {
		case 4:
			ptr = (unsigned int *) (base + table[i].offset);
			if (table[i].printhex) {
				printf("%s = 0x%08X\n",
					table[i].option_name, *ptr);
			} else {
				printf("%s = %u\n",
					table[i].option_name, *ptr);
			}
			break;
		case 8:
			ptr_l = (unsigned long long *) (base + table[i].offset);
			printf("%s = %llu\n",
				table[i].option_name, *ptr_l);
			break;
		default:
			printf("Unrecognized size!\n");
			break;
		}
	}
}

static int handle_option_table(const struct options_string *table,
			 int table_size,
			 void *base,
			 const char *opt,
			 const char *val)
{
	unsigned int i;
	unsigned int value, *ptr;
	unsigned long long value_l, *ptr_l;

	for (i = 0; i < table_size; i++) {
		if (strcmp(table[i].option_name, opt) != 0)
			continue;
		switch (table[i].size) {
		case 4:
			value = simple_strtoul(val, NULL, 0);
			ptr = base + table[i].offset;
			*ptr = value;
			break;
		case 8:
			value_l = simple_strtoull(val, NULL, 0);
			ptr_l = base + table[i].offset;
			*ptr_l = value_l;
			break;
		default:
			printf("Unrecognized size!\n");
			break;
		}
		return 1;
	}

	return 0;
}

static void fsl_ddr_generic_edit(void *pdata,
			   void *pend,
			   unsigned int element_size,
			   unsigned int element_num,
			   unsigned int value)
{
	char *pcdata = (char *)pdata;		/* BIG ENDIAN ONLY */

	pcdata += element_num * element_size;
	if ((pcdata + element_size) > (char *) pend) {
		printf("trying to write past end of data\n");
		return;
	}

	switch (element_size) {
	case 1:
		__raw_writeb(value, pcdata);
		break;
	case 2:
		__raw_writew(value, pcdata);
		break;
	case 4:
		__raw_writel(value, pcdata);
		break;
	default:
		printf("unexpected element size %u\n", element_size);
		break;
	}
}

static void fsl_ddr_spd_edit(fsl_ddr_info_t *pinfo,
		       unsigned int ctrl_num,
		       unsigned int dimm_num,
		       unsigned int element_num,
		       unsigned int value)
{
	generic_spd_eeprom_t *pspd;

	pspd = &(pinfo->spd_installed_dimms[ctrl_num][dimm_num]);
	fsl_ddr_generic_edit(pspd, pspd + 1, 1, element_num, value);
}

#define COMMON_TIMING(x) {#x, offsetof(common_timing_params_t, x), \
	sizeof((common_timing_params_t *)0)->x, 0}

static void lowest_common_dimm_parameters_edit(fsl_ddr_info_t *pinfo,
					unsigned int ctrl_num,
					const char *optname_str,
					const char *value_str)
{
	common_timing_params_t *p = &pinfo->common_timing_params[ctrl_num];

	static const struct options_string options[] = {
		COMMON_TIMING(tckmin_x_ps),
		COMMON_TIMING(tckmax_ps),
#if defined(CONFIG_SYS_FSL_DDR3) || defined(CONFIG_SYS_FSL_DDR4)
		COMMON_TIMING(taamin_ps),
#endif
		COMMON_TIMING(trcd_ps),
		COMMON_TIMING(trp_ps),
		COMMON_TIMING(tras_ps),

#ifdef CONFIG_SYS_FSL_DDR4
		COMMON_TIMING(trfc1_ps),
		COMMON_TIMING(trfc2_ps),
		COMMON_TIMING(trfc4_ps),
		COMMON_TIMING(trrds_ps),
		COMMON_TIMING(trrdl_ps),
		COMMON_TIMING(tccdl_ps),
		COMMON_TIMING(trfc_slr_ps),
#else
		COMMON_TIMING(twtr_ps),
		COMMON_TIMING(trfc_ps),
		COMMON_TIMING(trrd_ps),
		COMMON_TIMING(trtp_ps),
#endif
		COMMON_TIMING(twr_ps),
		COMMON_TIMING(trc_ps),
		COMMON_TIMING(refresh_rate_ps),
		COMMON_TIMING(extended_op_srt),
#if defined(CONFIG_SYS_FSL_DDR1) || defined(CONFIG_SYS_FSL_DDR2)
		COMMON_TIMING(tis_ps),
		COMMON_TIMING(tih_ps),
		COMMON_TIMING(tds_ps),
		COMMON_TIMING(tdh_ps),
		COMMON_TIMING(tdqsq_max_ps),
		COMMON_TIMING(tqhs_ps),
#endif
		COMMON_TIMING(ndimms_present),
		COMMON_TIMING(lowest_common_spd_caslat),
		COMMON_TIMING(highest_common_derated_caslat),
		COMMON_TIMING(additive_latency),
		COMMON_TIMING(all_dimms_burst_lengths_bitmask),
		COMMON_TIMING(all_dimms_registered),
		COMMON_TIMING(all_dimms_unbuffered),
		COMMON_TIMING(all_dimms_ecc_capable),
		COMMON_TIMING(total_mem),
		COMMON_TIMING(base_address),
	};
	static const unsigned int n_opts = ARRAY_SIZE(options);

	if (handle_option_table(options, n_opts, p, optname_str, value_str))
		return;

	printf("Error: couldn't find option string %s\n", optname_str);
}

#define DIMM_PARM(x) {#x, offsetof(dimm_params_t, x), \
	sizeof((dimm_params_t *)0)->x, 0}
#define DIMM_PARM_HEX(x) {#x, offsetof(dimm_params_t, x), \
	sizeof((dimm_params_t *)0)->x, 1}

static void fsl_ddr_dimm_parameters_edit(fsl_ddr_info_t *pinfo,
				   unsigned int ctrl_num,
				   unsigned int dimm_num,
				   const char *optname_str,
				   const char *value_str)
{
	dimm_params_t *p = &(pinfo->dimm_params[ctrl_num][dimm_num]);

	static const struct options_string options[] = {
		DIMM_PARM(n_ranks),
		DIMM_PARM(data_width),
		DIMM_PARM(primary_sdram_width),
		DIMM_PARM(ec_sdram_width),
		DIMM_PARM(package_3ds),
		DIMM_PARM(registered_dimm),
		DIMM_PARM(mirrored_dimm),
		DIMM_PARM(device_width),

		DIMM_PARM(n_row_addr),
		DIMM_PARM(n_col_addr),
		DIMM_PARM(edc_config),
#ifdef CONFIG_SYS_FSL_DDR4
		DIMM_PARM(bank_addr_bits),
		DIMM_PARM(bank_group_bits),
		DIMM_PARM_HEX(die_density),
#else
		DIMM_PARM(n_banks_per_sdram_device),
#endif
		DIMM_PARM(burst_lengths_bitmask),

		DIMM_PARM(tckmin_x_ps),
		DIMM_PARM(tckmin_x_minus_1_ps),
		DIMM_PARM(tckmin_x_minus_2_ps),
		DIMM_PARM(tckmax_ps),

		DIMM_PARM(caslat_x),
		DIMM_PARM(caslat_x_minus_1),
		DIMM_PARM(caslat_x_minus_2),

		DIMM_PARM(caslat_lowest_derated),

		DIMM_PARM(trcd_ps),
		DIMM_PARM(trp_ps),
		DIMM_PARM(tras_ps),
#ifdef CONFIG_SYS_FSL_DDR4
		DIMM_PARM(trfc1_ps),
		DIMM_PARM(trfc2_ps),
		DIMM_PARM(trfc4_ps),
		DIMM_PARM(trrds_ps),
		DIMM_PARM(trrdl_ps),
		DIMM_PARM(tccdl_ps),
		DIMM_PARM(trfc_slr_ps),
#else
		DIMM_PARM(twr_ps),
		DIMM_PARM(twtr_ps),
		DIMM_PARM(trfc_ps),
		DIMM_PARM(trrd_ps),
		DIMM_PARM(trtp_ps),
#endif
		DIMM_PARM(trc_ps),
		DIMM_PARM(refresh_rate_ps),
		DIMM_PARM(extended_op_srt),

#if defined(CONFIG_SYS_FSL_DDR1) || defined(CONFIG_SYS_FSL_DDR2)
		DIMM_PARM(tis_ps),
		DIMM_PARM(tih_ps),
		DIMM_PARM(tds_ps),
		DIMM_PARM(tdh_ps),
		DIMM_PARM(tdqsq_max_ps),
		DIMM_PARM(tqhs_ps),
#endif
#ifdef CONFIG_SYS_FSL_DDR4
		DIMM_PARM_HEX(dq_mapping[0]),
		DIMM_PARM_HEX(dq_mapping[1]),
		DIMM_PARM_HEX(dq_mapping[2]),
		DIMM_PARM_HEX(dq_mapping[3]),
		DIMM_PARM_HEX(dq_mapping[4]),
		DIMM_PARM_HEX(dq_mapping[5]),
		DIMM_PARM_HEX(dq_mapping[6]),
		DIMM_PARM_HEX(dq_mapping[7]),
		DIMM_PARM_HEX(dq_mapping[8]),
		DIMM_PARM_HEX(dq_mapping[9]),
		DIMM_PARM_HEX(dq_mapping[10]),
		DIMM_PARM_HEX(dq_mapping[11]),
		DIMM_PARM_HEX(dq_mapping[12]),
		DIMM_PARM_HEX(dq_mapping[13]),
		DIMM_PARM_HEX(dq_mapping[14]),
		DIMM_PARM_HEX(dq_mapping[15]),
		DIMM_PARM_HEX(dq_mapping[16]),
		DIMM_PARM_HEX(dq_mapping[17]),
		DIMM_PARM(dq_mapping_ors),
#endif
		DIMM_PARM(rank_density),
		DIMM_PARM(capacity),
		DIMM_PARM(base_address),
	};

	static const unsigned int n_opts = ARRAY_SIZE(options);

	if (handle_option_table(options, n_opts, p, optname_str, value_str))
		return;

	printf("couldn't find option string %s\n", optname_str);
}

static void print_dimm_parameters(const dimm_params_t *pdimm)
{
	static const struct options_string options[] = {
		DIMM_PARM(n_ranks),
		DIMM_PARM(data_width),
		DIMM_PARM(primary_sdram_width),
		DIMM_PARM(ec_sdram_width),
		DIMM_PARM(package_3ds),
		DIMM_PARM(registered_dimm),
		DIMM_PARM(mirrored_dimm),
		DIMM_PARM(device_width),

		DIMM_PARM(n_row_addr),
		DIMM_PARM(n_col_addr),
		DIMM_PARM(edc_config),
#ifdef CONFIG_SYS_FSL_DDR4
		DIMM_PARM(bank_addr_bits),
		DIMM_PARM(bank_group_bits),
		DIMM_PARM_HEX(die_density),
#else
		DIMM_PARM(n_banks_per_sdram_device),
#endif

		DIMM_PARM(tckmin_x_ps),
		DIMM_PARM(tckmin_x_minus_1_ps),
		DIMM_PARM(tckmin_x_minus_2_ps),
		DIMM_PARM(tckmax_ps),

		DIMM_PARM(caslat_x),
		DIMM_PARM_HEX(caslat_x),
		DIMM_PARM(taa_ps),
		DIMM_PARM(caslat_x_minus_1),
		DIMM_PARM(caslat_x_minus_2),
		DIMM_PARM(caslat_lowest_derated),

		DIMM_PARM(trcd_ps),
		DIMM_PARM(trp_ps),
		DIMM_PARM(tras_ps),
#if defined(CONFIG_SYS_FSL_DDR4) || defined(CONFIG_SYS_FSL_DDR3)
		DIMM_PARM(tfaw_ps),
#endif
#ifdef CONFIG_SYS_FSL_DDR4
		DIMM_PARM(trfc1_ps),
		DIMM_PARM(trfc2_ps),
		DIMM_PARM(trfc4_ps),
		DIMM_PARM(trrds_ps),
		DIMM_PARM(trrdl_ps),
		DIMM_PARM(tccdl_ps),
		DIMM_PARM(trfc_slr_ps),
#else
		DIMM_PARM(twr_ps),
		DIMM_PARM(twtr_ps),
		DIMM_PARM(trfc_ps),
		DIMM_PARM(trrd_ps),
		DIMM_PARM(trtp_ps),
#endif
		DIMM_PARM(trc_ps),
		DIMM_PARM(refresh_rate_ps),

#if defined(CONFIG_SYS_FSL_DDR1) || defined(CONFIG_SYS_FSL_DDR2)
		DIMM_PARM(tis_ps),
		DIMM_PARM(tih_ps),
		DIMM_PARM(tds_ps),
		DIMM_PARM(tdh_ps),
		DIMM_PARM(tdqsq_max_ps),
		DIMM_PARM(tqhs_ps),
#endif
#ifdef CONFIG_SYS_FSL_DDR4
		DIMM_PARM_HEX(dq_mapping[0]),
		DIMM_PARM_HEX(dq_mapping[1]),
		DIMM_PARM_HEX(dq_mapping[2]),
		DIMM_PARM_HEX(dq_mapping[3]),
		DIMM_PARM_HEX(dq_mapping[4]),
		DIMM_PARM_HEX(dq_mapping[5]),
		DIMM_PARM_HEX(dq_mapping[6]),
		DIMM_PARM_HEX(dq_mapping[7]),
		DIMM_PARM_HEX(dq_mapping[8]),
		DIMM_PARM_HEX(dq_mapping[9]),
		DIMM_PARM_HEX(dq_mapping[10]),
		DIMM_PARM_HEX(dq_mapping[11]),
		DIMM_PARM_HEX(dq_mapping[12]),
		DIMM_PARM_HEX(dq_mapping[13]),
		DIMM_PARM_HEX(dq_mapping[14]),
		DIMM_PARM_HEX(dq_mapping[15]),
		DIMM_PARM_HEX(dq_mapping[16]),
		DIMM_PARM_HEX(dq_mapping[17]),
		DIMM_PARM(dq_mapping_ors),
#endif
	};
	static const unsigned int n_opts = ARRAY_SIZE(options);

	if (pdimm->n_ranks == 0) {
		printf("DIMM not present\n");
		return;
	}
	printf("DIMM organization parameters:\n");
	printf("module part name = %s\n", pdimm->mpart);
	printf("rank_density = %llu bytes (%llu megabytes)\n",
	       pdimm->rank_density, pdimm->rank_density / 0x100000);
	printf("capacity = %llu bytes (%llu megabytes)\n",
	       pdimm->capacity, pdimm->capacity / 0x100000);
	printf("burst_lengths_bitmask = %02X\n",
	       pdimm->burst_lengths_bitmask);
	printf("base_addresss = %llu (%08llX %08llX)\n",
	       pdimm->base_address,
	       (pdimm->base_address >> 32),
	       pdimm->base_address & 0xFFFFFFFF);
	print_option_table(options, n_opts, pdimm);
}

static void print_lowest_common_dimm_parameters(
		const common_timing_params_t *plcd_dimm_params)
{
	static const struct options_string options[] = {
#if defined(CONFIG_SYS_FSL_DDR3) || defined(CONFIG_SYS_FSL_DDR4)
		COMMON_TIMING(taamin_ps),
#endif
		COMMON_TIMING(trcd_ps),
		COMMON_TIMING(trp_ps),
		COMMON_TIMING(tras_ps),
#ifdef CONFIG_SYS_FSL_DDR4
		COMMON_TIMING(trfc1_ps),
		COMMON_TIMING(trfc2_ps),
		COMMON_TIMING(trfc4_ps),
		COMMON_TIMING(trrds_ps),
		COMMON_TIMING(trrdl_ps),
		COMMON_TIMING(tccdl_ps),
		COMMON_TIMING(trfc_slr_ps),
#else
		COMMON_TIMING(twtr_ps),
		COMMON_TIMING(trfc_ps),
		COMMON_TIMING(trrd_ps),
		COMMON_TIMING(trtp_ps),
#endif
		COMMON_TIMING(twr_ps),
		COMMON_TIMING(trc_ps),
		COMMON_TIMING(refresh_rate_ps),
		COMMON_TIMING(extended_op_srt),
#if defined(CONFIG_SYS_FSL_DDR1) || defined(CONFIG_SYS_FSL_DDR2)
		COMMON_TIMING(tis_ps),
		COMMON_TIMING(tih_ps),
		COMMON_TIMING(tds_ps),
		COMMON_TIMING(tdh_ps),
		COMMON_TIMING(tdqsq_max_ps),
		COMMON_TIMING(tqhs_ps),
#endif
		COMMON_TIMING(lowest_common_spd_caslat),
		COMMON_TIMING(highest_common_derated_caslat),
		COMMON_TIMING(additive_latency),
		COMMON_TIMING(ndimms_present),
		COMMON_TIMING(all_dimms_registered),
		COMMON_TIMING(all_dimms_unbuffered),
		COMMON_TIMING(all_dimms_ecc_capable),
	};
	static const unsigned int n_opts = ARRAY_SIZE(options);

	/* Clock frequencies */
	printf("tckmin_x_ps = %u (%u MHz)\n",
	       plcd_dimm_params->tckmin_x_ps,
	       picos_to_mhz(plcd_dimm_params->tckmin_x_ps));
	printf("tckmax_ps = %u (%u MHz)\n",
	       plcd_dimm_params->tckmax_ps,
	       picos_to_mhz(plcd_dimm_params->tckmax_ps));
	printf("all_dimms_burst_lengths_bitmask = %02X\n",
	       plcd_dimm_params->all_dimms_burst_lengths_bitmask);

	print_option_table(options, n_opts, plcd_dimm_params);

	printf("total_mem = %llu (%llu megabytes)\n",
	       plcd_dimm_params->total_mem,
	       plcd_dimm_params->total_mem / 0x100000);
	printf("base_address = %llu (%llu megabytes)\n",
	       plcd_dimm_params->base_address,
	       plcd_dimm_params->base_address / 0x100000);
}

#define CTRL_OPTIONS(x) {#x, offsetof(memctl_options_t, x), \
	sizeof((memctl_options_t *)0)->x, 0}
#define CTRL_OPTIONS_CS(x, y) {"cs" #x "_" #y, \
	offsetof(memctl_options_t, cs_local_opts[x].y), \
	sizeof((memctl_options_t *)0)->cs_local_opts[x].y, 0}

static void fsl_ddr_options_edit(fsl_ddr_info_t *pinfo,
			   unsigned int ctl_num,
			   const char *optname_str,
			   const char *value_str)
{
	memctl_options_t *p = &(pinfo->memctl_opts[ctl_num]);
	/*
	 * This array all on the stack and *computed* each time this
	 * function is rung.
	 */
	static const struct options_string options[] = {
		CTRL_OPTIONS_CS(0, odt_rd_cfg),
		CTRL_OPTIONS_CS(0, odt_wr_cfg),
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 1)
		CTRL_OPTIONS_CS(1, odt_rd_cfg),
		CTRL_OPTIONS_CS(1, odt_wr_cfg),
#endif
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 2)
		CTRL_OPTIONS_CS(2, odt_rd_cfg),
		CTRL_OPTIONS_CS(2, odt_wr_cfg),
#endif
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 2)
		CTRL_OPTIONS_CS(3, odt_rd_cfg),
		CTRL_OPTIONS_CS(3, odt_wr_cfg),
#endif
#if defined(CONFIG_SYS_FSL_DDR3) || defined(CONFIG_SYS_FSL_DDR4)
		CTRL_OPTIONS_CS(0, odt_rtt_norm),
		CTRL_OPTIONS_CS(0, odt_rtt_wr),
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 1)
		CTRL_OPTIONS_CS(1, odt_rtt_norm),
		CTRL_OPTIONS_CS(1, odt_rtt_wr),
#endif
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 2)
		CTRL_OPTIONS_CS(2, odt_rtt_norm),
		CTRL_OPTIONS_CS(2, odt_rtt_wr),
#endif
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 2)
		CTRL_OPTIONS_CS(3, odt_rtt_norm),
		CTRL_OPTIONS_CS(3, odt_rtt_wr),
#endif
#endif
		CTRL_OPTIONS(memctl_interleaving),
		CTRL_OPTIONS(memctl_interleaving_mode),
		CTRL_OPTIONS(ba_intlv_ctl),
		CTRL_OPTIONS(ecc_mode),
		CTRL_OPTIONS(ecc_init_using_memctl),
		CTRL_OPTIONS(dqs_config),
		CTRL_OPTIONS(self_refresh_in_sleep),
		CTRL_OPTIONS(dynamic_power),
		CTRL_OPTIONS(data_bus_width),
		CTRL_OPTIONS(burst_length),
		CTRL_OPTIONS(cas_latency_override),
		CTRL_OPTIONS(cas_latency_override_value),
		CTRL_OPTIONS(use_derated_caslat),
		CTRL_OPTIONS(additive_latency_override),
		CTRL_OPTIONS(additive_latency_override_value),
		CTRL_OPTIONS(clk_adjust),
		CTRL_OPTIONS(cpo_override),
		CTRL_OPTIONS(write_data_delay),
		CTRL_OPTIONS(half_strength_driver_enable),

		/*
		 * These can probably be changed to 2T_EN and 3T_EN
		 * (using a leading numerical character) without problem
		 */
		CTRL_OPTIONS(twot_en),
		CTRL_OPTIONS(threet_en),
		CTRL_OPTIONS(mirrored_dimm),
		CTRL_OPTIONS(ap_en),
		CTRL_OPTIONS(x4_en),
		CTRL_OPTIONS(package_3ds),
		CTRL_OPTIONS(bstopre),
		CTRL_OPTIONS(wrlvl_override),
		CTRL_OPTIONS(wrlvl_sample),
		CTRL_OPTIONS(wrlvl_start),
		CTRL_OPTIONS(cswl_override),
		CTRL_OPTIONS(rcw_override),
		CTRL_OPTIONS(rcw_1),
		CTRL_OPTIONS(rcw_2),
		CTRL_OPTIONS(rcw_3),
		CTRL_OPTIONS(ddr_cdr1),
		CTRL_OPTIONS(ddr_cdr2),
		CTRL_OPTIONS(tfaw_window_four_activates_ps),
		CTRL_OPTIONS(trwt_override),
		CTRL_OPTIONS(trwt),
		CTRL_OPTIONS(rtt_override),
		CTRL_OPTIONS(rtt_override_value),
		CTRL_OPTIONS(rtt_wr_override_value),
	};

	static const unsigned int n_opts = ARRAY_SIZE(options);

	if (handle_option_table(options, n_opts, p,
					optname_str, value_str))
		return;

	printf("couldn't find option string %s\n", optname_str);
}

#define CFG_REGS(x) {#x, offsetof(fsl_ddr_cfg_regs_t, x), \
	sizeof((fsl_ddr_cfg_regs_t *)0)->x, 1}
#define CFG_REGS_CS(x, y) {"cs" #x "_" #y, \
	offsetof(fsl_ddr_cfg_regs_t, cs[x].y), \
	sizeof((fsl_ddr_cfg_regs_t *)0)->cs[x].y, 1}

static void print_fsl_memctl_config_regs(const fsl_ddr_cfg_regs_t *ddr)
{
	unsigned int i;
	static const struct options_string options[] = {
		CFG_REGS_CS(0, bnds),
		CFG_REGS_CS(0, config),
		CFG_REGS_CS(0, config_2),
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 1)
		CFG_REGS_CS(1, bnds),
		CFG_REGS_CS(1, config),
		CFG_REGS_CS(1, config_2),
#endif
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 2)
		CFG_REGS_CS(2, bnds),
		CFG_REGS_CS(2, config),
		CFG_REGS_CS(2, config_2),
#endif
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 2)
		CFG_REGS_CS(3, bnds),
		CFG_REGS_CS(3, config),
		CFG_REGS_CS(3, config_2),
#endif
		CFG_REGS(timing_cfg_3),
		CFG_REGS(timing_cfg_0),
		CFG_REGS(timing_cfg_1),
		CFG_REGS(timing_cfg_2),
		CFG_REGS(ddr_sdram_cfg),
		CFG_REGS(ddr_sdram_cfg_2),
		CFG_REGS(ddr_sdram_cfg_3),
		CFG_REGS(ddr_sdram_mode),
		CFG_REGS(ddr_sdram_mode_2),
		CFG_REGS(ddr_sdram_mode_3),
		CFG_REGS(ddr_sdram_mode_4),
		CFG_REGS(ddr_sdram_mode_5),
		CFG_REGS(ddr_sdram_mode_6),
		CFG_REGS(ddr_sdram_mode_7),
		CFG_REGS(ddr_sdram_mode_8),
#ifdef CONFIG_SYS_FSL_DDR4
		CFG_REGS(ddr_sdram_mode_9),
		CFG_REGS(ddr_sdram_mode_10),
		CFG_REGS(ddr_sdram_mode_11),
		CFG_REGS(ddr_sdram_mode_12),
		CFG_REGS(ddr_sdram_mode_13),
		CFG_REGS(ddr_sdram_mode_14),
		CFG_REGS(ddr_sdram_mode_15),
		CFG_REGS(ddr_sdram_mode_16),
#endif
		CFG_REGS(ddr_sdram_interval),
		CFG_REGS(ddr_data_init),
		CFG_REGS(ddr_sdram_clk_cntl),
		CFG_REGS(ddr_init_addr),
		CFG_REGS(ddr_init_ext_addr),
		CFG_REGS(timing_cfg_4),
		CFG_REGS(timing_cfg_5),
#ifdef CONFIG_SYS_FSL_DDR4
		CFG_REGS(timing_cfg_6),
		CFG_REGS(timing_cfg_7),
		CFG_REGS(timing_cfg_8),
		CFG_REGS(timing_cfg_9),
#endif
		CFG_REGS(ddr_zq_cntl),
		CFG_REGS(ddr_wrlvl_cntl),
		CFG_REGS(ddr_wrlvl_cntl_2),
		CFG_REGS(ddr_wrlvl_cntl_3),
		CFG_REGS(ddr_sr_cntr),
		CFG_REGS(ddr_sdram_rcw_1),
		CFG_REGS(ddr_sdram_rcw_2),
		CFG_REGS(ddr_sdram_rcw_3),
		CFG_REGS(ddr_cdr1),
		CFG_REGS(ddr_cdr2),
		CFG_REGS(dq_map_0),
		CFG_REGS(dq_map_1),
		CFG_REGS(dq_map_2),
		CFG_REGS(dq_map_3),
		CFG_REGS(err_disable),
		CFG_REGS(err_int_en),
		CFG_REGS(ddr_eor),
	};
	static const unsigned int n_opts = ARRAY_SIZE(options);

	print_option_table(options, n_opts, ddr);

	for (i = 0; i < 64; i++)
		printf("debug_%02d = 0x%08X\n", i+1, ddr->debug[i]);
}

static void fsl_ddr_regs_edit(fsl_ddr_info_t *pinfo,
			unsigned int ctrl_num,
			const char *regname,
			const char *value_str)
{
	unsigned int i;
	fsl_ddr_cfg_regs_t *ddr;
	char buf[20];
	static const struct options_string options[] = {
		CFG_REGS_CS(0, bnds),
		CFG_REGS_CS(0, config),
		CFG_REGS_CS(0, config_2),
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 1)
		CFG_REGS_CS(1, bnds),
		CFG_REGS_CS(1, config),
		CFG_REGS_CS(1, config_2),
#endif
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 2)
		CFG_REGS_CS(2, bnds),
		CFG_REGS_CS(2, config),
		CFG_REGS_CS(2, config_2),
#endif
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 3)
		CFG_REGS_CS(3, bnds),
		CFG_REGS_CS(3, config),
		CFG_REGS_CS(3, config_2),
#endif
		CFG_REGS(timing_cfg_3),
		CFG_REGS(timing_cfg_0),
		CFG_REGS(timing_cfg_1),
		CFG_REGS(timing_cfg_2),
		CFG_REGS(ddr_sdram_cfg),
		CFG_REGS(ddr_sdram_cfg_2),
		CFG_REGS(ddr_sdram_cfg_3),
		CFG_REGS(ddr_sdram_mode),
		CFG_REGS(ddr_sdram_mode_2),
		CFG_REGS(ddr_sdram_mode_3),
		CFG_REGS(ddr_sdram_mode_4),
		CFG_REGS(ddr_sdram_mode_5),
		CFG_REGS(ddr_sdram_mode_6),
		CFG_REGS(ddr_sdram_mode_7),
		CFG_REGS(ddr_sdram_mode_8),
#ifdef CONFIG_SYS_FSL_DDR4
		CFG_REGS(ddr_sdram_mode_9),
		CFG_REGS(ddr_sdram_mode_10),
		CFG_REGS(ddr_sdram_mode_11),
		CFG_REGS(ddr_sdram_mode_12),
		CFG_REGS(ddr_sdram_mode_13),
		CFG_REGS(ddr_sdram_mode_14),
		CFG_REGS(ddr_sdram_mode_15),
		CFG_REGS(ddr_sdram_mode_16),
#endif
		CFG_REGS(ddr_sdram_interval),
		CFG_REGS(ddr_data_init),
		CFG_REGS(ddr_sdram_clk_cntl),
		CFG_REGS(ddr_init_addr),
		CFG_REGS(ddr_init_ext_addr),
		CFG_REGS(timing_cfg_4),
		CFG_REGS(timing_cfg_5),
#ifdef CONFIG_SYS_FSL_DDR4
		CFG_REGS(timing_cfg_6),
		CFG_REGS(timing_cfg_7),
		CFG_REGS(timing_cfg_8),
		CFG_REGS(timing_cfg_9),
#endif
		CFG_REGS(ddr_zq_cntl),
		CFG_REGS(ddr_wrlvl_cntl),
		CFG_REGS(ddr_wrlvl_cntl_2),
		CFG_REGS(ddr_wrlvl_cntl_3),
		CFG_REGS(ddr_sr_cntr),
		CFG_REGS(ddr_sdram_rcw_1),
		CFG_REGS(ddr_sdram_rcw_2),
		CFG_REGS(ddr_sdram_rcw_3),
		CFG_REGS(ddr_cdr1),
		CFG_REGS(ddr_cdr2),
		CFG_REGS(dq_map_0),
		CFG_REGS(dq_map_1),
		CFG_REGS(dq_map_2),
		CFG_REGS(dq_map_3),
		CFG_REGS(err_disable),
		CFG_REGS(err_int_en),
		CFG_REGS(ddr_sdram_rcw_2),
		CFG_REGS(ddr_sdram_rcw_2),
		CFG_REGS(ddr_eor),
	};
	static const unsigned int n_opts = ARRAY_SIZE(options);

	debug("fsl_ddr_regs_edit: ctrl_num = %u, "
		"regname = %s, value = %s\n",
		ctrl_num, regname, value_str);
	if (ctrl_num > CONFIG_SYS_NUM_DDR_CTLRS)
		return;

	ddr = &(pinfo->fsl_ddr_config_reg[ctrl_num]);

	if (handle_option_table(options, n_opts, ddr, regname, value_str))
		return;

	for (i = 0; i < 64; i++) {
		unsigned int value = simple_strtoul(value_str, NULL, 0);
		sprintf(buf, "debug_%u", i + 1);
		if (strcmp(buf, regname) == 0) {
			ddr->debug[i] = value;
			return;
		}
	}
	printf("Error: couldn't find register string %s\n", regname);
}

#define CTRL_OPTIONS_HEX(x) {#x, offsetof(memctl_options_t, x), \
	sizeof((memctl_options_t *)0)->x, 1}

static void print_memctl_options(const memctl_options_t *popts)
{
	static const struct options_string options[] = {
		CTRL_OPTIONS_CS(0, odt_rd_cfg),
		CTRL_OPTIONS_CS(0, odt_wr_cfg),
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 1)
		CTRL_OPTIONS_CS(1, odt_rd_cfg),
		CTRL_OPTIONS_CS(1, odt_wr_cfg),
#endif
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 2)
		CTRL_OPTIONS_CS(2, odt_rd_cfg),
		CTRL_OPTIONS_CS(2, odt_wr_cfg),
#endif
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 3)
		CTRL_OPTIONS_CS(3, odt_rd_cfg),
		CTRL_OPTIONS_CS(3, odt_wr_cfg),
#endif
#if defined(CONFIG_SYS_FSL_DDR3) || defined(CONFIG_SYS_FSL_DDR4)
		CTRL_OPTIONS_CS(0, odt_rtt_norm),
		CTRL_OPTIONS_CS(0, odt_rtt_wr),
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 1)
		CTRL_OPTIONS_CS(1, odt_rtt_norm),
		CTRL_OPTIONS_CS(1, odt_rtt_wr),
#endif
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 2)
		CTRL_OPTIONS_CS(2, odt_rtt_norm),
		CTRL_OPTIONS_CS(2, odt_rtt_wr),
#endif
#if (CONFIG_CHIP_SELECTS_PER_CTRL > 3)
		CTRL_OPTIONS_CS(3, odt_rtt_norm),
		CTRL_OPTIONS_CS(3, odt_rtt_wr),
#endif
#endif
		CTRL_OPTIONS(memctl_interleaving),
		CTRL_OPTIONS(memctl_interleaving_mode),
		CTRL_OPTIONS_HEX(ba_intlv_ctl),
		CTRL_OPTIONS(ecc_mode),
		CTRL_OPTIONS(ecc_init_using_memctl),
		CTRL_OPTIONS(dqs_config),
		CTRL_OPTIONS(self_refresh_in_sleep),
		CTRL_OPTIONS(dynamic_power),
		CTRL_OPTIONS(data_bus_width),
		CTRL_OPTIONS(burst_length),
		CTRL_OPTIONS(cas_latency_override),
		CTRL_OPTIONS(cas_latency_override_value),
		CTRL_OPTIONS(use_derated_caslat),
		CTRL_OPTIONS(additive_latency_override),
		CTRL_OPTIONS(additive_latency_override_value),
		CTRL_OPTIONS(clk_adjust),
		CTRL_OPTIONS(cpo_override),
		CTRL_OPTIONS(write_data_delay),
		CTRL_OPTIONS(half_strength_driver_enable),
		/*
		 * These can probably be changed to 2T_EN and 3T_EN
		 * (using a leading numerical character) without problem
		 */
		CTRL_OPTIONS(twot_en),
		CTRL_OPTIONS(threet_en),
		CTRL_OPTIONS(registered_dimm_en),
		CTRL_OPTIONS(mirrored_dimm),
		CTRL_OPTIONS(ap_en),
		CTRL_OPTIONS(x4_en),
		CTRL_OPTIONS(package_3ds),
		CTRL_OPTIONS(bstopre),
		CTRL_OPTIONS(wrlvl_override),
		CTRL_OPTIONS(wrlvl_sample),
		CTRL_OPTIONS(wrlvl_start),
		CTRL_OPTIONS_HEX(cswl_override),
		CTRL_OPTIONS(rcw_override),
		CTRL_OPTIONS_HEX(rcw_1),
		CTRL_OPTIONS_HEX(rcw_2),
		CTRL_OPTIONS_HEX(rcw_3),
		CTRL_OPTIONS_HEX(ddr_cdr1),
		CTRL_OPTIONS_HEX(ddr_cdr2),
		CTRL_OPTIONS(tfaw_window_four_activates_ps),
		CTRL_OPTIONS(trwt_override),
		CTRL_OPTIONS(trwt),
		CTRL_OPTIONS(rtt_override),
		CTRL_OPTIONS(rtt_override_value),
		CTRL_OPTIONS(rtt_wr_override_value),
	};
	static const unsigned int n_opts = ARRAY_SIZE(options);

	print_option_table(options, n_opts, popts);
}

#ifdef CONFIG_SYS_FSL_DDR1
void ddr1_spd_dump(const ddr1_spd_eeprom_t *spd)
{
	unsigned int i;

	printf("%-3d    : %02x %s\n", 0, spd->info_size,
	       " spd->info_size,   *  0 # bytes written into serial memory *");
	printf("%-3d    : %02x %s\n", 1, spd->chip_size,
	       " spd->chip_size,   *  1 Total # bytes of SPD memory device *");
	printf("%-3d    : %02x %s\n", 2, spd->mem_type,
	       " spd->mem_type,    *  2 Fundamental memory type *");
	printf("%-3d    : %02x %s\n", 3, spd->nrow_addr,
	       " spd->nrow_addr,   *  3 # of Row Addresses on this assembly *");
	printf("%-3d    : %02x %s\n", 4, spd->ncol_addr,
	       " spd->ncol_addr,   *  4 # of Column Addrs on this assembly *");
	printf("%-3d    : %02x %s\n", 5, spd->nrows,
	       " spd->nrows        *  5 # of DIMM Banks *");
	printf("%-3d    : %02x %s\n", 6, spd->dataw_lsb,
	       " spd->dataw_lsb,   *  6 Data Width lsb of this assembly *");
	printf("%-3d    : %02x %s\n", 7, spd->dataw_msb,
	       " spd->dataw_msb,   *  7 Data Width msb of this assembly *");
	printf("%-3d    : %02x %s\n", 8, spd->voltage,
	       " spd->voltage,     *  8 Voltage intf std of this assembly *");
	printf("%-3d    : %02x %s\n", 9, spd->clk_cycle,
	       " spd->clk_cycle,   *  9 SDRAM Cycle time at CL=X *");
	printf("%-3d    : %02x %s\n", 10, spd->clk_access,
	       " spd->clk_access,  * 10 SDRAM Access from Clock at CL=X *");
	printf("%-3d    : %02x %s\n", 11, spd->config,
	       " spd->config,      * 11 DIMM Configuration type *");
	printf("%-3d    : %02x %s\n", 12, spd->refresh,
	       " spd->refresh,     * 12 Refresh Rate/Type *");
	printf("%-3d    : %02x %s\n", 13, spd->primw,
	       " spd->primw,       * 13 Primary SDRAM Width *");
	printf("%-3d    : %02x %s\n", 14, spd->ecw,
	       " spd->ecw,         * 14 Error Checking SDRAM width *");
	printf("%-3d    : %02x %s\n", 15, spd->min_delay,
	       " spd->min_delay,   * 15 Back to Back Random Access *");
	printf("%-3d    : %02x %s\n", 16, spd->burstl,
	       " spd->burstl,      * 16 Burst Lengths Supported *");
	printf("%-3d    : %02x %s\n", 17, spd->nbanks,
	       " spd->nbanks,      * 17 # of Banks on Each SDRAM Device *");
	printf("%-3d    : %02x %s\n", 18, spd->cas_lat,
	       " spd->cas_lat,     * 18 CAS# Latencies Supported *");
	printf("%-3d    : %02x %s\n", 19, spd->cs_lat,
	       " spd->cs_lat,      * 19 Chip Select Latency *");
	printf("%-3d    : %02x %s\n", 20, spd->write_lat,
	       " spd->write_lat,   * 20 Write Latency/Recovery *");
	printf("%-3d    : %02x %s\n", 21, spd->mod_attr,
	       " spd->mod_attr,    * 21 SDRAM Module Attributes *");
	printf("%-3d    : %02x %s\n", 22, spd->dev_attr,
	       " spd->dev_attr,    * 22 SDRAM Device Attributes *");
	printf("%-3d    : %02x %s\n", 23, spd->clk_cycle2,
	       " spd->clk_cycle2,  * 23 Min SDRAM Cycle time at CL=X-1 *");
	printf("%-3d    : %02x %s\n", 24, spd->clk_access2,
	       " spd->clk_access2, * 24 SDRAM Access from Clock at CL=X-1 *");
	printf("%-3d    : %02x %s\n", 25, spd->clk_cycle3,
	       " spd->clk_cycle3,  * 25 Min SDRAM Cycle time at CL=X-2 *");
	printf("%-3d    : %02x %s\n", 26, spd->clk_access3,
	       " spd->clk_access3, * 26 Max Access from Clock at CL=X-2 *");
	printf("%-3d    : %02x %s\n", 27, spd->trp,
	       " spd->trp,         * 27 Min Row Precharge Time (tRP)*");
	printf("%-3d    : %02x %s\n", 28, spd->trrd,
	       " spd->trrd,        * 28 Min Row Active to Row Active (tRRD) *");
	printf("%-3d    : %02x %s\n", 29, spd->trcd,
	       " spd->trcd,        * 29 Min RAS to CAS Delay (tRCD) *");
	printf("%-3d    : %02x %s\n", 30, spd->tras,
	       " spd->tras,        * 30 Minimum RAS Pulse Width (tRAS) *");
	printf("%-3d    : %02x %s\n", 31, spd->bank_dens,
	       " spd->bank_dens,   * 31 Density of each bank on module *");
	printf("%-3d    : %02x %s\n", 32, spd->ca_setup,
	       " spd->ca_setup,    * 32 Cmd + Addr signal input setup time *");
	printf("%-3d    : %02x %s\n", 33, spd->ca_hold,
	       " spd->ca_hold,     * 33 Cmd and Addr signal input hold time *");
	printf("%-3d    : %02x %s\n", 34, spd->data_setup,
	       " spd->data_setup,  * 34 Data signal input setup time *");
	printf("%-3d    : %02x %s\n", 35, spd->data_hold,
	       " spd->data_hold,   * 35 Data signal input hold time *");
	printf("%-3d    : %02x %s\n", 36, spd->res_36_40[0],
	       " spd->res_36_40[0], * 36 Reserved / tWR *");
	printf("%-3d    : %02x %s\n", 37, spd->res_36_40[1],
	       " spd->res_36_40[1], * 37 Reserved / tWTR *");
	printf("%-3d    : %02x %s\n", 38, spd->res_36_40[2],
	       " spd->res_36_40[2], * 38 Reserved / tRTP *");
	printf("%-3d    : %02x %s\n", 39, spd->res_36_40[3],
	       " spd->res_36_40[3], * 39 Reserved / mem_probe *");
	printf("%-3d    : %02x %s\n", 40, spd->res_36_40[4],
	       " spd->res_36_40[4], * 40 Reserved / trc,trfc extensions *");
	printf("%-3d    : %02x %s\n", 41, spd->trc,
	       " spd->trc,         * 41 Min Active to Auto refresh time tRC *");
	printf("%-3d    : %02x %s\n", 42, spd->trfc,
	       " spd->trfc,        * 42 Min Auto to Active period tRFC *");
	printf("%-3d    : %02x %s\n", 43, spd->tckmax,
	       " spd->tckmax,      * 43 Max device cycle time tCKmax *");
	printf("%-3d    : %02x %s\n", 44, spd->tdqsq,
	       " spd->tdqsq,       * 44 Max DQS to DQ skew *");
	printf("%-3d    : %02x %s\n", 45, spd->tqhs,
	       " spd->tqhs,        * 45 Max Read DataHold skew tQHS *");
	printf("%-3d    : %02x %s\n", 46, spd->res_46,
	       " spd->res_46,  * 46 Reserved/ PLL Relock time *");
	printf("%-3d    : %02x %s\n", 47, spd->dimm_height,
	       " spd->dimm_height  * 47 SDRAM DIMM Height *");

	printf("%-3d-%3d: ",  48, 61);

	for (i = 0; i < 14; i++)
		printf("%02x", spd->res_48_61[i]);

	printf(" * 48-61 IDD in SPD and Reserved space *\n");

	printf("%-3d    : %02x %s\n", 62, spd->spd_rev,
	       " spd->spd_rev,     * 62 SPD Data Revision Code *");
	printf("%-3d    : %02x %s\n", 63, spd->cksum,
	       " spd->cksum,       * 63 Checksum for bytes 0-62 *");
	printf("%-3d-%3d: ",  64, 71);

	for (i = 0; i < 8; i++)
		printf("%02x", spd->mid[i]);

	printf("* 64 Mfr's JEDEC ID code per JEP-108E *\n");
	printf("%-3d    : %02x %s\n", 72, spd->mloc,
	       " spd->mloc,        * 72 Manufacturing Location *");

	printf("%-3d-%3d: >>",  73, 90);

	for (i = 0; i < 18; i++)
		printf("%c", spd->mpart[i]);

	printf("<<* 73 Manufacturer's Part Number *\n");

	printf("%-3d-%3d: %02x %02x %s\n", 91, 92, spd->rev[0], spd->rev[1],
	       "* 91 Revision Code *");
	printf("%-3d-%3d: %02x %02x %s\n", 93, 94, spd->mdate[0], spd->mdate[1],
	       "* 93 Manufacturing Date *");
	printf("%-3d-%3d: ", 95, 98);

	for (i = 0; i < 4; i++)
		printf("%02x", spd->sernum[i]);

	printf("* 95 Assembly Serial Number *\n");

	printf("%-3d-%3d: ", 99, 127);

	for (i = 0; i < 27; i++)
		printf("%02x", spd->mspec[i]);

	printf("* 99 Manufacturer Specific Data *\n");
}
#endif

#ifdef CONFIG_SYS_FSL_DDR2
void ddr2_spd_dump(const ddr2_spd_eeprom_t *spd)
{
	unsigned int i;

	printf("%-3d    : %02x %s\n", 0, spd->info_size,
	       " spd->info_size,   *  0 # bytes written into serial memory *");
	printf("%-3d    : %02x %s\n", 1, spd->chip_size,
	       " spd->chip_size,   *  1 Total # bytes of SPD memory device *");
	printf("%-3d    : %02x %s\n", 2, spd->mem_type,
	       " spd->mem_type,    *  2 Fundamental memory type *");
	printf("%-3d    : %02x %s\n", 3, spd->nrow_addr,
	       " spd->nrow_addr,   *  3 # of Row Addresses on this assembly *");
	printf("%-3d    : %02x %s\n", 4, spd->ncol_addr,
	       " spd->ncol_addr,   *  4 # of Column Addrs on this assembly *");
	printf("%-3d    : %02x %s\n", 5, spd->mod_ranks,
	       " spd->mod_ranks    *  5 # of Module Rows on this assembly *");
	printf("%-3d    : %02x %s\n", 6, spd->dataw,
	       " spd->dataw,       *  6 Data Width of this assembly *");
	printf("%-3d    : %02x %s\n", 7, spd->res_7,
	       " spd->res_7,       *  7 Reserved *");
	printf("%-3d    : %02x %s\n", 8, spd->voltage,
	       " spd->voltage,     *  8 Voltage intf std of this assembly *");
	printf("%-3d    : %02x %s\n", 9, spd->clk_cycle,
	       " spd->clk_cycle,   *  9 SDRAM Cycle time at CL=X *");
	printf("%-3d    : %02x %s\n", 10, spd->clk_access,
	       " spd->clk_access,  * 10 SDRAM Access from Clock at CL=X *");
	printf("%-3d    : %02x %s\n", 11, spd->config,
	       " spd->config,      * 11 DIMM Configuration type *");
	printf("%-3d    : %02x %s\n", 12, spd->refresh,
	       " spd->refresh,     * 12 Refresh Rate/Type *");
	printf("%-3d    : %02x %s\n", 13, spd->primw,
	       " spd->primw,       * 13 Primary SDRAM Width *");
	printf("%-3d    : %02x %s\n", 14, spd->ecw,
	       " spd->ecw,         * 14 Error Checking SDRAM width *");
	printf("%-3d    : %02x %s\n", 15, spd->res_15,
	       " spd->res_15,      * 15 Reserved *");
	printf("%-3d    : %02x %s\n", 16, spd->burstl,
	       " spd->burstl,      * 16 Burst Lengths Supported *");
	printf("%-3d    : %02x %s\n", 17, spd->nbanks,
	       " spd->nbanks,      * 17 # of Banks on Each SDRAM Device *");
	printf("%-3d    : %02x %s\n", 18, spd->cas_lat,
	       " spd->cas_lat,     * 18 CAS# Latencies Supported *");
	printf("%-3d    : %02x %s\n", 19, spd->mech_char,
	       " spd->mech_char,   * 19 Mechanical Characteristics *");
	printf("%-3d    : %02x %s\n", 20, spd->dimm_type,
	       " spd->dimm_type,   * 20 DIMM type *");
	printf("%-3d    : %02x %s\n", 21, spd->mod_attr,
	       " spd->mod_attr,    * 21 SDRAM Module Attributes *");
	printf("%-3d    : %02x %s\n", 22, spd->dev_attr,
	       " spd->dev_attr,    * 22 SDRAM Device Attributes *");
	printf("%-3d    : %02x %s\n", 23, spd->clk_cycle2,
	       " spd->clk_cycle2,  * 23 Min SDRAM Cycle time at CL=X-1 *");
	printf("%-3d    : %02x %s\n", 24, spd->clk_access2,
	       " spd->clk_access2, * 24 SDRAM Access from Clock at CL=X-1 *");
	printf("%-3d    : %02x %s\n", 25, spd->clk_cycle3,
	       " spd->clk_cycle3,  * 25 Min SDRAM Cycle time at CL=X-2 *");
	printf("%-3d    : %02x %s\n", 26, spd->clk_access3,
	       " spd->clk_access3, * 26 Max Access from Clock at CL=X-2 *");
	printf("%-3d    : %02x %s\n", 27, spd->trp,
	       " spd->trp,         * 27 Min Row Precharge Time (tRP)*");
	printf("%-3d    : %02x %s\n", 28, spd->trrd,
	       " spd->trrd,        * 28 Min Row Active to Row Active (tRRD) *");
	printf("%-3d    : %02x %s\n", 29, spd->trcd,
	       " spd->trcd,        * 29 Min RAS to CAS Delay (tRCD) *");
	printf("%-3d    : %02x %s\n", 30, spd->tras,
	       " spd->tras,        * 30 Minimum RAS Pulse Width (tRAS) *");
	printf("%-3d    : %02x %s\n", 31, spd->rank_dens,
	       " spd->rank_dens,   * 31 Density of each rank on module *");
	printf("%-3d    : %02x %s\n", 32, spd->ca_setup,
	       " spd->ca_setup,    * 32 Cmd + Addr signal input setup time *");
	printf("%-3d    : %02x %s\n", 33, spd->ca_hold,
	       " spd->ca_hold,     * 33 Cmd and Addr signal input hold time *");
	printf("%-3d    : %02x %s\n", 34, spd->data_setup,
	       " spd->data_setup,  * 34 Data signal input setup time *");
	printf("%-3d    : %02x %s\n", 35, spd->data_hold,
	       " spd->data_hold,   * 35 Data signal input hold time *");
	printf("%-3d    : %02x %s\n", 36, spd->twr,
	       " spd->twr,         * 36 Write Recovery time tWR *");
	printf("%-3d    : %02x %s\n", 37, spd->twtr,
	       " spd->twtr,        * 37 Int write to read delay tWTR *");
	printf("%-3d    : %02x %s\n", 38, spd->trtp,
	       " spd->trtp,        * 38 Int read to precharge delay tRTP *");
	printf("%-3d    : %02x %s\n", 39, spd->mem_probe,
	       " spd->mem_probe,   * 39 Mem analysis probe characteristics *");
	printf("%-3d    : %02x %s\n", 40, spd->trctrfc_ext,
	       " spd->trctrfc_ext, * 40 Extensions to trc and trfc *");
	printf("%-3d    : %02x %s\n", 41, spd->trc,
	       " spd->trc,         * 41 Min Active to Auto refresh time tRC *");
	printf("%-3d    : %02x %s\n", 42, spd->trfc,
	       " spd->trfc,        * 42 Min Auto to Active period tRFC *");
	printf("%-3d    : %02x %s\n", 43, spd->tckmax,
	       " spd->tckmax,      * 43 Max device cycle time tCKmax *");
	printf("%-3d    : %02x %s\n", 44, spd->tdqsq,
	       " spd->tdqsq,       * 44 Max DQS to DQ skew *");
	printf("%-3d    : %02x %s\n", 45, spd->tqhs,
	       " spd->tqhs,        * 45 Max Read DataHold skew tQHS *");
	printf("%-3d    : %02x %s\n", 46, spd->pll_relock,
	       " spd->pll_relock,  * 46 PLL Relock time *");
	printf("%-3d    : %02x %s\n", 47, spd->t_casemax,
	       " spd->t_casemax,    * 47 t_casemax *");
	printf("%-3d    : %02x %s\n", 48, spd->psi_ta_dram,
	       " spd->psi_ta_dram,   * 48 Thermal Resistance of DRAM Package "
	       "from Top (Case) to Ambient (Psi T-A DRAM) *");
	printf("%-3d    : %02x %s\n", 49, spd->dt0_mode,
	       " spd->dt0_mode,    * 49 DRAM Case Temperature Rise from "
	       "Ambient due to Activate-Precharge/Mode Bits "
	       "(DT0/Mode Bits) *)");
	printf("%-3d    : %02x %s\n", 50, spd->dt2n_dt2q,
	       " spd->dt2n_dt2q,   * 50 DRAM Case Temperature Rise from "
	       "Ambient due to Precharge/Quiet Standby "
	       "(DT2N/DT2Q) *");
	printf("%-3d    : %02x %s\n", 51, spd->dt2p,
	       " spd->dt2p,        * 51 DRAM Case Temperature Rise from "
	       "Ambient due to Precharge Power-Down (DT2P) *");
	printf("%-3d    : %02x %s\n", 52, spd->dt3n,
	       " spd->dt3n,        * 52 DRAM Case Temperature Rise from "
	       "Ambient due to Active Standby (DT3N) *");
	printf("%-3d    : %02x %s\n", 53, spd->dt3pfast,
	       " spd->dt3pfast,    * 53 DRAM Case Temperature Rise from "
	       "Ambient due to Active Power-Down with Fast PDN Exit "
	       "(DT3Pfast) *");
	printf("%-3d    : %02x %s\n", 54, spd->dt3pslow,
	       " spd->dt3pslow,    * 54 DRAM Case Temperature Rise from "
	       "Ambient due to Active Power-Down with Slow PDN Exit "
	       "(DT3Pslow) *");
	printf("%-3d    : %02x %s\n", 55, spd->dt4r_dt4r4w,
	       " spd->dt4r_dt4r4w, * 55 DRAM Case Temperature Rise from "
	       "Ambient due to Page Open Burst Read/DT4R4W Mode Bit "
	       "(DT4R/DT4R4W Mode Bit) *");
	printf("%-3d    : %02x %s\n", 56, spd->dt5b,
	       " spd->dt5b,        * 56 DRAM Case Temperature Rise from "
	       "Ambient due to Burst Refresh (DT5B) *");
	printf("%-3d    : %02x %s\n", 57, spd->dt7,
	       " spd->dt7,         * 57 DRAM Case Temperature Rise from "
	       "Ambient due to Bank Interleave Reads with "
	       "Auto-Precharge (DT7) *");
	printf("%-3d    : %02x %s\n", 58, spd->psi_ta_pll,
	       " spd->psi_ta_pll,    * 58 Thermal Resistance of PLL Package form"
	       " Top (Case) to Ambient (Psi T-A PLL) *");
	printf("%-3d    : %02x %s\n", 59, spd->psi_ta_reg,
	       " spd->psi_ta_reg,    * 59 Thermal Reisitance of Register Package"
	       " from Top (Case) to Ambient (Psi T-A Register) *");
	printf("%-3d    : %02x %s\n", 60, spd->dtpllactive,
	       " spd->dtpllactive, * 60 PLL Case Temperature Rise from "
	       "Ambient due to PLL Active (DT PLL Active) *");
	printf("%-3d    : %02x %s\n", 61, spd->dtregact,
	       " spd->dtregact,    "
	       "* 61 Register Case Temperature Rise from Ambient due to "
	       "Register Active/Mode Bit (DT Register Active/Mode Bit) *");
	printf("%-3d    : %02x %s\n", 62, spd->spd_rev,
	       " spd->spd_rev,     * 62 SPD Data Revision Code *");
	printf("%-3d    : %02x %s\n", 63, spd->cksum,
	       " spd->cksum,       * 63 Checksum for bytes 0-62 *");

	printf("%-3d-%3d: ",  64, 71);

	for (i = 0; i < 8; i++)
		printf("%02x", spd->mid[i]);

	printf("* 64 Mfr's JEDEC ID code per JEP-108E *\n");

	printf("%-3d    : %02x %s\n", 72, spd->mloc,
	       " spd->mloc,        * 72 Manufacturing Location *");

	printf("%-3d-%3d: >>",  73, 90);
	for (i = 0; i < 18; i++)
		printf("%c", spd->mpart[i]);


	printf("<<* 73 Manufacturer's Part Number *\n");

	printf("%-3d-%3d: %02x %02x %s\n", 91, 92, spd->rev[0], spd->rev[1],
	       "* 91 Revision Code *");
	printf("%-3d-%3d: %02x %02x %s\n", 93, 94, spd->mdate[0], spd->mdate[1],
	       "* 93 Manufacturing Date *");
	printf("%-3d-%3d: ", 95, 98);

	for (i = 0; i < 4; i++)
		printf("%02x", spd->sernum[i]);

	printf("* 95 Assembly Serial Number *\n");

	printf("%-3d-%3d: ", 99, 127);
	for (i = 0; i < 27; i++)
		printf("%02x", spd->mspec[i]);


	printf("* 99 Manufacturer Specific Data *\n");
}
#endif

#ifdef CONFIG_SYS_FSL_DDR3
void ddr3_spd_dump(const ddr3_spd_eeprom_t *spd)
{
	unsigned int i;

	/* General Section: Bytes 0-59 */

#define PRINT_NXS(x, y, z...) printf("%-3d    : %02x " z "\n", x, (u8)y);
#define PRINT_NNXXS(n0, n1, x0, x1, s) \
	printf("%-3d-%3d: %02x %02x " s "\n", n0, n1, x0, x1);

	PRINT_NXS(0, spd->info_size_crc,
		"info_size_crc  bytes written into serial memory, "
		"CRC coverage");
	PRINT_NXS(1, spd->spd_rev,
		"spd_rev        SPD Revision");
	PRINT_NXS(2, spd->mem_type,
		"mem_type       Key Byte / DRAM Device Type");
	PRINT_NXS(3, spd->module_type,
		"module_type    Key Byte / Module Type");
	PRINT_NXS(4, spd->density_banks,
		"density_banks  SDRAM Density and Banks");
	PRINT_NXS(5, spd->addressing,
		"addressing     SDRAM Addressing");
	PRINT_NXS(6, spd->module_vdd,
		"module_vdd     Module Nominal Voltage, VDD");
	PRINT_NXS(7, spd->organization,
		"organization   Module Organization");
	PRINT_NXS(8, spd->bus_width,
		"bus_width      Module Memory Bus Width");
	PRINT_NXS(9, spd->ftb_div,
		"ftb_div        Fine Timebase (FTB) Dividend / Divisor");
	PRINT_NXS(10, spd->mtb_dividend,
		"mtb_dividend   Medium Timebase (MTB) Dividend");
	PRINT_NXS(11, spd->mtb_divisor,
		"mtb_divisor    Medium Timebase (MTB) Divisor");
	PRINT_NXS(12, spd->tck_min,
		  "tck_min        SDRAM Minimum Cycle Time");
	PRINT_NXS(13, spd->res_13,
		"res_13         Reserved");
	PRINT_NXS(14, spd->caslat_lsb,
		"caslat_lsb     CAS Latencies Supported, LSB");
	PRINT_NXS(15, spd->caslat_msb,
		"caslat_msb     CAS Latencies Supported, MSB");
	PRINT_NXS(16, spd->taa_min,
		  "taa_min        Min CAS Latency Time");
	PRINT_NXS(17, spd->twr_min,
		  "twr_min        Min Write REcovery Time");
	PRINT_NXS(18, spd->trcd_min,
		  "trcd_min       Min RAS# to CAS# Delay Time");
	PRINT_NXS(19, spd->trrd_min,
		  "trrd_min       Min Row Active to Row Active Delay Time");
	PRINT_NXS(20, spd->trp_min,
		  "trp_min        Min Row Precharge Delay Time");
	PRINT_NXS(21, spd->tras_trc_ext,
		  "tras_trc_ext   Upper Nibbles for tRAS and tRC");
	PRINT_NXS(22, spd->tras_min_lsb,
		  "tras_min_lsb   Min Active to Precharge Delay Time, LSB");
	PRINT_NXS(23, spd->trc_min_lsb,
		  "trc_min_lsb Min Active to Active/Refresh Delay Time, LSB");
	PRINT_NXS(24, spd->trfc_min_lsb,
		  "trfc_min_lsb   Min Refresh Recovery Delay Time LSB");
	PRINT_NXS(25, spd->trfc_min_msb,
		  "trfc_min_msb   Min Refresh Recovery Delay Time MSB");
	PRINT_NXS(26, spd->twtr_min,
		  "twtr_min Min Internal Write to Read Command Delay Time");
	PRINT_NXS(27, spd->trtp_min,
		  "trtp_min "
		  "Min Internal Read to Precharge Command Delay Time");
	PRINT_NXS(28, spd->tfaw_msb,
		  "tfaw_msb       Upper Nibble for tFAW");
	PRINT_NXS(29, spd->tfaw_min,
		  "tfaw_min       Min Four Activate Window Delay Time");
	PRINT_NXS(30, spd->opt_features,
		"opt_features   SDRAM Optional Features");
	PRINT_NXS(31, spd->therm_ref_opt,
		"therm_ref_opt  SDRAM Thermal and Refresh Opts");
	PRINT_NXS(32, spd->therm_sensor,
		"therm_sensor  SDRAM Thermal Sensor");
	PRINT_NXS(33, spd->device_type,
		"device_type  SDRAM Device Type");
	PRINT_NXS(34, spd->fine_tck_min,
		  "fine_tck_min  Fine offset for tCKmin");
	PRINT_NXS(35, spd->fine_taa_min,
		  "fine_taa_min  Fine offset for tAAmin");
	PRINT_NXS(36, spd->fine_trcd_min,
		  "fine_trcd_min Fine offset for tRCDmin");
	PRINT_NXS(37, spd->fine_trp_min,
		  "fine_trp_min  Fine offset for tRPmin");
	PRINT_NXS(38, spd->fine_trc_min,
		  "fine_trc_min  Fine offset for tRCmin");

	printf("%-3d-%3d: ",  39, 59);  /* Reserved, General Section */

	for (i = 39; i <= 59; i++)
		printf("%02x ", spd->res_39_59[i - 39]);

	puts("\n");

	switch (spd->module_type) {
	case 0x02:  /* UDIMM */
	case 0x03:  /* SO-DIMM */
	case 0x04:  /* Micro-DIMM */
	case 0x06:  /* Mini-UDIMM */
		PRINT_NXS(60, spd->mod_section.unbuffered.mod_height,
			"mod_height    (Unbuffered) Module Nominal Height");
		PRINT_NXS(61, spd->mod_section.unbuffered.mod_thickness,
			"mod_thickness (Unbuffered) Module Maximum Thickness");
		PRINT_NXS(62, spd->mod_section.unbuffered.ref_raw_card,
			"ref_raw_card  (Unbuffered) Reference Raw Card Used");
		PRINT_NXS(63, spd->mod_section.unbuffered.addr_mapping,
			"addr_mapping  (Unbuffered) Address mapping from "
			"Edge Connector to DRAM");
		break;
	case 0x01:  /* RDIMM */
	case 0x05:  /* Mini-RDIMM */
		PRINT_NXS(60, spd->mod_section.registered.mod_height,
			"mod_height    (Registered) Module Nominal Height");
		PRINT_NXS(61, spd->mod_section.registered.mod_thickness,
			"mod_thickness (Registered) Module Maximum Thickness");
		PRINT_NXS(62, spd->mod_section.registered.ref_raw_card,
			"ref_raw_card  (Registered) Reference Raw Card Used");
		PRINT_NXS(63, spd->mod_section.registered.modu_attr,
			"modu_attr     (Registered) DIMM Module Attributes");
		PRINT_NXS(64, spd->mod_section.registered.thermal,
			"thermal       (Registered) Thermal Heat "
			"Spreader Solution");
		PRINT_NXS(65, spd->mod_section.registered.reg_id_lo,
			"reg_id_lo     (Registered) Register Manufacturer ID "
			"Code, LSB");
		PRINT_NXS(66, spd->mod_section.registered.reg_id_hi,
			"reg_id_hi     (Registered) Register Manufacturer ID "
			"Code, MSB");
		PRINT_NXS(67, spd->mod_section.registered.reg_rev,
			"reg_rev       (Registered) Register "
			"Revision Number");
		PRINT_NXS(68, spd->mod_section.registered.reg_type,
			"reg_type      (Registered) Register Type");
		for (i = 69; i <= 76; i++) {
			printf("%-3d    : %02x rcw[%d]\n", i,
				spd->mod_section.registered.rcw[i-69], i-69);
		}
		break;
	default:
		/* Module-specific Section, Unsupported Module Type */
		printf("%-3d-%3d: ", 60, 116);

		for (i = 60; i <= 116; i++)
			printf("%02x", spd->mod_section.uc[i - 60]);

		break;
	}

	/* Unique Module ID: Bytes 117-125 */
	PRINT_NXS(117, spd->mmid_lsb, "Module MfgID Code LSB - JEP-106");
	PRINT_NXS(118, spd->mmid_msb, "Module MfgID Code MSB - JEP-106");
	PRINT_NXS(119, spd->mloc,     "Mfg Location");
	PRINT_NNXXS(120, 121, spd->mdate[0], spd->mdate[1], "Mfg Date");

	printf("%-3d-%3d: ", 122, 125);

	for (i = 122; i <= 125; i++)
		printf("%02x ", spd->sernum[i - 122]);
	printf("   Module Serial Number\n");

	/* CRC: Bytes 126-127 */
	PRINT_NNXXS(126, 127, spd->crc[0], spd->crc[1], "  SPD CRC");

	/* Other Manufacturer Fields and User Space: Bytes 128-255 */
	printf("%-3d-%3d: ", 128, 145);
	for (i = 128; i <= 145; i++)
		printf("%02x ", spd->mpart[i - 128]);
	printf("   Mfg's Module Part Number\n");

	PRINT_NNXXS(146, 147, spd->mrev[0], spd->mrev[1],
		"Module Revision code");

	PRINT_NXS(148, spd->dmid_lsb, "DRAM MfgID Code LSB - JEP-106");
	PRINT_NXS(149, spd->dmid_msb, "DRAM MfgID Code MSB - JEP-106");

	printf("%-3d-%3d: ", 150, 175);
	for (i = 150; i <= 175; i++)
		printf("%02x ", spd->msd[i - 150]);
	printf("   Mfg's Specific Data\n");

	printf("%-3d-%3d: ", 176, 255);
	for (i = 176; i <= 255; i++)
		printf("%02x", spd->cust[i - 176]);
	printf("   Mfg's Specific Data\n");

}
#endif

#ifdef CONFIG_SYS_FSL_DDR4
void ddr4_spd_dump(const struct ddr4_spd_eeprom_s *spd)
{
	unsigned int i;

	/* General Section: Bytes 0-127 */

#define PRINT_NXS(x, y, z...) printf("%-3d    : %02x " z "\n", x, (u8)y);
#define PRINT_NNXXS(n0, n1, x0, x1, s) \
	printf("%-3d-%3d: %02x %02x " s "\n", n0, n1, x0, x1);

	PRINT_NXS(0, spd->info_size_crc,
		  "info_size_crc  bytes written into serial memory, CRC coverage");
	PRINT_NXS(1, spd->spd_rev,
		  "spd_rev        SPD Revision");
	PRINT_NXS(2, spd->mem_type,
		  "mem_type       Key Byte / DRAM Device Type");
	PRINT_NXS(3, spd->module_type,
		  "module_type    Key Byte / Module Type");
	PRINT_NXS(4, spd->density_banks,
		  "density_banks  SDRAM Density and Banks");
	PRINT_NXS(5, spd->addressing,
		  "addressing     SDRAM Addressing");
	PRINT_NXS(6, spd->package_type,
		  "package_type   Package type");
	PRINT_NXS(7, spd->opt_feature,
		  "opt_feature    Optional features");
	PRINT_NXS(8, spd->thermal_ref,
		  "thermal_ref    Thermal and Refresh options");
	PRINT_NXS(9, spd->oth_opt_features,
		  "oth_opt_features Other SDRAM optional features");
	PRINT_NXS(10, spd->res_10,
		  "res_10         Reserved");
	PRINT_NXS(11, spd->module_vdd,
		  "module_vdd     Module Nominal Voltage, VDD");
	PRINT_NXS(12, spd->organization,
		  "organization Module Organization");
	PRINT_NXS(13, spd->bus_width,
		  "bus_width      Module Memory Bus Width");
	PRINT_NXS(14, spd->therm_sensor,
		  "therm_sensor   Module Thermal Sensor");
	PRINT_NXS(15, spd->ext_type,
		  "ext_type       Extended module type");
	PRINT_NXS(16, spd->res_16,
		  "res_16       Reserved");
	PRINT_NXS(17, spd->timebases,
		  "timebases    MTb and FTB");
	PRINT_NXS(18, spd->tck_min,
		  "tck_min      tCKAVGmin");
	PRINT_NXS(19, spd->tck_max,
		  "tck_max      TCKAVGmax");
	PRINT_NXS(20, spd->caslat_b1,
		  "caslat_b1    CAS latencies, 1st byte");
	PRINT_NXS(21, spd->caslat_b2,
		  "caslat_b2    CAS latencies, 2nd byte");
	PRINT_NXS(22, spd->caslat_b3,
		  "caslat_b3    CAS latencies, 3rd byte ");
	PRINT_NXS(23, spd->caslat_b4,
		  "caslat_b4    CAS latencies, 4th byte");
	PRINT_NXS(24, spd->taa_min,
		  "taa_min      Min CAS Latency Time");
	PRINT_NXS(25, spd->trcd_min,
		  "trcd_min     Min RAS# to CAS# Delay Time");
	PRINT_NXS(26, spd->trp_min,
		  "trp_min      Min Row Precharge Delay Time");
	PRINT_NXS(27, spd->tras_trc_ext,
		  "tras_trc_ext Upper Nibbles for tRAS and tRC");
	PRINT_NXS(28, spd->tras_min_lsb,
		  "tras_min_lsb tRASmin, lsb");
	PRINT_NXS(29, spd->trc_min_lsb,
		  "trc_min_lsb  tRCmin, lsb");
	PRINT_NXS(30, spd->trfc1_min_lsb,
		  "trfc1_min_lsb  Min Refresh Recovery Delay Time, LSB");
	PRINT_NXS(31, spd->trfc1_min_msb,
		  "trfc1_min_msb  Min Refresh Recovery Delay Time, MSB ");
	PRINT_NXS(32, spd->trfc2_min_lsb,
		  "trfc2_min_lsb  Min Refresh Recovery Delay Time, LSB");
	PRINT_NXS(33, spd->trfc2_min_msb,
		  "trfc2_min_msb  Min Refresh Recovery Delay Time, MSB");
	PRINT_NXS(34, spd->trfc4_min_lsb,
		  "trfc4_min_lsb Min Refresh Recovery Delay Time, LSB");
	PRINT_NXS(35, spd->trfc4_min_msb,
		  "trfc4_min_msb Min Refresh Recovery Delay Time, MSB");
	PRINT_NXS(36, spd->tfaw_msb,
		  "tfaw_msb      Upper Nibble for tFAW");
	PRINT_NXS(37, spd->tfaw_min,
		  "tfaw_min      tFAW, lsb");
	PRINT_NXS(38, spd->trrds_min,
		  "trrds_min     tRRD_Smin, MTB");
	PRINT_NXS(39, spd->trrdl_min,
		  "trrdl_min     tRRD_Lmin, MTB");
	PRINT_NXS(40, spd->tccdl_min,
		  "tccdl_min     tCCS_Lmin, MTB");

	printf("%-3d-%3d: ", 41, 59);  /* Reserved, General Section */
	for (i = 41; i <= 59; i++)
		printf("%02x ", spd->res_41[i - 41]);

	puts("\n");
	printf("%-3d-%3d: ", 60, 77);
	for (i = 60; i <= 77; i++)
		printf("%02x ", spd->mapping[i - 60]);
	puts("   mapping[] Connector to SDRAM bit map\n");

	PRINT_NXS(117, spd->fine_tccdl_min,
		  "fine_tccdl_min Fine offset for tCCD_Lmin");
	PRINT_NXS(118, spd->fine_trrdl_min,
		  "fine_trrdl_min Fine offset for tRRD_Lmin");
	PRINT_NXS(119, spd->fine_trrds_min,
		  "fine_trrds_min Fine offset for tRRD_Smin");
	PRINT_NXS(120, spd->fine_trc_min,
		  "fine_trc_min   Fine offset for tRCmin");
	PRINT_NXS(121, spd->fine_trp_min,
		  "fine_trp_min   Fine offset for tRPmin");
	PRINT_NXS(122, spd->fine_trcd_min,
		  "fine_trcd_min  Fine offset for tRCDmin");
	PRINT_NXS(123, spd->fine_taa_min,
		  "fine_taa_min   Fine offset for tAAmin");
	PRINT_NXS(124, spd->fine_tck_max,
		  "fine_tck_max   Fine offset for tCKAVGmax");
	PRINT_NXS(125, spd->fine_tck_min,
		  "fine_tck_min   Fine offset for tCKAVGmin");

	/* CRC: Bytes 126-127 */
	PRINT_NNXXS(126, 127, spd->crc[0], spd->crc[1], "  SPD CRC");

	switch (spd->module_type) {
	case 0x02:  /* UDIMM */
	case 0x03:  /* SO-DIMM */
		PRINT_NXS(128, spd->mod_section.unbuffered.mod_height,
			  "mod_height    (Unbuffered) Module Nominal Height");
		PRINT_NXS(129, spd->mod_section.unbuffered.mod_thickness,
			  "mod_thickness (Unbuffered) Module Maximum Thickness");
		PRINT_NXS(130, spd->mod_section.unbuffered.ref_raw_card,
			  "ref_raw_card  (Unbuffered) Reference Raw Card Used");
		PRINT_NXS(131, spd->mod_section.unbuffered.addr_mapping,
			  "addr_mapping  (Unbuffered) Address mapping from Edge Connector to DRAM");
		PRINT_NNXXS(254, 255, spd->mod_section.unbuffered.crc[0],
			    spd->mod_section.unbuffered.crc[1], "  Module CRC");
		break;
	case 0x01:  /* RDIMM */
		PRINT_NXS(128, spd->mod_section.registered.mod_height,
			  "mod_height    (Registered) Module Nominal Height");
		PRINT_NXS(129, spd->mod_section.registered.mod_thickness,
			  "mod_thickness (Registered) Module Maximum Thickness");
		PRINT_NXS(130, spd->mod_section.registered.ref_raw_card,
			  "ref_raw_card  (Registered) Reference Raw Card Used");
		PRINT_NXS(131, spd->mod_section.registered.modu_attr,
			  "modu_attr     (Registered) DIMM Module Attributes");
		PRINT_NXS(132, spd->mod_section.registered.thermal,
			  "thermal       (Registered) Thermal Heat Spreader Solution");
		PRINT_NXS(133, spd->mod_section.registered.reg_id_lo,
			  "reg_id_lo     (Registered) Register Manufacturer ID Code, LSB");
		PRINT_NXS(134, spd->mod_section.registered.reg_id_hi,
			  "reg_id_hi     (Registered) Register Manufacturer ID Code, MSB");
		PRINT_NXS(135, spd->mod_section.registered.reg_rev,
			  "reg_rev       (Registered) Register Revision Number");
		PRINT_NXS(136, spd->mod_section.registered.reg_map,
			  "reg_map       (Registered) Address mapping");
		PRINT_NNXXS(254, 255, spd->mod_section.registered.crc[0],
			    spd->mod_section.registered.crc[1], "  Module CRC");
		break;
	case 0x04:  /* LRDIMM */
		PRINT_NXS(128, spd->mod_section.loadreduced.mod_height,
			  "mod_height    (Loadreduced) Module Nominal Height");
		PRINT_NXS(129, spd->mod_section.loadreduced.mod_thickness,
			  "mod_thickness (Loadreduced) Module Maximum Thickness");
		PRINT_NXS(130, spd->mod_section.loadreduced.ref_raw_card,
			  "ref_raw_card  (Loadreduced) Reference Raw Card Used");
		PRINT_NXS(131, spd->mod_section.loadreduced.modu_attr,
			  "modu_attr     (Loadreduced) DIMM Module Attributes");
		PRINT_NXS(132, spd->mod_section.loadreduced.thermal,
			  "thermal       (Loadreduced) Thermal Heat Spreader Solution");
		PRINT_NXS(133, spd->mod_section.loadreduced.reg_id_lo,
			  "reg_id_lo     (Loadreduced) Register Manufacturer ID Code, LSB");
		PRINT_NXS(134, spd->mod_section.loadreduced.reg_id_hi,
			  "reg_id_hi     (Loadreduced) Register Manufacturer ID Code, MSB");
		PRINT_NXS(135, spd->mod_section.loadreduced.reg_rev,
			  "reg_rev       (Loadreduced) Register Revision Number");
		PRINT_NXS(136, spd->mod_section.loadreduced.reg_map,
			  "reg_map       (Loadreduced) Address mapping");
		PRINT_NXS(137, spd->mod_section.loadreduced.reg_drv,
			  "reg_drv       (Loadreduced) Reg output drive strength");
		PRINT_NXS(138, spd->mod_section.loadreduced.reg_drv_ck,
			  "reg_drv_ck    (Loadreduced) Reg output drive strength for CK");
		PRINT_NXS(139, spd->mod_section.loadreduced.data_buf_rev,
			  "data_buf_rev  (Loadreduced) Data Buffer Revision Numbe");
		PRINT_NXS(140, spd->mod_section.loadreduced.vrefqe_r0,
			  "vrefqe_r0     (Loadreduced) DRAM VrefDQ for Package Rank 0");
		PRINT_NXS(141, spd->mod_section.loadreduced.vrefqe_r1,
			  "vrefqe_r1     (Loadreduced) DRAM VrefDQ for Package Rank 1");
		PRINT_NXS(142, spd->mod_section.loadreduced.vrefqe_r2,
			  "vrefqe_r2     (Loadreduced) DRAM VrefDQ for Package Rank 2");
		PRINT_NXS(143, spd->mod_section.loadreduced.vrefqe_r3,
			  "vrefqe_r3     (Loadreduced) DRAM VrefDQ for Package Rank 3");
		PRINT_NXS(144, spd->mod_section.loadreduced.data_intf,
			  "data_intf     (Loadreduced) Data Buffer VrefDQ for DRAM Interface");
		PRINT_NXS(145, spd->mod_section.loadreduced.data_drv_1866,
			  "data_drv_1866 (Loadreduced) Data Buffer MDQ Drive Strength and RTT");
		PRINT_NXS(146, spd->mod_section.loadreduced.data_drv_2400,
			  "data_drv_2400 (Loadreduced) Data Buffer MDQ Drive Strength and RTT");
		PRINT_NXS(147, spd->mod_section.loadreduced.data_drv_3200,
			  "data_drv_3200 (Loadreduced) Data Buffer MDQ Drive Strength and RTT");
		PRINT_NXS(148, spd->mod_section.loadreduced.dram_drv,
			  "dram_drv      (Loadreduced) DRAM Drive Strength");
		PRINT_NXS(149, spd->mod_section.loadreduced.dram_odt_1866,
			  "dram_odt_1866 (Loadreduced) DRAM ODT (RTT_WR, RTT_NOM)");
		PRINT_NXS(150, spd->mod_section.loadreduced.dram_odt_2400,
			  "dram_odt_2400 (Loadreduced) DRAM ODT (RTT_WR, RTT_NOM)");
		PRINT_NXS(151, spd->mod_section.loadreduced.dram_odt_3200,
			  "dram_odt_3200 (Loadreduced) DRAM ODT (RTT_WR, RTT_NOM)");
		PRINT_NXS(152, spd->mod_section.loadreduced.dram_odt_park_1866,
			  "dram_odt_park_1866 (Loadreduced) DRAM ODT (RTT_PARK)");
		PRINT_NXS(153, spd->mod_section.loadreduced.dram_odt_park_2400,
			  "dram_odt_park_2400 (Loadreduced) DRAM ODT (RTT_PARK)");
		PRINT_NXS(154, spd->mod_section.loadreduced.dram_odt_park_3200,
			  "dram_odt_park_3200 (Loadreduced) DRAM ODT (RTT_PARK)");
		PRINT_NNXXS(254, 255, spd->mod_section.loadreduced.crc[0],
			    spd->mod_section.loadreduced.crc[1],
			    "  Module CRC");
		break;
	default:
		/* Module-specific Section, Unsupported Module Type */
		printf("%-3d-%3d: ", 128, 255);

		for (i = 128; i <= 255; i++)
			printf("%02x", spd->mod_section.uc[i - 128]);

		break;
	}

	/* Unique Module ID: Bytes 320-383 */
	PRINT_NXS(320, spd->mmid_lsb, "Module MfgID Code LSB - JEP-106");
	PRINT_NXS(321, spd->mmid_msb, "Module MfgID Code MSB - JEP-106");
	PRINT_NXS(322, spd->mloc,     "Mfg Location");
	PRINT_NNXXS(323, 324, spd->mdate[0], spd->mdate[1], "Mfg Date");

	printf("%-3d-%3d: ", 325, 328);

	for (i = 325; i <= 328; i++)
		printf("%02x ", spd->sernum[i - 325]);
	printf("   Module Serial Number\n");

	printf("%-3d-%3d: ", 329, 348);
	for (i = 329; i <= 348; i++)
		printf("%02x ", spd->mpart[i - 329]);
	printf("   Mfg's Module Part Number\n");

	PRINT_NXS(349, spd->mrev, "Module Revision code");
	PRINT_NXS(350, spd->dmid_lsb, "DRAM MfgID Code LSB - JEP-106");
	PRINT_NXS(351, spd->dmid_msb, "DRAM MfgID Code MSB - JEP-106");
	PRINT_NXS(352, spd->stepping, "DRAM stepping");

	printf("%-3d-%3d: ", 353, 381);
	for (i = 353; i <= 381; i++)
		printf("%02x ", spd->msd[i - 353]);
	printf("   Mfg's Specific Data\n");
}
#endif

static inline void generic_spd_dump(const generic_spd_eeprom_t *spd)
{
#if defined(CONFIG_SYS_FSL_DDR1)
	ddr1_spd_dump(spd);
#elif defined(CONFIG_SYS_FSL_DDR2)
	ddr2_spd_dump(spd);
#elif defined(CONFIG_SYS_FSL_DDR3)
	ddr3_spd_dump(spd);
#elif defined(CONFIG_SYS_FSL_DDR4)
	ddr4_spd_dump(spd);
#endif
}

static void fsl_ddr_printinfo(const fsl_ddr_info_t *pinfo,
			unsigned int ctrl_mask,
			unsigned int dimm_mask,
			unsigned int do_mask)
{
	unsigned int i, j, retval;

	/* STEP 1:  DIMM SPD data */
	if (do_mask & STEP_GET_SPD) {
		for (i = 0; i < CONFIG_SYS_NUM_DDR_CTLRS; i++) {
			if (!(ctrl_mask & (1 << i)))
				continue;

			for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
				if (!(dimm_mask & (1 << j)))
					continue;

				printf("SPD info:  Controller=%u "
						"DIMM=%u\n", i, j);
				generic_spd_dump(
					&(pinfo->spd_installed_dimms[i][j]));
				printf("\n");
			}
			printf("\n");
		}
		printf("\n");
	}

	/* STEP 2:  DIMM Parameters */
	if (do_mask & STEP_COMPUTE_DIMM_PARMS) {
		for (i = 0; i < CONFIG_SYS_NUM_DDR_CTLRS; i++) {
			if (!(ctrl_mask & (1 << i)))
				continue;
			for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
				if (!(dimm_mask & (1 << j)))
					continue;
				printf("DIMM parameters:  Controller=%u "
						"DIMM=%u\n", i, j);
				print_dimm_parameters(
					&(pinfo->dimm_params[i][j]));
				printf("\n");
			}
			printf("\n");
		}
		printf("\n");
	}

	/* STEP 3:  Common Parameters */
	if (do_mask & STEP_COMPUTE_COMMON_PARMS) {
		for (i = 0; i < CONFIG_SYS_NUM_DDR_CTLRS; i++) {
			if (!(ctrl_mask & (1 << i)))
				continue;
			printf("\"lowest common\" DIMM parameters:  "
					"Controller=%u\n", i);
			print_lowest_common_dimm_parameters(
				&pinfo->common_timing_params[i]);
			printf("\n");
		}
		printf("\n");
	}

	/* STEP 4:  User Configuration Options */
	if (do_mask & STEP_GATHER_OPTS) {
		for (i = 0; i < CONFIG_SYS_NUM_DDR_CTLRS; i++) {
			if (!(ctrl_mask & (1 << i)))
				continue;
			printf("User Config Options: Controller=%u\n", i);
			print_memctl_options(&pinfo->memctl_opts[i]);
			printf("\n");
		}
		printf("\n");
	}

	/* STEP 5:  Address assignment */
	if (do_mask & STEP_ASSIGN_ADDRESSES) {
		for (i = 0; i < CONFIG_SYS_NUM_DDR_CTLRS; i++) {
			if (!(ctrl_mask & (1 << i)))
				continue;
			for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
				printf("Address Assignment: Controller=%u "
						"DIMM=%u\n", i, j);
				printf("Don't have this functionality yet\n");
			}
			printf("\n");
		}
		printf("\n");
	}

	/* STEP 6:  computed controller register values */
	if (do_mask & STEP_COMPUTE_REGS) {
		for (i = 0; i < CONFIG_SYS_NUM_DDR_CTLRS; i++) {
			if (!(ctrl_mask & (1 << i)))
				continue;
			printf("Computed Register Values: Controller=%u\n", i);
			print_fsl_memctl_config_regs(
				&pinfo->fsl_ddr_config_reg[i]);
			retval = check_fsl_memctl_config_regs(
				&pinfo->fsl_ddr_config_reg[i]);
			if (retval) {
				printf("check_fsl_memctl_config_regs "
					"result = %u\n", retval);
			}
			printf("\n");
		}
		printf("\n");
	}
}

struct data_strings {
	const char *data_name;
	unsigned int step_mask;
	unsigned int dimm_number_required;
};

#define DATA_OPTIONS(name, step, dimm) {#name, step, dimm}

static unsigned int fsl_ddr_parse_interactive_cmd(
	char **argv,
	int argc,
	unsigned int *pstep_mask,
	unsigned int *pctlr_mask,
	unsigned int *pdimm_mask,
	unsigned int *pdimm_number_required
	 ) {

	static const struct data_strings options[] = {
		DATA_OPTIONS(spd, STEP_GET_SPD, 1),
		DATA_OPTIONS(dimmparms, STEP_COMPUTE_DIMM_PARMS, 1),
		DATA_OPTIONS(commonparms, STEP_COMPUTE_COMMON_PARMS, 0),
		DATA_OPTIONS(opts, STEP_GATHER_OPTS, 0),
		DATA_OPTIONS(addresses, STEP_ASSIGN_ADDRESSES, 0),
		DATA_OPTIONS(regs, STEP_COMPUTE_REGS, 0),
	};
	static const unsigned int n_opts = ARRAY_SIZE(options);

	unsigned int i, j;
	unsigned int error = 0;

	for (i = 1; i < argc; i++) {
		unsigned int matched = 0;

		for (j = 0; j < n_opts; j++) {
			if (strcmp(options[j].data_name, argv[i]) != 0)
				continue;
			*pstep_mask |= options[j].step_mask;
			*pdimm_number_required =
				options[j].dimm_number_required;
			matched = 1;
			break;
		}

		if (matched)
			continue;

		if (argv[i][0] == 'c') {
			char c = argv[i][1];
			if (isdigit(c))
				*pctlr_mask |= 1 << (c - '0');
			continue;
		}

		if (argv[i][0] == 'd') {
			char c = argv[i][1];
			if (isdigit(c))
				*pdimm_mask |= 1 << (c - '0');
			continue;
		}

		printf("unknown arg %s\n", argv[i]);
		*pstep_mask = 0;
		error = 1;
		break;
	}

	return error;
}

int fsl_ddr_interactive_env_var_exists(void)
{
	char buffer[CONFIG_SYS_CBSIZE];

	if (env_get_f("ddr_interactive", buffer, CONFIG_SYS_CBSIZE) >= 0)
		return 1;

	return 0;
}

unsigned long long fsl_ddr_interactive(fsl_ddr_info_t *pinfo, int var_is_set)
{
	unsigned long long ddrsize;
	const char *prompt = "FSL DDR>";
	char buffer[CONFIG_SYS_CBSIZE];
	char buffer2[CONFIG_SYS_CBSIZE];
	char *p = NULL;
	char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated */
	int argc;
	unsigned int next_step = STEP_GET_SPD;
	const char *usage = {
		"commands:\n"
		"print      print SPD and intermediate computed data\n"
		"reset      reboot machine\n"
		"recompute  reload SPD and options to default and recompute regs\n"
		"edit       modify spd, parameter, or option\n"
		"compute    recompute registers from current next_step to end\n"
		"copy       copy parameters\n"
		"next_step  shows current next_step\n"
		"help       this message\n"
		"go         program the memory controller and continue with u-boot\n"
	};

	if (var_is_set) {
		if (env_get_f("ddr_interactive", buffer2,
			      CONFIG_SYS_CBSIZE) > 0)
			p = buffer2;
		else
			var_is_set = 0;
	}

	/*
	 * The strategy for next_step is that it points to the next
	 * step in the computation process that needs to be done.
	 */
	while (1) {
		if (var_is_set) {
			char *pend = strchr(p, ';');
			if (pend) {
				/* found command separator, copy sub-command */
				*pend = '\0';
				strcpy(buffer, p);
				p = pend + 1;
			} else {
				/* separator not found, copy whole string */
				strcpy(buffer, p);
				p = NULL;
				var_is_set = 0;
			}
		} else {
			/*
			 * No need to worry for buffer overflow here in
			 * this function;  cli_readline() maxes out at
			 * CFG_CBSIZE
			 */
			cli_readline_into_buffer(prompt, buffer, 0);
		}
		argc = cli_simple_parse_line(buffer, argv);
		if (argc == 0)
			continue;


		if (strcmp(argv[0], "help") == 0) {
			puts(usage);
			continue;
		}

		if (strcmp(argv[0], "next_step") == 0) {
			printf("next_step = 0x%02X (%s)\n",
			       next_step,
			       step_to_string(next_step));
			continue;
		}

		if (strcmp(argv[0], "copy") == 0) {
			unsigned int error = 0;
			unsigned int step_mask = 0;
			unsigned int src_ctlr_mask = 0;
			unsigned int src_dimm_mask = 0;
			unsigned int dimm_number_required = 0;
			unsigned int src_ctlr_num = 0;
			unsigned int src_dimm_num = 0;
			unsigned int dst_ctlr_num = -1;
			unsigned int dst_dimm_num = -1;
			unsigned int i, num_dest_parms;

			if (argc == 1) {
				printf("copy <src c#> <src d#> <spd|dimmparms|commonparms|opts|addresses|regs> <dst c#> <dst d#>\n");
				continue;
			}

			error = fsl_ddr_parse_interactive_cmd(
				argv, argc,
				&step_mask,
				&src_ctlr_mask,
				&src_dimm_mask,
				&dimm_number_required
			);

			/* XXX: only dimm_number_required and step_mask will
			   be used by this function.  Parse the controller and
			   DIMM number separately because it is easier.  */

			if (error)
				continue;

			/* parse source destination controller / DIMM */

			num_dest_parms = dimm_number_required ? 2 : 1;

			for (i = 0; i < argc; i++) {
				if (argv[i][0] == 'c') {
					char c = argv[i][1];
					if (isdigit(c)) {
						src_ctlr_num = (c - '0');
						break;
					}
				}
			}

			for (i = 0; i < argc; i++) {
				if (argv[i][0] == 'd') {
					char c = argv[i][1];
					if (isdigit(c)) {
						src_dimm_num = (c - '0');
						break;
					}
				}
			}

			/* parse destination controller / DIMM */

			for (i = argc - 1; i >= argc - num_dest_parms; i--) {
				if (argv[i][0] == 'c') {
					char c = argv[i][1];
					if (isdigit(c)) {
						dst_ctlr_num = (c - '0');
						break;
					}
				}
			}

			for (i = argc - 1; i >= argc - num_dest_parms; i--) {
				if (argv[i][0] == 'd') {
					char c = argv[i][1];
					if (isdigit(c)) {
						dst_dimm_num = (c - '0');
						break;
					}
				}
			}

			/* TODO: validate inputs */

			debug("src_ctlr_num = %u, src_dimm_num = %u, dst_ctlr_num = %u, dst_dimm_num = %u, step_mask = %x\n",
				src_ctlr_num, src_dimm_num, dst_ctlr_num, dst_dimm_num, step_mask);


			switch (step_mask) {

			case STEP_GET_SPD:
				memcpy(&(pinfo->spd_installed_dimms[dst_ctlr_num][dst_dimm_num]),
					&(pinfo->spd_installed_dimms[src_ctlr_num][src_dimm_num]),
					sizeof(pinfo->spd_installed_dimms[0][0]));
				break;

			case STEP_COMPUTE_DIMM_PARMS:
				memcpy(&(pinfo->dimm_params[dst_ctlr_num][dst_dimm_num]),
					&(pinfo->dimm_params[src_ctlr_num][src_dimm_num]),
					sizeof(pinfo->dimm_params[0][0]));
				break;

			case STEP_COMPUTE_COMMON_PARMS:
				memcpy(&(pinfo->common_timing_params[dst_ctlr_num]),
					&(pinfo->common_timing_params[src_ctlr_num]),
					sizeof(pinfo->common_timing_params[0]));
				break;

			case STEP_GATHER_OPTS:
				memcpy(&(pinfo->memctl_opts[dst_ctlr_num]),
					&(pinfo->memctl_opts[src_ctlr_num]),
					sizeof(pinfo->memctl_opts[0]));
				break;

			/* someday be able to have addresses to copy addresses... */

			case STEP_COMPUTE_REGS:
				memcpy(&(pinfo->fsl_ddr_config_reg[dst_ctlr_num]),
					&(pinfo->fsl_ddr_config_reg[src_ctlr_num]),
					sizeof(pinfo->memctl_opts[0]));
				break;

			default:
				printf("unexpected step_mask value\n");
			}

			continue;

		}

		if (strcmp(argv[0], "edit") == 0) {
			unsigned int error = 0;
			unsigned int step_mask = 0;
			unsigned int ctlr_mask = 0;
			unsigned int dimm_mask = 0;
			char *p_element = NULL;
			char *p_value = NULL;
			unsigned int dimm_number_required = 0;
			unsigned int ctrl_num;
			unsigned int dimm_num;

			if (argc == 1) {
				/* Only the element and value must be last */
				printf("edit <c#> <d#> "
					"<spd|dimmparms|commonparms|opts|"
					"addresses|regs> <element> <value>\n");
				printf("for spd, specify byte number for "
					"element\n");
				continue;
			}

			error = fsl_ddr_parse_interactive_cmd(
				argv, argc - 2,
				&step_mask,
				&ctlr_mask,
				&dimm_mask,
				&dimm_number_required
			);

			if (error)
				continue;


			/* Check arguments */

			/* ERROR: If no steps were found */
			if (step_mask == 0) {
				printf("Error: No valid steps were specified "
						"in argument.\n");
				continue;
			}

			/* ERROR: If multiple steps were found */
			if (step_mask & (step_mask - 1)) {
				printf("Error: Multiple steps specified in "
						"argument.\n");
				continue;
			}

			/* ERROR: Controller not specified */
			if (ctlr_mask == 0) {
				printf("Error: controller number not "
					"specified or no element and "
					"value specified\n");
				continue;
			}

			if (ctlr_mask & (ctlr_mask - 1)) {
				printf("Error: multiple controllers "
						"specified, %X\n", ctlr_mask);
				continue;
			}

			/* ERROR: DIMM number not specified */
			if (dimm_number_required && dimm_mask == 0) {
				printf("Error: DIMM number number not "
					"specified or no element and "
					"value specified\n");
				continue;
			}

			if (dimm_mask & (dimm_mask - 1)) {
				printf("Error: multipled DIMMs specified\n");
				continue;
			}

			p_element = argv[argc - 2];
			p_value = argv[argc - 1];

			ctrl_num = __ilog2(ctlr_mask);
			dimm_num = __ilog2(dimm_mask);

			switch (step_mask) {
			case STEP_GET_SPD:
				{
					unsigned int element_num;
					unsigned int value;

					element_num = simple_strtoul(p_element,
								     NULL, 0);
					value = simple_strtoul(p_value,
							       NULL, 0);
					fsl_ddr_spd_edit(pinfo,
							       ctrl_num,
							       dimm_num,
							       element_num,
							       value);
					next_step = STEP_COMPUTE_DIMM_PARMS;
				}
				break;

			case STEP_COMPUTE_DIMM_PARMS:
				fsl_ddr_dimm_parameters_edit(
						 pinfo, ctrl_num, dimm_num,
						 p_element, p_value);
				next_step = STEP_COMPUTE_COMMON_PARMS;
				break;

			case STEP_COMPUTE_COMMON_PARMS:
				lowest_common_dimm_parameters_edit(pinfo,
						ctrl_num, p_element, p_value);
				next_step = STEP_GATHER_OPTS;
				break;

			case STEP_GATHER_OPTS:
				fsl_ddr_options_edit(pinfo, ctrl_num,
							   p_element, p_value);
				next_step = STEP_ASSIGN_ADDRESSES;
				break;

			case STEP_ASSIGN_ADDRESSES:
				printf("editing of address assignment "
						"not yet implemented\n");
				break;

			case STEP_COMPUTE_REGS:
				{
					fsl_ddr_regs_edit(pinfo,
								ctrl_num,
								p_element,
								p_value);
					next_step = STEP_PROGRAM_REGS;
				}
				break;

			default:
				printf("programming error\n");
				while (1)
					;
				break;
			}
			continue;
		}

		if (strcmp(argv[0], "reset") == 0) {
			/*
			 * Reboot machine.
			 * Args don't seem to matter because this
			 * doesn't return
			 */
			do_reset(NULL, 0, 0, NULL);
			printf("Reset didn't work\n");
		}

		if (strcmp(argv[0], "recompute") == 0) {
			/*
			 * Recalculate everything, starting with
			 * loading SPD EEPROM from DIMMs
			 */
			next_step = STEP_GET_SPD;
			ddrsize = fsl_ddr_compute(pinfo, next_step, 0);
			continue;
		}

		if (strcmp(argv[0], "compute") == 0) {
			/*
			 * Compute rest of steps starting at
			 * the current next_step/
			 */
			ddrsize = fsl_ddr_compute(pinfo, next_step, 0);
			continue;
		}

		if (strcmp(argv[0], "print") == 0) {
			unsigned int error = 0;
			unsigned int step_mask = 0;
			unsigned int ctlr_mask = 0;
			unsigned int dimm_mask = 0;
			unsigned int dimm_number_required = 0;

			if (argc == 1) {
				printf("print [c<n>] [d<n>] [spd] [dimmparms] "
				  "[commonparms] [opts] [addresses] [regs]\n");
				continue;
			}

			error = fsl_ddr_parse_interactive_cmd(
				argv, argc,
				&step_mask,
				&ctlr_mask,
				&dimm_mask,
				&dimm_number_required
			);

			if (error)
				continue;

			/* If no particular controller was found, print all */
			if (ctlr_mask == 0)
				ctlr_mask = 0xFF;

			/* If no particular dimm was found, print all dimms. */
			if (dimm_mask == 0)
				dimm_mask = 0xFF;

			/* If no steps were found, print all steps. */
			if (step_mask == 0)
				step_mask = STEP_ALL;

			fsl_ddr_printinfo(pinfo, ctlr_mask,
						dimm_mask, step_mask);
			continue;
		}

		if (strcmp(argv[0], "go") == 0) {
			if (next_step)
				ddrsize = fsl_ddr_compute(pinfo, next_step, 0);
			break;
		}

		printf("unknown command %s\n", argv[0]);
	}

	debug("end of memory = %llu\n", (u64)ddrsize);

	return ddrsize;
}
