/*
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>

#include <net-snmp/types.h>
#include <net-snmp/library/snmp_transport.h>

#include <stdio.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <ctype.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/output_api.h>
#include <net-snmp/utilities.h>

#include <net-snmp/library/default_store.h>

#include <net-snmp/library/snmpUDPDomain.h>
#ifdef NETSNMP_TRANSPORT_TLSBASE_DOMAIN
#include <net-snmp/library/snmpTLSBaseDomain.h>
#endif
#ifdef NETSNMP_TRANSPORT_TLSTCP_DOMAIN
#include <net-snmp/library/snmpTLSTCPDomain.h>
#endif
#ifdef NETSNMP_TRANSPORT_STD_DOMAIN
#include <net-snmp/library/snmpSTDDomain.h>
#endif
#ifdef NETSNMP_TRANSPORT_TCP_DOMAIN
#include <net-snmp/library/snmpTCPDomain.h>
#endif
#ifdef NETSNMP_TRANSPORT_DTLSUDP_DOMAIN
#include <net-snmp/library/snmpDTLSUDPDomain.h>
#endif
#ifdef NETSNMP_TRANSPORT_SSH_DOMAIN
#include <net-snmp/library/snmpSSHDomain.h>
#endif
#ifdef NETSNMP_TRANSPORT_ALIAS_DOMAIN
#include <net-snmp/library/snmpAliasDomain.h>
#endif
#ifdef NETSNMP_TRANSPORT_IPX_DOMAIN
#include <net-snmp/library/snmpIPXDomain.h>
#endif
#ifdef NETSNMP_TRANSPORT_UNIX_DOMAIN
#include <net-snmp/library/snmpUnixDomain.h>
#endif
#ifdef NETSNMP_TRANSPORT_AAL5PVC_DOMAIN
#include <net-snmp/library/snmpAAL5PVCDomain.h>
#endif
#ifdef NETSNMP_TRANSPORT_UDPIPV6_DOMAIN
#include <net-snmp/library/snmpUDPIPv6Domain.h>
#endif
#ifdef NETSNMP_TRANSPORT_TCPIPV6_DOMAIN
#include <net-snmp/library/snmpTCPIPv6Domain.h>
#endif
#ifdef NETSNMP_TRANSPORT_UDPSHARED_DOMAIN
#include <net-snmp/library/snmpUDPsharedDomain.h>
#endif
#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/snmp_service.h>
#include <net-snmp/library/read_config.h>

netsnmp_feature_child_of(transport_all, libnetsnmp)

netsnmp_feature_child_of(tdomain_support, transport_all)
netsnmp_feature_child_of(tdomain_transport_oid, transport_all)
netsnmp_feature_child_of(sockaddr_size, transport_all)
netsnmp_feature_child_of(transport_cache, transport_all)

/*
 * Our list of supported transport domains.  
 */

static netsnmp_tdomain *domain_list = NULL;



/*
 * The standard SNMP domains.  
 */

oid             netsnmpUDPDomain[] = { 1, 3, 6, 1, 6, 1, 1 };
size_t          netsnmpUDPDomain_len = OID_LENGTH(netsnmpUDPDomain);
oid             netsnmpCLNSDomain[] = { 1, 3, 6, 1, 6, 1, 2 };
size_t          netsnmpCLNSDomain_len = OID_LENGTH(netsnmpCLNSDomain);
oid             netsnmpCONSDomain[] = { 1, 3, 6, 1, 6, 1, 3 };
size_t          netsnmpCONSDomain_len = OID_LENGTH(netsnmpCONSDomain);
oid             netsnmpDDPDomain[] = { 1, 3, 6, 1, 6, 1, 4 };
size_t          netsnmpDDPDomain_len = OID_LENGTH(netsnmpDDPDomain);
oid             netsnmpIPXDomain[] = { 1, 3, 6, 1, 6, 1, 5 };
size_t          netsnmpIPXDomain_len = OID_LENGTH(netsnmpIPXDomain);

static netsnmp_container *_container = NULL;


static void     netsnmp_tdomain_dump(void);


#if !defined(NETSNMP_FEATURE_REMOVE_FILTER_SOURCE)
static netsnmp_container * filtered = NULL;

void netsnmp_transport_parse_filter(const char *word, char *cptr);
#endif /* NETSNMP_FEATURE_REMOVE_FILTER_SOURCE */

void
init_snmp_transport(void)
{
    netsnmp_ds_register_config(ASN_BOOLEAN,
                               "snmp", "dontLoadHostConfig",
                               NETSNMP_DS_LIBRARY_ID,
                               NETSNMP_DS_LIB_DONT_LOAD_HOST_FILES);
#ifndef NETSNMP_FEATURE_REMOVE_FILTER_SOURCE
    register_app_config_handler("sourceFilterType",
                                netsnmp_transport_parse_filterType,
                                NULL, "none|whitelist|blacklist");
    register_app_config_handler("sourceFilterAddress",
                                netsnmp_transport_parse_filter,
                                netsnmp_transport_filter_cleanup,
                                "host");
#endif /* NETSNMP_FEATURE_REMOVE_FILTER_SOURCE */
}

