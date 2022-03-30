// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 */

/*
 * Generic driver for Freescale DDR/DDR2/DDR3 memory controller.
 * Based on code from spd_sdram.c
 * Author: James Yang [at freescale.com]
 */

#include <common.h>
#include <i2c.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr.h>

/*
 * CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY is the physical address from the view
 * of DDR controllers. It is the same as CONFIG_SYS_DDR_SDRAM_BASE for
 * all Power SoCs. But it could be different for ARM SoCs. For example,
 * fsl_lsch3 has a mapping mechanism to map DDR memory to ranges (in order) of
 * 0x00_8000_0000 ~ 0x00_ffff_ffff
 * 0x80_8000_0000 ~ 0xff_ffff_ffff
 */
#ifndef CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY
#ifdef CONFIG_MPC83xx
#define CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY CONFIG_SYS_SDRAM_BASE
#else
#define CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY CONFIG_SYS_DDR_SDRAM_BASE
#endif
#endif

#ifdef CONFIG_PPC
#include <asm/fsl_law.h>

void fsl_ddr_set_lawbar(
		const common_timing_params_t *memctl_common_params,
		unsigned int memctl_interleaved,
		unsigned int ctrl_num);
#endif

void fsl_ddr_set_intl3r(const unsigned int granule_size);
#if defined(SPD_EEPROM_ADDRESS) || \
    defined(SPD_EEPROM_ADDRESS1) || defined(SPD_EEPROM_ADDRESS2) || \
    defined(SPD_EEPROM_ADDRESS3) || defined(SPD_EEPROM_ADDRESS4)
#if (CONFIG_SYS_NUM_DDR_CTLRS == 1) && (CONFIG_DIMM_SLOTS_PER_CTLR == 1)
u8 spd_i2c_addr[CONFIG_SYS_NUM_DDR_CTLRS][CONFIG_DIMM_SLOTS_PER_CTLR] = {
	[0][0] = SPD_EEPROM_ADDRESS,
};
#elif (CONFIG_SYS_NUM_DDR_CTLRS == 1) && (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
u8 spd_i2c_addr[CONFIG_SYS_NUM_DDR_CTLRS][CONFIG_DIMM_SLOTS_PER_CTLR] = {
	[0][0] = SPD_EEPROM_ADDRESS1,	/* controller 1 */
	[0][1] = SPD_EEPROM_ADDRESS2,	/* controller 1 */
};
#elif (CONFIG_SYS_NUM_DDR_CTLRS == 2) && (CONFIG_DIMM_SLOTS_PER_CTLR == 1)
u8 spd_i2c_addr[CONFIG_SYS_NUM_DDR_CTLRS][CONFIG_DIMM_SLOTS_PER_CTLR] = {
	[0][0] = SPD_EEPROM_ADDRESS1,	/* controller 1 */
	[1][0] = SPD_EEPROM_ADDRESS2,	/* controller 2 */
};
#elif (CONFIG_SYS_NUM_DDR_CTLRS == 2) && (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
u8 spd_i2c_addr[CONFIG_SYS_NUM_DDR_CTLRS][CONFIG_DIMM_SLOTS_PER_CTLR] = {
	[0][0] = SPD_EEPROM_ADDRESS1,	/* controller 1 */
	[0][1] = SPD_EEPROM_ADDRESS2,	/* controller 1 */
	[1][0] = SPD_EEPROM_ADDRESS3,	/* controller 2 */
	[1][1] = SPD_EEPROM_ADDRESS4,	/* controller 2 */
};
#elif (CONFIG_SYS_NUM_DDR_CTLRS == 3) && (CONFIG_DIMM_SLOTS_PER_CTLR == 1)
u8 spd_i2c_addr[CONFIG_SYS_NUM_DDR_CTLRS][CONFIG_DIMM_SLOTS_PER_CTLR] = {
	[0][0] = SPD_EEPROM_ADDRESS1,	/* controller 1 */
	[1][0] = SPD_EEPROM_ADDRESS2,	/* controller 2 */
	[2][0] = SPD_EEPROM_ADDRESS3,	/* controller 3 */
};
#elif (CONFIG_SYS_NUM_DDR_CTLRS == 3) && (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
u8 spd_i2c_addr[CONFIG_SYS_NUM_DDR_CTLRS][CONFIG_DIMM_SLOTS_PER_CTLR] = {
	[0][0] = SPD_EEPROM_ADDRESS1,	/* controller 1 */
	[0][1] = SPD_EEPROM_ADDRESS2,	/* controller 1 */
	[1][0] = SPD_EEPROM_ADDRESS3,	/* controller 2 */
	[1][1] = SPD_EEPROM_ADDRESS4,	/* controller 2 */
	[2][0] = SPD_EEPROM_ADDRESS5,	/* controller 3 */
	[2][1] = SPD_EEPROM_ADDRESS6,	/* controller 3 */
};

