// SPDX-License-Identifier: GPL-2.0+
/*
 * Pinctrl driver for Microchip PIC32 SoCs
 * Copyright (c) 2015 Microchip Technology Inc.
 * Written by Purna Chandra Mandal <purna.mandal@microchip.com>
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <mach/pic32.h>

DECLARE_GLOBAL_DATA_PTR;

/* PIC32 has 10 peripheral ports with 16 pins each.
 * Ports are marked PORTA-PORTK or PORT0-PORT9.
 */
enum {
	PIC32_PORT_A = 0,
	PIC32_PORT_B = 1,
	PIC32_PORT_C = 2,
	PIC32_PORT_D = 3,
	PIC32_PORT_E = 4,
	PIC32_PORT_F = 5,
	PIC32_PORT_G = 6,
	PIC32_PORT_H = 7,
	PIC32_PORT_J = 8, /* no PORT_I */
	PIC32_PORT_K = 9,
	PIC32_PINS_PER_PORT = 16,
};

#define PIN_CONFIG_PIC32_DIGITAL	(PIN_CONFIG_END + 1)
#define PIN_CONFIG_PIC32_ANALOG		(PIN_CONFIG_END + 2)

/* pin configuration descriptor */
struct pic32_pin_config {
	u16 port;	/* port number */
	u16 pin;	/* pin number in the port */
	u32 config;	/* one of PIN_CONFIG_* */
};
#define PIN_CONFIG(_prt, _pin, _cfg) \
	{.port = (_prt), .pin = (_pin), .config = (_cfg), }

/* In PIC32 muxing is performed at pin-level through two
 * different set of registers - one set for input functions,
 * and other for output functions.
 * Pin configuration is handled through port register.
 */
/* Port control registers */
struct pic32_reg_port {
	struct pic32_reg_atomic ansel;
	struct pic32_reg_atomic tris;
	struct pic32_reg_atomic port;
	struct pic32_reg_atomic lat;
	struct pic32_reg_atomic odc;
	struct pic32_reg_atomic cnpu;
	struct pic32_reg_atomic cnpd;
	struct pic32_reg_atomic cncon;
	struct pic32_reg_atomic unused[8];
};

/* Input function mux registers */
struct pic32_reg_in_mux {
	u32 unused0;
	u32 int1[4];
	u32 unused1;
	u32 t2ck[8];
	u32 ic1[9];
	u32 unused2;
	u32 ocfar;
	u32 unused3;
	u32 u1rx;
	u32 u1cts;
	u32 u2rx;
	u32 u2cts;
	u32 u3rx;
	u32 u3cts;
	u32 u4rx;
	u32 u4cts;
	u32 u5rx;
	u32 u5cts;
	u32 u6rx;
	u32 u6cts;
	u32 unused4;
	u32 sdi1;
	u32 ss1;
	u32 unused5;
	u32 sdi2;
	u32 ss2;
	u32 unused6;
	u32 sdi3;
	u32 ss3;
	u32 unused7;
	u32 sdi4;
	u32 ss4;
	u32 unused8;
	u32 sdi5;
	u32 ss5;
	u32 unused9;
	u32 sdi6;
	u32 ss6;
	u32 c1rx;
	u32 c2rx;
	u32 refclki1;
	u32 refclki2;
	u32 refclki3;
	u32 refclki4;
};

/* output mux register offset */
#define PPS_OUT(__port, __pin) \
	(((__port) * PIC32_PINS_PER_PORT + (__pin)) << 2)


struct pic32_pinctrl_priv {
	struct pic32_reg_in_mux *mux_in; /* mux input function */
	struct pic32_reg_port *pinconf; /* pin configuration*/
	void __iomem *mux_out;	/* mux output function */
};

enum {
	PERIPH_ID_UART1,
	PERIPH_ID_UART2,
	PERIPH_ID_ETH,
	PERIPH_ID_USB,
	PERIPH_ID_SDHCI,
	PERIPH_ID_I2C1,
	PERIPH_ID_I2C2,
	PERIPH_ID_SPI1,
	PERIPH_ID_SPI2,
	PERIPH_ID_SQI,
};

static int pic32_pinconfig_one(struct pic32_pinctrl_priv *priv,
			       u32 port_nr, u32 pin, u32 param)
{
	struct pic32_reg_port *port;

	port = &priv->pinconf[port_nr];
	switch (param) {
	case PIN_CONFIG_PIC32_DIGITAL:
		writel(BIT(pin), &port->ansel.clr);
		break;
	case PIN_CONFIG_PIC32_ANALOG:
		writel(BIT(pin), &port->ansel.set);
		break;
	case PIN_CONFIG_INPUT_ENABLE:
		writel(BIT(pin), &port->tris.set);
		break;
	case PIN_CONFIG_OUTPUT:
		writel(BIT(pin), &port->tris.clr);
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		writel(BIT(pin), &port->cnpu.set);
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		writel(BIT(pin), &port->cnpd.set);
		break;
	case PIN_CONFIG_DRIVE_OPEN_DRAIN:
		writel(BIT(pin), &port->odc.set);
		break;
	default:
		break;
	}

	return 0;
}

