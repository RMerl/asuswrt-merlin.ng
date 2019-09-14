/*
 * test/tests-u32-with-actions.c     Add ingress qdisc, create some hash filters, and add redirect action
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation version 2.1
 *      of the License.
 *
 * Stolen from tests/test-complex-HTB-with-hash-filters.c
 *
 * Copyright (c) 2013 Cong Wang <xiyou.wangcong@gmail.com>
 */

#include <netlink/route/link.h>
#include <netlink/route/tc.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/cls/u32.h>
#include <netlink/route/classifier.h>
#include <netlink/route/action.h>
#include <netlink/route/act/mirred.h>
#include <netlink/route/class.h>
#include <linux/if_ether.h>

#include <netlink/attr.h>
#include <stdio.h>
#include <string.h>

#define 	TC_HANDLE(maj, min)   (TC_H_MAJ((maj) << 16) | TC_H_MIN(min))

/* some functions are copied from iproute-tc tool */
static int get_u32(__u32 *val, const char *arg, int base)
{
	unsigned long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;
	res = strtoul(arg, &ptr, base);
	if (!ptr || ptr == arg || *ptr || res > 0xFFFFFFFFUL)
		return -1;
	*val = res;
	return 0;
}

static int get_u32_handle(__u32 *handle, const char *str)
{
	__u32 htid=0, hash=0, nodeid=0;
	char *tmp = strchr(str, ':');
        
	if (tmp == NULL) {
		if (memcmp("0x", str, 2) == 0)
			return get_u32(handle, str, 16);
		return -1;
	}
	htid = strtoul(str, &tmp, 16);
	if (tmp == str && *str != ':' && *str != 0)
		return -1;
	if (htid>=0x1000)
		return -1;
	if (*tmp) {
		str = tmp+1;
		hash = strtoul(str, &tmp, 16);
		if (tmp == str && *str != ':' && *str != 0)
			return -1;
		if (hash>=0x100)
			return -1;
		if (*tmp) {
			str = tmp+1;
			nodeid = strtoul(str, &tmp, 16);
			if (tmp == str && *str != 0)
				return -1;
			if (nodeid>=0x1000)
				return -1;
		}
	}
	*handle = (htid<<20)|(hash<<12)|nodeid;
	return 0;
}

static uint32_t get_u32_parse_handle(const char *cHandle)
{
	uint32_t handle=0;

	if(get_u32_handle(&handle, cHandle)) {
		printf ("Illegal \"ht\"\n");
		return -1;
	}

	if (handle && TC_U32_NODE(handle)) {
		printf("\"link\" must be a hash table.\n");
		return -1;
	}
	return handle;
}

/* 
 * Function that adds a new filter and attach it to a hash table 
 * and set next hash table link with hash mask
 *
 */
static
int u32_add_filter_on_ht_with_hashmask(struct nl_sock *sock, struct rtnl_link *rtnlLink, uint32_t prio, 
	    uint32_t keyval, uint32_t keymask, int keyoff, int keyoffmask,
	    uint32_t htid, uint32_t htlink, uint32_t hmask, uint32_t hoffset, struct rtnl_act *act)
{
    struct rtnl_cls *cls;
    int err;

    cls=rtnl_cls_alloc();
    if (!(cls)) {
        printf("Can not allocate classifier\n");
        nl_socket_free(sock);
        exit(1);
    }
    
    rtnl_tc_set_link(TC_CAST(cls), rtnlLink);

    if ((err = rtnl_tc_set_kind(TC_CAST(cls), "u32"))) {
        printf("Can not set classifier as u32\n");
        return 1;
    }

    rtnl_cls_set_prio(cls, prio);
    rtnl_cls_set_protocol(cls, ETH_P_IP);

    rtnl_tc_set_parent(TC_CAST(cls), TC_HANDLE(0xffff, 0));
    
    if (htid)
	rtnl_u32_set_hashtable(cls, htid);

    rtnl_u32_add_key_uint32(cls, keyval, keymask, keyoff, keyoffmask);

    rtnl_u32_set_hashmask(cls, hmask, hoffset);

    rtnl_u32_set_link(cls, htlink);

    rtnl_u32_add_action(cls, act);


    if ((err = rtnl_cls_add(sock, cls, NLM_F_CREATE))) {
        printf("Can not add classifier: %s\n", nl_geterror(err));
        return -1;
    }
    rtnl_cls_put(cls);
    return 0;
}

