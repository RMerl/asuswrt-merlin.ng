/*
 * test/test-complex-HTB-with-hash-filters.c     Add HTB qdisc, HTB classes and creates some hash filters
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation version 2.1
 *      of the License.
 *
 * Copyright (c) 2011 Adrian Ban <adrian.ban@mantech.ro>
 */

#include <netlink/route/link.h>
#include <netlink/route/tc.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/qdisc/htb.h>
#include <netlink/route/qdisc/sfq.h>
#include <netlink/route/cls/u32.h>
#include <netlink/route/classifier.h>
#include <netlink/route/class.h>
#include <linux/if_ether.h>

#include <netlink/attr.h>
//#include "include/rtnl_u32.h"

#include <stdio.h>
#include <string.h>
//#include "include/rtnl_u32_addon.h"

#define 	TC_HANDLE(maj, min)   (TC_H_MAJ((maj) << 16) | TC_H_MIN(min))

/* some functions are copied from iproute-tc tool */
int get_u32(__u32 *val, const char *arg, int base)
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

int get_u32_handle(__u32 *handle, const char *str)
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

uint32_t get_u32_parse_handle(const char *cHandle)
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

int get_tc_classid(__u32 *h, const char *str)
{
	__u32 maj, min;
	char *p;

	maj = TC_H_ROOT;
	if (strcmp(str, "root") == 0)
		goto ok;
	maj = TC_H_UNSPEC;
	if (strcmp(str, "none") == 0)
		goto ok;
	maj = strtoul(str, &p, 16);
	if (p == str) {
		maj = 0;
		if (*p != ':')
			return -1;
	}
	if (*p == ':') {
		if (maj >= (1<<16))
			return -1;
		maj <<= 16;
		str = p+1;
		min = strtoul(str, &p, 16);
		if (*p != 0)
			return -1;
		if (min >= (1<<16))
			return -1;
		maj |= min;
	} else if (*p != 0)
		return -1;

ok:
	*h = maj;
	return 0;
}

/* 
 * Function that adds a new filter and attach it to a hash table
 *
 */
int u32_add_filter_on_ht(struct nl_sock *sock, struct rtnl_link *rtnlLink, uint32_t prio, 
		uint32_t keyval, uint32_t keymask, int keyoff, int keyoffmask,
		uint32_t htid, uint32_t classid
)
{
    struct rtnl_cls *cls;
    int err;

    //printf("Key Val  : 0x%x\n", keyval);
    //printf("Key Mask : 0x%x\n", keymask);

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

    rtnl_tc_set_parent(TC_CAST(cls), TC_HANDLE(1, 0));

    rtnl_u32_set_hashtable(cls, htid);

    rtnl_u32_add_key_uint32(cls, keyval, keymask, keyoff, keyoffmask); /* 10.0.0.0/8 */

    rtnl_u32_set_classid(cls, classid);
    
    rtnl_u32_set_cls_terminal(cls);

    if ((err = rtnl_cls_add(sock, cls, NLM_F_CREATE))) {
        printf("Can not add classifier: %s\n", nl_geterror(err));
        return -1;
    }
    rtnl_cls_put(cls);
    return 0;

}

/* 
 * Function that adds a new filter and attach it to a hash table 
 * and set next hash table link with hash mask
 *
 */
