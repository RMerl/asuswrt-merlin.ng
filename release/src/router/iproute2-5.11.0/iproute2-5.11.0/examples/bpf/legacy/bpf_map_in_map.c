#include "../../../include/bpf_api.h"

#define MAP_INNER_ID	42

struct bpf_elf_map __section_maps map_inner = {
	.type		= BPF_MAP_TYPE_ARRAY,
	.size_key	= sizeof(uint32_t),
	.size_value	= sizeof(uint32_t),
	.id		= MAP_INNER_ID,
	.inner_idx	= 0,
	.pinning	= PIN_GLOBAL_NS,
	.max_elem	= 1,
};

struct bpf_elf_map __section_maps map_outer = {
	.type		= BPF_MAP_TYPE_ARRAY_OF_MAPS,
	.size_key	= sizeof(uint32_t),
	.size_value	= sizeof(uint32_t),
	.inner_id	= MAP_INNER_ID,
	.pinning	= PIN_GLOBAL_NS,
	.max_elem	= 1,
};

__section("egress")
int emain(struct __sk_buff *skb)
{
	struct bpf_elf_map *map_inner;
	int key = 0, *val;

	map_inner = map_lookup_elem(&map_outer, &key);
	if (map_inner) {
		val = map_lookup_elem(map_inner, &key);
		if (val)
			lock_xadd(val, 1);
	}

	return BPF_H_DEFAULT;
}

__section("ingress")
int imain(struct __sk_buff *skb)
{
	struct bpf_elf_map *map_inner;
	int key = 0, *val;

	map_inner = map_lookup_elem(&map_outer, &key);
	if (map_inner) {
		val = map_lookup_elem(map_inner, &key);
		if (val)
			printt("map val: %d\n", *val);
	}

	return BPF_H_DEFAULT;
}

BPF_LICENSE("GPL");
