// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#include <i2c.h>
#include <ram.h>

#include <mach/octeon_ddr.h>

#define DEVICE_TYPE	DDR4_SPD_KEY_BYTE_DEVICE_TYPE // same for DDR3 and DDR4
#define MODULE_TYPE	DDR4_SPD_KEY_BYTE_MODULE_TYPE // same for DDR3 and DDR4
#define BUS_WIDTH(t)	(((t) == DDR4_DRAM) ?		    \
			 DDR4_SPD_MODULE_MEMORY_BUS_WIDTH : \
			 DDR3_SPD_MEMORY_BUS_WIDTH)

/*
 * Allow legacy code to encode bus number in the upper bits of the address
 * These are only supported in read_spd()
 */
#define OCTEON_TWSI_BUS_IN_ADDR_BIT       12
#define OCTEON_TWSI_BUS_IN_ADDR_MASK      (15 << OCTEON_TWSI_BUS_IN_ADDR_BIT)
#define OCTEON_TWSI_GET_BUS(addr)			\
	(((addr) >> OCTEON_TWSI_BUS_IN_ADDR_BIT) & 0xf)

const char *ddr3_dimm_types[] = {
	/* 0000 */ "Undefined",
	/* 0001 */ "RDIMM",
	/* 0010 */ "UDIMM",
	/* 0011 */ "SO-DIMM",
	/* 0100 */ "Micro-DIMM",
	/* 0101 */ "Mini-RDIMM",
	/* 0110 */ "Mini-UDIMM",
	/* 0111 */ "Mini-CDIMM",
	/* 1000 */ "72b-SO-UDIMM",
	/* 1001 */ "72b-SO-RDIMM",
	/* 1010 */ "72b-SO-CDIMM"
	/* 1011 */ "LRDIMM",
	/* 1100 */ "16b-SO-DIMM",
	/* 1101 */ "32b-SO-DIMM",
	/* 1110 */ "Reserved",
	/* 1111 */ "Reserved"
};

const char *ddr4_dimm_types[] = {
	/* 0000 */ "Extended",
	/* 0001 */ "RDIMM",
	/* 0010 */ "UDIMM",
	/* 0011 */ "SO-DIMM",
	/* 0100 */ "LRDIMM",
	/* 0101 */ "Mini-RDIMM",
	/* 0110 */ "Mini-UDIMM",
	/* 0111 */ "Reserved",
	/* 1000 */ "72b-SO-RDIMM",
	/* 1001 */ "72b-SO-UDIMM",
	/* 1010 */ "Reserved",
	/* 1011 */ "Reserved",
	/* 1100 */ "16b-SO-DIMM",
	/* 1101 */ "32b-SO-DIMM",
	/* 1110 */ "Reserved",
	/* 1111 */ "Reserved"
};

