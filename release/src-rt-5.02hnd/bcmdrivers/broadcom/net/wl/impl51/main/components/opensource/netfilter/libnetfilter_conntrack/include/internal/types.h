/*
 * WARNING: Do *NOT* ever include this file, only for internal use!
 */
#ifndef _NFCT_TYPES_H_
#define _NFCT_TYPES_H_

/*
 * conntrack types
 */
typedef void (*set_attr)(struct nf_conntrack *ct, const void *value, size_t len);
typedef const void *(*get_attr)(const struct nf_conntrack *ct);
typedef void (*copy_attr)(struct nf_conntrack *d, const struct nf_conntrack *o);
typedef void (*filter_attr)(struct nfct_filter *filter, const void *value);
typedef int (*getobjopt)(const struct nf_conntrack *ct);
typedef void (*setobjopt)(struct nf_conntrack *ct);
typedef void (*set_attr_grp)(struct nf_conntrack *ct, const void *value);
typedef void (*get_attr_grp)(const struct nf_conntrack *ct, void *data);
typedef void (*set_filter_dump_attr)(struct nfct_filter_dump *filter_dump, const void *value);

/*
 * expectation types
 */
typedef void (*set_exp_attr)(struct nf_expect *exp, const void *value);
typedef const void *(*get_exp_attr)(const struct nf_expect *exp);

#endif
