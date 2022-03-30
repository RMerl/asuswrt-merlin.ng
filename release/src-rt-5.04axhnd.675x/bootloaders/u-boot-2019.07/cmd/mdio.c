// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 Freescale Semiconductor, Inc
 * Andy Fleming
 */

/*
 * MDIO Commands
 */

#include <common.h>
#include <command.h>
#include <miiphy.h>
#include <phy.h>

static char last_op[2];
static uint last_data;
static uint last_addr_lo;
static uint last_addr_hi;
static uint last_devad_lo;
static uint last_devad_hi;
static uint last_reg_lo;
static uint last_reg_hi;

static int extract_range(char *input, int *plo, int *phi)
{
	char *end;
	*plo = simple_strtol(input, &end, 16);
	if (end == input)
		return -1;

	if ((*end == '-') && *(++end))
		*phi = simple_strtol(end, NULL, 16);
	else if (*end == '\0')
		*phi = *plo;
	else
		return -1;

	return 0;
}

static int mdio_write_ranges(struct mii_dev *bus,
			     int addrlo,
			     int addrhi, int devadlo, int devadhi,
			     int reglo, int reghi, unsigned short data,
			     int extended)
{
	struct phy_device *phydev;
	int addr, devad, reg;
	int err = 0;

	for (addr = addrlo; addr <= addrhi; addr++) {
		phydev = bus->phymap[addr];

		for (devad = devadlo; devad <= devadhi; devad++) {
			for (reg = reglo; reg <= reghi; reg++) {
				if (!phydev)
					err = bus->write(bus, addr, devad,
							 reg, data);
				else if (!extended)
					err = phy_write_mmd(phydev, devad,
							    reg, data);
				else
					err = phydev->drv->writeext(phydev,
							addr, devad, reg, data);

				if (err)
					goto err_out;
			}
		}
	}

err_out:
	return err;
}

static int mdio_read_ranges(struct mii_dev *bus,
			    int addrlo,
			    int addrhi, int devadlo, int devadhi,
			    int reglo, int reghi, int extended)
{
	int addr, devad, reg;
	struct phy_device *phydev;

	printf("Reading from bus %s\n", bus->name);
	for (addr = addrlo; addr <= addrhi; addr++) {
		phydev = bus->phymap[addr];
		printf("PHY at address %x:\n", addr);

		for (devad = devadlo; devad <= devadhi; devad++) {
			for (reg = reglo; reg <= reghi; reg++) {
				int val;

				if (!phydev)
					val = bus->read(bus, addr, devad, reg);
				else if (!extended)
					val = phy_read_mmd(phydev, devad, reg);
				else
					val = phydev->drv->readext(phydev, addr,
						devad, reg);

				if (val < 0) {
					printf("Error\n");

					return val;
				}

				if (devad >= 0)
					printf("%d.", devad);

				printf("%d - 0x%x\n", reg, val & 0xffff);
			}
		}
	}

	return 0;
}

/* The register will be in the form [a[-b].]x[-y] */
static int extract_reg_range(char *input, int *devadlo, int *devadhi,
			     int *reglo, int *reghi)
{
	char *regstr;

	/* use strrchr to find the last string after a '.' */
	regstr = strrchr(input, '.');

	/* If it exists, extract the devad(s) */
	if (regstr) {
		char devadstr[32];

		strncpy(devadstr, input, regstr - input);
		devadstr[regstr - input] = '\0';

		if (extract_range(devadstr, devadlo, devadhi))
			return -1;

		regstr++;
	} else {
		/* Otherwise, we have no devad, and we just got regs */
		*devadlo = *devadhi = MDIO_DEVAD_NONE;

		regstr = input;
	}

	return extract_range(regstr, reglo, reghi);
}