static int pic32_pinconfig_set(struct pic32_pinctrl_priv *priv,
			       const struct pic32_pin_config *list, int count)
{
	int i;

	for (i = 0 ; i < count; i++)
		pic32_pinconfig_one(priv, list[i].port,
				    list[i].pin, list[i].config);

	return 0;
}

static void pic32_eth_pin_config(struct udevice *dev)
{
	struct pic32_pinctrl_priv *priv = dev_get_priv(dev);
	const struct pic32_pin_config configs[] = {
		/* EMDC - D11 */
		PIN_CONFIG(PIC32_PORT_D, 11, PIN_CONFIG_PIC32_DIGITAL),
		PIN_CONFIG(PIC32_PORT_D, 11, PIN_CONFIG_OUTPUT),
		/* ETXEN */
		PIN_CONFIG(PIC32_PORT_D, 6, PIN_CONFIG_PIC32_DIGITAL),
		PIN_CONFIG(PIC32_PORT_D, 6, PIN_CONFIG_OUTPUT),
		/* ECRSDV */
		PIN_CONFIG(PIC32_PORT_H, 13, PIN_CONFIG_PIC32_DIGITAL),
		PIN_CONFIG(PIC32_PORT_H, 13, PIN_CONFIG_INPUT_ENABLE),
		/* ERXD0 */
		PIN_CONFIG(PIC32_PORT_H, 8, PIN_CONFIG_PIC32_DIGITAL),
		PIN_CONFIG(PIC32_PORT_H, 8, PIN_CONFIG_INPUT_ENABLE),
		PIN_CONFIG(PIC32_PORT_H, 8, PIN_CONFIG_BIAS_PULL_DOWN),
		/* ERXD1 */
		PIN_CONFIG(PIC32_PORT_H, 5, PIN_CONFIG_PIC32_DIGITAL),
		PIN_CONFIG(PIC32_PORT_H, 5, PIN_CONFIG_INPUT_ENABLE),
		PIN_CONFIG(PIC32_PORT_H, 5, PIN_CONFIG_BIAS_PULL_DOWN),
		/* EREFCLK */
		PIN_CONFIG(PIC32_PORT_J, 11, PIN_CONFIG_PIC32_DIGITAL),
		PIN_CONFIG(PIC32_PORT_J, 11, PIN_CONFIG_INPUT_ENABLE),
		/* ETXD1 */
		PIN_CONFIG(PIC32_PORT_J, 9, PIN_CONFIG_PIC32_DIGITAL),
		PIN_CONFIG(PIC32_PORT_J, 9, PIN_CONFIG_OUTPUT),
		/* ETXD0 */
		PIN_CONFIG(PIC32_PORT_J, 8, PIN_CONFIG_PIC32_DIGITAL),
		PIN_CONFIG(PIC32_PORT_J, 8, PIN_CONFIG_OUTPUT),
		/* EMDIO */
		PIN_CONFIG(PIC32_PORT_J, 1, PIN_CONFIG_PIC32_DIGITAL),
		PIN_CONFIG(PIC32_PORT_J, 1, PIN_CONFIG_INPUT_ENABLE),
		/* ERXERR */
		PIN_CONFIG(PIC32_PORT_F, 3, PIN_CONFIG_PIC32_DIGITAL),
		PIN_CONFIG(PIC32_PORT_F, 3, PIN_CONFIG_INPUT_ENABLE),
	};

	pic32_pinconfig_set(priv, configs, ARRAY_SIZE(configs));
}

static void pic32_sdhci_pin_config(struct udevice *dev)
{
	struct pic32_pinctrl_priv *priv = dev_get_priv(dev);
	const struct pic32_pin_config configs[] = {
		/* SDWP - H2 */
		PIN_CONFIG(PIC32_PORT_H, 2, PIN_CONFIG_PIC32_DIGITAL),
		/* SDCD - A0 */
		PIN_CONFIG(PIC32_PORT_A, 0, PIN_CONFIG_PIC32_DIGITAL),
		/* SDCMD - D4 */
		PIN_CONFIG(PIC32_PORT_D, 4, PIN_CONFIG_PIC32_DIGITAL),
		/* SDCK - A6 */
		PIN_CONFIG(PIC32_PORT_A, 6, PIN_CONFIG_PIC32_DIGITAL),
		/* SDDATA0 - G13 */
		PIN_CONFIG(PIC32_PORT_G, 13, PIN_CONFIG_PIC32_DIGITAL),
		/* SDDATA1 - G12 */
		PIN_CONFIG(PIC32_PORT_G, 12, PIN_CONFIG_PIC32_DIGITAL),
		/* SDDATA2 - G14 */
		PIN_CONFIG(PIC32_PORT_G, 14, PIN_CONFIG_PIC32_DIGITAL),
		/* SDDATA3 - A7 */
		PIN_CONFIG(PIC32_PORT_A, 7, PIN_CONFIG_PIC32_DIGITAL),
	};

	pic32_pinconfig_set(priv, configs, ARRAY_SIZE(configs));
}