static u16 ddr3_crc16(u8 *ptr, int count)
{
	/* From DDR3 SPD specification */
	int crc, i;

	crc = 0;
	while (--count >= 0) {
		crc = crc ^ (int)*ptr++ << 8;
		for (i = 0; i < 8; ++i) {
			if (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			else
				crc = crc << 1;
		}
	}

	return (crc & 0xFFFF);
}

static int validate_spd_checksum_ddr4(struct dimm_config *dimm_config,
				      int dimm_index, int twsi_addr, int silent)
{
	u8 *spd_data = dimm_config->spd_data[dimm_index];
	int crc_bytes = 126;
	u16 crc_comp;

	/* Check byte 0 to see how many bytes checksum is over */
	if (spd_data[0] & 0x80)
		crc_bytes = 117;

	crc_comp = ddr3_crc16(spd_data, crc_bytes);

	if (spd_data[126] == (crc_comp & 0xff) &&
	    spd_data[127] == (crc_comp >> 8))
		return 1;

	if (!silent) {
		printf("DDR4 SPD CRC error, spd addr: 0x%x, calculated crc: 0x%04x, read crc: 0x%02x%02x\n",
		       twsi_addr, crc_comp, spd_data[127], spd_data[126]);
	}

	return 0;
}

static int validate_spd_checksum(struct ddr_priv *priv,
				 struct dimm_config *dimm_config,
				 int dimm_index, int twsi_addr,
				 int silent, u8 rv)
{
	if (ddr_verbose(priv))
		debug("Validating DIMM at address 0x%x\n", twsi_addr);

	if (rv >= 0x8 && rv <= 0xA)
		printf("%s: Error: DDR2 support disabled\n", __func__);

	if (rv == 0xB)
		printf("%s: Error: DDR3 support disabled\n", __func__);

	if (rv == 0xC) {
		return validate_spd_checksum_ddr4(dimm_config, dimm_index,
						  twsi_addr, silent);
	}

	if (!silent) {
		printf("Unrecognized DIMM type: 0x%x at spd address: 0x%x\n",
		       rv, twsi_addr);
	}

	return 0;
}

/*
 * Read an DIMM SPD value, either using TWSI to read it from the DIMM, or
 * from a provided array.
 */
int read_spd(struct dimm_config *dimm_config, int dimm_index, int spd_field)
{
	dimm_index = !!dimm_index;

	if (spd_field >= SPD_EEPROM_SIZE) {
		printf("ERROR: Trying to read unsupported SPD EEPROM value %d\n",
		       spd_field);
	}

	/*
	 * If pointer to data is provided, use it, otherwise read from SPD
	 * over twsi
	 */
	if (dimm_config->spd_ptrs[dimm_index])
		return dimm_config->spd_ptrs[dimm_index][spd_field];
	else if (dimm_config->spd_addrs[dimm_index])
		return dimm_config->spd_data[dimm_index][spd_field];

	return -1;
}

int read_spd_init(struct dimm_config *dimm_config, int dimm_index)
{
	u8 busno = OCTEON_TWSI_GET_BUS(dimm_config->spd_addrs[dimm_index]);
	u8 cmdno = dimm_config->spd_addrs[dimm_index];
	struct udevice *dev_i2c;
	u8 *spd_data;
	int ret;

	if (dimm_config->spd_cached[dimm_index])
		return 0;

	dimm_config->spd_cached[dimm_index] = 1;
	spd_data = dimm_config->spd_data[dimm_index];

	ret = i2c_get_chip_for_busnum(busno, cmdno, 2, &dev_i2c);
	if (ret) {
		debug("Cannot find SPL EEPROM: %d\n", ret);
		return -ENODEV;
	}

	ret = dm_i2c_read(dev_i2c, 0, spd_data, SPD_EEPROM_SIZE);

	return ret;
}

int validate_dimm(struct ddr_priv *priv, struct dimm_config *dimm_config,
		  int dimm_index)
{
	int spd_addr;

	dimm_index = !!dimm_index;  /* Normalize to 0/1 */
	spd_addr = dimm_config->spd_addrs[dimm_index];

	debug("Validating dimm %d, spd addr: 0x%02x spd ptr: %p\n",
	      dimm_index,
	      dimm_config->spd_addrs[dimm_index],
	      dimm_config->spd_ptrs[dimm_index]);

	/* Only validate 'real' dimms, assume compiled in values are OK */
	if (!dimm_config->spd_ptrs[dimm_index]) {
		int val0, val1;
		int dimm_type;
		int ret;

		ret = read_spd_init(dimm_config, dimm_index);
		if (ret)
			return 0;

		dimm_type = read_spd(dimm_config, dimm_index,
				     DDR2_SPD_MEM_TYPE) & 0xff;
		switch (dimm_type) {
		case 0x0B:              /* DDR3 */
			if (ddr_verbose(priv))
				printf("Validating DDR3 DIMM %d\n", dimm_index);
			val0 = read_spd(dimm_config, dimm_index,
					DDR3_SPD_DENSITY_BANKS);
			val1 = read_spd(dimm_config, dimm_index,
					DDR3_SPD_ADDRESSING_ROW_COL_BITS);
			if (val0 < 0 && val1 < 0) {
				if (ddr_verbose(priv))
					printf("Error reading SPD for DIMM %d\n",
					       dimm_index);
				return 0; /* Failed to read dimm */
			}
			if (val0 == 0xff && val1 == 0xff) {
				if (ddr_verbose(priv))
					printf("Blank or unreadable SPD for DIMM %d\n",
					       dimm_index);
				/* Blank SPD or otherwise unreadable device */
				return 0;
			}

			/* Don't treat bad checksums as fatal */
			validate_spd_checksum(priv, dimm_config, dimm_index,
					      spd_addr, 0, dimm_type);
			break;

		case 0x0C:              /* DDR4 */
			if (ddr_verbose(priv))
				printf("Validating DDR4 DIMM %d\n", dimm_index);
			val0 = read_spd(dimm_config, dimm_index,
					DDR4_SPD_DENSITY_BANKS);
			val1 = read_spd(dimm_config, dimm_index,
					DDR4_SPD_ADDRESSING_ROW_COL_BITS);
			if (val0 < 0 && val1 < 0) {
				if (ddr_verbose(priv))
					printf("Error reading SPD for DIMM %d\n",
					       dimm_index);
				return 0; /* Failed to read dimm */
			}
			if (val0 == 0xff && val1 == 0xff) {
				if (ddr_verbose(priv)) {
					printf("Blank or unreadable SPD for DIMM %d\n",
					       dimm_index);
				}
				/* Blank SPD or otherwise unreadable device */
				return 0;
			}

			/* Don't treat bad checksums as fatal */
			validate_spd_checksum(priv, dimm_config, dimm_index,
					      spd_addr, 0, dimm_type);
			break;

		case 0x00:
			/* Terminator detected. Fail silently. */
			return 0;

		default:
			debug("Unknown DIMM type 0x%x for DIMM %d @ 0x%x\n",
			      dimm_type, dimm_index,
			      dimm_config->spd_addrs[dimm_index]);
			return 0;      /* Failed to read dimm */
		}
	}

	return 1;
}

int get_ddr_type(struct dimm_config *dimm_config, int upper_dimm)
{
	int spd_ddr_type;

	spd_ddr_type = read_spd(dimm_config, upper_dimm, DEVICE_TYPE);

	debug("%s:%d spd_ddr_type=0x%02x\n", __func__, __LINE__,
	      spd_ddr_type);

	/* we return only DDR4 or DDR3 */
	return (spd_ddr_type == 0x0C) ? DDR4_DRAM : DDR3_DRAM;
}

static int get_dimm_ecc(struct dimm_config *dimm_config, int upper_dimm,
			int ddr_type)
{
	return !!(read_spd(dimm_config, upper_dimm, BUS_WIDTH(ddr_type)) & 8);
}

int get_dimm_module_type(struct dimm_config *dimm_config, int upper_dimm,
			 int ddr_type)
{
	return read_spd(dimm_config, upper_dimm, MODULE_TYPE) & 0x0f;
}

char *printable_rank_spec(char *buffer, int num_ranks, int dram_width,
			  int spd_package)
{
	int die_count = ((spd_package >> 4) & 7) + 1;

	if (spd_package & 0x80) { // non-monolithic
		if ((spd_package & 3) == 2) { // 3DS
			sprintf(buffer, "%dS%dRx%d", num_ranks, die_count,
				dram_width);
		} else { // MLS
			char hchar = (die_count == 2) ? 'D' : 'Q';

			sprintf(buffer, "%d%cRx%d", num_ranks, hchar,
				dram_width);
		}
	} else {
		sprintf(buffer, "%dRx%d", num_ranks, dram_width);
	}

	return buffer;
}

static void report_common_dimm(struct dimm_config *dimm_config, int upper_dimm,
			       int dimm, const char **dimm_types, int ddr_type,
			       char *volt_str, int if_num,
			       int num_ranks, int dram_width, int spd_package)
{
	unsigned int spd_module_type;
	char rank_spec[8];
	int spd_ecc;

	spd_module_type = get_dimm_module_type(dimm_config, upper_dimm,
					       ddr_type);
	spd_ecc = get_dimm_ecc(dimm_config, upper_dimm, ddr_type);

	printable_rank_spec(rank_spec, num_ranks, dram_width, spd_package);
	printf("LMC%d.DIMM%d: DDR%d %s %s %s, %s\n",
	       if_num, dimm, ddr_type, dimm_types[spd_module_type],
	       rank_spec, spd_ecc ? "ECC" : "non-ECC", volt_str);
}

static void report_ddr3_dimm(struct dimm_config *dimm_config, int upper_dimm,
			     int dimm, int if_num)
{
	int spd_voltage;
	char *volt_str;
	int spd_org = read_spd(dimm_config, upper_dimm,
			       DDR3_SPD_MODULE_ORGANIZATION);
	int num_ranks = 1 +  ((spd_org >> 3) & 0x7);
	int dram_width = 4 << ((spd_org >> 0) & 0x7);

	spd_voltage = read_spd(dimm_config, upper_dimm,
			       DDR3_SPD_NOMINAL_VOLTAGE);
	if (spd_voltage == 0 || spd_voltage & 3)
		volt_str = "1.5V";
	if (spd_voltage & 2)
		volt_str = "1.35V";
	if (spd_voltage & 4)
		volt_str = "1.2xV";

	report_common_dimm(dimm_config, upper_dimm, dimm, ddr3_dimm_types,
			   DDR3_DRAM, volt_str, if_num,
			   num_ranks, dram_width, /*spd_package*/0);
}

static void report_ddr4_dimm(struct dimm_config *dimm_config, int upper_dimm,
			     int dimm, int if_num)
{
	int spd_voltage;
	char *volt_str;
	int spd_package = 0xff & read_spd(dimm_config, upper_dimm,
					  DDR4_SPD_PACKAGE_TYPE);
	int spd_org     = 0xff & read_spd(dimm_config, upper_dimm,
					  DDR4_SPD_MODULE_ORGANIZATION);
	int num_ranks   = 1 +  ((spd_org >> 3) & 0x7);
	int dram_width  = 4 << ((spd_org >> 0) & 0x7);

	spd_voltage = read_spd(dimm_config, upper_dimm,
			       DDR4_SPD_MODULE_NOMINAL_VOLTAGE);
	if (spd_voltage == 0x01 || spd_voltage & 0x02)
		volt_str = "1.2V";
	if (spd_voltage == 0x04 || spd_voltage & 0x08)
		volt_str = "TBD1 V";
	if (spd_voltage == 0x10 || spd_voltage & 0x20)
		volt_str = "TBD2 V";

	report_common_dimm(dimm_config, upper_dimm, dimm, ddr4_dimm_types,
			   DDR4_DRAM, volt_str, if_num,
			   num_ranks, dram_width, spd_package);
}

void report_dimm(struct dimm_config *dimm_config, int upper_dimm,
		 int dimm, int if_num)
{
	int ddr_type;

	/* ddr_type only indicates DDR4 or DDR3 */
	ddr_type = get_ddr_type(dimm_config, upper_dimm);

	if (ddr_type == DDR4_DRAM)
		report_ddr4_dimm(dimm_config, 0, dimm, if_num);
	else
		report_ddr3_dimm(dimm_config, 0, dimm, if_num);
}