int u32_add_filter_on_ht_with_hashmask(struct nl_sock *sock, struct rtnl_link *rtnlLink, uint32_t prio, 
	    uint32_t keyval, uint32_t keymask, int keyoff, int keyoffmask,
	    uint32_t htid, uint32_t htlink, uint32_t hmask, uint32_t hoffset
)
{
    struct rtnl_cls *cls;
    int err;

    //printf("Key Val  : 0x%x\n", keyval);
    //printf("Key Mask : 0x%x\n", keymask);

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

    rtnl_tc_set_parent(TC_CAST(cls), TC_HANDLE(1, 0));
    
    if (htid)
	rtnl_u32_set_hashtable(cls, htid);

    rtnl_u32_add_key_uint32(cls, keyval, keymask, keyoff, keyoffmask);

    rtnl_u32_set_hashmask(cls, hmask, hoffset);

    rtnl_u32_set_link(cls, htlink);


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
    rtnl_tc_set_parent(TC_CAST(cls), TC_HANDLE(1, 0));

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
 * function that adds a new HTB qdisc and set the default class for unclassified traffic
 */
int qdisc_add_HTB(struct nl_sock *sock, struct rtnl_link *rtnlLink, uint32_t defaultClass)
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

    //delete the qdisc
    //printf("Delete current qdisc\n");
    rtnl_qdisc_delete(sock, qdisc);
    //rtnl_qdisc_put(qdisc);

    //add a HTB qdisc
    //printf("Add a new HTB qdisc\n");
    rtnl_tc_set_handle(TC_CAST(qdisc), TC_HANDLE(1,0));

    if ((err = rtnl_tc_set_kind(TC_CAST(qdisc), "htb"))) {
        printf("Can not allocate HTB\n");
	return -1;
    }

    /* Set default class for unclassified traffic */
    //printf("Set default class for unclassified traffic\n");
    rtnl_htb_set_defcls(qdisc, TC_HANDLE(1, defaultClass));
    rtnl_htb_set_rate2quantum(qdisc, 1);

    /* Submit request to kernel and wait for response */
    if ((err = rtnl_qdisc_add(sock, qdisc, NLM_F_CREATE))) {
        printf("Can not allocate HTB Qdisc\n");
	return -1;
    }

    /* Return the qdisc object to free memory resources */
    rtnl_qdisc_put(qdisc);

    return 0;
}

/*
 * function that adds a new HTB class and set its parameters
 */
int class_add_HTB(struct nl_sock *sock, struct rtnl_link *rtnlLink, 
		    uint32_t parentMaj, uint32_t parentMin,
		    uint32_t childMaj,  uint32_t childMin, 
		    uint64_t rate, uint64_t ceil,
		    uint32_t burst, uint32_t cburst, 
		    uint32_t prio
)
{
    int err;
    struct rtnl_class *class;
    //struct rtnl_class *class = (struct rtnl_class *) tc;

    //create a HTB class 
    //class = (struct rtnl_class *)rtnl_class_alloc();
    if (!(class = rtnl_class_alloc())) {
        printf("Can not allocate class object\n");
        return 1;
    }
    //
    rtnl_tc_set_link(TC_CAST(class), rtnlLink);
    //add a HTB qdisc
    //printf("Add a new HTB class with 0x%X:0x%X on parent 0x%X:0x%X\n", childMaj, childMin, parentMaj, parentMin);
    rtnl_tc_set_parent(TC_CAST(class), TC_HANDLE(parentMaj, parentMin));
    rtnl_tc_set_handle(TC_CAST(class), TC_HANDLE(childMaj, childMin));

    if ((err = rtnl_tc_set_kind(TC_CAST(class), "htb"))) {
        printf("Can not set HTB to class\n");
        return 1;
    }

    //printf("set HTB class prio to %u\n", prio);
    rtnl_htb_set_prio((struct rtnl_class *)class, prio);

    if (rate) {
	//rate=rate/8;
	rtnl_htb_set_rate(class, rate);
    }
    if (ceil) {
	//ceil=ceil/8;
	rtnl_htb_set_ceil(class, ceil);
    }
    
    if (burst) {
	//printf ("Class HTB: set rate burst: %u\n", burst);
        rtnl_htb_set_rbuffer(class, burst);
    }
    if (cburst) {
	//printf ("Class HTB: set rate cburst: %u\n", cburst);
        rtnl_htb_set_cbuffer(class, cburst);
    }
    /* Submit request to kernel and wait for response */
    if ((err = rtnl_class_add(sock, class, NLM_F_CREATE))) {
        printf("Can not allocate HTB Qdisc\n");
        return 1;
    }
    rtnl_class_put(class);
    return 0;
}

/*
 * function that adds a HTB root class and set its parameters
 */
int class_add_HTB_root(struct nl_sock *sock, struct rtnl_link *rtnlLink, 
			uint64_t rate, uint64_t ceil,
			uint32_t burst, uint32_t cburst
)
{
    int err;
    struct rtnl_class *class;

    //create a HTB class 
    class = (struct rtnl_class *)rtnl_class_alloc();
    //class = rtnl_class_alloc();
    if (!class) {
        printf("Can not allocate class object\n");
        return 1;
    }
    //
    rtnl_tc_set_link(TC_CAST(class), rtnlLink);
    rtnl_tc_set_parent(TC_CAST(class), TC_H_ROOT);
    //add a HTB class
    //printf("Add a new HTB ROOT class\n");
    rtnl_tc_set_handle(TC_CAST(class), 1);

    if ((err = rtnl_tc_set_kind(TC_CAST(class), "htb"))) {
        printf("Can not set HTB to class\n");
        return 1;
    }

    if (rate) {
	//rate=rate/8;
	rtnl_htb_set_rate(class, rate);
    }
    if (ceil) {
	//ceil=ceil/8;
	rtnl_htb_set_ceil(class, ceil);
    }
    
    if (burst) {
        rtnl_htb_set_rbuffer(class, burst);
    }
    if (cburst) {
        rtnl_htb_set_cbuffer(class, cburst);
    }
    
    /* Submit request to kernel and wait for response */
    if ((err = rtnl_class_add(sock, class, NLM_F_CREATE))) {
        printf("Can not allocate HTB Qdisc\n");
        return 1;
    }
    rtnl_class_put(class);
    return 0;
}

