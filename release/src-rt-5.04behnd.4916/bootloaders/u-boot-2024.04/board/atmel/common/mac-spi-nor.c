// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Tudor Ambarus <tudor.ambarus@microchip.com>
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <net.h>
#include <linux/mtd/spi-nor.h>
#include <netdev.h>

#define ETH_ADDR_SIZE			6

#ifdef CONFIG_SPI_FLASH_SST
#define SFDP_MICROCHIP_MANUF_ID		0xbf
#define SFDP_MICROCHIP_MEM_TYPE		0x26
#define SFDP_MICROCHIP_DEV_ID		0x43

#define SFDP_MICROCHIP_EUI_OFFSET	0x60
#define SFDP_MICROCHIP_EUI48		0x30

struct sst26vf064beui {
	u8 manufacturer_id;
	u8 memory_type;
	u8 device_id;
	u8 reserved;
};

/**
 * sst26vf064beui_check() - Check the validity of the EUI-48 information from
 * the sst26vf064beui SPI NOR Microchip SFDP table.
 * @manufacturer_sfdp:	pointer to the Microchip manufacturer specific SFDP
 *			table.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int sst26vf064beui_check(const u8 *manufacturer_sfdp)
{
	struct sst26vf064beui *sst26vf064beui =
		(struct sst26vf064beui *)manufacturer_sfdp;

	if (sst26vf064beui->manufacturer_id != SFDP_MICROCHIP_MANUF_ID)
		return -EINVAL;

	if (sst26vf064beui->memory_type != SFDP_MICROCHIP_MEM_TYPE)
		return -EINVAL;

	if (sst26vf064beui->device_id != SFDP_MICROCHIP_DEV_ID)
		return -EINVAL;

	/*
	 * Check if the EUI-48 MAC address is programmed in the next six address
	 * locations.
	 */
	if (manufacturer_sfdp[SFDP_MICROCHIP_EUI_OFFSET] !=
	    SFDP_MICROCHIP_EUI48)
		return -EINVAL;

	return 0;
}

/**
 * sst26vf064beui_get_ethaddr() - Get the ethernet address from the
 * sst26vf064beui SPI NOR Microchip SFDP table.
 * @manufacturer_sfdp:	pointer to the Microchip manufacturer specific SFDP
 *			table.
 * @ethaddr:		pointer where to fill the ethernet address
 * @size:		size of the ethernet address.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int sst26vf064beui_get_ethaddr(const u8 *manufacturer_sfdp,
				      u8 *ethaddr, size_t size)
{
	u64 eui_table[2];
	u64 *p = (u64 *)&manufacturer_sfdp[SFDP_MICROCHIP_EUI_OFFSET];
	int i, ret;

	ret = sst26vf064beui_check(manufacturer_sfdp);
	if (ret)
		return ret;

	for (i = 0; i < 2; i++)
		eui_table[i] = le64_to_cpu(p[i]);

	/* Ethaddr starts at offset one. */
	memcpy(ethaddr, &((u8 *)eui_table)[1], size);

	return 0;
}
#endif

/**
 * at91_spi_nor_set_ethaddr() - Retrieve and set the ethernet address from the
 * SPI NOR manufacturer specific SFDP table.
 */
void at91_spi_nor_set_ethaddr(void)
{
	struct udevice *dev;
	struct spi_nor *nor;
	const char *ethaddr_name = "ethaddr";
	u8 ethaddr[ETH_ADDR_SIZE] = {0};

	if (env_get(ethaddr_name))
		return;

	if (uclass_first_device_err(UCLASS_SPI_FLASH, &dev))
		return;

	nor = dev_get_uclass_priv(dev);
	if (!nor)
		return;

	if (!nor->manufacturer_sfdp)
		return;

#ifdef CONFIG_SPI_FLASH_SST
	if (sst26vf064beui_get_ethaddr(nor->manufacturer_sfdp, ethaddr,
				       ETH_ADDR_SIZE))
		return;
#endif

	if (is_valid_ethaddr(ethaddr))
		eth_env_set_enetaddr(ethaddr_name, ethaddr);
}
