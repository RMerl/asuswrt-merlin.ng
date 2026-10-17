#include <common.h>
#include <dm.h>
#include <miiphy.h>
#include <asm-generic/gpio.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#include "ihs_phys.h"
#include "dt_helpers.h"

enum {
	PORTTYPE_MAIN_CAT,
	PORTTYPE_TOP_CAT,
	PORTTYPE_16C_16F,
	PORTTYPE_UNKNOWN
};

static struct porttype {
	bool phy_invert_in_pol;
	bool phy_invert_out_pol;
} porttypes[] = {
	{ true, false },
	{ false, true },
	{ false, false },
};

static void ihs_phy_config(struct phy_device *phydev, bool qinpn, bool qoutpn)
{
	u16 reg;

	phydev->interface = PHY_INTERFACE_MODE_MII;
	phy_config(phydev);

	/* enable QSGMII autonegotiation with flow control */
	phy_write(phydev, MDIO_DEVAD_NONE, 22, 0x0004);
	reg = phy_read(phydev, MDIO_DEVAD_NONE, 16);
	reg |= (3 << 6);
	phy_write(phydev, MDIO_DEVAD_NONE, 16, reg);

	/*
	 * invert QSGMII Q_INP/N and Q_OUTP/N if required
	 * and perform global reset
	 */
	reg = phy_read(phydev, MDIO_DEVAD_NONE, 26);
	if (qinpn)
		reg |= (1 << 13);
	if (qoutpn)
		reg |= (1 << 12);
	reg |= (1 << 15);
	phy_write(phydev, MDIO_DEVAD_NONE, 26, reg);

	/* advertise 1000BASE-T full-duplex only  */
	phy_write(phydev, MDIO_DEVAD_NONE, 22, 0x0000);
	reg = phy_read(phydev, MDIO_DEVAD_NONE, 4);
	reg &= ~0x1e0;
	phy_write(phydev, MDIO_DEVAD_NONE, 4, reg);
	reg = phy_read(phydev, MDIO_DEVAD_NONE, 9);
	reg = (reg & ~0x300) | 0x200;
	phy_write(phydev, MDIO_DEVAD_NONE, 9, reg);

	/* copper power up */
	reg = phy_read(phydev, MDIO_DEVAD_NONE, 16);
	reg &= ~0x0004;
	phy_write(phydev, MDIO_DEVAD_NONE, 16, reg);
}

uint calculate_octo_phy_mask(void)
{
	uint k;
	uint octo_phy_mask = 0;
	struct gpio_desc gpio = {};
	char gpio_name[64];
	static const char * const dev_name[] = {"pca9698@23", "pca9698@21",
						"pca9698@24", "pca9698@25",
						"pca9698@26"};

	/* mark all octo phys that should be present */
	for (k = 0; k < 5; ++k) {
		snprintf(gpio_name, 64, "cat-gpio-%u", k);

		if (request_gpio_by_name(&gpio, dev_name[k], 0x20, gpio_name))
			continue;

		/* check CAT flag */
		if (dm_gpio_get_value(&gpio))
			octo_phy_mask |= (1 << (k * 2));
		else
			/* If CAT == 0, there's no second octo phy -> skip */
			continue;

		snprintf(gpio_name, 64, "second-octo-gpio-%u", k);

		if (request_gpio_by_name(&gpio, dev_name[k], 0x27, gpio_name)) {
			/* default: second octo phy is present */
			octo_phy_mask |= (1 << (k * 2 + 1));
			continue;
		}

		if (dm_gpio_get_value(&gpio) == 0)
			octo_phy_mask |= (1 << (k * 2 + 1));
	}

	return octo_phy_mask;
}

int register_miiphy_bus(uint k, struct mii_dev **bus)
{
	int retval;
	struct mii_dev *mdiodev = mdio_alloc();
	char *name = bb_miiphy_buses[k].name;

	if (!mdiodev)
		return -ENOMEM;
	strlcpy(mdiodev->name, name, MDIO_NAME_LEN);
	mdiodev->read = bb_miiphy_read;
	mdiodev->write = bb_miiphy_write;

	retval = mdio_register(mdiodev);
	if (retval < 0)
		return retval;
	*bus = miiphy_get_dev_by_name(name);

	return 0;
}

