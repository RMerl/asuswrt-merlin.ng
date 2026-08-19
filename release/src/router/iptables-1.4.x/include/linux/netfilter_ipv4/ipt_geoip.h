/* Header for the geoip match (tomato/xtables-addons heritage).
 * Restored: referenced by extensions/libipt_geoip.c but absent from tree. */
#ifndef _IPT_GEOIP_H
#define _IPT_GEOIP_H

#include <linux/types.h>

#define IPT_GEOIP_SRC  0x01
#define IPT_GEOIP_DST  0x02
#define IPT_GEOIP_INV  0x04

#define IPT_GEOIP_MAX  15

#define COUNTRY(cc) ((cc) >> 8), ((cc) & 0x00ff)

struct geoip_subnet {
	__u32 begin;
	__u32 end;
};

struct geoip_info {
	struct geoip_subnet *subnets;
	__u32 count;
	__u32 ref;
	__u16 cc;
	struct geoip_info *next;
	struct geoip_info *prev;
};

struct ipt_geoip_info {
	__u8  flags;
	__u8  count;
	__u16 cc[IPT_GEOIP_MAX];
	/* kernel-internal */
	struct geoip_info *mem[IPT_GEOIP_MAX];
	void *refcount;
	struct ipt_geoip_info *fini;
};

#endif /* _IPT_GEOIP_H */
