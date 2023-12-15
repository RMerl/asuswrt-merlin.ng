/* $Id$ */

/*
 *   Copyright (c) 2001-2010 Aaron Turner <aturner at synfin dot net>
 *   Copyright (c) 2013-2022 Fred Klassen <tcpreplay at appneta dot com> - AppNeta
 *
 *   The Tcpreplay Suite of tools is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, either version 3 of the
 *   License, or with the authors permission any later version.
 *
 *   The Tcpreplay Suite is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with the Tcpreplay Suite.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tree.h"
#include "config.h"
#include "common.h"
#include "tcpprep_api.h"
#include "tcpprep_opts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern tcpr_data_tree_t treeroot;
extern tcpprep_t *tcpprep;

/* static buffer used by tree_print*() functions */
char tree_print_buff[TREEPRINTBUFFLEN];

static tcpr_tree_t *new_tree();
static tcpr_tree_t *packet2tree(const u_char *, int, int);
#ifdef DEBUG /* prevent compile warnings */
static char *tree_print(tcpr_data_tree_t *);
static char *tree_printnode(const char *, const tcpr_tree_t *);
#endif /* DEBUG */
static void tree_buildcidr(tcpr_data_tree_t *, tcpr_buildcidr_t *);
static int tree_checkincidr(tcpr_data_tree_t *, tcpr_buildcidr_t *);

static int ipv6_cmp(const struct tcpr_in6_addr *a, const struct tcpr_in6_addr *b);

RB_PROTOTYPE(tcpr_data_tree_s, tcpr_tree_s, node, tree_comp)
RB_GENERATE(tcpr_data_tree_s, tcpr_tree_s, node, tree_comp)

/**
 * used with rbwalk to walk a tree and generate cidr_t * cidrdata.
 * is smart enough to prevent dupes.  void * arg is cast to bulidcidr_t
 */
void
tree_buildcidr(tcpr_data_tree_t *tree_root, tcpr_buildcidr_t *bcdata)
{
    tcpr_tree_t *node = NULL;
    tcpr_cidr_t *newcidr = NULL;
    unsigned long network;
    struct tcpr_in6_addr network6;
    unsigned long mask = ~0; /* turn on all bits */
    tcpprep_opt_t *options = tcpprep->options;
    uint32_t i, j, k;

    dbg(1, "Running: tree_buildcidr()");

    RB_FOREACH(node, tcpr_data_tree_s, tree_root)
    {
        /* we only check types that are valid */
        if (bcdata->type != DIR_ANY)        /* don't check if we're adding ANY */
            if (bcdata->type != node->type) /* no match, exit early */
                return;
        /*
         * in cases of leaves and last visit add to cidrdata if
         * necessary.  First check IPv4
         */
        dbgx(4, "Checking if %s exists in cidrdata...", get_addr2name4(node->u.ip, RESOLVE));
        if (node->family == AF_INET) {
            if (!check_ip_cidr(options->cidrdata, node->u.ip)) { /* if we exist, abort */
                dbgx(3, "Node %s doesn't exist... creating.", get_addr2name4(node->u.ip, RESOLVE));
                newcidr = new_cidr();
                newcidr->masklen = bcdata->masklen;
                network = node->u.ip & (mask << (32 - bcdata->masklen));
                dbgx(3, "Using network: %s", get_addr2name4(network, RESOLVE));
                newcidr->u.network = network;
                add_cidr(&options->cidrdata, &newcidr);
            }
        }
        /* Check IPv6 Address */
        else if (node->family == AF_INET6) {
            if (!check_ip6_cidr(options->cidrdata, &node->u.ip6)) { /* if we exist, abort */
                dbgx(3, "Node %s doesn't exist... creating.", get_addr2name6(&node->u.ip6, RESOLVE));

                newcidr = new_cidr();
                newcidr->masklen = bcdata->masklen;

                /* init each 4 quads to zero */
                for (i = 0; i < 4; i++)
                    network6.tcpr_s6_addr32[i] = 0;

                /* Build our mask */
                j = bcdata->masklen / 8;

                for (i = 0; i < j; i++)
                    network6.tcpr_s6_addr[i] = node->u.ip6.tcpr_s6_addr[i];

                if ((k = bcdata->masklen % 8) != 0) {
                    k = (uint32_t)~0 << (8 - k);
                    network6.tcpr_s6_addr[j] = node->u.ip6.tcpr_s6_addr[i] & k;
                }

                dbgx(3, "Using network: %s", get_addr2name6(&network6, RESOLVE));
                newcidr->u.network6 = network6;
                add_cidr(&options->cidrdata, &newcidr);
            }
        }
    }
}