#endif

#define SPD_SPA0_ADDRESS	0x36
#define SPD_SPA1_ADDRESS	0x37

static void __get_spd(generic_spd_eeprom_t *spd, u8 i2c_address)
{
	int ret;
#ifdef CONFIG_SYS_FSL_DDR4
	uint8_t dummy = 0;
#endif

	i2c_set_bus_num(CONFIG_SYS_SPD_BUS_NUM);

#ifdef CONFIG_SYS_FSL_DDR4
	/*
	 * DDR4 SPD has 384 to 512 bytes
	 * To access the lower 256 bytes, we need to set EE page address to 0
	 * To access the upper 256 bytes, we need to set EE page address to 1
	 * See Jedec standar No. 21-C for detail
	 */
	i2c_write(SPD_SPA0_ADDRESS, 0, 1, &dummy, 1);
	ret = i2c_read(i2c_address, 0, 1, (uchar *)spd, 256);
	if (!ret) {
		i2c_write(SPD_SPA1_ADDRESS, 0, 1, &dummy, 1);
		ret = i2c_read(i2c_address, 0, 1,
			       (uchar *)((ulong)spd + 256),
			       min(256,
				   (int)sizeof(generic_spd_eeprom_t) - 256));
	}
#else
	ret = i2c_read(i2c_address, 0, 1, (uchar *)spd,
				sizeof(generic_spd_eeprom_t));
#endif

	if (ret) {
		if (i2c_address ==
#ifdef SPD_EEPROM_ADDRESS
				SPD_EEPROM_ADDRESS
#elif defined(SPD_EEPROM_ADDRESS1)
				SPD_EEPROM_ADDRESS1
#endif
				) {
			printf("DDR: failed to read SPD from address %u\n",
				i2c_address);
		} else {
			debug("DDR: failed to read SPD from address %u\n",
				i2c_address);
		}
		memset(spd, 0, sizeof(generic_spd_eeprom_t));
	}
}

__attribute__((weak, alias("__get_spd")))
void get_spd(generic_spd_eeprom_t *spd, u8 i2c_address);

/* This function allows boards to update SPD address */
__weak void update_spd_address(unsigned int ctrl_num,
			       unsigned int slot,
			       unsigned int *addr)
{
}

void fsl_ddr_get_spd(generic_spd_eeprom_t *ctrl_dimms_spd,
		      unsigned int ctrl_num, unsigned int dimm_slots_per_ctrl)
{
	unsigned int i;
	unsigned int i2c_address = 0;

	if (ctrl_num >= CONFIG_SYS_NUM_DDR_CTLRS) {
		printf("%s unexpected ctrl_num = %u\n", __FUNCTION__, ctrl_num);
		return;
	}

	for (i = 0; i < dimm_slots_per_ctrl; i++) {
		i2c_address = spd_i2c_addr[ctrl_num][i];
		update_spd_address(ctrl_num, i, &i2c_address);
		get_spd(&(ctrl_dimms_spd[i]), i2c_address);
	}
}
#else
void fsl_ddr_get_spd(generic_spd_eeprom_t *ctrl_dimms_spd,
		      unsigned int ctrl_num, unsigned int dimm_slots_per_ctrl)
{
}
#endif /* SPD_EEPROM_ADDRESSx */

