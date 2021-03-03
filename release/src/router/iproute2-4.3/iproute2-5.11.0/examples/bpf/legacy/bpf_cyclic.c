#include "../../../include/bpf_api.h"

/* Cyclic dependency example to test the kernel's runtime upper
 * bound on loops. Also demonstrates on how to use direct-actions,
 * loaded as: tc filter add [...] bpf da obj [...]
 */
#define JMP_MAP_ID	0xabccba

struct bpf_elf_map __section_maps jmp_tc = {
	.type		= BPF_MAP_TYPE_PROG_ARRAY,
	.id		= JMP_MAP_ID,
	.size_key	= sizeof(uint32_t),
	.size_value	= sizeof(uint32_t),
	.pinning	= PIN_OBJECT_NS,
	.max_elem	= 1,
};

__section_tail(JMP_MAP_ID, 0)
int cls_loop(struct __sk_buff *skb)
{
	printt("cb: %u\n", skb->cb[0]++);
	tail_call(skb, &jmp_tc, 0);

	skb->tc_classid = TC_H_MAKE(1, 42);
	return TC_ACT_OK;
}

__section_cls_entry
int cls_entry(struct __sk_buff *skb)
{
	tail_call(skb, &jmp_tc, 0);
	return TC_ACT_SHOT;
}

BPF_LICENSE("GPL");
