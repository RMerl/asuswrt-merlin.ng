/*
 *  Ipaddress MIB architecture support
 *
 * $Id$
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/ipaddress.h>
#include <net-snmp/data_access/interface.h>

#include "ip-mib/ipAddressTable/ipAddressTable_constants.h"
#include "ipaddress.h"
#include "ipaddress_private.h"

#include <net-snmp/net-snmp-features.h>

netsnmp_feature_child_of(ipaddress_common, libnetsnmpmibs)

netsnmp_feature_child_of(ipaddress_common_copy_utilities, ipaddress_common)
netsnmp_feature_child_of(ipaddress_entry_copy, ipaddress_common)
netsnmp_feature_child_of(ipaddress_entry_update, ipaddress_common)
netsnmp_feature_child_of(ipaddress_prefix_copy, ipaddress_common_copy_utilities)

#ifdef NETSNMP_FEATURE_REQUIRE_IPADDRESS_ENTRY_COPY
netsnmp_feature_require(ipaddress_arch_entry_copy)
#endif /* NETSNMP_FEATURE_REQUIRE_IPADDRESS_ENTRY_COPY */

/**---------------------------------------------------------------------*/
/*
 * local static prototypes
 */
static int _access_ipaddress_entry_compare_addr(const void *lhs,
                                                const void *rhs);
static void _access_ipaddress_entry_release(netsnmp_ipaddress_entry * entry,
                                            void *unused);

/**---------------------------------------------------------------------*/
/*
 * container functions
 */
/**
 */
netsnmp_container *
netsnmp_access_ipaddress_container_init(u_int flags)
{
    netsnmp_container *container1;

    DEBUGMSGTL(("access:ipaddress:container", "init\n"));

    /*
     * create the containers. one indexed by ifIndex, the other
     * indexed by ifName.
     */
    container1 = netsnmp_container_find("access_ipaddress:table_container");
    if (NULL == container1) {
        snmp_log(LOG_ERR, "ipaddress primary container not found\n");
        return NULL;
    }
    container1->container_name = strdup("ia_index");
    container1->flags = CONTAINER_KEY_ALLOW_DUPLICATES;

    if (flags & NETSNMP_ACCESS_IPADDRESS_INIT_ADDL_IDX_BY_ADDR) {
        netsnmp_container *container2 =
            netsnmp_container_find("ipaddress_addr:access_ipaddress:table_container");
        if (NULL == container2) {
            snmp_log(LOG_ERR, "ipaddress secondary container not found\n");
            CONTAINER_FREE(container1);
            return NULL;
        }
        
        container2->compare = _access_ipaddress_entry_compare_addr;
        container2->container_name = strdup("ia_addr");
        /*
         * With allowed duplicates, CONTAINER_INSERT does not need to sort whole
         * container and check for duplicates. We remove duplicates manually in
         * netsnmp_access_ipaddress_container_load.
         */
        container2->flags = CONTAINER_KEY_ALLOW_DUPLICATES;
        
        netsnmp_container_add_index(container1, container2);
    }

    return container1;
}

/**
 * Remove duplicate entries from the container.
 * This function returns new copy of the container and destroys
 * the original one. Use like this:
 *   c = _remove_duplicates(c, flags);
 */
static netsnmp_container *
_remove_duplicates(netsnmp_container *container, u_int container_flags)
{
	netsnmp_container *c;
	netsnmp_iterator *it;
	netsnmp_container *ret;
	netsnmp_ipaddress_entry *entry, *prev_entry;

	if (! (container_flags & NETSNMP_ACCESS_IPADDRESS_INIT_ADDL_IDX_BY_ADDR)) {
		/* We don't have address index, we can't detect duplicates */
		return container;
	}

	ret = netsnmp_access_ipaddress_container_init(container_flags);

	/* use the IpAddress index */
	c = container->next;
	it = CONTAINER_ITERATOR(c);
	/* Sort the address index */
	CONTAINER_FIND(c, ITERATOR_FIRST(it));


	/*
	 * Sequentially iterate over sorted container and add only unique entries
	 * to 'ret'
	 */
	prev_entry = NULL;
	for (entry = ITERATOR_FIRST(it); entry; entry = ITERATOR_NEXT(it)) {
		if (prev_entry && _access_ipaddress_entry_compare_addr(prev_entry, entry) == 0) {
			/* 'entry' is duplicate of the previous one -> delete it */
			netsnmp_access_ipaddress_entry_free(entry);
		} else {
			CONTAINER_INSERT(ret, entry);
			prev_entry = entry;
		}
	}
	CONTAINER_FREE(container);
	free(it);
	return ret;
}

