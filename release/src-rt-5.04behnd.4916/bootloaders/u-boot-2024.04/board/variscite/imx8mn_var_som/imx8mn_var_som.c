// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Collabora Ltd.
 * Copyright 2018-2020 Variscite Ltd.
 * Copyright 2023 DimOnOff Inc.
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <i2c_eeprom.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <dt-bindings/gpio/gpio.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

/* Optional SOM features flags. */
#define VAR_EEPROM_F_WIFI		BIT(0)
#define VAR_EEPROM_F_ETH		BIT(1) /* Ethernet PHY on SOM. */
#define VAR_EEPROM_F_AUDIO		BIT(2)
#define VAR_EEPROM_F_MX8M_LVDS		BIT(3) /* i.MX8MM, i.MX8MN, i.MX8MQ only */
#define VAR_EEPROM_F_MX8Q_SOC_ID	BIT(3) /* 0 = i.MX8QM, 1 = i.MX8QP */
#define VAR_EEPROM_F_NAND		BIT(4)

#define VAR_IMX8_EEPROM_MAGIC	0x384D /* "8M" */

/* Number of DRAM adjustment tables. */
#define DRAM_TABLES_NUM 7

struct var_imx8_eeprom_info {
	u16 magic;
	u8 partnumber[3];         /* Part number */
	u8 assembly[10];          /* Assembly number */
	u8 date[9];               /* Build date */
	u8 mac[6];                /* MAC address */
	u8 somrev;
	u8 eeprom_version;
	u8 features;              /* SOM features */
	u8 dramsize;              /* DRAM size */
	u8 off[DRAM_TABLES_NUM + 1]; /* DRAM table offsets */
	u8 partnumber2[5];        /* Part number 2 */
} __packed;

int board_init(void)
{
	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

#if !defined(CONFIG_SPL_BUILD)

#if defined(CONFIG_DISPLAY_BOARDINFO)

static void display_som_infos(struct var_imx8_eeprom_info *info)
{
	char partnumber[sizeof(info->partnumber) +
			sizeof(info->partnumber2) + 1];
	char assembly[sizeof(info->assembly) + 1];
	char date[sizeof(info->date) + 1];

	/* Read first part of P/N. */
	memcpy(partnumber, info->partnumber, sizeof(info->partnumber));

	/* Read second part of P/N. */
	if (info->eeprom_version >= 3)
		memcpy(partnumber + sizeof(info->partnumber), info->partnumber2,
		       sizeof(info->partnumber2));

	memcpy(assembly, info->assembly, sizeof(info->assembly));
	memcpy(date, info->date, sizeof(info->date));

	/* Make sure strings are null terminated. */
	partnumber[sizeof(partnumber) - 1] = '\0';
	assembly[sizeof(assembly) - 1] = '\0';
	date[sizeof(date) - 1] = '\0';

	printf("SOM board: P/N: %s, Assy: %s, Date: %s\n"
	       "           Wifi: %s, EthPhy: %s, Rev: %d\n",
	       partnumber, assembly, date,
	       info->features & VAR_EEPROM_F_WIFI ? "yes" : "no",
	       info->features & VAR_EEPROM_F_ETH ? "yes" : "no",
	       info->somrev);
}

static int var_read_som_eeprom(struct var_imx8_eeprom_info *info)
{
	const char *path = "eeprom-som";
	struct udevice *dev;
	int ret, off;

	off = fdt_path_offset(gd->fdt_blob, path);
	if (off < 0) {
		pr_err("%s: fdt_path_offset() failed: %d\n", __func__, off);
		return off;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_I2C_EEPROM, off, &dev);
	if (ret) {
		pr_err("%s: uclass_get_device_by_of_offset() failed: %d\n",
		       __func__, ret);
		return ret;
	}

	ret = i2c_eeprom_read(dev, 0, (uint8_t *)info,
			      sizeof(struct var_imx8_eeprom_info));
	if (ret) {
		pr_err("%s: i2c_eeprom_read() failed: %d\n", __func__, ret);
		return ret;
	}

	if (htons(info->magic) != VAR_IMX8_EEPROM_MAGIC) {
		/* Do not fail if the content is invalid */
		pr_err("Board: Invalid board info magic: 0x%08x, expected 0x%08x\n",
		       htons(info->magic), VAR_IMX8_EEPROM_MAGIC);
	}

	return 0;
}

int checkboard(void)
{
	int rc;
	struct var_imx8_eeprom_info *info;

	info = malloc(sizeof(struct var_imx8_eeprom_info));
	if (!info)
		return -ENOMEM;

	rc = var_read_som_eeprom(info);
	if (rc)
		return rc;

	display_som_infos(info);

#if defined(CONFIG_BOARD_TYPES)
	gd->board_type = info->features;
#endif /* CONFIG_BOARD_TYPES */

	return 0;
}

#endif /* CONFIG_DISPLAY_BOARDINFO */

static int insert_gpios_prop(void *blob, int node, const char *prop,
			     unsigned int phandle, u32 gpio, u32 flags)
{
	fdt32_t val[3] = { cpu_to_fdt32(phandle), cpu_to_fdt32(gpio),
			   cpu_to_fdt32(flags) };
	return fdt_setprop(blob, node, prop, &val, sizeof(val));
}

static int configure_phy_reset_gpios(void *blob)
{
	int node;
	int phynode;
	int ret;
	u32 handle;
	u32 gpio;
	u32 flags;
	char path[1024];
	const char *eth_alias = "ethernet0";

	snprintf(path, sizeof(path), "%s/mdio/ethernet-phy@4",
		 fdt_get_alias(blob, eth_alias));

	phynode = fdt_path_offset(blob, path);
	if (phynode < 0) {
		pr_err("%s(): unable to locate PHY node: %s\n", __func__, path);
		return 0;
	}

	if (gd_board_type() & VAR_EEPROM_F_ETH) {
		snprintf(path, sizeof(path), "%s",
			 fdt_get_alias(blob, "gpio0")); /* Alias to gpio1 */
		gpio = 9;
		flags = GPIO_ACTIVE_LOW;
	} else {
		snprintf(path, sizeof(path), "%s/gpio@20",
			 fdt_get_alias(blob, "i2c1")); /* Alias to i2c2 */
		gpio = 5;
		flags = GPIO_ACTIVE_HIGH;
	}

	node = fdt_path_offset(blob, path);
	if (node < 0) {
		pr_err("%s(): unable to locate GPIO node: %s\n", __func__,
		       path);
		return 0;
	}

	handle = fdt_get_phandle(blob, node);
	if (handle < 0) {
		pr_err("%s(): unable to locate GPIO controller handle: %s\n",
		       __func__, path);
	}

	ret = insert_gpios_prop(blob, phynode, "reset-gpios",
				handle, gpio, flags);
	if (ret < 0) {
		pr_err("%s(): failed to set reset-gpios property\n", __func__);
		return ret;
	}

	return 0;
}

#if defined(CONFIG_OF_BOARD_FIXUP)
int board_fix_fdt(void *blob)
{
	/* Fix U-Boot device tree: */
	return configure_phy_reset_gpios(blob);
}
#endif /* CONFIG_OF_BOARD_FIXUP */

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	/* Fix kernel device tree: */
	return configure_phy_reset_gpios(blob);
}
#endif /* CONFIG_OF_BOARD_SETUP */

#endif /* CONFIG_SPL_BUILD */