void
shutdown_snmp_transport(void)
{
#ifndef NETSNMP_FEATURE_REMOVE_FILTER_SOURCE
    netsnmp_transport_filter_cleanup();
#endif
}

/*
 * Make a deep copy of an netsnmp_transport.  
 */
netsnmp_transport *
netsnmp_transport_copy(const netsnmp_transport *t)
{
    netsnmp_transport *n = NULL;

    if (t == NULL) {
        return NULL;
    }

    n = SNMP_MALLOC_TYPEDEF(netsnmp_transport);
    if (n == NULL) {
        return NULL;
    }

    if (t->domain != NULL) {
        n->domain = t->domain;
        n->domain_length = t->domain_length;
    } else {
        n->domain = NULL;
        n->domain_length = 0;
    }

    if (t->local != NULL) {
        n->local = netsnmp_memdup(t->local, t->local_length);
        if (n->local == NULL) {
            netsnmp_transport_free(n);
            return NULL;
        }
        n->local_length = t->local_length;
    } else {
        n->local = NULL;
        n->local_length = 0;
    }

    if (t->remote != NULL) {
        n->remote = netsnmp_memdup(t->remote, t->remote_length);
        if (n->remote == NULL) {
            netsnmp_transport_free(n);
            return NULL;
        }
        n->remote_length = t->remote_length;
    } else {
        n->remote = NULL;
        n->remote_length = 0;
    }

    if (t->data != NULL && t->data_length > 0) {
        n->data = netsnmp_memdup(t->data, t->data_length);
        if (n->data == NULL) {
            netsnmp_transport_free(n);
            return NULL;
        }
        n->data_length = t->data_length;
    } else {
        n->data = NULL;
        n->data_length = 0;
    }

    n->msgMaxSize = t->msgMaxSize;
    n->f_accept = t->f_accept;
    n->f_recv = t->f_recv;
    n->f_send = t->f_send;
    n->f_close = t->f_close;
    n->f_copy = t->f_copy;
    n->f_config = t->f_config;
    n->f_fmtaddr = t->f_fmtaddr;
    n->sock = t->sock;
    n->flags = t->flags;
    n->base_transport = netsnmp_transport_copy(t->base_transport);

    /* give the transport a chance to do "special things" */
    if (t->f_copy)
        t->f_copy(t, n);
                
    return n;
}



void
netsnmp_transport_free(netsnmp_transport *t)
{
    if (NULL == t)
        return;

#ifndef FEATURE_REMOVE_TRANSPORT_CACHE
    /** don't free a transport that is currently shared */
    if (netsnmp_transport_cache_remove(t) == 1)
        return;
#endif

    SNMP_FREE(t->local);
    SNMP_FREE(t->remote);
    SNMP_FREE(t->data);
    netsnmp_transport_free(t->base_transport);

    SNMP_FREE(t);
}

/*
 * netsnmp_transport_peer_string
 *
 * returns string representation of peer address.
 *
 * caller is responsible for freeing the allocated string.
 */
char *
netsnmp_transport_peer_string(netsnmp_transport *t, const void *data, int len)
{
    char           *str;

    if (NULL == t)
        return NULL;

    if (t->f_fmtaddr != NULL)
        str = t->f_fmtaddr(t, data, len);
    else
        str = strdup("<UNKNOWN>");

    return str;
}

#if !defined(NETSNMP_FEATURE_REMOVE_FILTER_SOURCE)
static int _transport_filter_init(void)
{
    if (filtered)
        return 0;

    filtered = netsnmp_container_find("transport_filter:cstring");
    if (NULL == filtered) {
        NETSNMP_LOGONCE((LOG_WARNING,
                         "couldn't allocate container for transport_filter list\n"));
        return -1;
    }
    filtered->container_name = strdup("transport_filter list");

    return 0;
}

int
netsnmp_transport_filter_add(const char *addrtxt)
{
    char *tmp;

    /*
     * create the container, if needed
     */
    if (!filtered && _transport_filter_init()) {
        snmp_log(LOG_ERR,"netsnmp_transport_filter_add %s failed\n",
                 addrtxt);
        return (-1);
    }
    tmp = strdup(addrtxt);
    if (NULL == tmp) {
        snmp_log(LOG_ERR,"netsnmp_transport_filter_add strdup failed\n");
        return(-1);
    }
    return CONTAINER_INSERT(filtered, tmp);
}

int
netsnmp_transport_filter_remove(const char *addrtxt)
{
    /*
     * create the container, if needed
     */
    if (NULL == filtered)
        return -1;
    return CONTAINER_REMOVE(filtered, addrtxt);
}

/*
 * netsnmp_transport_filter_check
 *
 * returns 1 if the specified address string is in the filter list
 */
int
netsnmp_transport_filter_check(const char *addrtxt)
{
    char *addr;
    if (NULL == filtered)
        return 0;
    addr = CONTAINER_FIND(filtered, addrtxt);
    return addr ? 1 : 0;
}