/*
 * ASSUMPTIONS:
 *    - Same number of CONFIG_DIMM_SLOTS_PER_CTLR on each controller
 *    - Same memory data bus width on all controllers
 *
 * NOTES:
 *
 * The memory controller and associated documentation use confusing
 * terminology when referring to the orgranization of DRAM.
 *
 * Here is a terminology translation table:
 *
 * memory controller/documention  |industry   |this code  |signals
 * -------------------------------|-----------|-----------|-----------------
 * physical bank/bank		  |rank       |rank	  |chip select (CS)
 * logical bank/sub-bank	  |bank       |bank	  |bank address (BA)
 * page/row			  |row	      |page	  |row address
 * ???				  |column     |column	  |column address
 *
 * The naming confusion is further exacerbated by the descriptions of the
 * memory controller interleaving feature, where accesses are interleaved
 * _BETWEEN_ two seperate memory controllers.  This is configured only in
 * CS0_CONFIG[INTLV_CTL] of each memory controller.
 *
 * memory controller documentation | number of chip selects
 *				   | per memory controller supported
 * --------------------------------|-----------------------------------------
 * cache line interleaving	   | 1 (CS0 only)
 * page interleaving		   | 1 (CS0 only)
 * bank interleaving		   | 1 (CS0 only)
 * superbank interleraving	   | depends on bank (chip select)
 *				   |   interleraving [rank interleaving]
 *				   |   mode used on every memory controller
 *
 * Even further confusing is the existence of the interleaving feature
 * _WITHIN_ each memory controller.  The feature is referred to in
 * documentation as chip select interleaving or bank interleaving,
 * although it is configured in the DDR_SDRAM_CFG field.
 *
 * Name of field		| documentation name	| this code
 * -----------------------------|-----------------------|------------------
 * DDR_SDRAM_CFG[BA_INTLV_CTL]	| Bank (chip select)	| rank interleaving
 *				|  interleaving
 */

const char *step_string_tbl[] = {
	"STEP_GET_SPD",
	"STEP_COMPUTE_DIMM_PARMS",
	"STEP_COMPUTE_COMMON_PARMS",
	"STEP_GATHER_OPTS",
	"STEP_ASSIGN_ADDRESSES",
	"STEP_COMPUTE_REGS",
	"STEP_PROGRAM_REGS",
	"STEP_ALL"
};

const char * step_to_string(unsigned int step) {

	unsigned int s = __ilog2(step);

	if ((1 << s) != step)
		return step_string_tbl[7];

	if (s >= ARRAY_SIZE(step_string_tbl)) {
		printf("Error for the step in %s\n", __func__);
		s = 0;
	}

	return step_string_tbl[s];
}

