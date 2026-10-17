/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2022 DENX Software Engineering GmbH, Philip Oberfichtner <pro@denx.de>
 */

/*
 * dh_mac_is_in_env - Check if MAC address is already set
 *
 * @env: name of environment variable
 * Return: true if MAC is set, false otherwise
 */
bool dh_mac_is_in_env(const char *env);

/*
 * dh_get_mac_from_eeprom - Get MAC address from eeprom and write it to enetaddr
 *
 * @enetaddr: buffer where address is to be stored
 * @alias: alias for EEPROM device tree node
 * Return: 0 if OK, other value on error
 */
int dh_get_mac_from_eeprom(unsigned char *enetaddr, const char *alias);

/*
 * dh_setup_mac_address - Try to get MAC address from various locations and write it to env
 *
 * Return: 0 if OK, other value on error
 */
int dh_setup_mac_address(void);
