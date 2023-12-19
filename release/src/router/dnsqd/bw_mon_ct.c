#include "bw_mon_ct.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>

#include "dnsarp.h"
#include "nfcfg.h"
#include "dnssql.h"


/*
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/types.h>
#include <string.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>
#include <libnetfilter_conntrack_tcp.h>
#include <internal/internal.h>
*/

extern sqlite3 *dns_db;

#define NFCT_ACCT_FILE "/proc/sys/net/netfilter/nf_conntrack_acct"

struct u32_mask {
    uint32_t value;
    uint32_t mask;
};


/* These are the template objects that are used to send commands. */
static struct {
    struct nf_conntrack *ct;
    struct nf_expect *exp;
    /* Expectations require the expectation tuple and the mask. */
    struct nf_conntrack *exptuple, *mask;

    /* Allows filtering/setting specific bits in the ctmark */
    struct u32_mask mark;

    /* Allow to filter by mark from kernel-space. */
    struct nfct_filter_dump_mark filter_mark_kernel;
    bool filter_mark_kernel_set;

    /* Allows filtering by ctlabels */
    struct nfct_bitmask *label;

    /* Allows setting/removing specific ctlabels */
    struct nfct_bitmask *label_modify;
} tmpl;

static struct nfct_handle *cth;
struct nfct_filter_dump *filter_dump;

static bool nf_set_ct_acct_flags(char *fname)
{
    char cmd[64];

    sprintf(cmd, "echo 1 > %s", fname);
    system(cmd);

    return true;
}


static void free_tmpl_objects(void)
{
    if (tmpl.ct) nfct_destroy(tmpl.ct);
    if (tmpl.exptuple) nfct_destroy(tmpl.exptuple);
    if (tmpl.mask) nfct_destroy(tmpl.mask);
    if (tmpl.exp) nfexp_destroy(tmpl.exp);
    if (tmpl.label) nfct_bitmask_destroy(tmpl.label);
    if (tmpl.label_modify) nfct_bitmask_destroy(tmpl.label_modify);
}


static int alloc_tmpl_objects(void)
{
    tmpl.ct = nfct_new();
    tmpl.exptuple = nfct_new();
    tmpl.mask = nfct_new();
    tmpl.exp = nfexp_new();

    memset(&tmpl.mark, 0, sizeof(tmpl.mark));

    return tmpl.ct != NULL && tmpl.exptuple != NULL &&
           tmpl.mask != NULL && tmpl.exp != NULL;
}

uint16_t get_port_from_ct(struct nf_conntrack *ct, int src_or_dst)
{
  struct __nfct_tuple *tuple = &ct->head.orig;
  uint16_t port;
  switch (tuple->protonum) {
    case IPPROTO_TCP:
        port  = src_or_dst == 0 ? tuple->l4src.tcp.port:tuple->l4dst.tcp.port;
        break;
    case IPPROTO_UDP:
        port = src_or_dst == 0 ? tuple->l4src.udp.port:tuple->l4dst.udp.port;
        break;
    case IPPROTO_SCTP:
        port = src_or_dst == 0 ? tuple->l4src.sctp.port:tuple->l4dst.sctp.port;
        break;
    case IPPROTO_DCCP:
        port = src_or_dst == 0 ? tuple->l4src.dccp.port:tuple->l4dst.dccp.port;
        break;
    //case IPPROTO_ICMP:
    //    nn->dst_port = (tuple->l4dst.icmp.type<<8) | tuple->l4dst.icmp.code;
    //    break;
    default:
        port = src_or_dst == 0 ? tuple->l4src.all:tuple->l4dst.all;
        break;
    }

    port = ntohs(port);

    return port;
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

bw_nf_node_t* nf_list_search(const struct nf_conntrack *ct,
                          struct list_head *iplist)
{
    bw_nf_node_t *nn;

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
            //printf("invalid proto [%d]\n", ct->head.orig.l3protonum);
            return NULL;
        }
    }

    return NULL;
}


bw_nf_node_t* nf_node_new()
{
    bw_nf_node_t *nn;

    nn = (bw_nf_node_t *)calloc(1, sizeof(bw_nf_node_t));
    if (!nn) return NULL;

    INIT_LIST_HEAD(&nn->list);

    return nn;
}

