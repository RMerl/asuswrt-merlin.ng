// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
 */

/*
 * MII Utilities
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <miiphy.h>

typedef struct _MII_field_desc_t {
	ushort hi;
	ushort lo;
	ushort mask;
	const char *name;
} MII_field_desc_t;

static const MII_field_desc_t reg_0_desc_tbl[] = {
	{ 15, 15, 0x01, "reset"                        },
	{ 14, 14, 0x01, "loopback"                     },
	{ 13,  6, 0x81, "speed selection"              }, /* special */
	{ 12, 12, 0x01, "A/N enable"                   },
	{ 11, 11, 0x01, "power-down"                   },
	{ 10, 10, 0x01, "isolate"                      },
	{  9,  9, 0x01, "restart A/N"                  },
	{  8,  8, 0x01, "duplex"                       }, /* special */
	{  7,  7, 0x01, "collision test enable"        },
	{  5,  0, 0x3f, "(reserved)"                   }
};

static const MII_field_desc_t reg_1_desc_tbl[] = {
	{ 15, 15, 0x01, "100BASE-T4 able"              },
	{ 14, 14, 0x01, "100BASE-X  full duplex able"  },
	{ 13, 13, 0x01, "100BASE-X  half duplex able"  },
	{ 12, 12, 0x01, "10 Mbps    full duplex able"  },
	{ 11, 11, 0x01, "10 Mbps    half duplex able"  },
	{ 10, 10, 0x01, "100BASE-T2 full duplex able"  },
	{  9,  9, 0x01, "100BASE-T2 half duplex able"  },
	{  8,  8, 0x01, "extended status"              },
	{  7,  7, 0x01, "(reserved)"                   },
	{  6,  6, 0x01, "MF preamble suppression"      },
	{  5,  5, 0x01, "A/N complete"                 },
	{  4,  4, 0x01, "remote fault"                 },
	{  3,  3, 0x01, "A/N able"                     },
	{  2,  2, 0x01, "link status"                  },
	{  1,  1, 0x01, "jabber detect"                },
	{  0,  0, 0x01, "extended capabilities"        },
};

static const MII_field_desc_t reg_2_desc_tbl[] = {
	{ 15,  0, 0xffff, "OUI portion"                },
};

static const MII_field_desc_t reg_3_desc_tbl[] = {
	{ 15, 10, 0x3f, "OUI portion"                },
	{  9,  4, 0x3f, "manufacturer part number"   },
	{  3,  0, 0x0f, "manufacturer rev. number"   },
};

static const MII_field_desc_t reg_4_desc_tbl[] = {
	{ 15, 15, 0x01, "next page able"               },
	{ 14, 14, 0x01, "(reserved)"                   },
	{ 13, 13, 0x01, "remote fault"                 },
	{ 12, 12, 0x01, "(reserved)"                   },
	{ 11, 11, 0x01, "asymmetric pause"             },
	{ 10, 10, 0x01, "pause enable"                 },
	{  9,  9, 0x01, "100BASE-T4 able"              },
	{  8,  8, 0x01, "100BASE-TX full duplex able"  },
	{  7,  7, 0x01, "100BASE-TX able"              },
	{  6,  6, 0x01, "10BASE-T   full duplex able"  },
	{  5,  5, 0x01, "10BASE-T   able"              },
	{  4,  0, 0x1f, "selector"                     },
};

static const MII_field_desc_t reg_5_desc_tbl[] = {
	{ 15, 15, 0x01, "next page able"               },
	{ 14, 14, 0x01, "acknowledge"                  },
	{ 13, 13, 0x01, "remote fault"                 },
	{ 12, 12, 0x01, "(reserved)"                   },
	{ 11, 11, 0x01, "asymmetric pause able"        },
	{ 10, 10, 0x01, "pause able"                   },
	{  9,  9, 0x01, "100BASE-T4 able"              },
	{  8,  8, 0x01, "100BASE-X full duplex able"   },
	{  7,  7, 0x01, "100BASE-TX able"              },
	{  6,  6, 0x01, "10BASE-T full duplex able"    },
	{  5,  5, 0x01, "10BASE-T able"                },
	{  4,  0, 0x1f, "partner selector"             },
};

