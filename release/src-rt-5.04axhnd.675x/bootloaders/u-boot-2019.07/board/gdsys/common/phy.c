// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#include <common.h>

#include <miiphy.h>

enum {
	MIICMD_SET,
	MIICMD_MODIFY,
	MIICMD_VERIFY_VALUE,
	MIICMD_WAIT_FOR_VALUE,
};

struct mii_setupcmd {
	u8 token;
	u8 reg;
	u16 data;
	u16 mask;
	u32 timeout;
};

/*
 * verify we are talking to a 88e1518
 */
struct mii_setupcmd verify_88e1518[] = {
	{ MIICMD_SET, 22, 0x0000 },
	{ MIICMD_VERIFY_VALUE, 2, 0x0141, 0xffff },
	{ MIICMD_VERIFY_VALUE, 3, 0x0dd0, 0xfff0 },
};

/*
 * workaround for erratum mentioned in 88E1518 release notes
 */
struct mii_setupcmd fixup_88e1518[] = {
	{ MIICMD_SET, 22, 0x00ff },
	{ MIICMD_SET, 17, 0x214b },
	{ MIICMD_SET, 16, 0x2144 },
	{ MIICMD_SET, 17, 0x0c28 },
	{ MIICMD_SET, 16, 0x2146 },
	{ MIICMD_SET, 17, 0xb233 },
	{ MIICMD_SET, 16, 0x214d },
	{ MIICMD_SET, 17, 0xcc0c },
	{ MIICMD_SET, 16, 0x2159 },
	{ MIICMD_SET, 22, 0x0000 },
};

/*
 * default initialization:
 * - set RGMII receive timing to "receive clock transition when data stable"
 * - set RGMII transmit timing to "transmit clock internally delayed"
 * - set RGMII output impedance target to 78,8 Ohm
 * - run output impedance calibration
 * - set autonegotiation advertise to 1000FD only
 */
struct mii_setupcmd default_88e1518[] = {
	{ MIICMD_SET, 22, 0x0002 },
	{ MIICMD_MODIFY, 21, 0x0030, 0x0030 },
	{ MIICMD_MODIFY, 25, 0x0000, 0x0003 },
	{ MIICMD_MODIFY, 24, 0x8000, 0x8000 },
	{ MIICMD_WAIT_FOR_VALUE, 24, 0x4000, 0x4000, 2000 },
	{ MIICMD_SET, 22, 0x0000 },
	{ MIICMD_MODIFY, 4, 0x0000, 0x01e0 },
	{ MIICMD_MODIFY, 9, 0x0200, 0x0300 },
};

/*
 * turn off CLK125 for PHY daughterboard
 */
struct mii_setupcmd ch1fix_88e1518[] = {
	{ MIICMD_SET, 22, 0x0002 },
	{ MIICMD_MODIFY, 16, 0x0006, 0x0006 },
	{ MIICMD_SET, 22, 0x0000 },
};

/*
 * perform copper software reset
 */
struct mii_setupcmd swreset_88e1518[] = {
	{ MIICMD_SET, 22, 0x0000 },
	{ MIICMD_MODIFY, 0, 0x8000, 0x8000 },
	{ MIICMD_WAIT_FOR_VALUE, 0, 0x0000, 0x8000, 2000 },
};

/*
 * special one for 88E1514:
 * Force SGMII to Copper mode
 */
struct mii_setupcmd mii_to_copper_88e1514[] = {
	{ MIICMD_SET, 22, 0x0012 },
	{ MIICMD_MODIFY, 20, 0x0001, 0x0007 },
	{ MIICMD_MODIFY, 20, 0x8000, 0x8000 },
	{ MIICMD_SET, 22, 0x0000 },
};

/*
 * turn off SGMII auto-negotiation
 */
struct mii_setupcmd sgmii_autoneg_off_88e1518[] = {
	{ MIICMD_SET, 22, 0x0001 },
	{ MIICMD_MODIFY, 0, 0x0000, 0x1000 },
	{ MIICMD_MODIFY, 0, 0x8000, 0x8000 },
	{ MIICMD_SET, 22, 0x0000 },
};

/*
 * invert LED2 polarity
 */
struct mii_setupcmd invert_led2_88e1514[] = {
	{ MIICMD_SET, 22, 0x0003 },
	{ MIICMD_MODIFY, 17, 0x0030, 0x0010 },
	{ MIICMD_SET, 22, 0x0000 },
};