static unsigned long long __step_assign_addresses(fsl_ddr_info_t *pinfo,
			  unsigned int dbw_cap_adj[])
{
	unsigned int i, j;
	unsigned long long total_mem, current_mem_base, total_ctlr_mem;
	unsigned long long rank_density, ctlr_density = 0;
	unsigned int first_ctrl = pinfo->first_ctrl;
	unsigned int last_ctrl = first_ctrl + pinfo->num_ctrls - 1;

	/*
	 * If a reduced data width is requested, but the SPD
	 * specifies a physically wider device, adjust the
	 * computed dimm capacities accordingly before
	 * assigning addresses.
	 */
	for (i = first_ctrl; i <= last_ctrl; i++) {
		unsigned int found = 0;

		switch (pinfo->memctl_opts[i].data_bus_width) {
		case 2:
			/* 16-bit */
			for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
				unsigned int dw;
				if (!pinfo->dimm_params[i][j].n_ranks)
					continue;
				dw = pinfo->dimm_params[i][j].primary_sdram_width;
				if ((dw == 72 || dw == 64)) {
					dbw_cap_adj[i] = 2;
					break;
				} else if ((dw == 40 || dw == 32)) {
					dbw_cap_adj[i] = 1;
					break;
				}
			}
			break;

		case 1:
			/* 32-bit */
			for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
				unsigned int dw;
				dw = pinfo->dimm_params[i][j].data_width;
				if (pinfo->dimm_params[i][j].n_ranks
				    && (dw == 72 || dw == 64)) {
					/*
					 * FIXME: can't really do it
					 * like this because this just
					 * further reduces the memory
					 */
					found = 1;
					break;
				}
			}
			if (found) {
				dbw_cap_adj[i] = 1;
			}
			break;

		case 0:
			/* 64-bit */
			break;

		default:
			printf("unexpected data bus width "
				"specified controller %u\n", i);
			return 1;
		}
		debug("dbw_cap_adj[%d]=%d\n", i, dbw_cap_adj[i]);
	}

	current_mem_base = pinfo->mem_base;
	total_mem = 0;
	if (pinfo->memctl_opts[first_ctrl].memctl_interleaving) {
		rank_density = pinfo->dimm_params[first_ctrl][0].rank_density >>
					dbw_cap_adj[first_ctrl];
		switch (pinfo->memctl_opts[first_ctrl].ba_intlv_ctl &
					FSL_DDR_CS0_CS1_CS2_CS3) {
		case FSL_DDR_CS0_CS1_CS2_CS3:
			ctlr_density = 4 * rank_density;
			break;
		case FSL_DDR_CS0_CS1:
		case FSL_DDR_CS0_CS1_AND_CS2_CS3:
			ctlr_density = 2 * rank_density;
			break;
		case FSL_DDR_CS2_CS3:
		default:
			ctlr_density = rank_density;
			break;
		}
		debug("rank density is 0x%llx, ctlr density is 0x%llx\n",
			rank_density, ctlr_density);
		for (i = first_ctrl; i <= last_ctrl; i++) {
			if (pinfo->memctl_opts[i].memctl_interleaving) {
				switch (pinfo->memctl_opts[i].memctl_interleaving_mode) {
				case FSL_DDR_256B_INTERLEAVING:
				case FSL_DDR_CACHE_LINE_INTERLEAVING:
				case FSL_DDR_PAGE_INTERLEAVING:
				case FSL_DDR_BANK_INTERLEAVING:
				case FSL_DDR_SUPERBANK_INTERLEAVING:
					total_ctlr_mem = 2 * ctlr_density;
					break;
				case FSL_DDR_3WAY_1KB_INTERLEAVING:
				case FSL_DDR_3WAY_4KB_INTERLEAVING:
				case FSL_DDR_3WAY_8KB_INTERLEAVING:
					total_ctlr_mem = 3 * ctlr_density;
					break;
				case FSL_DDR_4WAY_1KB_INTERLEAVING:
				case FSL_DDR_4WAY_4KB_INTERLEAVING:
				case FSL_DDR_4WAY_8KB_INTERLEAVING:
					total_ctlr_mem = 4 * ctlr_density;
					break;
				default:
					panic("Unknown interleaving mode");
				}
				pinfo->common_timing_params[i].base_address =
							current_mem_base;
				pinfo->common_timing_params[i].total_mem =
							total_ctlr_mem;
				total_mem = current_mem_base + total_ctlr_mem;
				debug("ctrl %d base 0x%llx\n", i, current_mem_base);
				debug("ctrl %d total 0x%llx\n", i, total_ctlr_mem);
			} else {
				/* when 3rd controller not interleaved */
				current_mem_base = total_mem;
				total_ctlr_mem = 0;
				pinfo->common_timing_params[i].base_address =
							current_mem_base;
				for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
					unsigned long long cap =
						pinfo->dimm_params[i][j].capacity >> dbw_cap_adj[i];
					pinfo->dimm_params[i][j].base_address =
						current_mem_base;
					debug("ctrl %d dimm %d base 0x%llx\n", i, j, current_mem_base);
					current_mem_base += cap;
					total_ctlr_mem += cap;
				}
				debug("ctrl %d total 0x%llx\n", i, total_ctlr_mem);
				pinfo->common_timing_params[i].total_mem =
							total_ctlr_mem;
				total_mem += total_ctlr_mem;
			}
		}
	} else {
		/*
		 * Simple linear assignment if memory
		 * controllers are not interleaved.
		 */
		for (i = first_ctrl; i <= last_ctrl; i++) {
			total_ctlr_mem = 0;
			pinfo->common_timing_params[i].base_address =
						current_mem_base;
			for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
				/* Compute DIMM base addresses. */
				unsigned long long cap =
					pinfo->dimm_params[i][j].capacity >> dbw_cap_adj[i];
				pinfo->dimm_params[i][j].base_address =
					current_mem_base;
				debug("ctrl %d dimm %d base 0x%llx\n", i, j, current_mem_base);
				current_mem_base += cap;
				total_ctlr_mem += cap;
			}
			debug("ctrl %d total 0x%llx\n", i, total_ctlr_mem);
			pinfo->common_timing_params[i].total_mem =
							total_ctlr_mem;
			total_mem += total_ctlr_mem;
		}
	}
	debug("Total mem by %s is 0x%llx\n", __func__, total_mem);

	return total_mem;
}