struct porttype *get_porttype(uint octo_phy_mask, uint k)
{
	uint octo_index = k * 4;

	if (!k) {
		if (octo_phy_mask & 0x01)
			return &porttypes[PORTTYPE_MAIN_CAT];
		else if (!(octo_phy_mask & 0x03))
			return &porttypes[PORTTYPE_16C_16F];
	} else {
		if (octo_phy_mask & (1 << octo_index))
			return &porttypes[PORTTYPE_TOP_CAT];
	}

	return NULL;
}

int init_single_phy(struct porttype *porttype, struct mii_dev *bus,
		    uint bus_idx, uint m, uint phy_idx)
{
	struct phy_device *phydev;

	phydev = phy_find_by_mask(bus, BIT(m * 8 + phy_idx));
	printf(" %u", bus_idx * 32 + m * 8 + phy_idx);

	if (!phydev)
		puts("!");
	else
		ihs_phy_config(phydev, porttype->phy_invert_in_pol,
			       porttype->phy_invert_out_pol);

	return 0;
}

int init_octo_phys(uint octo_phy_mask)
{
	uint bus_idx;

	/* there are up to four octo-phys on each mdio bus */
	for (bus_idx = 0; bus_idx < bb_miiphy_buses_num; ++bus_idx) {
		uint m;
		uint octo_index = bus_idx * 4;
		struct mii_dev *bus = NULL;
		struct porttype *porttype = NULL;
		int ret;

		porttype = get_porttype(octo_phy_mask, bus_idx);

		if (!porttype)
			continue;

		for (m = 0; m < 4; ++m) {
			uint phy_idx;

			/**
			 * Register a bus device if there is at least one phy
			 * on the current bus
			 */
			if (!m && octo_phy_mask & (0xf << octo_index)) {
				ret = register_miiphy_bus(bus_idx, &bus);
				if (ret)
					return ret;
			}

			if (!(octo_phy_mask & BIT(octo_index + m)))
				continue;

			for (phy_idx = 0; phy_idx < 8; ++phy_idx)
				init_single_phy(porttype, bus, bus_idx, m,
						phy_idx);
		}
	}

	return 0;
}

/*
 * MII GPIO bitbang implementation
 * MDC MDIO bus
 * 13  14   PHY1-4
 * 25  45   PHY5-8
 * 46  24   PHY9-10
 */

struct gpio_mii {
	int index;
	struct gpio_desc mdc_gpio;
	struct gpio_desc mdio_gpio;
	int mdc_num;
	int mdio_num;
	int mdio_value;
} gpio_mii_set[] = {
	{ 0, {}, {}, 13, 14, 1 },
	{ 1, {}, {}, 25, 45, 1 },
	{ 2, {}, {}, 46, 24, 1 },
};