/**
 * @retval NULL  error
 * @retval !NULL pointer to container
 */
netsnmp_container*
netsnmp_access_ipaddress_container_load(netsnmp_container* container,
                                        u_int load_flags)
{
    int rc;
    u_int container_flags = 0;

    DEBUGMSGTL(("access:ipaddress:container", "load\n"));

    if (load_flags & NETSNMP_ACCESS_IPADDRESS_LOAD_ADDL_IDX_BY_ADDR)
        container_flags |= NETSNMP_ACCESS_IPADDRESS_INIT_ADDL_IDX_BY_ADDR;

    if (NULL == container) {
        container = netsnmp_access_ipaddress_container_init(container_flags);
    }
    if (NULL == container) {
        snmp_log(LOG_ERR, "no container specified/found for access_ipaddress\n");
        return NULL;
    }

    rc =  netsnmp_arch_ipaddress_container_load(container, load_flags);
    if (0 != rc) {
        netsnmp_access_ipaddress_container_free(container,
                                                NETSNMP_ACCESS_IPADDRESS_FREE_NOFLAGS);
        container = NULL;
    }

    if (container)
        container = _remove_duplicates(container, container_flags);

    return container;
}

void
netsnmp_access_ipaddress_container_free(netsnmp_container *container, u_int free_flags)
{
    DEBUGMSGTL(("access:ipaddress:container", "free\n"));

    if (NULL == container) {
        snmp_log(LOG_ERR, "invalid container for netsnmp_access_ipaddress_free\n");
        return;
    }

    if(! (free_flags & NETSNMP_ACCESS_IPADDRESS_FREE_DONT_CLEAR)) {
        /*
         * free all items.
         */
        CONTAINER_CLEAR(container,
                        (netsnmp_container_obj_func*)_access_ipaddress_entry_release,
                        NULL);
    }

    if(! (free_flags & NETSNMP_ACCESS_IPADDRESS_FREE_KEEP_CONTAINER))
        CONTAINER_FREE(container);
}

/**---------------------------------------------------------------------*/
/*
 * ipaddress_entry functions
 */
/**
 */
/**
 */
netsnmp_ipaddress_entry *
netsnmp_access_ipaddress_entry_create(void)
{
    netsnmp_ipaddress_entry *entry =
        SNMP_MALLOC_TYPEDEF(netsnmp_ipaddress_entry);
    int rc = 0;

    entry->oid_index.len = 1;
    entry->oid_index.oids = &entry->ns_ia_index;

    /*
     * set up defaults
     */
    entry->ia_type = IPADDRESSTYPE_UNICAST;
    entry->ia_status = IPADDRESSSTATUSTC_PREFERRED;
    entry->ia_storagetype = STORAGETYPE_VOLATILE;

    rc = netsnmp_arch_ipaddress_entry_init(entry);
    if (SNMP_ERR_NOERROR != rc) {
        DEBUGMSGT(("access:ipaddress:create","error %d in arch init\n", rc));
        netsnmp_access_ipaddress_entry_free(entry);
        entry = NULL;
    }

    return entry;
}

/**
 */
void
netsnmp_access_ipaddress_entry_free(netsnmp_ipaddress_entry * entry)
{
    if (NULL == entry)
        return;

    if (NULL != entry->arch_data)
        netsnmp_arch_ipaddress_entry_cleanup(entry);

    free(entry);
}

