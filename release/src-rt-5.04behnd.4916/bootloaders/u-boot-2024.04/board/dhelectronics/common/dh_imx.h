/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2022 DENX Software Engineering GmbH, Philip Oberfichtner <pro@denx.de>
 */

/*
 * dh_imx_get_mac_from_fuse - Get MAC address from fuse and write it to env
 *
 * @enetaddr: buffer where address is to be stored
 * Return: 0 if OK, other value on error
 */
int dh_imx_get_mac_from_fuse(unsigned char *enetaddr);
