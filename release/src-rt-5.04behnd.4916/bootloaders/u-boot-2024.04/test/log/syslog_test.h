/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Header file for logging tests
 */

#ifndef __SYSLOG_TEST_H
#define __SYSLOG_TEST_H

/**
 * struct sb_log_env - private data for sandbox ethernet driver
 *
 * This structure is used for the private data of the sandbox ethernet
 * driver.
 *
 * @expected:	string expected to be written by the syslog driver
 * @uts:	unit test state
 */
struct sb_log_env {
	const char *expected;
	struct unit_test_state *uts;
};

/**
 * sb_log_tx_handler() - transmit callback function
 *
 * This callback function is invoked when a network package is sent using the
 * sandbox Ethernet driver. The private data of the driver holds a sb_log_env
 * structure with the unit test state and the expected UDP payload.
 *
 * The following checks are executed:
 *
 * * the Ethernet packet indicates a IP broadcast message
 * * the IP header is for a local UDP broadcast message to port 514
 * * the UDP payload matches the expected string
 *
 * After testing the pointer to the expected string is set to NULL to signal
 * that the callback function has been called.
 *
 * @dev:	sandbox ethernet device
 * @packet:	Ethernet packet
 * @len:	length of Ethernet packet
 * Return:	0 = success
 */
int sb_log_tx_handler(struct udevice *dev, void *packet, unsigned int len);

/**
 * syslog_test_setup() - Enable syslog logging ready for tests
 *
 * @uts: Test state
 * Return: 0 if OK, -ENOENT if the syslog log driver is not found
 */
int syslog_test_setup(struct unit_test_state *uts);

/**
 * syslog_test_finish() - Disable syslog logging after tests
 *
 * @uts: Test state
 * Return: 0 if OK, -ENOENT if the syslog log driver is not found
 */
int syslog_test_finish(struct unit_test_state *uts);

#endif
