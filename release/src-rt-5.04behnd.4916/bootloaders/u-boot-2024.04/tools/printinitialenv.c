// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022
 * Max Krummenacher, Toradex
 *
 * Snippets taken from tools/env/fw_env.c
 *
 * This prints the list of default environment variables as currently
 * configured.
 *
 */

#include <stdio.h>

/* Pull in the current config to define the default environment */
#include <linux/kconfig.h>

#ifndef __ASSEMBLY__
#define __ASSEMBLY__ /* get only #defines from config.h */
#include <config.h>
#undef	__ASSEMBLY__
#else
#include <config.h>
#endif

#define DEFAULT_ENV_INSTANCE_STATIC
#include <generated/environment.h>
#include <env_default.h>

int main(void)
{
	char *env, *nxt;

	for (env = default_environment; *env; env = nxt + 1) {
		for (nxt = env; *nxt; ++nxt) {
			if (nxt >= &default_environment[sizeof(default_environment)]) {
				fprintf(stderr, "## Error: environment not terminated\n");
				return -1;
			}
		}
		printf("%s\n", env);
	}
	return 0;
}