static const MII_field_desc_t reg_9_desc_tbl[] = {
	{ 15, 13, 0x07, "test mode"		       },
	{ 12, 12, 0x01, "manual master/slave enable"   },
	{ 11, 11, 0x01, "manual master/slave value"    },
	{ 10, 10, 0x01, "multi/single port"            },
	{  9,  9, 0x01, "1000BASE-T full duplex able"  },
	{  8,  8, 0x01, "1000BASE-T half duplex able"  },
	{  7,  7, 0x01, "automatic TDR on link down"   },
	{  6,  6, 0x7f, "(reserved)"                   },
};

static const MII_field_desc_t reg_10_desc_tbl[] = {
	{ 15, 15, 0x01, "master/slave config fault"    },
	{ 14, 14, 0x01, "master/slave config result"   },
	{ 13, 13, 0x01, "local receiver status OK"     },
	{ 12, 12, 0x01, "remote receiver status OK"    },
	{ 11, 11, 0x01, "1000BASE-T full duplex able"  },
	{ 10, 10, 0x01, "1000BASE-T half duplex able"  },
	{  9,  8, 0x03, "(reserved)"                   },
	{  7,  0, 0xff, "1000BASE-T idle error counter"},
};

typedef struct _MII_reg_desc_t {
	ushort regno;
	const MII_field_desc_t *pdesc;
	ushort len;
	const char *name;
} MII_reg_desc_t;

static const MII_reg_desc_t mii_reg_desc_tbl[] = {
	{ MII_BMCR,      reg_0_desc_tbl, ARRAY_SIZE(reg_0_desc_tbl),
		"PHY control register" },
	{ MII_BMSR,      reg_1_desc_tbl, ARRAY_SIZE(reg_1_desc_tbl),
		"PHY status register" },
	{ MII_PHYSID1,   reg_2_desc_tbl, ARRAY_SIZE(reg_2_desc_tbl),
		"PHY ID 1 register" },
	{ MII_PHYSID2,   reg_3_desc_tbl, ARRAY_SIZE(reg_3_desc_tbl),
		"PHY ID 2 register" },
	{ MII_ADVERTISE, reg_4_desc_tbl, ARRAY_SIZE(reg_4_desc_tbl),
		"Autonegotiation advertisement register" },
	{ MII_LPA,       reg_5_desc_tbl, ARRAY_SIZE(reg_5_desc_tbl),
		"Autonegotiation partner abilities register" },
	{ MII_CTRL1000,	 reg_9_desc_tbl, ARRAY_SIZE(reg_9_desc_tbl),
		"1000BASE-T control register" },
	{ MII_STAT1000,	 reg_10_desc_tbl, ARRAY_SIZE(reg_10_desc_tbl),
		"1000BASE-T status register" },
};

static void dump_reg(
	ushort             regval,
	const MII_reg_desc_t *prd);

static bool special_field(ushort regno, const MII_field_desc_t *pdesc,
			  ushort regval);

static void MII_dump(const ushort *regvals, uchar reglo, uchar reghi)
{
	ulong i;

	for (i = 0; i < ARRAY_SIZE(mii_reg_desc_tbl); i++) {
		const uchar reg = mii_reg_desc_tbl[i].regno;

		if (reg >= reglo && reg <= reghi)
			dump_reg(regvals[reg - reglo], &mii_reg_desc_tbl[i]);
	}
}

/* Print out field position, value, name */
static void dump_field(const MII_field_desc_t *pdesc, ushort regval)
{
	if (pdesc->hi == pdesc->lo)
		printf("%2u   ", pdesc->lo);
	else
		printf("%2u-%2u", pdesc->hi, pdesc->lo);

	printf(" = %5u	  %s", (regval >> pdesc->lo) & pdesc->mask,
	       pdesc->name);
}

static void dump_reg(
	ushort             regval,
	const MII_reg_desc_t *prd)
{
	ulong i;
	ushort mask_in_place;
	const MII_field_desc_t *pdesc;

	printf("%u.     (%04hx)                 -- %s --\n",
		prd->regno, regval, prd->name);

	for (i = 0; i < prd->len; i++) {
		pdesc = &prd->pdesc[i];

		mask_in_place = pdesc->mask << pdesc->lo;

		printf("  (%04hx:%04x) %u.",
		       mask_in_place,
		       regval & mask_in_place,
		       prd->regno);

		if (!special_field(prd->regno, pdesc, regval))
			dump_field(pdesc, regval);
		printf("\n");

	}
	printf("\n");
}

