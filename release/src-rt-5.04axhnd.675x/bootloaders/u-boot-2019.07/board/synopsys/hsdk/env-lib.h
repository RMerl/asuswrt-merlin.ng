/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Synopsys, Inc. All rights reserved.
 * Author: Eugeniy Paltsev <Eugeniy.Paltsev@synopsys.com>
 */

#ifndef __BOARD_ENV_LIB_H
#define __BOARD_ENV_LIB_H

#include <common.h>
#include <config.h>
#include <linux/kernel.h>

enum env_type {
	ENV_DEC,
	ENV_HEX
};

typedef struct {
	u32 val;
	bool set;
} u32_env;

struct env_map_common {
	const char *const env_name;
	enum env_type type;
	bool mandatory;
	u32 min;
	u32 max;
	u32_env *val;
};

struct env_map_percpu {
	const char *const env_name;
	enum env_type type;
	bool mandatory;
	u32 min[NR_CPUS];
	u32 max[NR_CPUS];
	u32_env (*val)[NR_CPUS];
};

void envs_cleanup_common(const struct env_map_common *map);
int envs_read_common(const struct env_map_common *map);
int envs_validate_common(const struct env_map_common *map);
int envs_read_validate_common(const struct env_map_common *map);

void envs_cleanup_core(const struct env_map_percpu *map);
int envs_read_validate_core(const struct env_map_percpu *map,
			    bool (*cpu_used)(u32));
int envs_process_and_validate(const struct env_map_common *common,
			      const struct env_map_percpu *core,
			      bool (*cpu_used)(u32));

int args_envs_enumerate(const struct env_map_common *map,
			int enum_by, int argc, char *const argv[]);

#endif /* __BOARD_ENV_LIB_H */