void
netsnmp_transport_parse_filterType(const char *word, char *cptr)
{
    int type = 42;
    if (strcmp(cptr,"whitelist") == 0)
        type = 1;
    else if (strcmp(cptr,"blacklist") == 0)
        type = -1;
    else if (strcmp(cptr,"none") == 0)
        type = 0;
    else
        netsnmp_config_error("unknown source filter type: %s", cptr);

    if (type != 42) {
        DEBUGMSGTL(("transport:filterType", "set to %d\n", type));
        netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID,
                           NETSNMP_DS_LIB_FILTER_TYPE, type);
    }
}

void
netsnmp_transport_parse_filter(const char *word, char *cptr)
{
    if (netsnmp_transport_filter_add(cptr))
        netsnmp_config_error("cannot create source filter: %s", cptr);
}

void
netsnmp_transport_filter_cleanup(void)
{
    if (NULL == filtered)
        return;
    CONTAINER_CLEAR(filtered, filtered->free_item, NULL);
    CONTAINER_FREE(filtered);
    filtered = NULL;
}
#endif /* NETSNMP_FEATURE_REMOVE_FILTER_SOURCE */


#ifndef NETSNMP_FEATURE_REMOVE_SOCKADDR_SIZE
int
netsnmp_sockaddr_size(const struct sockaddr *sa)
{
    if (NULL == sa)
        return 0;

    switch (sa->sa_family) {
        case AF_INET:
            return sizeof(struct sockaddr_in);
        break;
#ifdef NETSNMP_ENABLE_IPV6
        case AF_INET6:
            return sizeof(struct sockaddr_in6);
            break;
#endif
    }

    return 0;
}
#endif /* NETSNMP_FEATURE_REMOVE_SOCKADDR_SIZE */
    
int
netsnmp_transport_send(netsnmp_transport *t, const void *packet, int length,
                       void **opaque, int *olength)
{
    int dumpPacket, debugLength;

    if ((NULL == t) || (NULL == t->f_send)) {
        DEBUGMSGTL(("transport:pkt:send", "NULL transport or send function\n"));
        return SNMPERR_GENERR;
    }

    dumpPacket = netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                                        NETSNMP_DS_LIB_DUMP_PACKET);
    debugLength = (SNMPERR_SUCCESS ==
                   debug_is_token_registered("transport:send"));

    if (dumpPacket | debugLength) {
        char *str = netsnmp_transport_peer_string(t,
                                                  opaque ? *opaque : NULL,
                                                  olength ? *olength : 0);
        if (debugLength)
            DEBUGMSGT_NC(("transport:send","%lu bytes to %s\n",
                          (unsigned long)length, str));
        if (dumpPacket)
            snmp_log(LOG_DEBUG, "\nSending %lu bytes to %s\n", 
                     (unsigned long)length, str);
        SNMP_FREE(str);
    }
    if (dumpPacket)
        xdump(packet, length, "");

    return t->f_send(t, packet, length, opaque, olength);
}

int
netsnmp_transport_recv(netsnmp_transport *t, void *packet, int length,
                       void **opaque, int *olength)
{
    int debugLength;

    if ((NULL == t) || (NULL == t->f_recv)) {
        DEBUGMSGTL(("transport:recv", "NULL transport or recv function\n"));
        return SNMPERR_GENERR;
    }

    length = t->f_recv(t, packet, length, opaque, olength);

    if (length <=0)
        return length; /* don't log timeouts/socket closed */

    debugLength = (SNMPERR_SUCCESS ==
                   debug_is_token_registered("transport:recv"));

    if (debugLength) {
        char *str = netsnmp_transport_peer_string(t,
                                                  opaque ? *opaque : NULL,
                                                  olength ? *olength : 0);
        if (debugLength)
            DEBUGMSGT_NC(("transport:recv","%d bytes from %s\n",
                          length, str));
        SNMP_FREE(str);
    }

    return length;
}



#ifndef NETSNMP_FEATURE_REMOVE_TDOMAIN_SUPPORT
int
netsnmp_tdomain_support(const oid * in_oid,
                        size_t in_len,
                        const oid ** out_oid, size_t * out_len)
{
    netsnmp_tdomain *d = NULL;

    for (d = domain_list; d != NULL; d = d->next) {
        if (netsnmp_oid_equals(in_oid, in_len, d->name, d->name_length) == 0) {
            if (out_oid != NULL && out_len != NULL) {
                *out_oid = d->name;
                *out_len = d->name_length;
            }
            return 1;
        }
    }
    return 0;
}
#endif /* NETSNMP_FEATURE_REMOVE_TDOMAIN_SUPPORT */


void
netsnmp_tdomain_init(void)
{
    DEBUGMSGTL(("tdomain", "netsnmp_tdomain_init() called\n"));

/* include the configure generated list of constructor calls */
#include "transports/snmp_transport_inits.h"

    netsnmp_tdomain_dump();


}

