// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2018 NXP Semiconductor
 *
 * calculate the organization and timing parameter
 * from ddr3 spd, please refer to the spec
 * JEDEC standard No.21-C 4_01_02_12R23A.pdf
 *
 *
 */

#include <common.h>
#include <fsl_ddr_sdram.h>

#include <fsl_ddr.h>

/*
 * Calculate the Density of each Physical Rank.
 * Returned size is in bytes.
 *
 * Total DIMM size =
 * sdram capacity(bit) / 8 * primary bus width / sdram width
 *                     * Logical Ranks per DIMM
 *
 * where: sdram capacity  = spd byte4[3:0]
 *        primary bus width = spd byte13[2:0]
 *        sdram width = spd byte12[2:0]
 *        Logical Ranks per DIMM = spd byte12[5:3] for SDP, DDP, QDP
 *                                 spd byte12{5:3] * spd byte6[6:4] for 3DS
 *
 * To simplify each rank size = total DIMM size / Number of Package Ranks
 * where Number of Package Ranks = spd byte12[5:3]
 *
 * SPD byte4 - sdram density and banks
 *	bit[3:0]	size(bit)	size(byte)
 *	0000		256Mb		32MB
 *	0001		512Mb		64MB
 *	0010		1Gb		128MB
 *	0011		2Gb		256MB
 *	0100		4Gb		512MB
 *	0101		8Gb		1GB
 *	0110		16Gb		2GB
 *      0111		32Gb		4GB
 *
 * SPD byte13 - module memory bus width
 *	bit[2:0]	primary bus width
 *	000		8bits
 *	001		16bits
 *	010		32bits
 *	011		64bits
 *
 * SPD byte12 - module organization
 *	bit[2:0]	sdram device width
 *	000		4bits
 *	001		8bits
 *	010		16bits
 *	011		32bits
 *
 * SPD byte12 - module organization
 *	bit[5:3]	number of package ranks per DIMM
 *	000		1
 *	001		2
 *	010		3
 *	011		4
 *
 * SPD byte6 - SDRAM package type
 *	bit[6:4]	Die count
 *	000		1
 *	001		2
 *	010		3
 *	011		4
 *	100		5
 *	101		6
 *	110		7
 *	111		8
 *
 * SPD byte6 - SRAM package type
 *	bit[1:0]	Signal loading
 *	00		Not specified
 *	01		Multi load stack
 *	10		Sigle load stack (3DS)
 *	11		Reserved
 */
static unsigned long long
compute_ranksize(const struct ddr4_spd_eeprom_s *spd)
{
	unsigned long long bsize;

	int nbit_sdram_cap_bsize = 0;
	int nbit_primary_bus_width = 0;
	int nbit_sdram_width = 0;
	int die_count = 0;
	bool package_3ds;

	if ((spd->density_banks & 0xf) <= 7)
		nbit_sdram_cap_bsize = (spd->density_banks & 0xf) + 28;
	if ((spd->bus_width & 0x7) < 4)
		nbit_primary_bus_width = (spd->bus_width & 0x7) + 3;
	if ((spd->organization & 0x7) < 4)
		nbit_sdram_width = (spd->organization & 0x7) + 2;
	package_3ds = (spd->package_type & 0x3) == 0x2;
	if ((spd->package_type & 0x80) && !package_3ds) { /* other than 3DS */
		printf("Warning: not supported SDRAM package type\n");
		return 0;
	}
	if (package_3ds)
		die_count = (spd->package_type >> 4) & 0x7;

	bsize = 1ULL << (nbit_sdram_cap_bsize - 3 +
			 nbit_primary_bus_width - nbit_sdram_width +
			 die_count);

	debug("DDR: DDR rank density = 0x%16llx\n", bsize);

	return bsize;
}

#define spd_to_ps(mtb, ftb)	\
	(mtb * pdimm->mtb_ps + (ftb * pdimm->ftb_10th_ps) / 10)
/*
 * ddr_compute_dimm_parameters for DDR4 SPD
 *
 * Compute DIMM parameters based upon the SPD information in spd.
 * Writes the results to the dimm_params_t structure pointed by pdimm.
 *
 */