void nf_node_free(bw_nf_node_t *nn)
{
    if (nn)
        free(nn);

    return;
}

static int nf_set_src_address_ipv4(bw_nf_node_t *nn, const struct __nfct_tuple *tuple)
{
    memcpy(&nn->srcv4, &tuple->src.v4, sizeof(struct in_addr));
    //just for db can be inserted
    strcpy(nn->src6_ip, DEFAULT_IPV6_ADDR);
    return 0;
}

static int nf_set_src_address_ipv6(bw_nf_node_t *nn, const struct __nfct_tuple *tuple)
{
    memcpy(&nn->srcv6, &tuple->src.v6, sizeof(struct in6_addr));
    inet_ntop(AF_INET6, &tuple->src.v6, nn->src6_ip, INET6_ADDRSTRLEN);
    return 0;
}

int nf_set_src_address(bw_nf_node_t *nn, const struct __nfct_tuple *tuple)
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

static int nf_set_dst_address_ipv4(bw_nf_node_t *nn, const struct __nfct_tuple *tuple)
{
    memcpy(&nn->dstv4, &tuple->dst.v4, sizeof(struct in_addr));
    //just for db can be inserted
    strcpy(nn->dst6_ip, DEFAULT_IPV6_ADDR);
    return 0;
}

static int nf_set_dst_address_ipv6(bw_nf_node_t *nn, const struct __nfct_tuple *tuple)
{
    memcpy(&nn->dstv6, &tuple->dst.v6, sizeof(struct in6_addr));
    inet_ntop(AF_INET6, &tuple->dst.v6, nn->dst6_ip, INET6_ADDRSTRLEN);
    return 0;
}

int nf_set_dst_address(bw_nf_node_t *nn, const struct __nfct_tuple *tuple)
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


static int nf_set_proto(bw_nf_node_t *nn, const struct __nfct_tuple *tuple)
{
    nn->proto = tuple->protonum;

    return 0;
}

int nf_set_src_port(bw_nf_node_t *nn, const struct __nfct_tuple *tuple)
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
int nf_set_dst_port(bw_nf_node_t *nn, const struct __nfct_tuple *tuple)
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

static int nf_set_counters(bw_nf_node_t *nn, const struct nf_conntrack *ct, int dir)
{
    if (dir == __DIR_ORIG) { // uplink
        nn->up_bytes = (unsigned long long)ct->counters[dir].bytes;
    } else { // downlink
        nn->dn_bytes = (unsigned long long)ct->counters[dir].bytes;
    }

    return 0;
}

bool nf_set_layer1_info(bw_nf_node_t *nn, struct list_head *clilist, struct list_head *arlist)
{
    arp_node_t *ar;

    list_for_each_entry(ar, arlist, list) {
        if (ar->isv4) {
            if (ar->srcv4.s_addr == nn->srcv4.s_addr) {
                //nn->layer1_info.eth_type = (ar->is_wl) ? PHY_WIRELESS : PHY_ETHER;
                //nn->layer1_info.eth_port = ar->port;
                //nn->layer1_info.is_guest = ar->is_guest;
                //memcpy(nn->layer1_info.ifname, ar->ifname, IFNAMESIZE);
                memcpy(nn->src_mac, ar->mac, ETHER_ADDR_LENGTH);            
                return true;
            }
        } else {
            if (!memcmp(&ar->srcv6, &nn->srcv6, sizeof(struct in6_addr))) {
                //nn->layer1_info.eth_type = (ar->is_wl) ? PHY_WIRELESS : PHY_ETHER;
                //nn->layer1_info.eth_port = ar->port;
                //nn->layer1_info.is_guest = ar->is_guest;
                //memcpy(nn->layer1_info.ifname, ar->ifname, IFNAMESIZE);
                memcpy(nn->src_mac, ar->mac, ETHER_ADDR_LENGTH);            
                return true;
            }
        }
    }
    //fall through to lookup mac from /tmp/clientlist.json
#if 1
    cli_node_t *cli;

    list_for_each_entry(cli, clilist, list) {
        if (cli->isv4) {
            //if (cli->ipv4.s_addr == nn->srcv4.s_addr || !strcasecmp(cli->mac, nn->src_mac)) {
            if (cli->ipv4.s_addr == nn->srcv4.s_addr) {
                //nn->layer1_info.eth_type = client_list_get_phy_type(cli->type);
                //nn->layer1_info.eth_port = client_list_get_phy_port(cli->type);
                //nn->layer1_info.is_guest = false;
                //memcpy(nn->layer1_info.ifname, "RE", IFNAMESIZE);
                memcpy(nn->src_mac, cli->mac, ETHER_ADDR_LENGTH);            
                return true;
            }
        } else {
//ipv6
       //     if (!memcmp(&cli->ipv6, &nn->srcv6, sizeof(struct in6_addr)) || !strcasecmp(cli->mac, nn->src_mac)) {
            if (!memcmp(&cli->ipv6, &nn->srcv6, sizeof(struct in6_addr))) {
                //nn->layer1_info.eth_type = client_list_get_phy_type(cli->type);
                //nn->layer1_info.eth_port = client_list_get_phy_port(cli->type);
                //nn->layer1_info.is_guest = false;
                //memcpy(nn->layer1_info.ifname, "RE", IFNAMESIZE);
                memcpy(nn->src_mac, cli->mac, ETHER_ADDR_LENGTH);            
                return true;
            }  
        }
    }
#endif

    return false;
}