/**
 * uses rbwalk to check to see if a given ip address of a given type in the
 * tree is inside any of the cidrdata
 */
static int
tree_checkincidr(tcpr_data_tree_t *tree_root, tcpr_buildcidr_t *bcdata)
{
    tcpr_tree_t *node = NULL;
    tcpprep_opt_t *options = tcpprep->options;

    RB_FOREACH(node, tcpr_data_tree_s, tree_root)
    {
        /* we only check types that are valid */
        if (bcdata->type != DIR_ANY)        /* don't check if we're adding ANY */
            if (bcdata->type != node->type) /* no match, exit early */
                return 0;

        /*
         * in cases of leaves and last visit add to cidrdata if
         * necessary
         */
        if (node->family == AF_INET && check_ip_cidr(options->cidrdata, node->u.ip)) /* if we exist, abort */
            return 1;
        if (node->family == AF_INET6 && check_ip6_cidr(options->cidrdata, &node->u.ip6))
            return 1;
    }
    return 0;
}

/**
 * processes the tree using rbwalk / tree2cidr to generate a CIDR
 * used for 2nd pass, router mode
 *
 * returns > 0 for success (the mask len), 0 for fail
 */
int
process_tree(void)
{
    int mymask;
    tcpr_buildcidr_t *bcdata;
    tcpprep_opt_t *options = tcpprep->options;

    dbg(1, "Running: process_tree()");

    bcdata = (tcpr_buildcidr_t *)safe_malloc(sizeof(tcpr_buildcidr_t));

    for (mymask = options->max_mask; mymask <= options->min_mask; mymask++) {
        dbgx(1, "Current mask: %u", mymask);

        /* set starting vals */
        bcdata->type = DIR_SERVER;
        bcdata->masklen = mymask;

        /* build cidrdata with servers */
        tree_buildcidr(&treeroot, bcdata);

        /* calculate types of all IP's */
        tree_calculate(&treeroot);

        /* try to find clients in cidrdata */
        bcdata->type = DIR_CLIENT;

        if (!tree_checkincidr(&treeroot, bcdata)) { /* didn't find any clients in cidrdata */
            safe_free(bcdata);
            return (mymask); /* success! */
        } else {
            destroy_cidr(options->cidrdata); /* clean up after our mess */
            options->cidrdata = NULL;
        }
    }

    safe_free(bcdata);
    /* we failed to find a valid cidr list */
    notice("Unable to determine any IP addresses as a clients.");
    notice("Perhaps you should change the --ratio, --minmask/maxmask settings, or try another mode?");
    return (0);
}

/*
 * processes rbdata to build cidrdata based upon the
 * given type (SERVER, CLIENT, UNKNOWN) using the given masklen
 *
 * is smart enough to prevent dupes

void
tcpr_tree_to_cidr(int masklen, int type)
{

}
 */

/**
 * Checks to see if an IP is client or server by finding it in the tree
 * returns TCPR_DIR_C2S or TCPR_DIR_S2C or -1 on error
 * if mode = UNKNOWN, then abort on unknowns
 * if mode = CLIENT, then unknowns become clients
 * if mode = SERVER, then unknowns become servers
 */
