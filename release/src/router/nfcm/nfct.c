#include <sys/types.h>
#include <string.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>
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

#include "nfcfg.h"

bool find_dot_in_ip_str(char *ipstr)
{
    int i, len = strlen(ipstr);

    for (i = 0; i < len; i++) {
        if (ipstr[i] == '.') return true;
    }
    return false;
}

static int nf_set_tcp_state(nf_node_t *nn, uint8_t state)
{
    nn->state = state;

    return 0;
}

static int nf_set_counters(nf_node_t *nn, const struct nf_conntrack *ct, int dir)
{
    if (dir == __DIR_ORIG) { // uplink
        nn->up_bytes += (unsigned long long)ct->counters[dir].bytes;
    } else { // downlink
        nn->dn_bytes += (unsigned long long)ct->counters[dir].bytes;
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

bool nf_set_layer1_info(nf_node_t *nn, struct list_head *clilist, struct list_head *arlist)
{
    arp_node_t *ar;

    list_for_each_entry(ar, arlist, list) {
        if (ar->is_v4) {
            if (ar->srcv4.s_addr == nn->srcv4.s_addr) {
                nn->layer1_info.eth_type = (ar->is_wl) ? PHY_WIRELESS : PHY_ETHER;
                nn->layer1_info.eth_port = ar->port;
                nn->layer1_info.is_guest = ar->is_guest;
                memcpy(nn->layer1_info.ifname, ar->ifname, IFNAMESIZE);
                return true;
            }
        } else {
            if (!memcmp(&ar->srcv6, &nn->srcv6, sizeof(struct in6_addr))) {
                nn->layer1_info.eth_type = (ar->is_wl) ? PHY_WIRELESS : PHY_ETHER;
                nn->layer1_info.eth_port = ar->port;
                nn->layer1_info.is_guest = ar->is_guest;
                memcpy(nn->layer1_info.ifname, ar->ifname, IFNAMESIZE);
                return true;
            }
        }
    }

    //TODO: get MAC/IP from /tmp/clientlist.json
#if defined(AIMESH)
    cli_node_t *cli;

    list_for_each_entry(cli, clilist, list) {
        if (cli->isv4) {
            if (cli->ipv4.s_addr == nn->srcv4.s_addr || !strcasecmp(cli->mac, nn->src_mac)) {
                nn->layer1_info.eth_type = client_list_get_phy_type(cli->type);
                nn->layer1_info.eth_port = client_list_get_phy_port(cli->type);
                nn->layer1_info.is_guest = false;
                memcpy(nn->layer1_info.ifname, "RE", IFNAMESIZE);
                return true;
            }
        } else {
//ipv6
        }
    }
#endif

    return false;
}

nf_node_t* nf_node_new()
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

nf_node_t* nf_list_search(const struct nf_conntrack *ct,
                          struct list_head *iplist)
{
    nf_node_t *nn;

    list_for_each_entry(nn, iplist, list) {
        if (ct->head.orig.l3protonum == AF_INET) {
            if (nn->proto == ct->head.orig.protonum &&
                nn->srcv4.s_addr == ct->head.orig.src.v4 &&
                nn->dstv4.s_addr == ct->head.orig.dst.v4 &&
                nn->src_port == ct->head.orig.l4src.all &&
                nn->dst_port == ct->head.orig.l4dst.all)
            {
                return nn;
            }
        } else if (ct->head.orig.l3protonum == AF_INET6) {
            if (nn->proto == ct->head.orig.protonum &&
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

static bool ct_check_valid_state(const struct nf_conntrack *ct)
{
    if (ct->head.orig.protonum == IPPROTO_UDP)
        return true;

    if (ct->head.orig.protonum == IPPROTO_TCP) {
        if (test_bit(ATTR_TCP_STATE, ct->head.set)) {
            if (ct->protoinfo.tcp.state == TCP_CONNTRACK_ESTABLISHED)
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

int nf_conntrack_process(const struct nf_conntrack *ct, struct list_head *iplist,
                         struct list_head *clilist, struct list_head *arlist)
{
    nf_node_t *nn;

    if (!ct_check_valid_state(ct)) 
        return -1;

    // try to search, it not found, create one
    nn = nf_list_search(ct, iplist); //, arlist);
    if (!nn) {
        // not found, new one
        nn = nf_node_new();
        if (!nn) 
            return -1;

        // if the src is router's ip, discard it
        nf_set_src_address(nn, &ct->head.orig);
        if (is_router_addr(&nn->srcv4)) {
            nf_node_free(nn);
            return -1;
        }

        list_add_tail(&nn->list, iplist);

        nn->up_speed = nn->dn_speed = 0;
        // uplink
        nf_set_proto(nn, &ct->head.orig);
        //nf_set_src_address(nn, &ct->head.orig);
        nf_set_src_port(nn, &ct->head.orig);
        nf_set_dst_address(nn, &ct->head.orig);
        nf_set_dst_port(nn, &ct->head.orig);

        //nf_set_layer1_info(nn, arlist);

#if defined(NFCMDGB)
        nf_node_dump(nn);
#endif
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

    nf_set_layer1_info(nn, clilist, arlist);

    return 0;
}

/*
const char *const tcp_states[TCP_CONNTRACK_MAX] = {
	[TCP_CONNTRACK_NONE]		= "NONE",
	[TCP_CONNTRACK_SYN_SENT]	= "SYN_SENT",
	[TCP_CONNTRACK_SYN_RECV]	= "SYN_RECV",
	[TCP_CONNTRACK_ESTABLISHED]	= "ESTABLISHED",
	[TCP_CONNTRACK_FIN_WAIT]	= "FIN_WAIT",
	[TCP_CONNTRACK_CLOSE_WAIT]	= "CLOSE_WAIT",
	[TCP_CONNTRACK_LAST_ACK]	= "LAST_ACK",
	[TCP_CONNTRACK_TIME_WAIT]	= "TIME_WAIT",
	[TCP_CONNTRACK_CLOSE]		= "CLOSE",
	[TCP_CONNTRACK_SYN_SENT2]	= "SYN_SENT2",
};
*/

bool ct_check_valid_tcp_state(const struct nf_conntrack *ct)
{
    if (test_bit(ATTR_TCP_STATE, ct->head.set)) {
        switch (ct->protoinfo.tcp.state) {
        case TCP_CONNTRACK_SYN_SENT:
        case TCP_CONNTRACK_SYN_RECV:
            return false;

        case TCP_CONNTRACK_ESTABLISHED:
            if((ct->counters[__DIR_ORIG].packets <= 3) || // SYN, ACK, CMD
               (ct->counters[__DIR_REPL].packets <= 2))   // SYN/ACK, ACK
            {
                return false;
            }
            break;

        case TCP_CONNTRACK_FIN_WAIT:
        case TCP_CONNTRACK_CLOSE_WAIT:
        case TCP_CONNTRACK_LAST_ACK:
        case TCP_CONNTRACK_TIME_WAIT:
        case TCP_CONNTRACK_SYN_SENT2:
            if((ct->counters[__DIR_ORIG].packets <= 5) || // SYN, ACK, CMD, ACK, RST/ACK
               (ct->counters[__DIR_REPL].packets <= 4))   // SYN/ACK, ACK, ERR_RESP, FIN/ACK
            {
                return false;
            }
            break;

        case TCP_CONNTRACK_CLOSE:
        default:
            return true;
        }
    }

    return true;
}

int nf_conntrack_tcp_process(const struct nf_conntrack *ct, 
                         struct list_head *tcplist)
{
    nf_node_t *nnt;

    if (ct->head.orig.protonum != IPPROTO_TCP)
        return -1;

#if __NFCMDBG_TCP__
    nf_node_t *nn;

    nn = nf_node_new();
    if (!nn) 
        return -1;

    // uplink
    nf_set_proto(nn, &ct->head.orig);
    nf_set_src_address(nn, &ct->head.orig);
    nf_set_src_port(nn, &ct->head.orig);
    nf_set_dst_address(nn, &ct->head.orig);
    nf_set_dst_port(nn, &ct->head.orig);

    if (test_bit(ATTR_TCP_STATE, ct->head.set)) {
        nf_set_tcp_state(nn, ct->protoinfo.tcp.state);
        printf("tcp_state=[%u]\n", nn->state);
    } else {
        printf("TCP_STATE is not set\n");
    }

    char ipstr[INET_ADDRSTRLEN];

    printf("ct->head.orig.protonum=[%d] ", ct->head.orig.protonum);
    inet_ntop(AF_INET, &nn->srcv4, ipstr, INET_ADDRSTRLEN);
    printf("src[%s][%u] --[%s][%llu][%llu]--> ", ipstr, nn->src_port,
           tcp_states[nn->state],
           ct->counters[__DIR_ORIG].packets,
           ct->counters[__DIR_REPL].packets);
    inet_ntop(AF_INET, &nn->dstv4, ipstr, INET_ADDRSTRLEN);
    printf("dst[%s][%u], ", ipstr, nn->dst_port);

    if ((ct->status & IPS_EXPECTED)) 
        printf("[EXPECTED] ");
    else
        printf("[UNEXPECTED] ");

    if (ct->status & IPS_SEEN_REPLY)
        printf("[REPLIED] ");
    else
        printf("[UNREPLIED] ");

    if (ct->status & IPS_ASSURED)
        printf("[ASSURED] ");
    else
        printf("[UNASSURED] ");

    if (ct->status & IPS_CONFIRMED)
        printf("[CONFIRMED] ");
    else
        printf("[UNCONFIRMED] ");

    printf("\n");

    nf_node_free(nn);
#endif

    if (ct_check_valid_tcp_state(ct))
        return -1;

    // not found, create one
    nnt = nf_node_new();
    if (!nnt) 
        return -1;

    // if the src is router's ip, discard it
    nf_set_src_address(nnt, &ct->head.orig);
    if (is_router_addr(&nnt->srcv4)) {
        nf_node_free(nnt);
        return -1;
    }

    list_add_tail(&nnt->list, tcplist);

    nf_set_proto(nnt, &ct->head.orig);
    nf_set_dst_address(nnt, &ct->head.orig);
    nf_set_dst_port(nnt, &ct->head.orig);

    nf_set_tcp_state(nnt, ct->protoinfo.tcp.state);

    nnt->timestamp = time(NULL);
    nnt->time_in = 0;

#if defined(NFCMDGB)
    nf_node_dump(nnt);
#endif

    return 0;
}

