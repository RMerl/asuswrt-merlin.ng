/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Google, Inc
 */

#ifndef __DT_STRUCTS
#define __DT_STRUCTS

/* These structures may only be used in SPL */
#if CONFIG_IS_ENABLED(OF_PLATDATA)
struct driver_info;

/**
 * struct phandle_0_arg - hold a phandle record with no arguments
 *
 * This holds a phandle pointing to another device. See 'Indexes' in the
 * of-plat-rst documentation.
 *
 * @idx: udevice index (or driver_info index if !OF_PLATDATA_INST)
 * @arg: arguments
 */
struct phandle_0_arg {
	uint idx;
	int arg[0];
};

/**
 * struct phandle_2_arg - hold a phandle record with up to one argument
 *
 * This holds a phandle pointing to another device. See 'Indexes' in the
 * of-plat-rst documentation.
 *
 * @idx: udevice index (or driver_info index if !OF_PLATDATA_INST)
 * @arg: arguments
 */
struct phandle_1_arg {
	uint idx;
	int arg[1];
};

/**
 * struct phandle_2_arg - hold a phandle record with up to two arguments
 *
 * This holds a phandle pointing to another device. See 'Indexes' in the
 * of-plat-rst documentation.
 *
 * @idx: udevice index (or driver_info index if !OF_PLATDATA_INST)
 * @arg: arguments
 */
struct phandle_2_arg {
	uint idx;
	int arg[2];
};

#include <generated/dt-structs-gen.h>
#include <generated/dt-decl.h>
#endif

#endif