/**
 * update underlying data store (kernel) for entry
 *
 * @retval  0 : success
 * @retval -1 : error
 */
int
netsnmp_access_ipaddress_entry_set(netsnmp_ipaddress_entry * entry)
{
    int rc = SNMP_ERR_NOERROR;

    if (NULL == entry) {
        netsnmp_assert(NULL != entry);
        return -1;
    }
    
    /*
     * make sure interface and ifIndex match up
     */
    if (NULL == netsnmp_access_interface_name_find(entry->if_index)) {
        DEBUGMSGT(("access:ipaddress:set",
                   "cant find name for index %" NETSNMP_PRIo "d\n",
                  entry->if_index));
        return -1;
    }

    /*
     * don't support non-volatile yet
     */
    if (STORAGETYPE_VOLATILE != entry->ia_storagetype) {
        DEBUGMSGT(("access:ipaddress:set",
                   "non-volatile storagetypes unsupported\n"));
        return -1;
    }

    /*
     *
     */
    rc = -1;
    if (entry->flags & NETSNMP_ACCESS_IPADDRESS_CREATE) {
        rc = netsnmp_arch_ipaddress_create(entry);
    }
    else if (entry->flags & NETSNMP_ACCESS_IPADDRESS_CHANGE) {
    }
    else if (entry->flags & NETSNMP_ACCESS_IPADDRESS_DELETE) {
        rc = netsnmp_arch_ipaddress_delete(entry);
    }
    else {
        snmp_log(LOG_ERR,"netsnmp_access_ipaddress_entry_set with no mode\n");
        netsnmp_assert(!"ipaddress_entry_set == unknown mode"); /* always false */
        rc = -1;
    }
    
    return rc;
}

#ifndef NETSNMP_FEATURE_REMOVE_IPADDRESS_ENTRY_UPDATE
/**
 * update an old ipaddress_entry from a new one
 *
 * @note: only mib related items are compared. Internal objects
 * such as oid_index, ns_ia_index and flags are not compared.
 *
 * @retval -1  : error
 * @retval >=0 : number of fields updated
 */
int
netsnmp_access_ipaddress_entry_update(netsnmp_ipaddress_entry *lhs,
                                      netsnmp_ipaddress_entry *rhs)
{
    int rc, changed = 0;

    /*
     * copy arch stuff. we don't care if it changed
     */
    rc = netsnmp_arch_ipaddress_entry_copy(lhs,rhs);
    if (0 != rc) {
        snmp_log(LOG_ERR,"arch ipaddress copy failed\n");
        return -1;
    }

    if (lhs->if_index != rhs->if_index) {
        ++changed;
        lhs->if_index = rhs->if_index;
    }

    if (lhs->ia_storagetype != rhs->ia_storagetype) {
        ++changed;
        lhs->ia_storagetype = rhs->ia_storagetype;
    }

    if (lhs->ia_address_len != rhs->ia_address_len) {
        changed += 2;
        lhs->ia_address_len = rhs->ia_address_len;
        memcpy(lhs->ia_address, rhs->ia_address, rhs->ia_address_len);
    }
    else if (memcmp(lhs->ia_address, rhs->ia_address, rhs->ia_address_len) != 0) {
        ++changed;
        memcpy(lhs->ia_address, rhs->ia_address, rhs->ia_address_len);
    }

    if (lhs->ia_type != rhs->ia_type) {
        ++changed;
        lhs->ia_type = rhs->ia_type;
    }

    if (lhs->ia_status != rhs->ia_status) {
        ++changed;
        lhs->ia_status = rhs->ia_status;
    }

    if (lhs->ia_origin != rhs->ia_origin) {
        ++changed;
        lhs->ia_origin = rhs->ia_origin;
    }
   
    if (lhs->ia_onlink_flag != rhs->ia_onlink_flag) {
        ++changed;
        lhs->ia_onlink_flag = rhs->ia_onlink_flag;
    }

    if (lhs->ia_autonomous_flag != rhs->ia_autonomous_flag) {
        ++changed;
        lhs->ia_autonomous_flag = rhs->ia_autonomous_flag;
    }

    if (lhs->ia_prefered_lifetime != rhs->ia_prefered_lifetime) {
        ++changed;
        lhs->ia_prefered_lifetime = rhs->ia_prefered_lifetime;
    }

    if (lhs->ia_valid_lifetime != rhs->ia_valid_lifetime) {
        ++changed;
        lhs->ia_valid_lifetime = rhs->ia_valid_lifetime;
    }


    return changed;
}
#endif /* NETSNMP_FEATURE_REMOVE_IPADDRESS_ENTRY_UPDATE */