void nf_node_dump(bw_nf_node_t *nn)
{
    char ipstr[INET6_ADDRSTRLEN];
    char ethtype[10];

    if (nn->isv4) {
         //if (!is_acceptable_addr(nn)) {
         //   return;
         //}
        printf("proto:  %d\n", nn->proto);
        inet_ntop(AF_INET, &nn->srcv4, ipstr, INET_ADDRSTRLEN);
        printf("src:	%s[%u]\n", ipstr, nn->srcv4.s_addr);
        printf("port:   %d\n", nn->src_port);
        printf("src_mac:%s\n", nn->src_mac);
        inet_ntop(AF_INET, &nn->dstv4, ipstr, INET_ADDRSTRLEN);
        printf("dst:	%s[%u]\n", ipstr, nn->dstv4.s_addr);
        printf("port:   %d\n", nn->dst_port);
    } else {
        printf("proto:  %d\n", nn->proto);
        inet_ntop(AF_INET6, &nn->srcv6, ipstr, INET6_ADDRSTRLEN);
        printf("src:	%s\n", ipstr);
        printf("port:   %d\n", nn->src_port);
        printf("src_mac:%s\n", nn->src_mac);
        inet_ntop(AF_INET6, &nn->dstv6, ipstr, INET6_ADDRSTRLEN);
        printf("dst:	%s\n", ipstr);
        printf("port:   %d\n", nn->dst_port);
    }

        printf("up_bytes:       %llu\n", nn->up_bytes);

        printf("dn_bytes:       %llu\n", nn->dn_bytes);

        printf("app_name:       %s\n", nn->app_name);

        printf("app_id:       %d\n", nn->app_id);

        printf("cat_id:       %d\n", nn->cat_id);

        printf("app_checked:       %d\n", nn->is_app_checked);
        
        printf("-----------------------\n");

    return;
}


void nf_list_free(struct list_head *list)
{
    bw_nf_node_t * nn,*nnt;

    list_for_each_entry_safe(nn, nnt, list, list) {
        list_del(&nn->list);
        nf_node_free(nn);
    }
    return;
}

int nf_list_size(struct list_head *list)
{
    bw_nf_node_t *nn;
    int cnt = 0;

    list_for_each_entry(nn, list, list) {
        cnt++;
    }

    return cnt;
}

void nf_list_move(struct list_head *dst, struct list_head *src)
{
    bw_nf_node_t * nn,*nnt;

    list_for_each_entry_safe(nn, nnt, src, list) {
        list_move_tail(&nn->list, dst);
    }

}

void nf_list_dump(char *title, struct list_head *list)
{
    bw_nf_node_t *nn;

    printf("[%s]%s: %s, count=[%d]\n", __FILE__, __FUNCTION__, title, nf_list_size(list));
    list_for_each_entry(nn, list, list) {
        nf_node_dump(nn);
    }
    printf("=======================\n");

    return;
}