/* Use weak function to allow board file to override the address assignment */
__attribute__((weak, alias("__step_assign_addresses")))
unsigned long long step_assign_addresses(fsl_ddr_info_t *pinfo,
			  unsigned int dbw_cap_adj[]);

unsigned long long
fsl_ddr_compute(fsl_ddr_info_t *pinfo, unsigned int start_step,
				       unsigned int size_only)
{
	unsigned int i, j;
	unsigned long long total_mem = 0;
	int assert_reset = 0;
	unsigned int first_ctrl =  pinfo->first_ctrl;
	unsigned int last_ctrl = first_ctrl + pinfo->num_ctrls - 1;
	__maybe_unused int retval;
	__maybe_unused bool goodspd = false;
	__maybe_unused int dimm_slots_per_ctrl = pinfo->dimm_slots_per_ctrl;

	fsl_ddr_cfg_regs_t *ddr_reg = pinfo->fsl_ddr_config_reg;
	common_timing_params_t *timing_params = pinfo->common_timing_params;
	if (pinfo->board_need_mem_reset)
		assert_reset = pinfo->board_need_mem_reset();

	/* data bus width capacity adjust shift amount */
	unsigned int dbw_capacity_adjust[CONFIG_SYS_NUM_DDR_CTLRS];

	for (i = first_ctrl; i <= last_ctrl; i++)
		dbw_capacity_adjust[i] = 0;

	debug("starting at step %u (%s)\n",
	      start_step, step_to_string(start_step));

	switch (start_step) {
	case STEP_GET_SPD:
#if defined(CONFIG_DDR_SPD) || defined(CONFIG_SPD_EEPROM)
		/* STEP 1:  Gather all DIMM SPD data */
		for (i = first_ctrl; i <= last_ctrl; i++) {
			fsl_ddr_get_spd(pinfo->spd_installed_dimms[i], i,
					dimm_slots_per_ctrl);
		}

	case STEP_COMPUTE_DIMM_PARMS:
		/* STEP 2:  Compute DIMM parameters from SPD data */

		for (i = first_ctrl; i <= last_ctrl; i++) {
			for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
				generic_spd_eeprom_t *spd =
					&(pinfo->spd_installed_dimms[i][j]);
				dimm_params_t *pdimm =
					&(pinfo->dimm_params[i][j]);
				retval = compute_dimm_parameters(
							i, spd, pdimm, j);
#ifdef CONFIG_SYS_DDR_RAW_TIMING
				if (!j && retval) {
					printf("SPD error on controller %d! "
					"Trying fallback to raw timing "
					"calculation\n", i);
					retval = fsl_ddr_get_dimm_params(pdimm,
									 i, j);
				}
#else
				if (retval == 2) {
					printf("Error: compute_dimm_parameters"
					" non-zero returned FATAL value "
					"for memctl=%u dimm=%u\n", i, j);
					return 0;
				}
#endif
				if (retval) {
					debug("Warning: compute_dimm_parameters"
					" non-zero return value for memctl=%u "
					"dimm=%u\n", i, j);
				} else {
					goodspd = true;
				}
			}
		}
		if (!goodspd) {
			/*
			 * No valid SPD found
			 * Throw an error if this is for main memory, i.e.
			 * first_ctrl == 0. Otherwise, siliently return 0
			 * as the memory size.
			 */
			if (first_ctrl == 0)
				printf("Error: No valid SPD detected.\n");

			return 0;
		}
#elif defined(CONFIG_SYS_DDR_RAW_TIMING)
	case STEP_COMPUTE_DIMM_PARMS:
		for (i = first_ctrl; i <= last_ctrl; i++) {
			for (j = 0; j < CONFIG_DIMM_SLOTS_PER_CTLR; j++) {
				dimm_params_t *pdimm =
					&(pinfo->dimm_params[i][j]);
				fsl_ddr_get_dimm_params(pdimm, i, j);
			}
		}
		debug("Filling dimm parameters from board specific file\n");
#endif
	case STEP_COMPUTE_COMMON_PARMS:
		/*
		 * STEP 3: Compute a common set of timing parameters
		 * suitable for all of the DIMMs on each memory controller
		 */
		for (i = first_ctrl; i <= last_ctrl; i++) {
			debug("Computing lowest common DIMM"
				" parameters for memctl=%u\n", i);
			compute_lowest_common_dimm_parameters
				(i,
				 pinfo->dimm_params[i],
				 &timing_params[i],
				 CONFIG_DIMM_SLOTS_PER_CTLR);
		}

	case STEP_GATHER_OPTS:
		/* STEP 4:  Gather configuration requirements from user */
		for (i = first_ctrl; i <= last_ctrl; i++) {
			debug("Reloading memory controller "
				"configuration options for memctl=%u\n", i);
			/*
			 * This "reloads" the memory controller options
			 * to defaults.  If the user "edits" an option,
			 * next_step points to the step after this,
			 * which is currently STEP_ASSIGN_ADDRESSES.
			 */
			populate_memctl_options(
					&timing_params[i],
					&pinfo->memctl_opts[i],
					pinfo->dimm_params[i], i);
			/*
			 * For RDIMMs, JEDEC spec requires clocks to be stable
			 * before reset signal is deasserted. For the boards
			 * using fixed parameters, this function should be
			 * be called from board init file.
			 */
			if (timing_params[i].all_dimms_registered)
				assert_reset = 1;
		}
		if (assert_reset && !size_only) {
			if (pinfo->board_mem_reset) {
				debug("Asserting mem reset\n");
				pinfo->board_mem_reset();
			} else {
				debug("Asserting mem reset missing\n");
			}
		}

	case STEP_ASSIGN_ADDRESSES:
		/* STEP 5:  Assign addresses to chip selects */
		check_interleaving_options(pinfo);
		total_mem = step_assign_addresses(pinfo, dbw_capacity_adjust);
		debug("Total mem %llu assigned\n", total_mem);

	case STEP_COMPUTE_REGS:
		/* STEP 6:  compute controller register values */
		debug("FSL Memory ctrl register computation\n");
		for (i = first_ctrl; i <= last_ctrl; i++) {
			if (timing_params[i].ndimms_present == 0) {
				memset(&ddr_reg[i], 0,
					sizeof(fsl_ddr_cfg_regs_t));
				continue;
			}

			compute_fsl_memctl_config_regs
				(i,
				 &pinfo->memctl_opts[i],
				 &ddr_reg[i], &timing_params[i],
				 pinfo->dimm_params[i],
				 dbw_capacity_adjust[i],
				 size_only);
		}

	default:
		break;
	}

	{
		/*
		 * Compute the amount of memory available just by
		 * looking for the highest valid CSn_BNDS value.
		 * This allows us to also experiment with using
		 * only CS0 when using dual-rank DIMMs.
		 */
		unsigned int max_end = 0;

		for (i = first_ctrl; i <= last_ctrl; i++) {
			for (j = 0; j < CONFIG_CHIP_SELECTS_PER_CTRL; j++) {
				fsl_ddr_cfg_regs_t *reg = &ddr_reg[i];
				if (reg->cs[j].config & 0x80000000) {
					unsigned int end;
					/*
					 * 0xfffffff is a special value we put
					 * for unused bnds
					 */
					if (reg->cs[j].bnds == 0xffffffff)
						continue;
					end = reg->cs[j].bnds & 0xffff;
					if (end > max_end) {
						max_end = end;
					}
				}
			}
		}

		total_mem = 1 + (((unsigned long long)max_end << 24ULL) |
			    0xFFFFFFULL) - pinfo->mem_base;
	}

	return total_mem;
}

