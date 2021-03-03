/* SPDX-License-Identifier: GPL-2.0 */
#include "../../../include/bpf_api.h"

#define ENTRY_INIT	3
#define ENTRY_0		0
#define ENTRY_1		1
#define MAX_JMP_SIZE	2

#define FOO		42
#define BAR		43

/* This example doesn't really do anything useful, but it's purpose is to
 * demonstrate eBPF tail calls on a very simple example.
 *
 * cls_entry() is our classifier entry point, from there we jump based on
 * skb->hash into cls_case1() or cls_case2(). They are both part of the
 * program array jmp_tc. Indicated via __section_tail(), the tc loader
 * populates the program arrays with the loaded file descriptors already.
 *
 * To demonstrate nested jumps, cls_case2() jumps within the same jmp_tc
 * array to cls_case1(). And whenever we arrive at cls_case1(), we jump
 * into cls_exit(), part of the jump array jmp_ex.
 *
 * Also, to show it's possible, all programs share map_sh and dump the value
 * that the entry point incremented. The sections that are loaded into a
 * program array can be atomically replaced during run-time, e.g. to change
 * classifier behaviour.
 */

struct bpf_elf_map __section_maps jmp_tc = {
	.type		= BPF_MAP_TYPE_PROG_ARRAY,
	.id		= FOO,
	.size_key	= sizeof(uint32_t),
	.size_value	= sizeof(uint32_t),
	.pinning	= PIN_OBJECT_NS,
	.max_elem	= MAX_JMP_SIZE,
};

struct bpf_elf_map __section_maps jmp_ex = {
	.type		= BPF_MAP_TYPE_PROG_ARRAY,
	.id		= BAR,
	.size_key	= sizeof(uint32_t),
	.size_value	= sizeof(uint32_t),
	.pinning	= PIN_OBJECT_NS,
	.max_elem	= 1,
};

struct bpf_elf_map __section_maps map_sh = {
	.type		= BPF_MAP_TYPE_ARRAY,
	.size_key	= sizeof(uint32_t),
	.size_value	= sizeof(uint32_t),
	.pinning	= PIN_OBJECT_NS,
	.max_elem	= 1,
};

__section_tail(FOO, ENTRY_0)
int cls_case1(struct __sk_buff *skb)
{
	int key = 0, *val;

	val = map_lookup_elem(&map_sh, &key);
	if (val)
		printt("case1: map-val: %d from:%u\n", *val, skb->cb[0]);

	skb->cb[0] = ENTRY_0;
	tail_call(skb, &jmp_ex, ENTRY_0);

	return BPF_H_DEFAULT;
}

__section_tail(FOO, ENTRY_1)
int cls_case2(struct __sk_buff *skb)
{
	int key = 0, *val;

	val = map_lookup_elem(&map_sh, &key);
	if (val)
		printt("case2: map-val: %d from:%u\n", *val, skb->cb[0]);

	skb->cb[0] = ENTRY_1;
	tail_call(skb, &jmp_tc, ENTRY_0);

	return BPF_H_DEFAULT;
}

__section_tail(BAR, ENTRY_0)
int cls_exit(struct __sk_buff *skb)
{
	int key = 0, *val;

	val = map_lookup_elem(&map_sh, &key);
	if (val)
		printt("exit: map-val: %d from:%u\n", *val, skb->cb[0]);

	/* Termination point. */
	return BPF_H_DEFAULT;
}

__section_cls_entry
int cls_entry(struct __sk_buff *skb)
{
	int key = 0, *val;

	/* For transferring state, we can use skb->cb[0] ... skb->cb[4]. */
	val = map_lookup_elem(&map_sh, &key);
	if (val) {
		lock_xadd(val, 1);

		skb->cb[0] = ENTRY_INIT;
		tail_call(skb, &jmp_tc, skb->hash & (MAX_JMP_SIZE - 1));
	}

	printt("fallthrough\n");
	return BPF_H_DEFAULT;
}

BPF_LICENSE("GPL");