/* Special fields:
** 0.6,13
** 0.8
** 2.15-0
** 3.15-0
** 4.4-0
** 5.4-0
*/

static bool special_field(ushort regno, const MII_field_desc_t *pdesc,
			  ushort regval)
{
	const ushort sel_bits = (regval >> pdesc->lo) & pdesc->mask;

	if ((regno == MII_BMCR) && (pdesc->lo == 6)) {
		ushort speed_bits = regval & (BMCR_SPEED1000 | BMCR_SPEED100);
		printf("%2u,%2u =   b%u%u    speed selection = %s Mbps",
			6, 13,
			(regval >>  6) & 1,
			(regval >> 13) & 1,
			speed_bits == BMCR_SPEED1000 ? "1000" :
			speed_bits == BMCR_SPEED100  ? "100" :
			"10");
		return 1;
	}

	else if ((regno == MII_BMCR) && (pdesc->lo == 8)) {
		dump_field(pdesc, regval);
		printf(" = %s", ((regval >> pdesc->lo) & 1) ? "full" : "half");
		return 1;
	}

	else if ((regno == MII_ADVERTISE) && (pdesc->lo == 0)) {
		dump_field(pdesc, regval);
		printf(" = %s",
		       sel_bits == PHY_ANLPAR_PSB_802_3 ? "IEEE 802.3 CSMA/CD" :
		       sel_bits == PHY_ANLPAR_PSB_802_9 ?
		       "IEEE 802.9 ISLAN-16T" : "???");
		return 1;
	}

	else if ((regno == MII_LPA) && (pdesc->lo == 0)) {
		dump_field(pdesc, regval);
		printf(" = %s",
		       sel_bits == PHY_ANLPAR_PSB_802_3 ? "IEEE 802.3 CSMA/CD" :
		       sel_bits == PHY_ANLPAR_PSB_802_9 ?
		       "IEEE 802.9 ISLAN-16T" : "???");
		return 1;
	}

	return 0;
}

static char last_op[2];
static uint last_data;
static uint last_addr_lo;
static uint last_addr_hi;
static uint last_reg_lo;
static uint last_reg_hi;
static uint last_mask;

static void extract_range(
	char * input,
	unsigned char * plo,
	unsigned char * phi)
{
	char * end;
	*plo = hextoul(input, &end);
	if (*end == '-') {
		end++;
		*phi = hextoul(end, NULL);
	}
	else {
		*phi = *plo;
	}
}