void
netsnmp_clear_tdomain_list(void)
{
    netsnmp_tdomain *list = domain_list, *next = NULL;
    DEBUGMSGTL(("tdomain", "clear_tdomain_list() called\n"));

    while (list != NULL) {
	next = list->next;
	SNMP_FREE(list->prefix);
        /* attention!! list itself is not in the heap, so we must not free it! */
	list = next;
    }
    domain_list = NULL;
}


static void
netsnmp_tdomain_dump(void)
{
    netsnmp_tdomain *d;
    int i = 0;

    DEBUGMSGTL(("tdomain", "domain_list -> "));
    for (d = domain_list; d != NULL; d = d->next) {
        DEBUGMSG(("tdomain", "{ "));
        DEBUGMSGOID(("tdomain", d->name, d->name_length));
        DEBUGMSG(("tdomain", ", \""));
        for (i = 0; d->prefix[i] != NULL; i++) {
            DEBUGMSG(("tdomain", "%s%s", d->prefix[i],
		      (d->prefix[i + 1]) ? "/" : ""));
        }
        DEBUGMSG(("tdomain", "\" } -> "));
    }
    DEBUGMSG(("tdomain", "[NIL]\n"));
}



int
netsnmp_tdomain_register(netsnmp_tdomain *n)
{
    netsnmp_tdomain **prevNext = &domain_list, *d;

    if (n != NULL) {
        for (d = domain_list; d != NULL; d = d->next) {
            if (netsnmp_oid_equals(n->name, n->name_length,
                                d->name, d->name_length) == 0) {
                /*
                 * Already registered.  
                 */
                return 0;
            }
            prevNext = &(d->next);
        }
        n->next = NULL;
        *prevNext = n;
        return 1;
    } else {
        return 0;
    }
}



netsnmp_feature_child_of(tdomain_unregister, netsnmp_unused)
#ifndef NETSNMP_FEATURE_REMOVE_TDOMAIN_UNREGISTER
int
netsnmp_tdomain_unregister(netsnmp_tdomain *n)
{
    netsnmp_tdomain **prevNext = &domain_list, *d;

    if (n != NULL) {
        for (d = domain_list; d != NULL; d = d->next) {
            if (netsnmp_oid_equals(n->name, n->name_length,
                                d->name, d->name_length) == 0) {
                *prevNext = n->next;
		SNMP_FREE(n->prefix);
                return 1;
            }
            prevNext = &(d->next);
        }
        return 0;
    } else {
        return 0;
    }
}
#endif /* NETSNMP_FEATURE_REMOVE_TDOMAIN_UNREGISTER */


static netsnmp_tdomain *
find_tdomain(const char* spec)
{
    netsnmp_tdomain *d;
    for (d = domain_list; d != NULL; d = d->next) {
        int i;
        for (i = 0; d->prefix[i] != NULL; i++)
            if (strcasecmp(d->prefix[i], spec) == 0) {
                DEBUGMSGTL(("tdomain",
                            "Found domain \"%s\" from specifier \"%s\"\n",
                            d->prefix[0], spec));
                return d;
            }
    }
    DEBUGMSGTL(("tdomain", "Found no domain from specifier \"%s\"\n", spec));
    return NULL;
}

static int
netsnmp_is_fqdn(const char *thename)
{
    if (!thename)
        return 0;
    while(*thename) {
        if (*thename != '.' && !isupper((unsigned char)*thename) &&
            !islower((unsigned char)*thename) &&
            !isdigit((unsigned char)*thename) && *thename != '-') {
            return 0;
        }
        thename++;
    }
    return 1;
}

/*
 * Locate the appropriate transport domain and call the create function for
 * it.
 */
