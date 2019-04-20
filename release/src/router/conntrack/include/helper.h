#ifndef _CTD_HELPER_H_
#define _CTD_HELPER_H_

#include <stdint.h>
#include "linux_list.h"
#include "myct.h"

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

struct pkt_buff;

#define CTD_HELPER_NAME_LEN	16
#define CTD_HELPER_POLICY_MAX	4

struct ctd_helper_policy {
	char		name[CTD_HELPER_NAME_LEN];
	uint32_t	expect_timeout;
	uint32_t	expect_max;
};

struct ctd_helper {
	struct list_head	head;
	char			name[CTD_HELPER_NAME_LEN];
	uint8_t			l4proto;
	int			(*cb)(struct pkt_buff *pkt,
				      uint32_t protoff,
				      struct myct *ct,
				      uint32_t ctinfo);

	struct ctd_helper_policy policy[CTD_HELPER_POLICY_MAX];

	int			priv_data_len;
};

struct ctd_helper_instance {
	struct list_head	head;
	uint32_t		queue_num;
	uint32_t		queue_len;
	uint16_t		l3proto;
	uint8_t			l4proto;
	struct ctd_helper	*helper;
	struct ctd_helper_policy policy[CTD_HELPER_POLICY_MAX];
};

extern int cthelper_expect_init(struct nf_expect *exp, struct nf_conntrack *master, uint32_t class, union nfct_attr_grp_addr *saddr, union nfct_attr_grp_addr *daddr, uint8_t l4proto, uint16_t *sport, uint16_t *dport, uint32_t flags);
extern int cthelper_add_expect(struct nf_expect *exp);
extern int cthelper_del_expect(struct nf_expect *exp);

extern void cthelper_get_addr_src(struct nf_conntrack *ct, int dir, union nfct_attr_grp_addr *addr);
extern void cthelper_get_addr_dst(struct nf_conntrack *ct, int dir, union nfct_attr_grp_addr *addr);

void cthelper_get_port_src(struct nf_conntrack *ct, int dir, uint16_t *port);
void cthelper_get_port_dst(struct nf_conntrack *ct, int dir, uint16_t *port);

extern int in4_pton(const char *src, int srclen, uint8_t *dst, int delim, const char **end);
extern int in6_pton(const char *src, int srclen, uint8_t *dst, int delim, const char **end);

extern void helper_register(struct ctd_helper *helper);
struct ctd_helper *helper_find(const char *libdir_path, const char *name, uint8_t l4proto, int flags);

#define min_t(type, x, y) ({			\
	type __min1 = (x);			\
	type __min2 = (y);			\
	__min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({			\
	type __max1 = (x);			\
	type __max2 = (y);			\
	__max1 > __max2 ? __max1: __max2; })

enum ip_conntrack_dir {
	IP_CT_DIR_ORIGINAL,
	IP_CT_DIR_REPLY,
	IP_CT_DIR_MAX
};

#include <linux/netfilter/nf_conntrack_common.h>

#define CTINFO2DIR(ctinfo) ((ctinfo) >= IP_CT_IS_REPLY ? IP_CT_DIR_REPLY : IP_CT_DIR_ORIGINAL)

#if 0
#define pr_debug(fmt, arg...) \
	printf(fmt, ##arg)
#else
#define pr_debug(fmt, arg...) \
        ({ if (0) printf(fmt, ##arg); 0; })
#endif

#endif