#ifndef NETSNMP_FEATURE_REMOVE_IPADDRESS_ENTRY_COPY
/**
 * copy an  ipaddress_entry
 *
 * @retval -1  : error
 * @retval 0   : no error
 */
int
netsnmp_access_ipaddress_entry_copy(netsnmp_ipaddress_entry *lhs,
                                    netsnmp_ipaddress_entry *rhs)
{
    int rc;

    /*
     * copy arch stuff. we don't care if it changed
     */
    rc = netsnmp_arch_ipaddress_entry_copy(lhs,rhs);
    if (0 != rc) {
        snmp_log(LOG_ERR,"arch ipaddress copy failed\n");
        return -1;
    }

    lhs->if_index = rhs->if_index;
    lhs->ia_storagetype = rhs->ia_storagetype;
    lhs->ia_address_len = rhs->ia_address_len;
    memcpy(lhs->ia_address, rhs->ia_address, rhs->ia_address_len);
    lhs->ia_type = rhs->ia_type;
    lhs->ia_status = rhs->ia_status;
    lhs->ia_origin = rhs->ia_origin;
    
    return 0;
}
#endif /* NETSNMP_FEATURE_REMOVE_IPADDRESS_ENTRY_COPY */

/**---------------------------------------------------------------------*/
/*
 * Utility routines
 */

#ifndef NETSNMP_FEATURE_REMOVE_IPADDRESS_PREFIX_COPY
/**
 * copy the prefix portion of an ip address
 */
int
netsnmp_ipaddress_prefix_copy(u_char *dst, u_char *src, int addr_len, int pfx_len)
{
    int    bytes = pfx_len / 8;
    int    bits = pfx_len % 8;

    if ((NULL == dst) || (NULL == src) || (0 == pfx_len))
        return 0;

    memcpy(dst, src, bytes);

    if (bytes < addr_len)
        memset(&dst[bytes],0x0, addr_len - bytes);

    if (bits) {
        u_char mask = (0xff << (8-bits));
        
        dst[bytes] = (src[bytes] & mask);
    }

    return pfx_len;
}
#endif /* NETSNMP_FEATURE_REMOVE_IPADDRESS_PREFIX_COPY */


/**
 * Compute the prefix length of a network mask
 *
 * @param  mask  network byte order mask
 *
 * @returns number of prefix bits
 */
int
netsnmp_ipaddress_ipv4_prefix_len(in_addr_t mask)
{
    int i, len = 0;
    unsigned char *mp = (unsigned char *)&mask;

    for (i = 0; i < 4; i++)
	if (mp[i] == 0xFF) len += 8;
	else break;

    if (i == 4)
	return len;

    while(0x80 & mp[i]) {
        ++len;
        mp[i] <<= 1;
    }

    return len;
}

in_addr_t netsnmp_ipaddress_ipv4_mask(int len)
{
    int i = 0, m = 0x80;
    in_addr_t mask;
    unsigned char *mp = (unsigned char *)&mask;

    if (len < 0 || len > 32) abort();

    memset(mp, 0, sizeof(mask));

    while (len >= 8) {
        mp[i] = 0xFF;
	len -= 8;
	i++;
    }
    while (len) {
        mp[i] |= m;
	m >>= 1;
	len--;
    }
    return mask;
}