tcpr_dir_t
check_ip_tree(int mode, unsigned long ip)
{
    tcpr_tree_t *node, *finder;

    finder = new_tree();
    finder->family = AF_INET;
    finder->u.ip = ip;

    node = RB_FIND(tcpr_data_tree_s, &treeroot, finder);

    if (node == NULL && mode == DIR_UNKNOWN) {
        safe_free(finder);
        errx(-1,
             "%s (%lu) is an unknown system... aborting.!\n"
             "Try a different auto mode (-n router|client|server)",
             get_addr2name4(ip, RESOLVE),
             ip);
    }

    /* return node type if we found the node, else return the default (mode) */
    if (node != NULL) {
        switch (node->type) {
        case DIR_SERVER:
            dbgx(1, "DIR_SERVER: %s", get_addr2name4(ip, RESOLVE));
            safe_free(finder);
            return TCPR_DIR_S2C;
        case DIR_CLIENT:
            dbgx(1, "DIR_CLIENT: %s", get_addr2name4(ip, RESOLVE));
            safe_free(finder);
            return TCPR_DIR_C2S;
        case DIR_UNKNOWN:
            dbgx(1, "DIR_UNKNOWN: %s", get_addr2name4(ip, RESOLVE));
            /* use our current mode to determine return code */
            goto return_unknown;
        case DIR_ANY:
            dbgx(1, "DIR_ANY: %s", get_addr2name4(ip, RESOLVE));
            goto return_unknown;
        default:
            errx(-1, "Node for %s has invalid type: %d", get_addr2name4(ip, RESOLVE), node->type);
        }
    }

return_unknown:
    safe_free(finder);
    switch (mode) {
    case DIR_SERVER:
        return TCPR_DIR_S2C;
    case DIR_CLIENT:
        return TCPR_DIR_C2S;
    default:
        return -1;
    }
}

tcpr_dir_t
check_ip6_tree(int mode, const struct tcpr_in6_addr *addr)
{
    tcpr_tree_t *node, *finder;

    finder = new_tree();
    finder->family = AF_INET6;
    finder->u.ip6 = *addr;

    node = RB_FIND(tcpr_data_tree_s, &treeroot, finder);

    if (node == NULL && mode == DIR_UNKNOWN) {
        safe_free(finder);
        errx(-1,
             "%s is an unknown system... aborting.!\n"
             "Try a different auto mode (-n router|client|server)",
             get_addr2name6(addr, RESOLVE));
    }

    /* return node type if we found the node, else return the default (mode) */
    if (node != NULL) {
        switch (node->type) {
        case DIR_SERVER:
            dbgx(1, "DIR_SERVER: %s", get_addr2name6(addr, RESOLVE));
            safe_free(finder);
            return TCPR_DIR_S2C;
        case DIR_CLIENT:
            dbgx(1, "DIR_CLIENT: %s", get_addr2name6(addr, RESOLVE));
            safe_free(finder);
            return TCPR_DIR_C2S;
        case DIR_UNKNOWN:
            dbgx(1, "DIR_UNKNOWN: %s", get_addr2name6(addr, RESOLVE));
            /* use our current mode to determine return code */
            goto return_unknown;
        case DIR_ANY:
            dbgx(1, "DIR_ANY: %s", get_addr2name6(addr, RESOLVE));
            goto return_unknown;
        default:
            errx(-1, "Node for %s has invalid type: %d", get_addr2name6(addr, RESOLVE), node->type);
        }
    }

return_unknown:
    safe_free(finder);
    switch (mode) {
    case DIR_SERVER:
        return TCPR_DIR_S2C;
    case DIR_CLIENT:
        return TCPR_DIR_C2S;
    default:
        return -1;
    }
}

/**
 * Parses the IP header of the given packet (data) to get the SRC/DST IP
 * addresses.  If the SRC IP doesn't exist in the TREE, we add it as a
 * client, if the DST IP doesn't exist in the TREE, we add it as a server
 */
void
add_tree_first_ipv4(const u_char *data, int len, int datalink)
{
    tcpr_tree_t *newnode, *findnode;
    uint32_t _U_ vlan_offset;
    uint32_t pkt_len = len;
    uint16_t ether_type;
    uint32_t l2offset;
    ipv4_hdr_t ip_hdr;
    uint32_t l2len;
    int res;

    assert(data);

    res = get_l2len_protocol(data, pkt_len, datalink, &ether_type, &l2len, &l2offset, &vlan_offset);

    if (res == -1 || len < (int)(l2len + TCPR_IPV4_H)) {
        errx(-1, "Capture length %d too small for IPv4 parsing", len);
    }

    /*
     * first add/find the source IP/client
     */
    newnode = new_tree();

    /* prevent issues with byte alignment, must memcpy */
    memcpy(&ip_hdr, data + l2len, TCPR_IPV4_H);

    /* copy over the source ip, and values to guarantee this a client */
    newnode->family = AF_INET;
    newnode->u.ip = ip_hdr.ip_src.s_addr;
    newnode->type = DIR_CLIENT;
    newnode->client_cnt = 1000;
    findnode = RB_FIND(tcpr_data_tree_s, &treeroot, newnode);

    /* if we didn't find it, add it to the tree, else free it */
    if (findnode == NULL) {
        RB_INSERT(tcpr_data_tree_s, &treeroot, newnode);
    } else {
        safe_free(newnode);
    }

    /*
     * now add/find the destination IP/server
     */
    newnode = new_tree();
    memcpy(&ip_hdr, data + l2len, TCPR_IPV4_H);

    newnode->family = AF_INET;
    newnode->u.ip = ip_hdr.ip_dst.s_addr;
    newnode->type = DIR_SERVER;
    newnode->server_cnt = 1000;
    findnode = RB_FIND(tcpr_data_tree_s, &treeroot, newnode);

    if (findnode == NULL) {
        RB_INSERT(tcpr_data_tree_s, &treeroot, newnode);
    } else {
        safe_free(newnode);
    }
}