netsnmp_transport *
netsnmp_tdomain_transport_tspec(netsnmp_tdomain_spec *tspec)
{
    const char *application, *str, *default_domain, *default_target, *source;
    int local;
    netsnmp_tdomain    *match = NULL;
    const char         *addr = NULL;
    const char * const *spec = NULL;
    int                 any_found = 0;
    char buf[SNMP_MAXPATH];
    char **lspec = NULL;
    char *tokenized_domain = NULL;

    application = tspec->application;
    str = tspec->target;
    local = tspec->flags & NETSNMP_TSPEC_LOCAL;
    default_domain = tspec->default_domain;
    default_target = tspec->default_target;
    source = tspec->source;
    /** transport_config = tspec->transport_config; not used yet */

    DEBUGMSGTL(("tdomain",
                "tdomain_transport_spec(\"%s\", \"%s\", %d, \"%s\", \"%s\", \"%s\")\n",
                application, str ? str : "[NIL]", local,
                default_domain ? default_domain : "[NIL]",
                default_target ? default_target : "[NIL]",
                source ? source : "[NIL]"));

    /* see if we can load a host-name specific set of conf files */
    if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                                NETSNMP_DS_LIB_DONT_LOAD_HOST_FILES) &&
        netsnmp_is_fqdn(str)) {
        static int have_added_handler = 0;
        char *newhost;
        struct config_line *config_handlers;
        struct config_files file_names;
        char *prev_hostname;

        /* register a "transport" specifier */
        if (!have_added_handler) {
            have_added_handler = 1;
            netsnmp_ds_register_config(ASN_OCTET_STR,
                                       "snmp", "transport",
                                       NETSNMP_DS_LIBRARY_ID,
                                       NETSNMP_DS_LIB_HOSTNAME);
        }

        /* we save on specific setting that we don't allow to change
           from one transport creation to the next; ie, we don't want
           the "transport" specifier to be a default.  It should be a
           single invocation use only */
        prev_hostname = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID,
                                              NETSNMP_DS_LIB_HOSTNAME);
        if (prev_hostname)
            prev_hostname = strdup(prev_hostname);

        /* read in the hosts/STRING.conf files */
        config_handlers = read_config_get_handlers("snmp");
        snprintf(buf, sizeof(buf)-1, "hosts/%s", str);
        file_names.fileHeader = buf;
        file_names.start = config_handlers;
        file_names.next = NULL;
        DEBUGMSGTL(("tdomain", "checking for host specific config %s\n",
                    buf));
        read_config_files_of_type(EITHER_CONFIG, &file_names);

        if (NULL !=
            (newhost = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID,
                                             NETSNMP_DS_LIB_HOSTNAME))) {
            strlcpy(buf, newhost, sizeof(buf));
            str = buf;
        }

        netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID,
                              NETSNMP_DS_LIB_HOSTNAME,
                              prev_hostname);
        SNMP_FREE(prev_hostname);
    }

    /* First try - assume that there is a domain in str (domain:target) */

    if (str != NULL) {
        const char *cp;
        if ((cp = strchr(str, ':')) != NULL) {
            char* mystring = (char*)malloc(cp + 1 - str);
            memcpy(mystring, str, cp - str);
            mystring[cp - str] = '\0';
            addr = cp + 1;

            match = find_tdomain(mystring);
            free(mystring);
        }
    }

    /*
     * Second try, if there is no domain in str (target), then try the
     * default domain
     */

    if (match == NULL) {
        addr = str;
        if (addr && *addr == '/') {
            DEBUGMSGTL(("tdomain",
                        "Address starts with '/', so assume \"unix\" "
                        "domain\n"));
            match = find_tdomain("unix");
        } else if (default_domain) {
            DEBUGMSGTL(("tdomain",
                        "Use user specified default domain \"%s\"\n",
                        default_domain));
            if (!strchr(default_domain, ','))
                match = find_tdomain(default_domain);
            else {
                int commas = 0;
                const char *cp = default_domain;
                char *ptr = NULL;
                tokenized_domain = strdup(default_domain);

                while (*++cp) if (*cp == ',') commas++;
                lspec = calloc(commas+2, sizeof(char *));
                commas = 1;
                lspec[0] = strtok_r(tokenized_domain, ",", &ptr);
                while ((lspec[commas++] = strtok_r(NULL, ",", &ptr)))
                    ;
                spec = (const char * const *)lspec;
            }
        } else {
            spec = netsnmp_lookup_default_domains(application);
            if (spec == NULL) {
                DEBUGMSGTL(("tdomain",
                            "No default domain found, assume \"udp\"\n"));
                match = find_tdomain("udp");
            } else {
                const char * const * r = spec;
                DEBUGMSGTL(("tdomain",
                            "Use application default domains"));
                while(*r) {
                    DEBUGMSG(("tdomain", " \"%s\"", *r));
                    ++r;
                }
                DEBUGMSG(("tdomain", "\n"));
            }
        }
    }

    for(;;) {
        if (match) {
            netsnmp_transport *t = NULL;
            const char* addr2;

            any_found = 1;
            /*
             * Ok, we know what domain to try, lets see what default data
             * should be used with it
             */
            if (default_target != NULL)
                addr2 = default_target;
            else
                addr2 = netsnmp_lookup_default_target(application,
                                                      match->prefix[0]);
            DEBUGMSGTL(("tdomain",
                        "trying domain \"%s\" address \"%s\" "
                        "default address \"%s\"\n",
                        match->prefix[0], addr ? addr : "[NIL]",
                        addr2 ? addr2 : "[NIL]"));
            if (match->f_create_from_tspec) {
                netsnmp_tdomain_spec tspec_tmp;
                memcpy(&tspec_tmp, tspec, sizeof(tspec_tmp));
                /** if we didn't have a default target but looked one up,
                 *  copy the spec and use the found default. */
                if ((default_target == NULL) && (addr2 != NULL))
                    tspec_tmp.default_target = addr2;
                if (addr != tspec_tmp.target)
                    tspec_tmp.target = addr;
                t = match->f_create_from_tspec(&tspec_tmp);
            }
            else {
#if 0 /** remove warning until all transports implement tspec */
                NETSNMP_LOGONCE((LOG_WARNING,
                                 "transport domain %s uses deprecated f_create function\n",
                                 match->prefix[0]));
#endif
                if (match->f_create_from_tstring) {
                    t = match->f_create_from_tstring(addr, local);
                }
                else
                    t = match->f_create_from_tstring_new(addr, local, addr2);
            }
            if (t) {
                if (lspec) {
                    free(tokenized_domain);
                    free(lspec);
                }
                return t;
            }
        }
        addr = str;
        if (spec && *spec)
            match = find_tdomain(*spec++);
        else
            break;
    }
    if (!any_found)
        snmp_log(LOG_ERR, "No support for any checked transport domain\n");
    if (lspec) {
        free(tokenized_domain);
        free(lspec);
    }
    return NULL;
}