static int extract_phy_range(char *const argv[], int argc, struct mii_dev **bus,
			     struct phy_device **phydev,
			     int *addrlo, int *addrhi)
{
	struct phy_device *dev = *phydev;

	if ((argc < 1) || (argc > 2))
		return -1;

	/* If there are two arguments, it's busname addr */
	if (argc == 2) {
		*bus = miiphy_get_dev_by_name(argv[0]);

		if (!*bus)
			return -1;

		return extract_range(argv[1], addrlo, addrhi);
	}

	/* It must be one argument, here */

	/*
	 * This argument can be one of two things:
	 * 1) Ethernet device name
	 * 2) Just an address (use the previously-used bus)
	 *
	 * We check all buses for a PHY which is connected to an ethernet
	 * device by the given name.  If none are found, we call
	 * extract_range() on the string, and see if it's an address range.
	 */
	dev = mdio_phydev_for_ethname(argv[0]);

	if (dev) {
		*addrlo = *addrhi = dev->addr;
		*bus = dev->bus;

		return 0;
	}

	/* It's an address or nothing useful */
	return extract_range(argv[0], addrlo, addrhi);
}

/* ---------------------------------------------------------------- */
static int do_mdio(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char op[2];
	int addrlo, addrhi, reglo, reghi, devadlo, devadhi;
	unsigned short	data;
	int pos = argc - 1;
	struct mii_dev *bus;
	struct phy_device *phydev = NULL;
	int extended = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */
	op[0] = argv[1][0];
	addrlo = last_addr_lo;
	addrhi = last_addr_hi;
	devadlo = last_devad_lo;
	devadhi = last_devad_hi;
	reglo  = last_reg_lo;
	reghi  = last_reg_hi;
	data   = last_data;

	bus = mdio_get_current_dev();

	if (flag & CMD_FLAG_REPEAT)
		op[0] = last_op[0];

	if (strlen(argv[1]) > 1) {
		op[1] = argv[1][1];
		if (op[1] == 'x') {
			phydev = mdio_phydev_for_ethname(argv[2]);

			if (phydev) {
				addrlo = phydev->addr;
				addrhi = addrlo;
				bus = phydev->bus;
				extended = 1;
			} else {
				return CMD_RET_FAILURE;
			}

			if (!phydev->drv ||
			    (!phydev->drv->writeext && (op[0] == 'w')) ||
			    (!phydev->drv->readext && (op[0] == 'r'))) {
				puts("PHY does not have extended functions\n");
				return CMD_RET_FAILURE;
			}
		}
	}

	switch (op[0]) {
	case 'w':
		if (pos > 1)
			data = simple_strtoul(argv[pos--], NULL, 16);
	case 'r':
		if (pos > 1)
			if (extract_reg_range(argv[pos--], &devadlo, &devadhi,
					      &reglo, &reghi))
				return CMD_RET_FAILURE;

	default:
		if (pos > 1)
			if (extract_phy_range(&argv[2], pos - 1, &bus,
					      &phydev, &addrlo, &addrhi))
				return CMD_RET_FAILURE;

		break;
	}

	if (op[0] == 'l') {
		mdio_list_devices();

		return 0;
	}

	/* Save the chosen bus */
	miiphy_set_current_dev(bus->name);

	switch (op[0]) {
	case 'w':
		mdio_write_ranges(bus, addrlo, addrhi, devadlo, devadhi,
				  reglo, reghi, data, extended);
		break;

	case 'r':
		mdio_read_ranges(bus, addrlo, addrhi, devadlo, devadhi,
				 reglo, reghi, extended);
		break;
	}

	/*
	 * Save the parameters for repeats.
	 */
	last_op[0] = op[0];
	last_addr_lo = addrlo;
	last_addr_hi = addrhi;
	last_devad_lo = devadlo;
	last_devad_hi = devadhi;
	last_reg_lo  = reglo;
	last_reg_hi  = reghi;
	last_data    = data;

	return 0;
}

/***************************************************/

U_BOOT_CMD(
	mdio,	6,	1,	do_mdio,
	"MDIO utility commands",
	"list			- List MDIO buses\n"
	"mdio read <phydev> [<devad>.]<reg> - "
		"read PHY's register at <devad>.<reg>\n"
	"mdio write <phydev> [<devad>.]<reg> <data> - "
		"write PHY's register at <devad>.<reg>\n"
	"mdio rx <phydev> [<devad>.]<reg> - "
		"read PHY's extended register at <devad>.<reg>\n"
	"mdio wx <phydev> [<devad>.]<reg> <data> - "
		"write PHY's extended register at <devad>.<reg>\n"
	"<phydev> may be:\n"
	"   <busname>  <addr>\n"
	"   <addr>\n"
	"   <eth name>\n"
	"<addr> <devad>, and <reg> may be ranges, e.g. 1-5.4-0x1f.\n"
);
