/* SPDX-License-Identifier: GPL-2.0 */
/*
 * bpf_glue.c:	BPF code to call both legacy and libbpf code
 * Authors:	Hangbin Liu <haliu@redhat.com>
 *
 */
#include "bpf_util.h"
#ifdef HAVE_LIBBPF
#include <bpf/bpf.h>
#endif

int bpf_program_load(enum bpf_prog_type type, const struct bpf_insn *insns,
		     size_t size_insns, const char *license, char *log,
		     size_t size_log)
{
#ifdef HAVE_LIBBPF
	return bpf_load_program(type, insns, size_insns / sizeof(struct bpf_insn),
				license, 0, log, size_log);
#else
	return bpf_prog_load_dev(type, insns, size_insns, license, 0, log, size_log);
#endif
}

int bpf_program_attach(int prog_fd, int target_fd, enum bpf_attach_type type)
{
#ifdef HAVE_LIBBPF
	return bpf_prog_attach(prog_fd, target_fd, type, 0);
#else
	return bpf_prog_attach_fd(prog_fd, target_fd, type);
#endif
}

#ifdef HAVE_LIBBPF
static const char *_libbpf_compile_version = LIBBPF_VERSION;
static char _libbpf_version[10] = {};

const char *get_libbpf_version(void)
{
	/* Start by copying compile-time version into buffer so we have a
	 * fallback value in case we are dynamically linked, or can't find a
	 * version in /proc/self/maps below.
	 */
	strncpy(_libbpf_version, _libbpf_compile_version,
		sizeof(_libbpf_version)-1);
#ifdef LIBBPF_DYNAMIC
	char buf[PATH_MAX], *s;
	bool found = false;
	FILE *fp;

	/* When dynamically linking against libbpf, we can't be sure that the
	 * version we discovered at compile time is actually the one we are
	 * using at runtime. This can lead to hard-to-debug errors, so we try to
	 * discover the correct version at runtime.
	 *
	 * The simple solution to this would be if libbpf itself exported a
	 * version in its API. But since it doesn't, we work around this by
	 * parsing the mappings of the binary at runtime, looking for the full
	 * filename of libbpf.so and using that.
	 */
	fp = fopen("/proc/self/maps", "r");
	if (fp == NULL)
		goto out;

	while ((s = fgets(buf, sizeof(buf), fp)) != NULL) {
		if ((s = strstr(buf, "libbpf.so.")) != NULL) {
			strncpy(_libbpf_version, s+10, sizeof(_libbpf_version)-1);
			strtok(_libbpf_version, "\n");
			found = true;
			break;
		}
	}

	fclose(fp);
out:
	if (!found)
		fprintf(stderr, "Couldn't find runtime libbpf version - falling back to compile-time value!\n");
#endif /* LIBBPF_DYNAMIC */

	_libbpf_version[sizeof(_libbpf_version)-1] = '\0';
	return _libbpf_version;
}
#else
const char *get_libbpf_version(void)
{
	return NULL;
}
#endif /* HAVE_LIBBPF */