void
add_tree_first_ipv6(const u_char *data, int len, int datalink)
{
    tcpr_tree_t *newnode, *findnode;
    uint32_t _U_ vlan_offset;
    uint32_t pkt_len = len;
    uint16_t ether_type;
    ipv6_hdr_t ip6_hdr;
    uint32_t l2offset;
    uint32_t l2len;
    int res;

    assert(data);

    res = get_l2len_protocol(data, pkt_len, datalink, &ether_type, &l2len, &l2offset, &vlan_offset);

    if (res == -1 || len < (int)(l2len + TCPR_IPV6_H))
        errx(-1, "Capture length %d too small for IPv6 parsing", len);

    /*
     * first add/find the source IP/client
     */
    newnode = new_tree();

    /* prevent issues with byte alignment, must memcpy */
    memcpy(&ip6_hdr, data + l2len, TCPR_IPV6_H);

    /* copy over the source ip, and values to guarantee this a client */
    newnode->family = AF_INET6;
    newnode->u.ip6 = ip6_hdr.ip_src;
    newnode->type = DIR_CLIENT;
    newnode->client_cnt = 1000;
    findnode = RB_FIND(tcpr_data_tree_s, &treeroot, newnode);

    /* if we didn't find it, add it to the tree, else free it */
    if (findnode == NULL) {
        RB_INSERT(tcpr_data_tree_s, &treeroot, newnode);
    } else {
        safe_free(newnode);
    }

    /*
     * now add/find the destination IP/server
     */
    newnode = new_tree();
    memcpy(&ip6_hdr, data + l2len, TCPR_IPV6_H);

    newnode->family = AF_INET6;
    newnode->u.ip6 = ip6_hdr.ip_dst;
    newnode->type = DIR_SERVER;
    newnode->server_cnt = 1000;
    findnode = RB_FIND(tcpr_data_tree_s, &treeroot, newnode);

    if (findnode == NULL) {
        RB_INSERT(tcpr_data_tree_s, &treeroot, newnode);
    } else {
        safe_free(newnode);
    }
}

static void
add_tree_node(tcpr_tree_t *newnode)
{
    tcpr_tree_t *node;

    /* try to find a simular entry in the tree */
    node = RB_FIND(tcpr_data_tree_s, &treeroot, newnode);

    dbgx(3, "%s", tree_printnode("add_tree", node));

    /* new entry required */
    if (node == NULL) {
        /* increment counters */
        if (newnode->type == DIR_SERVER) {
            newnode->server_cnt++;
        } else if (newnode->type == DIR_CLIENT) {
            newnode->client_cnt++;
        }
        /* insert it in */
        RB_INSERT(tcpr_data_tree_s, &treeroot, newnode);

    } else {
        /* we found something, so update it */
        dbgx(2, "   node: %p\nnewnode: %p", node, newnode);
        dbgx(3, "%s", tree_printnode("update node", node));
        /* increment counter */
        if (newnode->type == DIR_SERVER) {
            node->server_cnt++;
        } else if (newnode->type == DIR_CLIENT) {
            /* temp debug code */
            node->client_cnt++;
        }

        /* didn't insert it, so free it */
        safe_free(newnode);
    }

    dbg(2, "------- START NEXT -------");
    dbgx(3, "%s", tree_print(&treeroot));
}

/**
 * adds an entry to the tree (phase 1 of auto mode).  We add each host
 * to the tree if it doesn't yet exist.  We go through and track:
 * - number of times each host acts as a client or server
 * - the way the host acted the first time we saw it (client or server)
 */