static int process_setupcmd(const char *bus, unsigned char addr,
			    struct mii_setupcmd *setupcmd)
{
	int res;
	u8 reg = setupcmd->reg;
	u16 data = setupcmd->data;
	u16 mask = setupcmd->mask;
	u32 timeout = setupcmd->timeout;
	u16 orig_data;
	unsigned long start;

	debug("mii %s:%u reg %2u ", bus, addr, reg);

	switch (setupcmd->token) {
	case MIICMD_MODIFY:
		res = miiphy_read(bus, addr, reg, &orig_data);
		if (res)
			break;
		debug("is %04x. (value %04x mask %04x) ", orig_data, data,
		      mask);
		data = (orig_data & ~mask) | (data & mask);
		/* fallthrough */
	case MIICMD_SET:
		debug("=> %04x\n", data);
		res = miiphy_write(bus, addr, reg, data);
		break;
	case MIICMD_VERIFY_VALUE:
		res = miiphy_read(bus, addr, reg, &orig_data);
		if (res)
			break;
		if ((orig_data & mask) != (data & mask))
			res = -1;
		debug("(value %04x mask %04x) == %04x? %s\n", data, mask,
		      orig_data, res ? "FAIL" : "PASS");
		break;
	case MIICMD_WAIT_FOR_VALUE:
		res = -1;
		start = get_timer(0);
		while ((res != 0) && (get_timer(start) < timeout)) {
			res = miiphy_read(bus, addr, reg, &orig_data);
			if (res)
				continue;
			if ((orig_data & mask) != (data & mask))
				res = -1;
		}
		debug("(value %04x mask %04x) == %04x? %s after %lu ms\n", data,
		      mask, orig_data, res ? "FAIL" : "PASS",
		      get_timer(start));
		break;
	default:
		res = -1;
		break;
	}

	return res;
}

static int process_setup(const char *bus, unsigned char addr,
			    struct mii_setupcmd *setupcmd, unsigned int count)
{
	int res = 0;
	unsigned int k;

	for (k = 0; k < count; ++k) {
		res = process_setupcmd(bus, addr, &setupcmd[k]);
		if (res) {
			printf("mii cmd %u on bus %s addr %u failed, aborting setup\n",
			       setupcmd[k].token, bus, addr);
			break;
		}
	}

	return res;
}

int setup_88e1518(const char *bus, unsigned char addr)
{
	int res;

	res = process_setup(bus, addr,
			    verify_88e1518, ARRAY_SIZE(verify_88e1518));
	if (res)
		return res;

	res = process_setup(bus, addr,
			    fixup_88e1518, ARRAY_SIZE(fixup_88e1518));
	if (res)
		return res;

	res = process_setup(bus, addr,
			    default_88e1518, ARRAY_SIZE(default_88e1518));
	if (res)
		return res;

	if (addr) {
		res = process_setup(bus, addr,
				    ch1fix_88e1518, ARRAY_SIZE(ch1fix_88e1518));
		if (res)
			return res;
	}

	res = process_setup(bus, addr,
			    swreset_88e1518, ARRAY_SIZE(swreset_88e1518));
	if (res)
		return res;

	return 0;
}

int setup_88e1514(const char *bus, unsigned char addr)
{
	int res;

	res = process_setup(bus, addr,
			    verify_88e1518, ARRAY_SIZE(verify_88e1518));
	if (res)
		return res;

	res = process_setup(bus, addr,
			    fixup_88e1518, ARRAY_SIZE(fixup_88e1518));
	if (res)
		return res;

	res = process_setup(bus, addr,
			    mii_to_copper_88e1514,
			    ARRAY_SIZE(mii_to_copper_88e1514));
	if (res)
		return res;

	res = process_setup(bus, addr,
			    sgmii_autoneg_off_88e1518,
			    ARRAY_SIZE(sgmii_autoneg_off_88e1518));
	if (res)
		return res;

	res = process_setup(bus, addr,
			    invert_led2_88e1514,
			    ARRAY_SIZE(invert_led2_88e1514));
	if (res)
		return res;

	res = process_setup(bus, addr,
			    default_88e1518, ARRAY_SIZE(default_88e1518));
	if (res)
		return res;

	if (addr) {
		res = process_setup(bus, addr,
				    ch1fix_88e1518, ARRAY_SIZE(ch1fix_88e1518));
		if (res)
			return res;
	}

	res = process_setup(bus, addr,
			    swreset_88e1518, ARRAY_SIZE(swreset_88e1518));
	if (res)
		return res;

	return 0;
}