int
netsnmp_ipaddress_ipv6_prefix_len(struct in6_addr mask)
{
    int i, len = 0;
    unsigned char *mp = (unsigned char *)&mask.s6_addr;

    for (i = 0; i < 16; i++)
	if (mp[i] == 0xFF) len += 8;
	else break;

    if (i == 16)
	return len;

    while(0x80 & mp[i]) {
        ++len;
        mp[i] <<= 1;
    }

    return len;
}


/**
 */
void
_access_ipaddress_entry_release(netsnmp_ipaddress_entry * entry, void *context)
{
    netsnmp_access_ipaddress_entry_free(entry);
}

static int _access_ipaddress_entry_compare_addr(const void *lhs,
                                                const void *rhs)
{
    const netsnmp_ipaddress_entry *lh = (const netsnmp_ipaddress_entry *)lhs;
    const netsnmp_ipaddress_entry *rh = (const netsnmp_ipaddress_entry *)rhs;

    netsnmp_assert(NULL != lhs);
    netsnmp_assert(NULL != rhs);

    /*
     * compare address length
     */
    if (lh->ia_address_len < rh->ia_address_len)
        return -1;
    else if (lh->ia_address_len > rh->ia_address_len)
        return 1;

    /*
     * length equal, compare address
     */
    return memcmp(lh->ia_address, rh->ia_address, lh->ia_address_len);
}

#ifndef NETSNMP_FEATURE_REMOVE_IPADDRESS_COMMON_COPY_UTILITIES
int
netsnmp_ipaddress_flags_copy(u_long *ipAddressPrefixAdvPreferredLifetime,
                             u_long *ipAddressPrefixAdvValidLifetime,
                             u_long *ipAddressPrefixOnLinkFlag,
                             u_long *ipAddressPrefixAutonomousFlag, 
                             u_long *ia_prefered_lifetime,
                             u_long *ia_valid_lifetime,
                             u_char *ia_onlink_flag,
                             u_char *ia_autonomous_flag)
{

    /*Copy all the flags*/
    *ipAddressPrefixAdvPreferredLifetime = *ia_prefered_lifetime;
    *ipAddressPrefixAdvValidLifetime = *ia_valid_lifetime;
    *ipAddressPrefixOnLinkFlag = *ia_onlink_flag;
    *ipAddressPrefixAutonomousFlag = *ia_autonomous_flag;
    return 0;
}

int
netsnmp_ipaddress_prefix_origin_copy(u_long *ipAddressPrefixOrigin,
                                     u_char ia_origin,
                                     int flags,
                                     u_long ipAddressAddrType)
{
    if(ipAddressAddrType == INETADDRESSTYPE_IPV4){
       if(ia_origin == 6) /*Random*/
          (*ipAddressPrefixOrigin) = 3 /*IPADDRESSPREFIXORIGINTC_WELLKNOWN*/;
       else
          (*ipAddressPrefixOrigin) = ia_origin;
    } else {
       if(ia_origin == 5) { /*Link Layer*/
          if(!flags) /*Global address assigned by router adv*/
             (*ipAddressPrefixOrigin) = 5 /*IPADDRESSPREFIXORIGINTC_ROUTERADV*/;
          else
             (*ipAddressPrefixOrigin) = 3 /*IPADDRESSPREFIXORIGINTC_WELLKNOWN*/;
       }
       else if(ia_origin == 6) /*Random*/
          (*ipAddressPrefixOrigin) = 5 /*IPADDRESSPREFIXORIGINTC_ROUTERADV*/;
       else
          (*ipAddressPrefixOrigin) = ia_origin;
    }
    return 0;
}
#endif /* NETSNMP_FEATURE_REMOVE_IPADDRESS_COMMON_COPY_UTILITIES */

