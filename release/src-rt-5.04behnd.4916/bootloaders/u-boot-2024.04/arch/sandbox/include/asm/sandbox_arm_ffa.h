/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#ifndef __SANDBOX_ARM_FFA_H
#define __SANDBOX_ARM_FFA_H

#include <arm_ffa.h>

/*
 * This header provides public sandbox FF-A emulator declarations
 * and declarations needed by FF-A sandbox clients
 */

/* UUIDs strings of the emulated services */
#define SANDBOX_SERVICE1_UUID	"ed32d533-4209-99e6-2d72-cdd998a79cc0"
#define SANDBOX_SERVICE2_UUID	"ed32d544-4209-99e6-2d72-cdd998a79cc0"

/* IDs of the emulated secure partitions (SPs) */
#define SANDBOX_SP1_ID 0x1245
#define SANDBOX_SP2_ID 0x9836
#define SANDBOX_SP3_ID 0x6452
#define SANDBOX_SP4_ID 0x7814

/* Invalid service UUID (no matching SP) */
#define SANDBOX_SERVICE3_UUID	"55d532ed-0942-e699-722d-c09ca798d9cd"

/* Invalid service UUID (invalid UUID string format) */
#define SANDBOX_SERVICE4_UUID	"32ed-0942-e699-722d-c09ca798d9cd"

/* Number of valid services */
#define SANDBOX_SP_COUNT_PER_VALID_SERVICE	2

/**
 * struct ffa_sandbox_data - query ABI state data structure
 * @data0_size:	size of the first argument
 * @data0:	pointer to the first argument
 * @data1_size>:	size of the second argument
 * @data1:	pointer to the second argument
 *
 * Used to pass various types of data with different sizes between
 * the test cases and the sandbox emulator.
 * The data is for querying FF-A ABIs state.
 */
struct ffa_sandbox_data {
	u32 data0_size; /* size of the first argument */
	void *data0; /* pointer to the first argument */
	u32 data1_size; /* size of the second argument */
	void *data1; /* pointer to the second argument */
};

/* The sandbox FF-A  emulator public functions */

/**
 * sandbox_query_ffa_emul_state() - Inspect the FF-A ABIs
 * @queried_func_id:	The FF-A function to be queried
 * @func_data:  Pointer to the FF-A function arguments container structure
 *
 * Query the status of FF-A ABI specified in the input argument.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
int sandbox_query_ffa_emul_state(u32 queried_func_id,
				 struct ffa_sandbox_data *func_data);

#endif