netsnmp_transport *
netsnmp_tdomain_transport_full(const char *application,
                               const char *str, int local,
                               const char *default_domain,
                               const char *default_target)
{
    netsnmp_tdomain_spec tspec;
    memset(&tspec, 0x0, sizeof(tspec));
    tspec.application = application;
    tspec.target = str;
    if (local)
        tspec.flags |= NETSNMP_TSPEC_LOCAL;
    tspec.default_domain = default_domain;
    tspec.default_target = default_target;
    tspec.source = NULL;
    tspec.transport_config = NULL;
    return netsnmp_tdomain_transport_tspec(&tspec);
}

netsnmp_transport *
netsnmp_tdomain_transport(const char *str, int local,
			  const char *default_domain)
{
    netsnmp_tdomain_spec tspec;
    memset(&tspec, 0x0, sizeof(tspec));
    tspec.application = "snmp";
    tspec.target = str;
    if (local)
        tspec.flags |= NETSNMP_TSPEC_LOCAL;
    tspec.default_domain = default_domain;
    tspec.default_target = NULL;
    tspec.source = NULL;
    tspec.transport_config = NULL;
    return netsnmp_tdomain_transport_tspec(&tspec);
}

#ifndef NETSNMP_FEATURE_REMOVE_TDOMAIN_TRANSPORT_OID
netsnmp_transport *
netsnmp_tdomain_transport_oid(const oid * dom,
                              size_t dom_len,
                              const u_char * o, size_t o_len, int local)
{
    netsnmp_tdomain *d;
    int             i;

    DEBUGMSGTL(("tdomain", "domain \""));
    DEBUGMSGOID(("tdomain", dom, dom_len));
    DEBUGMSG(("tdomain", "\"\n"));

    for (d = domain_list; d != NULL; d = d->next) {
        for (i = 0; d->prefix[i] != NULL; i++) {
            if (netsnmp_oid_equals(dom, dom_len, d->name, d->name_length) ==
                0) {
                return d->f_create_from_ostring(o, o_len, local);
            }
        }
    }

    snmp_log(LOG_ERR, "No support for requested transport domain\n");
    return NULL;
}
#endif /* NETSNMP_FEATURE_REMOVE_TDOMAIN_TRANSPORT_OID */

netsnmp_transport*
netsnmp_transport_open(const char* application, const char* str, int local)
{
    return netsnmp_tdomain_transport_full(application, str, local, NULL, NULL);
}

netsnmp_transport*
netsnmp_transport_open_server(const char* application, const char* str)
{
    return netsnmp_tdomain_transport_full(application, str, 1, NULL, NULL);
}

netsnmp_transport*
netsnmp_transport_open_client(const char* application, const char* str)
{
    return netsnmp_tdomain_transport_full(application, str, 0, NULL, NULL);
}

/** adds a transport to a linked list of transports.
    Returns 1 on failure, 0 on success */
int
netsnmp_transport_add_to_list(netsnmp_transport_list **transport_list,
                              netsnmp_transport *transport)
{
    netsnmp_transport_list *newptr =
        SNMP_MALLOC_TYPEDEF(netsnmp_transport_list);

    if (!newptr)
        return 1;

    newptr->next = *transport_list;
    newptr->transport = transport;

    *transport_list = newptr;

    return 0;
}


/**  removes a transport from a linked list of transports.
     Returns 1 on failure, 0 on success */
int
netsnmp_transport_remove_from_list(netsnmp_transport_list **transport_list,
                                   netsnmp_transport *transport)
{
    netsnmp_transport_list *ptr = *transport_list, *lastptr = NULL;

    while (ptr && ptr->transport != transport) {
        lastptr = ptr;
        ptr = ptr->next;
    }

    if (!ptr)
        return 1;

    if (lastptr)
        lastptr->next = ptr->next;
    else
        *transport_list = ptr->next;

    SNMP_FREE(ptr);

    return 0;
}

int
netsnmp_transport_config_compare(netsnmp_transport_config *left,
                                 netsnmp_transport_config *right) {
    return strcmp(left->key, right->key);
}

netsnmp_transport_config *
netsnmp_transport_create_config(char *key, char *value) {
    netsnmp_transport_config *entry =
        SNMP_MALLOC_TYPEDEF(netsnmp_transport_config);
    entry->key = strdup(key);
    entry->value = strdup(value);
    return entry;
}

#ifndef FEATURE_REMOVE_TRANSPORT_CACHE

