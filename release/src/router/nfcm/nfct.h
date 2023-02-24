#ifndef __NFCT_H__
#define __NFCT_H__

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include "list.h"
#include "nfcm.h"

#define DEFAULT_IPV6_ADDR "fec0::1"

extern bool ct_tcp_is_valid(const struct nf_conntrack *ct);
extern int nf_conntrack_process(const struct nf_conntrack *ct, struct list_head *iplist,
                                struct list_head *clilist, struct list_head *arlist);

extern int nf_conntrack_tcp_process(const struct nf_conntrack *ct,
                                struct list_head *tcplist);
extern nf_node_t *nf_node_new();
extern void nf_node_free(nf_node_t *nn);
extern bool find_dot_in_ip_str(char *ipstr);
extern bool nf_set_layer1_info(nf_node_t *nn, struct list_head *clilist, struct list_head *arlist);


#endif // __NFCT_H__