void
add_tree_ipv4(unsigned long ip, const u_char *data, int len, int datalink)
{
    tcpr_tree_t *newnode;
    assert(data);

    newnode = packet2tree(data, len, datalink);
    assert(newnode);
    assert(ip == newnode->u.ip);
    if (newnode->type == DIR_UNKNOWN) {
        /* couldn't figure out if packet was client or server */

        dbgx(2, "%s (%lu) unknown client/server", get_addr2name4(newnode->u.ip, RESOLVE), newnode->u.ip);
    }

    add_tree_node(newnode);
}

void
add_tree_ipv6(const struct tcpr_in6_addr *addr, const u_char *data, int len, int datalink)
{
    tcpr_tree_t *newnode;
    assert(data);

    newnode = packet2tree(data, len, datalink);
    assert(newnode);
    assert(ipv6_cmp(addr, &newnode->u.ip6) == 0);
    if (newnode->type == DIR_UNKNOWN) {
        /* couldn't figure out if packet was client or server */

        dbgx(2, "%s unknown client/server", get_addr2name6(&newnode->u.ip6, RESOLVE));
    }

    add_tree_node(newnode);
}

/**
 * calculates whether each node in the tree is a client, server, or unknown for each node in the tree
 */
void
tree_calculate(tcpr_data_tree_t *tree_root)
{
    tcpr_tree_t *node;
    tcpprep_opt_t *options = tcpprep->options;

    dbg(1, "Running tree_calculate()");

    RB_FOREACH(node, tcpr_data_tree_s, tree_root)
    {
        dbgx(4, "Processing %s", get_addr2name4(node->u.ip, RESOLVE));
        if ((node->server_cnt > 0) || (node->client_cnt > 0)) {
            /* type based on: server >= (client*ratio) */
            if ((double)node->server_cnt >= (double)node->client_cnt * options->ratio) {
                node->type = DIR_SERVER;
                dbgx(3, "Setting %s to server", get_addr2name4(node->u.ip, RESOLVE));
            } else {
                node->type = DIR_CLIENT;
                dbgx(3, "Setting %s to client", get_addr2name4(node->u.ip, RESOLVE));
            }
        } else { /* IP had no client or server connections */
            node->type = DIR_UNKNOWN;
            dbgx(3, "Setting %s to unknown", get_addr2name4(node->u.ip, RESOLVE));
        }
    }
}

static int
ipv6_cmp(const struct tcpr_in6_addr *a, const struct tcpr_in6_addr *b)
{
    int i;

    for (i = 0; i < 4; i++) {
        int k;
        if ((k = ((int)a->tcpr_s6_addr32[i] - (int)b->tcpr_s6_addr32[i]))) {
            return (k > 0) ? 1 : -1;
        }
    }

    return 0;
}

/**
 * tree_comp(), called by rbsearch compares two treees and returns:
 * 1  = first > second
 * -1 = first < second
 * 0  = first = second
 * based upon the ip address stored
 *
 */
int
tree_comp(tcpr_tree_t *t1, tcpr_tree_t *t2)
{
    if (t1->family > t2->family) {
        dbgx(2, "family %d > %d", t1->family, t2->family);
        return 1;
    }

    if (t1->family < t2->family) {
        dbgx(2, "family %d < %d", t1->family, t2->family);
        return -1;
    }

    if (t1->family == AF_INET) {
        if (t1->u.ip > t2->u.ip) {
            dbgx(2, "%s > %s", get_addr2name4(t1->u.ip, RESOLVE), get_addr2name4(t2->u.ip, RESOLVE));
            return 1;
        }

        if (t1->u.ip < t2->u.ip) {
            dbgx(2, "%s < %s", get_addr2name4(t1->u.ip, RESOLVE), get_addr2name4(t2->u.ip, RESOLVE));
            return -1;
        }

        dbgx(2, "%s = %s", get_addr2name4(t1->u.ip, RESOLVE), get_addr2name4(t2->u.ip, RESOLVE));

        return 0;
    }

    if (t1->family == AF_INET6) {
        int ret = ipv6_cmp(&t1->u.ip6, &t1->u.ip6);
        dbgx(2, "cmp(%s, %s) = %d", get_addr2name6(&t1->u.ip6, RESOLVE), get_addr2name6(&t2->u.ip6, RESOLVE), ret);
        return ret;
    }

    return 0;
}

