/*
 * Copyright (C) 2014 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <cutils/properties.h>

#include "hal-utils.h"

#define VALGRIND_BIN "/system/bin/valgrind"

#define BLUETOOTHD_BIN "/system/bin/bluetoothd-main"

static void run_valgrind(int debug, int mgmt_dbg)
{
	char *prg_argv[7];
	char *prg_envp[3];

	prg_argv[0] = VALGRIND_BIN;
	prg_argv[1] = "--leak-check=full";
	prg_argv[2] = "--track-origins=yes";
	prg_argv[3] = BLUETOOTHD_BIN;
	prg_argv[4] = debug ? "-d" : NULL;
	prg_argv[5] = mgmt_dbg ? "--mgmt-debug" : NULL;
	prg_argv[6] = NULL;

	prg_envp[0] = "G_SLICE=always-malloc";
	prg_envp[1] = "G_DEBUG=gc-friendly";
	prg_envp[2] = NULL;

	execve(prg_argv[0], prg_argv, prg_envp);
}

static void run_bluetoothd(int debug, int mgmt_dbg)
{
	char *prg_argv[4];
	char *prg_envp[1];

	prg_argv[0] = BLUETOOTHD_BIN;
	prg_argv[1] = debug ? "-d" : NULL;
	prg_argv[2] = mgmt_dbg ? "--mgmt-debug" : NULL;
	prg_argv[3] = NULL;

	prg_envp[0] = NULL;

	execve(prg_argv[0], prg_argv, prg_envp);
}

int main(int argc, char *argv[])
{
	char value[PROPERTY_VALUE_MAX];
	int debug = 0;
	int mgmt_dbg = 0;

	if (get_config("debug", value, NULL) > 0 &&
			(!strcasecmp(value, "true") || atoi(value) > 0))
		debug = 1;

	if (get_config("mgmtdbg", value, NULL) > 0 &&
			(!strcasecmp(value, "true") || atoi(value) > 0)) {
		debug = 1;
		mgmt_dbg = 1;
	}

	if (get_config("valgrind", value, NULL) > 0 &&
			(!strcasecmp(value, "true") || atoi(value) > 0))
		run_valgrind(debug, mgmt_dbg);

	/*
	 * In case we failed to execute Valgrind, try to run bluetoothd
	 * without it
	 */
	run_bluetoothd(debug, mgmt_dbg);

	return 0;
}
