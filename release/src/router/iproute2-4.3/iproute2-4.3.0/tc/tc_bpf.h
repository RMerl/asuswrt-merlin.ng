/*
 * tc_bpf.h	BPF common code
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Daniel Borkmann <dborkman@redhat.com>
 *		Jiri Pirko <jiri@resnulli.us>
 */

#ifndef _TC_BPF_H_
#define _TC_BPF_H_ 1

#include <linux/filter.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/bpf.h>
#include <sys/syscall.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>

#include "utils.h"
#include "bpf_scm.h"

#define BPF_ENV_UDS	"TC_BPF_UDS"

int bpf_parse_string(char *arg, bool from_file, __u16 *bpf_len,
		     char **bpf_string, bool *need_release,
		     const char separator);
int bpf_parse_ops(int argc, char **argv, struct sock_filter *bpf_ops,
		  bool from_file);
void bpf_print_ops(FILE *f, struct rtattr *bpf_ops, __u16 len);

const char *bpf_default_section(const enum bpf_prog_type type);

#ifdef HAVE_ELF
int bpf_open_object(const char *path, enum bpf_prog_type type,
		    const char *sec, bool verbose);

int bpf_send_map_fds(const char *path, const char *obj);
int bpf_recv_map_fds(const char *path, int *fds, struct bpf_map_aux *aux,
		     unsigned int entries);

static inline __u64 bpf_ptr_to_u64(const void *ptr)
{
	return (__u64) (unsigned long) ptr;
}

static inline int bpf(int cmd, union bpf_attr *attr, unsigned int size)
{
#ifdef __NR_bpf
	return syscall(__NR_bpf, cmd, attr, size);
#else
	fprintf(stderr, "No bpf syscall, kernel headers too old?\n");
	errno = ENOSYS;
	return -1;
#endif
}
#else
static inline int bpf_open_object(const char *path, enum bpf_prog_type type,
				  const char *sec, bool verbose)
{
	fprintf(stderr, "No ELF library support compiled in.\n");
	errno = ENOSYS;
	return -1;
}

static inline int bpf_send_map_fds(const char *path, const char *obj)
{
	return 0;
}

static inline int bpf_recv_map_fds(const char *path, int *fds,
				   struct bpf_map_aux *aux,
				   unsigned int entries)
{
	return -1;
}
#endif /* HAVE_ELF */
#endif /* _TC_BPF_H_ */
