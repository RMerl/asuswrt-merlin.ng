/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __BOOTOCT_CMD_H__
#define __BOOTOCT_CMD_H__

#include "cvmx-coremask.h"

enum octeon_boot_cmd_type {
	BOOTOCT,
	BOOTOCTLINUX,
	BOOTOCTELF
};

/** Structure to contain results of command line argument parsing */
struct octeon_boot_args {
	struct cvmx_coremask coremask;	/** Parsed coremask */
	int num_cores[CVMX_MAX_NODES];	/** number of cores */
	int num_skipped[CVMX_MAX_NODES];/** number of skipped cores */
	const char *app_name;		/** Application name */
	const char *named_block;	/** Named block to load Linux into */
	u32 stack_size;			/** stack size */
	u32 heap_size;			/** heap size */
	u32 boot_flags;			/** boot flags */
	int node_mask;			/** Node mask to use */
	int console_uart;		/** serial console number */
	bool forceboot;			/** force booting if core 0 not set */
	bool coremask_set;		/** set if coremask was set */
	bool num_cores_set;		/** Set if num_cores was set */
	bool num_skipped_set;		/** Set if num_skipped was set */
	/** Set if endbootargs parameter was passed. */
	bool endbootargs;
};

/**
 * Parse command line arguments
 *
 * @param argc			number of arguments
 * @param[in] argv		array of argument strings
 * @param cmd			command type
 * @param[out] boot_args	parsed values
 *
 * Return: number of arguments parsed
 */
int octeon_parse_bootopts(int argc, char *const argv[],
			  enum octeon_boot_cmd_type cmd,
			  struct octeon_boot_args *boot_args);

void nmi_bootvector(void);
extern u64 nmi_handler_para[];

#endif /* __BOOTOCT_CMD_H__ */
