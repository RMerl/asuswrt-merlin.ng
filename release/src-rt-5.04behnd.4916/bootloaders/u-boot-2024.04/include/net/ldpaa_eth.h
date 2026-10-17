/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2023 NXP
 */

#define LDPAA_ETH_DRIVER_NAME	"ldpaa_eth"

/**
 * ldpaa_eth_get_dpmac_id() - Get the dpmac_id of a DPAA2 ethernet device
 *
 * @dev:	DPAA2 ethernet udevice pointer
 * Return: requested dpmac_id
 */

uint32_t ldpaa_eth_get_dpmac_id(struct udevice *dev);
