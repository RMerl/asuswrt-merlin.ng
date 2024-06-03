// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Synopsys, Inc. All rights reserved.
 * Author: Eugeniy Paltsev <Eugeniy.Paltsev@synopsys.com>
 */

#include "env-lib.h"

#define MAX_CMD_LEN	25

static void env_clear_common(u32 index, const struct env_map_common *map)
{
	map[index].val->val = 0;
	map[index].val->set = false;
}

static int env_read_common(u32 index, const struct env_map_common *map)
{
	u32 val;

	if (!env_get_yesno(map[index].env_name)) {
		if (map[index].type == ENV_HEX) {
			val = (u32)env_get_hex(map[index].env_name, 0);
			debug("ENV: %s: = %#x\n", map[index].env_name, val);
		} else {
			val = (u32)env_get_ulong(map[index].env_name, 10, 0);
			debug("ENV: %s: = %d\n", map[index].env_name, val);
		}

		map[index].val->val = val;
		map[index].val->set = true;
	}

	return 0;
}

static void env_clear_core(u32 index, const struct env_map_percpu *map)
{
	for (u32 i = 0; i < NR_CPUS; i++) {
		(*map[index].val)[i].val = 0;
		(*map[index].val)[i].set = false;
	}
}

static int env_read_core(u32 index, const struct env_map_percpu *map)
{
	u32 val;
	char command[MAX_CMD_LEN];

	for (u32 i = 0; i < NR_CPUS; i++) {
		sprintf(command, "%s_%u", map[index].env_name, i);
		if (!env_get_yesno(command)) {
			if (map[index].type == ENV_HEX) {
				val = (u32)env_get_hex(command, 0);
				debug("ENV: %s: = %#x\n", command, val);
			} else {
				val = (u32)env_get_ulong(command, 10, 0);
				debug("ENV: %s: = %d\n", command, val);
			}

			(*map[index].val)[i].val = val;
			(*map[index].val)[i].set = true;
		}
	}

	return 0;
}

static int env_validate_common(u32 index, const struct env_map_common *map)
{
	u32 value = map[index].val->val;
	bool set = map[index].val->set;
	u32 min = map[index].min;
	u32 max = map[index].max;

	/* Check if environment is mandatory */
	if (map[index].mandatory && !set) {
		pr_err("Variable \'%s\' is mandatory, but it is not defined\n",
		       map[index].env_name);

		return -EINVAL;
	}

	/* Check environment boundary */
	if (set && (value < min || value > max)) {
		if (map[index].type == ENV_HEX)
			pr_err("Variable \'%s\' must be between %#x and %#x\n",
			       map[index].env_name, min, max);
		else
			pr_err("Variable \'%s\' must be between %u and %u\n",
			       map[index].env_name, min, max);

		return -EINVAL;
	}

	return 0;
}

static int env_validate_core(u32 index, const struct env_map_percpu *map,
			     bool (*cpu_used)(u32))
{
	u32 value;
	bool set;
	bool mandatory = map[index].mandatory;
	u32 min, max;

	for (u32 i = 0; i < NR_CPUS; i++) {
		set = (*map[index].val)[i].set;
		value = (*map[index].val)[i].val;

		/* Check if environment is mandatory */
		if (cpu_used(i) && mandatory && !set) {
			pr_err("CPU %u is used, but \'%s_%u\' is not defined\n",
			       i, map[index].env_name, i);

			return -EINVAL;
		}

		min = map[index].min[i];
		max = map[index].max[i];

		/* Check environment boundary */
		if (set && (value < min || value > max)) {
			if (map[index].type == ENV_HEX)
				pr_err("Variable \'%s_%u\' must be between %#x and %#x\n",
				       map[index].env_name, i, min, max);
			else
				pr_err("Variable \'%s_%u\' must be between %d and %d\n",
				       map[index].env_name, i, min, max);

			return -EINVAL;
		}
	}

	return 0;
}

void envs_cleanup_core(const struct env_map_percpu *map)
{
	/* Cleanup env struct first */
	for (u32 i = 0; map[i].env_name; i++)
		env_clear_core(i, map);
}

void envs_cleanup_common(const struct env_map_common *map)
{
	/* Cleanup env struct first */
	for (u32 i = 0; map[i].env_name; i++)
		env_clear_common(i, map);
}

int envs_read_common(const struct env_map_common *map)
{
	int ret;

	for (u32 i = 0; map[i].env_name; i++) {
		ret = env_read_common(i, map);
		if (ret)
			return ret;
	}

	return 0;
}

int envs_validate_common(const struct env_map_common *map)
{
	int ret;

	for (u32 i = 0; map[i].env_name; i++) {
		ret = env_validate_common(i, map);
		if (ret)
			return ret;
	}

	return 0;
}

int envs_read_validate_common(const struct env_map_common *map)
{
	int ret;

	envs_cleanup_common(map);

	ret = envs_read_common(map);
	if (ret)
		return ret;

	ret = envs_validate_common(map);
	if (ret)
		return ret;

	return 0;
}

int envs_read_validate_core(const struct env_map_percpu *map,
			    bool (*cpu_used)(u32))
{
	int ret;

	envs_cleanup_core(map);

	for (u32 i = 0; map[i].env_name; i++) {
		ret = env_read_core(i, map);
		if (ret)
			return ret;
	}

	for (u32 i = 0; map[i].env_name; i++) {
		ret = env_validate_core(i, map, cpu_used);
		if (ret)
			return ret;
	}

	return 0;
}

int envs_process_and_validate(const struct env_map_common *common,
			      const struct env_map_percpu *core,
			      bool (*cpu_used)(u32))
{
	int ret;

	ret = envs_read_validate_common(common);
	if (ret)
		return ret;

	ret = envs_read_validate_core(core, cpu_used);
	if (ret)
		return ret;

	return 0;
}

static int args_envs_read_search(const struct env_map_common *map,
				 int argc, char *const argv[])
{
	for (int i = 0; map[i].env_name; i++) {
		if (!strcmp(argv[0], map[i].env_name))
			return i;
	}

	pr_err("Unexpected argument '%s', can't parse\n", argv[0]);

	return -ENOENT;
}

static int arg_read_set(const struct env_map_common *map, u32 i, int argc,
			char *const argv[])
{
	char *endp = argv[1];

	if (map[i].type == ENV_HEX)
		map[i].val->val = simple_strtoul(argv[1], &endp, 16);
	else
		map[i].val->val = simple_strtoul(argv[1], &endp, 10);

	map[i].val->set = true;

	if (*endp == '\0')
		return 0;

	pr_err("Unexpected argument '%s', can't parse\n", argv[1]);

	map[i].val->set = false;

	return -EINVAL;
}

int args_envs_enumerate(const struct env_map_common *map, int enum_by,
			int argc, char *const argv[])
{
	u32 i;

	if (argc % enum_by) {
		pr_err("unexpected argument number: %d\n", argc);
		return -EINVAL;
	}

	while (argc > 0) {
		i = args_envs_read_search(map, argc, argv);
		if (i < 0)
			return i;

		debug("ARG: found '%s' with index %d\n", map[i].env_name, i);

		if (i < 0) {
			pr_err("unknown arg: %s\n", argv[0]);
			return -EINVAL;
		}

		if (arg_read_set(map, i, argc, argv))
			return -EINVAL;

		debug("ARG: value.s '%s' == %#x\n", argv[1], map[i].val->val);

		argc -= enum_by;
		argv += enum_by;
	}

	return 0;
}
