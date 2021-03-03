#include "../../include/bpf_api.h"

/* Minimal, stand-alone toy map pinning example:
 *
 * clang -target bpf -O2 [...] -o bpf_shared.o -c bpf_shared.c
 * tc filter add dev foo parent 1: bpf obj bpf_shared.o sec egress
 * tc filter add dev foo parent ffff: bpf obj bpf_shared.o sec ingress
 *
 * Both classifier will share the very same map instance in this example,
 * so map content can be accessed from ingress *and* egress side!
 *
 * This example has a pinning of PIN_OBJECT_NS, so it's private and
 * thus shared among various program sections within the object.
 *
 * A setting of PIN_GLOBAL_NS would place it into a global namespace,
 * so that it can be shared among different object files. A setting
 * of PIN_NONE (= 0) means no sharing, so each tc invocation a new map
 * instance is being created.
 */

struct {
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__uint(key_size, sizeof(uint32_t));
	__uint(value_size, sizeof(uint32_t));
	__uint(max_entries, 1);
	__uint(pinning, LIBBPF_PIN_BY_NAME);	/* or LIBBPF_PIN_NONE */
} map_sh __section(".maps");

__section("egress")
int emain(struct __sk_buff *skb)
{
	int key = 0, *val;

	val = map_lookup_elem(&map_sh, &key);
	if (val)
		lock_xadd(val, 1);

	return BPF_H_DEFAULT;
}

__section("ingress")
int imain(struct __sk_buff *skb)
{
	int key = 0, *val;

	val = map_lookup_elem(&map_sh, &key);
	if (val)
		printt("map val: %d\n", *val);

	return BPF_H_DEFAULT;
}

BPF_LICENSE("GPL");
