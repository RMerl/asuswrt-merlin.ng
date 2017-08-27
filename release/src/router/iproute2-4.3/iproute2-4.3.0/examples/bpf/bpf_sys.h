#ifndef __BPF_SYS__
#define __BPF_SYS__

#include <sys/syscall.h>
#include <linux/bpf.h>

static inline __u64 bpf_ptr_to_u64(const void *ptr)
{
	return (__u64) (unsigned long) ptr;
}

static inline int bpf_lookup_elem(int fd, void *key, void *value)
{
	union bpf_attr attr = {
		.map_fd		= fd,
		.key		= bpf_ptr_to_u64(key),
		.value		= bpf_ptr_to_u64(value),
	};

	return syscall(__NR_bpf, BPF_MAP_LOOKUP_ELEM, &attr, sizeof(attr));
}

#endif /* __BPF_SYS__ */