bool is_acceptable_addr(bw_nf_node_t *nn)
{

 if(nn->isv4) {
    if (!is_in_lanv4(&nn->srcv4))
        return false;

    if (is_broadcast_addr(&nn->dstv4))
        return false;

    if (is_router_addr(&nn->srcv4) || is_router_addr(&nn->dstv4))
        return false;

    if (is_multi_addr(&nn->dstv4))
        return false;

    if (nn->proto == IPPROTO_UDP) {
        if (nn->dst_port == 53)
            return false;
    }

    return true;
  } else {
    
    if (!is_in_lanv6(&nn->srcv6))
    {   
     return false;
    }
    if (is_local_link_addr(&nn->srcv6) || is_local_link_addr(&nn->dstv6))
        return false;

    if (is_router_v6addr(&nn->srcv6))
        return false;
    
    if (nn->proto == IPPROTO_UDP) {
        if (nn->dst_port == 53)
            return false;
    }
    return true;
  }
}

// compare the nn and nnt node according to five tuple
bool bw_nf_node_compare(bool v4, bw_nf_node_t *nn, bw_nf_node_t *nnt)
{
    if (v4) {
        if (nnt->srcv4.s_addr == nn->srcv4.s_addr &&
            nnt->dstv4.s_addr == nn->dstv4.s_addr &&
            nnt->src_port == nn->src_port &&
            nnt->dst_port == nn->dst_port &&
            nnt->proto == nn->proto)
        {
            return true;
        }
    } else {
        if (!memcmp(&nn->srcv6, &nnt->srcv6, sizeof(struct in6_addr)) &&
            !memcmp(&nn->dstv6, &nnt->dstv6, sizeof(struct in6_addr)) &&
            nnt->src_port == nn->src_port &&
            nnt->dst_port == nn->dst_port &&
            nnt->proto == nn->proto)
        {
            return true;
        }
    }

    return false;
}



int bw_nf_conntrack_process(const struct nf_conntrack *ct, struct list_head *iplist, 
                            struct list_head *clilist, struct list_head *arlist)
{
    bw_nf_node_t *nn;

    if (!ct_check_valid_state(ct)) 
        return -1;


#if 0
  char ipstr[INET_ADDRSTRLEN];
  char ipdst[INET_ADDRSTRLEN];
  
  if (ct->head.orig.l3protonum == AF_INET)
  {
    inet_ntop(AF_INET,&ct->head.orig.src.v4, ipstr, INET_ADDRSTRLEN);
    inet_ntop(AF_INET,&ct->head.orig.dst.v4, ipdst, INET_ADDRSTRLEN);
  
    // rtsp.stream and wowzaec2demo.streamlock.net
    printf("id[%lu] to[%lu] status[%lu] src[%s][%u] dst[%s][%u] --[%llu][%llu] --[%llu][%llu]\n", 
    ct->id, ct->timeout, ct->status,
    ipstr, get_port_from_ct(ct, 0), 
          ipdst, get_port_from_ct(ct, 1), 
    ct->counters[__DIR_ORIG].packets,
          ct->counters[__DIR_REPL].packets,
    ct->counters[__DIR_ORIG].bytes,
          ct->counters[__DIR_REPL].bytes);
  
  }
#endif 

    // try to search, it not found, create one    
    nn = nf_list_search(ct, iplist);
#if 1
    if (!nn) {
        // not found, new one
        nn = nf_node_new();
        if (!nn) 
            return -1;

        // if the src is router's ip, discard it
        nf_set_src_address(nn, &ct->head.orig);
        nf_set_proto(nn, &ct->head.orig);
        nf_set_src_port(nn, &ct->head.orig);
        nf_set_dst_address(nn, &ct->head.orig);
        nf_set_dst_port(nn, &ct->head.orig);

        /*
        if (nn->isv4 && (is_router_addr(&nn->srcv4) || !nf_set_layer1_info(nn, arlist))) {
            nf_node_free(nn);
            return -1;
        }

        if (!nn->isv4 && (is_local_link_addr(&nn->srcv6) || !is_in_lanv6(&nn->srcv6) || !nf_set_layer1_info(nn, arlist))) {
          nf_node_free(nn);
            return -1;
        }
        */
        if(!is_acceptable_addr(nn) || !nf_set_layer1_info(nn, clilist, arlist)) {
            nf_node_free(nn);
            return -1;
        }
        list_add_tail(&nn->list, iplist);

        //nn->up_speed = nn->dn_speed = 0;
        // uplink
        
#if defined(NFCMDGB)
        nf_node_dump(nn);
#endif
    }