/**
 * creates a new TREE * with reasonable defaults
 */
static tcpr_tree_t *
new_tree()
{
    tcpr_tree_t *node;

    node = (tcpr_tree_t *)safe_malloc(sizeof(tcpr_tree_t));

    memset(node, '\0', sizeof(tcpr_tree_t));
    node->server_cnt = 0;
    node->client_cnt = 0;
    node->type = DIR_UNKNOWN;
    node->masklen = -1;
    node->u.ip = 0;
    return (node);
}

/**
 * returns a struct of TREE * from a packet header
 * and sets the type to be SERVER or CLIENT or UNKNOWN
 * if it's an undefined packet, we return -1 for the type
 * the u_char * data should be the data that is passed by pcap_dispatch()
 */
static tcpr_tree_t *
packet2tree(const u_char *data, int len, int datalink)
{
    uint32_t _U_ vlan_offset;
    ssize_t pkt_len = len;
    tcpr_tree_t *node = NULL;
    ipv4_hdr_t ip_hdr;
    ipv6_hdr_t ip6_hdr;
    tcp_hdr_t tcp_hdr;
    udp_hdr_t udp_hdr;
    icmpv4_hdr_t icmp_hdr;
    dnsv4_hdr_t dnsv4_hdr;
    u_int16_t ether_type;
    uint32_t l2offset;
    u_char proto = 0;
    uint32_t l2len;
    int hl = 0;
    int res;

#ifdef DEBUG
    char srcip[INET6_ADDRSTRLEN];
#endif

    res = get_l2len_protocol(data, pkt_len, datalink, &ether_type, &l2len, &l2offset, &vlan_offset);

    if (res == -1)
        goto len_error;

    node = new_tree();

    if (ether_type == ETHERTYPE_IP) {
        if (pkt_len < (ssize_t)l2len + TCPR_IPV4_H + hl)
            goto len_error;

        memcpy(&ip_hdr, data + l2len + hl, TCPR_IPV4_H);

        node->family = AF_INET;
        node->u.ip = ip_hdr.ip_src.s_addr;
        proto = ip_hdr.ip_p;
        hl += ip_hdr.ip_hl * 4;

#ifdef DEBUG
        strlcpy(srcip, get_addr2name4(ip_hdr.ip_src.s_addr, RESOLVE), 16);
#endif
    } else if (ether_type == ETHERTYPE_IP6) {
        if (pkt_len < (ssize_t)l2len + TCPR_IPV6_H + hl) {
            goto len_error;
        }

        memcpy(&ip6_hdr, data + l2len + hl, TCPR_IPV6_H);

        node->family = AF_INET6;
        node->u.ip6 = ip6_hdr.ip_src;
        proto = ip6_hdr.ip_nh;
        hl += TCPR_IPV6_H;

#ifdef DEBUG
        strlcpy(srcip, get_addr2name6(&ip6_hdr.ip_src, RESOLVE), INET6_ADDRSTRLEN);
#endif
    } else {
        dbgx(2, "Unrecognized ether_type (%x)", ether_type);
    }

    /*
     * TCP
     */
    if (proto == IPPROTO_TCP) {
#ifdef DEBUG
        dbgx(3, "%s uses TCP...  ", srcip);
#endif

        if (pkt_len < (ssize_t)l2len + TCPR_TCP_H + hl)
            goto len_error;

        /* memcpy it over to prevent alignment issues */
        memcpy(&tcp_hdr, data + l2len + hl, TCPR_TCP_H);

        /* ftp-data is going to skew our results so we ignore it */
        if (tcp_hdr.th_sport == 20)
            return (node);

        /* set TREE->type based on TCP flags */
        if (tcp_hdr.th_flags == TH_SYN) {
            node->type = DIR_CLIENT;
            dbg(3, "is a client");
        } else if (tcp_hdr.th_flags == (TH_SYN | TH_ACK)) {
            node->type = DIR_SERVER;
            dbg(3, "is a server");
        } else {
            dbg(3, "is an unknown");
        }
    }
    /*
     * UDP
     */
    else if (proto == IPPROTO_UDP) {
        if (pkt_len < (ssize_t)l2len + TCPR_UDP_H + hl)
            goto len_error;

        /* memcpy over to prevent alignment issues */
        memcpy(&udp_hdr, data + l2len + hl, TCPR_UDP_H);
#ifdef DEBUG
        dbgx(3, "%s uses UDP...  ", srcip);
#endif

        switch (ntohs(udp_hdr.uh_dport)) {
        case 0x0035: /* dns */
            if (pkt_len < (ssize_t)l2len + TCPR_UDP_H + TCPR_DNS_H + hl)
                goto len_error;

            /* prevent memory alignment issues */
            memcpy(&dnsv4_hdr, data + l2len + hl + TCPR_UDP_H, TCPR_DNS_H);

            if (dnsv4_hdr.flags & DNS_QUERY_FLAG) {
                /* bit set, response */
                node->type = DIR_SERVER;

                dbg(3, "is a dns server");

            } else {
                /* bit not set, query */
                node->type = DIR_CLIENT;

                dbg(3, "is a dns client");
            }
            return (node);
        default:
            break;
        }

        switch (ntohs(udp_hdr.uh_sport)) {
        case 0x0035: /* dns */
            if (pkt_len < (ssize_t)l2len + TCPR_UDP_H + TCPR_DNS_H + hl)
                goto len_error;

            /* prevent memory alignment issues */
            memcpy(&dnsv4_hdr, data + l2len + hl + TCPR_UDP_H, TCPR_DNS_H);

            if ((dnsv4_hdr.flags & 0x7FFFF) ^ DNS_QUERY_FLAG) {
                /* bit set, response */
                node->type = DIR_SERVER;
                dbg(3, "is a dns server");
            } else {
                /* bit not set, query */
                node->type = DIR_CLIENT;
                dbg(3, "is a dns client");
            }
            return (node);
        default:

            dbgx(3, "unknown UDP protocol: %hu->%hu", udp_hdr.uh_sport, udp_hdr.uh_dport);
            break;
        }
    }
    /*
     * ICMP
     */
    else if (proto == IPPROTO_ICMP) {
        if (pkt_len < (ssize_t)l2len + TCPR_ICMPV4_H + hl)
            goto len_error;

        /* prevent alignment issues */
        memcpy(&icmp_hdr, data + l2len + hl, TCPR_ICMPV4_H);

#ifdef DEBUG
        dbgx(3, "%s uses ICMP...  ", srcip);
#endif

        /*
         * if port unreachable, then source == server, dst == client
         */
        if ((icmp_hdr.icmp_type == ICMP_UNREACH) && (icmp_hdr.icmp_code == ICMP_UNREACH_PORT)) {
            node->type = DIR_SERVER;
            dbg(3, "is a server with a closed port");
        }
    }

    return (node);

len_error:
    safe_free(node);
    errx(-1, "packet capture length %d too small to process", len);
}