/* ---------------------------------------------------------------- */
static int do_mii(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char		op[2];
	unsigned char	addrlo, addrhi, reglo, reghi;
	unsigned char	addr, reg;
	unsigned short	data, mask;
	int		rcode = 0;
	const char	*devname;

	if (argc < 2)
		return CMD_RET_USAGE;

#if defined(CONFIG_MII_INIT)
	mii_init ();
#endif

	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */
	op[0] = last_op[0];
	op[1] = last_op[1];
	addrlo = last_addr_lo;
	addrhi = last_addr_hi;
	reglo  = last_reg_lo;
	reghi  = last_reg_hi;
	data   = last_data;
	mask   = last_mask;

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		op[0] = argv[1][0];
		if (strlen(argv[1]) > 1)
			op[1] = argv[1][1];
		else
			op[1] = '\0';

		if (argc >= 3)
			extract_range(argv[2], &addrlo, &addrhi);
		if (argc >= 4)
			extract_range(argv[3], &reglo, &reghi);
		if (argc >= 5)
			data = hextoul(argv[4], NULL);
		if (argc >= 6)
			mask = hextoul(argv[5], NULL);
	}

	if (addrhi > 31 && strncmp(op, "de", 2)) {
		printf("Incorrect PHY address. Range should be 0-31\n");
		return CMD_RET_USAGE;
	}

	/* use current device */
	devname = miiphy_get_current_dev();

	/*
	 * check info/read/write.
	 */
	if (op[0] == 'i') {
		unsigned char j, start, end;
		unsigned int oui;
		unsigned char model;
		unsigned char rev;

		/*
		 * Look for any and all PHYs.  Valid addresses are 0..31.
		 */
		if (argc >= 3) {
			start = addrlo; end = addrhi;
		} else {
			start = 0; end = 31;
		}

		for (j = start; j <= end; j++) {
			if (miiphy_info (devname, j, &oui, &model, &rev) == 0) {
				printf("PHY 0x%02X: "
					"OUI = 0x%04X, "
					"Model = 0x%02X, "
					"Rev = 0x%02X, "
					"%3dbase%s, %s\n",
					j, oui, model, rev,
					miiphy_speed (devname, j),
					miiphy_is_1000base_x (devname, j)
						? "X" : "T",
					(miiphy_duplex (devname, j) == FULL)
						? "FDX" : "HDX");
			}
		}
	} else if (op[0] == 'r') {
		for (addr = addrlo; addr <= addrhi; addr++) {
			for (reg = reglo; reg <= reghi; reg++) {
				data = 0xffff;
				if (miiphy_read (devname, addr, reg, &data) != 0) {
					printf(
					"Error reading from the PHY addr=%02x reg=%02x\n",
						addr, reg);
					rcode = 1;
				} else {
					if ((addrlo != addrhi) || (reglo != reghi))
						printf("addr=%02x reg=%02x data=",
							(uint)addr, (uint)reg);
					printf("%04X\n", data & 0x0000FFFF);
				}
			}
			if ((addrlo != addrhi) && (reglo != reghi))
				printf("\n");
		}
	} else if (op[0] == 'w') {
		for (addr = addrlo; addr <= addrhi; addr++) {
			for (reg = reglo; reg <= reghi; reg++) {
				if (miiphy_write (devname, addr, reg, data) != 0) {
					printf("Error writing to the PHY addr=%02x reg=%02x\n",
						addr, reg);
					rcode = 1;
				}
			}
		}
	} else if (op[0] == 'm') {
		for (addr = addrlo; addr <= addrhi; addr++) {
			for (reg = reglo; reg <= reghi; reg++) {
				unsigned short val = 0;
				if (miiphy_read(devname, addr,
						reg, &val)) {
					printf("Error reading from the PHY");
					printf(" addr=%02x", addr);
					printf(" reg=%02x\n", reg);
					rcode = 1;
				} else {
					val = (val & ~mask) | (data & mask);
					if (miiphy_write(devname, addr,
							 reg, val)) {
						printf("Error writing to the PHY");
						printf(" addr=%02x", addr);
						printf(" reg=%02x\n", reg);
						rcode = 1;
					}
				}
			}
		}
	} else if (strncmp(op, "du", 2) == 0) {
		ushort regs[MII_STAT1000 + 1];  /* Last reg is 0x0a */
		int ok = 1;
		if (reglo > MII_STAT1000 || reghi > MII_STAT1000) {
			printf("The MII dump command only formats the standard MII registers, 0-5, 9-a.\n");
			return 1;
		}
		for (addr = addrlo; addr <= addrhi; addr++) {
			for (reg = reglo; reg <= reghi; reg++) {
				if (miiphy_read(devname, addr, reg,
						&regs[reg - reglo]) != 0) {
					ok = 0;
					printf(
					"Error reading from the PHY addr=%02x reg=%02x\n",
						addr, reg);
					rcode = 1;
				}
			}
			if (ok)
				MII_dump(regs, reglo, reghi);
			printf("\n");
		}
	} else if (strncmp(op, "de", 2) == 0) {
		if (argc == 2)
			miiphy_listdev ();
		else
			miiphy_set_current_dev (argv[2]);
	} else {
		return CMD_RET_USAGE;
	}

	/*
	 * Save the parameters for repeats.
	 */
	last_op[0] = op[0];
	last_op[1] = op[1];
	last_addr_lo = addrlo;
	last_addr_hi = addrhi;
	last_reg_lo  = reglo;
	last_reg_hi  = reghi;
	last_data    = data;
	last_mask    = mask;

	return rcode;
}

/***************************************************/

U_BOOT_CMD(
	mii, 6, 1, do_mii,
	"MII utility commands",
	"device                            - list available devices\n"
	"mii device <devname>                  - set current device\n"
	"mii info   <addr>                     - display MII PHY info\n"
	"mii read   <addr> <reg>               - read  MII PHY <addr> register <reg>\n"
	"mii write  <addr> <reg> <data>        - write MII PHY <addr> register <reg>\n"
	"mii modify <addr> <reg> <data> <mask> - modify MII PHY <addr> register <reg>\n"
	"                                        updating bits identified in <mask>\n"
	"mii dump   <addr> <reg>               - pretty-print <addr> <reg> (0-5 only)\n"
	"Addr and/or reg may be ranges, e.g. 2-7."
);