    //if (test_bit(ATTR_ORIG_COUNTER_PACKETS, ct->head.set) &&
    //    test_bit(ATTR_ORIG_COUNTER_BYTES, ct->head.set))
    //{
        nf_set_counters(nn, ct, __DIR_ORIG);
    //}

    //if (test_bit(ATTR_REPL_COUNTER_PACKETS, ct->head.set) &&
    //    test_bit(ATTR_REPL_COUNTER_BYTES, ct->head.set))
    //{
        nf_set_counters(nn, ct, __DIR_REPL);
    //}
    if(!nn->is_app_checked) {
      if(dns_db!=NULL) {
        sqlite_app_name_lookup(dns_db, nn->isv4, nn->src_mac, (u_int32_t) nn->dstv4.s_addr, nn->dst6_ip, nn->dst_port, &nn->app_id, &nn->cat_id, nn->app_name);
      }
      nn->is_app_checked = 1;
    }
    //nf_node_dump(nn);

#endif   
    return 0;
}


void bw_nf_list_diff_calc(struct list_head *iplist, struct list_head *bklist)
{
    bw_nf_node_t *nn,*nnt;
    int64_t diff;
    bool bFound = false;
    
    list_for_each_entry(nn, iplist, list) {
        list_for_each_entry(nnt, bklist, list) {
            if (bw_nf_node_compare(nn->isv4, nn, nnt)) {
                // five tuples are the same
                diff = nn->up_bytes - nnt->up_bytes;
                nn->up_dif_bytes = (diff < 0) ? 0 : diff;
                
                diff = nn->dn_bytes - nnt->dn_bytes;
                nn->dn_dif_bytes = (diff < 0) ? 0 : diff;
                
                bFound = true;
                break;
            }
        } // bklist

        if (!bFound) { // there is not the same node in bklist
            nn->up_dif_bytes = (nn->up_bytes < 0) ? 0 : nn->up_bytes;
            nn->dn_dif_bytes = (nn->dn_bytes < 0) ? 0 : nn->dn_bytes;
        }
        bFound = false;
    } // iplist

    return;
}

bw_nf_node_t*  bw_sum_list_search(char *mac, int app_id, struct list_head *bw_sum_list)
{
    bw_nf_node_t *nn;
    list_for_each_entry(nn, bw_sum_list, list) {
      if (!strcmp(nn->src_mac, mac) && nn->app_id == app_id)
      {
          return nn;
      }
    }
    return NULL;
}


int bw_nf_list_statistics_calc(struct list_head *iplist, struct list_head *smlist)
{
    bw_nf_node_t *nn = NULL;
    bw_nf_node_t *nnt;
  
    // phase 0 - mark all mark as 0
    list_for_each_entry(nnt, smlist, list) {
      nnt->available = 0;
    }
    // phase 1 - accumluate available ct (diff) to existing summary ct
    list_for_each_entry(nnt, iplist, list) {
        nn = bw_sum_list_search(nnt->src_mac, nnt->app_id, smlist);
        if (nn == NULL) {
            nn = nf_node_new();
            nn->isv4 = nnt->isv4;
            strcpy(&nn->src_mac, nnt->src_mac);
            mempcpy(&nn->srcv4, &nnt->srcv4, sizeof(struct in_addr));
            memcpy(&nn->srcv6, &nnt->srcv6, sizeof(struct in6_addr));
            nn->up_bytes = nnt->up_bytes;
            //uint64_t up_dif_bytes;
            nn->dn_bytes = nnt->dn_bytes;
            //uint64_t dn_dif_bytes;
            nn->is_app_checked = nnt->is_app_checked;
            strcpy(nn->app_name, nnt->app_name);
            nn->app_id = nnt->app_id;
            nn->cat_id = nnt->cat_id;
            list_add_tail(&nn->list, smlist);
        }
        else {
            nn->up_bytes += nnt->up_dif_bytes;
            nn->dn_bytes += nnt->dn_dif_bytes;
        }
        nn->available = 1;
    }

  // phase 2 - remove node not present in new ct from summary ct
    list_for_each_entry_safe(nn, nnt, smlist, list) {
        if (!nn->available) {
            list_del(&nn->list);
            nf_node_free(nn);
        }
    }
    return 0;
}

