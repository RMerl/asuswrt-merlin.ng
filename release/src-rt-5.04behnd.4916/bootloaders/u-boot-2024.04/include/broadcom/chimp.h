/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom.
 *
 */

#ifndef __CHIMP_H__
#define __CHIMP_H__

#include <linux/compiler.h>

/*
 * Chimp binary has health status like initialization complete,
 * crash or running fine
 */
#define BCM_CHIMP_RUNNIG_GOOD	0x8000

enum {
	CHIMP_HANDSHAKE_SUCCESS = 0,
	CHIMP_HANDSHAKE_WAIT_ERROR,
	CHIMP_HANDSHAKE_WAIT_TIMEOUT,
};

/**
 * chimp_fastboot_optee() - api to load bnxt firmware
 *
 * @return: 0 on success and -ve on failure
 */
int chimp_fastboot_optee(void);

/**
 * chimp_health_status_optee() - get chimp health status
 *
 * Chimp health status could be firmware is in good condition or
 * bad condition because of crash/hang.
 *
 * @status: pointer to get chimp health status
 *
 * @return: 0 on success and -ve on failure
 */
int chimp_health_status_optee(u32 *status);

/**
 * chimp_handshake_status_optee() - get chimp handshake status.
 *
 * To know firmware is loaded and running.
 *
 * @timeout: timeout value, if 0 then default timeout is considered by op-tee
 * @hstatus: pointer to chimp handshake status
 *
 * @return: 0 on success and -ve on failure
 */
int chimp_handshake_status_optee(u32 timeout, u32 *hstatus);

#endif
