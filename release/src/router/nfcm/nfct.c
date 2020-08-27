#include <string.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <internal/internal.h>

#include "nfct.h"
#include "nfarp.h"
#if defined(QCA)
  #include "nffdb.h"
#elif defined(HND)
  #include "nfmc.h"
#else
  #include "nfrob.h"
#endif

//extern struct in_addr lan_addr;

bool find_dot_in_ip_str(char *ipstr)
{
    int i, len = strlen(ipstr);

    for (i=0;i<len;i++) {
        if (ipstr[i] == '.')
            return true;
    }
    return false;
}

static int nf_set_counters(nf_node_t *nn, const struct nf_conntrack *ct, int dir)
{
	if (dir == __DIR_ORIG) { // uplink
		nn->up_pkts += (unsigned long long) ct->counters[dir].packets;
		nn->up_bytes += (unsigned long long) ct->counters[dir].bytes;
	} else { // downlink
		nn->dn_pkts += (unsigned long long) ct->counters[dir].packets;
		nn->dn_bytes += (unsigned long long) ct->counters[dir].bytes;
	}

	return 0;
}

static int nf_set_proto(nf_node_t *nn, const struct __nfct_tuple *tuple)
{
    nn->proto = tuple->protonum;

	return 0;
}

static int nf_set_src_address_ipv4(nf_node_t *nn, const struct __nfct_tuple *tuple)
{
	memcpy(&nn->srcv4, &tuple->src.v4, sizeof(struct in_addr));

	return 0;
}

static int nf_set_src_address_ipv6(nf_node_t *nn, const struct __nfct_tuple *tuple)
{
	memcpy(&nn->srcv6, &tuple->src.v6, sizeof(struct in6_addr));

	return 0;
}

int nf_set_src_address(nf_node_t *nn, const struct __nfct_tuple *tuple)
{
	switch (tuple->l3protonum) {
	case AF_INET:
		nn->isv4 = true;
		nf_set_src_address_ipv4(nn, tuple);
		break;
	case AF_INET6:
		nn->isv4 = false;
		nf_set_src_address_ipv6(nn, tuple);
		break;
	}

	return 0;
}

int nf_set_src_port(nf_node_t *nn, const struct __nfct_tuple *tuple)
{
    switch (tuple->protonum) {
    case IPPROTO_TCP:
        nn->src_port = tuple->l4src.tcp.port;
        break;
    case IPPROTO_UDP:
        nn->src_port = tuple->l4src.udp.port;
        break;
    case IPPROTO_SCTP:
        nn->src_port = tuple->l4src.sctp.port;
        break;
    case IPPROTO_DCCP:
        nn->src_port = tuple->l4src.dccp.port;
        break;
    case IPPROTO_ICMP:
        nn->src_port = tuple->l4src.icmp.id;
        break;
    default:
        nn->src_port = tuple->l4src.all;
        break;
    }

    nn->src_port = ntohs(nn->src_port);

    return 0;
}

static int nf_set_dst_address_ipv4(nf_node_t *nn, const struct __nfct_tuple *tuple)
{
	memcpy(&nn->dstv4, &tuple->dst.v4, sizeof(struct in_addr));

	return 0;
}

static int nf_set_dst_address_ipv6(nf_node_t *nn, const struct __nfct_tuple *tuple)
{
	memcpy(&nn->dstv6, &tuple->dst.v6, sizeof(struct in6_addr));

	return 0;
}

int nf_set_dst_address(nf_node_t *nn, const struct __nfct_tuple *tuple)
{
	switch (tuple->l3protonum) {
	case AF_INET:
		nn->isv4 = true;
		nf_set_dst_address_ipv4(nn, tuple);
		break;
	case AF_INET6:
		nn->isv4 = false;
		nf_set_dst_address_ipv6(nn, tuple);
		break;
	}

	return 0;
}

int nf_set_dst_port(nf_node_t *nn, const struct __nfct_tuple *tuple)
{
    switch (tuple->protonum) {
    case IPPROTO_TCP:
        nn->dst_port = tuple->l4dst.tcp.port;
        break;
    case IPPROTO_UDP:
        nn->dst_port = tuple->l4dst.udp.port;
        break;
    case IPPROTO_SCTP:
        nn->dst_port = tuple->l4dst.sctp.port;
        break;
    case IPPROTO_DCCP:
        nn->dst_port = tuple->l4dst.dccp.port;
        break;
    //case IPPROTO_ICMP:
    //    nn->dst_port = (tuple->l4dst.icmp.type<<8) | tuple->l4dst.icmp.code;
    //    break;
    default:
        nn->dst_port = tuple->l4dst.all;
        break;
    }

    nn->dst_port = ntohs(nn->dst_port);

    return 0;
}