static int pic32_pinctrl_request(struct udevice *dev, int func, int flags)
{
	struct pic32_pinctrl_priv *priv = dev_get_priv(dev);

	switch (func) {
	case PERIPH_ID_UART2:
		/* PPS for U2 RX/TX */
		writel(0x02, priv->mux_out + PPS_OUT(PIC32_PORT_G, 9));
		writel(0x05, &priv->mux_in->u2rx); /* B0 */
		/* set digital mode */
		pic32_pinconfig_one(priv, PIC32_PORT_G, 9,
				    PIN_CONFIG_PIC32_DIGITAL);
		pic32_pinconfig_one(priv, PIC32_PORT_B, 0,
				    PIN_CONFIG_PIC32_DIGITAL);
		break;
	case PERIPH_ID_ETH:
		pic32_eth_pin_config(dev);
		break;
	case PERIPH_ID_SDHCI:
		pic32_sdhci_pin_config(dev);
		break;
	default:
		debug("%s: unknown-unhandled case\n", __func__);
		break;
	}

	return 0;
}

static int pic32_pinctrl_get_periph_id(struct udevice *dev,
				       struct udevice *periph)
{
	int ret;
	u32 cell[2];

	ret = fdtdec_get_int_array(gd->fdt_blob, dev_of_offset(periph),
				   "interrupts", cell, ARRAY_SIZE(cell));
	if (ret < 0)
		return -EINVAL;

	/* interrupt number */
	switch (cell[0]) {
	case 112 ... 114:
		return PERIPH_ID_UART1;
	case 145 ... 147:
		return PERIPH_ID_UART2;
	case 109 ... 111:
		return PERIPH_ID_SPI1;
	case 142 ... 144:
		return PERIPH_ID_SPI2;
	case 115 ... 117:
		return PERIPH_ID_I2C1;
	case 148 ... 150:
		return PERIPH_ID_I2C2;
	case 132 ... 133:
		return PERIPH_ID_USB;
	case 169:
		return PERIPH_ID_SQI;
	case 191:
		return PERIPH_ID_SDHCI;
	case 153:
		return PERIPH_ID_ETH;
	default:
		break;
	}

	return -ENOENT;
}

static int pic32_pinctrl_set_state_simple(struct udevice *dev,
					  struct udevice *periph)
{
	int func;

	debug("%s: periph %s\n", __func__, periph->name);
	func = pic32_pinctrl_get_periph_id(dev, periph);
	if (func < 0)
		return func;
	return pic32_pinctrl_request(dev, func, 0);
}

static struct pinctrl_ops pic32_pinctrl_ops = {
	.set_state_simple	= pic32_pinctrl_set_state_simple,
	.request		= pic32_pinctrl_request,
	.get_periph_id		= pic32_pinctrl_get_periph_id,
};

static int pic32_pinctrl_probe(struct udevice *dev)
{
	struct pic32_pinctrl_priv *priv = dev_get_priv(dev);
	struct fdt_resource res;
	void *fdt = (void *)gd->fdt_blob;
	int node = dev_of_offset(dev);
	int ret;

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "ppsin", &res);
	if (ret < 0) {
		printf("pinctrl: resource \"ppsin\" not found\n");
		return ret;
	}
	priv->mux_in = ioremap(res.start, fdt_resource_size(&res));

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "ppsout", &res);
	if (ret < 0) {
		printf("pinctrl: resource \"ppsout\" not found\n");
		return ret;
	}
	priv->mux_out = ioremap(res.start, fdt_resource_size(&res));

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "port", &res);
	if (ret < 0) {
		printf("pinctrl: resource \"port\" not found\n");
		return ret;
	}
	priv->pinconf = ioremap(res.start, fdt_resource_size(&res));

	return 0;
}

static const struct udevice_id pic32_pinctrl_ids[] = {
	{ .compatible = "microchip,pic32mzda-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_pic32) = {
	.name		= "pinctrl_pic32",
	.id		= UCLASS_PINCTRL,
	.of_match	= pic32_pinctrl_ids,
	.ops		= &pic32_pinctrl_ops,
	.probe		= pic32_pinctrl_probe,
	.bind		= dm_scan_fdt_dev,
	.priv_auto	= sizeof(struct pic32_pinctrl_priv),
};
