#ifndef UCD_SNMP_PROXY_H
#define UCD_SNMP_PROXY_H

/*
 * @name: OID of the tree that is being proxied.
 * @name_len: Length of @name.
 * @base: Optional. If specified, the OID that @name is replaced with before
 *   an SNMP request is forwarded.
 * @base_len: Length of @base.
 * @context: Context string specified via <-Cn [contextname]>.
 * @sess: Session associated with this proxy.
 * @next: Next proxy in the single-linked proxy list.
 *
 * See also the Proxy Support section in the snmpd.conf(5) man page.
 */
struct simple_proxy {
    oid             name[MAX_OID_LEN];
    size_t          name_len;
    oid             base[MAX_OID_LEN];
    size_t          base_len;
    char           *context;
    netsnmp_session *sess;
    struct simple_proxy *next;
};

int             proxy_got_response(int, netsnmp_session *, int,
                                   netsnmp_pdu *, void *);
void            proxy_parse_config(const char *, char *);
void            init_proxy(void);
void            shutdown_proxy(void);
Netsnmp_Node_Handler proxy_handler;

#endif                          /* UCD_SNMP_PROXY_H */