phys_size_t __fsl_ddr_sdram(fsl_ddr_info_t *pinfo)
{
	unsigned int i, first_ctrl, last_ctrl;
#ifdef CONFIG_PPC
	unsigned int law_memctl = LAW_TRGT_IF_DDR_1;
#endif
	unsigned long long total_memory;
	int deassert_reset = 0;

	first_ctrl = pinfo->first_ctrl;
	last_ctrl = first_ctrl + pinfo->num_ctrls - 1;

	/* Compute it once normally. */
#ifdef CONFIG_FSL_DDR_INTERACTIVE
	if (tstc() && (getc() == 'd')) {	/* we got a key press of 'd' */
		total_memory = fsl_ddr_interactive(pinfo, 0);
	} else if (fsl_ddr_interactive_env_var_exists()) {
		total_memory = fsl_ddr_interactive(pinfo, 1);
	} else
#endif
		total_memory = fsl_ddr_compute(pinfo, STEP_GET_SPD, 0);

	/* setup 3-way interleaving before enabling DDRC */
	switch (pinfo->memctl_opts[first_ctrl].memctl_interleaving_mode) {
	case FSL_DDR_3WAY_1KB_INTERLEAVING:
	case FSL_DDR_3WAY_4KB_INTERLEAVING:
	case FSL_DDR_3WAY_8KB_INTERLEAVING:
		fsl_ddr_set_intl3r(
			pinfo->memctl_opts[first_ctrl].
			memctl_interleaving_mode);
		break;
	default:
		break;
	}

	/*
	 * Program configuration registers.
	 * JEDEC specs requires clocks to be stable before deasserting reset
	 * for RDIMMs. Clocks start after chip select is enabled and clock
	 * control register is set. During step 1, all controllers have their
	 * registers set but not enabled. Step 2 proceeds after deasserting
	 * reset through board FPGA or GPIO.
	 * For non-registered DIMMs, initialization can go through but it is
	 * also OK to follow the same flow.
	 */
	if (pinfo->board_need_mem_reset)
		deassert_reset = pinfo->board_need_mem_reset();
	for (i = first_ctrl; i <= last_ctrl; i++) {
		if (pinfo->common_timing_params[i].all_dimms_registered)
			deassert_reset = 1;
	}
	for (i = first_ctrl; i <= last_ctrl; i++) {
		debug("Programming controller %u\n", i);
		if (pinfo->common_timing_params[i].ndimms_present == 0) {
			debug("No dimms present on controller %u; "
					"skipping programming\n", i);
			continue;
		}
		/*
		 * The following call with step = 1 returns before enabling
		 * the controller. It has to finish with step = 2 later.
		 */
		fsl_ddr_set_memctl_regs(&(pinfo->fsl_ddr_config_reg[i]), i,
					deassert_reset ? 1 : 0);
	}
	if (deassert_reset) {
		/* Use board FPGA or GPIO to deassert reset signal */
		if (pinfo->board_mem_de_reset) {
			debug("Deasserting mem reset\n");
			pinfo->board_mem_de_reset();
		} else {
			debug("Deasserting mem reset missing\n");
		}
		for (i = first_ctrl; i <= last_ctrl; i++) {
			/* Call with step = 2 to continue initialization */
			fsl_ddr_set_memctl_regs(&(pinfo->fsl_ddr_config_reg[i]),
						i, 2);
		}
	}

#ifdef CONFIG_FSL_DDR_SYNC_REFRESH
	fsl_ddr_sync_memctl_refresh(first_ctrl, last_ctrl);
#endif

#ifdef CONFIG_PPC
	/* program LAWs */
	for (i = first_ctrl; i <= last_ctrl; i++) {
		if (pinfo->memctl_opts[i].memctl_interleaving) {
			switch (pinfo->memctl_opts[i].
				memctl_interleaving_mode) {
			case FSL_DDR_CACHE_LINE_INTERLEAVING:
			case FSL_DDR_PAGE_INTERLEAVING:
			case FSL_DDR_BANK_INTERLEAVING:
			case FSL_DDR_SUPERBANK_INTERLEAVING:
				if (i % 2)
					break;
				if (i == 0) {
					law_memctl = LAW_TRGT_IF_DDR_INTRLV;
					fsl_ddr_set_lawbar(
						&pinfo->common_timing_params[i],
						law_memctl, i);
				}
#if CONFIG_SYS_NUM_DDR_CTLRS > 3
				else if (i == 2) {
					law_memctl = LAW_TRGT_IF_DDR_INTLV_34;
					fsl_ddr_set_lawbar(
						&pinfo->common_timing_params[i],
						law_memctl, i);
				}
#endif
				break;
			case FSL_DDR_3WAY_1KB_INTERLEAVING:
			case FSL_DDR_3WAY_4KB_INTERLEAVING:
			case FSL_DDR_3WAY_8KB_INTERLEAVING:
				law_memctl = LAW_TRGT_IF_DDR_INTLV_123;
				if (i == 0) {
					fsl_ddr_set_lawbar(
						&pinfo->common_timing_params[i],
						law_memctl, i);
				}
				break;
			case FSL_DDR_4WAY_1KB_INTERLEAVING:
			case FSL_DDR_4WAY_4KB_INTERLEAVING:
			case FSL_DDR_4WAY_8KB_INTERLEAVING:
				law_memctl = LAW_TRGT_IF_DDR_INTLV_1234;
				if (i == 0)
					fsl_ddr_set_lawbar(
						&pinfo->common_timing_params[i],
						law_memctl, i);
				/* place holder for future 4-way interleaving */
				break;
			default:
				break;
			}
		} else {
			switch (i) {
			case 0:
				law_memctl = LAW_TRGT_IF_DDR_1;
				break;
			case 1:
				law_memctl = LAW_TRGT_IF_DDR_2;
				break;
			case 2:
				law_memctl = LAW_TRGT_IF_DDR_3;
				break;
			case 3:
				law_memctl = LAW_TRGT_IF_DDR_4;
				break;
			default:
				break;
			}
			fsl_ddr_set_lawbar(&pinfo->common_timing_params[i],
					   law_memctl, i);
		}
	}
#endif

	debug("total_memory by %s = %llu\n", __func__, total_memory);

#if !defined(CONFIG_PHYS_64BIT)
	/* Check for 4G or more.  Bad. */
	if ((first_ctrl == 0) && (total_memory >= (1ull << 32))) {
		puts("Detected ");
		print_size(total_memory, " of memory\n");
		printf("       This U-Boot only supports < 4G of DDR\n");
		printf("       You could rebuild it with CONFIG_PHYS_64BIT\n");
		printf("       "); /* re-align to match init_dram print */
		total_memory = CONFIG_MAX_MEM_MAPPED;
	}
#endif

	return total_memory;
}