#ifdef DEBUG
/**
 * prints out a node of the tree to stderr
 */
static char *
tree_printnode(const char *name, const tcpr_tree_t *node)
{
    memset(&tree_print_buff, '\0', TREEPRINTBUFFLEN);
    if (node == NULL) {
        snprintf(tree_print_buff, TREEPRINTBUFFLEN, "%s node is null", name);
    }

    else {
        snprintf(tree_print_buff,
                 TREEPRINTBUFFLEN,
                 "-- %s: %p\nIP: %s\nMask: %d\nSrvr: %d\nClnt: %d\n",
                 name,
                 (void *)node,
                 node->family == AF_INET ? get_addr2name4(node->u.ip, RESOLVE) : get_addr2name6(&node->u.ip6, RESOLVE),
                 node->masklen,
                 node->server_cnt,
                 node->client_cnt);
        if (node->type == DIR_SERVER) {
            strlcat(tree_print_buff, "Type: Server\n--\n", TREEPRINTBUFFLEN);
        } else {
            strlcat(tree_print_buff, "Type: Client\n--", TREEPRINTBUFFLEN);
        }
    }
    return (tree_print_buff);
}

/**
 * prints out the entire tree
 */
static char *
tree_print(tcpr_data_tree_t *tree_root)
{
    tcpr_tree_t *node = NULL;
    memset(&tree_print_buff, '\0', TREEPRINTBUFFLEN);
    RB_FOREACH(node, tcpr_data_tree_s, tree_root)
    {
        tree_printnode("my node", node);
    }
    return (tree_print_buff);
}
#endif /* DEBUG */