/* *************************************************************************
 * transport caching by address family, type and use
 */
typedef struct trans_cache_s {
    netsnmp_transport *t;
    int af;
    int type;
    int local;
    netsnmp_sockaddr_storage bind_addr;
    int count; /* number of times this transport has been returned */
} trans_cache;

static void _tc_free_item(trans_cache *tc, void *context);
static int _tc_compare(trans_cache *lhs, trans_cache *rhs);

/** initialize transport cache */
static int
_tc_init(void)
{
    DEBUGMSGTL(("transport:cache:init", "%p\n", _container));

    /** prevent double init */
    if (NULL != _container)
        return 0;

    _container = netsnmp_container_find("trans_cache:binary_array");
    if (NULL == _container) {
        snmp_log(LOG_ERR, "failed to allocate trans_cache container\n");
        return 1;
    }

    _container->container_name = strdup("trans_cache");
    _container->free_item = (netsnmp_container_obj_func*) _tc_free_item;
    _container->compare = (netsnmp_container_compare*) _tc_compare;

    return 0;
}

/*
 * container compare function
 *
 * sort by af, type, local
 */
static int
_tc_compare(trans_cache *lhs, trans_cache *rhs)
{
    netsnmp_assert((lhs != NULL) && (rhs != NULL));

    DEBUGMSGTL(("9:transport:cache:compare", "%p/%p\n", lhs, rhs));

   if (lhs->af < rhs->af)
        return -1;
    else if (lhs->af > rhs->af)
        return 1;

    if (lhs->type < rhs->type)
        return -1;
    else if (lhs->type > rhs->type)
        return 1;

    if (lhs->local < rhs->local)
        return -1;
    else if (lhs->local > rhs->local)
        return 1;

    if (AF_INET == lhs->af) {
        struct sockaddr_in *lha = &lhs->bind_addr.sin,
            *rha = &rhs->bind_addr.sin;
        if (lha->sin_addr.s_addr < rha->sin_addr.s_addr)
            return -1;
        else if (lha->sin_addr.s_addr > rha->sin_addr.s_addr)
            return 1;

        if (lha->sin_port < rha->sin_port)
            return -1;
        else if (lha->sin_port > rha->sin_port)
            return 1;
    }
#ifdef NETSNMP_ENABLE_IPV6
    else if (AF_INET6 == lhs->af) {
        struct sockaddr_in6 *lha = &lhs->bind_addr.sin6,
            *rha = &rhs->bind_addr.sin6;
        int rc = memcmp(lha->sin6_addr.s6_addr, rha->sin6_addr.s6_addr,
                        sizeof(rha->sin6_addr.s6_addr));
        if (rc)
            return rc;

        if (lha->sin6_port < rha->sin6_port)
            return -1;
        else if (lha->sin6_port > rha->sin6_port)
            return 1;

        if (lha->sin6_flowinfo < rha->sin6_flowinfo)
            return -1;
        else if (lha->sin6_flowinfo > rha->sin6_flowinfo)
            return 1;

        if (lha->sin6_scope_id < rha->sin6_scope_id)
            return -1;
        else if (lha->sin6_scope_id > rha->sin6_scope_id)
            return 1;
    }
#endif
    return 0;
}

static void
_tc_free(trans_cache *tc)
{
    if (NULL == tc)
        return;

    DEBUGMSGTL(("transport:cache:free", "%p %d/%d/%d/%p %d\n", tc, tc->af,
                tc->type, tc->local, tc->t, tc->count));
    netsnmp_transport_free(tc->t);
    memset(tc, 0x0, sizeof(*tc));
    free(tc);
}

static void
_tc_free_item(trans_cache *tc, void *context)
{
    _tc_free(tc);
}

static void
_tc_remove(trans_cache *tc)
{
    if (NULL == tc || NULL == _container)
        return;

    DEBUGMSGTL(("transport:cache:remove", "%p\n", tc));

    CONTAINER_REMOVE(_container, tc);
}

static trans_cache *
_tc_create(int af, int type, int local, const netsnmp_sockaddr_storage *addr,
           netsnmp_transport *t)
{
    trans_cache *tc = SNMP_MALLOC_TYPEDEF(trans_cache);
    if (NULL == tc) {
        snmp_log(LOG_ERR, "failed to allocate trans_cache\n");
        return NULL;
    }
    DEBUGMSGTL(("transport:cache:create", "%p\n", tc));
    tc->af = af;
    tc->type = type;
    tc->local = local;
    tc->t = t;
    if (addr)
        memcpy(&tc->bind_addr, addr, sizeof(tc->bind_addr));
    /** we only understand ipv6 and ipv6 sockaddrs in compare */
    if (AF_INET != tc->af && AF_INET6 != tc->af)
        NETSNMP_LOGONCE((LOG_WARNING, "transport cache not tested for af %d\n",
                         tc->af));
    return tc;
}

