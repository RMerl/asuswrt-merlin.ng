#ifndef _NETLINK_H_
#define _NETLINK_H_

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

struct nf_conntrack;
struct nfct_handle;

struct nfct_handle *nl_init_event_handler(void);
struct nlif_handle *nl_init_interface_handler(void);

int nl_send_resync(struct nfct_handle *h);
void nl_resize_socket_buffer(struct nfct_handle *h);
int nl_dump_conntrack_table(struct nfct_handle *h);
int nl_flush_conntrack_table_selective(void);
int nl_get_conntrack(struct nfct_handle *h, const struct nf_conntrack *ct);
int nl_create_conntrack(struct nfct_handle *h, const struct nf_conntrack *ct, int timeout);
int nl_update_conntrack(struct nfct_handle *h, const struct nf_conntrack *ct, int timeout);
int nl_destroy_conntrack(struct nfct_handle *h, const struct nf_conntrack *ct);

static inline int ct_is_related(const struct nf_conntrack *ct)
{
	return (nfct_attr_is_set(ct, ATTR_MASTER_L3PROTO) &&
		nfct_attr_is_set(ct, ATTR_MASTER_L4PROTO) &&
		((nfct_attr_is_set(ct, ATTR_MASTER_IPV4_SRC) &&
		  nfct_attr_is_set(ct, ATTR_MASTER_IPV4_DST)) ||
		 (nfct_attr_is_set(ct, ATTR_MASTER_IPV6_SRC) &&
		  nfct_attr_is_set(ct, ATTR_MASTER_IPV6_DST))) &&
		nfct_attr_is_set(ct, ATTR_MASTER_PORT_SRC) &&
		nfct_attr_is_set(ct, ATTR_MASTER_PORT_DST));
}

int nl_create_expect(struct nfct_handle *h, const struct nf_expect *orig, int timeout);
int nl_destroy_expect(struct nfct_handle *h, const struct nf_expect *exp);
int nl_get_expect(struct nfct_handle *h, const struct nf_expect *exp);
int nl_dump_expect_table(struct nfct_handle *h);
int nl_flush_expect_table(struct nfct_handle *h);
int nl_send_expect_resync(struct nfct_handle *h);

#endif