/*
 * function that adds a new SFQ qdisc as a leaf for a HTB class
 */
int qdisc_add_SFQ_leaf(struct nl_sock *sock, struct rtnl_link *rtnlLink,
			uint32_t parentMaj, uint32_t parentMin, 
			int quantum, int limit, int perturb
)
{
    int err;
    struct rtnl_qdisc *qdisc;

    if (!(qdisc = rtnl_qdisc_alloc())) {
        printf("Can not allocate qdisc object\n");
        return 1;
    }
    rtnl_tc_set_link(TC_CAST(qdisc), rtnlLink);
    rtnl_tc_set_parent(TC_CAST(qdisc), TC_HANDLE(parentMaj, parentMin));

    rtnl_tc_set_handle(TC_CAST(qdisc), TC_HANDLE(parentMin,0));

    if ((err = rtnl_tc_set_kind(TC_CAST(qdisc), "sfq"))) {
        printf("Can not set SQF class\n");
        return 1;
    }

    if(quantum) {
        rtnl_sfq_set_quantum(qdisc, quantum);
    } else {
        rtnl_sfq_set_quantum(qdisc, 16000); // tc default value
    }
    if(limit) {
        rtnl_sfq_set_limit(qdisc, limit); // default is 127
    }
    if(perturb) {
        rtnl_sfq_set_perturb(qdisc, perturb); // default never perturb the hash
    }

    /* Submit request to kernel and wait for response */
    if ((err = rtnl_qdisc_add(sock, qdisc, NLM_F_CREATE))) {
        printf("Can not allocate SFQ qdisc\n");
	return -1;
    }

    /* Return the qdisc object to free memory resources */
    rtnl_qdisc_put(qdisc);
    return 0;
}




int main() {
    
    struct nl_sock *sock;
    struct rtnl_link *link;

    //struct rtnl_qdisc *qdisc;
    //struct rtnl_class *class;
    //struct rtnl_cls   *cls;

    uint32_t ht, htlink, htid, direction, classid;
    //uint32_t hash, hashmask, nodeid, divisor, handle;
    //struct rtnl_u32 *f_u32;
    char chashlink[16]="";

    //uint64_t drops, qlen;

    //int master_index;
    int err;
    
    //uint64_t rate=0, ceil=0;

    struct nl_cache *link_cache;
    
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
    if (!(link = rtnl_link_get_by_name(link_cache, "imq0"))) {
        /* error */
        printf("Interface not found\n");
        nl_socket_free(sock);
        exit(1);
    }
    
    err=qdisc_add_HTB(sock, link, 0xffff);
    //drops = rtnl_tc_get_stat(TC_CAST(qdisc), RTNL_TC_DROPS);
    
    //printf("Add ROOT HTB class\n");
    err=class_add_HTB_root(sock, link, 12500000, 12500000, 25000, 25000);
    err=class_add_HTB(sock, link, 1, 0, 1, 0xffff, 1250000, 12500000, 25000, 25000, 5);
    err=qdisc_add_SFQ_leaf(sock, link, 1, 0xffff, 16000, 0, 10);
    err=class_add_HTB(sock, link, 1, 1, 1, 0x5, 2000000, 2000000, 25000, 25000, 5);
    err=qdisc_add_SFQ_leaf(sock, link, 1, 0x5, 16000, 0, 10);
    err=class_add_HTB(sock, link, 1, 1, 1, 0x6, 1000000, 1000000, 25000, 25000, 5);
    err=qdisc_add_SFQ_leaf(sock, link, 1, 0x6, 16000, 0, 10);
    //err=class_add_HTB(sock, link, 1, 0, 1, 0x7, 1024000, 100000000, 5);
    //err=class_add_HTB(sock, link, 1, 0, 1, 0x8, 2048000, 100000000, 5);
    //err=class_add_HTB(sock, link, 1, 0, 1, 0x9, 4096000, 100000000, 5);
    //err=class_add_HTB(sock, link, 1, 0, 1, 0xa, 8192000, 100000000, 5);

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
	    0, htlink, 0xff000000, direction);

    /*
     * For each first byte that we need to match we will create a new hash table
     * For example: you have those clases: 10.0.0.0/24 and 172.16.0.0/23
     * For byte 10 and byte 172 will create a separate hash table that will match the second
     * byte from each class.
     *
     */

    
    // Create a new hash table with prio 1, id 2 and 256 entries