/* 
 * function that creates a new hash table 
 */
static
int u32_add_ht(struct nl_sock *sock, struct rtnl_link *rtnlLink, uint32_t prio, uint32_t htid, uint32_t divisor)
{

    int err;
    struct rtnl_cls *cls;

    cls=rtnl_cls_alloc();
    if (!(cls)) {
        printf("Can not allocate classifier\n");
        nl_socket_free(sock);
        exit(1);
    }
    
    rtnl_tc_set_link(TC_CAST(cls), rtnlLink);

    if ((err = rtnl_tc_set_kind(TC_CAST(cls), "u32"))) {
        printf("Can not set classifier as u32\n");
        return 1;
    }

    rtnl_cls_set_prio(cls, prio);
    rtnl_cls_set_protocol(cls, ETH_P_IP);
    rtnl_tc_set_parent(TC_CAST(cls), TC_HANDLE(0xffff, 0));

    rtnl_u32_set_handle(cls, htid, 0x0, 0x0);
    //printf("htid: 0x%X\n", htid);
    rtnl_u32_set_divisor(cls, divisor);

    if ((err = rtnl_cls_add(sock, cls, NLM_F_CREATE))) {
        printf("Can not add classifier: %s\n", nl_geterror(err));
        return -1;
    }
    rtnl_cls_put(cls);
    return 0;
}

/*
 * function that adds a new ingress qdisc and set the default class for unclassified traffic
 */
static
int qdisc_add_ingress(struct nl_sock *sock, struct rtnl_link *rtnlLink)
{
    
    struct rtnl_qdisc *qdisc;
    int err;
    
    /* Allocation of a qdisc object */
    if (!(qdisc = rtnl_qdisc_alloc())) {
        printf("Can not allocate Qdisc\n");
	return -1;
    }

    //rtnl_tc_set_ifindex(TC_CAST(qdisc), master_index);
    rtnl_tc_set_link(TC_CAST(qdisc), rtnlLink);
    rtnl_tc_set_parent(TC_CAST(qdisc), TC_H_ROOT);

    //printf("Delete current qdisc\n");
    rtnl_qdisc_delete(sock, qdisc);
    //rtnl_qdisc_put(qdisc);

    rtnl_tc_set_handle(TC_CAST(qdisc), TC_HANDLE(0xffff, 0));

    if ((err = rtnl_tc_set_kind(TC_CAST(qdisc), "ingress"))) {
        printf("Can not allocate ingress\n");
	return -1;
    }

    /* Submit request to kernel and wait for response */
    if ((err = rtnl_qdisc_add(sock, qdisc, NLM_F_CREATE))) {
        printf("Can not allocate ingress Qdisc\n");
	return -1;
    }

    /* Return the qdisc object to free memory resources */
    rtnl_qdisc_put(qdisc);

    return 0;
}

