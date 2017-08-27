/*
 * e_bpf.c	BPF exec proxy
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Daniel Borkmann <daniel@iogearbox.net>
 */

#include <stdio.h>
#include <unistd.h>

#include "utils.h"

#include "tc_util.h"
#include "tc_bpf.h"

#include "bpf_elf.h"
#include "bpf_scm.h"

#define BPF_DEFAULT_CMD	"/bin/sh"

static char *argv_default[] = { BPF_DEFAULT_CMD, NULL };

static void explain(void)
{
	fprintf(stderr, "Usage: ... bpf [ import UDS_FILE ] [ run CMD ]\n\n");
	fprintf(stderr, "Where UDS_FILE provides the name of a unix domain socket file\n");
	fprintf(stderr, "to import eBPF maps and the optional CMD denotes the command\n");
	fprintf(stderr, "to be executed (default: \'%s\').\n", BPF_DEFAULT_CMD);
}

static int bpf_num_env_entries(void)
{
	char **envp;
	int num;

	for (num = 0, envp = environ; *envp != NULL; envp++)
		num++;
	return num;
}

static int parse_bpf(struct exec_util *eu, int argc, char **argv)
{
	char **argv_run = argv_default, **envp_run, *tmp;
	int ret, i, env_old, env_num, env_map;
	const char *bpf_uds_name = NULL;
	int fds[BPF_SCM_MAX_FDS];
	struct bpf_map_aux aux;

	if (argc == 0)
		return 0;

	while (argc > 0) {
		if (matches(*argv, "run") == 0) {
			NEXT_ARG();
			argv_run = argv;
			break;
		} else if (matches(*argv, "import") == 0 ||
			   matches(*argv, "imp") == 0) {
			NEXT_ARG();
			bpf_uds_name = *argv;
		} else {
			explain();
			return -1;
		}

		argc--;
		argv++;
	}

	if (!bpf_uds_name) {
		fprintf(stderr, "bpf: No import parameter provided!\n");
		explain();
		return -1;
	}

	if (argv_run != argv_default && argc == 0) {
		fprintf(stderr, "bpf: No run command provided!\n");
		explain();
		return -1;
	}

	memset(fds, 0, sizeof(fds));
	memset(&aux, 0, sizeof(aux));

	ret = bpf_recv_map_fds(bpf_uds_name, fds, &aux, ARRAY_SIZE(fds));
	if (ret < 0) {
		fprintf(stderr, "bpf: Could not receive fds!\n");
		return -1;
	}

	if (aux.num_ent == 0) {
		envp_run = environ;
		goto out;
	}

	env_old = bpf_num_env_entries();
	env_num = env_old + aux.num_ent + 2;
	env_map = env_old + 1;

	envp_run = malloc(sizeof(*envp_run) * env_num);
	if (!envp_run) {
		fprintf(stderr, "bpf: No memory left to allocate env!\n");
		goto err;
	}

	for (i = 0; i < env_old; i++)
		envp_run[i] = environ[i];

	ret = asprintf(&tmp, "BPF_NUM_MAPS=%u", aux.num_ent);
	if (ret < 0)
		goto err_free;

	envp_run[env_old] = tmp;

	for (i = env_map; i < env_num - 1; i++) {
		ret = asprintf(&tmp, "BPF_MAP%u=%u",
			       aux.ent[i - env_map].id,
			       fds[i - env_map]);
		if (ret < 0)
			goto err_free_env;

		envp_run[i] = tmp;
	}

	envp_run[env_num - 1] = NULL;
out:
	return execvpe(argv_run[0], argv_run, envp_run);

err_free_env:
	for (--i; i >= env_old; i--)
		free(envp_run[i]);
err_free:
	free(envp_run);
err:
	for (i = 0; i < aux.num_ent; i++)
		close(fds[i]);
	return -1;
}

struct exec_util bpf_exec_util = {
	.id = "bpf",
	.parse_eopt = parse_bpf,
};
