#include "../../include/bpf_api.h"

struct inner_map {
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__uint(key_size, sizeof(uint32_t));
	__uint(value_size, sizeof(uint32_t));
	__uint(max_entries, 1);
} map_inner __section(".maps");

struct {
	__uint(type, BPF_MAP_TYPE_ARRAY_OF_MAPS);
	__uint(key_size, sizeof(uint32_t));
	__uint(value_size, sizeof(uint32_t));
	__uint(max_entries, 1);
	__uint(pinning, LIBBPF_PIN_BY_NAME);
	__array(values, struct inner_map);
} map_outer __section(".maps") = {
	.values = {
		[0] = &map_inner,
	},
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