unsigned int ddr_compute_dimm_parameters(const unsigned int ctrl_num,
					 const generic_spd_eeprom_t *spd,
					 dimm_params_t *pdimm,
					 unsigned int dimm_number)
{
	unsigned int retval;
	int i;
	const u8 udimm_rc_e_dq[18] = {
		0x0c, 0x2c, 0x15, 0x35, 0x15, 0x35, 0x0b, 0x2c, 0x15,
		0x35, 0x0b, 0x35, 0x0b, 0x2c, 0x0b, 0x35, 0x15, 0x36
	};
	int spd_error = 0;
	u8 *ptr;
	u8 val;

	if (spd->mem_type) {
		if (spd->mem_type != SPD_MEMTYPE_DDR4) {
			printf("Ctrl %u DIMM %u: is not a DDR4 SPD.\n",
			       ctrl_num, dimm_number);
			return 1;
		}
	} else {
		memset(pdimm, 0, sizeof(dimm_params_t));
		return 1;
	}

	retval = ddr4_spd_check(spd);
	if (retval) {
		printf("DIMM %u: failed checksum\n", dimm_number);
		return 2;
	}

	/*
	 * The part name in ASCII in the SPD EEPROM is not null terminated.
	 * Guarantee null termination here by presetting all bytes to 0
	 * and copying the part name in ASCII from the SPD onto it
	 */
	memset(pdimm->mpart, 0, sizeof(pdimm->mpart));
	if ((spd->info_size_crc & 0xF) > 2)
		memcpy(pdimm->mpart, spd->mpart, sizeof(pdimm->mpart) - 1);

	/* DIMM organization parameters */
	pdimm->n_ranks = ((spd->organization >> 3) & 0x7) + 1;
	pdimm->rank_density = compute_ranksize(spd);
	pdimm->capacity = pdimm->n_ranks * pdimm->rank_density;
	pdimm->die_density = spd->density_banks & 0xf;
	pdimm->primary_sdram_width = 1 << (3 + (spd->bus_width & 0x7));
	if ((spd->bus_width >> 3) & 0x3)
		pdimm->ec_sdram_width = 8;
	else
		pdimm->ec_sdram_width = 0;
	pdimm->data_width = pdimm->primary_sdram_width
			  + pdimm->ec_sdram_width;
	pdimm->device_width = 1 << ((spd->organization & 0x7) + 2);
	pdimm->package_3ds = (spd->package_type & 0x3) == 0x2 ?
			     (spd->package_type >> 4) & 0x7 : 0;

	/* These are the types defined by the JEDEC SPD spec */
	pdimm->mirrored_dimm = 0;
	pdimm->registered_dimm = 0;
	switch (spd->module_type & DDR4_SPD_MODULETYPE_MASK) {
	case DDR4_SPD_MODULETYPE_RDIMM:
		/* Registered/buffered DIMMs */
		pdimm->registered_dimm = 1;
		if (spd->mod_section.registered.reg_map & 0x1)
			pdimm->mirrored_dimm = 1;
		val = spd->mod_section.registered.ca_stren;
		pdimm->rcw[3] = val >> 4;
		pdimm->rcw[4] = ((val & 0x3) << 2) | ((val & 0xc) >> 2);
		val = spd->mod_section.registered.clk_stren;
		pdimm->rcw[5] = ((val & 0x3) << 2) | ((val & 0xc) >> 2);
		/* Not all in SPD. For convience only. Boards may overwrite. */
		pdimm->rcw[6] = 0xf;
		/*
		 * A17 only used for 16Gb and above devices.
		 * C[2:0] only used for 3DS.
		 */
		pdimm->rcw[8] = pdimm->die_density >= 0x6 ? 0x0 : 0x8 |
				(pdimm->package_3ds > 0x3 ? 0x0 :
				 (pdimm->package_3ds > 0x1 ? 0x1 :
				  (pdimm->package_3ds > 0 ? 0x2 : 0x3)));
		if (pdimm->package_3ds || pdimm->n_ranks != 4)
			pdimm->rcw[13] = 0xc;
		else
			pdimm->rcw[13] = 0xd;	/* Fix encoded by board */

		break;

	case DDR4_SPD_MODULETYPE_UDIMM:
	case DDR4_SPD_MODULETYPE_SO_DIMM:
		/* Unbuffered DIMMs */
		if (spd->mod_section.unbuffered.addr_mapping & 0x1)
			pdimm->mirrored_dimm = 1;
		if ((spd->mod_section.unbuffered.mod_height & 0xe0) == 0 &&
		    (spd->mod_section.unbuffered.ref_raw_card == 0x04)) {
			/* Fix SPD error found on DIMMs with raw card E0 */
			for (i = 0; i < 18; i++) {
				if (spd->mapping[i] == udimm_rc_e_dq[i])
					continue;
				spd_error = 1;
				debug("SPD byte %d: 0x%x, should be 0x%x\n",
				      60 + i, spd->mapping[i],
				      udimm_rc_e_dq[i]);
				ptr = (u8 *)&spd->mapping[i];
				*ptr = udimm_rc_e_dq[i];
			}
			if (spd_error)
				puts("SPD DQ mapping error fixed\n");
		}
		break;

	default:
		printf("unknown module_type 0x%02X\n", spd->module_type);
		return 1;
	}

	/* SDRAM device parameters */
	pdimm->n_row_addr = ((spd->addressing >> 3) & 0x7) + 12;
	pdimm->n_col_addr = (spd->addressing & 0x7) + 9;
	pdimm->bank_addr_bits = (spd->density_banks >> 4) & 0x3;
	pdimm->bank_group_bits = (spd->density_banks >> 6) & 0x3;

	/*
	 * The SPD spec has not the ECC bit,
	 * We consider the DIMM as ECC capability
	 * when the extension bus exist
	 */
	if (pdimm->ec_sdram_width)
		pdimm->edc_config = 0x02;
	else
		pdimm->edc_config = 0x00;

	/*
	 * The SPD spec has not the burst length byte
	 * but DDR4 spec has nature BL8 and BC4,
	 * BL8 -bit3, BC4 -bit2
	 */
	pdimm->burst_lengths_bitmask = 0x0c;

	/* MTB - medium timebase
	 * The MTB in the SPD spec is 125ps,
	 *
	 * FTB - fine timebase
	 * use 1/10th of ps as our unit to avoid floating point
	 * eg, 10 for 1ps, 25 for 2.5ps, 50 for 5ps
	 */
	if ((spd->timebases & 0xf) == 0x0) {
		pdimm->mtb_ps = 125;
		pdimm->ftb_10th_ps = 10;

	} else {
		printf("Unknown Timebases\n");
	}

	/* sdram minimum cycle time */
	pdimm->tckmin_x_ps = spd_to_ps(spd->tck_min, spd->fine_tck_min);

	/* sdram max cycle time */
	pdimm->tckmax_ps = spd_to_ps(spd->tck_max, spd->fine_tck_max);

	/*
	 * CAS latency supported
	 * bit0 - CL7
	 * bit4 - CL11
	 * bit8 - CL15
	 * bit12- CL19
	 * bit16- CL23
	 */
	pdimm->caslat_x  = (spd->caslat_b1 << 7)	|
			   (spd->caslat_b2 << 15)	|
			   (spd->caslat_b3 << 23);

	BUG_ON(spd->caslat_b4 != 0);

	/*
	 * min CAS latency time
	 */
	pdimm->taa_ps = spd_to_ps(spd->taa_min, spd->fine_taa_min);

	/*
	 * min RAS to CAS delay time
	 */
	pdimm->trcd_ps = spd_to_ps(spd->trcd_min, spd->fine_trcd_min);

	/*
	 * Min Row Precharge Delay Time
	 */
	pdimm->trp_ps = spd_to_ps(spd->trp_min, spd->fine_trp_min);

	/* min active to precharge delay time */
	pdimm->tras_ps = (((spd->tras_trc_ext & 0xf) << 8) +
			  spd->tras_min_lsb) * pdimm->mtb_ps;

	/* min active to actice/refresh delay time */
	pdimm->trc_ps = spd_to_ps((((spd->tras_trc_ext & 0xf0) << 4) +
				   spd->trc_min_lsb), spd->fine_trc_min);
	/* Min Refresh Recovery Delay Time */
	pdimm->trfc1_ps = ((spd->trfc1_min_msb << 8) | (spd->trfc1_min_lsb)) *
		       pdimm->mtb_ps;
	pdimm->trfc2_ps = ((spd->trfc2_min_msb << 8) | (spd->trfc2_min_lsb)) *
		       pdimm->mtb_ps;
	pdimm->trfc4_ps = ((spd->trfc4_min_msb << 8) | (spd->trfc4_min_lsb)) *
			pdimm->mtb_ps;
	/* min four active window delay time */
	pdimm->tfaw_ps = (((spd->tfaw_msb & 0xf) << 8) | spd->tfaw_min) *
			pdimm->mtb_ps;

	/* min row active to row active delay time, different bank group */
	pdimm->trrds_ps = spd_to_ps(spd->trrds_min, spd->fine_trrds_min);
	/* min row active to row active delay time, same bank group */
	pdimm->trrdl_ps = spd_to_ps(spd->trrdl_min, spd->fine_trrdl_min);
	/* min CAS to CAS Delay Time (tCCD_Lmin), same bank group */
	pdimm->tccdl_ps = spd_to_ps(spd->tccdl_min, spd->fine_tccdl_min);

	if (pdimm->package_3ds) {
		if (pdimm->die_density <= 0x4) {
			pdimm->trfc_slr_ps = 260000;
		} else if (pdimm->die_density <= 0x5) {
			pdimm->trfc_slr_ps = 350000;
		} else {
			printf("WARN: Unsupported logical rank density 0x%x\n",
			       pdimm->die_density);
		}
	}

	/*
	 * Average periodic refresh interval
	 * tREFI = 7.8 us at normal temperature range
	 */
	pdimm->refresh_rate_ps = 7800000;

	for (i = 0; i < 18; i++)
		pdimm->dq_mapping[i] = spd->mapping[i];

	pdimm->dq_mapping_ors = ((spd->mapping[0] >> 6) & 0x3) == 0 ? 1 : 0;

	return 0;
}