static trans_cache *
_tc_add(int af, int type, int local, const netsnmp_sockaddr_storage *addr,
        netsnmp_transport *t)
{
    trans_cache *tc;
    int rc;

    DEBUGMSGTL(("transport:cache:add", "%d/%d/%d/%p\n", af, type, local, t));

    if (NULL == _container) {
        _tc_init();
        if (NULL == _container)
            return NULL;
    }

    tc = _tc_create(af, type, local, addr, t);
    if (NULL == tc) {
        DEBUGMSGTL(("transport:cache:add",
                    "could not create transport cache\n"));
        return NULL;
    }

    rc = CONTAINER_INSERT(_container, tc);
    if (rc) {
        DEBUGMSGTL(("transport:cache:add", "container insert failed\n"));
        _tc_free(tc);
        return NULL;
    }

    return tc;
}

trans_cache *
_tc_find(int af, int type, int local, const netsnmp_sockaddr_storage *addr)
{
    trans_cache tc, *rtn;

    DEBUGMSGTL(("transport:cache:find", "%d/%d/%d\n", af, type, local));

    if (NULL == _container)
        return NULL;

    memset(&tc, 0x00, sizeof(tc));
    tc.af = af;
    tc.type = type;
    tc.local = local;
    if (addr)
        memcpy(&tc.bind_addr, addr, sizeof(tc.bind_addr));

    rtn = CONTAINER_FIND(_container, &tc);
    DEBUGMSGTL(("transport:cache:find", "%p\n", rtn));
    return rtn;
}

trans_cache *
_tc_find_transport(netsnmp_transport *t)
{
    /*
     * we shouldn't really have that many transports, so instead of
     * using an additional key, just iterate over the whole container.
     */
    netsnmp_iterator  *itr;
    trans_cache *tc;

    DEBUGMSGTL(("transport:cache:find_transport", "%p\n", t));

    if (NULL == _container)
        return NULL;

    itr = CONTAINER_ITERATOR(_container);
    if (NULL == itr) {
        snmp_log(LOG_ERR, "could not get iterator for transport cache\n");
        return NULL;
    }

    tc = ITERATOR_FIRST(itr);
    for( ; tc; tc = ITERATOR_NEXT(itr))
        if (tc->t == t)
            break;
    ITERATOR_RELEASE(itr);

    DEBUGMSGT(("transport:cache:find_transport","found %p\n", tc));

    return tc;
}

int
netsnmp_transport_cache_remove(netsnmp_transport *t)
{
    trans_cache *tc;

    DEBUGMSGTL(("transport:cache:close", "%p\n", t));

    if (NULL == t)
        return 0;

    /** transport in cache? */
    tc = _tc_find_transport(t);
    if (NULL == tc) {
        DEBUGMSGTL(("transport:cache:close", "%p not found in cache\n", t));
        return 0;
    }

    --tc->count;

    /** still in use? */
    if (tc->count > 0) {
        DEBUGMSGTL(("transport:cache:close", "still %d user(s) of %p\n",
                    tc->count, t));
        return 1;
    }

    /** unbalanced get/close? */
    if (tc->count < 0)
        snmp_log(LOG_WARNING, "transport cache get/close mismatch\n");

    _tc_remove(tc);
    _tc_free(tc); /* also does close */

    return 0;
}

/*
 * netsnmp_transport_get: get a (possibly duplicate, cached) transport
 */
netsnmp_transport *
netsnmp_transport_cache_get(int af, int type, int local,
                            const netsnmp_sockaddr_storage *bind_addr)
{
    trans_cache       *tc;
    netsnmp_transport *t;

    DEBUGMSGTL(("transport:cache:get", "%d/%d/%d\n", af, type, local));

#define USE_CACHE 1

#if USE_CACHE
    /** check for existing transport */
    tc = _tc_find(af, type, local, bind_addr);
    if (tc) {
        DEBUGMSGTL(("transport:cache:get", "using existing transport %p\n",
                    tc->t));
        ++tc->count;
        return tc->t;
    }
#endif
    /** get transport */
    t = NULL; /* _transport(af, type, 0);*/
    if (NULL == t) {
        snmp_log(LOG_ERR, "could not get new transport for %d/%d/%d\n", af,
                 type, local);
        return NULL;
    }
    DEBUGMSGTL(("transport:cache:get", "new transport %p\n", t));

#if USE_CACHE
    /** create transport cache for new transport */
    tc = _tc_add(af, type, local, bind_addr, t);
    if (NULL == tc) {
        DEBUGMSGTL(("transport:cache:get", "could not create transport cache\n"));
        /*
         * hmmm.. this isn't really a critical error, is it? We have a
         * transport, just no cache for it. Let's continue on and hope for the
         * best.
         */
        /** return -1; */
    }
    tc->count = 1;
#endif

    return t;
}

int
netsnmp_transport_cache_save(int af, int type, int local,
                             const netsnmp_sockaddr_storage *addr,
                             netsnmp_transport *t)
{
    if (NULL == t)
        return 1;

    if (NULL == _tc_add(af, type, local, addr, t))
        return 1;

    return 0;
}
#endif /* FEATURE_REMOVE_TRANSPORT_CACHE */