struct json_object* bw_ct_node_to_json(bw_nf_node_t *node)
{   
    struct json_object *node_obj = json_object_new_object();
    if (node_obj == NULL)  {
        //printf("new json object failed.\n");
        return NULL;
    }
   
    json_object_object_add(node_obj,"mac", json_object_new_string(node->src_mac));
    json_object_object_add(node_obj, "app", json_object_new_string(node->app_name));
    json_object_object_add(node_obj, "tx", json_object_new_int64(node->up_bytes));
    json_object_object_add(node_obj, "rx", json_object_new_int64(node->dn_bytes));
    json_object_object_add(node_obj, "app_id", json_object_new_int(node->app_id));
    json_object_object_add(node_obj, "cat_id", json_object_new_int(node->cat_id));

    return node_obj;  
}


int bw_ct_list_to_json(struct list_head *bw_ct_list)
{
    int lock = 0;
    bw_nf_node_t *node;

    struct json_object *rootObj = json_object_new_object();
    struct json_object *json_obj;
    struct json_object *ary_obj = json_object_new_array();
    
    if (rootObj == NULL) return -1;    
    if (ary_obj == NULL) {
      json_object_put(rootObj);
      return -1;
    }

    // dump to json file based on tcplist
    list_for_each_entry(node, bw_ct_list, list) {
        json_obj = bw_ct_node_to_json(node);
        json_object_array_add(ary_obj, json_obj);
    }
    
    json_object_object_add(rootObj, "ts", json_object_new_int(time(NULL)));
    json_object_object_add(rootObj, "contents", ary_obj);
 
    lock = file_lock(DNS_BW_MON_TRAF_LOCK_NAME);
    if(lock) {
      json_object_to_file(JSON_BW_MON_TRAFFIC_FILE, rootObj);
      file_unlock(lock);
    }
    // free the root object will also free its descending tree nodes
    json_object_put(rootObj);

    return 0;

}


int init_bw_mon_ct() 
{
  int is_OK = 0;
  int family = AF_UNSPEC;

  nf_set_ct_acct_flags(NFCT_ACCT_FILE);

  /* we release these objects in the exit_error() path. */
  if (!alloc_tmpl_objects())
      goto err;
  cth = nfct_open(CONNTRACK, 0);
  if (!cth)
  {
   // printf("can't open handle\n");
    goto err;
  }

  // tmpl.ct will pass to nf_conntrack_handle_cb as 'void *data'
  nfct_callback_register(cth, NFCT_T_ALL, bw_mon_ct_cb, tmpl.ct);

  filter_dump = nfct_filter_dump_create();
  if (filter_dump == NULL) {
   // printf("OOM\n");
    goto err;
  }

  if (tmpl.filter_mark_kernel_set) {
      nfct_filter_dump_set_attr(filter_dump, NFCT_FILTER_DUMP_MARK, &tmpl.filter_mark_kernel);
      nfct_filter_dump_set_attr_u8(filter_dump, NFCT_FILTER_DUMP_L3NUM, family);
  }

  nfct_query(cth, NFCT_Q_DUMP_FILTER, filter_dump);


  is_OK = 1;
err:
  if(!is_OK)
    free_tmpl_objects();
  
  return is_OK;
}

int deinit_bw_mon_ct()
{
  //printf("deinit_bw_mon_ct\n");
  
 // nf_list_free(&iplist);
 // nf_list_free(&smlist);
    
  if(cth)
    nfct_close(cth);
  free_tmpl_objects();

  return 0;
}


int bw_mon_ct_invoke()
{
  // every nfct_query's entry will call nf_conntrack_handle_cb
  if(cth) {
    //printf("bw_mon_ct_invoke\n");
    nfct_query(cth, NFCT_Q_DUMP_FILTER, filter_dump);

  }
}