bool nf_set_layer1_info(nf_node_t *nn, struct list_head *arlist)
{
    arp_node_t *ar;

    list_for_each_entry(ar, arlist, list) {
        if (ar->isv4) {
            if (ar->srcv4.s_addr == nn->srcv4.s_addr) {
                nn->layer1_info.eth_type = (ar->iswl) ? PHY_WIRELESS : PHY_ETHER;
                nn->layer1_info.eth_port = ar->port;
                return true;
            }
        } else {
            if (!memcmp(&ar->srcv6, &nn->srcv6, sizeof(struct in6_addr))) {
                nn->layer1_info.eth_type = (ar->iswl) ? PHY_WIRELESS : PHY_ETHER;
                nn->layer1_info.eth_port = ar->port;
                return true;
            }
        }
    }

	return false;
}

nf_node_t *nf_node_new()
{
    nf_node_t *nn;

    nn = (nf_node_t *)calloc(1, sizeof(nf_node_t));
    if (!nn) return NULL;

    INIT_LIST_HEAD(&nn->list);

    return nn;
}

void nf_node_free(nf_node_t *nn)
{
	if (nn)
		free(nn);

	return;
}

nf_node_t *nf_list_search(const struct nf_conntrack *ct , struct list_head *iplist)
{
	nf_node_t *nn;

	list_for_each_entry(nn, iplist, list) {
		if(ct->head.orig.l3protonum == AF_INET) {
            if(nn->proto == ct->head.orig.protonum &&
               nn->srcv4.s_addr == ct->head.orig.src.v4 &&
               nn->dstv4.s_addr == ct->head.orig.dst.v4 &&
               nn->src_port == ct->head.orig.l4src.all &&
               nn->dst_port == ct->head.orig.l4dst.all) 
            {
                return nn;
            }
		} else if (ct->head.orig.l3protonum == AF_INET6) {
            if(nn->proto == ct->head.orig.protonum &&
               !memcmp(&nn->srcv6, &ct->head.orig.src.v6, sizeof(struct in6_addr)) &&
               !memcmp(&nn->dstv6, &ct->head.orig.dst.v6, sizeof(struct in6_addr)) &&
               nn->src_port == ct->head.orig.l4src.all &&
               nn->dst_port == ct->head.orig.l4dst.all)
            {
				return nn;
            }
		} else {
			printf("invalid proto [%d]\n", ct->head.orig.l3protonum);
            return NULL;
		}
	}

    return NULL;
}

static bool check_valid_state(const struct nf_conntrack *ct)
{
    if (ct->head.orig.protonum == IPPROTO_UDP) 
        return true;

    if (ct->head.orig.protonum == IPPROTO_TCP) { 
        if (test_bit(ATTR_TCP_STATE, ct->head.set)) {
            if(ct->protoinfo.tcp.state == TCP_CONNTRACK_ESTABLISHED)
                return true;
        } 
    }

    if (ct->head.orig.protonum == IPPROTO_SCTP) {
        if (test_bit(ATTR_SCTP_STATE, ct->head.set)) {
            if (ct->protoinfo.sctp.state == SCTP_CONNTRACK_ESTABLISHED) 
                return true;
        }
    }

    //if (test_bit(ATTR_DCCP_STATE, ct->head.set)) {
    //    if (ct->protoinfo.dccp.state != ) {
    //    }
    //} 

    return false;
}

int nf_process_conntrack(const struct nf_conntrack *ct,
						 struct list_head *iplist,
						 struct list_head *arlist)
{
	nf_node_t *nn;

    if (!check_valid_state(ct)) 
        return -1;

    nn = nf_list_search(ct, iplist);
    if (!nn) {
        nn = nf_node_new();
        list_add_tail(&nn->list, iplist);

        nn->up_speed = nn->dn_speed = 0;
        // uplink
        nf_set_proto(nn, &ct->head.orig);
        nf_set_src_address(nn, &ct->head.orig);
        nf_set_src_port(nn, &ct->head.orig);
        nf_set_dst_address(nn, &ct->head.orig);
        nf_set_dst_port(nn, &ct->head.orig);

        nf_set_layer1_info(nn, arlist);
    }

	if (test_bit(ATTR_ORIG_COUNTER_PACKETS, ct->head.set) &&
	    test_bit(ATTR_ORIG_COUNTER_BYTES, ct->head.set))
	{
		nf_set_counters(nn, ct, __DIR_ORIG);
	}

	if (test_bit(ATTR_REPL_COUNTER_PACKETS, ct->head.set) &&
	    test_bit(ATTR_REPL_COUNTER_BYTES, ct->head.set))
	{
		nf_set_counters(nn, ct, __DIR_REPL);
	}

	return 0;
}
