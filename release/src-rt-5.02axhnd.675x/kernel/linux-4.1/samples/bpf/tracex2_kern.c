/* Copyright (c) 2013-2015 PLUMgrid, http://plumgrid.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 */
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/version.h>
#include <uapi/linux/bpf.h>
#include "bpf_helpers.h"

struct bpf_map_def SEC("maps") my_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(long),
	.value_size = sizeof(long),
	.max_entries = 1024,
};

/* kprobe is NOT a stable ABI. If kernel internals change this bpf+kprobe
 * example will no longer be meaningful
 */
SEC("kprobe/kfree_skb")
int bpf_prog2(struct pt_regs *ctx)
{
	long loc = 0;
	long init_val = 1;
	long *value;

	/* x64 specific: read ip of kfree_skb caller.
	 * non-portable version of __builtin_return_address(0)
	 */
	bpf_probe_read(&loc, sizeof(loc), (void *)ctx->sp);

	value = bpf_map_lookup_elem(&my_map, &loc);
	if (value)
		*value += 1;
	else
		bpf_map_update_elem(&my_map, &loc, &init_val, BPF_ANY);
	return 0;
}

static unsigned int log2(unsigned int v)
{
	unsigned int r;
	unsigned int shift;

	r = (v > 0xFFFF) << 4; v >>= r;
	shift = (v > 0xFF) << 3; v >>= shift; r |= shift;
	shift = (v > 0xF) << 2; v >>= shift; r |= shift;
	shift = (v > 0x3) << 1; v >>= shift; r |= shift;
	r |= (v >> 1);
	return r;
}

static unsigned int log2l(unsigned long v)
{
	unsigned int hi = v >> 32;
	if (hi)
		return log2(hi) + 32;
	else
		return log2(v);
}

struct bpf_map_def SEC("maps") my_hist_map = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(u32),
	.value_size = sizeof(long),
	.max_entries = 64,
};

SEC("kprobe/sys_write")
int bpf_prog3(struct pt_regs *ctx)
{
	long write_size = ctx->dx; /* arg3 */
	long init_val = 1;
	long *value;
	u32 index = log2l(write_size);

	value = bpf_map_lookup_elem(&my_hist_map, &index);
	if (value)
		__sync_fetch_and_add(value, 1);
	return 0;
}
char _license[] SEC("license") = "GPL";
u32 _version SEC("version") = LINUX_VERSION_CODE;
