#ifndef _LIBNETFILTER_CTTIMEOUT_H_
#define _LIBNETFILTER_CTTIMEOUT_H_

#include <stdint.h>
#include <sys/types.h>
#include <linux/netfilter/nfnetlink_conntrack.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nfct_timeout;

struct nfct_timeout *nfct_timeout_alloc(void);
void nfct_timeout_free(struct nfct_timeout *);

enum nfct_timeout_attr {
	NFCT_TIMEOUT_ATTR_NAME= 0,
	NFCT_TIMEOUT_ATTR_L3PROTO,
	NFCT_TIMEOUT_ATTR_L4PROTO,
	NFCT_TIMEOUT_ATTR_POLICY,
	NFCT_TIMEOUT_ATTR_MAX
};

enum nfct_timeout_tcp_attr {
	NFCT_TIMEOUT_ATTR_TCP_SYN_SENT = 0,
	NFCT_TIMEOUT_ATTR_TCP_SYN_RECV,
	NFCT_TIMEOUT_ATTR_TCP_ESTABLISHED,
	NFCT_TIMEOUT_ATTR_TCP_FIN_WAIT,
	NFCT_TIMEOUT_ATTR_TCP_CLOSE_WAIT,
	NFCT_TIMEOUT_ATTR_TCP_LAST_ACK,
	NFCT_TIMEOUT_ATTR_TCP_TIME_WAIT,
	NFCT_TIMEOUT_ATTR_TCP_CLOSE,
	NFCT_TIMEOUT_ATTR_TCP_SYN_SENT2,
	NFCT_TIMEOUT_ATTR_TCP_RETRANS,
	NFCT_TIMEOUT_ATTR_TCP_UNACK,
	NFCT_TIMEOUT_ATTR_TCP_MAX
};

enum nfct_timeout_udp_attr {
	NFCT_TIMEOUT_ATTR_UDP_UNREPLIED = 0,
	NFCT_TIMEOUT_ATTR_UDP_REPLIED,
	NFCT_TIMEOUT_ATTR_UDP_MAX
};

enum nfct_timeout_udplite_attr {
	NFCT_TIMEOUT_ATTR_UDPLITE_UNREPLIED = 0,
	NFCT_TIMEOUT_ATTR_UDPLITE_REPLIED,
	NFCT_TIMEOUT_ATTR_UDPLITE_MAX
};

enum nfct_timeout_dccp_attr {
	NFCT_TIMEOUT_ATTR_DCCP_REQUEST,
	NFCT_TIMEOUT_ATTR_DCCP_RESPOND,
	NFCT_TIMEOUT_ATTR_DCCP_PARTOPEN,
	NFCT_TIMEOUT_ATTR_DCCP_OPEN,
	NFCT_TIMEOUT_ATTR_DCCP_CLOSEREQ,
	NFCT_TIMEOUT_ATTR_DCCP_CLOSING,
	NFCT_TIMEOUT_ATTR_DCCP_TIMEWAIT,
	NFCT_TIMEOUT_ATTR_DCCP_MAX
};

enum nfct_timeout_sctp_attr {
	NFCT_TIMEOUT_ATTR_SCTP_CLOSED = 0,
	NFCT_TIMEOUT_ATTR_SCTP_COOKIE_WAIT,
	NFCT_TIMEOUT_ATTR_SCTP_COOKIE_ECHOED,
	NFCT_TIMEOUT_ATTR_SCTP_ESTABLISHED,
	NFCT_TIMEOUT_ATTR_SCTP_SHUTDOWN_SENT,
	NFCT_TIMEOUT_ATTR_SCTP_SHUTDOWN_RECD,
	NFCT_TIMEOUT_ATTR_SCTP_SHUTDOWN_ACK_SENT,
	NFCT_TIMEOUT_ATTR_SCTP_MAX
};

enum nfct_timeout_icmp_attr {
	NFCT_TIMEOUT_ATTR_ICMP = 0,
	NFCT_TIMEOUT_ATTR_ICMP_MAX
};

enum nfct_timeout_icmpv6_attr {
	NFCT_TIMEOUT_ATTR_ICMPV6 = 0,
	NFCT_TIMEOUT_ATTR_ICMPV6_MAX
};

enum nfct_timeout_gre_attr {
	NFCT_TIMEOUT_ATTR_GRE_UNREPLIED = 0,
	NFCT_TIMEOUT_ATTR_GRE_REPLIED,
	NFCT_TIMEOUT_ATTR_GRE_MAX
};

enum nfct_timeout_generic_attr {
	NFCT_TIMEOUT_ATTR_GENERIC = 0,
	NFCT_TIMEOUT_ATTR_GENERIC_MAX
};

int nfct_timeout_attr_set(struct nfct_timeout *t, uint32_t type, const void *data);
int nfct_timeout_attr_set_u8(struct nfct_timeout *t, uint32_t type, uint8_t data);
int nfct_timeout_attr_set_u16(struct nfct_timeout *t, uint32_t type, uint16_t data);
void nfct_timeout_attr_unset(struct nfct_timeout *t, uint32_t type);
const char *nfct_timeout_policy_attr_to_name(uint8_t l4proto, uint32_t state);

int nfct_timeout_policy_attr_set_u32(struct nfct_timeout *, uint32_t type, uint32_t data);
void nfct_timeout_policy_attr_unset(struct nfct_timeout *t, uint32_t type);

struct nlmsghdr;

struct nlmsghdr *nfct_timeout_nlmsg_build_hdr(char *buf, uint8_t cmd, uint16_t flags, uint32_t seq);
void nfct_timeout_nlmsg_build_payload(struct nlmsghdr *, const struct nfct_timeout *);
int nfct_timeout_nlmsg_parse_payload(const struct nlmsghdr *nlh, struct nfct_timeout *);

enum {
	NFCT_TIMEOUT_O_DEFAULT,
};

int nfct_timeout_snprintf(char *buf, size_t size, const struct nfct_timeout *, unsigned int type, unsigned int flags);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _LIBNETFILTER_CTTIMEOUT_H_ */