/*
 * fsl_ddr_sdram(void) -- this is the main function to be
 * called by dram_init() in the board file.
 *
 * It returns amount of memory configured in bytes.
 */
phys_size_t fsl_ddr_sdram(void)
{
	fsl_ddr_info_t info;

	/* Reset info structure. */
	memset(&info, 0, sizeof(fsl_ddr_info_t));
	info.mem_base = CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY;
	info.first_ctrl = 0;
	info.num_ctrls = CONFIG_SYS_FSL_DDR_MAIN_NUM_CTRLS;
	info.dimm_slots_per_ctrl = CONFIG_DIMM_SLOTS_PER_CTLR;
	info.board_need_mem_reset = board_need_mem_reset;
	info.board_mem_reset = board_assert_mem_reset;
	info.board_mem_de_reset = board_deassert_mem_reset;
	remove_unused_controllers(&info);

	return __fsl_ddr_sdram(&info);
}

#ifdef CONFIG_SYS_FSL_OTHER_DDR_NUM_CTRLS
phys_size_t fsl_other_ddr_sdram(unsigned long long base,
				unsigned int first_ctrl,
				unsigned int num_ctrls,
				unsigned int dimm_slots_per_ctrl,
				int (*board_need_reset)(void),
				void (*board_reset)(void),
				void (*board_de_reset)(void))
{
	fsl_ddr_info_t info;

	/* Reset info structure. */
	memset(&info, 0, sizeof(fsl_ddr_info_t));
	info.mem_base = base;
	info.first_ctrl = first_ctrl;
	info.num_ctrls = num_ctrls;
	info.dimm_slots_per_ctrl = dimm_slots_per_ctrl;
	info.board_need_mem_reset = board_need_reset;
	info.board_mem_reset = board_reset;
	info.board_mem_de_reset = board_de_reset;

	return __fsl_ddr_sdram(&info);
}
#endif

/*
 * fsl_ddr_sdram_size(first_ctrl, last_intlv) - This function only returns the
 * size of the total memory without setting ddr control registers.
 */
phys_size_t
fsl_ddr_sdram_size(void)
{
	fsl_ddr_info_t  info;
	unsigned long long total_memory = 0;

	memset(&info, 0 , sizeof(fsl_ddr_info_t));
	info.mem_base = CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY;
	info.first_ctrl = 0;
	info.num_ctrls = CONFIG_SYS_FSL_DDR_MAIN_NUM_CTRLS;
	info.dimm_slots_per_ctrl = CONFIG_DIMM_SLOTS_PER_CTLR;
	info.board_need_mem_reset = NULL;
	remove_unused_controllers(&info);

	/* Compute it once normally. */
	total_memory = fsl_ddr_compute(&info, STEP_GET_SPD, 1);

	return total_memory;
}