static int mii_mdio_init(struct bb_miiphy_bus *bus)
{
	struct gpio_mii *gpio_mii = bus->priv;
	char name[32] = {};
	struct udevice *gpio_dev1 = NULL;
	struct udevice *gpio_dev2 = NULL;

	if (uclass_get_device_by_name(UCLASS_GPIO, "gpio@18100", &gpio_dev1) ||
	    uclass_get_device_by_name(UCLASS_GPIO, "gpio@18140", &gpio_dev2)) {
		printf("Could not get GPIO device.\n");
		return 1;
	}

	if (gpio_mii->mdc_num > 31) {
		gpio_mii->mdc_gpio.dev = gpio_dev2;
		gpio_mii->mdc_gpio.offset = gpio_mii->mdc_num - 32;
	} else {
		gpio_mii->mdc_gpio.dev = gpio_dev1;
		gpio_mii->mdc_gpio.offset = gpio_mii->mdc_num;
	}
	gpio_mii->mdc_gpio.flags = 0;
	snprintf(name, 32, "bb_miiphy_bus-%d-mdc", gpio_mii->index);
	dm_gpio_request(&gpio_mii->mdc_gpio, name);

	if (gpio_mii->mdio_num > 31) {
		gpio_mii->mdio_gpio.dev = gpio_dev2;
		gpio_mii->mdio_gpio.offset = gpio_mii->mdio_num - 32;
	} else {
		gpio_mii->mdio_gpio.dev = gpio_dev1;
		gpio_mii->mdio_gpio.offset = gpio_mii->mdio_num;
	}
	gpio_mii->mdio_gpio.flags = 0;
	snprintf(name, 32, "bb_miiphy_bus-%d-mdio", gpio_mii->index);
	dm_gpio_request(&gpio_mii->mdio_gpio, name);

	dm_gpio_set_dir_flags(&gpio_mii->mdc_gpio, GPIOD_IS_OUT);
	dm_gpio_set_value(&gpio_mii->mdc_gpio, 1);

	return 0;
}

static int mii_mdio_active(struct bb_miiphy_bus *bus)
{
	struct gpio_mii *gpio_mii = bus->priv;

	dm_gpio_set_value(&gpio_mii->mdc_gpio, gpio_mii->mdio_value);

	return 0;
}

static int mii_mdio_tristate(struct bb_miiphy_bus *bus)
{
	struct gpio_mii *gpio_mii = bus->priv;

	dm_gpio_set_dir_flags(&gpio_mii->mdio_gpio, GPIOD_IS_IN);

	return 0;
}

static int mii_set_mdio(struct bb_miiphy_bus *bus, int v)
{
	struct gpio_mii *gpio_mii = bus->priv;

	dm_gpio_set_dir_flags(&gpio_mii->mdio_gpio, GPIOD_IS_OUT);
	dm_gpio_set_value(&gpio_mii->mdio_gpio, v);
	gpio_mii->mdio_value = v;

	return 0;
}

static int mii_get_mdio(struct bb_miiphy_bus *bus, int *v)
{
	struct gpio_mii *gpio_mii = bus->priv;

	dm_gpio_set_dir_flags(&gpio_mii->mdio_gpio, GPIOD_IS_IN);
	*v = (dm_gpio_get_value(&gpio_mii->mdio_gpio));

	return 0;
}

static int mii_set_mdc(struct bb_miiphy_bus *bus, int v)
{
	struct gpio_mii *gpio_mii = bus->priv;

	dm_gpio_set_value(&gpio_mii->mdc_gpio, v);

	return 0;
}

static int mii_delay(struct bb_miiphy_bus *bus)
{
	udelay(1);

	return 0;
}

struct bb_miiphy_bus bb_miiphy_buses[] = {
	{
		.name = "ihs0",
		.init = mii_mdio_init,
		.mdio_active = mii_mdio_active,
		.mdio_tristate = mii_mdio_tristate,
		.set_mdio = mii_set_mdio,
		.get_mdio = mii_get_mdio,
		.set_mdc = mii_set_mdc,
		.delay = mii_delay,
		.priv = &gpio_mii_set[0],
	},
	{
		.name = "ihs1",
		.init = mii_mdio_init,
		.mdio_active = mii_mdio_active,
		.mdio_tristate = mii_mdio_tristate,
		.set_mdio = mii_set_mdio,
		.get_mdio = mii_get_mdio,
		.set_mdc = mii_set_mdc,
		.delay = mii_delay,
		.priv = &gpio_mii_set[1],
	},
	{
		.name = "ihs2",
		.init = mii_mdio_init,
		.mdio_active = mii_mdio_active,
		.mdio_tristate = mii_mdio_tristate,
		.set_mdio = mii_set_mdio,
		.get_mdio = mii_get_mdio,
		.set_mdc = mii_set_mdc,
		.delay = mii_delay,
		.priv = &gpio_mii_set[2],
	},
};

int bb_miiphy_buses_num = ARRAY_SIZE(bb_miiphy_buses);