//    u32_CreateNewHashTable(sock, link, 1, 2, 256);
    // Create a new hash table with prio 1, id 3 and 256 entries
//    u32_CreateNewHashTable(sock, link, 1, 3, 256);
//    u32_CreateNewHashTable(sock, link, 1, 4, 256);
//    u32_CreateNewHashTable(sock, link, 1, 5, 256);

    /*
     * Now we will create other filter under (ATENTION) our first hash table (link) 1:
     * Previous rule redirects the trafic according the hash mask to hash table (link) no 1:
     * Here we will match the hash tables from 1:0 to 1:ff. Under each hash table we will attach 
     * other rules that matches next byte from IP source/destination IP and we will repeat the 
     * previous steps.
     *
     */
    

    // /8 check

    // 10.0.0.0/8
    ht=get_u32_parse_handle("1:a:");
    htid = (ht&0xFFFFF000);
    htlink=get_u32_parse_handle("2:");

    u32_add_filter_on_ht_with_hashmask(sock, link, 1, 
	    0x0a000000, 0xff000000, direction, 0,
	    htid, htlink, 0x00ff0000, direction);

    // 172.0.0.0/8
    ht=get_u32_parse_handle("1:ac:");
    htid = (ht&0xFFFFF000);
    htlink=get_u32_parse_handle("3:");

    u32_add_filter_on_ht_with_hashmask(sock, link, 1, 
	    0xac000000, 0xff000000, direction, 0,
	    htid, htlink, 0x00ff0000, direction);


    // /16 check
    // 10.0.0.0/16
    ht=get_u32_parse_handle("2:0:");
    htid = (ht&0xFFFFF000);
    htlink=get_u32_parse_handle("4:");

    u32_add_filter_on_ht_with_hashmask(sock, link, 1, 
	    0x0a000000, 0xffff0000, direction, 0,
	    htid, htlink, 0x0000ff00, direction);

    // 172.17.0.0/16
    ht=get_u32_parse_handle("3:11:");
    htid = (ht&0xFFFFF000);
    htlink=get_u32_parse_handle("5:");

    u32_add_filter_on_ht_with_hashmask(sock, link, 1, 
	    0xac110000, 0xffff0000, direction, 0,
	    htid, htlink, 0x0000ff00, direction);

    // /24 check
    // 10.0.9.0/24
    ht=get_u32_parse_handle("4:9:");
    htid = (ht&0xFFFFF000);
    htlink=get_u32_parse_handle("6:");

    u32_add_filter_on_ht_with_hashmask(sock, link, 1, 
	    0x0a000900, 0xffffff00, direction, 0,
	    htid, htlink, 0x000000ff, direction);
    
    // 172.17.2.0/16
    ht=get_u32_parse_handle("5:2:");
    htid = (ht&0xFFFFF000);
    htlink=get_u32_parse_handle("7:");

    u32_add_filter_on_ht_with_hashmask(sock, link, 1, 
	    0xac110200, 0xffffff00, direction, 0,
	    htid, htlink, 0x000000ff, direction);


    // final filters
    // 10.0.9.20
    ht=get_u32_parse_handle("6:14:");
    htid = (ht&0xFFFFF000);

    err = get_tc_classid(&classid, "1:5");

    u32_add_filter_on_ht(sock, link, 1, 
	    0x0a000914, 0xffffffff, direction, 0,
	    htid, classid);

    // 172.17.2.120
    ht=get_u32_parse_handle("7:78:");
    htid = (ht&0xFFFFF000);

    err = get_tc_classid(&classid, "1:6");

    u32_add_filter_on_ht(sock, link, 1, 
	    0xac110278, 0xffffffff, direction, 0,
	    htid, classid);

    

    nl_socket_free(sock);
    return 0;
}