int main(void)
{
    struct nl_sock *sock;
    struct rtnl_link *link;
    uint32_t ht, htlink, htid, direction;
    char chashlink[16]="";
    int err;
    struct nl_cache *link_cache;
    struct rtnl_act *act;

    if (!(sock = nl_socket_alloc())) {
        printf("Unable to allocate netlink socket\n");
        exit(1);
    }

    if ((err = nl_connect(sock, NETLINK_ROUTE)) < 0 ) {
        printf("Nu s-a putut conecta la NETLINK!\n");
        nl_socket_free(sock);
        exit(1);
    }

    if ((err = rtnl_link_alloc_cache(sock, AF_UNSPEC, &link_cache)) < 0) {
        printf("Unable to allocate link cache: %s\n",
                             nl_geterror(err));
        nl_socket_free(sock);
        exit(1);
    }
    
    /* lookup interface index of eth0 */
    if (!(link = rtnl_link_get_by_name(link_cache, "eth0"))) {
        /* error */
        printf("Interface not found\n");
        nl_socket_free(sock);
        exit(1);
    }
    
    err=qdisc_add_ingress(sock, link);
    //printf("Add main hash table\n");

    /* create u32 first hash filter table
     *
     */
    /* formula calcul handle:
    *         uint32_t handle = (htid << 20) | (hash << 12) | nodeid;
    */

    /*
     * Upper limit of number of hash tables: 4096 (0xFFF)
     * Number of hashes in a table: 256 values (0xFF)
     *
     */

    /* using 256 values for hash table 
     * each entry in hash table match a byte from IP address specified later by a hash key
     */

    uint32_t i;
    for (i = 1; i <= 0xf; i++) 
	u32_add_ht(sock, link, 1, i, 256);

    /* 
     * attach a u32 filter to the first hash 
     * that redirects all traffic and make a hash key
     * from the fist byte of the IP address
     *
     */

    //divisor=0x0;	// unused here
    //handle = 0x0;	// unused here
    //hash = 0x0;		// unused here
    //htid = 0x0;		// unused here
    //nodeid = 0x0;	// unused here

    // direction = 12 -> source IP
    // direction = 16 -> destination IP
    direction = 16;

    /*
     * which hash table will use
     * in our case is hash table no 1 defined previous
     *
     * There are 2 posibilities to set the the hash table:
     * 1. Using function get_u32_handle and sent a string in
     *  format 10: where 10 is number of the hash table
     * 2. Create your own value in format: 0xa00000
     *
     */
    strcpy(chashlink, "1:");
    //printf("Hash Link: %s\n", chashlink);
    //chashlink=malloc(sizeof(char) *
    htlink = 0x0;		// is used by get_u32_handle to return the correct value of hash table (link)
    
    if(get_u32_handle(&htlink, chashlink)) {
        printf ("Illegal \"link\"");
        nl_socket_free(sock);
        exit(1);
    }
    //printf ("hash link : 0x%X\n", htlink);
    //printf ("hash link test : %u\n", (htlink && TC_U32_NODE(htlink)));

    if (htlink && TC_U32_NODE(htlink)) {
	printf("\"link\" must be a hash table.\n");
        nl_socket_free(sock);
        exit(1);
    }

    /* the hash mask will hit the hash table (link) no 1: in our case
     */

    /* set the hash key mask */
    //hashmask = 0xFF000000UL;	// the mask that is used to match the hash in specific table, in our case for example 1:a with mean the first byte which is 10 in hash table 1

    /* Here we add a hash filter which match the first byte (see the hashmask value)
     * of the source IP (offset 12 in the packet header)
     * You can use also offset 16 to match the destination IP
     */

    /*
     * Also we need a filter to match our rule
     * This mean that we will put a 0.0.0.0/0 filter in our first rule
     * that match the offset 12 (source IP)
     * Also you can put offset 16 to match the destination IP
     */

    u32_add_filter_on_ht_with_hashmask(sock, link, 1, 
	    0x0, 0x0, direction, 0,
	    0, htlink, 0xff000000, direction, NULL);

    /*
     * For each first byte that we need to match we will create a new hash table
     * For example: you have those clases: 10.0.0.0/24 and 172.16.0.0/23
     * For byte 10 and byte 172 will create a separate hash table that will match the second
     * byte from each class.
     *
     */

    
    /*
     * Now we will create other filter under (ATENTION) our first hash table (link) 1:
     * Previous rule redirects the trafic according the hash mask to hash table (link) no 1:
     * Here we will match the hash tables from 1:0 to 1:ff. Under each hash table we will attach 
     * other rules that matches next byte from IP source/destination IP and we will repeat the 
     * previous steps.
     *
     */
    
    act = rtnl_act_alloc();
    if (!act) {
            printf("rtnl_act_alloc() returns %p\n", act);
            return -1;
   }
    rtnl_tc_set_kind(TC_CAST(act), "mirred");
    rtnl_mirred_set_action(act, TCA_EGRESS_REDIR);
    rtnl_mirred_set_policy(act, TC_ACT_STOLEN);
    rtnl_mirred_set_ifindex(act, rtnl_link_name2i(link_cache, "eth1"));
    // /8 check

    // 10.0.0.0/8
    ht=get_u32_parse_handle("1:a:");
    htid = (ht&0xFFFFF000);
    htlink=get_u32_parse_handle("2:");

    u32_add_filter_on_ht_with_hashmask(sock, link, 1, 
	    0x0a000000, 0xff000000, direction, 0,
	    htid, htlink, 0x00ff0000, direction, act);

    rtnl_act_put(act);
    nl_socket_free(sock);
    return 0;
}
